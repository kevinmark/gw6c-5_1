/*
---------------------------------------------------------------------------
 $Id: tsp_client.c,v 1.75 2007/11/28 17:27:36 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

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
#include <assert.h>

#define _USES_SYS_SOCKET_H_

#include "platform.h"

#include "tsp_client.h"

#include "net_udp.h"
#include "net_rudp.h"
#include "net_rudp6.h"
#include "net_tcp.h"
#include "net_tcp6.h"

#include "net.h"  /* net_tools_t */
#include "config.h" /* tConf  */
#include "tsp_cap.h"  /* tCapability */
#include "tsp_auth.h" /* tspAuthenticate */
#include "tsp_net.h"  /* tPayload */
#include "xml_tun.h"  /* tTunnel, tspXMLParse() */
#include "xml_req.h"  /* build* */
#include "tsp_redirect.h"

#include "version.h"
#include "log.h"
#include "hex_strings.h"
#include "lib.h"    // tspGetErrorByCode()

// The Gateway6 Client Messaging subsystem.
#include <gw6cmessaging/clientmsgdataretriever.h>
#include <gw6cmessaging/clientmsgnotifier.h>
#include <gw6cmessaging/gw6c_c_wrapper.h>

#ifdef HAP6
#include "hap6.h"
#endif

// Global static variables used throughout program.
Gw6cStatusInfo gStatusInfo;       // Declared `extern' in gw6c_c_wrapper.h
Gw6cTunnelInfo gTunnelInfo;       // Declared `extern' in gw6c_c_wrapper.h
char* gszBrokerListFile = NULL;   // Local only. NOT USED
HAP6StatusInfo gHAP6StatusInfo;   // Declared `extern' in gw6c_c_wrapper.h


#define MAX_RETRIES 32768
#define TSP_VERSION_FALLBACK_DELAY 5

char *TspProtocolVersionStrings[] = { CLIENT_VERSION_STRING_2_0_1,
                    CLIENT_VERSION_STRING_2_0_0,
                    CLIENT_VERSION_STRING_1_0_1,
                    0 };

/*
   Call the XML parser. Data will be contained in the
   structure t (struct tTunnel)
*/

int
tspExtractPayload(char *Payload, tTunnel *t)
{
  char *p; /* First byte of payload. */
  int   rc;

  memset(t, 0, sizeof(tTunnel));

  Display(LOG_LEVEL_3, ELInfo, "tspExtractPayload", HEX_STR_PROCESS_SERVER_RESPONSE);
  if((p = strchr(strchr(Payload, '\n'), '<')) == NULL)
    return 1;
  if((rc = tspXMLParse(p, t)) != 0)
    return 1;

  return(0);
}

/* */

void tspClearPayload(tPayload *Payload)
{
  if (Payload->payload) {
    free(Payload->payload);
  }
  memset(Payload, 0, sizeof(tPayload));
}

/* */

int tspGetStatusCode(char *payload)
{
  if (payload)
    return atoi(payload);
  return(0);
}

/* */

char *tspGetStatusMsg(char *payload)
{
  static char Msg[1024], *ptr;

  if (!payload)
    return("");

  memset(Msg, 0, sizeof(Msg));

  if ((ptr = strchr(payload, '\n')) != NULL) {
    if (ptr - payload > sizeof Msg)
      ptr = payload + sizeof Msg;
    memcpy(Msg, payload, (int)(ptr-payload));
  } else {
    if ((ptr = strchr(payload, ' ')) != NULL) {
      snprintf(Msg, sizeof Msg, "%s", ptr+1);
    } else {
      snprintf(Msg, sizeof Msg, "%s", payload);
    }
  }

  return(Msg);
}

/* */

char
*tspAddPayloadString(tPayload *Payload, char *Addition)
{
  char *NewPayload;

  if(Addition) {
    if(Payload->PayloadCapacity == 0) {
      if((NewPayload = Payload->payload = (char *)malloc(PROTOCOLMAXPAYLOADCHUNK)) == NULL) {
        Display(LOG_LEVEL_3, ELError, "tspAddPayloadString", HEX_STR_CANT_MALLOC_FOR_PAYLOAD);
        return NULL;
      }
      *Payload->payload = 0;
      Payload->PayloadCapacity = PROTOCOLMAXPAYLOADCHUNK;
    }

    if((Payload->size + (long)strlen(Addition) + 1) > Payload->PayloadCapacity) {
      Payload->PayloadCapacity += PROTOCOLMAXPAYLOADCHUNK;
      if((NewPayload = (char *) malloc(Payload->PayloadCapacity)) == NULL) {
        Display(LOG_LEVEL_3, ELError, "tspAddPayloadString", HEX_STR_CANT_MALLOC_FOR_PAYLOAD);
        return NULL;
      }

      memcpy(NewPayload, Payload->payload, Payload->size + 1);
      free(Payload->payload);
      Payload->payload = NewPayload;
    }

    strcat(Payload->payload, Addition);
    Payload->size += (long)strlen(Addition);
  }

  return Payload->payload;
}


// --------------------------------------------------------------------------
// Attempts to negotiate a tunnel with the broker.
//
int tspSetupTunnel(tConf *conf, net_tools_t *nt[], int version_index, tBrokerList **broker_list)
{
  SOCKET socket;
  int status = 0, ret = 0;
  tPayload plin, plout;
  tCapability cap;
  tTunnel t;


  // -----------------------------------------------------------
  // Send(update) connectivity status to GUI.
  // -----------------------------------------------------------
  gStatusInfo.eStatus = GW6C_CLISTAT__CONNECTING;
  gStatusInfo.nStatus = GW6CM_UIS__NOERROR;
  send_status_info();


  /* we have an index of the transport to use into the net_tools_t array */
  if (conf->transport == NET_TOOLS_T_TCP)
    Display( LOG_LEVEL_2, ELInfo, "tspSetupTunnel", HEX_STR_CONNECT_TCP, conf->server);
  else if (conf->transport == NET_TOOLS_T_RUDP)
    Display( LOG_LEVEL_2, ELInfo, "tspSetupTunnel", HEX_STR_CONNECT_RUDP, conf->server);
#ifdef V4V6_SUPPORT
  else if (conf->transport == NET_TOOLS_T_TCP6)
    Display( LOG_LEVEL_2, ELInfo, "tspSetupTunnel", HEX_STR_CONNECT_TCPV6, conf->server);
  else if (conf->transport == NET_TOOLS_T_RUDP6)
    Display( LOG_LEVEL_2, ELInfo, "tspSetupTunnel", HEX_STR_CONNECT_RUDPV6, conf->server);
#endif /* V4V6_SUPPORT */

  // ----------------------------------------------------------------------
  // Perform connection to the server using a specific transport protocol.
  // ----------------------------------------------------------------------
  if( (socket = tspConnect(conf->server, nt[conf->transport])) == -1 )
  {
    Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", HEX_STR_CANT_ESTABLISH_CONNECTION, conf->server);
    tspClose(socket, nt[conf->transport]);

    /* Return a more specific SOCKET_ERROR indicating that we could not connect */
    return SOCKET_ERROR_CANT_CONNECT;
  }


  // --------------------------------------
  // Get the capabilities from the server.
  // --------------------------------------
  Display(LOG_LEVEL_3, ELInfo, "tspSetupTunnel", HEX_STR_GETTING_CAPS_FROM_SERVER);
  ret = tspGetCapabilities(socket, nt[conf->transport], &cap, version_index, conf, broker_list);
  if (ret != NO_ERROR)
  {
    if ((ret != BROKER_REDIRECTION) && (ret != BROKER_REDIRECTION_ERROR))
      Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", HEX_STR_TSPGETCAPABILITIES_ERROR, ret, tspGetErrorByCode(ret));
    if (ret == SOCKET_ERROR)
      Display(LOG_LEVEL_1, ELError, "tspSetupTunnel", HEX_STR_USING_UDP_NO_LISTENER, conf->server);
    tspClose(socket, nt[conf->transport]);

    if ((ret == SOCKET_ERROR) && ((conf->transport == NET_TOOLS_T_RUDP)
                  || (conf->transport == NET_TOOLS_T_UDP)
#ifdef V4V6_SUPPORT
                  || (conf->transport == NET_TOOLS_T_RUDP6)
#endif
                  ))
    {
      /* Return a more specific SOCKET_ERROR indicating that we could not connect */
      /* UDP connection will fail here instead of at tspConnect */
      return SOCKET_ERROR_CANT_CONNECT;
    }
    else
    {
      return ret;
    }
  }
  Display(LOG_LEVEL_1, ELInfo, "tspSetupTunnel", HEX_STR_CONNECTED, conf->server);


  // --------------------------------------------------------------
  // Verify if the requested tunnel mode is offered on the server.
  // --------------------------------------------------------------
  if ((conf->tunnel_mode & cap) == 0)
  {
    /* tunnel mode not supported on server */
    Display(LOG_LEVEL_3, ELError,"tspSetupTunnel", HEX_STR_TUN_MODE_NOT_SUPPORTED, conf->server);
    tspClose(socket, nt[conf->transport]);
    return TSP_ERROR;
  }


  // --------------------------------------
  // Perform authentication on the server.
  // --------------------------------------
  Display(LOG_LEVEL_3, ELInfo, "tspSetupTunnel", HEX_STR_AUTHENTICATING, conf->userid);
  ret = tspAuthenticate(socket, cap, nt[conf->transport], conf, broker_list, version_index);
  if (ret != 0) {
    if ((ret != BROKER_REDIRECTION) && (ret != BROKER_REDIRECTION_ERROR))
      Display(LOG_LEVEL_1, ELError,"tspSetupTunnel", HEX_STR_AUTH_ERROR);
    tspClose(socket, nt[conf->transport]);
    if ((ret != BROKER_REDIRECTION) && (ret != BROKER_REDIRECTION_ERROR))
      return AUTHENTICATION_ERROR;
    else
      return ret;
  }
  Display(LOG_LEVEL_2, ELInfo, "tspSetupTunnel", HEX_STR_AUTH_SUCCESS);


  /* update source addresses */
  if (tspUpdateSourceAddr(conf, socket) != 0)
  {
    Display(LOG_LEVEL_3, ELError, "tspSetupTunnel", HEX_STR_CANT_GET_SRC_ADDRESS);
    tspClose(socket, nt[conf->transport]);
    return SOCKET_ERROR;
  }

  /* prepare request */
  memset(&plin, 0, sizeof(plin));
  memset(&plout, 0, sizeof(plout));

  plin.payload = tspAddPayloadString(&plin, tspBuildCreateRequest(conf));

  /* get some mem for the answer */

  plout.payload = (char *)malloc(REDIRECT_RECEIVE_BUFFER_SIZE);
  plout.size = REDIRECT_RECEIVE_BUFFER_SIZE;

        /* send it */
  memset(plout.payload, 0, plout.size);
  ret = tspSendRecv(socket, &plin, &plout, nt[conf->transport]);
  if (ret < 0)
  {
    Display(LOG_LEVEL_3, ELError, "tspTryServer", HEX_STR_CANT_SEND_REQUEST_TO, conf->server);
    tspClose(socket, nt[conf->transport]);
    return SOCKET_ERROR;
  }
  if (ret == 0)
  {
    Display(LOG_LEVEL_3, ELError, "tspTryServer", HEX_STR_SERVER_DISCONNECT_NEGO, conf->server);
    tspClose(socket, nt[conf->transport]);
    return SOCKET_ERROR;
  }


  /* process it */

  status = tspGetStatusCode(plout.payload);

  if (tspIsRedirectStatus(status)) {
    tspClose(socket, nt[conf->transport]);

    if (tspHandleRedirect(plout.payload, conf, broker_list) == TSP_REDIRECT_OK) {
      free(plout.payload);
      plout.size = 0;

      free(plin.payload);
      plin.size = 0;

      return BROKER_REDIRECTION;
    }
    else {
      free(plout.payload);
      plout.size = 0;

      free(plin.payload);
      plin.size = 0;

      return BROKER_REDIRECTION_ERROR;
    }
  }
  else if (status != 200) {
    Display(LOG_LEVEL_1, ELError, "tspTryServer", HEX_STR_STATUS_ERROR_IN_NEGO, status, &plout.payload[1]);
    tspClose(socket, nt[conf->transport]);
    if (status == 310) /* server side error */
      status = SERVER_SIDE_ERROR;
    return status;
  }

  // Extract the tunnel information from the XML payload.
  tspExtractPayload(plout.payload, &t);

  // free some memory
  free(plout.payload);
  plout.size = 0;

  free(plin.payload);
  plin.size=0;

  /* version 1.0.1 requires that we immediatly jump in tunnel mode */
  if (version_index == CLIENT_VERSION_INDEX_1_0_1)
    goto start_show;

  /* and acknowledge it */
  memset(&plin, 0, sizeof(plin));
  plin.payload = tspAddPayloadString(&plin, tspBuildCreateAcknowledge());

  if ( tspSend(socket, &plin, nt[conf->transport]) == -1 ){
    Display(LOG_LEVEL_3, ELError, "tspTryServer", HEX_STR_ERR_IN_TUN_ACK);
    tspClose(socket, nt[conf->transport]);
    return SOCKET_ERROR;
  }

  // free the last of the memory
  free(plin.payload);
  plin.size=0;


start_show:

  Display(LOG_LEVEL_2, ELInfo, "tspSetupTunnel", HEX_STR_GOT_TUNNEL_PARAMS_SETTING_UP);

  // -----------------------------------------------------------------
  // Save the current server(broker) to the last-tsp-server.txt file.
  // -----------------------------------------------------------------
  if( tspWriteLastServerToFile(conf->last_server, conf->server) != TSP_REDIRECT_OK )
  {
    Display(LOG_LEVEL_2, ELError, "tspSetupTunnel", HEX_STR_RDR_CANT_WRITE_LAST_SERVER, conf->server, conf->last_server);
  }


  // --------------------------------------------------------
  // Run tunnel script and maintain keep-alive (if enabled).
  // --------------------------------------------------------
  ret = tspStartLocal(socket, conf, &t, nt[conf->transport]);


  // -------------------------------------------
  // Clear tunnel information and close socket.
  // -------------------------------------------
  free( gTunnelInfo.szDelegatedPrefix );
  memset( &gTunnelInfo, 0x00, sizeof(struct __TUNNEL_INFO) );
  tspClearTunnelInfo( &t );
  tspClose(socket, nt[conf->transport]);

  return ret;
}


// --------------------------------------------------------------------------
// Function : RetrieveStatusInfo
//
// Description:
//   Will set the status info in ppStatusInfo.
//
// Arguments:
//   ppStatusInfo: Gw6cStatusInfo** [IN,OUT], The status info.
//
// Return values:
//   GW6CM_UIS__NOERROR: Successfully retrieved status info and populated the
//                       Status Info object.
//
// --------------------------------------------------------------------------
error_t RetrieveStatusInfo( Gw6cStatusInfo** ppStatusInfo )
{
  assert( *ppStatusInfo == NULL );

  // No allocation is made, really, we're just assigning the global variable.
  *ppStatusInfo = &gStatusInfo;

  return GW6CM_UIS__NOERROR;
}

// --------------------------------------------------------------------------
// Function : RetrieveTunnelInfo
//
// Description:
//   Will set the tunnel info in ppTunnelInfo.
//
// Arguments:
//   ppTunnelInfo: Gw6cTunnelInfo** [IN,OUT], The tunnel info.
//
// Return values:
//   GW6CM_UIS__NOERROR: Successfully retrieved tunnel info and populated the
//                       Tunnel Info object.
//
// --------------------------------------------------------------------------
error_t RetrieveTunnelInfo( Gw6cTunnelInfo** ppTunnelInfo )
{
  assert( *ppTunnelInfo == NULL );

  // No allocation is made, really, we're just assigning the global variable.
  *ppTunnelInfo = &gTunnelInfo;

  return GW6CM_UIS__NOERROR;
}

// --------------------------------------------------------------------------
// Function : RetrieveBrokerList
//
// Description:
//   Will set the broker list in ppBrokerList.
//
// Arguments:
//   ppBrokerList: Gw6cBrokerList** [IN,OUT], The tunnel info.
//
// Return values:
//   GW6CM_UIS__NOERROR: Successfully retrieved broker list and populated the
//                       Broker List object.
//   GW6CM_UIS_FAILEDBROKERLISTEXTRACTION: Failed broker list extraction.
//
// --------------------------------------------------------------------------
error_t RetrieveBrokerList( Gw6cBrokerList** ppBrokerList )
{
  tBrokerList* tspBrokerList = NULL;// Local format of broker list
  Gw6cBrokerList* pList;            // Intermediate var
  assert( *ppBrokerList == NULL );  // Allocation is made here.


  // Read broker list from file.
  //
  if( gszBrokerListFile != NULL  &&  tspReadBrokerListFromFile( gszBrokerListFile,
                                       &tspBrokerList ) != TSP_REDIRECT_OK )
  {
    // Failed to extract broker list.
    return GW6CM_UIS_FAILEDBROKERLISTEXTRACTION;
  }


  // Convert contents.
  if( tspBrokerList != NULL )
  {
    *ppBrokerList = pList = (Gw6cBrokerList*)malloc( sizeof(Gw6cBrokerList) );


    // Copy the contents of the broker list struct to the
    // messaging subsystem broker list format.
    //
    while( tspBrokerList != NULL )
    {
      pList->szAddress = strdup(tspBrokerList->address);
      pList->nDistance = tspBrokerList->distance;

      if( (tspBrokerList = tspBrokerList->next) != NULL )
        pList = (pList->next = (Gw6cBrokerList*)malloc( sizeof(Gw6cBrokerList) ));
    }
  }

  return GW6CM_UIS__NOERROR;
}

// --------------------------------------------------------------------------
// Function : RetrieveHAP6StatusInfo
//
// Description:
//   Will set the global HAP6 Status Info object in ppHAP6StatusInfo.
//
// Arguments:
//   ppHAP6StatusInfo: HAP6StatusInfo** [IN,OUT], The HAP6 Status info
//                     returned to the GUI.
//
// Return values:
//   GW6CM_UIS__NOERROR: Successfully retrieved the HAP6 status info.
//
// --------------------------------------------------------------------------
error_t RetrieveHAP6StatusInfo( HAP6StatusInfo** ppHAP6StatusInfo )
{
#ifdef HAP6

  HAP6StatusInfo *hap6_status_info_copy = NULL;
  hap6_status status = HAP6_STATUS_OK;

  hap6_status_info_copy = (HAP6StatusInfo *)malloc(sizeof(HAP6StatusInfo));

  if (hap6_status_info_copy == NULL)
  {
    return GW6CM_UIS_MEMERROR;
  }

  /* Ask the HAP6 module for a copy of the current status information. */
  status = hap6_messaging_get_status_info(&hap6_status_info_copy);

  if (status != HAP6_STATUS_OK)
  {
    FreeHAP6StatusInfo(&hap6_status_info_copy);

    return GW6CM_UIS_UNKNOWNERROR;
  }

  *ppHAP6StatusInfo = hap6_status_info_copy;

  return GW6CM_UIS__NOERROR;
#else
  /* Not supposed to happen if HAP6 is not enabled */
  return GW6CM_UIS_UNKNOWNERROR;
#endif
}

// --------------------------------------------------------------------------
void FreeStatusInfo( Gw6cStatusInfo** ppStatusInfo )
{
  if( *ppStatusInfo != NULL )
  {
    // Nothing is really freed, because we're using the global variable.
    *ppStatusInfo = NULL;
  }
}

// --------------------------------------------------------------------------
void FreeTunnelInfo( Gw6cTunnelInfo** ppTunnelInfo )
{
  if( *ppTunnelInfo != NULL )
  {
    // Nothing is really freed, because we're using the global variable.
    *ppTunnelInfo = NULL;
  }
}

// --------------------------------------------------------------------------
void FreeBrokerList( Gw6cBrokerList** ppBrokerList )
{
  Gw6cBrokerList* pList = *ppBrokerList;

  while( *ppBrokerList != NULL )
  {
    pList = *ppBrokerList;  // Keep reference for free()
    free( pList->szAddress );
    *ppBrokerList = pList->next;
    free( pList );
  }
}

// --------------------------------------------------------------------------
void FreeHAP6StatusInfo( HAP6StatusInfo** ppHAP6StatusInfo )
{
  PMAPPING_STATUS pList = (*ppHAP6StatusInfo)->hap6_devmap_statuses;

  while ( (*ppHAP6StatusInfo)->hap6_devmap_statuses != NULL)
  {
    pList = (*ppHAP6StatusInfo)->hap6_devmap_statuses;
    free( pList->device_name );
    (*ppHAP6StatusInfo)->hap6_devmap_statuses = pList->next;
    free( pList );
  }

  free(*ppHAP6StatusInfo);

  *ppHAP6StatusInfo = NULL;
}

// --------------------------------------------------------------------------
// Function : NotifyHap6ConfigInfo
//
// Description:
//   CALLBACK function from the Messaging Subsystem upon reception of a
//   HAP6 Config Info message.
//
// Arguments:
//   aHAP6ConfigInfo: HAP6ConfigInfo* [IN], The HAP6 Config Info from the GUI.
//
// Return values:
//   GW6CM_UIS__NOERROR: Successfully processed the HAP6 Config info.
//
// --------------------------------------------------------------------------
error_t NotifyHap6ConfigInfo( const HAP6ConfigInfo* aHAP6ConfigInfo )
{
  return GW6CM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function: FormatBrokerListAddr
//
// Description: Copies a tBrokerList element address to a destination 
//              address. If the address is IPv6, a set of brackets are put
//              at the beginning and end.
// i.e.:  
//        1.2.3.4            stays the same
//        broker.domain.com  stays the same
//        2001:BABA::0045    becomes [2001:BABA::0045]
//
// Arguments:
//   listElement: An element of the broker redirection list.
//   ppAddr: A pointer to a char array.
//
// Returns 0 on success, 1 on error.
//
int FormatBrokerListAddr( tBrokerList* listElement, char **ppAddr )
{
  if( ppAddr == NULL )
  {
    // Invalid pointer.
    Display(LOG_LEVEL_1, ELError, "FormatBrokerListAddr", HEX_STR_INVALID_POINTER);
    return 1;
  }

  // Check if there's stuff pointed by ppAddr. Free it if so.
  if( *ppAddr != NULL )
  {
    free( *ppAddr );
    *ppAddr = NULL;
  }

  // Copy address.
  if( listElement->address_type == TSP_REDIRECT_BROKER_TYPE_IPV6 )
  {
    size_t len = strlen(listElement->address) + 3;

    // Must put address between brackets ([]).
    *ppAddr = (char*)malloc( len );
    if( *ppAddr == NULL )
    {
      // Memory allocation error
      Display(LOG_LEVEL_1, ELError, "FormatBrokerListAddr", HEX_STR_MALLOC_ERROR);
      return 1;
    }

    snprintf( *ppAddr, len, "[%s]", listElement->address );
  }
  else
  {
    // Not an IPv6 address, just strdup' it.
    *ppAddr = strdup(listElement->address);
  }

  return 0;
}


// --------------------------------------------------------------------------
// Gateway6 Client TSP main entry point.
// Called from every platform main() or service_main().
//
int tspMain(int argc, char *argv[])
{
  int status, i;
  tConf c;
  net_tools_t **nt;
  int version_index = CLIENT_VERSION_INDEX_CURRENT;
  int connected = 1; /* try servers as long as connected is true */
  int cycle = 0; /* reconnect and fallback cycle */
  int tsp_version_fallback = 0; /* true if the TSP protocol version needs to fall back for the next retry */
  tLogConfiguration *log_configuration = NULL;
  int log_configuration_error = 0;
  int dump_logs_to_file = 0;
  int quick_cycle = 0;
  tBrokerList *broker_list = NULL;
  tBrokerList *current_broker_in_list = NULL;
  int trying_original_server = 0;
  int trying_broker_list = 0;
  int read_last_server = 0;
  char original_server[MAX_REDIRECT_ADDRESS_LENGTH];
  char last_server[MAX_REDIRECT_ADDRESS_LENGTH];
  tRedirectStatus last_server_status = TSP_REDIRECT_OK;
  tRedirectStatus broker_list_status = TSP_REDIRECT_OK;



  // -------------------------------------------------------------------
  // Get the required memory for the net tools structure and initialize
  // it with NULLs.
  // -------------------------------------------------------------------
  nt =  malloc(sizeof(net_tools_t *) * NET_TOOLS_T_SIZE );

  for (i = 0; i < NET_TOOLS_T_SIZE; i++) {
    nt[i] = (net_tools_t *) malloc (sizeof (net_tools_t) );
    memset(nt[i], 0, sizeof(net_tools_t));
  }

  // ---------------------------------------------------------
  // Fill up the array. The correct index to use is in tConf.
  // ---------------------------------------------------------
  nt[NET_TOOLS_T_RUDP]->netopen = NetRUDPConnect;
  nt[NET_TOOLS_T_RUDP]->netclose = NetRUDPClose;
  nt[NET_TOOLS_T_RUDP]->netsendrecv = NetRUDPReadWrite;
  nt[NET_TOOLS_T_RUDP]->netsend = NetRUDPWrite;
  nt[NET_TOOLS_T_RUDP]->netprintf = NetRUDPPrintf;
  nt[NET_TOOLS_T_RUDP]->netrecv = NetRUDPRead;

  nt[NET_TOOLS_T_UDP]->netopen = NetUDPConnect;
  nt[NET_TOOLS_T_UDP]->netclose = NetUDPClose;
  nt[NET_TOOLS_T_UDP]->netsendrecv = NetUDPReadWrite;
  nt[NET_TOOLS_T_UDP]->netsend = NetUDPWrite;
  nt[NET_TOOLS_T_UDP]->netprintf = NetUDPPrintf;
  nt[NET_TOOLS_T_UDP]->netrecv = NetUDPRead;

  nt[NET_TOOLS_T_TCP]->netopen = NetTCPConnect;
  nt[NET_TOOLS_T_TCP]->netclose = NetTCPClose;
  nt[NET_TOOLS_T_TCP]->netsendrecv = NetTCPReadWrite;
  nt[NET_TOOLS_T_TCP]->netsend = NetTCPWrite;
  nt[NET_TOOLS_T_TCP]->netprintf = NetTCPPrintf;
  nt[NET_TOOLS_T_TCP]->netrecv = NetTCPRead;

#ifdef V4V6_SUPPORT
  nt[NET_TOOLS_T_TCP6]->netopen = NetTCP6Connect;
  nt[NET_TOOLS_T_TCP6]->netclose = NetTCP6Close;
  nt[NET_TOOLS_T_TCP6]->netsendrecv = NetTCP6ReadWrite;
  nt[NET_TOOLS_T_TCP6]->netsend = NetTCP6Write;
  nt[NET_TOOLS_T_TCP6]->netprintf = NetTCP6Printf;
  nt[NET_TOOLS_T_TCP6]->netrecv = NetTCP6Read;

  nt[NET_TOOLS_T_RUDP6]->netopen = NetRUDP6Connect;
  nt[NET_TOOLS_T_RUDP6]->netclose = NetRUDP6Close;
  nt[NET_TOOLS_T_RUDP6]->netsendrecv = NetRUDP6ReadWrite;
  nt[NET_TOOLS_T_RUDP6]->netsend = NetRUDP6Write;
  nt[NET_TOOLS_T_RUDP6]->netprintf = NetRUDP6Printf;
  nt[NET_TOOLS_T_RUDP6]->netrecv = NetRUDP6Read;
#endif /* V4V6_SUPPORT */


  // ------------------------
  // Initialize status info.
  // ------------------------
  gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDIDLE;
  gStatusInfo.nStatus = GW6CM_UIS__NOERROR;


  // ------------------------------------------------------------------------
  // Zero-memory the configuration object, because tspInitialize requires
  // it initialized. Then call the tspInitialize function.
  // ------------------------------------------------------------------------
  memset( &c, 0, sizeof(c) );

  if ((status = tspInitialize(argc, argv, &c)) != NO_ERROR) {
    if( status != NO_ERROR_SHOW_HELP ) {
      dump_logs_to_file = 1;
    }

    connected = 0;
    gStatusInfo.nStatus = GW6CM_UIS_CFGDATAERROR;

    goto endtspc;
  }

  gszBrokerListFile = c.broker_list; // For BROKER_LIST message.


  // ---------------------------------------------------------
  // Allocate memory for the logging configuration structure.
  // ---------------------------------------------------------
  log_configuration = (tLogConfiguration *)malloc(sizeof(tLogConfiguration));
  if (log_configuration == NULL)
  {
    DirectErrorMessage(HEX_STR_COULD_NOT_MALLOC_FOR_CONFIG);

    log_configuration_error = 1;
    status = MEMORY_ERROR;

    gStatusInfo.nStatus = GW6CM_UIS_MEMERROR;
    connected = 0;
    goto endtspc;
  }

  /* Fill the logging configuration structure with the values parsed */
  /* from the configuration. It is possible that some of those values */
  /* are default values that were automatically set by the client code */
  /* if the user did not specify alternative values. */
  log_configuration->identity = strdup(LOG_IDENTITY);
  log_configuration->log_filename = strdup(c.log_filename);
  log_configuration->log_level_stderr = c.log_level_stderr;
  log_configuration->log_level_console = c.log_level_console;
  log_configuration->log_level_syslog = c.log_level_syslog;
  log_configuration->log_level_file = c.log_level_file;
  log_configuration->syslog_facility = c.syslog_facility;
  log_configuration->log_rotation = c.log_rotation;
  log_configuration->log_rotation_size = c.log_rotation_size;
  log_configuration->delete_rotated_log = c.log_rotation_delete;
  log_configuration->buffer = 0;

  /* Configure the logging system with the values provided above. */
  if( LogConfigure(log_configuration) != 0 ) {
    DirectErrorMessage(HEX_STR_COULD_NOT_CONFIGURE_LOGGING);

    log_configuration_error = 1;
    status = LOGGING_CONFIGURATION_ERROR;
    gStatusInfo.nStatus = GW6CM_UIS_CFGDATAERROR;

    connected = 0;
    goto endtspc;
  }

  {
    char bufOSInfo[256];

    // Display Gateway6 Client version and build option(s).
    Display( LOG_LEVEL_1, ELInfo, "tspMain", "%s", tsp_get_version() );

    // Display OS specific information. Handy for bug reporting.
    tspGetOSInfo( sizeof(bufOSInfo), bufOSInfo );  // in tsp_local.c
    Display( LOG_LEVEL_1, ELInfo, "tspMain", "%s", bufOSInfo );
  }

  /* Save the original server value */
  strcpy(original_server, c.server);

  /* If always_use_same_server is enabled */
  if ((c.always_use_same_server == TRUE) && (strlen(c.last_server) > 0)) {
    /* Try to get the last server from the last_server file */
    last_server_status = tspReadLastServerFromFile(c.last_server, last_server);

    /* If it was successful */
    if (last_server_status == TSP_REDIRECT_OK) {
      /* Replace the configuration file's server value with the last server */
      free(c.server);
      c.server = strdup(last_server);
      /* We found the last server */
      read_last_server = 1;
      /* We're not trying the original server */
      trying_original_server = 0;
      Display(LOG_LEVEL_2, ELInfo, "tspMain", HEX_STR_RDR_TRYING_LAST_SERVER, last_server);
    }
    /* Otherwise if we could not find the last server */
    else if (last_server_status == TSP_REDIRECT_NO_LAST_SERVER) {
      /* Try the original server instead */
      trying_original_server = 1;
      Display(LOG_LEVEL_2, ELInfo, "tspMain", HEX_STR_RDR_NO_LAST_SERVER_FOUND, c.last_server, original_server);
    }
    /* Or if we could not open the last_server file */
    else if (last_server_status == TSP_REDIRECT_CANT_OPEN_FILE) {
      /* Try the original server instead */
      trying_original_server = 1;
      Display(LOG_LEVEL_2, ELInfo, "tspMain", HEX_STR_RDR_CANT_OPEN_LAST_SERVER, c.last_server, original_server);
    }
    /* Otherwise there's some other kind of error reading the last server */
    else
    {
      Display(LOG_LEVEL_1, ELError, "tspMain", HEX_STR_RDR_ERROR_READING_LAST_SERVER, c.last_server);
      status = BROKER_REDIRECTION_ERROR;
      gStatusInfo.nStatus = GW6CM_UIS_REDIRECTIONERROR;
      connected = 0;
      goto endtspc;
    }
  }
  /* If always_use_same_server is disabled */
  else {
    /* Try the original server */
    trying_original_server = 1;
  }


  // ------------------------------------------------------------------------
  // Connection loop.
  //   Perform loop until we give up (i.e.: an error is indicated), or user
  //   requested a stop in the service (HUP signal or service stop).
  //
  while( connected  &&  tspCheckForStopOrWait(0) == 0 )
  {
    quick_cycle = 0;

    // While we loop in this while(), keep everything updated on our status.
    //
    if( gStatusInfo.eStatus != GW6C_CLISTAT__DISCONNECTEDIDLE &&
        gStatusInfo.nStatus != GW6CM_UIS__NOERROR )
    {
      // Status has been changed.
      send_status_info();
    }

    // Choose the transport or cycle thru the list
    switch(c.tunnel_mode)
    {
      case V6UDPV4:
      switch (cycle)
      {
        default:
        cycle = 0;
        case 0:
          if (tsp_version_fallback) {
            if (version_index < CLIENT_VERSION_INDEX_V6UDPV4_START) {
              version_index++;
            }
          tsp_version_fallback = 0;
          }
          else {
            version_index = CLIENT_VERSION_INDEX_CURRENT;
          }
          c.transport = NET_TOOLS_T_RUDP;
        break;
      }
      break;

      case V6ANYV4:
      case V6V4:
      switch (cycle)
      {
        default:
        cycle = 0;  /* this will catch an overflow of the cycle counts and reset it */
        case 0:
        if (tsp_version_fallback) {
          if (version_index < CLIENT_VERSION_INDEX_OLDEST) {
          version_index++;
          }
          tsp_version_fallback = 0;
        }
        else {
          version_index = CLIENT_VERSION_INDEX_CURRENT;
        }

        c.transport = NET_TOOLS_T_RUDP;
        break;

        case 1:
        if (tsp_version_fallback) {
          if (version_index < CLIENT_VERSION_INDEX_OLDEST) {
            version_index++;
          }
          tsp_version_fallback = 0;
        }
        else {
          version_index = CLIENT_VERSION_INDEX_CURRENT;
        }
        c.transport = NET_TOOLS_T_TCP;
        break;
      }
      break;

#ifdef V4V6_SUPPORT
      case V4V6:
      switch (cycle)
      {
        default:
        cycle = 0;
        case 0:
        if (tsp_version_fallback) {
          if (version_index < CLIENT_VERSION_INDEX_V4V6_START) {
          version_index++;
          }
          tsp_version_fallback = 0;
        }
        else {
          version_index = CLIENT_VERSION_INDEX_CURRENT;
        }
        c.transport = NET_TOOLS_T_RUDP6;
        break;

        case 1:
        if (tsp_version_fallback) {
          if (version_index < CLIENT_VERSION_INDEX_V4V6_START) {
          version_index++;
          }
        tsp_version_fallback = 0;
        }
        else {
          version_index = CLIENT_VERSION_INDEX_CURRENT;
        }
        c.transport = NET_TOOLS_T_TCP6;
        break;
        }
      break;
#endif

      default:
      break;
    } // switch(c.tunnel_mode)


    // Determine if we need to sleep between connection attempts.
    if (((c.tunnel_mode == V6ANYV4) || (c.tunnel_mode == V6V4)) && (c.transport == NET_TOOLS_T_RUDP)) {
      quick_cycle = 1;
    }
#ifdef V4V6_SUPPORT
    if ((c.tunnel_mode == V4V6) && (c.transport == NET_TOOLS_T_RUDP6)) {
      quick_cycle = 1;
    }
#endif

    Display( LOG_LEVEL_3, ELInfo, "tspMain", HEX_STR_USING_TSP_PROTO_VER,
    TspProtocolVersionStrings[version_index]);


    // -----------------------------------------------
    // *** Attempt to negotiate tunnel with broker ***
    // -----------------------------------------------
    status = tspSetupTunnel(&c, nt, version_index, &broker_list);


    // If we are here with no error, we can assume we are done.
    //
    if( status == NO_ERROR )
    {
      gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDIDLE;
      gStatusInfo.nStatus = GW6CM_UIS__NOERROR;

      connected = 0;
      continue;
    }

    // If the error is TSP_ERROR, try another connection mecanism.
    //
    else if( status == TSP_ERROR )
    {
      cycle++;

      gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
      gStatusInfo.nStatus = GW6CM_UIS_TSPERROR;

      if (quick_cycle) {
        Display (LOG_LEVEL_1, ELInfo, "tspMain", HEX_STR_DISCONNECTED_RETRY_NOW);
        continue;
      }
    }

    // Likewise for a SOCKET_ERROR, try another connection mecanism.
    //
    else if (status == SOCKET_ERROR) {
      cycle++;

      gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
      gStatusInfo.nStatus = GW6CM_UIS_SOCKETERROR;

      if (quick_cycle) {
        Display (LOG_LEVEL_1, ELInfo, "tspMain", HEX_STR_DISCONNECTED_RETRY_NOW);
        continue;
      }
    }

    /* This means we could not connect to a server */
    else if (status == SOCKET_ERROR_CANT_CONNECT)
    {
      /* Do this only if we have tried all transports */
      if (!quick_cycle)
      {
        // Status update.
        gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
        gStatusInfo.nStatus = GW6CM_UIS_SOCKETERRORCANTCONNECT;

        /* If we have the last server */
        if (read_last_server == 1) {
          /* Just cycle transports, try again with the last server */
          cycle++;
        }
        /* If we don't have the last server */
        else
        {
          /* If we're trying to connect to the original server */
          if (trying_original_server == 1) {
            /* Clear the broker list */
            tspFreeBrokerList(broker_list);
            broker_list = NULL;

            /* If a broker_list file is specified, try to create the list */
            if (strlen(c.broker_list) > 0)
            {
              Display (LOG_LEVEL_2, ELInfo, "tspMain", HEX_STR_RDR_READING_BROKER_LIST, c.broker_list);

              broker_list_status = tspReadBrokerListFromFile(c.broker_list, &broker_list);

              /* If we could create the list successfully */
              if (broker_list_status == TSP_REDIRECT_OK) {
                /* If the broker list is empty */
                if (broker_list == NULL) {
                  Display (LOG_LEVEL_2, ELInfo, "tspMain", HEX_STR_RDR_READ_BROKER_LIST_EMPTY);
                  /* Just cycle transports, we'll try the original server again */
                  cycle++;
                }
                /* If the broker list is not empty */
                else {
                  Display (LOG_LEVEL_2, ELInfo, "tspMain", HEX_STR_RDR_READ_BROKER_LIST_CREATED);

                  tspLogRedirectionList(broker_list, 0);

                  /* We're going through a broker list */
                  trying_broker_list = 1;
                  /* We're not trying the original server anymore */
                  trying_original_server = 0;
                  /* Start with the first broker in the list */
                  current_broker_in_list = broker_list;
                  // Copy the brokerList address to configuration server.
                  if( FormatBrokerListAddr( current_broker_in_list, &(c.server) ) != 0 )
                  {
                    tspFreeBrokerList(broker_list);
                    broker_list = NULL;
                    status = BROKER_REDIRECTION_ERROR;
                    gStatusInfo.nStatus = GW6CM_UIS_REDIRECTIONERROR;
                    connected = 0;
                    goto endtspc;
                  }
                  /* Adjust the transport cycle to start from the first one */
                  cycle = 0;
                  /* Try the first broker in the list right now */
                  continue;
                }
              }
              /* If we can't open the file, maybe it's just not there. */
              /* This is normal if it hasn't been created. */
              else if (broker_list_status == TSP_REDIRECT_CANT_OPEN_FILE) {
                Display (LOG_LEVEL_2, ELInfo, "tspMain", HEX_STR_RDR_CANT_OPEN_BROKER_LIST, c.broker_list);
                cycle++;
                tspFreeBrokerList(broker_list);
                broker_list = NULL;
              }
              /* If there were more brokers in the list than the allowed limit. */
              else if (broker_list_status == TSP_REDIRECT_TOO_MANY_BROKERS) {
                Display (LOG_LEVEL_2, ELError, "tspMain", HEX_STR_RDR_TOO_MANY_BROKERS, MAX_REDIRECT_BROKERS_IN_LIST);
                cycle++;
                tspFreeBrokerList(broker_list);
                broker_list = NULL;
              }
              /* Otherwise there was a problem creating the list from the broker_list file */
              else {
                Display (LOG_LEVEL_2, ELError, "tspMain", HEX_STR_RDR_ERROR_READING_BROKER_LIST, c.broker_list);
                cycle++;
                tspFreeBrokerList(broker_list);
                broker_list = NULL;
              }
            }
            /* Nothing specified in broker_list */
            /* Cycle transports, but try same server again. */
            else {
              cycle++;
            }
          }
          /* If we're not trying to connect to the original server */
          else {
            /* If we're going through a broker list */
            if (trying_broker_list == 1) {
              /* If the pointers are safe */
              if ((broker_list != NULL) && (current_broker_in_list != NULL)) 
              {
                /* If this is the last broker in the list */
                if (current_broker_in_list->next == NULL) 
                {
                  Display (LOG_LEVEL_2, ELInfo, "tspMain", HEX_STR_RDR_BROKER_LIST_END);

                  /* Prepare to retry the original server after the retry delay */
                  free(c.server);
                  c.server = strdup(original_server);
                  cycle = 0;
                  trying_original_server = 1;
                }
                /* If this is not the last broker in the list */
                else
                {
                  /* Prepare to try the next broker in the list */
                  current_broker_in_list = current_broker_in_list->next;

                  // Copy the brokerList address to configuration server.
                  if( FormatBrokerListAddr( current_broker_in_list, &(c.server) ) != 0 )
                  {
                    tspFreeBrokerList(broker_list);
                    broker_list = NULL;
                    status = BROKER_REDIRECTION_ERROR;
                    gStatusInfo.nStatus = GW6CM_UIS_REDIRECTIONERROR;
                    connected = 0;
                    goto endtspc;
                  }
                  cycle = 0;

                  Display(LOG_LEVEL_2, ELInfo, "tspMain", HEX_STR_RDR_NEXT_IN_BROKER_LIST, current_broker_in_list->address);

                  /* Try the next broker now, don't wait for the retry delay */
                  continue;
                }
              }
              /* If the pointers aren't safe */
              else {
                Display(LOG_LEVEL_1, ELError, "tspMain", HEX_STR_RDR_BROKER_LIST_INTERNAL_ERROR, current_broker_in_list->address);
                gStatusInfo.nStatus = GW6CM_UIS_INTERNALERROR;
                status = BROKER_REDIRECTION_ERROR;

                tspFreeBrokerList(broker_list);
                broker_list = NULL;
                connected = 0;
                continue;
              }
            }
          }
        }
      }
      /* If we haven't tried all transports for this broker, there are more to try */
      else
      {
        cycle++;
        Display (LOG_LEVEL_1, ELInfo, "tspMain", HEX_STR_DISCONNECTED_RETRY_NOW);
        continue;
      }
    }   // status == SOCKET_ERROR_CANT_CONNECT

    /* if setup interface failed,
      quit right away.
    */
    else if (status == INTERFACE_SETUP_FAILED)
    {
      Display(LOG_LEVEL_3, ELError, "tspMain", HEX_STR_RUNNING_TEMPLATE_FAILED, c.template);

      gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
      gStatusInfo.nStatus = GW6CM_UIS_FAILEDSCRIPT;

      connected = 0;
      continue;
    }

#ifdef HAP6
    else if (status == HAP6_SETUP_ERROR) {
      Display(LOG_LEVEL_1, ELError, "tspMain", HAP6_LOG_PREFIX_ERROR HEX_STR_HAP6_ERR_FAILED_TO_SETUP_HAP6_FEATURES);

      gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDHAP6SETUPERROR;
      gStatusInfo.nStatus = GW6CM_UIS_HAP6_SETUP_ERROR;

      connected = 0;
      continue;
    }

    else if (status == HAP6_EXPOSE_DEVICES_ERROR) {
      Display(LOG_LEVEL_1, ELError, "tspMain", HAP6_LOG_PREFIX_ERROR HEX_STR_HAP6_ERR_FAILED_TO_EXPOSE_DEVICES);

      gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDHAP6EXPOSEDEVICESERROR;
      gStatusInfo.nStatus = GW6CM_UIS_HAP6_EXPOSE_DEVICES_ERROR;

      connected = 0;
      continue;
    }
#endif

  /* warn about keepalive timeouts.
  */
  else if( status == KEEPALIVE_TIMEOUT )
  {
    Display(LOG_LEVEL_1, ELError, "tspMain", HEX_STR_KEEPALIVE_TIMEOUT);

    gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
    gStatusInfo.nStatus = GW6CM_UIS_KEEPALIVETIMEOUT;

    if (c.auto_retry_connect == FALSE) {
      gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDNORETRY;
      connected = 0;
      continue;
    }
  }

  /* TUNNEL_ERROR occurs if there is a problem
    during the tunneling with a TUN interface.
  */
  else if( status == TUNNEL_ERROR )
  {
    gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
    gStatusInfo.nStatus = GW6CM_UIS_TUNNELERROR;
  }

  /* TSP_VERSION_ERROR we need to
    try with another version of the protocol.
  */
  else if( status == TSP_VERSION_ERROR )
  {
    tsp_version_fallback = 1;

    gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
    gStatusInfo.nStatus = GW6CM_UIS_TSPVERSIONERROR;

    if (version_index == CLIENT_VERSION_INDEX_2_0_0) {
        cycle = 1; /* Force next cycle to 1 */
      }

    /* Wait a little to prevent the TSP version fallback */
    /* problem with UDP connections that have the same source */
    /* port (see Bugzilla bug #3539. */
    if ((version_index != CLIENT_VERSION_INDEX_2_0_0) && (c.transport == NET_TOOLS_T_RUDP
#ifdef V4V6_SUPPORT
      || c.transport == NET_TOOLS_T_RUDP6
#endif
      )) {
        SLEEP(TSP_VERSION_FALLBACK_DELAY);
      }
    Display (LOG_LEVEL_1, ELInfo, "tspMain", HEX_STR_DISCONNECTED_RETRY_NOW);
    continue;
  }

  /* AUTHENTICATION_ERROR, quit right away.
    This condition is fatal.
  */
  else if (status == AUTHENTICATION_ERROR) {
    gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
    gStatusInfo.nStatus = GW6CM_UIS_AUTHENTICATIONERROR;

    connected = 0;
    continue;
  }

  /* if lease expired, send back tsp request
    right away.
  */
  else if (status == LEASE_EXPIRED) {
    gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
    gStatusInfo.nStatus = GW6CM_UIS_LEASEEXPIRED;

    Display(LOG_LEVEL_3, ELError, "tspMain", HEX_STR_ALLOC_LEASE_EXPIRED);
    continue;
  }

  /* for SERVER_SIDE_ERROR, try another mecanism,
    this error is commonly found on mis-configured
    brokers and changing mecanism might allow a
    tunnel to be created.
  */
  else if (status == SERVER_SIDE_ERROR) {
    gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
    gStatusInfo.nStatus = GW6CM_UIS_SERVERSIDEERROR;

    cycle++;

    if (quick_cycle){
      Display (LOG_LEVEL_1, ELInfo, "tspMain", HEX_STR_DISCONNECTED_RETRY_NOW);
      continue;
    }
  }

  /* This means we got a broker redirection message */
  /* The handling function that sent us this signal has created the broker list */
  else if (status == BROKER_REDIRECTION) {
    gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
    gStatusInfo.nStatus = GW6CM_UIS_BROKERREDIRECTION;

    /* If the list is not empty */
    if (broker_list != NULL) {
      /* We're going through a broker list */
      trying_broker_list = 1;
      /* We're not trying to connect to the original server */
      trying_original_server = 0;
      /* Prepare to try the first broker in the list */
      current_broker_in_list = broker_list;
      // Copy the brokerList address to configuration server.
      if( FormatBrokerListAddr( current_broker_in_list, &(c.server) ) != 0 )
      {
        status = BROKER_REDIRECTION_ERROR;
        gStatusInfo.nStatus = GW6CM_UIS_REDIRECTIONERROR;
        connected = 0;
        goto endtspc;
      }
      cycle = 0;
      /* Try the first broker in the list without waiting for the retry delay */
      continue;
    }
    /* If the list is empty */
    else {
      Display (LOG_LEVEL_1, ELError, "tspMain", HEX_STR_RDR_NULL_LIST);
      status = BROKER_REDIRECTION_ERROR;
      gStatusInfo.nStatus = GW6CM_UIS_REDIRECTIONERROR;
      connected = 0;
      continue;
    }
  }
  /* This means we got a broker redirection message, but */
  /* there were errors handling it */
  else if (status == BROKER_REDIRECTION_ERROR) {
    gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
    gStatusInfo.nStatus = GW6CM_UIS_BROKERREDIRECTION;

    Display (LOG_LEVEL_1, ELError, "tspMain", HEX_STR_RDR_ERROR_PROCESSING_REDIRECTION, c.server);
    gStatusInfo.nStatus = GW6CM_UIS_REDIRECTIONERROR;

    tspFreeBrokerList(broker_list);
    broker_list = NULL;

    connected = 0;
    continue;
  }

  // Any other error, quit immediatly.
  else
  {
    gStatusInfo.eStatus = GW6C_CLISTAT__DISCONNECTEDERROR;
    gStatusInfo.nStatus = GW6CM_UIS_UNKNOWNERROR;

    connected = 0;
    continue;
  }

    // Send status info about the current connection failure.
    //
    send_status_info();

    // Log connection failure & sleep before retrying.
    //
    {
      int sleepTime = c.retry;

      Display(LOG_LEVEL_1, ELInfo, "tspMain", HEX_STR_DISCONNECTED_RETRY, c.retry);

      // Sleep for 'sleepTime' seconds. Check for stop at each second.
      while( sleepTime-- > 0  &&  tspCheckForStopOrWait(1000) == 0 );
    }
  }


endtspc:


  if (status != NO_ERROR) {
    if (status != NO_ERROR_SHOW_HELP) {
      if (log_configuration_error == 0) {
        Display(LOG_LEVEL_3, ELError, "tspMain", HEX_STR_LAST_ERROR, status, tspGetErrorByCode(status));
        Display(LOG_LEVEL_1, ELInfo, "tspMain", HEX_STR_DONE);
      }
      else {
        DirectErrorMessage(HEX_STR_LAST_ERROR, status, tspGetErrorByCode(status));
        DirectErrorMessage(HEX_STR_DONE);
      }
    }
  }
  else {
    if (log_configuration_error == 0)
      Display(LOG_LEVEL_1, ELInfo, "tspMain", HEX_STR_DONE);
    else
      DirectErrorMessage(HEX_STR_DONE);
  }

#ifdef WIN32
  if (dump_logs_to_file == 1)
    DumpBufferToFile(DEFAULT_LOG_FILENAME);
#endif

  // --------------------------
  // Free the net tools array.
  for (i = 0; i < NET_TOOLS_T_SIZE; i++)
    free (nt[i]);
  free(nt);

  // --------------------------
  // Send final status to GUI.
  // --------------------------
  send_status_info();

  /* Free the broker list */
  tspFreeBrokerList(broker_list);

#ifndef WIN32
  LogClose();
#endif
  return(status);
}
