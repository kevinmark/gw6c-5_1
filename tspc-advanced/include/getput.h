/*
---------------------------------------------------------------------------
 $Id: getput.h,v 1.4 2007/05/02 13:32:21 cnepveu Exp $
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
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Macros for storing and retrieving data in msb first and lsb first order.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#ifndef GETPUT_H
#define GETPUT_H

/*------------ macros for storing/extracting msb first words -------------*/

#define GET_64BIT(cp) (((u_int64_t)(u_char)(cp)[0] << 56) | \
           ((u_int64_t)(u_char)(cp)[1] << 48) | \
           ((u_int64_t)(u_char)(cp)[2] << 40) | \
           ((u_int64_t)(u_char)(cp)[3] << 32) | \
           ((u_int64_t)(u_char)(cp)[4] << 24) | \
           ((u_int64_t)(u_char)(cp)[5] << 16) | \
           ((u_int64_t)(u_char)(cp)[6] << 8) | \
           ((u_int64_t)(u_char)(cp)[7]))

#define GET_32BIT(cp) (((u_long)(u_char)(cp)[0] << 24) | \
           ((u_long)(u_char)(cp)[1] << 16) | \
           ((u_long)(u_char)(cp)[2] << 8) | \
           ((u_long)(u_char)(cp)[3]))

#define GET_16BIT(cp) (((u_long)(u_char)(cp)[0] << 8) | \
           ((u_long)(u_char)(cp)[1]))

#define PUT_64BIT(cp, value) do { \
  (cp)[0] = (value) >> 56; \
  (cp)[1] = (value) >> 48; \
  (cp)[2] = (value) >> 40; \
  (cp)[3] = (value) >> 32; \
  (cp)[4] = (value) >> 24; \
  (cp)[5] = (value) >> 16; \
  (cp)[6] = (value) >> 8; \
  (cp)[7] = (value); } while (0)

#define PUT_32BIT(cp, value) do { \
  (cp)[0] = (value) >> 24; \
  (cp)[1] = (value) >> 16; \
  (cp)[2] = (value) >> 8; \
  (cp)[3] = (value); } while (0)

#define PUT_16BIT(cp, value) do { \
  (cp)[0] = (value) >> 8; \
  (cp)[1] = (value); } while (0)

#endif        /* GETPUT_H */
