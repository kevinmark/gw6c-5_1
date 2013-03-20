/*
---------------------------------------------------------------------------
 $Id: tsp_net.h,v 1.10 2007/05/23 19:19:29 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code
---------------------------------------------------------------------------
*/

#ifndef _TSP_NET_H_
#define _TSP_NET_H_

#include "net.h"

/* definitions and constants */

#define MAXNAME 1024
#define MAXSERVER 1024
#define SERVER_PORT       3653
#define SERVER_PORT_STR   "3653"
#define PROTOCOLMAXPAYLOADCHUNK 2048
#define PROTOCOLFRAMESIZE 4096
#define PROTOCOLMAXHEADER 70

enum { PROTOCOL_OK, PROTOCOL_ERROR, PROTOCOL_EMEM, PROTOCOL_ESYNTAX, PROTOCOL_ESIZE, PROTOCOL_EREAD };

/* structures */

typedef struct stPayload {
  long size, PayloadCapacity;
  char *payload;
} tPayload;


/* exports */

extern SOCKET tspConnect(char *, net_tools_t *);
extern int tspClose(SOCKET, net_tools_t *);

extern int tspSendRecv(SOCKET socket, tPayload *, tPayload *, net_tools_t *) ;
extern int tspSend(SOCKET, tPayload *, net_tools_t *);
extern int tspReceive(SOCKET, tPayload *, net_tools_t *);



#endif

