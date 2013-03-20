/* *********************************************************************** */
/* $Id: gw6cuistrings.h,v 1.17 2007/04/05 22:35:48 krause Exp $           */
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
/*   Contains the user interface(UI) strings of the Gateway6 Configuration */
/*   subsystem.                                                            */
/*                                                                         */
/* Author: Charles Nepveu                                                  */
/*                                                                         */
/* Creation Date: November 2006                                            */
/* _______________________________________________________________________ */
/* *********************************************************************** */
#ifndef __gw6cmessaging_gw6cuistrings_h__
#define __gw6cmessaging_gw6cuistrings_h__


#ifndef ERRORT_DEFINED
#define ERRORT_DEFINED
typedef signed int error_t;
#endif


/* ----------------------------------------------------------------------- */
/* Gateway6 Client User Interface string ID definitions.                   */
/* ----------------------------------------------------------------------- */
#define GW6CM_UIS__NOERROR                          (error_t)0x00000000

#define GW6CM_UIS_WRITEPIPEFAILED                   (error_t)0x00000001
#define GW6CM_UIS_PEEKPIPEFAILED                    (error_t)0x00000002
#define GW6CM_UIS_READPIPEFAILED                    (error_t)0x00000003
#define GW6CM_UIS_PIPESERVERALRDUP                  (error_t)0x00000004
#define GW6CM_UIS_FAILCREATESERVERPIPE              (error_t)0x00000005
#define GW6CM_UIS_CLIENTALRDYCONN                   (error_t)0x00000006
#define GW6CM_UIS_CLIENTCONNFAILED                  (error_t)0x00000007
#define GW6CM_UIS_PIPESVRDISCFAIL                   (error_t)0x00000008
#define GW6CM_UIS_FAILCREATECLIENTPIPE              (error_t)0x00000009
#define GW6CM_UIS_PIPECLIDISCFAIL                   (error_t)0x0000000A

#define GW6CM_UIS_BADPACKET                         (error_t)0x0000000B
#define GW6CM_UIS_IPCDESYNCHRONIZED                 (error_t)0x0000000C
#define GW6CM_UIS_PACKETSNOTORDERED                 (error_t)0x0000000D
#define GW6CM_UIS_READBUFFERTOOSMALL                (error_t)0x0000000E
#define GW6CM_UIS_SENDBUFFERTOOBIG                  (error_t)0x0000000F
#define GW6CM_UIS_IOWAITTIMEOUT                     (error_t)0x00000010
#define GW6CM_UIS_MSGPROCDISABLED                   (error_t)0x00000011
#define GW6CM_UIS_MESSAGENOTIMPL                    (error_t)0x00000012
#define GW6CM_UIS_CWRAPALRDYINIT                    (error_t)0x00000013
#define GW6CM_UIS_CWRAPNOTINIT                      (error_t)0x00000014

// Gateway6 Client errors
#define GW6CM_UIS_FAILEDBROKERLISTEXTRACTION        (error_t)0x00000015
#define GW6CM_UIS_CFGDATAERROR                      (error_t)0x00000016
#define GW6CM_UIS_MEMERROR                          (error_t)0x00000017
#define GW6CM_UIS_REDIRECTIONERROR                  (error_t)0x00000018
#define GW6CM_UIS_INTERNALERROR                     (error_t)0x00000019
#define GW6CM_UIS_FAILEDSCRIPT                      (error_t)0x0000001A
#define GW6CM_UIS_AUTHENTICATIONERROR               (error_t)0x0000001B
#define GW6CM_UIS_UNKNOWNERROR                      (error_t)0x0000001C
#define GW6CM_UIS_DISCONNECTEDSLEEPRETRY            (error_t)0x0000001D
#define GW6CM_UIS_TSPERROR                          (error_t)0x0000001E
#define GW6CM_UIS_SOCKETERROR                       (error_t)0x0000001F
#define GW6CM_UIS_SOCKETERRORCANTCONNECT            (error_t)0x00000020
#define GW6CM_UIS_KEEPALIVETIMEOUT                  (error_t)0x00000021
#define GW6CM_UIS_TUNNELERROR                       (error_t)0x00000022
#define GW6CM_UIS_TSPVERSIONERROR                   (error_t)0x00000023
#define GW6CM_UIS_LEASEEXPIRED                      (error_t)0x00000024
#define GW6CM_UIS_SERVERSIDEERROR                   (error_t)0x00000025
#define GW6CM_UIS_BROKERREDIRECTION                 (error_t)0x00000026
#define GW6CM_UIS_HAP6_INITIALIZATION_ERROR         (error_t)0x00000027
#define GW6CM_UIS_HAP6_SETUP_ERROR                  (error_t)0x00000028
#define GW6CM_UIS_HAP6_EXPOSE_DEVICES_ERROR         (error_t)0x00000029


/* ----------------------------------------------------------------------- */
/* Get string function.                                                    */
/* ----------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" const char* get_mui_string( const error_t id );
#else
const char* get_mui_string( const error_t id );
#endif

#endif
