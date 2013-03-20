/*
---------------------------------------------------------------------------
 $Id: log.h,v 1.10 2007/11/28 17:27:07 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

#ifdef LOG_IT
# define ACCESS
#else
# define ACCESS extern
#endif

#define LOG_LEVEL_DISABLED  0
#define LOG_LEVEL_1     1
#define LOG_LEVEL_2     2
#define LOG_LEVEL_3     3

#define LOG_LEVEL_MIN   LOG_LEVEL_DISABLED
#define LOG_LEVEL_MAX   LOG_LEVEL_3

#define LOG_IDENTITY_MAX_LENGTH 32
#define LOG_FILENAME_MAX_LENGTH 255

#define MAX_LOG_LINE_LENGTH 4096

#define LOG_IDENTITY "gw6c"

#define DEFAULT_LOG_FILENAME "gw6c.log"

#define DEFAULT_LOG_ROTATION_SIZE 32

enum tSeverityLevel {
  ELError,
  ELWarning,
  ELInfo,
  ELDebug
};

typedef struct stLogConfiguration {
  char * identity;
  char * log_filename;
  int log_level_stderr;
  int log_level_console;
  int log_level_syslog;
  int log_level_file;
  int syslog_facility;
  int log_rotation_size;
  int log_rotation;
  int buffer;
  int delete_rotated_log;       // 0 = FALSE
} tLogConfiguration;

ACCESS int  DirectErrorMessage(char *message, ...);

ACCESS void Display(int, enum tSeverityLevel, const char *, char *, ...);

ACCESS int  LogConfigure(tLogConfiguration *);
ACCESS void LogClose(void);
ACCESS int DumpBufferToFile(char *filename);

#undef ACCESS
#endif

/*----- log.h ----------------------------------------------------------------------------------*/
