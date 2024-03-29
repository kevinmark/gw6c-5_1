// **************************************************************************
// $Id: parser.h,v 1.4 2007/01/30 18:53:08 cnepveu Exp $
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
//   Provides a generic means of parsing a file.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gw6cconfig_parser_h__
#define __gw6cconfig_parser_h__


#include <string>
using namespace std;


namespace gw6cconfig
{
  // ------------------------------------------------------------------------
  class Parser
  {
  protected:
    // Construction / destruction
                    Parser                ( void ) {;};
  public:
    virtual         ~Parser               ( void ) {;};

  public:
    // Pure abstract
    virtual bool    ReadConfigurationData ( const string& aFilename ) = 0;
    virtual bool    WriteConfigurationData( const string& aFilename ) = 0;
  };

}

#endif
