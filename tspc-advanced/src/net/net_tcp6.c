/*
---------------------------------------------------------------------------
 $Id: net_tcp6.c,v 1.8 2007/05/23 19:19:36 cnepveu Exp $
---------------------------------------------------------------------------
* This source code copyright (c) Hexago Inc. 2002-2004,2007.
* 
* This program is free software; you can redistribute it and/or modify it 
* under the terms of the GNU General Public License (GPL) Version 2, 
* June 1991 as published by the Free Software Foundation.
* 
* This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY;  without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License 
* along with this program; see the file GPL_LICENSE.txt. If not, write 
* to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
* MA 02111-1307 USA
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

#include "net_tcp6.h"
#include "net.h" /* NetText2Addr */


/* */


SOCKET NetTCP6Connect(char *Host, unsigned short Port) 
{
  SOCKET                 sockfd;
  struct sockaddr_in6  serv_addr;
  struct in6_addr      addr;

  if (NULL == NetText2Addr6(Host, &addr)) {
    return (SOCKET)(-1);
  }

  memset(&serv_addr, 0, sizeof(serv_addr)); 
  serv_addr.sin6_family      = AF_INET6;
  serv_addr.sin6_port        = htons(Port);
  serv_addr.sin6_addr = addr;

/*
 * Open a TCP socket (an Internet 6 stream socket).
 */

  if ( (sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
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



/* */


int NetTCP6Close(SOCKET Socket)
{
  shutdown(Socket, SHUT_RDWR);
  return CLOSESOCKET(Socket);
}

int NetTCP6ReadWrite (SOCKET sock, char *bi, int li, char *bo, int lo) 
{
   if ( NetTCP6Write(sock, bi, li) != li)
                return -1;

   return NetTCP6Read(sock, bo, lo);
}


/* */


int NetTCP6Write (SOCKET sock, char *b, int l) 
{
  int nleft, nwritten;
  char *ptr;

  ptr = b;   /* can't do pointer arithmetic on void * */
  nleft = l;
  while (nleft > 0) {
    if ((nwritten = send(sock, ptr, nleft, 0)) <= 0) {
      return nwritten;          /* error */
    }

    nleft -= nwritten;
    ptr   += nwritten;
  }
  return(l);
}


/* */


int NetTCP6Printf (SOCKET sock, char *out, int pl, char *Format, ...) 
{
  va_list argp;
  int Length;
  char Data[1024];

  va_start(argp, Format);
  vsnprintf(Data, sizeof Data, Format, argp);
  va_end(argp);

  Length = (int)strlen(Data);

  if(NetTCP6Write(sock, Data, (int)strlen(Data)) != Length) {
    return 0;
  }

  return NetTCP6Read(sock, out, pl);
}



/* */ 

int NetTCP6Read (SOCKET sock, char *in, int l) 
{
  return( recv(sock, in, l, 0) );
}
