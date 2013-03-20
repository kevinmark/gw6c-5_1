// **************************************************************************
// $Id: semaphore.h,v 1.2 2007/01/30 18:53:27 cnepveu Exp $
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
//   Semaphore wrapper for Windows & other platforms.
//
// Author: Charles Nepveu
//
// Creation Date: December 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_semaphore_h__
#define __gw6cmessaging_semaphore_h__


#ifdef WIN32
#include <windows.h>
#else
#include <semaphore.h>
#endif


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class Semaphore
  {
  protected:
#ifdef WIN32
    HANDLE          m_Semaphore;          // Semaphore windows handle.
#else
    sem_t           m_Semaphore;          // Semaphore *nix struct.
#endif

  public:
                    Semaphore             ( unsigned int nMaxCount=1, unsigned int nInitialCount=1 );
                    ~Semaphore            ( void );

    int             WaitAndLock           ( unsigned long ulWaitms=0 ); // Blocking
    int             ReleaseLock           ( void );
  };

}

#endif
