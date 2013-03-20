/*
---------------------------------------------------------------------------
 $Id: cli.h,v 1.8 2007/05/18 16:33:45 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _CLI_H_
#define _CLI_H_

int ask(char *question, ...);
void PrintUsage(char *, ...);
int ParseArguments(int, char *[], tConf *);


#endif
