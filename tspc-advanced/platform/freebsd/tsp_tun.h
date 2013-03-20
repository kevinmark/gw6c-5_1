/*
---------------------------------------------------------------------------
 $Id: tsp_tun.h,v 1.9 2007/11/28 17:27:12 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2005 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef TUN_H
#define TUN_H

#include "config.h"   // tBoolean

extern void TunName( int tunfd, char* name, size_t name_len );
extern int TunInit(char *dont_care);
extern int TunMainLoop(int tun, int Socket, tBoolean keepalive,
                       int keepalive_interval, char *local_address_ipv6,
             char *keepalive_address);

#endif /* TUN_H */
