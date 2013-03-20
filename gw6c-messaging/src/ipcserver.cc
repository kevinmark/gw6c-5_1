// **************************************************************************
// $Id: ipcserver.cc,v 1.6 2007/01/30 18:53:30 cnepveu Exp $
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
//  Implementation of the IPCServer class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/ipcserver.h>
#include <stddef.h>     // NULL macro definition (for BSD interop).
#include <assert.h>


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : IPCServer constructor
//
// Description:
//   Will initialize a new IPCServer object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
IPCServer::IPCServer( void ) :
  IPCServent()
{
  // Semaphore is locked initially.
  m_pSemReadyState = new Semaphore( 1, 0 );
  assert( m_pSemReadyState != NULL );
}


// --------------------------------------------------------------------------
// Function : IPCServer destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
IPCServer::~IPCServer( void )
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
//   Will initialize the IPC Server by setting up a connection and waiting
//   for an IPC client to connect.
//
// Arguments: (none)
//
// Return values:
//   GW6CM_UIS__NOERROR on success
//   See CreateConnectionPoint() and AcceptConnection() specializations.
//
// --------------------------------------------------------------------------
error_t IPCServer::Initialize( void )
{
  error_t nRetCode;

  if( (nRetCode = CreateConnectionPoint()) == GW6CM_UIS__NOERROR )
  {
    // SIGNAL THE READY STATE
    m_pSemReadyState->ReleaseLock();

    return AcceptConnection();
  }

  return nRetCode;
}


// --------------------------------------------------------------------------
// Function : UnInitialize
//
// Description:
//   Will terminate the connection with the IPC client and shut down.
//
// Arguments: (none)
//
// Return values:
//   GW6CM_UIS__NOERROR on success
//   See CloseConnection() specialization.
//
// --------------------------------------------------------------------------
error_t IPCServer::UnInitialize( void )
{
  return CloseConnection();
}


// --------------------------------------------------------------------------
// Function : WaitReady
//
// Description:
//   Will wait until the m_pSemReadyState semaphore is released.
//   The semaphore is released when the server has created the connection
//   point and is ready to accept incomming connection(s).
//
// Arguments:
//   ulWaitms: unsigned long [IN], The timeout for the wait operation.
//             If 0, the timeout is INFINITE.
//
// Return values:
//   Returns true if the semaphore is released before the specified timeout.
//
// --------------------------------------------------------------------------
bool IPCServer::WaitReady( unsigned long ulWaitms )
{
  bool bRetVal;

  if( bRetVal = (m_pSemReadyState->WaitAndLock( ulWaitms ) == 0) )
    m_pSemReadyState->ReleaseLock();

  return bRetVal;
}

}
