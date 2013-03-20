/*
---------------------------------------------------------------------------
 $Id: md5.h,v 1.6 2007/05/02 13:32:21 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

/*
$FreeBSD: src/sys/sys/md5.h,v 1.13 1999/12/29 04:24:44 peter Exp $
---------------------------------------------------------------------------
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
*/

/*
#ifdef sun
typedef unsigned long uint32_t;
#endif
*/

#ifndef _SYS_MD5_H_
#define _SYS_MD5_H_
/* MD5 context. */
typedef struct MD5Context {
  uint32_t state[4];  /* state (ABCD) */
  uint32_t count[2];  /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64]; /* input buffer */
} MD5_CTX;

/*
#ifndef sun
#include <sys/cdefs.h>

__BEGIN_DECLS
#endif
*/

void   MD5Init (MD5_CTX *);
void   MD5Update (MD5_CTX *, const unsigned char *, unsigned int);
void   MD5Pad (MD5_CTX *);
void   MD5Final (unsigned char [16], MD5_CTX *);
char * MD5End(MD5_CTX *, char *);
char * MD5File(const char *, char *);
char * MD5Data(const unsigned char *, unsigned int, char *);
#ifdef _KERNEL
void MD5Transform __P((uint32_t [4], const unsigned char [64]));
#endif

#ifndef sun
//__END_DECLS
#endif

char * md5(char *, size_t);
void md5digest(char *, size_t, char *);
#endif /* _SYS_MD5_H_ */
