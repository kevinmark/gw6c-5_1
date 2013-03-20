/*
---------------------------------------------------------------------------
 $Id: net_rudp6.h,v 1.5 2007/05/23 19:19:28 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifdef V4V6_SUPPORT

#ifndef _rudp6_h_
#define _rudp6_h_

#include <sys/types.h>

extern SOCKET   NetRUDP6Connect   (char *, unsigned short);
extern int      NetRUDP6Close     (SOCKET);

extern int      NetRUDP6ReadWrite (SOCKET, char *, int, char *, int);

extern int      NetRUDP6Write     (SOCKET, char *, int);
extern int      NetRUDP6Printf    (SOCKET, char *, int, char *, ...);

extern int      NetRUDP6Read      (SOCKET, char *, int);

#endif

#endif /* V4V6_SUPPORT */
