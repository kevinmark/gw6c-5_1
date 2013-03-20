/*
---------------------------------------------------------------------------
 $Id: tsp_auth_passdss.h,v 1.7 2007/05/02 13:32:22 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code
---------------------------------------------------------------------------
*/

#ifndef _TSP_AUTH_PASSDSS_H_
#define _TSP_AUTH_PASSDSS_H_

#ifndef NO_OPENSSL

#include "net.h"
#include "tsp_redirect.h"

int AuthPASSDSS_3DES_1(SOCKET s, net_tools_t *nt, tConf *conf, tBrokerList **broker_list);

#endif

#endif
