// **************************************************************************
// $Id: semaphore.cc,v 1.2 2007/01/30 18:53:32 cnepveu Exp $
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
//   POSIX Semaphore wrapper. Use -lrt link option.
//
// Author: Charles Nepveu
//
// Creation Date: December 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/semaphore.h>
#include <assert.h>
#include <unistd.h>


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
Semaphore::Semaphore( unsigned int nMaxCount, unsigned int nInitialCount )
{
  assert( nMaxCount >= nInitialCount );
  int i;

  i = sem_init( &m_Semaphore, 0, nMaxCount );
  assert( i == 0 );

  // Lock a certain count.
  for( i=nMaxCount-nInitialCount; i>0; i-- )
    WaitAndLock();
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
  sem_destroy( &m_Semaphore );
}


// --------------------------------------------------------------------------
// Function : WaitAndLock
//
// Description:
//   Blocks execution until semaphore object is available.
//   Locks (decrements) semaphore count.
//
// Arguments:
//   ulWaitms: long [IN], The time to wait until state is signalled.
//                        If 0, the timeout is infinite.
//
// Return values:
//   0: Successfuly obtained lock on semaphore object
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
int Semaphore::WaitAndLock( unsigned long ulWaitms )
{
  int retCode = -1;

  if( ulWaitms > 0 )
  {
    unsigned long wait = 0;

    do
    {
      retCode = sem_trywait( &m_Semaphore );
      usleep( 25000 ); wait += 25;
    }
    while( retCode != 0  &&  wait < ulWaitms );
  }
  else
    retCode = sem_wait( &m_Semaphore );

  return retCode;
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
//   0: Successfuly released lock on semaphore object.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
int Semaphore::ReleaseLock( void )
{
  return sem_post( &m_Semaphore );
}


} // namespace
