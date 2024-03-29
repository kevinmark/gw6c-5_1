// **************************************************************************
// $Id: config.cc,v 1.4 2007/01/30 18:53:09 cnepveu Exp $
//
// Copyright (c) 2007 Hexago Inc. All rights reserved.
// 
//   LICENSE NOTICE: You may use and modify this source code only if you
//   have executed a valid license agreement with Hexago Inc. granting
//   you the right to do so, the said license agreement governing such
//   use and modifications.   Copyright or other intellectual property
//   notices are not to be removed from the source code.
//
// Implementation of the Config class
//
// Description:
//   Provides a simple implementation to read configuration data, write or
//   discard changes done to the configuration data.
//   NOTE: This class cannot be instantiated directly. It is meant to be 
//         overriden and specialized.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cconfig/config.h>
#include <gw6cconfig/gw6cuistrings.h>
#include <assert.h>


namespace gw6cconfig
{
// --------------------------------------------------------------------------
// Function : Config constructor [PROTECTED]
//
// Description:
//   Will initialize a new Config object.
//
// Arguments:
//   aConfigFile: string [IN], The configuration file name.
//   aEAccessMode: enum [IN], Contains the access mode.
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
Config::Config( const string& aConfigFile, const t_accessMode aEAccessMode ) : 
  m_pParser(NULL), 
  m_eAccessMode(aEAccessMode), 
  m_sConfigFile(aConfigFile)
{
}


// --------------------------------------------------------------------------
// Function : Config destructor
//
// Description:
//   Will destroy the class.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
Config::~Config( void )
{
}


// --------------------------------------------------------------------------
// Function : LoadConfiguration
//
// Description:
//   Will attempt to load the configuration data from the configuration file.
//   Operation specifics are delegated to the Parser object.
//
// Arguments: (none)
//
// Return values:
//   true if parser successfully parsed the configuration data, false 
//   otherwise.
//
// Exceptions:
//   - When access mode is incoherent with operation.
//   See Parser::ReadConfigurationData().
//
// --------------------------------------------------------------------------
bool Config::LoadConfiguration( void )
{
  assert( m_pParser != NULL );    // Need to have a parser!
  
  // If access mode is CREATE, we're not suposed to load any existing 
  // configuration.
  if( m_eAccessMode == AM_CREATE )
  {
    throw GW6C_UIS__CFG_CANNOTLOADWHENCREATE;
  }
  
  // Read configuration.
  // May throw exception if file doesn't exist.
  return m_pParser->ReadConfigurationData( m_sConfigFile );
}


// --------------------------------------------------------------------------
// Function : ApplyConfiguration
//
// Description:
//   Will apply changes made to the configuration data. This is done by the
//   parser object which will write the changes to the medium.
//
// Arguments: (none)
//
// Return values:
//   true if changes have been saved, false otherwise.
//
// Exceptions:
//   See Parser::WriteConfigurationData().
//
// --------------------------------------------------------------------------
bool Config::ApplyConfiguration( void )
{
  assert( m_pParser != NULL );    // Need to have a parser!
  
  // If access mode is READ only, we're not suposed to write any existing 
  // configuration data.
  if( m_eAccessMode == AM_READ )
  {
    throw GW6C_UIS__CFG_CANNOTAPPLYWHENREAD;
  }

  // Apply the changes to disk.
  return m_pParser->WriteConfigurationData( m_sConfigFile );  
}


// --------------------------------------------------------------------------
// Function : CancelConfiguration
//
// Description:
//   Will cancel any changes made to the configuration data since the last
//   write operation(apply), if any. This is done by simply re-loading the 
//   configuration from the parser object.
//
// Arguments: (none)
//
// Return values:
//   true if changes have been cancelled, false otherwise.
//
// Exceptions:
//   - When access mode is READ only.
//   See LoadConfiguration().
//
// --------------------------------------------------------------------------
bool Config::CancelConfiguration( void )
{
  assert( m_pParser != NULL );    // Need to have a parser!

  // If access mode is READ only, we're not suposed to be cancelling changes.
  if( m_eAccessMode == AM_READ )
  {
    throw GW6C_UIS__CFG_CANNOTCANCELWHENREAD;
  }

  // Cancelling changes is the same as re-loading configuration data.
  return LoadConfiguration();
}


// --------------------------------------------------------------------------
// Function : OverrideConfiguration
//
// Description:
//   Will override the current configuration data with the one from the 
//   specified file. Then, the new configuration data will be written to the 
//   current configuration file, thus completing the operation.
//
// Arguments: 
//   aFileName: string [IN], The alternate configuration file to load, which
//              will override the current configuration.
//
// Return values:
//   true if the current configuration data has been overriden by the data 
//   found in aFileName.
//
// Exceptions:
//   - When the specified file is the same as the configuration file.
//   - When access mode is READ only.
//   - See Parser::ReadConfigurationData()
//   - See Parser::WriteConfigurationData()
//
// --------------------------------------------------------------------------
bool Config::OverrideConfiguration( const string& aFileName )
{
  assert( m_pParser != NULL );
  
  // Check if we're overriding with the same data from the same file.
  if( aFileName == m_sConfigFile )
  {
    throw GW6C_UIS__CFG_CANNOTOVERRIDESAMECONTENTS;
  }

  // If access mode is READ only, we're not suposed to be overriding 
  // configuration data.
  if( m_eAccessMode == AM_READ )
  {
    throw GW6C_UIS__CFG_CANNOTOVERRIDEWHENREAD;
  }

  // Override the configuration data with the one from the specified file.
  // and apply the configuration to the config file.
  return m_pParser->ReadConfigurationData( aFileName ) &&
         m_pParser->WriteConfigurationData( m_sConfigFile );  
}

}
