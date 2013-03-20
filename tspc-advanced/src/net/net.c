/*
---------------------------------------------------------------------------
 $Id: net.c,v 1.22 2007/11/28 17:27:33 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define _USES_SYS_SOCKET_H_
#define _USES_NETINET_IN_H_
#define _USES_ARPA_INET_H_
#define _USES_NETDB_H_

#include "platform.h"

#include "tsp_net.h"
#include "net.h"
#include "log.h"
#include "hex_strings.h"

/*
 * Convert IPv4 address from string format to in_addr structure.
 * Port information are discarted.
 *
 * in_addr structure must be supplied and allocated
 *
 * return NULL if errors, supplied in_addr structure if success.
 */
struct in_addr *NetText2Addr(char *Address, struct in_addr *in_p)
{
  struct addrinfo hints;
  struct addrinfo *res=NULL, *result=NULL;
  char addr_cp[MAXSERVER];
  char *addr;

  if (NULL == Address || NULL == in_p)
    return NULL;

  /* Prepare hints structure */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = PF_UNSPEC;

  /* copy the string before using strtok */
  strcpy(addr_cp, Address);

  /* be sure it is not an in-brackets v6 address */
  if ((strchr(Address, '[') != NULL) ||
      (strchr(Address, ']') != NULL))
    goto error_v4;

  /* be sure no more than on ':' is used to specify
     port number (not IPv6 address without brakets! */
  if ((addr = strchr(Address, ':')) != NULL) {
    if (strchr(addr+1,':') != NULL)
      goto error_v4;
  }

  /* Remove port number if any */
  addr = addr_cp;
  strtok(addr_cp, ":");

  if ((getaddrinfo(addr, NULL, &hints, &res)) == 0) {

    for (result = res; result; result = result->ai_next) {
      if (result->ai_family != AF_INET)
        continue;
      memcpy(in_p,
       &((struct sockaddr_in *)result->ai_addr)->sin_addr,
       sizeof(struct in_addr));
      freeaddrinfo(res);
      return in_p;
    }

  }

 error_v4:
  /* Cannot resolve */
  Display(LOG_LEVEL_3, ELWarning, "NetText2Addr", HEX_STR_SERVER_NOT_IPV4);

  if (res != NULL)
    freeaddrinfo(res);

  return NULL;
}

/*
 * Convert IPv6 address from string format to in6_addr structure.
 *
 * The following format are supported (port information is ignored):
 *
 * "X:X::X:X"
 * "[X:X::X:X]"
 * "[X:X::X:X]:port"
 * "hostname"
 * "hostname:port"
 *
 * in6_addr structure must be supplied and allocated
 *
 * return NULL if errors, supplied in6_addr structure if success.
 */
struct in6_addr *NetText2Addr6(char *Address, struct in6_addr *in6_p)
{
  struct addrinfo hints;
  struct addrinfo *res    = NULL;
  struct addrinfo *result = NULL;
  char addr[MAXSERVER];
  char *p;
  int c = 0;

  if (NULL == Address || NULL == in6_p)
    return NULL;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = PF_INET6;

  /* Copy the address before stripping */
  addr[sizeof(addr) - 1] = '\0';
  strncpy(addr, Address, sizeof(addr) - 1);

  /*
   * Numeric address has more than one colon
   */
  for(p = addr; *p != '\0'; p++)
    if (':' == *p)
      c++;

  p = addr;

  if (c > 1) {
    /* Numeric address */
    hints.ai_flags = AI_NUMERICHOST;
    /* Strip the bracket and port information if any */
    if ('[' == *p) {
      strtok(p, "]");
      p++; /* Skip [ */
    }
  } else {
    /* Hostname: strip the port information if any */
    strtok(p, ":");
  }

  if ((getaddrinfo(p, NULL, &hints, &res)) == 0) {

    for (result = res; result; result = result->ai_next) {
      if (result->ai_family != AF_INET6)
        continue;
      memcpy(in6_p,
       &((struct sockaddr_in6 *)result->ai_addr)->sin6_addr,
       sizeof(struct in6_addr));
      freeaddrinfo(res);
      return in6_p;
    }

  }

  /* Cannot resolve */
  Display(LOG_LEVEL_3, ELWarning, "NetText2Addr6", HEX_STR_SERVER_NOT_IPV6);

  if (res != NULL)
    freeaddrinfo(res);

  return NULL;

}
