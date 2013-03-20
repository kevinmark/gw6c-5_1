/*
---------------------------------------------------------------------------
 $Id: tsp_tun.h,v 1.3 2007/04/25 19:31:39 cnepveu Exp $
---------------------------------------------------------------------------
This source code copyright (c) Hexago Inc. 2002-2005,2007.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef TUN_H
#define TUN_H

#include "config.h"

extern int TunInit(char *TunDevice);
extern int TunMainLoop(int tunfd, SOCKET Socket, tBoolean keepalive, int keepalive_interval,
		       char *local_address_ipv6, char *keepalive_address);


#endif /* TUN_H */
