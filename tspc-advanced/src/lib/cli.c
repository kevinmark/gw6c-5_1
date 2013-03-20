/*
---------------------------------------------------------------------------
 $Id: cli.c,v 1.28 2007/11/28 17:27:29 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2006 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>

#include "platform.h"

#include "config.h"
#include "cli.h"
#include "log.h"  // Verbose
#include "hex_strings.h"
#include "lib.h"
#include "errors.h"
#include "cnfchk.h"   // tspFixConfig

#ifdef WIN32
#include "console.h"
#include "service.h"
#endif

static
void flush_stdin(void)
{
  char x;

  while ( (x=(char)fgetc(stdin)) )
    if (x == '\n')
      break;
}


/* Ask the question,
   return 0 if answer is N or n,
   1 if answer is Y or y
*/

int ask(char *question, ...) {
  va_list ap;
  char *buf;
  int c;
  int ret;

#ifdef WIN32
  enable_console_input();
#endif

  if ( (buf = malloc(sizeof(char) * 1024)) == NULL ) {
    Display(LOG_LEVEL_3, ELError, "ask", HEX_STR_MALLOC_ERROR);
    return 0;
  }

  va_start(ap, question);
  vsnprintf(buf, 1024, question, ap);
  va_end(ap);

ask_again:

  printf("%s? (Y/N) ", buf);

  c = fgetc(stdin);

  /* empty stdin */
  flush_stdin();

  c = tolower(c);

  if ((char)c == 'y')
    ret = 1;
  else if ((char)c == 'n')
    ret = 0;
  else goto ask_again;

  free(buf);

#ifdef WIN32
  disable_console_input();
#endif

  return ret;
}

void PrintUsage(char *Message, ...) {

  if(Message) {
    va_list     argp;
    va_start(argp, Message);
    vprintf(Message, argp);
    va_end(argp);
  }

   printf("usage: gw6c [options] [-f config_file] [-r seconds]\n");
   printf("  where options are :\n");
   printf("    -i    gif interface to use for tunnel_v6v4\n");
   printf("    -u    interface to use for tunnel_v6udpv4\n");
   printf("    -s    interface to query to get IPv4 source address\n");
   printf("    -f    Read this config file instead of %s \n", FileName);
   printf("    -r    Retry after n seconds until success\n");
   printf("    -c    Verify and fix the config file (to migrate template names)\n");
#ifdef WIN32
   /* Not clean. Should be a hook
      to print platform specific
      options
   */
   printf("    --register    install to run as service\n");
   printf("    --unregister  uninstall the service\n");
#endif
   printf("    -h    help\n");
   printf("    -?    help\n");
   printf("\n");
   return;
}


int ParseArguments(int argc, char *argv[], tConf *Conf) {
    int ch;

#ifdef WIN32
    /* Not clean. Should be a hook
       to platform specific options
       parser
    */
    service_parse_cli(argc, argv);
#endif

    while ((ch = getopt(argc, argv, "h?cf:r:i:u:s:")) != -1) {
       switch (ch) {
       case 's':
         Conf->client_v4 = optarg;
         break;
       case 'i':
         Conf->if_tunnel_v6v4 = optarg;
         break;
       case 'u':
   Conf->if_tunnel_v6udpv4 = optarg;
   break;
       case 'f':
         FileName = optarg;
         break;
       case 'r':
   Conf->retry = atoi(optarg);
   break;
     case 'c':
     tspFixConfig();
  // The name of the error code below is confusing in this
  // case since we're not showing help. The desired behaviour
  // is the same, however: no error, but quit the application
  // without continuing.
  return NO_ERROR_SHOW_HELP;
       case '?':
       case 'h':
         PrintUsage(NULL);
         if ((optopt != '?') && (optopt != 'h') && (optopt != 0)) {
      return INVALID_ARGUMENTS;
         }
         else {
      return NO_ERROR_SHOW_HELP;
         }
       default:
         PrintUsage("Error while parsing command line arguments");
   return INVALID_ARGUMENTS;
       }

    }
    return NO_ERROR;
}









