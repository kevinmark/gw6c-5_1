/*
---------------------------------------------------------------------------
 $Id: tsp_tun_mgt.h,v 1.2 2007/11/28 17:27:07 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef __TSP_TUN_MGT_H__
#define __TSP_TUN_MGT_H__


typedef struct __TUNNEL_LOOP_CONFIG
{
  char*         ka_src_addr;    // KA source address (usually local endpoint).
  char*         ka_dst_addr;    // KA destination address (usually broker).
  int           sa_family;      // Socket address family [AF_INET, AF_INET6].
  unsigned int  ka_interval;    // Keep-alive interval in seconds.
  long          tun_lifetime;   // Tunnel lifetime (tunnel expiration feature).
} TUNNEL_LOOP_CONFIG, *PTUNNEL_LOOP_CONFIG;


int tspPerformTunnelLoop( const PTUNNEL_LOOP_CONFIG pTunLoopCfg );


#endif
