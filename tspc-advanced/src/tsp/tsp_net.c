/*
---------------------------------------------------------------------------
 $Id: tsp_net.c,v 1.27 2007/11/28 17:27:36 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define _USES_SYS_SOCKET_H_

#include "platform.h"

#include "tsp_net.h"
#include "net.h"
#include "log.h"
#include "hex_strings.h"


SOCKET tspConnect(char *server, net_tools_t *nt)
{
   SOCKET socket;
   char Buffer[MAXNAME];
   char *srvname=NULL;
   char *srvport=NULL;

   memset(Buffer, 0, sizeof(Buffer));
   snprintf(Buffer, sizeof Buffer, "%s", server);

   /* First look for brackets in case of IPv6 address */
   srvname = strchr(Buffer,'[');
   if (srvname != NULL) {
     srvname = strtok(Buffer, "]");
     srvname = Buffer+1;
   } else {
     srvname = strtok(Buffer, ":");
   }

   if((srvport = strtok(NULL, ":"))==NULL) {
     srvport = SERVER_PORT_STR;
   }

#ifdef WIN32
  WindowsUDPPort = (unsigned short)atoi(srvport);
#endif

   if(atoi(srvport) <= 0) {
     Display(LOG_LEVEL_3, ELError, "tspConnect", HEX_STR_SERVICE_PORT_INVALID, srvport);
     return (SOCKET)(-1);
   }

   if((socket = nt->netopen(server, (unsigned short)atoi(srvport))) < 0) {
     Display(LOG_LEVEL_3, ELError, "tspConnect", HEX_STR_CANT_CONNECT_SERVICE_PORT, srvport);
     return (SOCKET)(-1);
   }

   return socket;
}


int tspClose(SOCKET socket, net_tools_t *nt)
{
  return nt->netclose(socket);
}

/* XXX add all the error checking done in send / receive */

int tspSendRecv(SOCKET socket, tPayload *plin, tPayload *plout, net_tools_t *nt) {

  char string[] = "Content-length: %ld\r\n";
  char buffer[PROTOCOLFRAMESIZE];
  char *ptr_b, *ptr_c;
  int read, ret, size, left;

  /* From what I understand, plin is data to be sent
     and plout->payload is a buffer for data to be received.
     We should be clearer... */

  /* add in content-length to data to be sent */
  snprintf(buffer, PROTOCOLFRAMESIZE, string, plin->size);
  size = (int)strlen(buffer);
  memcpy(buffer + size, plin->payload, plin->size);

  buffer[size + plin->size] = 0;
  Display(LOG_LEVEL_3,ELInfo,"tspSendRecv",HEX_STR_SENT);
  Display(LOG_LEVEL_3,ELInfo,"tspSendRecv","%s",buffer);

  /* send 'buffer', recv in 'plout->payload' */
  ret =  nt->netsendrecv(socket, buffer, size + plin->size, plout->payload, plout->size);

  /* test if data received (see bug 3295) */
  if (ret <= 0)
    return ret;

  /* valid we got 'Content-Length' */
  if (memcmp(plout->payload, "Content-length:", 15)) {
    Display(LOG_LEVEL_3, ELError, "tspReceive", HEX_STR_EXPECTED_CONTENT_LENGTH, plout->payload);
    return PROTOCOL_ERROR;
  }

  /* strip it from the returned string */
  ptr_c = strchr(plout->payload, '\n');

  /* test if valid data received (see bug 3295) */
  if (ptr_c == NULL) {
    Display(LOG_LEVEL_3,ELError,"tspSendRecv",HEX_STR_RECV_INVALID_TSP_DATA);
    return PROTOCOL_ERROR;
  }
  ptr_c++;
  size = (int)strlen(ptr_c);

  /* validate received data using Content-Length (see bug: 3164)
     (taken from tspReceive()) */
  if (((plout->size = atol(plout->payload + 15)) <= 0L) || (size > plout->size)) {
    Display(LOG_LEVEL_3, ELError, "tspSendRecv", HEX_STR_INVALID_PAYLOAD_SIZE);
    return PROTOCOL_ERROR;
  }

  left = plout->size - size;

  while (left) {
    if ((read = nt->netrecv(socket, (ptr_c + size), left)) <= 0) {
      Display(LOG_LEVEL_3, ELError, "tspSendRecv", HEX_STR_CANT_R_SERVER_SOCKET);
      return PROTOCOL_ERROR;
    }

    size += read;
    left -= read;
  }

  ptr_b = (char *)malloc(++size); // need space for a little 0 at the end */
  memset(ptr_b, 0, size);
  memcpy(ptr_b, ptr_c, --size);   /* but need not to overwrite that little 0 */

  free(plout->payload);
  plout->payload = ptr_b;

  Display(LOG_LEVEL_3,ELInfo,"tspSendRecv",HEX_STR_RECV);
  Display(LOG_LEVEL_3,ELInfo,"tspSendRecv","%s",plout->payload);

  return ret;
}

int tspSend(SOCKET socket, tPayload *pl, net_tools_t *nt)
{
  char Buffer[PROTOCOLFRAMESIZE];
  long ClSize;
  int ret = -1;

  snprintf(Buffer, PROTOCOLFRAMESIZE, "Content-length: %ld\r\n", pl->size);
  ClSize = (long)strlen(Buffer);

  if (ClSize + pl->size > PROTOCOLFRAMESIZE) {
    Display(LOG_LEVEL_3, ELError, "tspSend", HEX_STR_PAYLOAD_BIGGER_PROTOFRMSIZE);
    return -1;
  }

  memcpy(Buffer + ClSize,pl->payload, pl->size);

  Buffer[ClSize + pl->size] = 0;
  Display(LOG_LEVEL_3,ELInfo,"tspSend",HEX_STR_SENT);
  Display(LOG_LEVEL_3,ELInfo,"tspSend","%s",Buffer);

  if ( (ret = nt->netsend(socket, Buffer, ClSize + pl->size)) == -1) {
    Display(LOG_LEVEL_3, ELError, "tspSend", HEX_STR_CANT_W_TSP_REQ_SERVER_SOCKET);
    return ret;
  }

  return ret;
}

int tspReceive(SOCKET socket, tPayload *pl, net_tools_t *nt)
{
  int BytesTotal = 0, BytesRead = 0, BytesLeft = 0;
  char Buffer[PROTOCOLFRAMESIZE+1];
  char *StartOfPayload;

  memset(Buffer,0,sizeof(Buffer));

  if (pl->payload) free(pl->payload);
  if ((BytesRead = nt->netrecv(socket, Buffer, sizeof(Buffer))) <= 0) {
    Display(LOG_LEVEL_3, ELError, "tspReceive", HEX_STR_CANT_R_SERVER_SOCKET);
    return PROTOCOL_EREAD;
  }

  Display(LOG_LEVEL_3,ELInfo,"tspReceive",HEX_STR_RECV);
  Display(LOG_LEVEL_3,ELInfo,"tspReceive","%s",Buffer);

  if (memcmp(Buffer, "Content-length:", 15)) {
    Display(LOG_LEVEL_3, ELError, "tspReceive", HEX_STR_EXPECTED_CONTENT_LENGTH, Buffer);
    return PROTOCOL_ERROR;
  }

/* Start of payload is after Content-length: XX\r\n*/

  if ((StartOfPayload = strchr(Buffer,'\n')) == NULL) {
    Display(LOG_LEVEL_3, ELError, "tspReceive", HEX_STR_INVALID_RESPONSE_RECEIVED);
    return PROTOCOL_ERROR;
  }

  StartOfPayload++;
  BytesTotal = (int)strlen(StartOfPayload);

  if (((pl->size = atol(Buffer + 15)) <= 0L) || BytesTotal > pl->size) {
    Display(LOG_LEVEL_3, ELError, "tspReceive", HEX_STR_INVALID_PAYLOAD_SIZE);
    return PROTOCOL_ERROR;
  }

  BytesLeft = pl->size - BytesTotal;

  while(BytesLeft) {
    if((BytesRead = nt->netrecv(socket, (StartOfPayload + BytesTotal), BytesLeft)) <= 0) {
      Display(LOG_LEVEL_3, ELError, "tspReceive", HEX_STR_CANT_R_SERVER_SOCKET);
      return(PROTOCOL_EREAD);
    }

    BytesTotal += BytesRead;
    BytesLeft -= BytesRead;
  }

  if((pl->payload = (char *)malloc((pl->size) + 1)) == NULL) {
    Display(LOG_LEVEL_3, ELError, "tspReceive", HEX_STR_MALLOC_ERROR);
    return(PROTOCOL_EMEM);
  }

  memset(pl->payload, 0, sizeof(pl->payload));
  strcpy(pl->payload, StartOfPayload);

  return PROTOCOL_OK;
}
