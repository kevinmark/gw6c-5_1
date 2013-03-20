// **************************************************************************
// $Id: semaphore.cc,v 1.2 2007/01/30 18:53:33 cnepveu Exp $
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
//   Windows Semaphore wrapper.
//
// Author: Charles Nepveu
//
// Creation Date: December 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/semaphore.h>
#include <assert.h>


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : Semaphore constructor
//
// Description:
//   Will initialize a new Semaphore object.
//
// Arguments:
//   nCount: int [IN], The initial count of semaphore
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
Semaphore::Semaphore( unsigned int nMaxCount, unsigned int nInitialCount ):
  m_Semaphore(NULL)
{
  m_Semaphore = CreateSemaphore( NULL, nInitialCount, nMaxCount, NULL );
  assert( m_Semaphore != NULL );
}


// --------------------------------------------------------------------------
// Function : Semaphore destructor
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
Semaphore::~Semaphore( void )
{
  if( m_Semaphore != NULL )
  {
    CloseHandle( m_Semaphore );
    m_Semaphore = NULL;
  }
}


// --------------------------------------------------------------------------
// Function : WaitAndLock
//
// Description:
//   Blocks execution until semaphore object is available.
//   Locks (decrements) semaphore count.
//
// Arguments:
//   ulWaitms: The number of miliseconds to wait until state is signalled.
//             If 0, the wait time is INFINITE.
//
// Return values:
//   0: Successfuly obtained lock on semaphore object
//  -1: Invalid semaphore object, or an error occured.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
int Semaphore::WaitAndLock( unsigned long ulWaitms )
{
  DWORD dwWait = (ulWaitms!=0) ? ulWaitms : INFINITE;

  return ( (m_Semaphore != NULL) && (WaitForSingleObject( m_Semaphore, dwWait ) == WAIT_OBJECT_0) ) ? 0 : -1;
}


// --------------------------------------------------------------------------
// Function : ReleaseLock
//
// Description:
//   Releases lock on semaphore object (increments semaphore count).
//
// Arguments: (none)
//
// Return values:
//   0: Successfuly released lock on semaphore object
//  -1: Invalid semaphore object, or an error occured.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
int Semaphore::ReleaseLock( void )
{
  return ( (m_Semaphore != NULL) && (ReleaseSemaphore( m_Semaphore, 1, NULL ) == TRUE) ) ? 0 : -1;;
}


} // namespace
