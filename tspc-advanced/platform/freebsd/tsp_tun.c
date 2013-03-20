/*
---------------------------------------------------------------------------
 $Id: tsp_tun.c,v 1.28 2007/12/18 18:58:04 carl Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2006 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

/* FreeBSD */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>

#define _USES_SYS_IOCTL_H_
#define _USES_SYS_SOCKET_H_
#define _USES_NET_IF_H_
#define _USES_ARPA_INET_H_
#define _USES_NETINET_IN_H_
#define _USES_NETINET_IP6_H_
#define _USES_NETINET_ICMP6_H_

#include "platform.h"

#include <fcntl.h>
#include <net/if_tun.h>

#include "tsp_tun.h"
#include "tsp_client.h"   /* tspCheckForStopOrWait() */
#include "net_ka.h"
#include "log.h"
#include "hex_strings.h"
#include "lib.h"          /* Display */
#include "config.h"       /* tBoolean define */
#include "errors.h"


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


/* Initialize tun interface */
int TunInit( char* dont_care )
{
  int tunfd;
  int ifmode = IFF_POINTOPOINT | IFF_MULTICAST;
  int tundebug = 0;
  int tunhead = 1;
  const char* iftun = "/dev/tun";

  tunfd = open(iftun,O_RDWR);
  if (tunfd == -1) {
    Display(LOG_LEVEL_1, ELError, "TunInit", HEX_STR_ERR_OPEN_TUN_DEV, iftun);
    return(-1);
  }

  if ((ioctl(tunfd,TUNSDEBUG,&tundebug) == -1) ||
      (ioctl(tunfd,TUNSIFMODE,&ifmode) == -1) ||
      (ioctl(tunfd,TUNSIFHEAD,&tunhead) == -1)) {
    close(tunfd);
    Display(LOG_LEVEL_3, ELError, "TunInit", HEX_STR_ERR_CONFIG_TUN_DEV, iftun);
    return(-1);
  }
  return tunfd;
}

int TunMainLoop(int tunfd, SOCKET Socket, tBoolean keepalive, int keepalive_interval,
        char *local_address_ipv6, char *keepalive_address) {
  fd_set rfds;
  int count, maxfd;
  char bufin[2048];
  char bufout[2048];
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

  bufin[0] = 0;
  bufin[1] = 0;
  bufin[2] = 0;
  bufin[3] = 0x1c;

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


    // Wait until data is sent or received. - OR - A signal has been caught.
    ret = select(maxfd+1,&rfds,0,0,&timeout);
    if (ret > 0)
    {
      if( FD_ISSET(tunfd, &rfds) )
      {
        /* data sent through udp tunnel */
        ioctl(tunfd, FIONREAD, &count);
        if (count > sizeof(bufout))
        {
          Display(LOG_LEVEL_3, ELError, "TunMainLoop", HEX_STR_ERR_RD_TUN_DEV_BUFSMALL);
          ret = TUNNEL_ERROR;
          goto done;
        }
        if (read(tunfd, bufout, count) != count)
        {
          Display(LOG_LEVEL_3, ELError, "TunMainLoop", HEX_STR_ERR_READING_TUN_DEV);
          ret = TUNNEL_ERROR;
          goto done;
        }

        NetKeepaliveGotWrite();

        if (send(Socket, bufout+4, count-4, 0) != count-4)
        {
          Display(LOG_LEVEL_3, ELError, "TunMainLoop", HEX_STR_ERR_WRITE_SOCKET);
          ret = TUNNEL_ERROR;
          goto done;
        }
      }

      if(FD_ISSET(Socket,&rfds))
      {
        /* data received through udp tunnel */
        count=recvfrom(Socket,bufin+4,2048 - 4,0,NULL,NULL);

        NetKeepaliveGotRead();

        if (write(tunfd, bufin, count + 4) != count + 4)
        {
          Display(LOG_LEVEL_3, ELError, "TunMainLoop", HEX_STR_ERR_WRITE_TUN_DEV);
          ret = TUNNEL_ERROR;
          goto done;
        }
      }
    }
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
