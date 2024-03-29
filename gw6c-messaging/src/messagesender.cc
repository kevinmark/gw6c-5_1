// **************************************************************************
// $Id: messagesender.cc,v 1.3 2007/01/30 18:53:30 cnepveu Exp $
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
//   Implementation of the MessageSender class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/messagesender.h>
#include <assert.h>


#define MAX_QUEUE_ITEMS     512


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : MessageSender constructor
//
// Description:
//   Will initialize a new MessageSender object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
MessageSender::MessageSender( void ) :
  m_eSenderState( STATE_DISABLED ),
  m_pSemaphore(NULL)
{
  assert( m_SendQueue.empty() );

  // Create a semaphore.
  m_pSemaphore = new Semaphore( MAX_QUEUE_ITEMS, 0 );
  assert( m_pSemaphore != NULL );
}


// --------------------------------------------------------------------------
// Function : MessageSender destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
MessageSender::~MessageSender( void )
{
  if( m_pSemaphore != NULL )
  {
    delete m_pSemaphore;
    m_pSemaphore = NULL;
  }
}


// --------------------------------------------------------------------------
// Function : PostMessage
//
// Description:
//   Will put the message in the send queue for further processing, only if 
//   the state is enabled.
//
// Arguments:
//   pMsg: Message* [IN], The message to post.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void MessageSender::PostMessage( Message* pMsg )
{
  if( m_eSenderState == STATE_ENABLED  &&  m_SendQueue.size() < MAX_QUEUE_ITEMS )
  {
    m_SendQueue.push( pMsg );
    m_pSemaphore->ReleaseLock();      // Increase semaphore count.
  }
}


// --------------------------------------------------------------------------
// Function : Reset
//
// Description:
//   Resets the MessageSender. Empties send queue and re-initializes the
//   semaphore object.
//
// Arguments:
//   pMsg: Message* [IN], The message to post.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void MessageSender::Reset( void )
{
  // Empty queue
  while( !m_SendQueue.empty() )
    m_SendQueue.pop();

  // Reset semaphore object
  if( m_pSemaphore != NULL )
    delete m_pSemaphore;

  m_pSemaphore = new Semaphore( MAX_QUEUE_ITEMS, 0 );
}

}
