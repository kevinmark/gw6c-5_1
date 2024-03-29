// **************************************************************************
// $Id: messageprocessor.h,v 1.4 2007/01/30 18:53:26 cnepveu Exp $
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
//   This component provides a way of processing incoming messages. It is 
//   fully abstracted and forces the applications to implement the 
//   ProcessMessage function.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_messageprocessor_h__
#define __gw6cmessaging_messageprocessor_h__


#include <gw6cmessaging/message.h>
#include <gw6cmessaging/gw6cuistrings.h>


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class MessageProcessor
  {
  public:
    // Type definition.
    typedef enum { STATE_DISABLED, STATE_ENABLED } tProcessorState;

  protected:
    tProcessorState m_eProcessorState;

  protected:
    // Construction / destruction.
                    MessageProcessor      ( void ) : m_eProcessorState(STATE_DISABLED) {};
  public:
    virtual         ~MessageProcessor     ( void ) {};

    // Message Processing.
    virtual error_t ProcessMessage        ( Message* pMsg )=0;
  };

}

#endif
