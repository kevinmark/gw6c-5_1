/*
---------------------------------------------------------------------------
 $Id: version.c,v 1.3 2005/06/07 21:16:42 dgregoire Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2005 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include "platform.h"

#include "version.h"

char *tsp_get_version(void) {
  return IDENTIFICATION;
}
