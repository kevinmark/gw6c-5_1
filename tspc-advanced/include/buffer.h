/*
---------------------------------------------------------------------------
 $Id: buffer.h,v 1.5 2007/05/18 16:10:37 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

/*  $NetBSD: buffer.h,v 1.1.1.6 2002/03/08 01:20:34 itojun Exp $  */
/*  $OpenBSD: buffer.h,v 1.11 2002/03/04 17:27:39 stevesk Exp $ */

/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Code for manipulating FIFO buffers.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <sys/types.h>

typedef struct {
  u_char  *buf;   /* Buffer for data. */
  size_t alloc;   /* Number of bytes allocated for data. */
  size_t offset;  /* Offset of first byte containing data. */
  size_t end;   /* Offset of last byte containing data. */
}       Buffer;

void   buffer_init(Buffer *);
void   buffer_clear(Buffer *);
void   buffer_free(Buffer *);

size_t  buffer_len(Buffer *);
void  *buffer_ptr(Buffer *);

void   buffer_append(Buffer *, const void *, size_t);
void  *buffer_append_space(Buffer *, size_t);

void   buffer_get(Buffer *, void *, u_int);

void   buffer_consume(Buffer *, u_int);
void   buffer_consume_end(Buffer *, u_int);

#endif        /* BUFFER_H */
