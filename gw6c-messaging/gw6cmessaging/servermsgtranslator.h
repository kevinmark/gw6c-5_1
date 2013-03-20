// **************************************************************************
// $Id: servermsgtranslator.h,v 1.6 2007/04/02 20:12:02 cnepveu Exp $
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
//   Defines the message processing of Gateway6 Client messages destined 
//   for the Client (which is the server-side of the communication).
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_servermsgtranslator_h__
#define __gw6cmessaging_servermsgtranslator_h__


#include <gw6cmessaging/messageprocessor.h>
#include <gw6cmessaging/gw6cuistrings.h>
#include <gw6cmessaging/hap6msgdata.h>


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class ServerMsgTranslator: public MessageProcessor
  {
  protected:
    // Construction / destruction.
                    ServerMsgTranslator   ( void );
  public:
    virtual         ~ServerMsgTranslator  ( void );

  protected:
    // Override from MessageProcessor.
    error_t         ProcessMessage        ( Message* pMsg );

    // To be implemented by derived classes.
    virtual error_t Recv_StatusInfoRequest( void )=0;
    virtual error_t Recv_TunnelInfoRequest( void )=0;
    virtual error_t Recv_BrokerListRequest( void )=0;
    virtual error_t Recv_HAP6ConfigInfo   ( const HAP6ConfigInfo* aHAP6ConfigInfo )=0;
    virtual error_t Recv_HAP6StatusInfoRequest( void )=0;

  private:
    // Message data translators.
    error_t         TranslateStatusInfoReq( BYTE* pData, const WORD nDataLen );
    error_t         TranslateTunnelInfoReq( BYTE* pData, const WORD nDataLen );
    error_t         TranslateBrokerListReq( BYTE* pData, const WORD nDataLen );
    error_t         TranslateHAP6ConfigInfo( BYTE* pData, const WORD nDataLen );
    error_t         TranslateHAP6StatusInfoReq( BYTE* pData, const WORD nDataLen );
  };

}

#endif
