/*
---------------------------------------------------------------------------
 $Id: tsp_auth.h,v 1.9 2007/05/02 13:32:22 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _TSP_AUTH_H_
#define _TSP_AUTH_H_

#include "tsp_cap.h"
#include "net.h"
#include "tsp_redirect.h"

/**Bug1455:
 * If connecting to a Migration Server using TSP version 2.0.0 or earlier,
 * MD5 digest authentication may be wrongly calculated in some
 * username and other credentials combinations. The define below ensure
 * an MD5 digest compatible with earlier Migration Broker is generated
 * when the TSP protocol version is 2.0.0 or earlier.
 */
#define SUPPORT_MD5_BUG1455 1

int tspAuthenticate(SOCKET, tCapability, net_tools_t *, tConf *, tBrokerList **, int);

#endif
