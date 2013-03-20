/*
---------------------------------------------------------------------------
 $Id: tsp_lease.c,v 1.8 2007/11/28 17:27:36 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2006 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <unistd.h>

#define _USES_SYS_TIME_H_
#include "platform.h"

#include "log.h"
#include "hex_strings.h"
#include "errors.h"


// --------------------------------------------------------------------------
// This function returns the time at which tunnel will be expired.
// The return value should be used with function tspLeaseCheck().
//
long tspLeaseGetExpTime( const long tun_lifetime )
{
  struct timeval tv;

  // get current time.
  GETTIMEOFDAY(&tv, NULL);

  // calculate expiration time.
  return tv.tv_sec + tun_lifetime;
}


/*
 * This function verify if expiration time is reached.
 */
int tspLeaseCheckExp( const long tun_expiration )
{
  struct timeval tv;

  /* get current time */
  GETTIMEOFDAY(&tv, NULL);

  /* if expired, return 1 */
  if (tv.tv_sec > tun_expiration)
    return 1;

  else return 0;
}

/*
 * This function calculate expiration time and loop
 * until we reach it.
 */
int tspLeaseExpLoop( const long tun_lifetime )
{
  long expiration;

  /* calculate expiration time */
  expiration = tspLeaseGetExpTime( tun_lifetime );

  Display(LOG_LEVEL_3, ELInfo, "tspLeaseLoop", HEX_STR_SLEEPING_FOR_ALLOC_LEASE);

  /* loop until expiration */
  for (;;) {
    SLEEP(1);
    if (tspLeaseCheckExp(expiration) == 1)
      return LEASE_EXPIRED;
  }

  /* should never reach here */
  return 0;
}
