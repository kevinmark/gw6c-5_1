/*
---------------------------------------------------------------------------
 $Id: log.c,v 1.27 2007/11/28 17:27:30 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <time.h>
#include <string.h>

#define _USES_SYSLOG_H_
#define _USES_PTHREAD_H_

#include "platform.h"

#define LOG_IT
#define TEST 0

#include "log.h"
#include "config.h"
#include "buffer.h"
#include "hex_strings.h"

static FILE *Logfp;
static tLogConfiguration *LogConfiguration = NULL;
static Buffer LogBuffer;
MUTEX logMutex;


// --------------------------------------------------------------------------
// Returns a printable character representing a severity level.
//
char SeverityToChar( const enum tSeverityLevel sLvl )
{
  switch( sLvl )
  {
  case ELError:     return 'E';
  case ELWarning:   return 'W';
  case ELInfo:      return 'I';
  case ELDebug:     return 'D';
  }

  // Never reached
  return 'E';
}


// --------------------------------------------------------------------------
/* Generate a name for the backed up ('rotated') log file. */
static int GetLogFileBackupName(char *filename, char *backupname)
{
  time_t t;
  struct tm *tm;
  char smallname[LOG_FILENAME_MAX_LENGTH];
  char extension[LOG_FILENAME_MAX_LENGTH];
  char *lastdot;

  if ((filename == NULL) || (backupname == NULL)) {
    return 1;
  }

  /* Create a timestamp with the current date and time. */
    if ((t = time(NULL)) == (time_t)-1) {
    return 1;
  }

    if ((tm = localtime(&t)) == NULL) {
    return 1;
  }

  memset(smallname, 0, LOG_FILENAME_MAX_LENGTH);
  memset(extension, 0, LOG_FILENAME_MAX_LENGTH);

  /* Find the last dot, which should be where the extension starts. */
  lastdot = strrchr(filename, '.');

  /* If we find an extension, split the filename in two */
  if (lastdot != NULL) {
    strncpy(smallname, filename, lastdot - filename);
    strncpy(extension, lastdot, strlen(lastdot));

    snprintf(backupname, LOG_FILENAME_MAX_LENGTH, "%s.%04d%02d%02d%02d%02d%02d%s",
                            smallname,
                            tm->tm_year + 1900,
                            tm->tm_mon + 1,
                            tm->tm_mday,
                            tm->tm_hour,
                            tm->tm_min,
                            tm->tm_sec,
                            extension);
  }
  else {
    snprintf(backupname, LOG_FILENAME_MAX_LENGTH, "%s.%04d%02d%02d%02d%02d%02d",
                            filename,
                            tm->tm_year + 1900,
                            tm->tm_mon + 1,
                            tm->tm_mday,
                            tm->tm_hour,
                            tm->tm_min,
                            tm->tm_sec);
  }

  return 0;
}

// --------------------------------------------------------------------------
/* Copy the original log file to the backup ('rotated') file. */
static int CopyLogFileToBackup(char *filename, char *backupname)
{
  FILE *from, *to;
    char buffer[MAX_LOG_LINE_LENGTH];

  if ((filename == NULL) || (backupname == NULL)) {
    return 1;
  }

  /* Open the source and destination files. */
  if ((from = fopen(filename, "r")) == NULL) {
    return 1;
  }

  if ((to = fopen(backupname, "w+")) == NULL) {
    fclose(from);
    return 1;
  }

  /* Loop, reading line after line from the source file. */
    while (fgets(buffer, MAX_LOG_LINE_LENGTH, from)) {
    /* Copy each read line to the destination file. */
    if (fputs(buffer, to) == EOF) {
      fclose(from);
      fclose(to);

      return 1;
    }
    }

  /* Check if we stopped looping because something went wrong. */
    if (!feof(from)) {
    return 1;
    }

  /* Close the source and destination files. */
  fclose(from);
  fclose(to);

  return 0;
}


// --------------------------------------------------------------------------
/* A chance to do something before the log file is closed and */
/* rotation to the backup file occurs. */
static int RotationPendingHook()
{
  time_t t;
  struct tm *tm;
  char concat_buffer[MAX_LOG_LINE_LENGTH];

  /* Get a timestamp to prepend to the message */
    if ((t = time(NULL)) == (time_t)-1) {
    return 1;
  }

    if ((tm = localtime(&t)) == NULL) {
    return 1;
  }

  /* Concatenate the timestamp and the message. */
    snprintf(concat_buffer,
    sizeof(concat_buffer),
      "%04d/%02d/%02d %02d:%02d:%02d %s: %s\n",
      (tm->tm_year+1900),
      (tm->tm_mon+1),
      tm->tm_mday,
      tm->tm_hour,
      tm->tm_min,
      tm->tm_sec,
        ((LogConfiguration == NULL) || (LogConfiguration->identity == NULL)) ? LOG_IDENTITY : LogConfiguration->identity,
    HEX_STR_MESSAGE_ROTATING);

  /* Try to write to the open log file, if any. */
  if (Logfp != NULL) {
    if (fprintf(Logfp, "%s", concat_buffer) < 0) {
      return 1;
    }
    else {
      fflush(Logfp);
      return 0;
    }
  }
  else {
    return 1;
  }
}

// --------------------------------------------------------------------------
/* Rotate the log file. This basically means moving the file to a */
/* new name, and continuing to write to the same filename but with the */
/* previous contents gone. */
static int RotateLogFile(char *filename, int max_size, char *log_line)
{
  struct stat stats;
  char backup_file_name[LOG_FILENAME_MAX_LENGTH + 1];
  size_t delta = 0;

  /* Make sure there's a valid file pointer. */
  if (Logfp == NULL) {
    return 1;
  }

  /* Make sure there's a file name. */
  if (filename == NULL) {
    // [Temp removal] DirectErrorMessage(HEX_STR_CANT_ROTATE_LOG_NO_FILENAME);
    return 1;
  }

  /* Flush to make sure everything is in sync. */
  if (fflush(Logfp) != 0) {
    // [Temp removal] DirectErrorMessage(HEX_STR_CANT_ROTATE_LOG_CANT_FLUSH);
    return 1;
  }

  /* Stat the file to get its size. */
  if (fstat(fileno(Logfp), &stats) != 0) {
    // [Temp removal] DirectErrorMessage(HEX_STR_CANT_ROTATE_LOG_CANT_STAT);
    return 1;
  }

  /* Determine the size of what we want to add to the file. */
  if (log_line != NULL) {
    delta = strlen(log_line);
  }

  /* If we're going to blow the limit... */
  if ((int)(stats.st_size + delta) >= (max_size * 1024)) {
    /* Need to do something before we close the file for rotation? */
    if (RotationPendingHook() != 0) {
      // Nothing for now
    }

    /* Close the file. */
    fclose(Logfp);

    /* If we're configured to delete rotated logs, skip this step. */
    if( LogConfiguration->delete_rotated_log == FALSE )
    {
      /* Get a file name to save the current file under. */
      if (GetLogFileBackupName(filename, backup_file_name) != 0) {
        // [Temp removal] DirectErrorMessage(HEX_STR_CANT_ROTATE_LOG_BACKUP_NAME);
        return 1;
      }

      /* Copy the current contents to that backup file. */
      if (CopyLogFileToBackup(filename, backup_file_name) != 0) {
        // [Temp removal] DirectErrorMessage(HEX_STR_CANT_ROTATE_LOG_CANT_COPY);
        return 1;
      }
    }

    /* Reopen the current file, and start from scratch. */
    if ((Logfp = fopen(filename, "w")) == NULL) {
      // [Temp removal] DirectErrorMessage(HEX_STR_CANT_ROTATE_LOG_CANT_OPEN_NEW);
      return 1;
    }

  }

  return 0;
}

// --------------------------------------------------------------------------
/* Send a message to syslog. */
static int LogToSyslog(int VerboseLevel, enum tSeverityLevel SeverityLvl, const char *FunctionName, char *Format, ...)
{
  va_list argp;
  char buffer[MAX_LOG_LINE_LENGTH];
  char line_to_log[MAX_LOG_LINE_LENGTH];

  va_start(argp, Format);
  vsnprintf(line_to_log, sizeof(line_to_log), Format, argp);
  va_end(argp);

  /* Store what we want to send to syslog in a buffer, prepending */
  /* the function name if it's a debug build. */
#ifdef _DEBUG
  snprintf(buffer, sizeof(buffer),  " %s: %s", FunctionName, line_to_log);
#else
  snprintf(buffer, sizeof(buffer),  "%s", line_to_log);
#endif

  /* Send the message to syslog using the platform-specific code. */
  /* Made this a switch case in case we want to do more stuff based */
  /* on the log level in the future. */
  switch( SeverityLvl )
  {
    case ELError:
      SYSLOG(LOG_ERR, buffer); break;

    case ELWarning:
      SYSLOG(ELWarning, buffer); break;

    case ELInfo:
    case ELDebug:
      SYSLOG(LOG_DEBUG, buffer); break;
  }

  return 0;
}

// --------------------------------------------------------------------------
/* Dump a line of log that was stored in a buffer to file. */
static int LogBufferLineToFile(char *LogLine, tLogConfiguration *configuration, int *OutputBufferChars)
{
  char buffer[MAX_LOG_LINE_LENGTH];
  int output_chars;

  /* No configuration is bad. */
  if (configuration == NULL) {
    // [Temp removal] DirectErrorMessage(HEX_STR_NO_CONFIG_CANNOT_LOG_TO_FILE);
    *OutputBufferChars = 0;
    return 1;
  }

  /* No log filename in the configuration is bad. */
  if (configuration->log_filename == NULL) {
    // [Temp removal] DirectErrorMessage(HEX_STR_NO_LOG_FILENAME_IN_CONFIG);
    *OutputBufferChars = 0;
    return 1;
  }

  /* A closed logging file is bad. */
  if (Logfp == NULL) {
    // [Temp removal] DirectErrorMessage(HEX_STR_LOG_FILE_CLOSED, configuration->log_filename);
    *OutputBufferChars = 0;
    return 1;
  }

  /* Trying to dump nothing is bad. */
  if (LogLine == NULL) {
    *OutputBufferChars = 0;
    return 1;
  }

  /* Write to a local temporary buffer */
  output_chars = snprintf(buffer, sizeof(buffer), "%s", LogLine);

  /* Rotate the log file (if needed) before we write this new line. */
  if (configuration->log_rotation == TRUE) {
    if (RotateLogFile(configuration->log_filename, configuration->log_rotation_size, buffer) != 0) {
      // [Temp removal] DirectErrorMessage(HEX_STR_ERR_ROTATING_LOG_FILE);
    }
  }

  /* Write the contents of the local temporary buffer to the log file. */
  if (fprintf(Logfp, "%s", buffer) < 0) {
    // [Temp removal] DirectErrorMessage(HEX_STR_CANT_FPRINTF_TO_LOG);
    *OutputBufferChars = 0;
    return 1;
  }

  /* Flush to make sure everything is in the file. */
  if (fflush(Logfp) != 0) {
    *OutputBufferChars = 0;
    return 1;
  }

  *OutputBufferChars = output_chars;

  return 0;
}


// --------------------------------------------------------------------------
/* Write a log message to the log file. */
static int LogToFile(int buffer, enum tSeverityLevel SeverityLvl, const char *FunctionName, char *Format, ...)
{
  va_list argp;
  time_t t;
  struct tm *tm;
  char *s1, *s2;
  size_t i, j;
  char line_to_log[MAX_LOG_LINE_LENGTH];
  char temp_buffer[MAX_LOG_LINE_LENGTH];


  /* We don't want to use the temporary file logging buffer, but we don't */
  /* have an open file. That won't work. */
  if( (Logfp == NULL) && (buffer == 0) )
  {
    return 1;
  }

  va_start(argp, Format);
  vsnprintf(line_to_log, sizeof(line_to_log), Format, argp);
  va_end(argp);

  /* Get a timestamp to prepend to the message */
  t = time(NULL);
  if( t == (time_t)-1 )
  {
    return 1;
  }

  tm = localtime(&t);
  if( tm == NULL )
  {
    return 1;
  }

  i = strlen(line_to_log);
  s1 = s2 = malloc(i + 1);

  if( s1 == NULL )
  {
    return 1;
  }

  /* Remove EOL characters from the message */
  for( j = 0;j < i; j++ )
  {
    if( line_to_log[j] != '\r' && line_to_log[j] != '\n' )
    {
      *s1++ = line_to_log[j];
    }
  }

  *s1++ = '\0';

  /* Concatenate everything into one single local buffer. */
  snprintf(temp_buffer,
    sizeof(temp_buffer),
#ifdef _DEBUG
    "%04d/%02d/%02d %02d:%02d:%02d %c %s: %s: %s\n",
#else
    "%04d/%02d/%02d %02d:%02d:%02d %c %s: %s\n",
#endif
    (tm->tm_year+1900),
    (tm->tm_mon+1),
    tm->tm_mday,
    tm->tm_hour,
    tm->tm_min,
    tm->tm_sec,
    SeverityToChar( SeverityLvl ),
    LogConfiguration->identity == NULL ? "" : LogConfiguration->identity,
#ifdef _DEBUG
    FunctionName == NULL ? "" : FunctionName,
#endif
    s2 );

  free(s2);


  if( buffer != 0 )
  {
    /* If we're using the log file buffer (logging to file, but we don't */
    /* know the file name yet), add the message to the buffer. */
    buffer_append(&LogBuffer, (void *)temp_buffer, strlen(temp_buffer) + 1);
  }
  else
  {
    /* Rotate the log file if required. */
    if( LogConfiguration->log_rotation == TRUE )
    {
      if( RotateLogFile(LogConfiguration->log_filename, LogConfiguration->log_rotation_size, temp_buffer) != 0 )
      {
      }
    }

    /* Write the concatenated string to the log file. */
    if( fprintf(Logfp, "%s", temp_buffer) < 0 )
    {
      return 1;
    }

    /* Make sure everything is there by flushing the log file. */
    if( fflush(Logfp) != 0 )
    {
      return 1;
    }
  }

  return 0;
}

// --------------------------------------------------------------------------
/* Send a message to the console (stdout) or stderr */
static int LogToLocal(FILE *location, char *Format, ...)
{
  static char buffer[MAX_LOG_LINE_LENGTH];
  va_list argp;

  va_start(argp, Format);

  vsnprintf(buffer, sizeof(buffer), Format, argp);

  va_end(argp);

  /* location should be stdout or stderr. Print to that. */
  if (fprintf(location, "%s\n", buffer) < 0) {
    return 1;
  }

  return 0;
}

// --------------------------------------------------------------------------
// This function is the main logging function.
// Input:
// - VerboseLevel: The internal verbosity level assigned to the message.
// - SeverityLvl:  The message severity
//
void Display(int VerboseLevel, enum tSeverityLevel SeverityLvl, const char *func, char *format, ...)
{
  va_list argp;
  int i, j;
  char fmt[5000];
  char clean[5000];

#ifndef _DEBUG
  // This is a RELEASE build. Remove debug messages.
  if( SeverityLvl == ELDebug )
  {
    return;
  }
#endif


  MUTEX_LOCK(&logMutex);

  va_start(argp, format);
  vsnprintf(fmt, sizeof(fmt), format, argp);
  va_end(argp);

  /* Change CRLF to LF for log output */
  for( i = 0, j = 0; i < sizeof(fmt); i++ )
  {
    if( fmt[i] == '\r' && fmt[i + 1] == '\n' )
    {
      continue;
    }

    clean[j++] = fmt[i];
    if( fmt[i] == '\0' )
    {
      break;
    }
  }

  if( LogConfiguration == NULL )
  {
    MUTEX_UNLOCK(&logMutex);
    return;
  }


  /* Level says we should log the message to the console. */
  if( VerboseLevel <= LogConfiguration->log_level_console )
  {
    /* Log to the console. */
    LogToLocal( stdout, clean );
  }

  /* Level says we should log the message to stderr. */
  if( VerboseLevel <= LogConfiguration->log_level_stderr )
  {
    /* Log to stderr. */
    LogToLocal( stderr, clean );
  }

  /* Level says we should log the message to file. */
  if( VerboseLevel <= LogConfiguration->log_level_file )
  {
    /* Log to file. */
    LogToFile( LogConfiguration->buffer, SeverityLvl, func, clean );
  }

  /* Level says we should log the message to syslog. */
  if( VerboseLevel <= LogConfiguration->log_level_syslog )
  {
    /* Log to syslog. */
    LogToSyslog( VerboseLevel, SeverityLvl, func, clean );
  }

  MUTEX_UNLOCK(&logMutex);
}

// --------------------------------------------------------------------------
/* This is the function used to try to send some log out */
/* when there's a problem with the logging system itself. */
/* It tries to write to stderr, and then to the open log file. */
/* If there's no open log file, it will try to open the */
/* default log file, and write to that. */
int DirectErrorMessage(char *message, ...)
{
  va_list argp;
  time_t t;
  struct tm *tm;
  char buffer[MAX_LOG_LINE_LENGTH];
  char concat_buffer[MAX_LOG_LINE_LENGTH];
#ifdef WIN32 // Just to remove an 'unused variable' warning on !WIN32
  FILE *file;
#endif


  MUTEX_LOCK(&logMutex);

  /* Write the message to a local buffer. */
  va_start(argp, message);
  vsnprintf(buffer, sizeof(buffer), message, argp);
  va_end(argp);

  /* Write the message to stderr. */
  if (fprintf(stderr, "%s\n", buffer) < 0) {
    MUTEX_UNLOCK(&logMutex);
    return 1;
  }

  /* Get a timestamp for file logging. */
    if ((t = time(NULL)) == (time_t)-1) {
    MUTEX_UNLOCK(&logMutex);
    return 1;
  }

    if ((tm = localtime(&t)) == NULL) {
    MUTEX_UNLOCK(&logMutex);
    return 1;
  }

  /* Concatenate the timestamp and the message. */
    snprintf(concat_buffer,
    sizeof(concat_buffer),
      "%04d/%02d/%02d %02d:%02d:%02d %s: %s\n",
      (tm->tm_year+1900),
      (tm->tm_mon+1),
      tm->tm_mday,
      tm->tm_hour,
      tm->tm_min,
      tm->tm_sec,
        ((LogConfiguration == NULL) || (LogConfiguration->identity == NULL)) ? LOG_IDENTITY : LogConfiguration->identity,
    buffer);

  /* Try to write to the open log file, if any. */
  if (Logfp != NULL) {
    if (fprintf(Logfp, "%s", concat_buffer) >= 0) {
      fflush(Logfp);
      MUTEX_UNLOCK(&logMutex);
      return 0;
    }
  }

/* On windows, we also try to log to the default log file to give */
/* the GUI one more chance to have something to show to the user. */
#ifdef WIN32
  /* If that didn't work, try to open the default log file, and write to it. */
  if ((file = fopen(DEFAULT_LOG_FILENAME, "a+")) == NULL) {
    MUTEX_UNLOCK(&logMutex);
    return 1;
  }
  else {
    if (fprintf(file, "%s", concat_buffer) >= 0) {
      fflush(file);
      fclose(file);

      MUTEX_UNLOCK(&logMutex);

      return 0;
    }
    else {
      fclose(file);

      MUTEX_UNLOCK(&logMutex);

      return 1;
    }
  }
#endif

  MUTEX_UNLOCK(&logMutex);

  return 1;
}


// --------------------------------------------------------------------------
/* Free a logging configuration object that we have allocated. */
static void FreeLogConfiguration(tLogConfiguration *configuration)
{
  if (configuration != NULL)
  {
    /* 'identity' comes from a strdup(). */
    if (configuration->identity != NULL) {
      free(configuration->identity);
    }
    /* 'log_filename' comes from a strdup(). */
    if (configuration->log_filename != NULL) {
      free(configuration->log_filename);
    }

    free(configuration);
  }
}

// --------------------------------------------------------------------------
/* Configure the logging system with the values in the configuration structure. */
int LogConfigure(tLogConfiguration *configuration)
{
  /* We have to initialize the file logging buffer once. */
  /* Once we've done it, remember it. */
  static int LogBufferInitialized = 0;
  static int LogMutexInitialized = 0;
  int OutputBufferChars = 0;

  /* If we haven't done so already, initialize the */
  /* file logging buffer. */
  if (LogBufferInitialized == 0) {
    buffer_init(&LogBuffer);
    LogBufferInitialized = 1;
  }

  /* If we haven't done so already, initialize the */
  /* logging mutex. */
  if (LogMutexInitialized == 0) {
    MUTEX_INIT(&logMutex);
    LogMutexInitialized = 1;
  }

  /* We expect to be sent a configuration to use... */
  if (configuration == NULL) {
    DirectErrorMessage(HEX_STR_LOG_CFG_RECEIVED_NULL_CFG);

    if (LogConfiguration != NULL) {
      FreeLogConfiguration(LogConfiguration);
      LogConfiguration = NULL;
    }

    return 1;
  }

  /* If the configuration to apply says we want to log to file... */
  if (configuration->log_level_file > LOG_LEVEL_DISABLED) {
    /* If we know which file name to log to. */
    if (configuration->log_filename != NULL) {
      /* If there's a log file currently open. */
      if (Logfp != NULL) {
        /* If there was no previous configuration or the new configuration */
        /* specifies a different file name, we can't continue using that */
        /* open file. */
        if ((LogConfiguration == NULL) || (LogConfiguration->log_filename == NULL) ||
        (strcmp(LogConfiguration->log_filename, configuration->log_filename) != 0)) {
          /* Make sure everything goes to the file by flushing it. */
          fflush(Logfp);
          /* Close the file. */
          fclose(Logfp);

          /* We then need to open the logging file again using the new */
          /* logging file name. */
          if ((Logfp = fopen(configuration->log_filename, "a")) == NULL) {
                DirectErrorMessage(HEX_STR_CANNOT_OPEN_LOG_FILE, configuration->log_filename);

            if (LogConfiguration != NULL) {
              FreeLogConfiguration(LogConfiguration);
              LogConfiguration = NULL;
            }

            FreeLogConfiguration(configuration);

            return 1;
          }
        }
      }
      /* Otherwise, there's no configuration file currently open. */
      else {
        /* Therefore, we open the file using the file name specified in the configuration. */
        if ((Logfp = fopen(configuration->log_filename, "a")) == NULL) {
          DirectErrorMessage(HEX_STR_CANNOT_OPEN_LOG_FILE, configuration->log_filename);

          if (LogConfiguration != NULL) {
            FreeLogConfiguration(LogConfiguration);
            LogConfiguration = NULL;
          }

          FreeLogConfiguration(configuration);

          return 1;
        }
      }


      /* While there's something in the buffer... */
      while (buffer_len(&LogBuffer) > 0) {
        /* Write the next line of logging data to the log file */
        if (LogBufferLineToFile(buffer_ptr(&LogBuffer), configuration, &OutputBufferChars) != 0) {
          DirectErrorMessage(HEX_STR_CANT_WRITE_LOG_BUFFER_TO_FILE);
        }
        OutputBufferChars++;
        /* And 'consume' the characters we just dumped to file */
        buffer_consume(&LogBuffer, OutputBufferChars);
      }

      /* Reset the log file buffer. */
      buffer_clear(&LogBuffer);
    }
  }
  /* If the configuration to apply says we don't want to log to file... */
  else {
    /* Reset the log file buffer. */
    buffer_clear(&LogBuffer);

    /* If the log file is currently open, flush the contents and close it. */
    if (Logfp != NULL) {
      fflush(Logfp);
      fclose(Logfp);
    }
  }

  /* If the configuration to apply says we want to log to syslog... */
  if (configuration->log_level_syslog > LOG_LEVEL_DISABLED) {
    /* If there's no previous configuration.. */
    if (LogConfiguration == NULL) {
      /* Just open syslog. */
      OPENLOG(configuration->identity, 0, configuration->syslog_facility);
    }
    /* But if there was a previous configuration.. */
    else {
      /* And it specified that syslog logging was enabled... */
      if (LogConfiguration->log_level_syslog > LOG_LEVEL_DISABLED) {
        /* If the facility or identity changed, we can't continue */
        /* with the same open handle to syslog. */
        if ((LogConfiguration == NULL) || (LogConfiguration->identity == NULL) ||
        (strcmp(LogConfiguration->identity, configuration->identity) != 0) ||
        (LogConfiguration->syslog_facility != configuration->syslog_facility)) {
          /* Close syslog */
          CLOSELOG();
          /* Open it again with the new identity and facility */
          OPENLOG(configuration->identity, 0, configuration->syslog_facility);
        }
      }
      /* If the previous configuration had syslog turned off.. */
      else {
        /* Open syslog */
        OPENLOG(configuration->identity, 0, configuration->syslog_facility);
      }
    }
  }
  /* If the configuration to apply says we shouldn't log to syslog */
  else {
    /* And syslog is open because of the previous configuration */
    if ((LogConfiguration != NULL) && (LogConfiguration->log_level_syslog > LOG_LEVEL_DISABLED)) {
      /* Close syslog */
      CLOSELOG();
    }
  }

  /* Free the previous configuration if there was one. */
  if (LogConfiguration != NULL) {
    FreeLogConfiguration(LogConfiguration);
  }

  /* The current configuration is now the new one. */
  LogConfiguration = configuration;

  return 0;
}

// --------------------------------------------------------------------------
/* Dump the file log buffer to file. */
/* This is used to write the buffer to the default */
/* log file if something happens before the configuration */
/* file has been parsed, and the new configuration has */
/* been applied. */
int DumpBufferToFile(char *filename)
{
  tLogConfiguration *log_configuration;

  /* Check that we are provided with a file name */
  if (filename == NULL) {
    return 1;
  }

  /* Allocate memory for the logging configuration structure */
  log_configuration = (tLogConfiguration *)malloc(sizeof(tLogConfiguration));

  /* Validate that memory could be allocated correctly */
  if (log_configuration == NULL) {
    DirectErrorMessage(HEX_STR_COULD_NOT_MALLOC_FOR_CONFIG);

    LogClose();

    return 1;
  }

  /* Fill the logging configuration with values that will make the */
  /* logging system dump the buffer to file */
  log_configuration->identity = strdup(LOG_IDENTITY);
  log_configuration->log_filename = strdup(filename);
  log_configuration->log_level_stderr = LOG_LEVEL_DISABLED;
  log_configuration->log_level_console = LOG_LEVEL_DISABLED;
  log_configuration->log_level_syslog = LOG_LEVEL_DISABLED;
  log_configuration->log_level_file = LOG_LEVEL_MAX;
  log_configuration->syslog_facility = 0;
  log_configuration->log_rotation = 0;
  log_configuration->log_rotation_size = 0;
  log_configuration->buffer = 0;

  /* Configure the logging system with the values provided above. */
  if (LogConfigure(log_configuration) != 0) {
    DirectErrorMessage(HEX_STR_COULD_NOT_CONFIGURE_LOGGING);

    LogClose();

    return 1;
  }

  return 0;
}

// --------------------------------------------------------------------------
/* Close the logging system. */
void LogClose(void)
{
  /* Free the log file buffer. */
  buffer_free(&LogBuffer);

  /* If there's a logging configuration object floating around, free it. */
  if (LogConfiguration != NULL) {
    FreeLogConfiguration(LogConfiguration);
    LogConfiguration = NULL;
  }

  /* If the log file is open, close it. */
  if (Logfp != NULL) {
    fclose(Logfp);
    Logfp = NULL;
  }

  /* Close syslog. */
  CLOSELOG();
}

/*---- log.c ----------------------------------------------------------------*/
