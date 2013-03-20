/*
---------------------------------------------------------------------------
 $Id: console.h,v 1.1 2006/09/24 01:09:22 dgregoire Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2006 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

HANDLE initialize_console_input();

BOOL enable_console_input(void);
BOOL disable_console_input(void);

#endif
