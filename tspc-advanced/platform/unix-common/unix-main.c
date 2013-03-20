/*
---------------------------------------------------------------------------
 $Id: unix-main.c,v 1.3 2007/11/28 17:27:23 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2006 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#define _USES_SYS_SOCKET_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "platform.h"
#include "tsp_client.h"
#include "hex_strings.h"
#include "errors.h"
#include "log.h"
#include "os_uname.h"


extern int indSigHUP; /* Declared in every unix platform tsp_local.c */


/* --------------------------------------------------------------------------
// Signal handler function. KEEP THIS FUNCTION AS SIMPLE AS POSSIBLE.
*/
void signal_handler( int sigraised )
{
  if( sigraised == SIGHUP )
    indSigHUP = 1;
}


// --------------------------------------------------------------------------
// Retrieves OS information and puts it nicely in a string ready for display.
//
// Defined in tsp_client.h
//
void tspGetOSInfo( const size_t len, char* buf )
{
  if( len > 0  &&  buf != NULL )
  {
#ifdef OS_UNAME_INFO
    snprintf( buf, len, "Built on ///%s///", OS_UNAME_INFO );
#else
    snprintf( buf, len, "Built on ///unknown UNIX/BSD/Linux version///" );
#endif
  }
}


// --------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  tLogConfiguration *log_configuration;

  /* Allocate memory for the logging configuration structure */
  log_configuration = (tLogConfiguration *)malloc(sizeof(tLogConfiguration));

  /* Validate that memory could be allocated correctly */
  if (log_configuration == NULL) {
    DirectErrorMessage(HEX_STR_COULD_NOT_MALLOC_FOR_CONFIG);

    LogClose();

    return MEMORY_ERROR;
  }

  /* Fill the logging configuration structure with the defaults to apply */
  /* until the configuration file has been parsed and validated. */
  /* Before this has been done, we do not know what logging configuration */
  /* the user wants. */
  log_configuration->identity = strdup(LOG_IDENTITY);
  log_configuration->log_filename = NULL;
  log_configuration->log_level_stderr = DEFAULT_PRECFG_LOG_LEVEL_STDERR;
  log_configuration->log_level_console = DEFAULT_PRECFG_LOG_LEVEL_CONSOLE;
  log_configuration->log_level_syslog = DEFAULT_PRECFG_LOG_LEVEL_SYSLOG;
  log_configuration->log_level_file = DEFAULT_PRECFG_LOG_LEVEL_FILE;
  log_configuration->syslog_facility = 0;
  log_configuration->log_rotation = 0;
  log_configuration->log_rotation_size = 0;
  log_configuration->buffer = 1;

  /* Configure the logging system with the values provided above. */
  if (LogConfigure(log_configuration) != 0) {
    DirectErrorMessage(HEX_STR_COULD_NOT_CONFIGURE_LOGGING);

    LogClose();

    return LOGGING_CONFIGURATION_ERROR;
  }

  /* Install new signal handler for HUP signal. */
  signal( SIGHUP, &signal_handler );

  /* entry point */
  return tspMain(argc, argv);

}

