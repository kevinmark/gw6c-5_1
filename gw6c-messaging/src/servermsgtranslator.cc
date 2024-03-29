// **************************************************************************
// $Id: servermsgtranslator.cc,v 1.10 2007/06/22 18:34:43 cnepveu Exp $
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
//   Implementation of the ServerMsgTranslator class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/servermsgtranslator.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


#ifdef WIN32
#define strdup _strdup
#endif


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : ServerMsgTranslator constructor
//
// Description:
//   Will initialize a new ServerMsgTranslator object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ServerMsgTranslator::ServerMsgTranslator( void ) :
  MessageProcessor()
{
  MessageProcessor::m_eProcessorState = STATE_ENABLED;
}


// --------------------------------------------------------------------------
// Function : ServerMsgTranslator destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
ServerMsgTranslator::~ServerMsgTranslator( void )
{
}


// --------------------------------------------------------------------------
// Function : ProcessMessage
//
// Description:
//   Will verify the message type and call the proper translator.
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
error_t ServerMsgTranslator::ProcessMessage( Message* pMsg )
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
    case MESSAGEID_REQUEST_STATUSINFO:
      retCode = TranslateStatusInfoReq( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    case MESSAGEID_REQUEST_TUNNELINFO:
      retCode = TranslateTunnelInfoReq( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    case MESSAGEID_REQUEST_BROKERLIST:
      retCode = TranslateBrokerListReq( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    case MESSAGEID_HAP6CONFIGINFO:
      retCode = TranslateHAP6ConfigInfo( pMsg->msg._data, pMsg->msg.header._datalen );
      break;

    case MESSAGEID_REQUEST_HAP6STATUSINFO:
      retCode = TranslateHAP6StatusInfoReq( pMsg->msg._data, pMsg->msg.header._datalen );
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
// Function : TranslateStatusInfoReq
//
// Description:
//   Will translate the data sent from the GUI. In this case, there is no
//   data sent from the GUI so there isn't anything to translate.
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
error_t ServerMsgTranslator::TranslateStatusInfoReq( BYTE* pData, const WORD nDataLen )
{
  // No translation is required for requests.
  // i.e.: GUI doesn't send information in requests.
  return Recv_StatusInfoRequest();
}


// --------------------------------------------------------------------------
// Function : TranslateTunnelInfoReq
//
// Description:
//   Will translate the data sent from the GUI. In this case, there is no
//   data sent from the GUI so there isn't anything to translate.
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
error_t ServerMsgTranslator::TranslateTunnelInfoReq( BYTE* pData, const WORD nDataLen )
{
  // No translation is required for requests.
  // i.e.: GUI doesn't send information in requests.
  return Recv_TunnelInfoRequest();
}


// --------------------------------------------------------------------------
// Function : TranslateBrokerListReq
//
// Description:
//   Will translate the data sent from the GUI. In this case, there is no
//   data sent from the GUI so there isn't anything to translate.
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
error_t ServerMsgTranslator::TranslateBrokerListReq( BYTE* pData, const WORD nDataLen )
{
  // No translation is required for requests.
  // i.e.: GUI doesn't send information in requests.
  return Recv_BrokerListRequest();
}


// --------------------------------------------------------------------------
// Function : TranslateHAP6ConfigInfo
//
// Description:
//   Will translate the HAP6 configuration info message data and call the
//   handler with the translated information.
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
error_t ServerMsgTranslator::TranslateHAP6ConfigInfo( BYTE* pData, const WORD nDataLen )
{
  HAP6ConfigInfo HAP6ConfigInfo;
  WORD nCursor = 0;
  error_t retCode;


  // -- D A T A   E X T R A C T I O N --

  // Extract HAP6 WWW document root from data buffer.
  HAP6ConfigInfo.hap6_doc_root = strdup( (char*)(pData + nCursor) );
  nCursor += (WORD)strlen( (char*)(pData + nCursor) ) + 1;

  // Extract proxy enabled from data buffer.
  memcpy( (void*)&(HAP6ConfigInfo.hap6_proxy_enabled), pData + nCursor, sizeof(HAP6ConfigInfo.hap6_proxy_enabled) );
  nCursor += sizeof(HAP6ConfigInfo.hap6_proxy_enabled);

  // Extract web enabled from data buffer.
  memcpy( (void*)&(HAP6ConfigInfo.hap6_web_enabled), pData + nCursor, sizeof(HAP6ConfigInfo.hap6_web_enabled) );
  nCursor += sizeof(HAP6ConfigInfo.hap6_web_enabled);

  // Extract if device mappings changed from data buffer.
  memcpy( (void*)&(HAP6ConfigInfo.hap6_devmap_changed), pData + nCursor, sizeof(HAP6ConfigInfo.hap6_devmap_changed) );
  nCursor += sizeof(HAP6ConfigInfo.hap6_devmap_changed);


  // -----------------------------------------------------------------------
  // Sanity check. Verify that the bytes of data we extracted match that of
  // what was expected.
  // -----------------------------------------------------------------------
  assert( nCursor == nDataLen );


  // ---------------------------------
  // Invoke derived function handler.
  // ---------------------------------
  retCode = Recv_HAP6ConfigInfo( &HAP6ConfigInfo );


  // -----------------------------------------------
  // Clean up allocated memory used for extraction.
  // -----------------------------------------------
  free( HAP6ConfigInfo.hap6_doc_root );


  return retCode;
}


// --------------------------------------------------------------------------
// Function : TranslateHAP6StatusInfoReq
//
// Description:
//   Will translate the data sent from the GUI. In this case, there is no
//   data sent from the GUI so there isn't anything to translate.
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
error_t ServerMsgTranslator::TranslateHAP6StatusInfoReq( BYTE* pData, const WORD nDataLen )
{
  // No translation is required for requests.
  // i.e.: GUI doesn't send information in requests.
  return Recv_HAP6StatusInfoRequest();
}

} // namespace
