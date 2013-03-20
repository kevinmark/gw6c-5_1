// **************************************************************************
// $Id: pipeclient.h,v 1.3 2007/01/30 18:53:27 cnepveu Exp $
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
//   Defines a specialized IPC Client: The Named pipe client.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cmessaging_pipeclient_h__
#define __gw6cmessaging_pipeclient_h__


#include <gw6cmessaging/ipcclient.h>
#include <gw6cmessaging/pipeio.h>
#include <gw6cmessaging/gw6cuistrings.h>
#include <string>
using namespace std;


namespace gw6cmessaging
{
  // ------------------------------------------------------------------------
  class PipeClient : public IPCClient, public PipeIO
  {
  private:
    HANDLE          m_PipeHandle;
    string          m_PipeName;

  public:
    // Construction / destruction
                    PipeClient            ( const string& aPipeName );
    virtual         ~PipeClient           ( void );

    // IPC Client overrides.
    virtual error_t Connect               ( void );   // Blocking
    virtual error_t Disconnect            ( void );
  };

}

#endif
