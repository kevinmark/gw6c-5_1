// **************************************************************************
// $Id: threadwrapper.cc,v 1.2 2007/01/30 18:53:33 cnepveu Exp $
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
//   IPC Layer tester.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/threadwrapper.h>
#include <assert.h>


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : ThreadWrapper constructor
//
// Description:
//   Will initialize a new ThreadWrapper object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
ThreadWrapper::ThreadWrapper( void ):
  m_hThread(NULL), 
  m_hQuitEvent(NULL)
{
}


// --------------------------------------------------------------------------
// Function : ThreadWrapper destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
ThreadWrapper::~ThreadWrapper( void )
{
}


// --------------------------------------------------------------------------
// Function : Run
//
// Description:
//   Will start executing the ThreadProc function with this class.
//
// Arguments: (none)
//
// Return values:
//   true if thread execution started normally.
//   false otherwise.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
bool ThreadWrapper::Run( void )
{
  DWORD tID;

  // Create event for thread termination.
  m_hQuitEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

  if( m_hQuitEvent != NULL )
  {
    // Create and launch thread.
    m_hThread = CreateThread(NULL, 0, &ThreadWrapper::ThreadProc, (void*)this, 0, &tID);
  }

	return ( m_hThread != NULL );
}


// --------------------------------------------------------------------------
// Function : Stop
//
// Description:
//   Will stop execution of a running thread.
//
// Arguments: (none)
//
// Return values:
//   true if thread execution has stopped.
//   false otherwise.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
bool ThreadWrapper::Stop( void )
{
  // Check thread handle.
  if( m_hThread == NULL )
    return false;

  // Signal the thread that we want to quit.
  SetEvent( m_hQuitEvent );

  // Wait 200ms for the thread to finish up...
  if( WaitForSingleObject( m_hThread, 200 ) != WAIT_OBJECT_0 )
  {
    // Wait for thread failed. Terminate it.
    TerminateThread( m_hThread, 0 );
  }

  // Close Event handle.
  CloseHandle( m_hQuitEvent );

  // Close thread handle.
  CloseHandle( m_hThread );

  return true;
}


// --------------------------------------------------------------------------
// Function : ShouldStop
//
// Description:
//   Verifies if the Quit event is set.
//
// Arguments: (none)
//
// Return values:
//   true if thread should terminate
//   false otherwise.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
bool ThreadWrapper::ShouldStop( void ) const
{
  assert( m_hQuitEvent != NULL );
  return( WaitForSingleObject( m_hQuitEvent, 0 ) == WAIT_OBJECT_0);
}


// --------------------------------------------------------------------------
// Function : ThreadProc        [ STATIC ]
//
// Description:
//   Will start executing the derived work function.
//
// Arguments:
//   lpvParam: void* [IN], this pointer.
//
// Return values:
//   true if thread execution started normally.
//   false otherwise.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
DWORD WINAPI ThreadWrapper::ThreadProc( LPVOID lpvParam )
{
  assert( lpvParam != NULL );

  // Run the Work function of the object.
  ((ThreadWrapper*)lpvParam)->Work();

  return 1;
}

}
