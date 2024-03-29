// **************************************************************************
// $Id: ipcclient.cc,v 1.5 2007/01/30 18:53:30 cnepveu Exp $
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
//  Implementation of the IPCClient class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/ipcclient.h>
#include <stddef.h>     // NULL macro definition (for BSD interop).
#include <assert.h>


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : IPCClient constructor
//
// Description:
//   Will initialize a new IPCClient object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
IPCClient::IPCClient( void ) :
  IPCServent()
{
  // Semaphore is locked initially.
  m_pSemReadyState = new Semaphore( 1, 0 );
  assert( m_pSemReadyState != NULL );
}


// --------------------------------------------------------------------------
// Function : IPCClient destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
IPCClient::~IPCClient( void )
{
  if( m_pSemReadyState != NULL )
  {
    delete m_pSemReadyState;
    m_pSemReadyState = NULL;
  }
}


// --------------------------------------------------------------------------
// Function : Initialize
//
// Description:
//   Will initialize the IPC client by connecting to the IPC server.
//
// Arguments: (none)
//
// Return values:
//   See Connect() specialization.
//
// --------------------------------------------------------------------------
error_t IPCClient::Initialize( void )
{
  // Signal the ready state before blocking on connect.
  m_pSemReadyState->ReleaseLock();

  return Connect();
}


// --------------------------------------------------------------------------
// Function : UnInitialize
//
// Description:
//   Will terminate the connection with the IPC server and shut down.
//
// Arguments: (none)
//
// Return values:
//   See Disconnect() specialization.
//
// --------------------------------------------------------------------------
error_t IPCClient::UnInitialize( void )
{
  return Disconnect();
}


// --------------------------------------------------------------------------
// Function : WaitReady
//
// Description:
//   Will wait until the m_pSemReadyState semaphore is released.
//   The semaphore is released when the client is ready to connect
//   to the peer IPC server.
//
// Arguments:
//   ulWaitms: unsigned long [IN], The timeout for the wait operation.
//             If 0, the timeout is INFINITE.
//
// Return values:
//   Returns true if the semaphore is released before the specified timeout.
//
// --------------------------------------------------------------------------
bool IPCClient::WaitReady( unsigned long ulWaitms )
{
  bool bRetVal;

  if( (bRetVal = m_pSemReadyState->WaitAndLock( ulWaitms ) == 0) )
    m_pSemReadyState->ReleaseLock();

  return bRetVal;
}

}
