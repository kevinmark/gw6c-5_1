// **************************************************************************
// $Id: clientmsgdataretriever.h,v 1.5 2007/02/26 19:19:42 cnepveu Exp $
//
// Copyright (c) 2007 Hexago Inc. All rights reserved.
// 
//   LICENSE NOTICE: You may use and modify this source code only if you
//   have executed a valid license agreement with Hexago Inc. granting
//   you the right to do so, the said license agreement governing such
//   use and modifications.   Copyright or other intellectual property
//   notices are not to be removed from the source code.
//
// Description:
//   Defines prototypes used by the messenger to retrieve information on
//     - status, tunnel and brokers.
//   Defines a way of freeing the information allocated in the retrievers.
//
//   These functions need to be implemented in the Gateway6 Client.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_clientsgdataretriever_h__
#define __gw6cmessaging_clientsgdataretriever_h__


#include <gw6cmessaging/gw6cmsgdata.h>
#include <gw6cmessaging/hap6msgdata.h>
#include <gw6cmessaging/gw6cuistrings.h>      // error_t definition & codes.


#ifdef __cplusplus
extern "C" {
#endif

error_t   RetrieveStatusInfo    ( Gw6cStatusInfo** ppStatusInfo );
error_t   RetrieveTunnelInfo    ( Gw6cTunnelInfo** ppTunnelInfo );
error_t   RetrieveBrokerList    ( Gw6cBrokerList** ppBrokerList );
error_t   RetrieveHAP6StatusInfo( HAP6StatusInfo** ppHAP6StatusInfo );

void      FreeStatusInfo        ( Gw6cStatusInfo** ppStatusInfo );
void      FreeTunnelInfo        ( Gw6cTunnelInfo** ppTunnelInfo );
void      FreeBrokerList        ( Gw6cBrokerList** ppBrokerList );
void      FreeHAP6StatusInfo    ( HAP6StatusInfo** ppHAP6StatusInfo );

#ifdef __cplusplus
}
#endif

#endif
