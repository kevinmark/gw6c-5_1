/*
---------------------------------------------------------------------------
 $Id: tsp_setup.c,v 1.32 2007/12/04 17:16:48 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>

#define _USES_SYS_SOCKET_H_
#include "platform.h"

#include "tsp_setup.h"
#include "tsp_client.h"

#include "config.h"       // tConf
#include "xml_tun.h"      // tTunnel
#include "log.h"          // Display()
#include "hex_strings.h"  // Strings for Display()
#include "lib.h"          // IsAll, IPv4Addr, IPv6Addr, IPAddrAny, Numeric.
#include "errors.h"       // Error codes

// Gateway6 Client Messaging Subsystem.
#include <gw6cmessaging/gw6c_c_wrapper.h>

#define TSP_OPERATION_CREATETUNNEL    "TSP_TUNNEL_CREATION"
#define TSP_OPERATION_TEARDOWNTUNNEL  "TSP_TUNNEL_TEARDOWN"


/*  Should be defined in platform.h  */
#ifndef SCRIPT_TMP_FILE
#error "SCRIPT_TMP_FILE is not defined in platform.h"
#endif


/* Execute cmd and send output to log subsystem */
int execScript(const char *cmd)
{
  char buf[1024];
  FILE* f_log;
  int retVal;

  memset( buf, 0, sizeof(buf) );
  snprintf( buf, sizeof(buf), "%s > %s", cmd, SCRIPT_TMP_FILE );
  retVal = system(buf);

  // Open resulting output file.
  f_log = fopen( SCRIPT_TMP_FILE, "r" );
  if( f_log == NULL )
  {
    Display( LOG_LEVEL_3, ELError, "execScript", HEX_STR_CANT_OPEN_TMP_FILE SCRIPT_TMP_FILE );
    return -1;
  }

  // Loop on the output file, and log the contents.
  while( !feof( f_log ) )
  {
    if( fgets( buf, sizeof(buf), f_log ) != NULL )
    {
      Display( LOG_LEVEL_MAX, ELInfo, "execScript", "%s", buf );
    }
  }

  // Close ouput file and delete.
  fclose( f_log );
  unlink( SCRIPT_TMP_FILE );

  return retVal;
}


// --------------------------------------------------------------------------
// This function validates the information found in the tunnel information
// structure.
// Returns number of errors found. 0 is successful validation.
//
int validate_tunnel_info( const tTunnel* pTunnelInfo )
{
  int err_num = 0;


  if( !IsAll(IPv4Addr, pTunnelInfo->client_address_ipv4) )
  {
    Display(LOG_LEVEL_3, ELError, "validate_tunnel_info", HEX_STR_BAD_CLIENT_IPV4_RECVD);
    err_num++;
  }

  if( !IsAll(IPv6Addr, pTunnelInfo->client_address_ipv6) )
  {
    Display(LOG_LEVEL_3, ELError, "validate_tunnel_info", HEX_STR_BAD_CLIENT_IPV6_RECVD);
    err_num++;
  }

  if( !IsAll(IPv4Addr, pTunnelInfo->server_address_ipv4) )
  {
    Display(LOG_LEVEL_3, ELError, "validate_tunnel_info", HEX_STR_BAD_SERVER_IPV4_RECVD);
    err_num++;
  }

  if( !IsAll(IPv6Addr, pTunnelInfo->server_address_ipv6) )
  {
    Display(LOG_LEVEL_3, ELError, "validate_tunnel_info", HEX_STR_BAD_SERVER_IPV6_RECVD);
    err_num++;
  }

  // If prefix information is found, validate it.
  if( pTunnelInfo->prefix != NULL )
  {
    if( !IsAll(IPAddrAny, pTunnelInfo->prefix) )
    {
      Display(LOG_LEVEL_3, ELError, "validate_tunnel_info", HEX_STR_BAD_SERVER_PREFIX_RECVD);
      err_num++;
    }

    if( !IsAll(Numeric, pTunnelInfo->prefix_length) )
    {
      Display(LOG_LEVEL_3, ELError, "validate_tunnel_info", HEX_STR_BAD_PREFIX_LEN_RECVD);
      err_num++;
    }
  }

  return err_num;
}


// --------------------------------------------------------------------------

void set_tsp_env_variables( const tConf* pConfig, const tTunnel* pTunnelInfo )
{
  char buffer[8];

  // Specify log verbosity (MAXIMAL).
  snprintf( buffer, sizeof buffer, "%d", LOG_LEVEL_MAX );
  tspSetEnv("TSP_VERBOSE", buffer, 1);

  // Specify Gateway6 Client installation directory.
  tspSetEnv("TSP_HOME_DIR", TspHomeDir, 1);

  // Specify the tunnel mode.
  tspSetEnv("TSP_TUNNEL_MODE", pTunnelInfo->type, 1);

  // Specify host type {router, host}
  tspSetEnv("TSP_HOST_TYPE", pConfig->host_type, 1);

  // Specify tunnel interface, for setup.
  if (strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6V4) == 0 )
  {
    tspSetEnv("TSP_TUNNEL_INTERFACE", pConfig->if_tunnel_v6v4, 1);
    gTunnelInfo.eTunnelType = TUNTYPE_V6V4;
  }
  else if (strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6UDPV4) == 0 )
  {
    tspSetEnv("TSP_TUNNEL_INTERFACE", pConfig->if_tunnel_v6udpv4, 1);
    gTunnelInfo.eTunnelType = TUNTYPE_V6UDPV4;
  }
#ifdef V4V6_SUPPORT
  else if (strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V4V6) == 0 )
  {
    tspSetEnv("TSP_TUNNEL_INTERFACE", pConfig->if_tunnel_v4v6, 1);
    gTunnelInfo.eTunnelType = TUNTYPE_V4V6;
  }
#endif /* V4V6_SUPPORT */

  // Specify what interface will be used for routing advertizement,
  // if enabled.
  tspSetEnv("TSP_HOME_INTERFACE", pConfig->if_prefix, 1);

  // Specify local endpoint IPv4 address
  tspSetEnv("TSP_CLIENT_ADDRESS_IPV4", pTunnelInfo->client_address_ipv4, 1);
  gTunnelInfo.szIPV4AddrLocalEndpoint = pTunnelInfo->client_address_ipv4;

  // Specify local endpoint IPv6 address
  tspSetEnv("TSP_CLIENT_ADDRESS_IPV6", pTunnelInfo->client_address_ipv6, 1);
  gTunnelInfo.szIPV6AddrLocalEndpoint = pTunnelInfo->client_address_ipv6;

  // Specify local endpoint domain name
  if( pTunnelInfo->client_dns_name != NULL)
  {
    tspSetEnv("TSP_CLIENT_DNS_NAME", pTunnelInfo->client_dns_name, 1);
    gTunnelInfo.szUserDomain = pTunnelInfo->client_dns_name;
  }

  // Specify remote endpoint IPv4 address.
  tspSetEnv("TSP_SERVER_ADDRESS_IPV4", pTunnelInfo->server_address_ipv4, 1);
  gTunnelInfo.szIPV4AddrRemoteEndpoint = pTunnelInfo->server_address_ipv4;

  // Specify remote endpoint IPv6 address.
  tspSetEnv("TSP_SERVER_ADDRESS_IPV6", pTunnelInfo->server_address_ipv6, 1);
  gTunnelInfo.szIPV6AddrRemoteEndpoint = pTunnelInfo->server_address_ipv6;

  // Specify prefix for tunnel endpoint.
  if ((strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6V4) == 0) ||
      (strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6UDPV4) == 0))
    tspSetEnv("TSP_TUNNEL_PREFIXLEN", "128", 1);
#ifdef V4V6_SUPPORT
  else
    tspSetEnv("TSP_TUNNEL_PREFIXLEN", "32", 1);
#endif /* V4V6_SUPPORT */


  // Free and clear delegated prefix from tunnel info.
  if( gTunnelInfo.szDelegatedPrefix != NULL )
  {
    free( gTunnelInfo.szDelegatedPrefix );
    gTunnelInfo.szDelegatedPrefix = NULL;
  }

  // Have we been allocated a prefix for routing advertizement..?
  if( pTunnelInfo->prefix != NULL )
  {
    char chPrefix[128];
    size_t len, sep;

    /* Compute the number of characters that are significant out of the prefix. */
    /* This is meaningful only for IPv6 prefixes; no contraction is possible for IPv4. */
    if ((strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6V4) == 0) ||
        (strcasecmp(pTunnelInfo->type, STR_XML_TUNNELMODE_V6UDPV4) == 0))
    {
      len = (atoi(pTunnelInfo->prefix_length) % 16) ? (atoi(pTunnelInfo->prefix_length) / 16 + 1) * 4 : atoi(pTunnelInfo->prefix_length) / 16 * 4;
      sep = (atoi(pTunnelInfo->prefix_length) % 16) ? (atoi(pTunnelInfo->prefix_length) / 16) : (atoi(pTunnelInfo->prefix_length) / 16) -1;
    }
    else
    {
      len = strlen( pTunnelInfo->prefix );
      sep = 0;
    }

    memset(chPrefix, 0, 128);
    memcpy(chPrefix, pTunnelInfo->prefix, len+sep);

    // Specify delegated prefix for routing advertizement, if enabled.
    tspSetEnv("TSP_PREFIX", chPrefix, 1);
    gTunnelInfo.szDelegatedPrefix = (char*) malloc( strlen(chPrefix) + 10/*To append prefix_length*/ );
    strcpy( gTunnelInfo.szDelegatedPrefix, chPrefix );

    // Specify prefix length for routing advertizement, if enabled.
    tspSetEnv("TSP_PREFIXLEN", pTunnelInfo->prefix_length, 1);
    strcat( gTunnelInfo.szDelegatedPrefix, "/" );
    strcat( gTunnelInfo.szDelegatedPrefix, pTunnelInfo->prefix_length );
  }
}


// --------------------------------------------------------------------------
// Builds the template script execution path and returns the string.
//
// Returns NULL if error is detected and template execution path cannot be
// built.
//
// NOTE: This function is NOT thread safe. Callee should not retain
//       pointer returned from this function.
//
static char* get_template_script( const tConf* pConfig )
{
  static char buffer[1024] = { 0x00 };
  FILE* f_test;

  // If first run, get the required information and build template
  // script execution path.
  //
  if( buffer[0] == 0x00 )
  {
    snprintf( buffer, sizeof buffer, "%s%c%s.%s", ScriptDir, DirSeparator, pConfig->template, ScriptExtension);

    f_test = fopen( buffer, "r" );
    if( f_test == NULL )
    {
      Display( LOG_LEVEL_1, ELError, "tspSetupInterface", HEX_STR_TEMPLATE_NOT_FOUND, buffer );
      return NULL;
    }

    // Close the just opened file.
    fclose( f_test );
    memset( buffer, 0, sizeof buffer );

    // Append script interpretor to buffer, if any.
    if( ScriptInterpretor != NULL )
    {
      snprintf( buffer, sizeof buffer, 
               "%s \"%s%c%s.%s\"", 
               ScriptInterpretor, ScriptDir, DirSeparator, pConfig->template, ScriptExtension);
    }
    else
    {
      snprintf( buffer, sizeof buffer, 
                "\"%s%c%s.%s\"",  
                ScriptDir, DirSeparator, pConfig->template, ScriptExtension);
    }
  }

  return buffer;
}


// --------------------------------------------------------------------------
// This function will set the required environment variables that will later
// be used when invoking the template script to actually create the tunnel.
//
int tspSetupInterface(tConf *c, tTunnel *t)
{
  int ret;
  char* template_script;


  // Perform validation on tunnel information provided by server.
  if( validate_tunnel_info(t) != 0 )
  {
    // Errors occured during verification of tunnel parameters.
    Display( LOG_LEVEL_1,ELError, "tspSetupInterface", HEX_STR_ERRS_IN_SERVER_RESP );
    return 1;
  }


  // Specify TSP Operation: Tunnel Creation.
  tspSetEnv("TSP_OPERATION", TSP_OPERATION_CREATETUNNEL, 1 );

  // Set environment variable for script execution.
  set_tsp_env_variables( c, t );


  // Do some platform-specific stuff before tunnel setup script is launched.
  // The "tspSetupInterfaceLocal" is defined in tsp_local.c in every platform.
  if( tspSetupInterfaceLocal( c, t ) != 0 )
  {
    // Errors occured during verification of tunnel parameters.
    Display( LOG_LEVEL_1,ELError, "tspSetupInterface", HEX_STR_ERRS_IN_SERVER_RESP );
    return 1;
  }


  // Get template script command string.
  template_script = get_template_script( c );
  if( template_script == NULL )
  {
    return 1;
  }


  // ------------------------------------------------
  // Run the template script to bring the tunnel up.
  // ------------------------------------------------
  Display( LOG_LEVEL_2, ELInfo, "tspSetupInterface", HEX_STR_EXEC_CFG_SCRIPT, template_script );

  ret = execScript( template_script );
  if( ret == 0 )
  {
    Display(LOG_LEVEL_2, ELInfo, "tspSetupInterface", HEX_STR_SCRIPT_COMPLETED_OK);

    if ((strcasecmp(t->type, STR_XML_TUNNELMODE_V6V4) == 0) ||
          (strcasecmp(t->type, STR_XML_TUNNELMODE_V6UDPV4) == 0))
    {
      Display(LOG_LEVEL_1, ELInfo, "tspSetupInterface", HEX_STR_YOUR_IPV6_IP_IS, t->client_address_ipv6);
    }
#ifdef V4V6_SUPPORT
    else
    {
      Display(LOG_LEVEL_1, ELInfo, "tspSetupInterface", HEX_STR_YOUR_IPV4_IP_IS, t->client_address_ipv4);
    }
#endif /* V4V6_SUPPORT */

    if( (t->prefix != NULL) && (t->prefix_length != NULL) )
    {
      if ((strcasecmp(t->type, STR_XML_TUNNELMODE_V6V4) == 0) ||
        (strcasecmp(t->type, STR_XML_TUNNELMODE_V6UDPV4) == 0))
      {
        Display(LOG_LEVEL_1, ELInfo, "tspSetupInterface", HEX_STR_YOUR_IPV6_PREFIX_IS, t->prefix, t->prefix_length);
      }
#ifdef V4V6_SUPPORT
      else
      {
        Display(LOG_LEVEL_1, ELInfo, "tspSetupInterface", HEX_STR_YOUR_IPV4_PREFIX_IS, t->prefix, t->prefix_length);
      }
#endif /* V4V6_SUPPORT */
    }

    if (t->type != NULL)
    {
      Display(LOG_LEVEL_2, ELInfo, "tspSetupInterface", HEX_STR_SETUP_TUNNEL_TYPE, t->type);
    }

    Display(LOG_LEVEL_3, ELInfo, "tspSetupInterface", HEX_STR_SETUP_PROXY, c->proxy_client == TRUE ? HEX_STR_ENABLED : HEX_STR_DISABLED);

    if (c->host_type != NULL)
    {
      Display(LOG_LEVEL_3, ELInfo, "tspSetupInterface", HEX_STR_SETUP_HOST_TYPE, c->host_type);
    }

    // Set the broker used for connection.
    gTunnelInfo.szBrokerName = c->server;

    // Set the current time(now) for tunnel start.
    gTunnelInfo.tunnelUpTime = time(NULL);

    // Send the tunnel info through the messaging subsystem.
    send_tunnel_info();
  }

  return ret;
}


// --------------------------------------------------------------------------
// This function will set the required environment variables that will later
// be used when invoking the template script to tear down the existing 
// tunnel.
//
int tspTearDownTunnel( tConf* pConf, tTunnel* pTunInfo )
{
  char* scriptName;
  int retCode = NO_ERROR;


  // Specify TSP Operation: Tunnel Teardown.
  tspSetEnv( "TSP_OPERATION", TSP_OPERATION_TEARDOWNTUNNEL, 1 );

  // Set environment variables (They may be not set).
  set_tsp_env_variables( pConf, pTunInfo );


  // Format path to script.
  scriptName = get_template_script( pConf );
  if( scriptName == NULL )
  {
    return 1;
  }


  // -------------------------------------------------
  // Run the template script to tear the tunnel down.
  // -------------------------------------------------
  Display(LOG_LEVEL_2, ELInfo, "tspTearDownTunnel", HEX_STR_EXEC_CFG_SCRIPT, scriptName );

  retCode = execScript( scriptName );
  if( retCode == 0 )
  {
    Display(LOG_LEVEL_2, ELInfo, "tspTearDownTunnel", HEX_STR_SCRIPT_COMPLETED_OK );
  }
  else
  {
    // !! ERROR !!
    Display(LOG_LEVEL_2, ELError, "tspTearDownTunnel", HEX_STR_SCRIPT_FAILED );
  }


  // Return script execution return code.
  return retCode;
}

