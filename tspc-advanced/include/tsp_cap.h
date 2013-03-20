/*
---------------------------------------------------------------------------
 $Id: tsp_cap.h,v 1.14 2007/05/02 13:32:22 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _TSP_CAP_H_
#define _TSP_CAP_H_

#include "net.h"
#include "errors.h"
#include "tsp_redirect.h"

/*
   Capability bytes

   16      8      1
   |-------|------|
               `--' - TUNNEL TYPE 1-4
           `--' AUTH METHOD 5-8
   `------' RESERVED 9-16
*/

/* the tunnel modes values correspond to tTunnelMode defined in config.h */
#define TUNNEL_V6V4 0x0001
#define TUNNEL_V6UDPV4  0x0002
#define TUNNEL_ANY      0x0003
#define TUNNEL_V4V6     0x0004

/* Authentication values */
#ifndef NO_OPENSSL
#define AUTH_PASSDSS_3DES_1 0x0080
#endif
#define AUTH_DIGEST_MD5 0x0040
#define AUTH_PLAIN  0x0020
#define AUTH_ANONYMOUS  0x0010
#define AUTH_ANY  0x00F0

typedef unsigned int tCapability;

tCapability tspSetCapability(char *, char *);
tErrors tspGetCapabilities(SOCKET, net_tools_t *, tCapability *, int, tConf *, tBrokerList **);
char* tspFormatCapabilities( char* szBuffer, const size_t bufLen, const tCapability cap );

#endif
