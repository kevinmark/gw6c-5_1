/*
---------------------------------------------------------------------------
 $Id: lib.h,v 1.17 2007/11/28 17:27:07 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _LIB_H_
#define _LIB_H_

/* globals */

#define IPv4Addr    ".0123456789"
#define IPv6Addr    "ABCDEFabcdef:0123456789"
#define IPAddrAny   "ABCDEFabcdef:.0123456789"
#define Numeric     "0123456789"

/* exports */

int IsAll(char *, char *);
int IsPresent(char *);
int GetSizeOfNullTerminatedArray(char **);

char *tspGetErrorByCode(int code);

int IsAddressInPrefix( const char* address, const char* prefix, const short prefix_len );

#endif

