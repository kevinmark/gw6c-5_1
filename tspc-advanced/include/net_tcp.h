/*
---------------------------------------------------------------------------
 $Id: net_tcp.h,v 1.8 2007/05/23 19:19:28 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _NET_TCP_H_
#define _NET_TCP_H_

extern SOCKET   NetTCPConnect   (char *, unsigned short);
extern int      NetTCPClose     (SOCKET);

extern int      NetTCPReadWrite (SOCKET, char *, int, char *, int);

extern int      NetTCPWrite     (SOCKET, char *, int);
extern int      NetTCPPrintf    (SOCKET, char *, int, char *, ...);

extern int      NetTCPRead      (SOCKET, char *, int);

#endif
