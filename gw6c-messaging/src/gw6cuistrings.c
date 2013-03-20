/* *********************************************************************** */
/* $Id: gw6cuistrings.c,v 1.9 2007/04/05 22:37:04 krause Exp $            */
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
/*   Offers default UI string for errors and other.                        */
/*                                                                         */
/* You may translate the strings herein as you wish.                       */
/*                                                                         */
/* Author: Charles Nepveu                                                  */
/*                                                                         */
/* Creation Date: November 2006                                            */
/* _______________________________________________________________________ */
/* *********************************************************************** */
#include <gw6cmessaging/gw6cuistrings.h>


/* Struct containing string IDs with the related string.                   */
typedef struct { error_t _id; const char* _str; } tGw6cUIStrings;


static const tGw6cUIStrings Gw6cUIStrings[] = {

  { GW6CM_UIS__NOERROR, "SUCCESS" },    // Should never log this, but...

  /* PIPE ERRORS */
  { GW6CM_UIS_WRITEPIPEFAILED,
    "Failed writing on the named pipe." },
  { GW6CM_UIS_PEEKPIPEFAILED,
    "Failed \"peeking\" IO status on named pipe." },
  { GW6CM_UIS_READPIPEFAILED,
    "Failed reading on the named pipe." },
  { GW6CM_UIS_PIPESERVERALRDUP,
    "Pipe server is already up." },
  { GW6CM_UIS_FAILCREATESERVERPIPE,
    "Failed creation of pipe server." },
  { GW6CM_UIS_CLIENTALRDYCONN,
    "Pipe client is already connected." },
  { GW6CM_UIS_CLIENTCONNFAILED,
    "Pipe client connection failed." },
  { GW6CM_UIS_PIPESVRDISCFAIL,
    "Pipe server disconnection failed." },
  { GW6CM_UIS_FAILCREATECLIENTPIPE,
    "Failed creation of client pipe." },
  { GW6CM_UIS_PIPECLIDISCFAIL,
    "Pipe client disconnection failed." },

  /* IPC LAYER ERRORS */
  { GW6CM_UIS_BADPACKET,
    "Invalid/erroneous IPC data packet received." },
  { GW6CM_UIS_IPCDESYNCHRONIZED,
    "IPC communication desynchronized. Need re-initialization." },
  { GW6CM_UIS_PACKETSNOTORDERED,
    "ERROR, IPC sequential packet number is not ordered." },
  { GW6CM_UIS_READBUFFERTOOSMALL,
    "IPC layer internal buffer size too small to read data packet." },
  { GW6CM_UIS_SENDBUFFERTOOBIG,
    "User message data is too big to be sent through the IPC." },
  { GW6CM_UIS_IOWAITTIMEOUT,
    "Failed acquiring IO mutex to perform requested IPC operation." },

  /* MESSAGING LAYER ERRORS */
  { GW6CM_UIS_MSGPROCDISABLED,
    "Message processing is disabled. Reception of messages is unavailable." },
  { GW6CM_UIS_MESSAGENOTIMPL,
    "Unknown message received. Processing for that message is not implemented." },
  { GW6CM_UIS_CWRAPALRDYINIT,
    "C language wrapper for messaging layer is already initialized." },
  { GW6CM_UIS_CWRAPNOTINIT,
    "C language wrapper for messaging layer is not implemented call initialize_messaging()." },

  /* GATEWAY6 CLIENT ERRORS */
  { GW6CM_UIS_FAILEDBROKERLISTEXTRACTION,
    "Failed redirection broker list extraction." },
  { GW6CM_UIS_CFGDATAERROR,
    "Configuration data is invalid." },
  { GW6CM_UIS_MEMERROR,
    "Memory allocation error." },
  { GW6CM_UIS_REDIRECTIONERROR,
    "Redirection error." },
  { GW6CM_UIS_INTERNALERROR,
    "Internal memory error." },
  { GW6CM_UIS_FAILEDSCRIPT,
    "Failed to execute tunnel creation script." },
  { GW6CM_UIS_AUTHENTICATIONERROR,
    "Authentication error." },
  { GW6CM_UIS_UNKNOWNERROR,
    "Unknown error." },
  { GW6CM_UIS_DISCONNECTEDSLEEPRETRY,
    "Disconnected. Sleeping a bit and retrying later." },
  { GW6CM_UIS_TSPERROR,
	"TSP protocol error." },
  { GW6CM_UIS_SOCKETERROR,
	"Socket error." },
  { GW6CM_UIS_SOCKETERRORCANTCONNECT,
	"Socket error, cannot connect." },
  { GW6CM_UIS_KEEPALIVETIMEOUT,
	"A keepalive timeout happened." },
  { GW6CM_UIS_TUNNELERROR,
	"Internal tunnel error." },
  { GW6CM_UIS_TSPVERSIONERROR,
	"The broker is not supporting this TSP protocol version." },
  { GW6CM_UIS_LEASEEXPIRED,
	"Your tunnel lease is expired." },
  { GW6CM_UIS_SERVERSIDEERROR,
	"Socket error." },
  { GW6CM_UIS_BROKERREDIRECTION,
	"Socket error." },
  { GW6CM_UIS_HAP6_INITIALIZATION_ERROR,
  "The HAP6 subsystem could not be initialized successfully." },
  { GW6CM_UIS_HAP6_SETUP_ERROR,
  "The HAP6 setup script did not complete successfully." },
  { GW6CM_UIS_HAP6_EXPOSE_DEVICES_ERROR,
  "The HAP6 subsystem could not apply the HomeAccess configuration successfully." }
};


// --------------------------------------------------------------------------
// Function : get_mui_string
//
// Description:
//   Returns the user interface string specified by the id.
//
// Arguments:
//   id: int [IN], The string ID.
//
// Return values:
//   The UI string.
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
const char* get_mui_string( const error_t id )
{
  const unsigned int n = sizeof(Gw6cUIStrings) / sizeof(Gw6cUIStrings[0]);
  unsigned int i;

  for(i=0; i<n; i++)
    if(Gw6cUIStrings[i]._id == id)
      return Gw6cUIStrings[i]._str;

  return (const char*)0;    // NULL
}
