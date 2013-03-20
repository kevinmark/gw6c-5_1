// **************************************************************************
// $Id: ipcclient.h,v 1.4 2007/01/30 18:53:26 cnepveu Exp $
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
//   Defines a generic IPC Client.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_ipcclient_h__
#define __gw6cmessaging_ipcclient_h__


#include <gw6cmessaging/ipcservent.h>


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class IPCClient : virtual public IPCServent
  {
  protected:
    // Construction / destruction
                    IPCClient             ( void );
  public:
    virtual         ~IPCClient            ( void );

    // IPC Servent overrides.
    virtual error_t Initialize            ( void );   // Blocking
    virtual error_t UnInitialize          ( void );   // Blocking
    virtual bool    WaitReady             ( unsigned long ulWaitms );

    // IPC Client operations.
    virtual error_t Connect               ( void ) = 0;   // Blocking
    virtual error_t Disconnect            ( void ) = 0;
  };

}

#endif
