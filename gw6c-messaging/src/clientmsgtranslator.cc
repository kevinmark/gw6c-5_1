// **************************************************************************
// $Id: clientmsgtranslator.cc,v 1.11 2007/03/06 21:29:26 cnepveu Exp $
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
//   Implementation of the ClientMsgTranslator class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/clientmsgtranslator.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


#ifdef WIN32
#define strdup _strdup
#endif


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : ClientMsgTranslator constructor
//
// Description:
//   Will initialize a new ClientMsgTranslator object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ClientMsgTranslator::ClientMsgTranslator( void ) :
  MessageProcessor()
{
  // Enable message processing.
  MessageProcessor::m_eProcessorState = STATE_ENABLED;
}


// --------------------------------------------------------------------------
// Function : Gw6cMsgClientReceiver destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ClientMsgTranslator::~ClientMsgTranslator( void )
{
}


// --------------------------------------------------------------------------
// Function : ProcessMessage
//
// Description:
//   Will verify the message type and call the proper translator.
//   NOTE: Try not to do any lengthy operations here, because we're executing
//         in the receiver thread.
//
// Arguments:
//   pMsg: Message* [IN], The message to process.
//
// Return values:
//   GW6CM_UIS__NOERROR: Successful operation.
//   GW6CM_UIS_MESSAGENOTIMPL: Message not implemented.
//   GW6CM_UIS_MSGPROCDISABLED: Message processing is disabled.
//   <any other error message>
//
// --------------------------------------------------------------------------
error_t ClientMsgTranslator::ProcessMessage( Message* pMsg )
{
  error_t retCode;
  assert( pMsg );


  // Process messages only if the message processor is enabled.
  if( m_eProcessorState == STATE_ENABLED )
  {
    // -------------------------------------
    // Verify what kind of message this is.
    // -------------------------------------
    switch( pMsg->msg.header._msgid )
    {
    case MESSAGEID_STATUSINFO:
      retCode = TranslateStatusInfo( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    case MESSAGEID_TUNNELINFO:
      retCode = TranslateTunnelInfo( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    case MESSAGEID_BROKERLIST:
      retCode = TranslateBrokerList( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    case MESSAGEID_HAP6STATUSINFO:
      retCode = TranslateHAP6StatusInfo( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    default:
      retCode = GW6CM_UIS_MESSAGENOTIMPL; // Unknown / invalid message.
      break;
    }
  }
  else
    retCode = GW6CM_UIS_MSGPROCDISABLED;

  // Return completion status.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : TranslateStatusInfo
//
// Description:
//   Will extract the status information from the byte buffer and invoke the
//   handler.
//
// Arguments:
//   pData: BYTE* [IN], The raw data.
//   nDataLen: WORD [IN], The length of the raw data.
//
// Return values:
//   GW6CM_UIS__NOERROR: Successful operation.
//   any other value on error.
//
// --------------------------------------------------------------------------
error_t ClientMsgTranslator::TranslateStatusInfo( BYTE* pData, const WORD nDataLen )
{
  Gw6cStatusInfo statusInfo;
  WORD nCursor = 0;
  error_t retCode;


  // ----------------------------------------------
  // Extract the status info from the byte buffer.
  // ----------------------------------------------
  memcpy( (void*)&(statusInfo.eStatus), (void*)pData, sizeof(Gw6cCliStatus) );
  nCursor += sizeof(Gw6cCliStatus);

  // Extract message sent along status info.
  memcpy( (void*)&(statusInfo.nStatus), (void*)(pData + nCursor), sizeof(statusInfo.nStatus) );
  nCursor += sizeof(statusInfo.nStatus);


  // -----------------------------------------------------------------------
  // Sanity check. Verify that the bytes of data we extracted match that of
  // what was expected.
  // -----------------------------------------------------------------------
  assert( nCursor == nDataLen );


  // ---------------------------------
  // Invoke derived function handler.
  // ---------------------------------
  retCode = Recv_StatusInfo( &statusInfo );


  // Return completion code.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : TranslateTunnelInfo
//
// Description:
//   Will extract the tunnel information from the byte buffer and invoke the
//   handler.
//
// Arguments:
//   pData: BYTE* [IN], The raw data.
//   nDataLen: WORD [IN], The length of the raw data.
//
// Return values:
//   GW6CM_UIS__NOERROR: Successful operation.
//   any other value on error.
//
// --------------------------------------------------------------------------
error_t ClientMsgTranslator::TranslateTunnelInfo( BYTE* pData, const WORD nDataLen )
{
  Gw6cTunnelInfo tunnelInfo;
  WORD nCursor = 0;
  error_t retCode;


  // -- D A T A   E X T R A C T I O N --

  // Extract broker name from data buffer.
  tunnelInfo.szBrokerName = strdup( (char*)(pData + nCursor) );
  nCursor += strlen( (char*)(pData + nCursor) ) + 1;

  // Extract tunnel type from data buffer.
  memcpy( (void*)&(tunnelInfo.eTunnelType), pData + nCursor, sizeof(Gw6cTunnelType) );
  nCursor += sizeof(Gw6cTunnelType);

  // Extract Local tunnel endpoint IPv4 address from data buffer.
  tunnelInfo.szIPV4AddrLocalEndpoint = strdup( (char*)(pData + nCursor) );
  nCursor += strlen( (char*)(pData + nCursor) ) + 1;

  // Extract Local tunnel endpoint IPv6 address from data buffer.
  tunnelInfo.szIPV6AddrLocalEndpoint = strdup( (char*)(pData + nCursor) );
  nCursor += strlen( (char*)(pData + nCursor) ) + 1;

  // Extract Remote tunnel endpoint IPv4 address from data buffer.
  tunnelInfo.szIPV4AddrRemoteEndpoint = strdup( (char*)(pData + nCursor) );
  nCursor += strlen( (char*)(pData + nCursor) ) + 1;

  // Extract Remote tunnel endpoint IPv6 address from data buffer.
  tunnelInfo.szIPV6AddrRemoteEndpoint = strdup( (char*)(pData + nCursor) );
  nCursor += strlen( (char*)(pData + nCursor) ) + 1;

  // Extract The delegated prefix from data buffer.
  tunnelInfo.szDelegatedPrefix = strdup( (char*)(pData + nCursor) );
  nCursor += strlen( (char*)(pData + nCursor) ) + 1;

  // Extract The delegated user domain from data buffer.
  tunnelInfo.szUserDomain = strdup( (char*)(pData + nCursor) );
  nCursor += strlen( (char*)(pData + nCursor) ) + 1;

  // Extract tunnel uptime from data buffer.
  memcpy( (void*)&(tunnelInfo.tunnelUpTime), pData + nCursor, sizeof(time_t) );
  nCursor += sizeof(time_t);


  // -----------------------------------------------------------------------
  // Sanity check. Verify that the bytes of data we extracted match that of
  // what was expected.
  // -----------------------------------------------------------------------
  assert( nCursor == nDataLen );


  // ---------------------------------
  // Invoke derived function handler.
  // ---------------------------------
  retCode = Recv_TunnelInfo( &tunnelInfo );


  // -----------------------------------------------
  // Clean up allocated memory used for extraction.
  // -----------------------------------------------
  free( tunnelInfo.szBrokerName );
  free( tunnelInfo.szIPV4AddrLocalEndpoint );
  free( tunnelInfo.szIPV6AddrLocalEndpoint );
  free( tunnelInfo.szIPV4AddrRemoteEndpoint );
  free( tunnelInfo.szIPV6AddrRemoteEndpoint );
  free( tunnelInfo.szDelegatedPrefix );
  free( tunnelInfo.szUserDomain );


  // Return completion code.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : TranslateBrokerList
//
// Description:
//   Will extract the broker list from the byte buffer and invoke the
//   handler.
//
// Arguments:
//   pData: BYTE* [IN], The raw data.
//   nDataLen: WORD [IN], The length of the raw data.
//
// Return values:
//   GW6CM_UIS__NOERROR: Successful operation.
//   any other value on error.
//
// --------------------------------------------------------------------------
Gw6cBrokerList* BuildBrokerList( BYTE* pData, const size_t nDataLen, size_t& nCursor )
{
  if( nCursor >= nDataLen )   // End condition.
    return NULL;

  Gw6cBrokerList* pList = new Gw6cBrokerList;
  size_t nNameLen = strlen( (const char*)pData ) + 1;
  nCursor += nNameLen;

  pList->szAddress = strdup( (const char*)pData );
  memcpy( (void*)&(pList->nDistance), pData + nCursor, sizeof(int) );
  nCursor += sizeof(int);
  pList->next = BuildBrokerList( pData + nNameLen + sizeof(int), nDataLen, nCursor );

  return pList;
}


// --------------------------------------------------------------------------
error_t ClientMsgTranslator::TranslateBrokerList( BYTE* pData, const WORD nDataLen )
{
  Gw6cBrokerList* pList;
  size_t nCursor = 0;
  error_t retCode;


  // ------------------------------------------
  // Extract broker list from the byte buffer.
  // ------------------------------------------
  pList = BuildBrokerList( pData, nDataLen, nCursor );


  // -----------------------------------------------------------------------
  // Sanity check. Verify that the bytes of data we extracted match that of
  // what was expected.
  // -----------------------------------------------------------------------
  assert( nCursor == nDataLen );


  // ---------------------------------
  // Invoke derived function handler.
  // ---------------------------------
  retCode = Recv_BrokerList( pList );


  // -----------------------------------------------
  // Clean up allocated memory used for extraction.
  // -----------------------------------------------
  Gw6cBrokerList* pListLast = NULL;
  for(; pList!=NULL; pList = pList->next )
  {
    if( pListLast != NULL ) delete pListLast;
    free( pList->szAddress );
    pListLast = pList;
  }
  if( pListLast != NULL ) delete pListLast;


  // Return completion status.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : TranslateHAP6StatusInfo
//
// Description:
//   Will extract the HAP6 Status Info from the byte buffer and invoke the
//   handler.
//
// Arguments:
//   pData: BYTE* [IN], The raw data.
//   nDataLen: WORD [IN], The length of the raw data.
//
// Return values:
//   GW6CM_UIS__NOERROR: Successful operation.
//   any other value on error.
//
// --------------------------------------------------------------------------
PMAPPING_STATUS BuildMappingStatuses( BYTE* pData, const size_t nDataLen, size_t& nCursor )
{
  if( nCursor >= nDataLen )   // End condition.
    return NULL;

  PMAPPING_STATUS pMappingStatus = new MAPPING_STATUS;

  // Extract Device Name
  pMappingStatus->device_name = strdup( (const char*)pData );
  size_t nNameLen = strlen( (const char*)pData ) + 1;
  nCursor += nNameLen;

  // Extract Device mapping status
  memcpy( (void*)&(pMappingStatus->mapping_status), pData + nNameLen, sizeof(HAP6DevMapStts) );
  nCursor += sizeof(int);

  // Extract the next.
  pMappingStatus->next = BuildMappingStatuses( pData + nNameLen + sizeof(int), nDataLen, nCursor );

  return pMappingStatus;
}

void FreeMappingStatus( PMAPPING_STATUS mapStat )
{
  if( mapStat != NULL )
  {
    FreeMappingStatus( mapStat->next );
    assert( mapStat->device_name != NULL );
    delete [] mapStat->device_name;
    delete mapStat;
  }
}

error_t ClientMsgTranslator::TranslateHAP6StatusInfo( BYTE* pData, const WORD nDataLen )
{
  HAP6StatusInfo statusInfo;
  size_t nCursor = 0;
  error_t retCode;


  // -- D A T A   E X T R A C T I O N --

  // Extract the proxy status from data buffer.
  memcpy( (void*)&(statusInfo.hap6_proxy_status), pData + nCursor, sizeof(HAP6FeatStts) );
  nCursor += sizeof(HAP6FeatStts);

  // Extract the web status from data buffer.
  memcpy( (void*)&(statusInfo.hap6_web_status), pData + nCursor, sizeof(HAP6FeatStts) );
  nCursor += sizeof(HAP6FeatStts);

  // Extract the Device Mapping Module status from data buffer.
  memcpy( (void*)&(statusInfo.hap6_devmapmod_status), pData + nCursor, sizeof(HAP6FeatStts) );
  nCursor += sizeof(HAP6FeatStts);

  // Extract the device mapping statuses. Unserialize the linked list.
  statusInfo.hap6_devmap_statuses = BuildMappingStatuses( pData + nCursor, nDataLen, nCursor );


  // -----------------------------------------------------------------------
  // Sanity check. Verify that the bytes of data we extracted match that of
  // what was expected.
  // -----------------------------------------------------------------------
  assert( nCursor == nDataLen );


  // ---------------------------------
  // Invoke derived function handler.
  // ---------------------------------
  retCode = Recv_HAP6StatusInfo( &statusInfo );


  // -----------------------------------------------
  // Clean up allocated memory used for extraction.
  // -----------------------------------------------
  FreeMappingStatus( statusInfo.hap6_devmap_statuses );


  // Return completion code.
  return retCode;
}


} // namespace
