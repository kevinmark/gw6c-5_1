/*
---------------------------------------------------------------------------
 $Id: tsp_local.c,v 1.14 2007/11/28 17:27:10 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

/* DARWIN */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#define _USES_SYS_SOCKET_H_
#define _USES_ARPA_INET_H_

#include "platform.h"

#include "config.h"       /* tConf */
#include "xml_tun.h"      /* tTunnel */
#include "net.h"          /* net_tools_t */

#include "log.h"          /* Display */
#include "hex_strings.h"  /* Display strings */
#include "errors.h"       /* Error codes */

#include "tsp_net.h"      /* tspClose */
#include "tsp_setup.h"    /* tspSetupInterface */
#include "tsp_tun.h"      /* TunInit, TunMainLoop */
#include "tsp_tun_mgt.h"  /* tspPerformTunnelLoop */

/* these globals are defined by US used by alot of things in  */
#define IFNAMSIZ 16     // from net/if.h

char *FileName  = "gw6c.conf";
char *ScriptInterpretor = "/bin/sh";
char *ScriptExtension = "sh";
char *ScriptDir = NULL;
char *TspHomeDir = "/usr/local/etc/gw6";
char DirSeparator = '/';

int RootUid = 0;
int indSigHUP = 0;    // Set to 1 when HUP signal is trapped.


#include <gw6cmessaging/gw6cuistrings.h>
// Dummy implementation for non-win32 targets
// (Library gw6cmessaging is not linked in non-win32 targets).
error_t send_status_info( void ) { return GW6CM_UIS__NOERROR; }
error_t send_tunnel_info( void ) { return GW6CM_UIS__NOERROR; }
error_t send_broker_list( void ) { return GW6CM_UIS__NOERROR; }
error_t send_hap6_status_info( void ) { return GW6CM_UIS__NOERROR; }


// --------------------------------------------------------------------------
// Set an environment variable in the current process.
//
void tspSetEnv(char *Variable, char *Value, int Flag)
{
  Display(LOG_LEVEL_3, ELInfo, "tspSetEnv", HEX_STR_ENV_PRINT_VALUE, Variable, Value);
  setenv(Variable, Value, Flag);
}

// --------------------------------------------------------------------------
// Checks if the Gateway6 Client has been requested to stop and exit.
//
// Returns 1 if Gateway6 Client is being requested to stop and exit.
// Else, waits 'uiWaitMs' miliseconds and returns 0.
//
// Defined in tsp_client.h
//
int tspCheckForStopOrWait( const unsigned int uiWaitMs )
{
  // Sleep for the amount of time specified, if signal has not been sent.
  if( indSigHUP == 0 )
  {
    // usleep is expecting microseconds (1 microsecond = 0.000001 second).
    usleep( uiWaitMs * 1000 );
  }

  return indSigHUP;
}


// --------------------------------------------------------------------------
// Called from tsp_setup.c -> tspSetupInterface
//   Do extra platform-specific stuff before tunnel script is launched.
//
int tspSetupInterfaceLocal( tConf* pConf, tTunnel* pTun )
{
  return 0;
}


// --------------------------------------------------------------------------
// Returns local address.
// tspSetupTunnel() will callback here
//
char* tspGetLocalAddress(int socket, char *buffer, int size)
{
  struct sockaddr_in6 addr; /* enough place for v4 and v6 */
  struct sockaddr_in  *addr_v4 = (struct sockaddr_in *)&addr;
  struct sockaddr_in6 *addr_v6 = (struct sockaddr_in6 *)&addr;
  socklen_t len;

  len = sizeof addr;
  if (getsockname(socket, (struct sockaddr *)&addr, &len) < 0) {
    Display(LOG_LEVEL_3, ELError, "TryServer", HEX_STR_ERR_FIND_SRC_IP);
    return NULL;
  }

  if (addr.sin6_family == AF_INET6)
    return (char *)inet_ntop(AF_INET6, (const void*) &addr_v6->sin6_addr, buffer, size);
  else
    return (char *)inet_ntop(AF_INET, (const void*) &addr_v4->sin_addr, buffer, size);
}


// --------------------------------------------------------------------------
// setup interface and any daemons or anything needed
// tspSetupTunnel() will callback here
//
int tspStartLocal( int socket, tConf *c, tTunnel *t, net_tools_t *nt )
{
  TUNNEL_LOOP_CONFIG tun_loop_cfg;
  int status = NO_ERROR;
  int keepalive_interval = 0;

  /* Test for root privileges */
  if( geteuid() != 0 )
  {
    Display( LOG_LEVEL_1, ELError, "tspStartLocal", HEX_STR_FATAL_NOT_ROOT_FOR_TUN );
    return INTERFACE_SETUP_FAILED;
  }


  if( t->keepalive_interval != NULL )
  {
    keepalive_interval = atoi(t->keepalive_interval);
    Display( LOG_LEVEL_3, ELInfo, "tspStartLocal", HEX_STR_KEEPALIVE_INTERVAL, t->keepalive_interval );
  }

  {
    int tunfd = (-1);

    Display( LOG_LEVEL_3, ELInfo, "tspStartLocal", HEX_STR_GOING_DAEMON );
    if (daemon(1, 0) == -1)
    {
      Display( LOG_LEVEL_3, ELError, "tspStartLocal", HEX_STR_CANT_FORK );
      return INTERFACE_SETUP_FAILED;
    }

    if( strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V6UDPV4) == 0 )
    {
      tunfd = TunInit(c->if_tunnel_v6udpv4);
      if( tunfd == -1 )
      {
        Display( LOG_LEVEL_1, ELError, "tspStartLocal", HEX_STR_UNABLE_INIT_TUN_DEV );
        return(INTERFACE_SETUP_FAILED);
      }

      /* We need the real name of the tun device for the template */
      free( c->if_tunnel_v6udpv4 );
      c->if_tunnel_v6udpv4 = (char*) malloc( IFNAMSIZ );
      TunName(tunfd, c->if_tunnel_v6udpv4, IFNAMSIZ );
    }

    if( strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V4V6) == 0 )
    {
      Display( LOG_LEVEL_1, ELError, "tspStartLocal", HEX_STR_NO_V4V6_ON_PLATFORM );
      return(INTERFACE_SETUP_FAILED);
    }

    /* Run the config script without giving it our tunnel file descriptor.
    //
    // This is important because otherwise the tunnnel will stay open even
    // if we get killed
    */
    {
      int pid = fork();
      if( pid < 0 )
      {
        // fork() error
        return INTERFACE_SETUP_FAILED;
      }
      else if( pid == 0 )
      {
        // Child processing: run template script.
        close(tunfd);

        if( tspSetupInterface(c, t) != 0 )
          exit(INTERFACE_SETUP_FAILED);
        exit(0);
      }
      else
      {
        // Parent processing
        int s = 0;

        // Wait for child process to exit.
        Display( LOG_LEVEL_3, ELInfo, "tspStartLocal", HEX_STR_WAITING_FOR_SETUP_SCRIPT );
        if (wait(&s) == pid)
        {
          // ok our child returned
          if( !WIFEXITED(s) )
          {
            Display( LOG_LEVEL_3, ELError, "tspStartLocal", HEX_STR_SCRIPT_FAILED );
            return INTERFACE_SETUP_FAILED;
          }

          // Check child exit code.
          if( WEXITSTATUS(s) != 0 )
          {
            Display( LOG_LEVEL_3, ELError, "tspStartLocal", HEX_STR_SCRIPT_FAILED );
            return INTERFACE_SETUP_FAILED;
          }
          // else everything is fine
        }
        else
        {
          // Error occured: we have no other child
          Display( LOG_LEVEL_1, ELError, "tspStartLocal", HEX_STR_ERR_WAITING_SCRIPT );
          return INTERFACE_SETUP_FAILED;
        }
      }
    }


    if( strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V6UDPV4) == 0 )
    {
      status = TunMainLoop( tunfd, socket, c->keepalive,
                            keepalive_interval, t->client_address_ipv6,
                            t->keepalive_address );

      /* We got out of main V6 UDP V4 loop */
      close(tunfd);
      tspClose(socket, nt);
    }
    else if( strcasecmp(t->type, STR_CONFIG_TUNNELMODE_V6V4) == 0 )
    {
      memset( &tun_loop_cfg, 0x00, sizeof(TUNNEL_LOOP_CONFIG) );
      tun_loop_cfg.ka_interval  = keepalive_interval;
      tun_loop_cfg.ka_src_addr  = t->client_address_ipv6;
      tun_loop_cfg.ka_dst_addr  = t->keepalive_address;
      tun_loop_cfg.sa_family    = AF_INET6;
      tun_loop_cfg.tun_lifetime = 0;

      status = tspPerformTunnelLoop( &tun_loop_cfg );
    }
  }

  // Handle tunnel teardown.
  if( tspTearDownTunnel( c, t ) != 0 )
  {
    // Log the error.
    Display( LOG_LEVEL_2, ELError, "tspStartLocal", HEX_STR_SCRIPT_FAILED );
  }


  return status;
}
