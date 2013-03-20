/*
---------------------------------------------------------------------------
 $Id: version.h,v 1.58 2007/11/28 17:27:07 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef VERSION_H
#define VERSION_H

// Software option.
#ifdef HAP6
#define TSP_CLIENT_OPT_SOFT "HAP6 enabled"
#else
#define TSP_CLIENT_OPT_SOFT ""
#endif

// Architecture type (For Windows builds only).
#ifdef WIN32
  #ifdef BUILD_OPT_X64
    #define TSP_CLIENT_OPT_ARCH "64-bit"
  #else
    #define TSP_CLIENT_OPT_ARCH "32-bit"
  #endif
#else
  #define TSP_CLIENT_OPT_ARCH ""
#endif

// Version number.
#define TSP_CLIENT_VERSION "5.1-RELEASE"

// Identification string.
#define IDENTIFICATION "Gateway6 Client v" TSP_CLIENT_VERSION " build " __DATE__ \
        "-" __TIME__ " " TSP_CLIENT_OPT_SOFT " " TSP_CLIENT_OPT_ARCH


// defined in tsp_client.c
extern char *TspProtocolVersionStrings[];
char *tsp_get_version(void);

#endif

/*----- version.h ----------------------------------------------------------*/

