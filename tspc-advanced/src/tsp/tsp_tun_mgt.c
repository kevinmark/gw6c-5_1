/*
-----------------------------------------------------------------------------
 $Id: tsp_tun_mgt.c,v 1.2 2007/11/28 17:27:36 cnepveu Exp $
-----------------------------------------------------------------------------
Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
-----------------------------------------------------------------------------
*/

// Tunnel Management functions

#define _USES_SYS_SOCKET_H_
#include "platform.h"

#include "tsp_tun_mgt.h"
#include "tsp_client.h"       // tspCheckForStopOrWait().
#include "net_ka.h"           // Keep-alive functionality.
#include "tsp_lease.h"        // Tunnel lifetime functions.
#include "errors.h"           // Error codes.


#define LOOP_WAIT_MS  100


// --------------------------------------------------------------------------
// Function: tspPerformTunnelLoop
//
// Performs the 3 following actions:
//   1. Handles tunnel keep-alive functionnality (if configured).
//   2. Checks tunnel lifetime expiration (if any).
//   3. Checks for tunnel termination signal (Usually user-generated).
//
//   When this function returns, program should tear down tunnel and exit.
//
// Possible return values:
//   - NO_ERROR: Exiting because we're terminating tunnel (user request).
//   - KEEP_ALIVE_ERROR: Something went wrong in keep-alive initialisation.
//   - KEEP_ALIVE_TIMEOUT: Keep-alive timeout occured.
//   - LEASE_EXPIRED: Tunnel lifetime has ended.
//
int tspPerformTunnelLoop( const PTUNNEL_LOOP_CONFIG pTunLoopCfg )
{
  long tun_expiration = 0;
  int retCode = NO_ERROR;


  // Compute tunnel expiration time. (IETF DSTM Draft 6.1)
  //
  if( pTunLoopCfg->tun_lifetime != 0 )
  {
    tun_expiration = tspLeaseGetExpTime( pTunLoopCfg->tun_lifetime );
  }


  // Initialize keep-alive, if configured.
  //
  if( pTunLoopCfg->ka_interval > 0 )
  {
    retCode = NetKeepaliveInit( pTunLoopCfg->ka_src_addr,
                                pTunLoopCfg->ka_dst_addr,
                                pTunLoopCfg->ka_interval,
                                pTunLoopCfg->sa_family );
  }


  // Start tunnel management loop.
  //
  while( retCode == NO_ERROR  &&  tspCheckForStopOrWait( LOOP_WAIT_MS ) == 0 )
  {
    // Perform a round of keep-alive.
    if( pTunLoopCfg->ka_interval > 0  &&  NetKeepaliveDo() == 2 )
    {
      retCode = KEEPALIVE_TIMEOUT;
    }

    // Check for tunnel lease expiration.
    if( tun_expiration != 0  &&  tspLeaseCheckExp( tun_expiration ) == 1 )
    {
      retCode = LEASE_EXPIRED;
    }
  }


  // Clean-up resources allocated for keep-alive functionnality.
  //
  if( pTunLoopCfg->ka_interval > 0 )
  {
    NetKeepaliveDestroy();
  }


  // Exit function & return status.
  return retCode;
}

