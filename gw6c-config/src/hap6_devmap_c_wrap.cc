// **************************************************************************
// $Id: hap6_devmap_c_wrap.cc,v 1.2 2007/03/06 21:29:56 cnepveu Exp $
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
//   Wraps the HAP6 Device Mapping configuration data to offer C access.
//
// Author: Charles Nepveu
//
// Creation Date: Febuary 2007
// __________________________________________________________________________
// **************************************************************************
#include <gw6cconfig/hap6_devmap_c_wrap.h>
#include <gw6cconfig/hap6devicemappingconfig.h>
#include <gw6cconfig/gw6cuistrings.h>
using namespace gw6cconfig;
#include <assert.h>

#ifdef WIN32
#define strcasecmp      _stricmp
#define strdup          _strdup
#endif


// The instance holder for the HAP6 Device Mapping configuration data object.
static HAP6DeviceMappingConfig* gpConfig = NULL;


// --------------------------------------------------------------------------
// Function : init_hap6_devmap
//
// Description:
//   Will create a HAP6 Device Mapping configuration object and load the 
//   configuration data.
//
// Arguments:
//   szFileName: char* [IN], The name of the configuration file to load.
//
// Return values:
//   * 0 on successful initialization,
//   * -1 means that some mappings were removed because they were invalid.
//   * any other positive value indicates a severe error and should be
//     logged using get_ui_string().
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
extern "C" int init_hap6_devmap( const char* szFileName )
{
  int iRet = 0;

  // Check if already initialized.
  if( gpConfig != NULL )
    return iRet;

  try
  {
    // Create new instance, initialize...
    gpConfig = new HAP6DeviceMappingConfig();
    gpConfig->Initialize( szFileName, AM_READ );  // READ ONLY
    
    // ...and load configuration data.
    iRet = gpConfig->Load() ? 0 : -1;
  }
  catch( error_t nErr )
  {
    iRet = nErr;
  }  

  return iRet;
}


// --------------------------------------------------------------------------
// Function : reload_hap6_devmap
//
// Description:
//   Will reload the HAP6 Device Mapping configuration data.
//
// Arguments: (none)
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
extern "C" int reload_hap6_devmap( void )
{
  assert( gpConfig != NULL );
  int iRet = 0;

  try
  {
    // Re-load the configuratrion.
    iRet = gpConfig->Load() ? 0 : -1;
  }
  catch( error_t nErr )
  {
    iRet = nErr;
  }

  return iRet;
}


// --------------------------------------------------------------------------
// Function : uninit_hap6_devmap
//
// Description:
//   Will clean-up the HAP6 Device Mapping configuration data object.
//
// Arguments: (none)
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
extern "C" void uninit_hap6_devmap( void )
{
  if( gpConfig != NULL )
    delete gpConfig;

  gpConfig = NULL;
}


// --------------------------------------------------------------------------
// Function : get_device_mapping
//
// Description:
//   Will create a chained list containing the device mapping found in the
//   HAP6 Device Mapping File.
//
// Arguments:
//   ppMapping: DEVICE_MAPPING double reference. To be filled.
//
// Return values:
//   0 on success,
///  any other value indicates an error.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
extern "C" int get_device_mapping( PDEVICE_MAPPING* ppDeviceMapping )
{
  assert( gpConfig != NULL );
  assert( ppDeviceMapping != NULL );
  assert( *ppDeviceMapping == NULL ); // Will be instantiated here.


  t_stringmap deviceMap;
  t_stringmap::const_iterator iter;
  PDEVICE_MAPPING* current = ppDeviceMapping;

  try
  {
    // Get the device mapping list.
    if( !gpConfig->GetDeviceList( deviceMap ) )
      return -1;

    for( iter=deviceMap.begin(); iter != deviceMap.end(); iter++ )
    {
      if( ((*current) = (PDEVICE_MAPPING) malloc( sizeof(DEVICE_MAPPING) )) != NULL )
      {
        (*current)->szName      = strdup( iter->first.c_str() );
        (*current)->szAddress   = strdup( iter->second.c_str() );
        (*current)->next        = NULL;
      }
      current = &((*current)->next);
    }
  }
  catch( error_t err )
  {
    return (int)err;
  }

  return 0;
}


// --------------------------------------------------------------------------
// Function : get_device_mapping
//
// Description:
//   Will free a linked list created by get_device_mapping()
//
// Arguments:
//   ppMapping: DEVICE_MAPPING double reference. To be freed.
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
extern "C" void free_device_mapping( PDEVICE_MAPPING* ppDeviceMapping )
{
  assert( ppDeviceMapping != NULL );

  if( (*ppDeviceMapping) != NULL )  // If not end of linked list.
  {
    // Recurse.
    free_device_mapping( &((*ppDeviceMapping)->next) );
    assert( (*ppDeviceMapping)->next == NULL );

    // Delete this instance.
    free( (*ppDeviceMapping)->szName );
    free( (*ppDeviceMapping)->szAddress );
    free( (*ppDeviceMapping) );

    (*ppDeviceMapping) = NULL;
  }
}
