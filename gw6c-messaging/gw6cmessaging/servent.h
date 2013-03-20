// **************************************************************************
// $Id: servent.h,v 1.7 2007/01/30 18:53:27 cnepveu Exp $
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
//   This component will be used by the messaging layer to communicate 
//   through the IPC. Prior to use the Servent component, an application must
//   register a IPCServent-derived object. The Initialize method will 
//   initialize the IPCServent component. The SendData and ReceiveData 
//   methods are standard IO routines used to send and receive user data. 
//   The Servent will implement a way of fragmenting user data in several 
//   packets before sending them on the IPC medium. Also, it will be able to 
//   reconstitute the fragmented user data upon reception.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_servent_h__
#define __gw6cmessaging_servent_h__


#include <gw6cmessaging/ipcservent.h>
#include <gw6cmessaging/semaphore.h>


namespace gw6cmessaging
{
  typedef unsigned long long counter_t;

  // Type definitions.
  typedef struct __SERVENT_INFO
  {
    counter_t       nTtlBytesRead;
    counter_t       nTtlBytesWritten;
  } SERVENT_INFO, *PSERVENT_INFO;


  // ------------------------------------------------------------------------
  class Servent
  {
  private:
    IPCServent*     m_pIPCServent;      // IPC server/client connectivity.
    counter_t       m_nTtlBytesRead;    // Bytes received counter.
    counter_t       m_nTtlBytesWritten; // Bytes sent counter.
    Semaphore*      m_pSemIPCMutex;     // Mutex for IO operations on IPC.

  public:
    // Construction / destruction
                    Servent               ( void );
    virtual         ~Servent              ( void );

    // Initialization routine.
    error_t         Initialize            ( IPCServent* pIPCServent );
    bool            WaitReady             ( unsigned long ulWaitms );

    // IO operation routines.
    error_t         ReadData              ( void* pvReadBuffer, const size_t nBufferSize, size_t& nRead );
    error_t         WriteData             ( const void* pvData, const size_t nDataSize, size_t& nWritten );
    error_t         CanRead               ( bool& bCanRead );
    error_t         CanWrite              ( bool& bCanWrite );

    // Object Statistics info.
    void            GetServentInfo        ( PSERVENT_INFO pObj );

  private:
    error_t         _ReadData             ( void* pvReadBuffer, const size_t nBufferSize, size_t& nRead );
    error_t         _WriteData            ( const void* pvData, const size_t nDataSize, size_t& nWritten );
  };

}

#endif
