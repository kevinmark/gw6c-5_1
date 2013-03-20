/*
---------------------------------------------------------------------------
 $Id: service.h,v 1.2 2007/05/02 13:32:22 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2001-2005 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _SERVICE_H_
#define _SERVICE_H_

BOOL service_init();
BOOL service_create(TCHAR *name);
BOOL service_delete(TCHAR *name);
void service_parse_cli(int argc, TCHAR *argv[]);

void service_main(int argc, TCHAR *argv[]);

#endif
