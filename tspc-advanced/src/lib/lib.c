/*
---------------------------------------------------------------------------
 $Id: lib.c,v 1.23 2007/05/23 19:59:39 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <stdio.h>
#include <string.h>

#define _USES_NETINET_IP6_H_
#define _USES_SYS_SOCKET_H_
#define _USES_NETDB_H_
#define _USES_NETINET_IN_H_
#include "platform.h"
#include "hex_strings.h"

#include "lib.h"

// these must be kept in sync with errors.h

char *TspErrorCodesArray[] = { "NO_ERROR",
             "NO_ERROR_SHOW_HELP",
             "TSP_ERROR",
             "SOCKET_ERROR",
             "INTERFACE_SETUP_FAILED",
             "KEEPALIVE_TIMEOUT",
           "KEEPALIVE_ERROR",
             "TUNNEL_ERROR",
             "TSP_VERSION_ERROR",
             "AUTHENTICATION_ERROR",
             "LEASE_EXPIRED",
             "SERVER_SIDE_ERROR",
             "INVALID_ARGUMENTS",
             "MEMORY_ERROR",
             "INVALID_SERVER",
             "INVALID_CONFIG_FILE",
             "INVALID_CLIENT_IPV4",
             "INVALID_CLIENT_IPV6",
                   "LOGGING_CONFIGURATION_ERROR",
                   "BROKER_REDIRECTION",
                   "BROKER_REDIRECTION_ERROR",
                   "SOCKET_ERROR_CANT_CONNECT",
           "INITIALIZATION_ERROR",
#ifdef HAP6
              "HAP6_INITIALIZATION_ERROR",
              "HAP6_SETUP_ERROR",
              "HAP6_EXPOSE_DEVICES_ERROR",
#endif
             NULL
};

/*
   Check if all characters in Value are within AllowedChars list.
*/

int IsAll(char *AllowedChars, char *Value)
{
  if(Value) {
    for(;*Value; Value++) {
      if(strchr(AllowedChars, *Value) == NULL)
        return 0;
    }
  } else {
    return 0;
  }
  return 1;
}

/*
   Check to see if there is a value in the char *
   If not, then the value was not supplied.
*/

int IsPresent(char *Value)
{
  if(Value)
    if(strlen(Value))
      return 1;
  return 0;
}

/* This next function is very dangerous.
   If you can call be certain the array
   finished by NULL or it will do bad things.
   */

int GetSizeOfNullTerminatedArray(char **a) {
  int i;
  for (i = 0;;i++) 
  {
    if (a[i] == NULL)
      return i;
  }
  // unreachable code.
}


char *tspGetErrorByCode(int code) {
  static char buf[1024];
  int i;

  i = GetSizeOfNullTerminatedArray(TspErrorCodesArray);
  if (code < i && code > -1)
    return TspErrorCodesArray[code];
  else
    snprintf(buf, sizeof(buf), HEX_STR_NOT_DEF_AS_CLIENT_ERROR, code);
  return buf;
}


/* Function: IsAddressInPrefix

  Verifies if a given IPv6 address is part of the given IPv6 prefix.

  Returns 0 if address is part of prefix.
  Returns 1 if address is NOT part of prefix.
  Returns -1 if either address or prefix is an invalid IPv6 address.

*/
int IsAddressInPrefix( const char* address, const char* prefix, const short prefix_len )
{
  short compare_bytes=0;
  short compare_bits =0;
  int ret_code=1;
  struct addrinfo hints, *res_address=NULL, *res_prefix=NULL;
  struct in6_addr *in6_address, *in6_prefix;

  memset( &hints, 0x00, sizeof(struct addrinfo) );
  hints.ai_family = AF_INET6;

  while( 1 )
  {
    if( (prefix_len > 0   &&  prefix_len <= 128) &&
        (address != NULL  &&  prefix != NULL   ) &&
        (getaddrinfo( address, NULL, &hints, &res_address ) != 0  ||  getaddrinfo( prefix, NULL, &hints, &res_prefix )   != 0) )
    {
      ret_code = -1;
      break;
    }

    if( res_address == NULL  ||  res_prefix == NULL )
    {
      ret_code = -1;
      break;
    }
    in6_address   = &((struct sockaddr_in6*)(res_address->ai_addr))->sin6_addr;
    in6_prefix    = &((struct sockaddr_in6*)(res_prefix->ai_addr))->sin6_addr;

    /* Compute how many bytes and bits to compare */
    compare_bytes = prefix_len / 8;
    compare_bits  = prefix_len % 8;

    /* Compare the bytes of address and prefix */
    if( memcmp( in6_address, in6_prefix, compare_bytes ) != 0 )
      break;

    /* Compare the bits */
    if( compare_bits > 0 )
      if( (in6_address->s6_addr[compare_bytes] >> compare_bits) != (in6_prefix->s6_addr[compare_bytes] >> compare_bits) )
        break;

    /* address is part of prefix */
    ret_code = 0;
    break;
  }

  /* Free memory used. */
  if( res_address ) freeaddrinfo( res_address );
  if( res_prefix  ) freeaddrinfo( res_prefix );


  return ret_code;
}


