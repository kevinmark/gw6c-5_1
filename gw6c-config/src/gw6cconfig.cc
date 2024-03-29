// **************************************************************************
// $Id: gw6cconfig.cc,v 1.21 2007/11/12 17:29:32 cnepveu Exp $
//
// Copyright (c) 2007 Hexago Inc. All rights reserved.
//
//   LICENSE NOTICE: You may use and modify this source code only if you
//   have executed a valid license agreement with Hexago Inc. granting
//   you the right to do so, the said license agreement governing such
//   use and modifications.   Copyright or other intellectual property
//   notices are not to be removed from the source code.
//
// Implementation of the GW6CConfig class
//
// Description:
//   Implementation of the Gateway6 Client configuration data accessors.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************

#include <gw6cconfig/gw6cconfig.h>
#include <gw6cconfig/gw6cvalidation.h>
#include <gw6cconfig/gw6cuistrings.h>
#include <assert.h>
#include <string>


// Configuration data variable NAMES
#define CFG_STR_USERID            "userid"
#define CFG_STR_PASSWD            "passwd"
#define CFG_STR_SERVER            "server"
#define CFG_STR_HOSTTYPE          "host_type"
#define CFG_STR_PREFIXLEN         "prefixlen"
#define CFG_STR_IFPREFIX          "if_prefix"
#define CFG_STR_DNSSERVER         "dns_server"
#define CFG_STR_GW6CDIR           "gw6_dir"
#define CFG_STR_AUTHMETHOD        "auth_method"
#define CFG_STR_AUTORETRYCONNECT  "auto_retry_connect"
#define CFG_STR_RETRYDELAY        "retry_delay"
#define CFG_STR_KEEPALIVE         "keepalive"
#define CFG_STR_KEEPALIVEINTERVAL "keepalive_interval"
#define CFG_STR_TUNNELMODE        "tunnel_mode"
#define CFG_STR_IFTUNV6V4         "if_tunnel_v6v4"
#define CFG_STR_IFTUNV6UDPV4      "if_tunnel_v6udpv4"
#define CFG_STR_IFTUNV4V6         "if_tunnel_v4v6"
#define CFG_STR_CLIENTV4          "client_v4"
#define CFG_STR_CLIENTV6          "client_v6"
#define CFG_STR_TEMPLATE          "template"
#define CFG_STR_PROXYCLIENT       "proxy_client"
#define CFG_STR_BROKERLIST        "broker_list"
#define CFG_STR_LASTSERVER        "last_server"
#define CFG_STR_ALWAYSUSELASTSVR  "always_use_same_server"
#define CFG_STR_LOG               "log"
#define CFG_STR_LOGFILENAME       "log_filename"
#define CFG_STR_LOGROTATION       "log_rotation"
#define CFG_STR_LOGROTATIONSZ     "log_rotation_size"
#define CFG_STR_LOGROTATIONDEL    "log_rotation_delete"
#define CFG_STR_SYSLOGFACILITY    "syslog_facility"
#define CFG_STR_HAP6PROXYENABLED  "hap6_proxy_enabled"
#define CFG_STR_HAP6WEBENABLED    "hap6_web_enabled"
#define CFG_STR_HAP6DOCUMENTROOT  "hap6_document_root"


// Configuration data DEFAULT VALUES
// These values will be pushed to the configuration data if they were not
// found in the configuration file.
//  IMPORTANT !! Make precautions when modifying these values, because they
//               won't be validated!
#define CFG_DFLT_HOSTTYPE         "host"
#define CFG_DFLT_PREFIXLEN        "64"
#define CFG_DFLT_IFPREFIX         ""
#define CFG_DFLT_GW6CDIR          ""
#define CFG_DFLT_AUTHMETHOD       STR_ANY
#define CFG_DFLT_AUTORETRYCONNECT STR_YES
#define CFG_DFLT_RETRYDELAY       "30"
#define CFG_DFLT_KEEPALIVE        STR_YES
#define CFG_DFLT_KEEPALIVEINTERVAL "30"
#define CFG_DFLT_TUNNELMODE       "v6anyv4"
#define CFG_DFLT_CLIENTV4         "auto"
#define CFG_DFLT_CLIENTV6         "auto"
#define CFG_DFLT_PROXYCLIENT      STR_NO
#define CFG_DFLT_BROKERLIST       "tsp-broker-list.txt"
#define CFG_DFLT_LASTSERVER       "tsp-last-server.txt"
#define CFG_DFLT_ALWAYSUSELASTSVR STR_NO
#define CFG_DFLT_LOGFILENAME      "gw6c.log"
#define CFG_DFLT_LOGROTATION      STR_YES
#define CFG_DFLT_LOGROTATIONSZ    "32"
#define CFG_DFLT_LOGROTATIONDEL   STR_NO
#define CFG_DFLT_SYSLOGFACILITY   "USER"
#define CFG_DFLT_HAP6PROXYENABLED STR_NO        // HAP6 defaults
#define CFG_DFLT_HAP6WEBENABLED   STR_NO
#define CFG_DFLT_HAP6DOCUMENTROOT ""

#ifdef WIN32
#define CFG_DFLT_LOGLEVEL_STDERR  "0"
#define CFG_DFLT_LOGLEVEL_SYSLOG  "0"
#define CFG_DFLT_LOGLEVEL_CONSOLE "0"
#define CFG_DFLT_LOGLEVEL_FILE    "1"
#else
#define CFG_DFLT_LOGLEVEL_STDERR  "1"
#define CFG_DFLT_LOGLEVEL_SYSLOG  "0"
#define CFG_DFLT_LOGLEVEL_CONSOLE "0"
#define CFG_DFLT_LOGLEVEL_FILE    "0"
#endif
#define CFG_DFLT_LOGLEVEL         "1"   // When unknown device.


// --------------------------------------------------------------------------
// Macro definitions
// --------------------------------------------------------------------------

// For configuration data accessors (GET)
#define ASSERT_VALID_CONFIG                   \
  assert( m_pConfig != NULL );                \
  if( !m_bValid )                             \
    throw GW6C_UIS__G6C_INVALIDCONF

// For configuration data accessors (SET)
#define VERIFY_AND_SET(X,Y)                   \
  if( Validate_##X( s##X ) )                  \
    m_pConfig->SetVariableValue( Y, s##X );   \
  else                                        \
    throw GetLastError()

// For configuration data validation.
#define VALIDATE_LOGERRMSG( X, Y )                    \
  m_pConfig->GetVariableValue( Y, sValue );           \
  if( !Validate_##X( sValue ) )                       \
  {                                                   \
    m_lsValidationErrors.push_back( GetLastError() ); \
    m_bValid = false;                                 \
  }


// --------------------------------------------------------------------------
// Function : GetDfltLogLevelForDevice
//
// Description:
//   Will return the default log level for the specified log device.
//
// Arguments:
//   sLogDevice: string [IN], The log device.
//
// Return values:
//   A string representing the default log level for the device.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
string GetDfltLogLevelForDevice( const string& sLogDevice )
{
  if( sLogDevice == STR_LOGDEV_CONSOLE )
    return CFG_DFLT_LOGLEVEL_CONSOLE;

  if( sLogDevice == STR_LOGDEV_STDERR )
    return CFG_DFLT_LOGLEVEL_STDERR;

  if( sLogDevice == STR_LOGDEV_FILE )
    return CFG_DFLT_LOGLEVEL_FILE;

  if( sLogDevice == STR_LOGDEV_SYSLOG )
    return CFG_DFLT_LOGLEVEL_SYSLOG;

  // Should assert(false) - here -
  return CFG_DFLT_LOGLEVEL;
}


// --------------------------------------------------------------------------
// Replaces the occurence of a string with another.
// --------------------------------------------------------------------------
void replace( string& str, const char* s, const char* r )
{
  assert( strlen(s) == strlen(r) );

  string::size_type pos;
  while( (pos = str.find_first_of(s)) != string::npos )
    str.replace( pos, strlen(r), r );
}


namespace gw6cconfig
{
// --------------------------------------------------------------------------
// Function : GW6CConfig constructor
//
// Description:
//   Will initialize a new GW6CConfig object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
GW6CConfig::GW6CConfig( void ) :
  m_pConfig(NULL),
  m_sDfltCfgFile(""),
  m_bValid(false)
{
  m_lsValidationErrors.clear();
}


// --------------------------------------------------------------------------
// Function : GW6CConfig destructor
//
// Description:
//   Will destroy GW6CConfig object.
//
// Arguments: (N/A)
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
GW6CConfig::~GW6CConfig( void )
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
//   aConfigFile: string [IN], The gateway6 client configuration file name.
//   aEAccessMode: enum [IN], The desired access mode (READ, CREATE, RW)
//   aDfltCfgFile: string [IN], A configuration file containing defaults.
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
void GW6CConfig::Initialize( const string& aConfigFile, const t_accessMode aEAccessMode, const string& aDfltCfgFile )
{
  // Keep the default configuration file, for later use.
  m_sDfltCfgFile = aDfltCfgFile;

  // Instantiate the name value configuration parser.
  m_pConfig = new NameValueConfig( aConfigFile, aEAccessMode );
}


// --------------------------------------------------------------------------
// Function : Load
//
// Description:
//   Will load the configuration data.
//
// Arguments: (none)
//
// Return values:
//   true if configuration data was successfully loaded from the file, and
//   the contents are valid.
//
// Exceptions:
//   - See Config::LoadConfiguration()
//
// --------------------------------------------------------------------------
bool GW6CConfig::Load( void )
{
  assert( m_pConfig != NULL );

  return m_pConfig->LoadConfiguration() && ValidateConfigData();
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
bool GW6CConfig::Save( void )
{
  assert( m_pConfig != NULL );

  bool bRetCode = ValidateConfigData() && m_pConfig->ApplyConfiguration();

  m_bValid = true;

  return bRetCode;
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
bool GW6CConfig::CancelChanges( void )
{
  assert( m_pConfig != NULL );

  return m_pConfig->CancelConfiguration();
}


// --------------------------------------------------------------------------
// Function : LoadDefaults
//
// Description:
//   Will replace the current configuration data with defaults. This default
//   configuration data is found in a file. The contents of the original
//   configuration is lost and overwritten.
//
// Arguments: (none)
//
// Return values:
//   true if configuration was overriden with the data from the default file.
//
// Exceptions:
//   - If no default configuration file was provided on initialization.
//   - See Config::OverrideConfiguration()
//
// --------------------------------------------------------------------------
bool GW6CConfig::LoadDefaults( void )
{
  assert( m_pConfig != NULL );

  if( m_sDfltCfgFile.size() == 0 )
  {
    throw GW6C_UIS__G6C_FAILLOADDFLTCONF;
  }

  return m_pConfig->OverrideConfiguration( m_sDfltCfgFile ) && ValidateConfigData();
}


// --------------------------------------------------------------------------
// Function : ValidateConfigData [PRIVATE]
//
// Description:
//   Will run all the validation routines on the configuration data, to
//   ensure validity. If some data is invalid, the error message is appended
//   to the class variable `m_lsValidationErrors'.
//
// Arguments: (none)
//
// Return values:
//   true if validation was completely successful, false otherwise.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
bool GW6CConfig::ValidateConfigData( void )
{
  string sValue;

  // Initialization
  assert( m_pConfig != NULL );
  m_bValid = true;
  m_lsValidationErrors.clear();


  // ------------------------------------------
  // Individual configuration data validation.
  // ------------------------------------------
  VALIDATE_LOGERRMSG( UserID, CFG_STR_USERID );
  VALIDATE_LOGERRMSG( Passwd, CFG_STR_PASSWD );
  VALIDATE_LOGERRMSG( Server, CFG_STR_SERVER );
  VALIDATE_LOGERRMSG( HostType, CFG_STR_HOSTTYPE );
  VALIDATE_LOGERRMSG( PrefixLen, CFG_STR_PREFIXLEN );
  VALIDATE_LOGERRMSG( IfPrefix, CFG_STR_IFPREFIX );
  VALIDATE_LOGERRMSG( DnsServer, CFG_STR_DNSSERVER );
  VALIDATE_LOGERRMSG( Gw6cDir, CFG_STR_GW6CDIR );
  VALIDATE_LOGERRMSG( AuthMethod, CFG_STR_AUTHMETHOD );
  VALIDATE_LOGERRMSG( RetryDelay, CFG_STR_RETRYDELAY );
  VALIDATE_LOGERRMSG( KeepAlive, CFG_STR_KEEPALIVE );
  VALIDATE_LOGERRMSG( KeepAliveInterval, CFG_STR_KEEPALIVEINTERVAL );
  VALIDATE_LOGERRMSG( TunnelMode, CFG_STR_TUNNELMODE );
  VALIDATE_LOGERRMSG( IfTunV6V4, CFG_STR_IFTUNV6V4 );
  VALIDATE_LOGERRMSG( IfTunV6UDPV4, CFG_STR_IFTUNV6UDPV4 );
  VALIDATE_LOGERRMSG( IfTunV4V6, CFG_STR_IFTUNV4V6 );
  VALIDATE_LOGERRMSG( ClientV4, CFG_STR_CLIENTV4 );
  VALIDATE_LOGERRMSG( ClientV6, CFG_STR_CLIENTV6 );
  VALIDATE_LOGERRMSG( Template, CFG_STR_TEMPLATE );
  VALIDATE_LOGERRMSG( ProxyClient, CFG_STR_PROXYCLIENT );
  VALIDATE_LOGERRMSG( BrokerLstFile, CFG_STR_BROKERLIST );
  VALIDATE_LOGERRMSG( LastServFile, CFG_STR_LASTSERVER );
  VALIDATE_LOGERRMSG( AlwaysUseLastSrv, CFG_STR_ALWAYSUSELASTSVR );
  VALIDATE_LOGERRMSG( LogLevel, (string)"log_" + STR_LOGDEV_CONSOLE );
  VALIDATE_LOGERRMSG( LogLevel, (string)"log_" + STR_LOGDEV_STDERR );
  VALIDATE_LOGERRMSG( LogLevel, (string)"log_" + STR_LOGDEV_SYSLOG );
  VALIDATE_LOGERRMSG( LogLevel, (string)"log_" + STR_LOGDEV_FILE );
  VALIDATE_LOGERRMSG( LogFileName, CFG_STR_LOGFILENAME );
  VALIDATE_LOGERRMSG( LogRotation, CFG_STR_LOGROTATION );
  VALIDATE_LOGERRMSG( LogRotationSz, CFG_STR_LOGROTATIONSZ );
  VALIDATE_LOGERRMSG( LogRotationDel, CFG_STR_LOGROTATIONDEL );
  VALIDATE_LOGERRMSG( SysLogFacility, CFG_STR_SYSLOGFACILITY );
  VALIDATE_LOGERRMSG( Hap6ProxyEnabled, CFG_STR_HAP6PROXYENABLED );
  VALIDATE_LOGERRMSG( Hap6WebEnabled, CFG_STR_HAP6WEBENABLED  );
  VALIDATE_LOGERRMSG( Hap6DocumentRoot, CFG_STR_HAP6DOCUMENTROOT );

  // Do not go on if configuration data is invalid.
  if( !m_bValid )
    return m_bValid;


  // ------------------------------------------------------------------------
  // --                 Value-dependant cross validation.                  --
  // ------------------------------------------------------------------------
  // NOTE: These GETs need to be put in try...catch blocks because they may
  //       raise an 'Invalid configuration' exception.
  // ------------------------------------------------------------------------
  string sValue2;

  // ------------------------------------------------------
  // 1. If authentication mode is NOT anonymous, password
  //    must be supplied.
  try
  {
    Get_AuthMethod( sValue );
    Get_Passwd( sValue2 );
    if( (sValue != STR_ANONYMOUS)  &&  sValue2.empty() )
    {
      m_lsValidationErrors.push_back( GW6C_UIS__G6C_SUPPLYPASSWDWHENNOTANON );
      m_bValid = false;
    }
  }
  catch(...)
  { // Catched an invalid configuration exception.
    m_bValid = false;
    return m_bValid;
  }

  // ---------------------------------------------------------------
  // 2. If proxy_client is true, tunnel mode must NOT be using UDP.
  try
  {
    Get_ProxyClient( sValue );
    Get_TunnelMode( sValue2 );
    if( sValue == STR_YES  &&  sValue2 == STR_V6UDPV4 )
    {
      m_lsValidationErrors.push_back( GW6C_UIS__G6C_PROXYCINVALIDMODE );
      m_bValid = false;
    }
  }
  catch(...)
  { // Catched an invalid configuration exception.
    m_bValid = false;
    return m_bValid;
  }

  // ---------------------------------------------------------
  // 3. If keep-alive is enabled, the interval must not be 0.
  try
  {
    Get_KeepAlive( sValue );
    Get_KeepAliveInterval( sValue2 );
    if( sValue == STR_YES  &&  strtol(sValue2.c_str(), (char**)NULL, 10) == 0 )
    {
      m_lsValidationErrors.push_back( GW6C_UIS__G6C_KAINTERVALINVALID );
      m_bValid = false;
    }
  }
  catch(...)
  { // Catched an invalid configuration exception.
    m_bValid = false;
    return m_bValid;
  }

  // ------------------------------------------------------------------------
  // 4. Verify that tunnel interface is provided, depending on tunnel mode.
  try
  {
    // Tunnel mode requested.
    Get_TunnelMode( sValue );

    if( sValue == STR_V6ANYV4 )
    {
      string sValue3;

      // Must specify the V6V4 and V4UDPV6 interface
      Get_IfTunV6V4( sValue2 );
      Get_IfTunV6UDPV4( sValue3 );
      if( sValue2.empty()  ||  sValue3.empty() )
      {
        m_lsValidationErrors.push_back( GW6C_UIS__G6C_IFTUNV6V4ANDV6UDPV4REQUIRED );
        m_bValid = false;
      }
    }
    else if( sValue == STR_V6V4 )
    {
      // Must specify the V6V4 interface
      Get_IfTunV6V4( sValue2 );
      if( sValue2.empty() )
      {
        m_lsValidationErrors.push_back( GW6C_UIS__G6C_IFTUNV6V4REQUIRED );
        m_bValid = false;
      }
    }
    else if( sValue == STR_V6UDPV4 )
    {
      // Must specify the V6UDPV4 interface
      Get_IfTunV6UDPV4( sValue2 );
      if( sValue2.empty() )
      {
        m_lsValidationErrors.push_back( GW6C_UIS__G6C_IFTUNV6UDPV4REQUIRED );
        m_bValid = false;
      }
    }
    else if( sValue == STR_V4V6 )
    {
      // Must specify the V4V6 interface
      Get_IfTunV4V6( sValue2 );
      if( sValue2.empty() )
      {
        m_lsValidationErrors.push_back( GW6C_UIS__G6C_IFTUNV4V6REQUIRED );
        m_bValid = false;
      }
    }
    else
      assert( false );    // Should never reach here.
  }
  catch(...)
  { // Catched an invalid configuration exception.
    m_bValid = false;
    return m_bValid;
  }

  // -------------------------------------------------------------
  // 5. If host_type is 'router', the if_prefix must be supplied.
  try
  {
    Get_HostType( sValue );
    Get_IfPrefix( sValue2 );

    if( sValue == STR_HOSTTYPE_ROUTER  &&  sValue2.empty() )
    {
      m_lsValidationErrors.push_back( GW6C_UIS__G6V_IFPREFIXMUSTBESPEC );
      m_bValid = false;
    }
  }
  catch(...)
  { // Catched an invalid configuration exception.
    m_bValid = false;
    return m_bValid;
  }

#ifdef HAP6
  // --------------------------------------------------------------
  // 5. If HAP6 Web is enabled, the document root cannot be empty.
  try
  {
    Get_Hap6WebEnabled( sValue );
    Get_Hap6DocumentRoot( sValue2 );
    if( sValue == STR_YES  &&  sValue2.empty() )
    {
      m_lsValidationErrors.push_back( GW6C_UIS__G6V_HAP6DOCROOTNOTSPEC );
      m_bValid = false;
    }
  }
  catch(...)
  { // Catched an invalid configuration exception.
    m_bValid = false;
    return m_bValid;
  }

  // --------------------------------------------------------------
  // 6. If HAP6 is built, the tunnel mode cannot be V4V6.
  try
  {
    Get_TunnelMode( sValue2 );
    if( (sValue == STR_YES  ||  sValue2 == STR_YES)  &&  sValue2 == STR_V4V6 )
    {
      m_lsValidationErrors.push_back( GW6C_UIS__G6V_HAP6INCOMPV4V6 );
      m_bValid = false;
    }
  }
  catch(...)
  { // Catched an invalid configuration exception.
    m_bValid = false;
    return m_bValid;
  }
#endif

  return m_bValid;
}


// --------------------------------------------------------------------------
/* *************************************************************************/

/*            GATEWAY6 CLIENT CONFIGURATION DATA ACCESSORS                 */

/* *************************************************************************/
// --------------------------------------------------------------------------

void GW6CConfig::Get_UserID( string& sUserID ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_USERID, sUserID );
}


void GW6CConfig::Set_UserID( const string& sUserID )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( UserID, CFG_STR_USERID );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_Passwd( string& sPasswd ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_PASSWD, sPasswd );
}

void GW6CConfig::Set_Passwd( const string& sPasswd )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( Passwd, CFG_STR_PASSWD );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_Server( string& sServer ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_SERVER, sServer );
}

void GW6CConfig::Set_Server( const string& sServer )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( Server, CFG_STR_SERVER );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_HostType( string& sHostType ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_HOSTTYPE, sHostType );

  // Push default value, if not present.
  if( sHostType.size() == 0 )
    sHostType = CFG_DFLT_HOSTTYPE;
}

void GW6CConfig::Set_HostType( const string& sHostType )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( HostType, CFG_STR_HOSTTYPE );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_PrefixLen( string& sPrefixLen ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_PREFIXLEN, sPrefixLen );

  // Push default value, if not present.
  if( sPrefixLen.size() == 0 )
    sPrefixLen = CFG_DFLT_PREFIXLEN;
}

void GW6CConfig::Set_PrefixLen( const string& sPrefixLen )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( PrefixLen, CFG_STR_PREFIXLEN );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_IfPrefix( string& sIfPrefix ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_IFPREFIX, sIfPrefix );

  // Push default value, if not present.
  if( sIfPrefix.size() == 0 )
    sIfPrefix = CFG_DFLT_IFPREFIX;
}

void GW6CConfig::Set_IfPrefix( const string& sIfPrefix )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( IfPrefix, CFG_STR_IFPREFIX );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_DnsServer( string& sDnsServer ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_DNSSERVER, sDnsServer );
}

void GW6CConfig::Set_DnsServer( const string& sDnsServer )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( DnsServer, CFG_STR_DNSSERVER );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_Gw6cDir( string& sGw6cDir ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_GW6CDIR, sGw6cDir );

  // Push default value, if not present.
  if( sGw6cDir.size() == 0 )
    sGw6cDir = CFG_DFLT_GW6CDIR;
}

void GW6CConfig::Set_Gw6cDir( const string& sGw6cDir )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( Gw6cDir, CFG_STR_GW6CDIR );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_AuthMethod( string& sAuthMethod ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_AUTHMETHOD, sAuthMethod );

  // Push default value, if not present.
  if( sAuthMethod.size() == 0 )
    sAuthMethod = CFG_DFLT_AUTHMETHOD;
}

void GW6CConfig::Set_AuthMethod( const string& sAuthMethod )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( AuthMethod, CFG_STR_AUTHMETHOD );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_AutoRetryConnect( string& sAutoRetryConnect ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_AUTORETRYCONNECT, sAutoRetryConnect );

  // Push default value, if not present.
  if( sAutoRetryConnect.size() == 0 )
    sAutoRetryConnect = CFG_DFLT_AUTORETRYCONNECT;
}

void GW6CConfig::Set_AutoRetryConnect( const string& sAutoRetryConnect )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( AutoRetryConnect, CFG_STR_AUTORETRYCONNECT );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_RetryDelay( string& sRetryDelay ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_RETRYDELAY, sRetryDelay );

  // Push default value, if not present.
  if( sRetryDelay.size() == 0 )
    sRetryDelay = CFG_DFLT_RETRYDELAY;
}

void GW6CConfig::Set_RetryDelay( const string& sRetryDelay )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( RetryDelay, CFG_STR_RETRYDELAY );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_KeepAlive( string& sKeepAlive ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_KEEPALIVE, sKeepAlive );

  // Push default value, if not present.
  if( sKeepAlive.size() == 0 )
    sKeepAlive = CFG_DFLT_KEEPALIVE;
}

void GW6CConfig::Set_KeepAlive( const string& sKeepAlive )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( KeepAlive, CFG_STR_KEEPALIVE );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_KeepAliveInterval( string& sKeepAliveInterval ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_KEEPALIVEINTERVAL, sKeepAliveInterval );

  // Push default value, if not present.
  if( sKeepAliveInterval.size() == 0 )
    sKeepAliveInterval = CFG_DFLT_KEEPALIVEINTERVAL;
}

void GW6CConfig::Set_KeepAliveInterval( const string& sKeepAliveInterval )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( KeepAliveInterval, CFG_STR_KEEPALIVEINTERVAL );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_TunnelMode( string& sTunnelMode ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_TUNNELMODE, sTunnelMode );

  // Push default value, if not present.
  if( sTunnelMode.size() == 0 )
    sTunnelMode = CFG_DFLT_TUNNELMODE;
}

void GW6CConfig::Set_TunnelMode( const string& sTunnelMode )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( TunnelMode, CFG_STR_TUNNELMODE );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_IfTunV6V4( string& sIfTunV6V4 ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_IFTUNV6V4, sIfTunV6V4 );
}

void GW6CConfig::Set_IfTunV6V4( const string& sIfTunV6V4 )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( IfTunV6V4, CFG_STR_IFTUNV6V4 );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_IfTunV6UDPV4( string& sIfTunV6UDPV4 ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_IFTUNV6UDPV4, sIfTunV6UDPV4 );
}

void GW6CConfig::Set_IfTunV6UDPV4( const string& sIfTunV6UDPV4 )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( IfTunV6UDPV4, CFG_STR_IFTUNV6UDPV4 );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_IfTunV4V6( string& sIfTunV4V6 ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_IFTUNV4V6, sIfTunV4V6 );
}

void GW6CConfig::Set_IfTunV4V6( const string& sIfTunV4V6 )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( IfTunV4V6, CFG_STR_IFTUNV4V6 );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_ClientV4( string& sClientV4 ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_CLIENTV4, sClientV4 );

  // Push default value, if not present.
  if( sClientV4.size() == 0 )
    sClientV4 = CFG_DFLT_CLIENTV4;
}

void GW6CConfig::Set_ClientV4( const string& sClientV4 )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( ClientV4, CFG_STR_CLIENTV4 );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_ClientV6( string& sClientV6 ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_CLIENTV6, sClientV6 );

  // Push default value, if not present.
  if( sClientV6.size() == 0 )
    sClientV6 = CFG_DFLT_CLIENTV6;
}

void GW6CConfig::Set_ClientV6( const string& sClientV6 )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( ClientV6, CFG_STR_CLIENTV6 );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_Template( string& sTemplate ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_TEMPLATE, sTemplate );
}

void GW6CConfig::Set_Template( const string& sTemplate )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( Template, CFG_STR_TEMPLATE );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_ProxyClient( string& sProxyClient ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_PROXYCLIENT, sProxyClient );

  // Push default value, if not present.
  if( sProxyClient.size() == 0 )
    sProxyClient = CFG_DFLT_PROXYCLIENT;
}

void GW6CConfig::Set_ProxyClient( const string& sProxyClient )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( ProxyClient, CFG_STR_PROXYCLIENT );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_BrokerLstFile( string& sBrokerLstFile ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_BROKERLIST, sBrokerLstFile );

  // Push default value, if not present.
  if( sBrokerLstFile.size() == 0 )
    sBrokerLstFile = CFG_DFLT_BROKERLIST;
}

void GW6CConfig::Set_BrokerLstFile( const string& sBrokerLstFile )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( BrokerLstFile, CFG_STR_BROKERLIST );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_LastServFile( string& sLastServFile ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_LASTSERVER, sLastServFile );

  // Push default value, if not present.
  if( sLastServFile.size() == 0 )
    sLastServFile = CFG_DFLT_LASTSERVER;
}

void GW6CConfig::Set_LastServFile( const string& sLastServFile )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( LastServFile, CFG_STR_LASTSERVER );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_AlwaysUseLastSrv( string& sAlwaysUseLastSrv ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_ALWAYSUSELASTSVR, sAlwaysUseLastSrv );

  // Push default value, if not present.
  if( sAlwaysUseLastSrv.size() == 0 )
    sAlwaysUseLastSrv = CFG_DFLT_ALWAYSUSELASTSVR;
}

void GW6CConfig::Set_AlwaysUseLastSrv( const string& sAlwaysUseLastSrv )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( AlwaysUseLastSrv, CFG_STR_ALWAYSUSELASTSVR );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_Log( const string& sLogDevice, string& sLogLevel ) const
{
  string sLogRequest;
  ASSERT_VALID_CONFIG;

  if( !Validate_LogDevice( sLogDevice ) )
  {
    throw GetLastError();
  }

  sLogRequest = "log_" + sLogDevice;

  m_pConfig->GetVariableValue( sLogRequest, sLogLevel );

  // Push default value, if not present.
  if( sLogLevel.size() == 0 )
  {
    // Retrieve the default log value for the specified device.
    sLogLevel = GetDfltLogLevelForDevice( sLogDevice );
  }
}

void GW6CConfig::Set_Log( const string& sLogDevice, const string& sLogLevel )
{
  string sLogRequest;
  ASSERT_VALID_CONFIG;

  // Check if specified device is within domain values.
  if( !Validate_LogDevice( sLogDevice ) )
  {
    throw GetLastError();
  }

  // Check if log level is within domain values.
  if( !Validate_LogLevel( sLogLevel ) )
  {
    throw GetLastError();
  }

  // Set the log level for the device.
  sLogRequest = "log_" + sLogDevice;
  m_pConfig->SetVariableValue( sLogRequest, sLogLevel );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_LogFileName( string& sLogFileName ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_LOGFILENAME, sLogFileName );

  // Push default value, if not present.
  if( sLogFileName.size() == 0 )
    sLogFileName = CFG_DFLT_LOGFILENAME;
}

void GW6CConfig::Set_LogFileName( const string& sLogFileName )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( LogFileName, CFG_STR_LOGFILENAME );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_LogRotation( string& sLogRotation ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_LOGROTATION, sLogRotation );

  // Push default value, if not present.
  if( sLogRotation.size() == 0 )
    sLogRotation = CFG_DFLT_LOGROTATION;
}

void GW6CConfig::Set_LogRotation( const string& sLogRotation )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( LogRotation, CFG_STR_LOGROTATION );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_LogRotationSz( string& sLogRotationSz ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_LOGROTATIONSZ, sLogRotationSz );

  // Push default value, if not present.
  if( sLogRotationSz.size() == 0 )
    sLogRotationSz = CFG_DFLT_LOGROTATIONSZ;
}

void GW6CConfig::Set_LogRotationSz( const string& sLogRotationSz )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( LogRotationSz, CFG_STR_LOGROTATIONSZ );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_LogRotationDel( string& sLogRotationDel ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_LOGROTATIONDEL, sLogRotationDel );

  // Push default value, if not present.
  if( sLogRotationDel.size() == 0 )
    sLogRotationDel = CFG_DFLT_LOGROTATIONDEL;
}

void GW6CConfig::Set_LogRotationDel( const string& sLogRotationDel )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( LogRotationDel, CFG_STR_LOGROTATIONDEL );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_SysLogFacility( string& sSysLogFacility ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_SYSLOGFACILITY, sSysLogFacility );

  // Push default value, if not present.
  if( sSysLogFacility.size() == 0 )
    sSysLogFacility = CFG_DFLT_SYSLOGFACILITY;
}

void GW6CConfig::Set_SysLogFacility( const string& sSysLogFacility )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( SysLogFacility, CFG_STR_SYSLOGFACILITY );
}



// --------------------------------------------------------------------------
void GW6CConfig::Get_Hap6ProxyEnabled( string& sHap6ProxyEnabled ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_HAP6PROXYENABLED, sHap6ProxyEnabled );

  // Push default value, if not present.
  if( sHap6ProxyEnabled.size() == 0 )
    sHap6ProxyEnabled = CFG_DFLT_HAP6PROXYENABLED;
}

void GW6CConfig::Set_Hap6ProxyEnabled( const string& sHap6ProxyEnabled )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( Hap6ProxyEnabled, CFG_STR_HAP6PROXYENABLED );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_Hap6WebEnabled( string& sHap6WebEnabled ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_HAP6WEBENABLED, sHap6WebEnabled );

  // Push default value, if not present.
  if( sHap6WebEnabled.size() == 0 )
    sHap6WebEnabled = CFG_DFLT_HAP6WEBENABLED;
}

void GW6CConfig::Set_Hap6WebEnabled( const string& sHap6WebEnabled )
{
  ASSERT_VALID_CONFIG;
  VERIFY_AND_SET( Hap6WebEnabled, CFG_STR_HAP6WEBENABLED );
}


// --------------------------------------------------------------------------
void GW6CConfig::Get_Hap6DocumentRoot( string& sHap6DocumentRoot ) const
{
  ASSERT_VALID_CONFIG;
  m_pConfig->GetVariableValue( CFG_STR_HAP6DOCUMENTROOT, sHap6DocumentRoot );

  // Push default value, if not present.
  if( sHap6DocumentRoot.size() == 0 )
    sHap6DocumentRoot = CFG_DFLT_HAP6DOCUMENTROOT;

  replace( sHap6DocumentRoot, "\\", "/" );
}

void GW6CConfig::Set_Hap6DocumentRoot( const string& sHap6DocumentRoot )
{
  ASSERT_VALID_CONFIG;

  if( Validate_Hap6DocumentRoot( sHap6DocumentRoot ) )
  {
    string formatted = sHap6DocumentRoot;
    replace( formatted, "\\", "/" );
    m_pConfig->SetVariableValue( CFG_STR_HAP6DOCUMENTROOT, formatted );
  }
  else
    throw GetLastError();
}

} // Namespace
