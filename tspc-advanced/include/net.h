/*
---------------------------------------------------------------------------
 $Id: net.h,v 1.15 2007/05/23 19:19:28 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _NET_H_
#define _NET_H_

#include <sys/types.h>

struct net_tools {
  SOCKET  (*netopen)      (char *, unsigned short);
  int     (*netclose)     (SOCKET);

  int     (*netsendrecv)  (SOCKET, char *, int, char *, int);

  int     (*netsend)      (SOCKET, char *, int);
  int     (*netprintf)    (SOCKET, char *, int, char *, ...);

  int     (*netrecv)      (SOCKET, char *, int);
  int     (*netreadline)  (char *, int, char*, int);
};

typedef struct net_tools net_tools_t;

#define NET_TOOLS_T_SIZE 5

#define NET_TOOLS_T_RUDP 0
#define NET_TOOLS_T_UDP  1
#define NET_TOOLS_T_TCP  2
#ifdef V4V6_SUPPORT
#define NET_TOOLS_T_TCP6 3
#define NET_TOOLS_T_RUDP6 4
#endif /* V4V6_SUPPORT */

extern struct in_addr  *NetText2Addr    (char *, struct in_addr *);
extern struct in6_addr *NetText2Addr6   (char *, struct in6_addr *);

#endif
