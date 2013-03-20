/*
---------------------------------------------------------------------------
 $Id: tsp_tun.c,v 1.5 2007/12/18 18:58:02 carl Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2006 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

/* Darwin */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>

#define _USES_SYS_IOCTL_H_
#define _USES_SYS_SOCKET_H_

#include "platform.h"

#include <fcntl.h>

#include "tsp_tun.h"
#include "tsp_client.h"
#include "net_ka.h"
#include "log.h"
#include "hex_strings.h"
#include "errors.h"

#define HEXTUN_BUFSIZE 2048



// --------------------------------------------------------------------------
// Get the name of the tun device using file descriptor
//
void TunName(int tunfd, char *name, size_t name_len )
{
  struct stat sbuf;
  char *unsafe_buffer;

  if( fstat( tunfd, &sbuf ) != -1 )
  {
    unsafe_buffer = devname(sbuf.st_rdev, S_IFCHR);
    strncpy( name, unsafe_buffer, name_len );
  }
}


/* Open tun interface */
int TunInit( char* name )
{
  int tunfd;
  char iftun[128];

  snprintf( iftun, sizeof(iftun), "/dev/%s", name );

  tunfd = open( iftun, O_RDWR );
  if( tunfd == -1 )
  {
    Display(LOG_LEVEL_1, ELError, "TunInit", HEX_STR_ERR_OPEN_TUN_DEV, iftun);
    return(-1);
  }

  return tunfd;
}

int TunMainLoop( int tunfd,
                 SOCKET Socket,
                 tBoolean keepalive,
                 int keepalive_interval,
                 char *local_address_ipv6,
                 char *keepalive_address )
{
  fd_set rfds;
  int count, maxfd;
  char bufin[HEXTUN_BUFSIZE];
  char bufout[HEXTUN_BUFSIZE];
  struct timeval timeout;
  int ret = 0;

  if (keepalive_interval != 0) {
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000; // 100 ms
    NetKeepaliveInit(local_address_ipv6, keepalive_address, keepalive_interval, AF_INET6);
  }
  else {
    keepalive = FALSE;
    timeout.tv_sec = 7 * 24 * 60 * 60 ; /* one week */
    timeout.tv_usec = 0;
  }

  while( tspCheckForStopOrWait( 0 ) == 0 )
  {
    FD_ZERO(&rfds);
    FD_SET(tunfd,&rfds);
    FD_SET(Socket,&rfds);

    maxfd = tunfd>Socket?tunfd:Socket;

    if(keepalive)
      if (NetKeepaliveDo() == 2)
      {
        ret = KEEPALIVE_TIMEOUT;
        goto done;
      }


    ret = select(maxfd+1,&rfds,0,0,&timeout);

    if (ret > 0)
    {
      if( FD_ISSET(tunfd, &rfds) )
      {
        /* data sent through udp tunnel */
        /* ioctl(tunfd, FIONREAD, &count); */
        if ((count = read(tunfd, bufout, HEXTUN_BUFSIZE)) < -1)
        {
          Display(LOG_LEVEL_3, ELError, "TunMainLoop", HEX_STR_ERR_READING_TUN_DEV);
          ret = TUNNEL_ERROR;
          goto done;
        }
        NetKeepaliveGotWrite();
        if (send(Socket, bufout, count, 0) != count)
        {
          Display(LOG_LEVEL_3, ELError, "TunMainLoop", HEX_STR_ERR_WRITE_SOCKET);
          ret = TUNNEL_ERROR;
          goto done;
        }
      }

      if(FD_ISSET(Socket,&rfds))
      {
        /* data received through udp tunnel */
        count=recvfrom(Socket,bufin,HEXTUN_BUFSIZE,0,NULL,NULL);
        NetKeepaliveGotRead();
        if (write(tunfd, bufin, count) != count)
        {
          Display(LOG_LEVEL_3, ELError, "TunMainLoop", HEX_STR_ERR_WRITE_TUN_DEV);
          ret = TUNNEL_ERROR;
          goto done;
        }
      }
    }//if (ret>0)
  }

  /* Normal loop end */
  ret = NO_ERROR;

done:
  if (keepalive)
  {
    NetKeepaliveDestroy();
  }

  return ret;
}





