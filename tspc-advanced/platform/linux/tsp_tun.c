/*
---------------------------------------------------------------------------
 $Id: tsp_tun.c,v 1.40 2007/12/18 18:58:06 carl Exp $
---------------------------------------------------------------------------
This source code copyright (c) Hexago Inc. 2002-2006.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

/* Linux */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/select.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#define _USES_SYS_IOCTL_H_
#define _USES_SYS_SOCKET_H_

#include "platform.h"

#include <linux/if.h>
#include <linux/if_tun.h>

#include "tsp_tun.h"        // Local function prototypes.
#include "tsp_client.h"     // tspCheckForStopOrWait()
#include "net_ka.h"         // NetKA*
#include "log.h"            // Display()
#include "hex_strings.h"    // String constants
#include "errors.h"         // Error codes.



/* Initialize tun interface */
int TunInit(char *TunDevice)
{
  int tunfd;
  struct ifreq ifr;
  char iftun[128];
  unsigned long ioctl_nochecksum = 1;

  /* for linux, force the use of "tun" */
  strcpy(iftun,"/dev/net/tun");

  tunfd = open(iftun,O_RDWR);
  if (tunfd == -1) {
    Display(LOG_LEVEL_1, ELError, "TunInit", HEX_STR_ERR_OPEN_DEV, iftun);
    Display(LOG_LEVEL_1, ELError, "TunInit", HEX_STR_TRY_MODPROBE_TUN);
    return (-1);
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN;
  strncpy(ifr.ifr_name, TunDevice, IFNAMSIZ);


  /* Enable debugging on tunnel device */
  /*
  Display(1, ELError, "TunInit" , "Enable tun device debugging");
  if(ioctl(tunfd, TUNSETDEBUG, (void *) &ioctl_nochecksum) == -1) {
    Display(1, ELError,"TunInit","ioctl failed");
  }*/

  if((ioctl(tunfd, TUNSETIFF, (void *) &ifr) == -1) ||
     (ioctl(tunfd, TUNSETNOCSUM, (void *) ioctl_nochecksum) == -1)) {
    Display(LOG_LEVEL_3, ELError, "TunInit", HEX_STR_ERR_CONFIG_TUN_DEV_REASON, iftun,strerror(errno));
    close(tunfd);

    return(-1);
  }

  return tunfd;
}

int TunMainLoop(int tunfd, SOCKET Socket, tBoolean keepalive, int keepalive_interval,
        char *local_address_ipv6, char *keepalive_address)
{
  fd_set rfds;
  int count, maxfd;
  unsigned char bufin[2048];
  unsigned char bufout[2048];
  struct timeval timeout;
  int ret;

  if (keepalive_interval != 0)
  {
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000; // 100 ms
    NetKeepaliveInit(local_address_ipv6, keepalive_address, keepalive_interval, AF_INET6);
  }
  else
  {
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

    bufin[0]=0;
    bufin[1]=0;
    bufin[2]=0x86;
    bufin[3]=0xdd;

    if( keepalive )
    {
      // Reinit timeout variable; in linux select modifies it.
      timeout.tv_sec = 0;
      timeout.tv_usec = 100000; // 100 ms
      if (NetKeepaliveDo() == 2)
      {
        ret = KEEPALIVE_TIMEOUT;
        goto done;
      }
    }

    ret = select(maxfd+1,&rfds,0,0,&timeout);
    if( ret > 0 )
    {
      if( FD_ISSET(tunfd,&rfds) )
      {
        /* data sent through udp tunnel */
        if ( (count = read(tunfd,bufout,sizeof(bufout) )) == -1 )
        {
          Display( LOG_LEVEL_3, ELError, "TunMainLoop",HEX_STR_ERR_READING_TUN_DEV );
          ret = TUNNEL_ERROR;
          goto done;
        }
        NetKeepaliveGotWrite();
        if (send(Socket,bufout+4,count-4,0) != count-4)
        {
          Display( LOG_LEVEL_3, ELError, "TunMainLoop",HEX_STR_ERR_WRITE_SOCKET );
          ret = TUNNEL_ERROR;
          goto done;
        }
      }
      if( FD_ISSET(Socket,&rfds) )
      {
        /* data received through udp tunnel */
        count = recvfrom(Socket,bufin+4,2048-4,0,NULL,NULL);
        NetKeepaliveGotRead();
        if( write(tunfd,bufin,count+4) != count+4 )
        {
          Display( LOG_LEVEL_3, ELError, "TunMainLoop", HEX_STR_ERR_WRITE_TUN_DEV );
          ret = TUNNEL_ERROR;
          goto done;
        }
      }
    }
  }   // while()

  /* Normal loop end */
  ret = NO_ERROR;

done:
  if (keepalive)
  {
    NetKeepaliveDestroy();
  }

  return ret;
}
