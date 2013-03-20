/* *********************************************************************** */
/* $Id: hap6_devmap_c_wrap.h,v 1.2 2007/03/22 16:14:20 cnepveu Exp $       */
/*                                                                         */
/* Copyright (c) 2007 Hexago Inc. All rights reserved.                     */
/*                                                                         */
/*   LICENSE NOTICE: You may use and modify this source code only if you   */
/*   have executed a valid license agreement with Hexago Inc. granting     */
/*   you the right to do so, the said license agreement governing such     */
/*   use and modifications.   Copyright or other intellectual property     */
/*   notices are not to be removed from the source code.                   */
/*                                                                         */
/* Description:                                                            */
/*   Wraps the HAP6 Device Mapping configuration data to offer C access.   */
/*                                                                         */
/* Author: Charles Nepveu                                                  */
/*                                                                         */
/* Creation Date: Febuary 2007                                             */
/* _______________________________________________________________________ */
/* *********************************************************************** */
#ifndef __gw6cconfig_hap6_devmap_c_wrap_h__
#define __gw6cconfig_hap6_devmap_c_wrap_h__


#ifdef __cplusplus
extern "C" {
#endif


// Structure definition to hold HAP6 device mappings
struct __DEVICE_MAPPING;
typedef struct __DEVICE_MAPPING
{
  char*             szName;
  char*             szAddress;
  struct __DEVICE_MAPPING* next;
} DEVICE_MAPPING, *PDEVICE_MAPPING;


/* ----------------------------------------------------------------------- */
/* HAP6 Device Mapping management functions.                               */
/* ----------------------------------------------------------------------- */
int                 init_hap6_devmap      ( const char* );
int                 reload_hap6_devmap    ( void );
void                uninit_hap6_devmap    ( void );

/* ----------------------------------------------------------------------- */
/* HAP6 Mapping data accessors.                                            */
/* ----------------------------------------------------------------------- */
int                 get_device_mapping    ( PDEVICE_MAPPING* ppDeviceMapping );
void                free_device_mapping   ( PDEVICE_MAPPING* ppDeviceMapping );


#ifdef __cplusplus
}
#endif

#endif
