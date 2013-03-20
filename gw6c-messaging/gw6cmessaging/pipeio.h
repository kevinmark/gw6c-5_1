// **************************************************************************
// $Id: pipeio.h,v 1.4 2007/01/30 18:53:27 cnepveu Exp $
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
//   Defines a specialized way of transferring data using IPC: Pipes!
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_pipeio_h__
#define __gw6cmessaging_pipeio_h__


#include <gw6cmessaging/ipcservent.h>
#include <gw6cmessaging/gw6cuistrings.h>


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class PipeIO : virtual public IPCServent
  {
  public:
    // Construction / destruction
                    PipeIO                ( void );
    virtual         ~PipeIO               ( void );

    // Overrides from IPC Servent
    virtual error_t CanRead               ( bool &bCanRead ) const;   // Non-Blocking
    virtual error_t CanWrite              ( bool &bCanWrite ) const;  // Non-Blocking

    virtual error_t Read                  ( void* pvReadBuffer, 
                                            const size_t nBufferSize, 
                                            size_t& nRead );          // Blocking
    virtual error_t Write                 ( const void* pvData, 
                                            const size_t nDataSize, 
                                            size_t& nWritten );       // Blocking
  };

}

#endif
