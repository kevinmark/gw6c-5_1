// **************************************************************************
// $Id: clientmsgtranslator.h,v 1.5 2007/02/23 21:30:24 cnepveu Exp $
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
//   for the GUI (which is the client-side).
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_clientmsgtranslator_h__
#define __gw6cmessaging_clientmsgtranslator_h__


#include <gw6cmessaging/messageprocessor.h>
#include <gw6cmessaging/gw6cuistrings.h>
#include <gw6cmessaging/gw6cmsgdata.h>
#include <gw6cmessaging/hap6msgdata.h>


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class ClientMsgTranslator: public MessageProcessor
  {
  protected:
    // Construction / destruction.
                    ClientMsgTranslator   ( void );
  public:
    virtual         ~ClientMsgTranslator  ( void );

  protected:
    // Override from MessageProcessor.
    error_t         ProcessMessage        ( Message* pMsg );

    // To be implemented by derived classes.
    virtual error_t Recv_StatusInfo       ( const Gw6cStatusInfo* aStatusInfo )=0;
    virtual error_t Recv_TunnelInfo       ( const Gw6cTunnelInfo* aTunnelInfo )=0;
    virtual error_t Recv_BrokerList       ( const Gw6cBrokerList* aBrokerList )=0;
    virtual error_t Recv_HAP6StatusInfo   ( const HAP6StatusInfo* aHAP6StatusInfo )=0;

  private:
    // Message data translators.
    error_t         TranslateStatusInfo   ( BYTE* pData, const WORD nDataLen );
    error_t         TranslateTunnelInfo   ( BYTE* pData, const WORD nDataLen );
    error_t         TranslateBrokerList   ( BYTE* pData, const WORD nDataLen );
    error_t         TranslateHAP6StatusInfo( BYTE* pData, const WORD nDataLen );
  };

}

#endif
