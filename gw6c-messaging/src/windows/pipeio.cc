// **************************************************************************
// $Id: pipeio.cc,v 1.3 2007/05/25 20:12:01 cnepveu Exp $
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
//   Windows implementation of the PipeIO class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/pipeio.h>
#include <windows.h>
#include <assert.h>


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : PipeIO constructor
//
// Description:
//   Will initialize a new PipeIO object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
PipeIO::PipeIO( void ) : 
  IPCServent()
{
}


// --------------------------------------------------------------------------
// Function : PipeIO destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
PipeIO::~PipeIO( void )
{
}


// --------------------------------------------------------------------------
// Function : CanRead
//
// Description:
//   Will verify if a Read operation will be blocking. This is done by 
//   verifying if there's data available for a read operation.
//
// Arguments:
//   bCanRead: boolean [OUT], Boolean indicating if there's data to read.
//
// Return values:
//   GW6CM_UIS__NOERROR: On successful peek
//   GW6CM_UIS_PEEKPIPEFAILED: Upon an IO error on the peek.
//
// --------------------------------------------------------------------------
error_t PipeIO::CanRead( bool& bCanRead ) const
{
  error_t retCode = GW6CM_UIS__NOERROR;
  DWORD   nBytesAvailable;

  // Verify IPC handle.
  assert( m_Handle != INVALID_HANDLE_VALUE );

  // Take a peek at the pipe to see if we've got stuff to read.
  if( PeekNamedPipe( m_Handle, NULL, 0, NULL, &nBytesAvailable, NULL ) == 0 )
  {
    // PeekNamedPipe failed.
    retCode = GW6CM_UIS_PEEKPIPEFAILED;
    nBytesAvailable = 0;
  }

  // Set whether we can read or not.
  bCanRead = (nBytesAvailable > 0);


  // Return operation result.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : CanWrite
//
// Description:
//   Will verify if a Write operation will be blocking.
//
// Arguments:
//   bCanWrite: boolean [OUT], Boolean indicating if it's possible to write.
//
// Return values:
//   GW6CM_UIS__NOERROR: On successful peek
//   GW6CM_UIS_PEEKPIPEFAILED: Upon an IO error on the peek.
//
// --------------------------------------------------------------------------
error_t PipeIO::CanWrite( bool& bCanWrite ) const
{
  error_t retCode = GW6CM_UIS__NOERROR;
  DWORD dummy;

  // Verify IPC handle.
  assert( m_Handle != INVALID_HANDLE_VALUE );
  bCanWrite = true;

  // This (dummy) call to PeekNamedPipe will verify if the handle is valid.
  if( PeekNamedPipe( m_Handle, NULL, 0, NULL, &dummy, NULL ) == 0 )
  {
    // PeekNamedPipe failed.
    retCode = GW6CM_UIS_PEEKPIPEFAILED;
    bCanWrite = false;
  }


  // -----------------------------------------------------------
  // Writing should not block, except if send buffer is full...
  // ... and there's no way of knowing that.
  // -----------------------------------------------------------


  // Return operation result.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : Read
//
// Description:
//   Will attempt to receive data from the pipe.
//
// Arguments:
//   pvReadBuffer: void* [OUT], The receiving buffer.
//   nBufferSize: int [IN], The size in bytes allocated for read at the
//                receive buffer.
//
// Return values:
//   GW6CM_UIS__NOERROR: Operation successful.
//   Any other value is an error.
//
// --------------------------------------------------------------------------
error_t PipeIO::Read( void* pvReadBuffer, const size_t nBufferSize, size_t& nRead )
{
  // Verify IPC handle.
  assert( m_Handle != INVALID_HANDLE_VALUE );

  // Read from pipe.
  if( ReadFile( m_Handle, pvReadBuffer, nBufferSize, (DWORD*)(&nRead), NULL ) == 0 )
  {
    return GW6CM_UIS_READPIPEFAILED;
  }

  // Operation successful.
  return GW6CM_UIS__NOERROR;
}


// --------------------------------------------------------------------------
// Function : Write
//
// Description:
//   Will attempt to write data to the pipe.
//
// Arguments:
//   pvData: void* [IN], The receiving buffer.
//   nDataSize: size_t [IN], The size in bytes to write.
//   nWritten: size_t [OUT], The number of bytes written.
//
// Return values:
//   GW6CM_UIS__NOERROR: Operation successful.
//   Any other value is an error.
//
// --------------------------------------------------------------------------
error_t PipeIO::Write( const void* pvData, const size_t nDataSize, size_t& nWritten )
{
  // Verify IPC handle.
  assert( m_Handle != INVALID_HANDLE_VALUE );

  // Write to pipe.
  if( WriteFile( m_Handle, pvData, nDataSize, (DWORD*)(&nWritten), NULL ) == 0 )
  {
    return GW6CM_UIS_WRITEPIPEFAILED;
  }

  // Operation successful.
  return GW6CM_UIS__NOERROR;
}

}
