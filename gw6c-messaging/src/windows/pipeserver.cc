// **************************************************************************
// $Id: pipeserver.cc,v 1.2 2007/01/30 18:53:33 cnepveu Exp $
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
//   Windows implementation of the PipeServer class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/pipeserver.h>
#include <windows.h>
#include <assert.h>


#define PIPE_BUFSIZ           65000   // Should be large enough to accomodate user messages.
#define PIPE_DFLT_TIMEOUT     200     // miliseconds


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : PipeServer constructor
//
// Description:
//   Will initialize a new PipeServer object.
//
// Arguments:
//   aPipeName: string [IN], The name of the pipe this server will create.
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
PipeServer::PipeServer( const string& aPipeName ) :
  IPCServer(),
  PipeIO(),
  m_PipeName(aPipeName),
  m_bClientConnected(false)
{
}


// --------------------------------------------------------------------------
// Function : PipeServer destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
PipeServer::~PipeServer( void )
{
  // Is server pipe still online ?
  assert( m_Handle == INVALID_HANDLE_VALUE );

  // Is client still connected ?
  assert( m_bClientConnected == false );
}


// --------------------------------------------------------------------------
// Function : CreateConnectionPoint
//
// Description:
//   Will create a NamedPipe server endpoint.
//
// Arguments: (none)
//
// Return values:
//   GW6CM_UIS__NOERROR: Operation successful.
//   Any other value is an error. Use GetLastError() for more information.
//
// --------------------------------------------------------------------------
error_t PipeServer::CreateConnectionPoint( void )
{
  assert( !m_PipeName.empty() );

  if( m_Handle != INVALID_HANDLE_VALUE )
    return GW6CM_UIS_PIPESERVERALRDUP;


  // ---------------------------------
  // Create the server pipe endpoint.
  // ---------------------------------
  m_Handle = CreateNamedPipe( m_PipeName.c_str(),
                                  PIPE_ACCESS_DUPLEX,     // Open Mode
                                  PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                  1,
                                  PIPE_BUFSIZ,
                                  PIPE_BUFSIZ,
                                  PIPE_DFLT_TIMEOUT,
                                  NULL );

  // Verify returned value.
  if( m_Handle == INVALID_HANDLE_VALUE )
  { 
    return GW6CM_UIS_FAILCREATESERVERPIPE;
  }

  // Successful operation.
  return GW6CM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : AcceptConnection
//
// Description:
//   Will accept a pipe client connection.
//   NOTE: THIS FUNCTION IS (big time) BLOCKING!!!
//
// Arguments: (none)
//
// Return values:
//   GW6CM_UIS__NOERROR: Operation successful.
//   Any other value is an error. Use GetLastError() for more information.
//
// --------------------------------------------------------------------------
error_t PipeServer::AcceptConnection( void )
{
  if( m_bClientConnected )
    return GW6CM_UIS_CLIENTALRDYCONN;

  // Verify the server pipe handle.
  assert( m_Handle != INVALID_HANDLE_VALUE );

  // Await client connection
  if( ConnectNamedPipe( m_Handle, NULL ) == 0 )
  {
    DWORD dwError = GetLastError();

    // If a client connects before the ConnectNamedPipe function is called,
    // the function returns zero and GetLastError returns
    // ERROR_PIPE_CONNECTED. This can happen if a client connects in the
    // interval between the call to CreateNamedPipe and the call to 
    // ConnectNamedPipe. In this situation, there is a good connection 
    // between client and server, even though the function returns zero.
    if( dwError != ERROR_PIPE_CONNECTED )
      return GW6CM_UIS_CLIENTCONNFAILED;
  }

  // Client connection successful
  return GW6CM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : CloseConnection
//
// Description:
//   Will close the server-side pipe instance.
//
// Arguments: (none)
//
// Return values:
//   GW6CM_UIS__NOERROR: Operation successful.
//   Any other value is an error. Use GetLastError() for more information.
//
// --------------------------------------------------------------------------
error_t PipeServer::CloseConnection( void )
{
  // Verify if the server pipe handle is valid.
  if( m_Handle == INVALID_HANDLE_VALUE )
    return GW6CM_UIS__NOERROR;    // Already closed, or cannot close.

  // Disconnect the client from the server side.
  if( DisconnectNamedPipe( m_Handle ) == 0 )
  {
    return GW6CM_UIS_PIPESVRDISCFAIL;
  }

  // Close server-side pipe instance.
  if( CloseHandle( m_Handle ) == 0 )
  {
    return GW6CM_UIS_PIPESVRDISCFAIL;
  }

  // Invalidate server pipe handle.
  m_Handle = INVALID_HANDLE_VALUE;

  // Mark client as not connected.
  m_bClientConnected = false;

  // Server pipe disconnection successful.
  return GW6CM_UIS__NOERROR;
}

}
