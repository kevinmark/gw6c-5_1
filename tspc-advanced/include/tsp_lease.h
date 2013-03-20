/*
---------------------------------------------------------------------------
 $Id: tsp_lease.h,v 1.6 2007/11/28 17:27:07 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

extern long tspLeaseGetExpTime    ( const long tun_lifetime );

extern int  tspLeaseCheckExp      ( const long tun_expiration );

extern int  tspLeaseExpLoop       ( const long tun_lifetime );
