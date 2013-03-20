/*
---------------------------------------------------------------------------
 $Id: net_cksm.c,v 1.7 2007/05/23 19:59:40 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2005 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include "net_cksm.h"

/* checksum calculation */
unsigned short in_cksum(unsigned short *addr, int len) 
{
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	if (nleft == 1) {
		*(unsigned char *) (&answer) = *(unsigned char *) w;
		sum += answer;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = (unsigned short)(~sum);
	return (answer);
}
