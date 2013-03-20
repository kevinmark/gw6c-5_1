/*
---------------------------------------------------------------------------
 $Id: config.c,v 1.60 2007/11/28 17:27:29 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

/*  Configuration file handling. */
#define _USES_NETDB_H_
#define _USES_SYS_IOCTL_H_
#define _USES_SYS_SOCKET_H_
#define _USES_NETINET_IN_H_
#define _USES_NET_IF_H_
#define _USES_ARPA_INET_H_
#define _USES_SYSLOG_H_

#include "platform.h"
#include "config.h"
#include "tsp_client.h" // tspGetLocalAddress
#include "log.h"
#include "hex_strings.h"
#include "lib.h"
#include "errors.h"
#include "cli.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Gateway6 Client Configuration Subsystem */
#define TBOOLEAN_DECLARED
#include <gw6cconfig/gw6c_c_wrapper.h>
#include <gw6cconfig/gw6cuistrings.h>
#undef TBOOLEAN_DECLARED


static syslog_facility_t syslog_facilities[] = {
  { STR_CONFIG_SLOG_FACILITY_USER, LOG_USER },
  { STR_CONFIG_SLOG_FACILITY_LOCAL0, LOG_LOCAL0 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL1, LOG_LOCAL1 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL2, LOG_LOCAL2 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL3, LOG_LOCAL3 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL4, LOG_LOCAL4 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL5, LOG_LOCAL5 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL6, LOG_LOCAL6 },
  { STR_CONFIG_SLOG_FACILITY_LOCAL7, LOG_LOCAL7 },
  { NULL, 0 }
};


/* ----------------------------------------------------------------------- */
/* Function: ParseSyslogFacility                                           */
/*                                                                         */
/* Description:                                                            */
/*   Parse the configuration file's 'syslog_facility' directive.           */
/*                                                                         */
/* Arguments:                                                              */
/*   pConf: tConf* [OUT], The global configuration object.                 */
/*   facility: char* [IN], The input syslog facility.                      */
/*                                                                         */
/* Return Values:                                                          */
/*   1 on error.                                                           */
/*   0 on successful completion.                                           */
/*                                                                         */
/* ----------------------------------------------------------------------- */
static int ParseSyslogFacility( tConf *pConf, char *facility )
{
  int index = 0;

#ifdef WIN32
  return 0;
#endif

  /* Loop through the known facility strings, and compare with the one we found. */
  while( (syslog_facilities != NULL) && (syslog_facilities[index].string != NULL) )
  {
    if (strcmp(facility, syslog_facilities[index].string) == 0)
    {
      pConf->syslog_facility = syslog_facilities[index].value;
      return 0;
    }
    index++;
  }

  return 1;
}


/* ----------------------------------------------------------------------- */
/* Function: tspReadConfigFile                                             */
/*                                                                         */
/* Description:                                                            */
/*   Will extract the configuration data from the configuration file.      */
/*                                                                         */
/* Arguments:                                                              */
/*   szFile: char* [IN], The input configuration filename.                 */
/*   pConf: tConf* [OUT], The global configuration object.                 */
/*                                                                         */
/* Return Values:                                                          */
/*   INVALID_CONFIG_FILE on error.                                         */
/*   NO_ERROR on successful completion.                                    */
/*                                                                         */
/* ----------------------------------------------------------------------- */
int tspReadConfigFile( char* szFile, tConf* pConf )
{
  int i, nErrors, iRet;
  unsigned int* tErrors = NULL;
  char* szValue = NULL;


  /* Check input parameters. */
  if( szFile == NULL  ||  pConf == NULL )
  {
    return INVALID_CONFIG_FILE;
  }


  /* --------------------------------------------------------------------- */
  /* Read and load the configuration file.                                 */
  /* Will also perform thorough validation.                                */
  /* --------------------------------------------------------------------- */
  if( (iRet = initialize( szFile )) != 0 )
  {
    if( iRet == -1 )
    {
      /* Retrieve confguration error(s). */
      get_config_errors( &nErrors, &tErrors );

      for( i=0; i<nErrors; i++ )
        Display(LOG_LEVEL_1,
                ELError,
                "tspReadConfigFile",
                (char*)get_ui_string( tErrors[i] ) );
    }
    else
    {
      /* Initialization error */
      Display(LOG_LEVEL_1, ELError, "tspReadConfigFile", (char*)get_ui_string( iRet ) );
    }

    return INVALID_CONFIG_FILE;
  }


  /* --------------------------------------------------------------------- */
  /* Fill in the tConf structure from the file contents.                   */
  /* --------------------------------------------------------------------- */

  // Server is facultative in the gw6c-config validation routine, but not here.
  get_server( &(pConf->server) );
  if( strlen( pConf->server ) == 0 )
  {
    free( pConf->server );
    Display(LOG_LEVEL_1, ELError, "tspReadConfigFile",
            (char*)get_ui_string( GW6C_UIS__G6V_SERVERMUSTBESPEC ) );
    return INVALID_CONFIG_FILE;
  }

  get_gw6_dir( &(pConf->tsp_dir) );

  get_client_v4( &(pConf->client_v4) );

#ifdef V4V6_SUPPORT
  get_client_v6( &(pConf->client_v6) );
#endif /* V4V6_SUPPORT  */

  get_user_id( &(pConf->userid) );

  get_passwd( &(pConf->passwd) );

  get_auth_method( &(pConf->auth_method) );

  get_host_type( &(pConf->host_type) );

  get_template( &(pConf->template) );

  get_if_tun_v6v4( &(pConf->if_tunnel_v6v4) );

  get_if_tun_v6udpv4( &(pConf->if_tunnel_v6udpv4) );

#ifdef V4V6_SUPPORT
  get_if_tun_v4v6( &(pConf->if_tunnel_v4v6) );
#endif /* V4V6_SUPPORT */

  get_tunnel_mode( &szValue );

  if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V6ANYV4) == 0) {
    pConf->tunnel_mode = V6ANYV4;
  }
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V6V4) == 0) {
    pConf->tunnel_mode = V6V4;
  }
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V6UDPV4) == 0) {
    pConf->tunnel_mode = V6UDPV4;
  }
#ifdef V4V6_SUPPORT
  else if (strcmp(szValue, STR_CONFIG_TUNNELMODE_V4V6) == 0) {
    pConf->tunnel_mode = V4V6;
  }
#endif /* V4V6_SUPPORT */
  free( szValue );  szValue = NULL;

  get_dns_server( &(pConf->dns_server) );

  get_ifprefix( &(pConf->if_prefix) );

  get_prefixlen( &(pConf->prefixlen) );

  get_retry_delay( &(pConf->retry) );

  get_keepalive( &(pConf->keepalive) );

  get_keepalive_interval( &(pConf->keepalive_interval) );

  get_proxy_client( &(pConf->proxy_client) );

  get_syslog_facility( &szValue );
  ParseSyslogFacility( pConf, szValue );
  free( szValue );  szValue = NULL;

  get_log_filename( &(pConf->log_filename) );

  get_log_rotation( &(pConf->log_rotation) );

  get_log_rotation_sz( &(pConf->log_rotation_size) );

  get_log_rotation_del( &(pConf->log_rotation_delete) );

  get_log( STR_CONFIG_LOG_DESTINATION_STDERR, &(pConf->log_level_stderr) );

  get_log( STR_CONFIG_LOG_DESTINATION_SYSLOG, &(pConf->log_level_syslog) );

  get_log( STR_CONFIG_LOG_DESTINATION_CONSOLE, &(pConf->log_level_console) );

  get_log( STR_CONFIG_LOG_DESTINATION_FILE, &(pConf->log_level_file) );

  get_auto_retry_connect( &(pConf->auto_retry_connect) );

  get_last_server_file( &(pConf->last_server) );

  get_always_use_last_server( &(pConf->always_use_same_server) );

  get_broker_list_file( &(pConf->broker_list) );

  get_hap6_web_enabled( &(pConf->hap6_web_enabled) );

  get_hap6_proxy_enabled( &(pConf->hap6_proxy_enabled) );

  get_hap6_document_root( &(pConf->hap6_document_root) );

  /* Close the Gateway6 Client configuration object. */
  un_initialize();

  /* Successful completion. */
  return NO_ERROR;
}


/* ----------------------------------------------------------------------- */
/* Function: tspInitialize                                                 */
/*                                                                         */
/* Description:                                                            */
/*   Initialize with default values, read configuration file and override  */
/*   defaults with config file values.                                     */
/*                                                                         */
/* Arguments:                                                              */
/*   argc: char* [IN], Number of arguments passed on command line.         */
/*   argv: char*[] [IN], The command-line arguments.                       */
/*   pConf: tConf* [OUT], The global configuration object.                 */
/*                                                                         */
/* Return Values:                                                          */
/*   INVALID_CONFIG_FILE on error.                                         */
/*   NO_ERROR on successful completion.                                    */
/*                                                                         */
/* ----------------------------------------------------------------------- */
int tspInitialize(int argc, char *argv[], tConf *pConf)
{
  tConf CmdLine;
  int status = NO_ERROR;
  struct in_addr addr;
  const char* cszTemplDir = "template";
#ifdef V4V6_SUPPORT
  struct in6_addr addr6;
#endif

  // Hard-coded parameters. Not configurable anymore.
  pConf->syslog = FALSE;
  pConf->protocol = strdup( "default_route" );
  pConf->routing_info = strdup("");

  // This parameter may get overridden later on.
  pConf->transport = NET_TOOLS_T_RUDP;


  /* --------------------------------------------------------------------- */
  /* Read configuration data from the command-line arguments.              */
  /* --------------------------------------------------------------------- */
  memset(&CmdLine, 0, sizeof(CmdLine));
  if( argc > 1 )
  {
    if( (status = ParseArguments(argc, argv, &CmdLine)) != NO_ERROR )
    {
      if (status != NO_ERROR_SHOW_HELP)
      {
        Display(LOG_LEVEL_1, ELInfo, "tspInitialize", HEX_STR_ERR_PARSE_CLI_ARGS);
      }
      return status;
    }
  }

  /* --------------------------------------------------------------------- */
  /* Read configuration data from the file.                                */
  /* --------------------------------------------------------------------- */
  if( (status = tspReadConfigFile(FileName, pConf)) != NO_ERROR )
  {
          return status;
  }

  /* --------------------------------------------------------------------- */
  /* Override the config file with parameters from the command line.       */
  /* --------------------------------------------------------------------- */
  if(CmdLine.if_tunnel_v6v4)
    pConf->if_tunnel_v6v4 = CmdLine.if_tunnel_v6v4;

  if(CmdLine.if_tunnel_v6udpv4)
    pConf->if_tunnel_v6udpv4 = CmdLine.if_tunnel_v6udpv4;

#ifdef V4V6_SUPPORT
  if(CmdLine.if_tunnel_v4v6)
    pConf->if_tunnel_v4v6 = CmdLine.if_tunnel_v4v6;
#endif /* V4V6_SUPPORT  */

  if(CmdLine.client_v4)
    pConf->client_v4 = CmdLine.client_v4;


  /* --------------------------------------------------------------------- */
  /* Extrapolate directory in which template scripts are located.          */
  /* --------------------------------------------------------------------- */
  if( strlen(pConf->tsp_dir) != 0 )
  {
    TspHomeDir = pConf->tsp_dir;
    if( (ScriptDir = (char*)malloc( (size_t)(strlen(pConf->tsp_dir)+strlen(cszTemplDir)+2)) ) == NULL )
    {
      Display(LOG_LEVEL_3, ELError, "Initialise", HEX_STR_NOT_ENOUGH_MEM);
      return MEMORY_ERROR;
    }
    sprintf(ScriptDir, "%s%c%s", pConf->tsp_dir, DirSeparator, cszTemplDir);
  }
  else
  {
    if((ScriptDir = (char *)malloc((size_t)(strlen(TspHomeDir)+strlen(cszTemplDir)+2)))==NULL)
    {
      Display(LOG_LEVEL_3, ELError, "Initialise", HEX_STR_NOT_ENOUGH_MEM);
      return MEMORY_ERROR;
    }
    sprintf(ScriptDir, "%s%c%s", TspHomeDir, DirSeparator, cszTemplDir);
  }


  /* --------------------------------------------------------------------- */
  /* Find out what transport protocol to use, depending on Gateway6 server */
  /* specified in configuration data.                                      */
  /* --------------------------------------------------------------------- */
#ifdef V4V6_SUPPORT
  /* Reliable is the default transport protocol.
     For v4v6, the default protocol version is v6, fallback to v4. */
  if( pConf->tunnel_mode == V4V6 )
  {
    /* be sure config has a v6 server specified */
    if( NetText2Addr6(pConf->server, &addr6) != NULL )
      pConf->transport = NET_TOOLS_T_RUDP6;

    /* if no v6 server specified, fallback to v4 */
    else if( NetText2Addr(pConf->server, &addr) != NULL )
      pConf->transport = NET_TOOLS_T_RUDP;

    /* no valid server configured */
    else
    {
      Display(LOG_LEVEL_3, ELError, "Initialise", HEX_STR_NO_VALID_SERVER);
      return INVALID_SERVER;
    }
  }
#endif /* V4V6_SUPPORT */

  /* For v6v4, the default protocol version is v4, fallback to v6. */
  if( pConf->tunnel_mode == V6V4 )
  {
    /* be sure config has a v4 server specified */
    if( NetText2Addr(pConf->server, &addr) != NULL )
      pConf->transport = NET_TOOLS_T_RUDP;

#ifdef V4V6_SUPPORT
    /* if no v4 server specified, fallback to v6 */
    else if( NetText2Addr6(pConf->server, &addr6) != NULL )
      pConf->transport = NET_TOOLS_T_RUDP6;
#endif /* V4V6_SUPPORT */

    /* no valid server configured */
    else
    {
      Display(LOG_LEVEL_3, ELError, "Initialise", HEX_STR_NO_VALID_SERVER);
      return INVALID_SERVER;
    }
  }

  /* For v6udpv4, the protocol version MUST BE v4. */
  if( pConf->tunnel_mode == V6UDPV4 )
  {
    /* be sure config has v4 server specified */
    if( NetText2Addr(pConf->server, &addr) != NULL )
      pConf->transport = NET_TOOLS_T_RUDP;

    /* cannot fallback on v6udpv4 */
    else
    {
      Display(LOG_LEVEL_3, ELError, "Initialise", HEX_STR_MODE_V6UDPV4_MUST_CONN_V4);
      return INVALID_SERVER;
    }
  }


  return NO_ERROR;
}


/* ----------------------------------------------------------------------- */
/* Function: tspUpdateSourceAddr                                           */
/*                                                                         */
/* Description:                                                            */
/*   Validates IPv4 address, or validates and extracts the IPv6 address.   */
/*                                                                         */
/* Arguments:                                                              */
/*   pConf: tConf* [OUT], The global configuration object.                 */
/*                                                                         */
/* Return Values:                                                          */
/*   INVALID_CLIENT_IPV4 on error.                                         */
/*   INVALID_CLIENT_IPV6 on error.                                         */
/*   NO_ERROR on successful completion.                                    */
/*                                                                         */
/* ----------------------------------------------------------------------- */
int tspUpdateSourceAddr( tConf *pConf, SOCKET fd )
{

  if( pConf->tunnel_mode != V4V6 )
  {
    /* tunnel mode v6*v4, we need source IPv4 address */
    if(IsPresent(pConf->client_v4))
    {
      if(!strcmp(pConf->client_v4, "auto"))
      {
        char addr_str[INET_ADDRSTRLEN+1];

        /* if current transport is v4, get source address of tsp session. If not, get host address. */
        if (((pConf->transport == NET_TOOLS_T_RUDP) || (pConf->transport == NET_TOOLS_T_TCP)) &&
           (tspGetLocalAddress(fd, addr_str, INET_ADDRSTRLEN) != NULL))
        {
          pConf->client_v4 = strdup(addr_str);
          Display(LOG_LEVEL_3, ELInfo, "tspUpdateSourceAddr", HEX_STR_USING_AS_SOURCE_IPV4, pConf->client_v4);
        }
        else
        {
          Display(LOG_LEVEL_1, ELError, "tspUpdateSourceAddr", HEX_STR_CANT_AUTO_CLIENT_IPV4_TO_V6);
          return INVALID_CLIENT_IPV4;
        }
      }
    }
  }

#ifdef V4V6_SUPPORT
  if( pConf->tunnel_mode == V4V6 )
  {
    /* tunnel mode v4v6, we need source IPv6 address. */
    if(IsPresent(pConf->client_v6))
    {
      if(!strcmp(pConf->client_v6, "auto"))
      {
        char addr_str[INET6_ADDRSTRLEN+1];

        /* if current transport is v6, get source address of tsp session. If not, get host address. */
        if (((pConf->transport == NET_TOOLS_T_RUDP6) || (pConf->transport == NET_TOOLS_T_TCP6)) &&
            (tspGetLocalAddress(fd, addr_str, INET6_ADDRSTRLEN) != NULL))
        {
          pConf->client_v6 = strdup(addr_str);
          Display(LOG_LEVEL_3, ELInfo, "tspUpdateSourceAddr", HEX_STR_USING_AS_SOURCE_IPV6, pConf->client_v6);
        }
        else
        {
          Display(LOG_LEVEL_1, ELError, "tspUpdateSourceAddr", HEX_STR_CANT_AUTO_CLIENT_IPV6_TO_V4);
          return INVALID_CLIENT_IPV6;
        }
      }
      else
      {
        /* remove brackets if any */
        char addr_str[INET6_ADDRSTRLEN+1];
        char *ptr;

        memset(addr_str, 0, sizeof(addr_str));
        strcpy(addr_str, pConf->client_v6);

        if ((ptr = strtok(addr_str, "[")) != NULL)
        {
          if ((ptr = strtok(ptr, "]")) != NULL)
          {
            /* copy back address */
            strcpy(pConf->client_v6, ptr);
          }
        }
      }
    }
  }
#endif /* V4V6_SUPPORT */

  return NO_ERROR;
}
