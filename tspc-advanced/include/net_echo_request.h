/*
---------------------------------------------------------------------------
 $Id: net_echo_request.h,v 1.2 2007/05/02 13:32:21 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _NET_ECHO_REQUEST_H_
#define _NET_ECHO_REQUEST_H

#define ECHO_REQUEST_COMMAND    "ECHO REQUEST"
#define ECHO_REQUEST_SUCCESS_STATUS 200
#define ECHO_REQUEST_IN_BUF_SIZE  4096
#define ECHO_REQUEST_OUT_BUF_SIZE 4096

#define ECHO_REQUEST_TIMEOUT        10 * 1000
#define ECHO_REQUEST_TIMEOUT_ADJUST     ECHO_REQUEST_TIMEOUT * 2
#define ECHO_REQUEST_ERROR_ADJUST     ECHO_REQUEST_TIMEOUT * 2
#define ECHO_REQUEST_WRONG_FAMILY_ADJUST  ECHO_REQUEST_TIMEOUT * 3

#define ECHO_REQUEST_ATTEMPTS   3

typedef enum {
  SOCKET_ADDRESS_OK,
  SOCKET_ADDRESS_WRONG_FAMILY,
  SOCKET_ADDRESS_PROBLEM_RESOLVING,
  SOCKET_ADDRESS_ERROR
} tSocketAddressStatus;

extern tRedirectStatus tspDoEchoRequest(char *address, tBrokerAddressType address_type, tConf *conf, unsigned int *distance);

#endif
