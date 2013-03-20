/*
---------------------------------------------------------------------------
 $Id: net_ka.h,v 1.25 2007/11/28 17:27:07 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

int   NetKeepaliveInit    ( char *src, char *dst, int maximal_keepalive, int family );

void  NetKeepaliveDestroy ( void );

int   NetKeepaliveDo      ( void );

void  NetKeepaliveGotRead ( void );

void  NetKeepaliveGotWrite( void );
