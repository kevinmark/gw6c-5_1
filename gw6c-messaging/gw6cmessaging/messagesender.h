// **************************************************************************
// $Id: messagesender.h,v 1.4 2007/01/30 18:53:26 cnepveu Exp $
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
//   This component contains a way of posting messages. The messages are 
//   accumulated into a send queue.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_messagesender_h__
#define __gw6cmessaging_messagesender_h__


#include <gw6cmessaging/message.h>
#include <gw6cmessaging/semaphore.h>
#include <queue>
using namespace std;


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class MessageSender
  {
  public:
    // Type definition.
    typedef enum { STATE_DISABLED, STATE_ENABLED } tSenderState;

  protected:
    tSenderState    m_eSenderState;
    queue<Message*> m_SendQueue;
    Semaphore*      m_pSemaphore; // Semaphore on queue.


  protected:
    // Construction / destruction.
                    MessageSender         ( void );
  public:
    virtual         ~MessageSender        ( void );

    // Offers a means of posting messages.
    void            PostMessage           ( Message* pMsg );

  protected:
    void            Reset                 ( void );
  };

}

#endif
