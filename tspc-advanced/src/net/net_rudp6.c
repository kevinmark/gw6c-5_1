/*
---------------------------------------------------------------------------
 $Id: net_rudp6.c,v 1.7 2007/05/23 19:19:36 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2005,2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
--------------------------------------------------------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <errno.h>

#define _USES_SYS_TIME_H_
#define _USES_SYS_SOCKET_H_
#define _USES_NETINET_IN_H_
#define _USES_ARPA_INET_H_
#define _USES_NETDB_H_

#include "platform.h"

#include "net_rudp.h"
#include "net_rudp6.h"
#include "net.h"
#include "log.h"

#ifdef V4V6_SUPPORT

// forward declarations (IPv6 version of internal_get_sai())

static struct sockaddr_in6 *internal_get_sai6(rttengine_stat_t *, char *, unsigned short);


/* Exported functions */

int NetRUDP6Init(void) 
{
	memset(&rttengine_stats, 0, sizeof(rttengine_stat_t));
	return rttengine_init(&rttengine_stats);
}

/* */

int NetRUDP6Destroy(void) 
{
	if ( rttengine_deinit(&rttengine_stats, NULL, NULL) == 0)
		return 1;
	return 0;
}

/* */

SOCKET NetRUDP6Connect(char *Host, unsigned short Port) 
{
	SOCKET sfd;
	struct sockaddr_in6 *sai;

	if (rttengine_stats.initiated == 0)
		NetRUDP6Init();

	if ( (sai = internal_get_sai6(&rttengine_stats, Host, Port)) == NULL) {
		return (SOCKET)(-1);
	}


	/* and get a socket */

	if ( (sfd = socket(PF_INET6, SOCK_DGRAM, 0)) == -1 ) {
		return (SOCKET)(-1);
	}

	/* then connect it */

	if ( (connect(sfd,(struct sockaddr *) sai, sizeof(struct sockaddr_in6))) == -1 ) {

		return (SOCKET)(-1);
	}
	
	return sfd;
}

/* */

int NetRUDP6Close(SOCKET sock) 
{
	shutdown(sock, SHUT_RDWR);
	CLOSESOCKET(sock);
	return NetRUDP6Destroy();
}

/* */

int NetRUDP6ReadWrite(SOCKET sock, char *in, int il, char *out, int ol)
{
	return internal_send_recv(sock, in, il, out, ol);
}

/* */

int NetRUDP6Write(SOCKET sock, char *b, int l) 
{
	return NetRUDPReadWrite(sock, b, l, NULL, 0);
}

/* */

int NetRUDP6Printf(SOCKET sock, char *out, int ol, char *Format, ...)
{
  va_list argp;
  int Length;
  char Data[1024];

  va_start(argp, Format);
  vsnprintf(Data, sizeof Data, Format, argp);
  va_end(argp);

  Length = (int)strlen(Data);

  return NetRUDP6ReadWrite(sock, Data, (int)strlen(Data), out, ol);
}

/* */

int NetRUDP6Read(SOCKET sock, char *b, int l) 
{
	return NetRUDP6ReadWrite(sock, NULL, 0, b, l);
}

static struct sockaddr_in6 *
internal_get_sai6(rttengine_stat_t *s, char *Host, unsigned short Port) 
{
	/* we need to be reinitialised for each new connection,
	 * so we can check if we already have something
	 * cached and assume it is fit for the
	 * current situation
	 */

	struct sockaddr_in6 *sai;
	struct in6_addr addr6;

	/* so, is it cached? */
	
	if (s->sai != NULL)
		return (struct sockaddr_in6 *)s->sai;

	/* its not */
	
	/* get the IP address from the hostname */

	if(NetText2Addr6(Host, &addr6) == NULL )
			return NULL;

	/* get memory for our patente */
	if ( (sai = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6))) == NULL)
		return NULL;
	
	/* clear out our sockaddr_in entry, fill it and cache it */

	memset(sai, 0, sizeof(struct sockaddr_in6));
	sai->sin6_family = PF_INET6;
	sai->sin6_port = htons(Port);
	memcpy(&sai->sin6_addr, &addr6, sizeof(struct in6_addr));
	s->sai = (struct sockaddr *)sai;

	return sai;
}


#endif /* V4V6_SUPPORT */
