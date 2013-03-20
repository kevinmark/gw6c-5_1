/*
---------------------------------------------------------------------------
 $Id: bufaux.c,v 1.11 2007/11/28 17:27:29 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef NO_OPENSSL

/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Auxiliary functions for storing and retrieving various data types to/from
 * Buffers.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 *
 *
 * SSH2 packet format added by Markus Friedl
 * Copyright (c) 2000,2006,2007 Markus Friedl.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <openssl/bn.h>

#include "platform.h"

#include "bufaux.h"
#include "getput.h"
#include "log.h"
#include "hex_strings.h"

/**
 * Stores a BIGNUM to buffer in mpint format
 *
 * From draft-newman-sasl-passdss-01.txt:
 *    mpint   Represents multiple precision integers in two's complement
 *            format, stored as a string, most significant octet first.
 *            Negative numbers have one in the most significant bit of
 *            the first octet of the string data. If the most significant
 *            bit would be set for a positive number, the number MUST be
 *            preceded by a zero byte.  Unnecessary leading zero or 255
 *            bytes MUST NOT be included.  The value zero MUST be stored
 *            as a string with zero bytes of data.
 *
 *
 * @param buffer
 * @param value
 *
 * @return
 */
void
buffer_put_bignum(Buffer *buffer, BIGNUM *value)
{
  int bits = BN_num_bits(value);
  int bin_size = (bits + 7) / 8;
  u_char *buf = malloc(bin_size);
  int oi;

  if (buf == NULL) {
    Display(LOG_LEVEL_3, ELError,
      "buffer_put_bignum", HEX_STR_MALLOC_ERROR);
    exit(-1);
  }
  /* Get the value of in binary */
  oi = BN_bn2bin(value, buf);
  if (oi != bin_size) {
    Display(LOG_LEVEL_3, ELError,
      "buffer_put_bignum", HEX_STR_BN_BN2BIN_FAILED,
      oi, bin_size);
    exit(1);
  }

  /* Store the number of bits in the buffer in 4 bytes, msb first. */
  buffer_put_int(buffer, bin_size);
  /* Store the binary data. */
  buffer_append(buffer, (char *)buf, oi);

  memset(buf, 0, bin_size);
  free(buf);
}

/*
 * Retrieves an BIGNUM from the buffer.
 */
void
buffer_get_bignum(Buffer *buffer, BIGNUM *value)
{
  int bytes;
  u_char *bin;

  /* Get the number of bytes. */
  bytes = buffer_get_int(buffer);
  if (bytes > 8 * 1024) {
    Display(LOG_LEVEL_3, ELError,
      "buffer_get_bignum", HEX_STR_CANT_HANDLE_BN_SIZE, bytes);
    exit(1);
  }
  if( (int)buffer_len(buffer) < bytes) {
    Display(LOG_LEVEL_3, ELError,
      "buffer_get_bignum", HEX_STR_INPUT_BUF_TOO_SMALL);
    exit(1);
  }
  bin = buffer_ptr(buffer);
  BN_bin2bn(bin, bytes, value);
  buffer_consume(buffer, bytes);
}

u_int
buffer_get_int(Buffer *buffer)
{
  u_char buf[4];

  buffer_get(buffer, (char *) buf, 4);
  return GET_32BIT(buf);
}

void
buffer_put_int(Buffer *buffer, u_int value)
{
  char buf[4];

  PUT_32BIT(buf, value);
  buffer_append(buffer, buf, 4);
}

/**
 * From draft-newman-sasl-passdss-01.txt:
 * "A string is a length-prefixed octet string.  The length is
 * represented as a uint32 with the data immediately
 * following.  A length of 0 indicates an empty string.  The
 * string MAY contain NUL or 8-bit octets.  When used to
 * represent textual strings, the characters are interpreted
 * in UTF-8 [UTF-8].  Other character encoding schemes MUST
 * NOT be used."
 *
 * Returns an arbitrary binary string from the buffer.  The string
 * cannot be longer than 256k.  The returned value points to memory
 * allocated with malloc; it is the responsibility of the calling
 * function to free the data.  If length_ptr is non-NULL, the length
 * of the returned data will be stored there.  A null character will
 * be automatically appended to the returned string, and is not
 * counted in length.
 *
 * @param buffer
 * @param s
 *
 * @return
 */
void *
buffer_get_string(Buffer *buffer, u_int *length_ptr)
{
  u_char *value;
  u_int len;

  /* Get the length. */
  len = buffer_get_int(buffer);
  if (len > 256 * 1024) {
    Display(LOG_LEVEL_3, ELError,
      "buffer_get_string", HEX_STR_BAD_STRING_LENGTH, len);
    exit(1);
  }
  /* Allocate space for the string.  Add one byte for a null character. */
  value = malloc(len + 1);
  /* Get the string. */
  buffer_get(buffer, value, len);
  /* Append a null character to make processing easier. */
  value[len] = 0;
  /* Optionally return the length of the string. */
  if (length_ptr)
    *length_ptr = len;
  return value;
}

/*
 * Stores and arbitrary binary string in the buffer.
 */
void
buffer_put_string(Buffer *buffer, const void *buf, u_int len)
{
  buffer_put_int(buffer, len);
  buffer_append(buffer, buf, len);
}
void
buffer_put_cstring(Buffer *buffer, const char *s)
{
  if (s == NULL) {
    Display(LOG_LEVEL_3, ELError,
      "buffer_put_cstring", HEX_STR_S_NULL);
    exit(1);
  }
  buffer_put_string(buffer, s, (unsigned int)strlen(s));
}

/*
 * Returns a character from the buffer (0 - 255).
 */
int
buffer_get_octet(Buffer *buffer)
{
  char ch;

  buffer_get(buffer, &ch, 1);
  return (u_char) ch;
}

/*
 * Stores a character in the buffer.
 */
void buffer_put_octet(Buffer *buffer, int value)
{
  char ch = (char)value;

  buffer_append(buffer, &ch, 1);
}

#endif

