/*
---------------------------------------------------------------------------
 $Id: errors.h,v 1.20 2007/05/02 13:32:21 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _ERRORS_H_
#define _ERRORS_H_

/* globals */

#ifdef NO_ERROR
#undef NO_ERROR
#endif

#ifdef TSP_ERROR
#undef TSP_ERROR
#endif

#ifdef SOCKET_ERROR
#undef SOCKET_ERROR
#endif

#ifdef INTERFACE_SETUP_FAILED
#undef INTERFACE_SETUP_FAILED
#endif

#ifdef KEEPALIVE_TIMEOUT
#undef KEEPALIVE_TIMEOUT
#endif

#ifdef TUNNEL_ERROR
#undef TUNNEL_ERROR
#endif



typedef enum {
  NO_ERROR = 0,
  NO_ERROR_SHOW_HELP,
  TSP_ERROR,
  SOCKET_ERROR,
  INTERFACE_SETUP_FAILED,
  KEEPALIVE_TIMEOUT,
  KEEPALIVE_ERROR,
  TUNNEL_ERROR,
  TSP_VERSION_ERROR,
  AUTHENTICATION_ERROR,
  LEASE_EXPIRED,
  SERVER_SIDE_ERROR,  /* added for at&t */
  /* added for bug 3373/3382 */
  INVALID_ARGUMENTS,
  MEMORY_ERROR,
  INVALID_SERVER,
  INVALID_CONFIG_FILE,
  INVALID_CLIENT_IPV4,
  INVALID_CLIENT_IPV6,
  LOGGING_CONFIGURATION_ERROR,
  BROKER_REDIRECTION,
  BROKER_REDIRECTION_ERROR,
  SOCKET_ERROR_CANT_CONNECT,
  INITIALIZATION_ERROR,
#ifdef HAP6
  HAP6_INITIALIZATION_ERROR,
  HAP6_SETUP_ERROR,
  HAP6_EXPOSE_DEVICES_ERROR
#endif
} tErrors;


#endif

