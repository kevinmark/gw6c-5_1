// **************************************************************************
// $Id: hap6devicemappingconfig.cc,v 1.2 2007/03/06 21:29:56 cnepveu Exp $
//
// Copyright (c) 2007 Hexago Inc. All rights reserved.
// 
//   LICENSE NOTICE: You may use and modify this source code only if you
//   have executed a valid license agreement with Hexago Inc. granting
//   you the right to do so, the said license agreement governing such
//   use and modifications.   Copyright or other intellectual property
//   notices are not to be removed from the source code.
//
// Implementation of the HAP6DeviceMappingConfig class
//
// Description:
//   Implementation of the HAP6 Device Mapping configuration accessors.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cconfig/hap6devicemappingconfig.h>
#include <gw6cconfig/gw6cuistrings.h>
#include <assert.h>


#ifdef WIN32
#include <direct.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define INET_PTON(A, B, C)                tsp_windows_inet_pton(A, B, C)
#else
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define INET_PTON(A, B, C)                inet_pton(A,B,C)
#endif


#define MAX_DEVICENAME_LEN  64
#define CHRS_DEVICENAME     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-"

#define MAX_IPADDR_LEN      40
#define CHRS_IPADDR         "abcdefABCDEF0123456789.:"


#ifdef WIN32
// --------------------------------------------------------------------------
// Function : tsp_windows_inet_pton    [ WINDOWS SPECIFIC ]
//
// Description:
//   Will translate the string network address in a network structure.
//
// Arguments:
//   af: int [IN], The network address family.
//   src: char* [IN], A string representation of the internet address
//   dst: void* [OUT], The structured representation of the network address 
//                     in a V4 or V6 representation (depending on the AF).
//
// Return values:
//   0 on error, 1 on success.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
int tsp_windows_inet_pton(int af, const char *src, void *dst) 
{
	unsigned long ret;
	char buf[1024];
	int sa_size = 1024;
	
	memset(buf, 0, sizeof(buf));

	if ( (ret = WSAStringToAddress( (LPTSTR)src, af, NULL, (LPSOCKADDR)buf, &sa_size)) != 0)
		return 0;

	if (af == AF_INET6) 
	{
		struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *) buf;
		memcpy(dst, &sa6->sin6_addr, sizeof(struct in6_addr));
		return 1;
	}
	else if ( af == AF_INET) 
	{
		struct sockaddr_in *sa4 = (struct sockaddr_in *) buf;
		memcpy(dst, &sa4->sin_addr, sizeof(struct in_addr));
		return 1;
	}

	return 0;
}
#endif

namespace gw6cconfig
{
// --------------------------------------------------------------------------
// Function : HAP6DeviceMappingConfig constructor
//
// Description:
//   Will initialize a new HAP6DeviceMappingConfig object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
HAP6DeviceMappingConfig::HAP6DeviceMappingConfig( void ) :
  m_pConfig(NULL)
{
}


// --------------------------------------------------------------------------
// Function : HAP6DeviceMappingConfig destructor
//
// Description:
//   Will destroy HAP6DeviceMappingConfig object.
//
// Arguments: (N/A)
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
HAP6DeviceMappingConfig::~HAP6DeviceMappingConfig( void )
{
  // Delete the configuration object, if it was constructed.
  if( m_pConfig !=  NULL )
    delete m_pConfig;

  m_pConfig = NULL;
}


// --------------------------------------------------------------------------
// Function : Initialize
//
// Description:
//   Will initialize the configuration object.
//
// Arguments:
//   aConfigFile: string [IN], The HAP6 Device Mapping configuration file name.
//   aEAccessMode: enum [IN], The desired access mode (READ, CREATE, RW)
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
void HAP6DeviceMappingConfig::Initialize( const string& aConfigFile, const t_accessMode aEAccessMode )
{
  // If instance was already initialized...
  if( m_pConfig != NULL)
    delete m_pConfig;

  // Instantiate the name value configuration parser.
  m_pConfig = new NameValueConfig( aConfigFile, aEAccessMode );
}


// --------------------------------------------------------------------------
// Function : Load
//
// Description:
//   Will load the HAP6 Device Mapping configuration data.
//
// Arguments: (none)
//
// Return values:
//   true if configuration data was successfully loaded from the file.
//
// Exceptions:
//   - See Config::LoadConfiguration()
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::Load( void )
{
  assert( m_pConfig != NULL );

  // Read the configuration file.
  return m_pConfig->LoadConfiguration()  &&  ValidateConfig();
}


// --------------------------------------------------------------------------
// Function : Save
//
// Description:
//   Will save the configuration data.
//
// Arguments: (none)
//
// Return values:
//   true if configuration data has been saved to the file.
//
// Exceptions:
//   - See Config::ApplyConfiguration()
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::Save( void )
{
  assert( m_pConfig != NULL );

  return ValidateConfig()  &&  m_pConfig->ApplyConfiguration();
}


// --------------------------------------------------------------------------
// Function : CancelChanges
//
// Description:
//   Will cancel the changes made to the configuration data.
//
// Arguments: (none)
//
// Return values:
//   true if changes made to the configuration data were successfully 
//   cancelled, false otherwise.
//
// Exceptions:
//   - See Config::CancelConfiguration()
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::CancelChanges( void )
{
  assert( m_pConfig != NULL );

  return m_pConfig->CancelConfiguration();
}


// --------------------------------------------------------------------------
// Function : ValidateConfig
//
// Description:
//   Will loop thru the device mappings and check if the configuration is 
//   valid.
//
// Arguments: (none)
//
// Return values:
//   true if validation has passed.
//
// Exceptions:
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::ValidateConfig( void )
{
  bool bRetCode = true;
  t_stringmap mappings;
  t_stringmap::iterator iter;


  bRetCode = GetDeviceList( mappings );

  for( iter=mappings.begin(); iter != mappings.end(); iter ++ )
  {
    if( !ValidateDeviceName( iter->first )  ||  !ValidateIPAddress( iter->second ) )
    {
      // Mapping is invalid. Remove it.
      DelDeviceMapping( iter->first );
      bRetCode = false;
    }
  }

  return bRetCode;
}


// --------------------------------------------------------------------------
// Function : ValidateDeviceName
//
// Description:
//   Validates a device name.
//
// Arguments: (none)
//
// Return values:
//   true if validation has passed.
//
// Exceptions:
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::ValidateDeviceName( const string& aDeviceName )
{
  if( aDeviceName.empty() )
    return false;

  if( aDeviceName.length() > MAX_DEVICENAME_LEN )
    return false;

  if( aDeviceName.find_first_not_of( CHRS_DEVICENAME ) != string::npos )
    return false;

  return true;
}


// --------------------------------------------------------------------------
// Function : ValidateIPAddress
//
// Description:
//   Validates an IP address.
//
// Arguments: (none)
//
// Return values:
//   true if validation has passed.
//
// Exceptions:
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::ValidateIPAddress( const string& aIPAddress )
{
  if( aIPAddress.empty() )
    return false;

  if( aIPAddress.length() > MAX_IPADDR_LEN )
    return false;

  if( aIPAddress.find_first_not_of( CHRS_IPADDR ) != string::npos )
    return false;


  // Check if IPv4 or IPv6 address
  struct in6_addr addressv6;
  struct in_addr  addressv4;
  unsigned long net;

  net = inet_addr( aIPAddress.c_str() );
  memcpy(&addressv4, &net, sizeof(net));

  if( aIPAddress != inet_ntoa(addressv4)  &&  (INET_PTON(AF_INET6, aIPAddress.c_str(), &addressv6) <= 0) )
    return false;

  return true;
}


// --------------------------------------------------------------------------
// Function : AddDeviceMapping
//
// Description:
//   Will add a new device mapping to the configuration file.
//   NOTE: This function will update the address if the device name specified
//         already exists.
//
// Arguments:
//   aName: string [IN], The device name.
//   aAddress: string[IN], The device (IPv6 or IPv4) address.
//
// Return values:
//   true if device mapping has been successfully added (or updated).
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::AddDeviceMapping( const string& aName, const string& aAddress )
{
  assert( m_pConfig != NULL );

  m_pConfig->SetVariableValue( aName, aAddress );

  return true;
}


// --------------------------------------------------------------------------
// Function : DelDeviceMapping
//
// Description:
//   Will remove a device mapping from the configuration file.
//
// Arguments:
//   aName: string [IN], The device name to remove.
//
// Return values:
//   true if device mapping has been successfully removed.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::DelDeviceMapping( const string& aName )
{
  assert( m_pConfig != NULL );

  m_pConfig->RemoveVariable( aName );

  return true;
}


// --------------------------------------------------------------------------
// Function : GetDeviceAddress
//
// Description:
//   Will retrieve the address of the specified device name from the 
//   configuration file.
//
// Arguments:
//   aName: string [IN], The device name.
//
// Return values:
//   true if address has been successfully retrieved.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::GetDeviceAddress( const string& aName, string& aAddress ) const
{
  string sAddress;
  assert( m_pConfig != NULL );

  m_pConfig->GetVariableValue( aName, aAddress );

  return true;
}


// --------------------------------------------------------------------------
// Function : SetDeviceAddress
//
// Description:
//   Will set(overwrite) the address of the specified device name from the 
//   configuration file.
//   NOTE: This function will create the mapping if the device name specified
//         does not exists.
//
// Arguments:
//   aName: string [IN], The device name.
//   aAddress: string[IN], The new device (IPv6 or IPv4) address.
//
// Return values:
//   true if mapping has been successfully updated (or created).
//
// Exceptions:
//   - When device mapping doesn't exists.
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::SetDeviceAddress( const string& aName, const string& aAddress )
{
  assert( m_pConfig != NULL );

  m_pConfig->SetVariableValue( aName, aAddress );

  return true;
}


// --------------------------------------------------------------------------
// Function : GetDeviceNameList
//
// Description:
//   Will retrieve a list of device names that are present in the HAP6 
//   Device Mapping configuration file.
//
// Arguments: (none)
//
// Return values:
//   true if device names were successfully enumerated.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::GetDeviceNameList( t_stringarray& aNameList ) const
{
  stringlist nameList;
  stringlist::const_iterator iter;

  m_pConfig->GetVariableNameList( nameList );

  for( iter=nameList.begin(); iter != nameList.end() ; iter++ )
    aNameList.push_back( *iter );

  return true;
}


// --------------------------------------------------------------------------
// Function : GetDeviceList
//
// Description:
//   Will retrieve a list of device names and their associated address that 
//   are present in the HAP6 Device Mapping configuration file.
//
// Arguments: (none)
//
// Return values:
//   true if device mapping was successfully extracted.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
bool HAP6DeviceMappingConfig::GetDeviceList( t_stringmap& aDeviceMap ) const
{
  stringlist nameList;
  stringlist::const_iterator iter;
  string sValue;

  m_pConfig->GetVariableNameList( nameList );

  for( iter=nameList.begin(); iter != nameList.end() ; iter++ )
  {
    m_pConfig->GetVariableValue( *iter, sValue );
    aDeviceMap[ *iter ] = sValue;
  }

  return true;
}

} // Namespace
