/*
---------------------------------------------------------------------------
 $Id: tsp_cap.c,v 1.26 2007/05/18 16:10:49 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define _USES_SYS_SOCKET_H_

#include "platform.h"

#include "tsp_cap.h"
#include "tsp_client.h"	// tspGetStatusCode()
#include "net.h"
#include "log.h"
#include "hex_strings.h"
#include "version.h"
#include "tsp_redirect.h"

/* static functions */

/* Convert a CAPABILITY string in corresponding bit in capability flag */

static tCapability tspExtractCapability(char *String)
{
	tCapability flags = 0;
	char *s, *e, *Token, *Value;
	int len;

  Token = (char*) malloc( strlen(String)+1 );
  Value = (char*) malloc( strlen(String)+1 );
	*Token = *Value = 0;

	for(s=e=String+11; *e; e++) {
		if(*e== ' ' || *e == '\r' || *e == '\n' || *e == 0) {
			if(s!=e) {
				if(*Token && (*Value == 0)) {
					len = (int)((char *)e-(char *)s);
					memcpy(Value, s, len);
					Value[len] = 0;
				}
				if(*Token && *Value) {
					flags |= tspSetCapability(Token,Value);
					*Value = *Token = 0;
				}
			}
			s = ++e;
		}

		if((*e=='=' || *e== ' ' || *e == '\r' || *e == '\n' || *e == 0) && (e != s)) {
			len = (int)((char *)e-(char *)s);
			memcpy(Token, s, len);
			Token[len] = 0;
			s = ++e;
		}
	}
  
  free( Token );
  free( Value );

	return flags;
}

/* exports */

/* Return the capability flag corrsponding to the token */
tCapability
tspSetCapability(char *Token, char *Value) {
	if(strcmp("TUNNEL", Token)==0) {
		if(strcmp("V6V4", Value)==0)
			return TUNNEL_V6V4;
		if(strcmp("V6UDPV4", Value)==0)
			return TUNNEL_V6UDPV4;
#ifdef V4V6_SUPPORT		
		if(strcmp("V4V6", Value)==0)
			return TUNNEL_V4V6;
#endif /* V4V6_SUPPORT */
	}
	if(strcmp("AUTH", Token)==0) {
#ifndef NO_OPENSSL
		if(strcasecmp("PASSDSS-3DES-1", Value)==0)
			return AUTH_PASSDSS_3DES_1;
#endif
		if(strcasecmp("DIGEST-MD5", Value)==0)
			return AUTH_DIGEST_MD5;
		if(strcasecmp("ANONYMOUS", Value)==0)
			return AUTH_ANONYMOUS;
		if(strcasecmp("PLAIN", Value)==0)
			return AUTH_PLAIN;
	}
	return 0;
}

tErrors
tspGetCapabilities(SOCKET socket, net_tools_t *nt, tCapability *capability, int version_index, tConf *conf, tBrokerList **broker_list) {
	char dataout[256];
	char datain[REDIRECT_RECEIVE_BUFFER_SIZE];

	snprintf(dataout, sizeof(dataout), "VERSION=%s\r\n", TspProtocolVersionStrings[version_index]);
	memset(datain, -0, sizeof(datain));

	if (nt->netsendrecv(socket, dataout, (int)strlen(dataout), datain, (int)sizeof(datain)) == -1) 
		return SOCKET_ERROR;

	if(memcmp("CAPABILITY ", datain, 11) == 0)
		*capability = tspExtractCapability(datain);
	else {
		if ( tspGetStatusCode(datain) == 302 )
			return TSP_VERSION_ERROR;
	
		if (tspIsRedirectStatus(tspGetStatusCode(datain))) {
			if (tspHandleRedirect(datain, conf, broker_list) == TSP_REDIRECT_OK) {
				return BROKER_REDIRECTION;
			}
			else {
				return BROKER_REDIRECTION_ERROR;
			}
		}

		Display(LOG_LEVEL_3, ELInfo, "Capability", HEX_STR_ERR_FROM_SERVER, datain);
		return TSP_ERROR;
	}

	return NO_ERROR;
}

// Formats a comma-separated string containing the capability(ies) in the buffer provided.
// Returns the buffer.
char* tspFormatCapabilities( char* szBuffer, const size_t bufLen, const tCapability cap )
{
  static const char* authTab[] = { "Any, ", 
#ifndef NO_OPENSSL
                                   "Passdss-3des-1, ",
#endif
                                   "Digest MD5, ",
                                   "Anonymous, ",
                                   "Plain, " };
  size_t nWritten = 0;

  /*
    AUTH_ANY is explicitly expanded in the following capabilities.
  */

#ifndef NO_OPENSSL
  if( (cap & AUTH_PASSDSS_3DES_1)  == AUTH_PASSDSS_3DES_1 )
  {
    strncpy( szBuffer + nWritten, authTab[1], bufLen - nWritten );
    nWritten += strlen( authTab[1] );
  }
#endif

  if( (cap & AUTH_DIGEST_MD5)  == AUTH_DIGEST_MD5 )
  {
    strncpy( szBuffer + nWritten, authTab[2], bufLen - nWritten );
    nWritten += strlen( authTab[2] );
  }
  if( (cap & AUTH_ANONYMOUS)  ==  AUTH_ANONYMOUS  &&  (cap & AUTH_ANY)  !=  AUTH_ANY )
  {
    // ANY does not include ANONYMOUS
    strncpy( szBuffer + nWritten, authTab[3], bufLen - nWritten );
    nWritten += strlen( authTab[3] );
  }
  if( (cap & AUTH_PLAIN)  ==  AUTH_PLAIN )
  {
    strncpy( szBuffer + nWritten, authTab[4], bufLen - nWritten );
    nWritten += strlen( authTab[4] );
  }

  if( nWritten > 2 )
    szBuffer[nWritten-2] = '\0';  // Remove the end comma.


  return szBuffer;
}

