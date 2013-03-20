/*
---------------------------------------------------------------------------
 $Id: tsp_client.h,v 1.17 2007/11/28 17:27:07 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _TSP_CLIENT_H_
#define _TSP_CLIENT_H_

#include "config.h" /* tConf */
#include "xml_tun.h"  /* tTunnel , tspXMLParse() */
#include "tsp_net.h"  /* tPayload */
#include "net.h"  /* net_tools_t */
#include "tsp_redirect.h"

typedef enum {
  CLIENT_VERSION_INDEX_2_0_1,
  CLIENT_VERSION_INDEX_2_0_0,
  CLIENT_VERSION_INDEX_1_0_1
} tClientVersionIndex;

#define CLIENT_VERSION_INDEX_CURRENT  CLIENT_VERSION_INDEX_2_0_1
#define CLIENT_VERSION_INDEX_OLDEST   CLIENT_VERSION_INDEX_1_0_1

#define CLIENT_VERSION_INDEX_V6UDPV4_START  CLIENT_VERSION_INDEX_2_0_0
#define CLIENT_VERSION_INDEX_V4V6_START   CLIENT_VERSION_INDEX_2_0_0

#define CLIENT_VERSION_STRING_2_0_1   "2.0.1"
#define CLIENT_VERSION_STRING_2_0_0   "2.0.0"
#define CLIENT_VERSION_STRING_1_0_1   "1.0.1"

extern int tspExtractPayload(char *, tTunnel *);
extern void tspClearPayload(tPayload *);
extern int tspGetStatusCode(char *);
extern char *tspGetStatusMsg(char *);
extern char *tspAddPayloadString(tPayload *, char *);

extern int tspSetupTunnel(tConf *, net_tools_t *[], int version_index, tBrokerList **broker_list);
int tspMain(int, char *[]);


/* IMPORTS, should be defined in platform/tsp_local.c
** this will be ran when the nego is done.
*/

extern char*  tspGetLocalAddress    ( SOCKET, char *, int);
extern unsigned short tspGetLocalPort( SOCKET socket);
extern int    tspStartLocal         ( SOCKET, tConf *, tTunnel *, net_tools_t *);
extern void   tspSetEnv             ( char *, char *, int);
extern int    tspSetupInterfaceLocal( tConf* pConf, tTunnel* pTun );
extern int    tspCheckForStopOrWait ( const unsigned int uiWaitMs );
extern void   tspGetOSInfo          ( const size_t len, char* buf );

#endif

