// **************************************************************************
// $Id: message.cc,v 1.4 2007/01/30 18:53:30 cnepveu Exp $
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
//   Implementation of the Message union.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/message.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : CreateMessage     [ STATIC ]
//
// Description:
//   Will create a new Message object.
//
// Arguments:
//   aMsgId: WORD [IN], The message identifier.
//   aDataLen: WORD [IN], The length of data pointed by `aData'.
//   aData: BYTE* [IN], The message data.
//
// Return values:
//   Pointer to newly created message. NULL is returned on error.
//
// --------------------------------------------------------------------------
Message* Message::CreateMessage( const WORD aMsgId, const WORD aDataLen, const BYTE* aData )
{
  const size_t cnMsgSize = (MSG_HEADER_LEN) + aDataLen;
  Message* pMsg = NULL;


  // Only create messages that are within message data length bounds.
  if( aDataLen < MSG_MAX_USERDATA )
  {
    // Create enough space to copy user data.
    pMsg = (Message*) new BYTE[ cnMsgSize ];
    assert( pMsg != NULL );

    // Set message members.
    pMsg->msg.header._msgid = aMsgId;
    pMsg->msg.header._datalen = aDataLen;
    memcpy( pMsg->msg._data, aData, aDataLen );
  }

  // return the new message.
  return pMsg;
}


// --------------------------------------------------------------------------
// Function : FreeMessage       [ static ]
//
// Description:
//   Will delete the space used by the message.
//
// Arguments:
//   pMsg: Message* [IN,OUT], The message to be deleted.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void Message::FreeMessage( Message* pMsg )
{
  BYTE* pBuf = (BYTE*)pMsg;

  delete [] pBuf;
  pMsg = NULL;
}


// --------------------------------------------------------------------------
// Function : AssertMessage       [ static ]
//
// Description:
//   Will perform verification on the message content.
//
// Arguments:
//   pMsg: Message* [IN], The message to be verified.
//
// Return values:
//   
//
// --------------------------------------------------------------------------
void Message::AssertMessage( const Message* pMsg )
{
  assert( pMsg->msg.header._datalen <= MSG_MAX_USERDATA );
}


// --------------------------------------------------------------------------
// Function : GetRawSize
//
// Description:
//   Will return the byte size of the data managed by this class:
//   - Message header length + user data length.
//  This is useful to know the amount of bytes to copy from member `rawdata'.
//
// Arguments: (none)
//
// Return values:
//   The size of this class' data.
//
// --------------------------------------------------------------------------
size_t Message::GetRawSize( void ) const
{
  return (size_t)((MSG_HEADER_LEN) + this->msg.header._datalen);
}

}
