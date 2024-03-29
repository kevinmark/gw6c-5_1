// **************************************************************************
// $Id: gw6cvalidation.cc,v 1.27 2007/11/12 20:50:25 krause Exp $
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
//   Implementation of the Gateway6 Configuration data validation routines.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cconfig/gw6cvalidation.h>


#ifdef WIN32
#include <direct.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define getcwd                            _getcwd
#define chdir                             _chdir
#define strdup                            _strdup
#define INET_PTON(A, B, C)                tsp_windows_inet_pton(A, B, C)
#else
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define INET_PTON(A, B, C)                inet_pton(A,B,C)
#endif


// Definition of valid characters for different strings.
#define CFG_SERVER_CHRS                   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.:[]"
#define CFG_DNS_CHRS                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-."
#define CFG_FILENAME_CHRS                 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_./\\"
#define CFG_NUMERIC_CHRS                  "1234567890"

// Value limits.
#define CFG_MAX_USERID                    253
#define CFG_MAX_PASSWD                    128
#define CFG_MAX_SERVER                    1025
#define CFG_MIN_PREFIXLEN                 0
#define CFG_MAX_PREFIXLEN                 128
#define CFG_MAX_DNSSERVER                 1025
#define CFG_MIN_RETRYDELAY                0
#define CFG_MAX_RETRYDELAY                3600
#define CFG_MAX_FILENAME_LEN              256
#define CFG_MIN_LOG_LEVEL                 0
#define CFG_MAX_LOG_LEVEL                 3

// Domain values.
static const char* cfgHOSTTYPE_values[]         = { STR_HOSTTYPE_HOST, STR_HOSTTYPE_ROUTER };
static const char* cfgAUTHMETHOD_values[]       = { STR_ANONYMOUS, STR_ANY, STR_DIGESTMD5, STR_PLAIN, STR_PASSDSS3DES1 };
static const char* cfgAUTORETRYCONNECT_values[] = { STR_YES, STR_NO };
static const char* cfgKEEPALIVE_values[]        = { STR_YES, STR_NO };
static const char* cfgTUNNELMODE_values[]       = { STR_V6ANYV4, STR_V6V4, STR_V6UDPV4, STR_V4V6 };
static const char* cfgTEMPLATE_values[]         = { "checktunnel","freebsd","netbsd","linux",STR_TEMPL_WINDOWS,"darwin","cisco","solaris","openbsd","openwrt", "dongle6" };
static const char* cfgPROXYCLIENT_values[]      = { STR_YES, STR_NO };
static const char* cfgALWAYSUSELASTSVR_values[] = { STR_YES, STR_NO };
static const char* cfgLOGDEVICE_values[]        = { STR_LOGDEV_CONSOLE,STR_LOGDEV_STDERR,STR_LOGDEV_FILE,STR_LOGDEV_SYSLOG };
static const char* cfgLOGROTATION_values[]      = { STR_YES, STR_NO };
static const char* cfgLOGROTATIONSZ_values[]    = { STR_LOGROTSZ_16K, STR_LOGROTSZ_32K, STR_LOGROTSZ_128K, STR_LOGROTSZ_1024K };
static const char* cfgLOGROTATIONDEL_values[]   = { STR_YES, STR_NO };
static const char* cfgSYSLOGFACILITY_values[]   = { "USER","LOCAL0","LOCAL1","LOCAL2","LOCAL3","LOCAL4","LOCAL5","LOCAL6","LOCAL7" };
static const char* cfgHAP6PROXYENABLED_values[] = { STR_YES, STR_NO };
static const char* cfgHAP6WEBENABLED_values[]   = { STR_YES, STR_NO };

namespace gw6cconfig
{
// global static integer containing the last error.
static error_t gssLastError = GW6C_UIS__NOERROR;

// --------------------------------------------------------------------------
// Function : GetLastError
//
// Description:
//   Returns the last error set.
//
// Arguments: (none)
//
// Return values:
//   The error number of the last error. See gw6cUIstrings.h
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
const error_t GetLastError( void )
{
  return gssLastError;
}


// --------------------------------------------------------------------------
// Function : DirExists
//
// Description:
//   Helper function to verify if a directory exists.
//
// Arguments:
//   aDir: string [IN], The directory name as a string.
//
// Return values:
//   true if directory exists, false otherwise.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
bool DirExists( const string& aDir )
{
  char buf[2048];
  bool bValid = true;


  // Get and save actual working directory.
  if( getcwd(buf,2048) )
  {
    // Attempt changing directory.
    bValid = chdir(aDir.c_str()) == 0;

    // restore previous working directory.
    chdir(buf);
  }

  return bValid;
}


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


// --------------------------------------------------------------------------
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
//                    G A T E W A Y 6   C L I E N T
//                V A L I D A T I O N   R O U T I N E S
// __________________________________________________________________________
// --------------------------------------------------------------------------
bool Validate_UserID( const string& sUserID )
{
  // Facultative
  if( sUserID.size() == 0 ) return true;

  // Check string length.
  if( sUserID.size() > CFG_MAX_USERID )
  {
    gssLastError = GW6C_UIS__G6V_USERIDTOOLONG;
    return false;
  }

  // Check for invalid characters
  const char* c;
  for( c = sUserID.c_str(); *c; c++ )
    if (*c < ' ' || *c > '~')
    {
      gssLastError = GW6C_UIS__G6V_USERIDINVALIDCHRS;
      return false;
    }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_Passwd( const string& sPasswd )
{
  // Facultative
  if( sPasswd.size() == 0 ) return true;

  // Check string length.
  if( sPasswd.size() > CFG_MAX_PASSWD )
  {
    gssLastError = GW6C_UIS__G6V_PASSWDTOOLONG;
    return false;
  }

  // Check for invalid characters
  const char* c;
  for( c = sPasswd.c_str(); *c; c++ )
    if (*c < ' ' || *c > '~')
    {
      gssLastError = GW6C_UIS__G6V_PASSWDINVALIDCHRS;
      return false;
    }


  return true;
}

// --------------------------------------------------------------------------
bool Validate_Server( const string& sServer )
{
  /* Bug 3882: removed server from config file -> Is now facultative.
  // NOT facultative
  if( sServer.size() == 0 )
  {
    gssLastError = GW6C_UIS__G6V_SERVERMUSTBESPEC;
    return false;
  }*/

  // Facultative
  if( sServer.size() == 0 ) return true;


  // Check string length.
  if( sServer.size() > CFG_MAX_SERVER )
  {
    gssLastError = GW6C_UIS__G6V_SERVERTOOLONG;
    return false;
  }

  // check for invalid characters
  if( sServer.find_first_not_of( CFG_SERVER_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_SERVERINVALIDCHRS;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_HostType( const string& sHostType )
{
  // Facultative
  if( sHostType.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgHOSTTYPE_values)/sizeof(cfgHOSTTYPE_values[0])); i++)
  {
    if( sHostType == cfgHOSTTYPE_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_HOSTTYPEINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_PrefixLen( const string& sPrefixLen )
{
  // Facultative
  if( sPrefixLen.size() == 0 ) return true;

  // Check characters are all numeric.
  if( sPrefixLen.find_first_not_of( CFG_NUMERIC_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_PREFIXLENINVALIDVALUE;
    return false;
  }

  short _PrefixLen = (short)strtol(sPrefixLen.c_str(), (char**)NULL, 10);
  if( _PrefixLen <= CFG_MIN_PREFIXLEN || _PrefixLen > CFG_MAX_PREFIXLEN )
  {
    gssLastError = GW6C_UIS__G6V_PREFIXLENINVALIDVALUE;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_IfPrefix( const string& sIfPrefix )
{
  // Check for invalid characters
  if( sIfPrefix.find_first_not_of( CFG_FILENAME_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_IFPREFIXINVALIDCHRS;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_DnsServer( const string& sDnsServer )
{
  // Facultative
  if( sDnsServer.size() == 0 ) return true;

  // Check string length
  if( sDnsServer.size() > CFG_MAX_DNSSERVER )
  {
    gssLastError = GW6C_UIS__G6V_DNSSERVERSTOOLONG;
    return false;
  }

  // Check every server to see if they're all valid.
  char* szDNSServers;
  char* szServer;
  string sServer;

  szDNSServers = strdup( sDnsServer.c_str() );
  for(szServer = strtok(szDNSServers, ":");szServer; szServer=strtok(NULL, ":") )
  {
    sServer = szServer;

    // check for invalid characters
    if( sServer.find_first_not_of( CFG_DNS_CHRS ) != string::npos )
    {
      gssLastError = GW6C_UIS__G6V_DNSSERVERSINVALIDCHRS;
      free( szDNSServers );
      return false;
    }

    /*
    // Attempt to resolve hostname
    if( gethostbyname(szServer) == NULL )
    {
      gssLastError = GW6C_UIS__G6V_DNSSERVERSUNRESOLVABLE;
      free( szDNSServers );
      return false;
    }
    */
  }
  free( szDNSServers );

  return true;
}

// --------------------------------------------------------------------------
bool Validate_Gw6cDir( const string& sGw6cDir )
{
  // Facultative
  if( sGw6cDir.size() == 0 ) return true;

  if( !DirExists(sGw6cDir) )
  {
    gssLastError = GW6C_UIS__G6V_GW6CDIRDOESNTEXIST;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_AuthMethod( const string& sAuthMethod )
{
  // Facultative
  if( sAuthMethod.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgAUTHMETHOD_values)/sizeof(cfgAUTHMETHOD_values[0])); i++)
  {
    if( sAuthMethod == cfgAUTHMETHOD_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_AUTHMETHODINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_AutoRetryConnect( const string& sAutoRetryConnect )
{
  // Facultative
  if( sAutoRetryConnect.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgAUTORETRYCONNECT_values)/sizeof(cfgAUTORETRYCONNECT_values[0])); i++)
  {
    if( sAutoRetryConnect == cfgAUTORETRYCONNECT_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_AUTORETRYCONNECTINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_RetryDelay( const string& sRetryDelay )
{
  // Facultative
  if( sRetryDelay.size() == 0 ) return true;

  // Check characters are all numeric.
  if( sRetryDelay.find_first_not_of( CFG_NUMERIC_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_RETRYDELAYINVALIDVALUE;
    return false;
  }

  long _RetryDelay = strtol(sRetryDelay.c_str(), (char**)NULL, 10);
  if( _RetryDelay < CFG_MIN_RETRYDELAY || _RetryDelay > CFG_MAX_RETRYDELAY)
  {
    gssLastError = GW6C_UIS__G6V_RETRYDELAYINVALIDVALUE;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_KeepAlive( const string& sKeepAlive )
{
  // Facultative
  if( sKeepAlive.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgKEEPALIVE_values)/sizeof(cfgKEEPALIVE_values[0])); i++)
  {
    if( sKeepAlive == cfgKEEPALIVE_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_KEEPALIVEINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_KeepAliveInterval( const string& sKeepAliveInterval )
{
  // Facultative
  if( sKeepAliveInterval.size() == 0 ) return true;

  // Check characters are all numeric.
  if( sKeepAliveInterval.find_first_not_of( CFG_NUMERIC_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_KEEPALIVEINTERVINVALID;
    return false;
  }

  long _KAInterval = strtol(sKeepAliveInterval.c_str(), (char**)NULL, 10);
  if( _KAInterval < 0 )
  {
    gssLastError = GW6C_UIS__G6V_KEEPALIVEINTERVINVALID;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_TunnelMode( const string& sTunnelMode )
{
  // Facultative
  if( sTunnelMode.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgTUNNELMODE_values)/sizeof(cfgTUNNELMODE_values[0])); i++)
  {
    if( sTunnelMode == cfgTUNNELMODE_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_TUNNELMODEINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_IfTunV6V4( const string& sIfTunV6V4 )
{
  // Facultative (checked later with tunnel mode)
  if( sIfTunV6V4.size() == 0 ) return true;

  // Check for invalid characters
  if( sIfTunV6V4.find_first_not_of( CFG_DNS_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_IFTUNV6V4INVALIDCHRS;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_IfTunV6UDPV4( const string& sIfTunV6UDPV4 )
{
  // Facultative (checked later with tunnel mode)
  if( sIfTunV6UDPV4.size() == 0 ) return true;

  // Check for invalid characters
  if( sIfTunV6UDPV4.find_first_not_of( CFG_DNS_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_IFTUNV6UDPV4INVALIDCHRS;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_IfTunV4V6( const string& sIfTunV4V6 )
{
  // Facultative (checked later with tunnel mode)
  if( sIfTunV4V6.size() == 0 ) return true;

  // Check for invalid characters
  if( sIfTunV4V6.find_first_not_of( CFG_DNS_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_IFTUNV4V6INVALIDCHRS;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_ClientV4( const string& sClientV4 )
{
  // Facultative
  if( sClientV4.size() == 0 ) return true;

  // If not auto then check if ip address is ok.
  if( sClientV4 != STR_AUTO )
  {
    struct in_addr address;
    unsigned long net;

    net = inet_addr( sClientV4.c_str() );
    memcpy(&address, &net, sizeof(net));

    if( sClientV4 != inet_ntoa(address) )
    {
      gssLastError = GW6C_UIS__G6V_CLIENTV4INVALIDVALUE;
      return false;
    }
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_ClientV6( const string& sClientV6 )
{
  // Facultative
  if( sClientV6.size() == 0 ) return true;

  // If not auto then check if ip address is ok.
  if( sClientV6 != STR_AUTO )
  {
    struct in6_addr address;

    if( INET_PTON(AF_INET6, sClientV6.c_str(), &address) <= 0 )
    {
      gssLastError = GW6C_UIS__G6V_CLIENTV6INVALIDVALUE;
      return false;
    }
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_Template( const string& sTemplate )
{
  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgTEMPLATE_values)/sizeof(cfgTEMPLATE_values[0])); i++)
  {
    if( sTemplate == cfgTEMPLATE_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_TEMPLATEINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_ProxyClient( const string& sProxyClient )
{
  // Facultative
  if( sProxyClient.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgPROXYCLIENT_values)/sizeof(cfgPROXYCLIENT_values[0])); i++)
  {
    if( sProxyClient == cfgPROXYCLIENT_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_PROXYCLIENTINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_BrokerLstFile( const string& sBrokerLstFile )
{
  // Check string length
  if( sBrokerLstFile.size() > CFG_MAX_FILENAME_LEN )
  {
    gssLastError = GW6C_UIS__G6V_BROKERLISTTOOLONG;
    return false;
  }

  // Check for invalid characters
  if( sBrokerLstFile.find_first_not_of( CFG_FILENAME_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_BROKERLISTINVALIDCHRS;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_LastServFile( const string& sLastServFile )
{
  // Check string length
  if( sLastServFile.size() > CFG_MAX_FILENAME_LEN )
  {
    gssLastError = GW6C_UIS__G6V_LASTSERVTOOLONG;
    return false;
  }

  // Check for invalid characters
  if( sLastServFile.find_first_not_of( CFG_FILENAME_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_LASTSERVINVALIDCHRS;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_AlwaysUseLastSrv( const string& sAlwaysUseLastSrv )
{
  // Facultative
  if( sAlwaysUseLastSrv.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgALWAYSUSELASTSVR_values)/sizeof(cfgALWAYSUSELASTSVR_values[0])); i++)
  {
    if( sAlwaysUseLastSrv == cfgALWAYSUSELASTSVR_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_ALWAYSUSERLASTSERVINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_LogDevice( const string& sLogDevice )
{
  // Not facultative.
  // Check log device
  for(unsigned int i=0; i<(sizeof(cfgLOGDEVICE_values)/sizeof(cfgLOGDEVICE_values[0])); i++)
  {
    if( sLogDevice == cfgLOGDEVICE_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_LOGDEVICEINVALIDVALUE;

  return false;
}

bool Validate_LogLevel( const string& sLogLevel )
{
  // Facultative
  if( sLogLevel.size() == 0 ) return true;

  // Check log level, if any
  long _LogLevel = strtol(sLogLevel.c_str(), (char**)NULL, 10);
  if( _LogLevel < CFG_MIN_LOG_LEVEL  ||  _LogLevel > CFG_MAX_LOG_LEVEL )
  {
    gssLastError = GW6C_UIS__G6V_LOGLEVELINVALIDVALUE;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_LogFileName( const string& sLogFileName )
{
  // Check string length
  if( sLogFileName.size() > CFG_MAX_FILENAME_LEN )
  {
    gssLastError = GW6C_UIS__G6V_LOGFILENAMETOOLONG;
    return false;
  }

  // Check for invalid characters
  if( sLogFileName.find_first_not_of( CFG_FILENAME_CHRS ) != string::npos )
  {
    gssLastError = GW6C_UIS__G6V_LOGFILENAMEINVALIDCHRS;
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool Validate_LogRotation( const string& sLogRotation )
{
  // Facultative
  if( sLogRotation.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgLOGROTATION_values)/sizeof(cfgLOGROTATION_values[0])); i++)
  {
    if( sLogRotation == cfgLOGROTATION_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_LOGROTATIONINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_LogRotationSz( const string& sLogRotationSz )
{
  // Facultative
  if( sLogRotationSz.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgLOGROTATIONSZ_values)/sizeof(cfgLOGROTATIONSZ_values[0])); i++)
  {
    if( sLogRotationSz == cfgLOGROTATIONSZ_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_LOGROTSZINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_LogRotationDel( const string& sLogRotationDel )
{
  // Facultative
  if( sLogRotationDel.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgLOGROTATIONDEL_values)/sizeof(cfgLOGROTATIONDEL_values[0])); i++)
  {
    if( sLogRotationDel == cfgLOGROTATIONDEL_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_LOGROTDELINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_SysLogFacility( const string& sSysLogFacility )
{
  // Facultative
  if( sSysLogFacility.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgSYSLOGFACILITY_values)/sizeof(cfgSYSLOGFACILITY_values[0])); i++)
  {
    if( sSysLogFacility == cfgSYSLOGFACILITY_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_SYSLOGFACILITYINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_Hap6ProxyEnabled( const string& sHap6ProxyEnabled )
{
  // Facultative
  if( sHap6ProxyEnabled.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgHAP6PROXYENABLED_values)/sizeof(cfgHAP6PROXYENABLED_values[0])); i++)
  {
    if( sHap6ProxyEnabled == cfgHAP6PROXYENABLED_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_HAP6PROXYENABLEDINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_Hap6WebEnabled( const string& sHap6WebEnabled )
{
  // Facultative
  if( sHap6WebEnabled.size() == 0 ) return true;

  // Check against domain values.
  for(unsigned int i=0; i<(sizeof(cfgHAP6WEBENABLED_values)/sizeof(cfgHAP6WEBENABLED_values[0])); i++)
  {
    if( sHap6WebEnabled == cfgHAP6WEBENABLED_values[i] )
      return true;
  }
  gssLastError = GW6C_UIS__G6V_HAP6WEBENABLEDINVALIDVALUE;

  return false;
}

// --------------------------------------------------------------------------
bool Validate_Hap6DocumentRoot( const string& sHap6DocumentRoot )
{
  // Facultative
  if( sHap6DocumentRoot.size() == 0 ) return true;

  // Check if directory exists.
  if( !DirExists(sHap6DocumentRoot) )
  {
    gssLastError = GW6C_UIS__G6V_HAP6DOCROOTDOESNTEXIST;
    return false;
  }

  return true;
}


}
