// **************************************************************************
// $Id: pipeserver.h,v 1.4 2007/01/30 18:53:27 cnepveu Exp $
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
//   Defines a specialized IPC Server: The Named pipe server.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_pipeserver_h__
#define __gw6cmessaging_pipeserver_h__


#include <gw6cmessaging/ipcserver.h>
#include <gw6cmessaging/pipeio.h>
#include <gw6cmessaging/gw6cuistrings.h>
#include <string>
using namespace std;


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class PipeServer : public IPCServer, public PipeIO
  {
  private:
    string          m_PipeName;
    bool            m_bClientConnected;

  public:
    // Construction / destruction
                    PipeServer            ( const string& aPipeName );
    virtual         ~PipeServer           ( void );

    // IPC Server overrides.
    virtual error_t CreateConnectionPoint ( void );   // Non-Blocking
    virtual error_t AcceptConnection      ( void );   // Blocking
    virtual error_t CloseConnection       ( void );   // Non-Blocking
  };

}

#endif
