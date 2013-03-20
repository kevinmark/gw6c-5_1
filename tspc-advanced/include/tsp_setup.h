/*
---------------------------------------------------------------------------
 $Id: tsp_setup.h,v 1.6 2007/11/28 17:27:07 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _TSP_SETUP_H_
#define _TSP_SETUP_H_

#include "config.h"
#include "xml_tun.h"


int execScript            ( const char *cmd );

int tspSetupInterface     ( tConf *c, tTunnel *t );

int tspTearDownTunnel     ( tConf* pConf, tTunnel* pTunInfo );

#endif
