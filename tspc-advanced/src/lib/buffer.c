/*
---------------------------------------------------------------------------
 $Id: buffer.c,v 1.8 2007/05/18 16:10:44 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

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
 * Functions for manipulating fifo buffers (that can grow if needed).
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#include <stdlib.h>
#include <string.h>

#include "platform.h"

#include "buffer.h"
#include "log.h"
#include "hex_strings.h"

/* Initializes the buffer structure. */

void
buffer_init(Buffer *buffer)
{
	const u_int len = 4096;

	buffer->alloc = 0;
	buffer->buf = malloc(len);
	buffer->alloc = len;
	buffer->offset = 0;
	buffer->end = 0;
}

/* Frees any memory used for the buffer. */

void
buffer_free(Buffer *buffer)
{
	if (buffer->alloc > 0) {
		memset(buffer->buf, 0, buffer->alloc);
		free(buffer->buf);
		buffer->buf = NULL;
		buffer->alloc = 0;
	}
}

/*
 * Clears any data from the buffer, making it empty.  This does not actually
 * zero the memory.
 */

void
buffer_clear(Buffer *buffer)
{
	buffer->offset = 0;
	buffer->end = 0;
}

/* Appends data to the buffer, expanding it if necessary. */

void
buffer_append(Buffer *buffer, const void *data, size_t len)
{
	void *p;
	p = buffer_append_space(buffer, len);
	memcpy(p, data, len);
}

/*
 * Appends space to the buffer, expanding the buffer if necessary. This does
 * not actually copy the data into the buffer, but instead returns a pointer
 * to the allocated region.
 */

void *
buffer_append_space(Buffer *buffer, size_t len)
{
	size_t newlen;
	void *p;

	if (len > 0x100000) {
		Display(LOG_LEVEL_3, ELError,
			"buffer_append_space", HEX_STR_LEN_NOT_SUPPORTED, len);
		exit(-1);
	}
	/* If the buffer is empty, start using it from the beginning. */
	if (buffer->offset == buffer->end) {
		buffer->offset = 0;
		buffer->end = 0;
	}
restart:
	/* If there is enough space to store all data, store it now. */
	if (buffer->end + len < buffer->alloc) {
		p = buffer->buf + buffer->end;
		buffer->end += len;
		return p;
	}
	/*
	 * If the buffer is quite empty, but all data is at the end, move the
	 * data to the beginning and retry.
	 */
	if (buffer->offset > buffer->alloc / 2) {
		memmove(buffer->buf, buffer->buf + buffer->offset,
			buffer->end - buffer->offset);
		buffer->end -= buffer->offset;
		buffer->offset = 0;
		goto restart;
	}
	/* Increase the size of the buffer and retry. */
	newlen = buffer->alloc + len + 32768;
	if (newlen > 0xa00000) {
		Display(LOG_LEVEL_3, ELError,
			"buffer_append_space", HEX_STR_ALLOC_NOT_SUPPORTED, newlen);
		exit(-1);
	}
	buffer->buf = realloc(buffer->buf, newlen);
	buffer->alloc = newlen;
	goto restart;
	/* NOTREACHED */
}

/* Returns the number of bytes of data in the buffer. */

size_t
buffer_len(Buffer *buffer)
{
	return buffer->end - buffer->offset;
}

/* Gets data from the beginning of the buffer. */

void
buffer_get(Buffer *buffer, void *buf, u_int len)
{
	if (len > buffer->end - buffer->offset) {
		Display(LOG_LEVEL_3, ELError,
			"buffer_get", HEX_STR_TRYING_GET_MORE_BYTES,
			len, buffer->end - buffer->offset);
		exit(-1);
	}
	memcpy(buf, buffer->buf + buffer->offset, len);
	buffer->offset += len;
}

/* Consumes the given number of bytes from the beginning of the buffer. */

void
buffer_consume(Buffer *buffer, u_int bytes)
{
	if (bytes > buffer->end - buffer->offset) {
		Display(LOG_LEVEL_3, ELError,
			"buffer_consume", HEX_STR_TRYING_GET_MORE_BYTES,
			bytes, buffer->end - buffer->offset);
		exit(-1);
	}
	buffer->offset += bytes;
}

/* Consumes the given number of bytes from the end of the buffer. */

void
buffer_consume_end(Buffer *buffer, u_int bytes)
{
	if (bytes > buffer->end - buffer->offset) {
		Display(LOG_LEVEL_3, ELError,
			"buffer_consume_end",
			HEX_STR_TRYING_GET_MORE_BYTES,
			bytes, buffer->end - buffer->offset);
		exit(-1);
	}
	buffer->end -= bytes;
}

/* Returns a pointer to the first used byte in the buffer. */

void *
buffer_ptr(Buffer *buffer)
{
	return buffer->buf + buffer->offset;
}
