/*
----------------------------------------------------------------------
 base64.h - Base64 encoding and decoding prototypes.
----------------------------------------------------------------------
 $Id: base64.h,v 1.4 2007/05/02 13:32:20 cnepveu Exp $
----------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
----------------------------------------------------------------------
*/


int base64decode_len(const char *bufcoded);
int base64decode(char *bufplain, const char *bufcoded);
int base64decode_binary(unsigned char *bufplain, const char *bufcoded);
int base64encode_len(int len);
int base64encode(char *encoded, const char *string, int len);
int base64encode_binary(char *encoded, const unsigned char *string, int len);

