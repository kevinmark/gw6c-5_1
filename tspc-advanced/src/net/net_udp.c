/*
---------------------------------------------------------------------------
 $Id: net_udp.c,v 1.13 2007/05/23 19:19:36 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2005,2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define _USES_SYS_SOCKET_H_
#define _USES_NETINET_IN_H_
#define _USES_ARPA_INET_H_
#define _USES_NETDB_H_

#include "platform.h"

#include "net_udp.h"
#include "net.h"

SOCKET NetUDPConnect(char *Host, unsigned short Port)
{
  SOCKET              sockfd;
  struct sockaddr_in  serv_addr;
  struct in_addr      addr;

  if(NetText2Addr(Host, &addr) == NULL) {
    return (SOCKET)(-1);
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_port        = htons(Port);
  serv_addr.sin_addr.s_addr = addr.s_addr;

  /*
   * Open a UDP socket.
   */

  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    return (SOCKET)(-1);
  }

  /*
   * Connect to the server.
   */

  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    return (SOCKET)(-1);
  }

  return sockfd;
}


int
NetUDPClose(SOCKET Socket)
{
  shutdown(Socket, SHUT_RDWR);
  return CLOSESOCKET(Socket);
}


int NetUDPReadWrite(SOCKET sock, char *bi, int li, char *bo, int lo) 
{
	if ( NetUDPWrite(sock, bi, li) != li)
		return -1;
	
	return NetUDPRead(sock, bo, lo);
}
	


int NetUDPWrite(SOCKET sock, char *b, int l) 
{
	int nwritten, nleft;
	char *ptr;

	ptr = b;	/* can't do pointer arithmetic on void* */
	nleft = l;
	while (nleft > 0) {
    nwritten = send(sock, ptr, nleft, 0);
		if (nwritten <= 0) {
			return nwritten;		/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return l;
}


int
NetUDPPrintf (SOCKET sock, char *out, int ol, char *Format, ...) 
{
	va_list argp;
	char Data[1024];

	va_start(argp, Format);
	vsnprintf(Data, sizeof Data, Format, argp);
	va_end(argp);

	return NetUDPReadWrite(sock, Data, (int)strlen(Data), out, ol);
}

int NetUDPRead (SOCKET sock, char *b, int l) 
{
	return(recvfrom(sock, b, l, 0, NULL, NULL));

}


