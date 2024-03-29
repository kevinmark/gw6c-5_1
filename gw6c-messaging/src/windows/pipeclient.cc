// **************************************************************************
// $Id: pipeclient.cc,v 1.2 2007/01/30 18:53:33 cnepveu Exp $
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
//   Windows implementation of the PipeClient class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/pipeclient.h>
#include <windows.h>
#include <assert.h>


#define PIPE_BUFSIZ           4096
#define PIPE_DFLT_TIMEOUT     200     // miliseconds


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : PipeClient constructor
//
// Description:
//   Will initialize a new PipeClient object.
//
// Arguments:
//   aPipeName: string [IN], The name of the pipe this client will 
//              connect to.
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
PipeClient::PipeClient( const string& aPipeName ) :
  IPCClient(),
  PipeIO(),
  m_PipeName(aPipeName)
{
  assert( !aPipeName.empty() );
}


// --------------------------------------------------------------------------
// Function : PipeClient destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
PipeClient::~PipeClient( void )
{
  // Is client pipe still online ?
  assert( m_Handle == INVALID_HANDLE_VALUE );
}


// --------------------------------------------------------------------------
// Function : Connect
//
// Description:
//   Will establish a client pipe with the server.
//   NOTE: THIS FUNCTION IS (big time) BLOCKING!!!
//
// Arguments: (none)
//
// Return values:
//   GW6CM_UIS__NOERROR: Operation successful.
//   Any other value is an error. Use GetLastError() for more information.
//
// --------------------------------------------------------------------------
error_t PipeClient::Connect( void )
{
  // Check if client pipe handle is valid.
  if( m_Handle != INVALID_HANDLE_VALUE )
    return GW6CM_UIS_CLIENTALRDYCONN;

  // Loop until we're connected.
  while( m_Handle == INVALID_HANDLE_VALUE )
  {
    // Check if server pipe endpoint is available.
    if( WaitNamedPipe( m_PipeName.c_str(), NMPWAIT_WAIT_FOREVER ) == TRUE )
    {
      // Connect to server endpoint.
      m_Handle = CreateFile( m_PipeName.c_str(),
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL );

      // Validate returned value.
      if( m_Handle == INVALID_HANDLE_VALUE )
      {
        return GW6CM_UIS_FAILCREATECLIENTPIPE;
      }

      // Specify client transfer mode: MESSAGES
      DWORD dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
      if( SetNamedPipeHandleState( m_Handle, &dwMode, NULL, NULL ) == 0 )
      {
        CloseHandle( m_Handle );
        m_Handle = INVALID_HANDLE_VALUE;
        return GW6CM_UIS_FAILCREATECLIENTPIPE;
      }
    }
    else
    {
      // If no instances of the specified named pipe exist, the 
      // WaitNamedPipe function returns immediately, regardless of 
      // the time-out value.
      Sleep(20);      // Wait a bit before retrying.

      // Failure to sleep will cause 100% CPU usage.
    }
  }

  // Connection to server successful.
  return GW6CM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : Disconnect
//
// Description:
//   Will terminate connection to the pipe server.
//
// Arguments: (none)
//
// Return values:
//   GW6CM_UIS__NOERROR: Operation successful.
//   Any other value is an error. Use GetLastError() for more information.
//
// --------------------------------------------------------------------------
error_t PipeClient::Disconnect( void )
{
  // Check if client pipe handle is valid.
  if( m_Handle == INVALID_HANDLE_VALUE )
    return GW6CM_UIS_PIPECLIDISCFAIL;

  // Close connection to server.
  if( CloseHandle( m_Handle ) == 0 )
  {
    return GW6CM_UIS_PIPECLIDISCFAIL;
  }
  m_Handle = INVALID_HANDLE_VALUE;

  // Disconnection successful.
  return GW6CM_UIS__NOERROR;
}

}
