// **************************************************************************
// $Id: threadwrapper.h,v 1.7 2007/01/30 18:53:27 cnepveu Exp $
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
//   Thread wrapper for Windows & other platforms.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_threadwrapper_h__
#define __gw6cmessaging_threadwrapper_h__


#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#define WINAPI
#define DWORD void*        // To match pthread threadproc prototype.
#endif


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class ThreadWrapper
  {
  protected:
#ifdef WIN32
    HANDLE          m_hThread;          // Thread handle.
    HANDLE          m_hQuitEvent;       // Handle for quit event.
#else
    pthread_t       m_tID;              // Thread identifier
    bool            m_bShouldStop;      // Thread is required to terminate.
#endif

    // Construction / destruction.
  protected:
                    ThreadWrapper       ( void );
  public:
    virtual         ~ThreadWrapper      ( void );

    // Start and stop the thread.
    bool            Run                 ( void );
    bool            Stop                ( void );

  protected:
    // The work function to be called by Run().
    virtual void    Work                ( void )=0;
    bool            ShouldStop          ( void ) const;

  private:
    static DWORD WINAPI ThreadProc      ( void* lpvParam );
  };

}

#endif
