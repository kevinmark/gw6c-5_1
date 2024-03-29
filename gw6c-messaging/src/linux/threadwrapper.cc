// **************************************************************************
// $Id: threadwrapper.cc,v 1.3 2007/01/30 18:53:32 cnepveu Exp $
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
//   Thread wrapper implementation for unix systems, using pthreads.
//
// Author: Charles Nepveu
//
// Creation Date: December 2006
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
#ifdef WIN32
  m_hThread(NULL), 
  m_hQuitEvent(NULL)
#else
  m_tID(0),
  m_bShouldStop(false)
#endif
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
  pthread_attr_t attr;
  int retCode;


  // Initialize thread attributes
  if( pthread_attr_init(&attr) != 0 ) 
    return false;

  // The thread must be joinable.
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // Launch thread procedure.
  retCode = pthread_create( &m_tID, &attr, &ThreadWrapper::ThreadProc, (void*)this );

  // We don't need the thread creation argument anymore.
  pthread_attr_destroy(&attr);

  // Return completion.
  return retCode == 0;
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
  m_bShouldStop = true;

  // Wait for thread to exit.
  int retCode = pthread_join( m_tID, NULL );

  return retCode == 0;
}


// --------------------------------------------------------------------------
// Function : ShouldStop
//
// Description:
//   Verifies if the running thread should exit (called periodically from the
//   running thread).
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
  return m_bShouldStop;
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
DWORD WINAPI ThreadWrapper::ThreadProc( void* lpvParam )
{
  assert( lpvParam != NULL );

  // Run the Work function of the object.
  ((ThreadWrapper*)lpvParam)->Work();

  return (DWORD)1;
}

}
