/*
---------------------------------------------------------------------------
 $Id: bufaux.h,v 1.5 2007/05/02 13:32:20 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

/*  $NetBSD: bufaux.h,v 1.1.1.7 2002/04/22 07:37:19 itojun Exp $  */
/*  $OpenBSD: bufaux.h,v 1.18 2002/04/20 09:14:58 markus Exp $  */

/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#ifndef BUFAUX_H
#define BUFAUX_H

#ifndef NO_OPENSSL

#include "buffer.h"
#include <openssl/bn.h>

void    buffer_put_bignum(Buffer *, BIGNUM *);
void  buffer_get_bignum(Buffer *, BIGNUM *);
u_int buffer_get_int(Buffer *);
void    buffer_put_int(Buffer *, u_int);

int     buffer_get_octet(Buffer *);
void    buffer_put_octet(Buffer *, int);

void   *buffer_get_string(Buffer *, u_int *);
void    buffer_put_string(Buffer *, const void *, u_int);
void  buffer_put_cstring(Buffer *, const char *);

#define buffer_skip_string(b) \
    do { u_int l = buffer_get_int(b); buffer_consume(b, l); } while(0)

#endif

#endif        /* BUFAUX_H */
