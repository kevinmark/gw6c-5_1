// **************************************************************************
// $Id: ipcservent.h,v 1.9 2007/01/30 18:53:26 cnepveu Exp $
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
//   Defines a generic IPC servent. A servent may be a Server or Client.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_ipcservent_h__
#define __gw6cmessaging_ipcservent_h__


#include <gw6cmessaging/gw6cuistrings.h>
#include <gw6cmessaging/semaphore.h>
#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
#else
typedef void*                 HANDLE;
#define INVALID_HANDLE_VALUE  ((HANDLE)-1)
#endif


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class IPCServent
  {
  protected:
    HANDLE          m_Handle;         // IPC Handle for Connection & IO operations.
    Semaphore*      m_pSemReadyState; // Semaphore released when ready state is reached.

  protected:
    // Construction / destruction
                    IPCServent            ( void ) : m_Handle(INVALID_HANDLE_VALUE), 
                                                     m_pSemReadyState(0) {};
  public:
    virtual         ~IPCServent           ( void ) { };

    // Connection operations.
    virtual error_t Initialize            ( void )=0;             // Blocking
    virtual error_t UnInitialize          ( void )=0;             // Blocking
    virtual bool    WaitReady             ( unsigned long ulWaitms )=0;

    // IO operations
    virtual error_t CanRead               ( bool& bCanRead ) const = 0;   // Non-Blocking
    virtual error_t CanWrite              ( bool& bCanWrite ) const = 0;  // Non-Blocking
    virtual error_t Read                  ( void* pvReadBuffer, 
                                            const size_t nBufferSize, 
                                            size_t& nRead )=0;    // Blocking
    virtual error_t Write                 ( const void* pvData, 
                                            const size_t nDataSize, 
                                            size_t& nWritten )=0; // Blocking
  };

}

#endif
