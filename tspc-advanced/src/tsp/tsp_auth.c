/*
---------------------------------------------------------------------------
 $Id: tsp_auth.c,v 1.28 2007/11/28 17:27:35 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define _USES_SYS_SOCKET_H_

#include "platform.h"

#include "tsp_redirect.h"
#include "tsp_client.h"
#include "tsp_auth.h"

#ifndef NO_OPENSSL
#include "tsp_auth_passdss.h"
#endif

#include "tsp_cap.h"
#include "log.h"
#include "hex_strings.h"
#include "base64.h"
#include "md5.h"
#include "version.h"


/* structures */

typedef struct  {
  char *realm,
      *nonce,
      *qop,
      *algorithm,
      *charset,
      *rspauth;
} tChallenge;


static
int
AuthANONYMOUS(SOCKET socket, net_tools_t *nt, tConf *conf, tBrokerList **broker_list)
{
  char Buffer[REDIRECT_RECEIVE_BUFFER_SIZE];
  char string[] = "AUTHENTICATE ANONYMOUS\r\n";

  if (nt->netsendrecv(socket, string, sizeof(string), Buffer, sizeof(Buffer)) == -1) {
    Display(LOG_LEVEL_3, ELError, "AuthANONYMOUS", HEX_STR_CANT_RW_SERVER_SOCKET);
    return -1;
  }

  if (tspIsRedirectStatus(tspGetStatusCode(Buffer))) {
    if (tspHandleRedirect(Buffer, conf, broker_list) == TSP_REDIRECT_OK) {
      return BROKER_REDIRECTION;
    }
    else {
      return BROKER_REDIRECTION_ERROR;
    }
  }

  if (memcmp(Buffer, "200", 3)) { /* Not successful */
    Display(LOG_LEVEL_3, ELError, "AuthANONYMOUS", HEX_STR_NO_SUCCESS, Buffer);
    return -1;
  }

  return 0;
}

/* */

static
int
AuthPLAIN(SOCKET socket, net_tools_t *nt, tConf *conf, tBrokerList **broker_list) {

  char BufferIn[1024];
  char BufferOut[REDIRECT_RECEIVE_BUFFER_SIZE];
  char string[] = "AUTHENTICATE PLAIN\r\n";
  int Length;

  if ( nt->netsend(socket, string, sizeof(string)) == -1 ) {
    Display(LOG_LEVEL_3, ELError, "AuthPLAIN", HEX_STR_CANT_W_SERVER_SOCKET);
    return -1;
  }

  memset(BufferIn, 0, sizeof(BufferIn));
  Length = snprintf(BufferIn, sizeof(BufferIn), "%c%s%c%s\r\n",'\0', conf->userid, '\0', conf->passwd);

  if ( nt->netsendrecv(socket, BufferIn, Length, BufferOut, sizeof(BufferOut)) == -1 ) {
    Display(LOG_LEVEL_3, ELError, "AuthPLAIN", HEX_STR_CANT_RW_SERVER_SOCKET);
    return -1;
  }

  if (tspIsRedirectStatus(tspGetStatusCode(BufferOut))) {
    if (tspHandleRedirect(BufferOut, conf, broker_list) == TSP_REDIRECT_OK) {
      return BROKER_REDIRECTION;
    }
    else {
      return BROKER_REDIRECTION_ERROR;
    }
  }

  if (memcmp(BufferOut, "200", 3)) { /* Not successful */
    Display(LOG_LEVEL_3, ELError, "AuthPLAIN", HEX_STR_NO_SUCCESS, BufferOut);
    return -1;
  }

  return 0;
}

/* */

static
int
InsertInChallegeStruct(tChallenge *c, char *Token, char *Value) {

  if(strcmp(Token, "realm")==0) {
    c->realm = strdup(Value);
    return 0;
  }

  if(strcmp(Token, "nonce")==0) {
    c->nonce = strdup(Value);
    return 0;
  }

  if(strcmp(Token, "qop")==0) {
    c->qop = strdup(Value);
    return 0;
  }

  if(strcmp(Token, "algorithm")==0) {
    c->algorithm = strdup(Value);
    return 0;
  }

  if(strcmp(Token, "charset")==0) {
    c->charset = strdup(Value);
    return 0;
  }

  if(strcmp(Token, "rspauth")==0) {
    c->rspauth = strdup(Value);
    return 0;
  }

  return -1;
}

/* this is sick code */

static void ExtractChallenge(tChallenge *c, char *String)
{
  char *s, *e, *Token, *Value;
  int len;

  memset(c, 0, sizeof(tChallenge));

  Token = (char*) malloc( strlen(String)+1 );
  Value = (char*) malloc( strlen(String)+1 );

  *Token=*Value=0;

  for(s=e=String; ; e++) {
    if(*e== ',' || *e == '\r' || *e == '\n' || *e==0) {
      if(s!=e) {
        if(*Token && (*Value==0)) {
          len = (int)((char *)e-(char *)s);
/* Chop the quotes */
          if((*s == '"') && len) { s++; len--; }
          if((s[len-1] == '"') && len) len--;
          if(len) memcpy(Value, s, len);
          Value[len] = 0;
        }
        if(*Token && *Value) {
          InsertInChallegeStruct(c, Token,Value);
          *Value = *Token = 0;
        }
      }

      if(*e == 0) break;
      s = ++e;
    }

    if((*e=='=' || *e== ',' || *e == '\r' || *e == '\n' || *e==0) && (*Token == 0) && (e != s)) {
      len = (int)((char *)e-(char *)s);
      memcpy(Token, s, len);
      Token[len] = 0;
      if(*e == 0) break;
      s = ++e;
    }
  }

  free( Token );
  free( Value );
}

/* */

static
int
AuthDIGEST_MD5(SOCKET socket, net_tools_t *nt, tConf *conf, tBrokerList **broker_list, int version_index) {

  char Buffer[4096], Response[33], cResponse[33], *ChallengeString;
  char string[] = "AUTHENTICATE DIGEST-MD5\r\n";
  char BufferIn[REDIRECT_RECEIVE_BUFFER_SIZE];
  time_t cnonce = time(NULL);
  tChallenge c;
  memset(BufferIn, 0, sizeof(BufferIn));

  if (nt->netsendrecv(socket, string, sizeof(string), BufferIn, sizeof(BufferIn)) == -1) {
    Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_CANT_RW_SERVER_SOCKET);
    return 1;
  }

  if (tspIsRedirectStatus(tspGetStatusCode(BufferIn))) {
    if (tspHandleRedirect(BufferIn, conf, broker_list) == TSP_REDIRECT_OK) {
      return BROKER_REDIRECTION;
    }
    else {
      return BROKER_REDIRECTION_ERROR;
    }
  }

  if (memcmp(BufferIn, "300", 3) == 0) { /* Not successful */
    Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_NO_SUCCESS, Buffer+3);
    return -1;
  }

  if((ChallengeString = malloc(strlen(BufferIn) + 1)) == NULL) {
    Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_NO_MEM_FOR_CHALLENGE);
    return -1;
  }

  base64decode(ChallengeString, BufferIn);
  ExtractChallenge(&c, ChallengeString);
  free(ChallengeString);

   {
   /*-----------------------------------------------------------*/
   /*
      Extract from : RFC 2831 Digest SASL Mechanism

      Let H(s) be the 16 octet MD5 hash [RFC 1321] of the octet string s.

      Let KD(k, s) be H({k, ":", s}), i.e., the 16 octet hash of the string
      k, a colon and the string s.

      Let HEX(n) be the representation of the 16 octet MD5 hash n as a
      string of 32 hex digits (with alphabetic characters always in lower
      case, since MD5 is case sensitive).

      response-value  =
         HEX( KD ( HEX(H(A1)),
                 { nonce-value, ":" nc-value, ":",
                   cnonce-value, ":", qop-value, ":", HEX(H(A2)) }))

      If authzid is not specified, then A1 is

         A1 = { H( { username-value, ":", realm-value, ":", passwd } ),
           ":", nonce-value, ":", cnonce-value }

      If the "qop" directive's value is "auth", then A2 is:

         A2       = { "AUTHENTICATE:", digest-uri-value }

   */
     char *A1_1Fmt     = "%s:%s:%s",
#ifndef WIN32
         *A1Fmt        = ":%s:%lu",
         *ChallRespFmt = "%s:%s:00000001:%lu:%s:%s",
         *ResponseFmt  = "charset=%s,username=\"%s\",realm=\"%s\",nonce=\"%s\",nc=00000001,cnonce=\"%lu\",digest-uri=\"tsp/%s\",response=%s,qop=auth",
#else
          // 64 bit version.
         *A1Fmt        = ":%s:%I64d",
         *ChallRespFmt = "%s:%s:00000001:%I64d:%s:%s",
         *ResponseFmt  = "charset=%s,username=\"%s\",realm=\"%s\",nonce=\"%s\",nc=00000001,cnonce=\"%I64d\",digest-uri=\"tsp/%s\",response=%s,qop=auth",
#endif
         *A2Fmt        = "%s:tsp/%s",
         A1[33], A1_1[33], A2[33], cA2[33], *String;
     size_t len;

      /*-----------------------------------------------------------*/
      /* Build HEX(H(A2)) & HEX(H(cA2))                            */

     len = strlen(A2Fmt) + 12 /* AUTHENTICATE */ + strlen(conf->server) + 1;
     if((String = malloc(len)) == NULL) {
       Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_NO_MEM_FOR_STRING);
       return -1;
     }

     snprintf(String, len, A2Fmt, "AUTHENTICATE", conf->server);
#ifdef _DEBUG
     printf("A2 = %s\n", String);
#endif
     strncpy(A2, md5(String, strlen(String)), 33);
     snprintf(String, len, A2Fmt, "", conf->server);
#if _DEBUG
     printf("cA2 = %s\n", String);
#endif
     strncpy(cA2, md5(String, strlen(String)), 33);
     free(String);

      /*-----------------------------------------------------------*/
      /* Build HEX(H(A1))                                          */
      /* A1_1 = { username-value, ":", realm-value, ":", passwd }  */
      /* A1 = { H( A1_1 ), ":", nonce-value, ":", cnonce-value }   */

     len = strlen(A1_1Fmt) + strlen(conf->userid) + strlen(c.realm) +
         strlen(conf->passwd) +  1;
     if((String = malloc(len)) == NULL) {
       Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_NO_MEM_FOR_STRING);
       return -1;
     }

     snprintf(String, len, A1_1Fmt, conf->userid, c.realm, conf->passwd);
#if _DEBUG
     printf("A1_1 = %s\n", String);
#endif
     md5digest(String, strlen(String), A1_1);
     free(String);
     len = 16 /* A1_1 */ + 1 +
         strlen(c.nonce) + 16 /* cnonce */ +  1;
     if((String = malloc(len)) == NULL) {
       Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_NO_MEM_FOR_STRING);
       return -1;
     }

     memcpy(String, A1_1, 16);
     snprintf(String + 16, len - 16, A1Fmt, c.nonce, cnonce);
#ifdef SUPPORT_MD5_BUG1455
     A1_1[16] = '\0';
     if ((strlen(A1_1) < 16) &&
        !((strlen(TspProtocolVersionStrings[version_index]) > 5) ||
    (strcmp(TspProtocolVersionStrings[version_index], CLIENT_VERSION_STRING_2_0_0) > 0)))
        strncpy(A1, md5(String, strlen(String)), 33);
     else
#endif /* SUPPORT_MD5_BUG1455 */
         strncpy(A1, md5(String, 16 + strlen(String + 16)), 33);
     free(String);
#if _DEBUG
     printf("A1 = [%s]\n", A1);
#endif

      /*-----------------------------------------------------------*/
      /* Build server's and client's challenge responses           */
     len = strlen(ChallRespFmt) + 32 /* md5(A1) */ + strlen(c.nonce) +16 /* cnonce */ + strlen(c.qop) + 32 /* md5(A2) */ +  1;
     if((String = malloc(len)) == NULL) {
       Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_NO_MEM_FOR_STRING);
       return -1;
     }

     snprintf(String, len, ChallRespFmt, A1, c.nonce, cnonce, c.qop, A2);
#if _DEBUG
     printf("Response = [%s]\n", String);
#endif
     strncpy(Response, md5(String, strlen(String)), 33);
#if _DEBUG
     printf("MD5 Response = %s\n", Response);
#endif
     snprintf(String, len, ChallRespFmt, A1, c.nonce, cnonce, c.qop, cA2);
#if _DEBUG
     printf("cResponse = [%s]\n", String);
#endif
     strncpy(cResponse, md5(String, strlen(String)), 33);
#if _DEBUG
     printf("MD5 cResponse = %s\n", cResponse);
#endif
     free(String);

      /*-----------------------------------------------------------*/
      /* Build Response                                            */
     {
       char   userid[512];  // UserId is theorically limited to 253 chars.
       char * cc;
       size_t i;

        // Escape malicious " and \ from conf->userid.
       for(cc=conf->userid, i=0; *cc && i<512; cc++, i++)
       {
         // Prepend a backslash (\).
         if( *cc == '"'  ||  *cc == '\\' )
           userid[i++] = '\\';
          // Copy character.
          userid[i] = *cc;
       }
       userid[i] = '\0';

       len = strlen(ResponseFmt) + strlen(c.charset) + strlen(userid) +
           strlen(c.realm) + strlen(c.nonce) + 16 /*cnonce*/ +
           strlen(conf->server)    + 32 /* md5 response */;
       if((String = malloc(len)) == NULL) {
         Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_NO_MEM_FOR_STRING);
         return -1;
       }

       snprintf(String, len, ResponseFmt, c.charset, userid, c.realm, c.nonce, cnonce, conf->server, Response);
       memset(Buffer, 0, sizeof(Buffer));
       base64encode(Buffer, String, (int)strlen(String));
       free(String);
     }
   }


   memset(BufferIn, 0, sizeof(BufferIn));

   if ( nt->netprintf(socket, BufferIn, sizeof(BufferIn), "%s\r\n", Buffer) == -1) {
     Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_CANT_W_SERVER_SOCKET);
     return -1;
   }

   /*-----------------------------------------------------------*/
   /* Verify server response                                    */

  if (tspIsRedirectStatus(tspGetStatusCode(BufferIn))) {
    if (tspHandleRedirect(BufferIn, conf, broker_list) == TSP_REDIRECT_OK) {
      return BROKER_REDIRECTION;
    }
    else {
      return BROKER_REDIRECTION_ERROR;
    }
  }

   if (memcmp(BufferIn, "300", 3) == 0) { /* Not successful */
      Display(LOG_LEVEL_3 ,ELError, "AuthDIGEST_MD5", HEX_STR_NO_SUCCESS, Buffer+4);
      return -1;
   }

   if((ChallengeString = malloc(strlen(BufferIn) + 1)) == NULL) {
      Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_NO_MEM_FOR_CHALLENGE);
      return -1;
   }

   base64decode(ChallengeString, BufferIn);
   ExtractChallenge(&c, ChallengeString);
   free(ChallengeString);

   if(memcmp(c.rspauth, cResponse, 32)) {
      Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_INVALID_RESPONSE_FROM_SERVER);
      return -1;
   }

   if (nt->netrecv(socket, Buffer, sizeof (Buffer) ) == -1) {
      Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_CANT_R_SERVER_SOCKET);
      return -1;
   }

  if (tspIsRedirectStatus(tspGetStatusCode(BufferIn))) {
    if (tspHandleRedirect(BufferIn, conf, broker_list) == TSP_REDIRECT_OK) {
      return BROKER_REDIRECTION;
    }
    else {
      return BROKER_REDIRECTION_ERROR;
    }
  }

   if (memcmp(Buffer, "200", 3)) { /* Not successful */
      Display(LOG_LEVEL_3, ELError, "AuthDIGEST_MD5", HEX_STR_NO_SUCCESS, Buffer+4);
      return -1;
   }

   return 0;
}

/* */

// --------------------------------------------------------------------------
// Function : tspAuthenticate
//
// Synopsys: Will authenticate a session with the broker.
//
// Description:
//   First, we'll try to find the most secure common authentication method.
//   Once the authentication method has been chosen, the authentication
//   process is initiated with the broker.
//
// Arguments: (only local-specific arguments are listed here)
//   cap: bitfield [IN], The authentication methods suported by the broker.
//   conf: tConf* [IN], The global configuration object.
//
// Return values:
//   0 upon successful authentication;
//  -1 upon error;
//  -100 when no common authentication method was found;
//  BROKER_REDIRECTION when a redirection occured.
//
// --------------------------------------------------------------------------
int tspAuthenticate(SOCKET socket, tCapability cap, net_tools_t *nt, tConf *conf, tBrokerList **broker_list, int version_index)
{
  int rc = -100;
  tCapability Mechanism;


  // Get mechanism, depending on requested authentication method.
  if( strcasecmp( conf->auth_method, "any" ) == 0 )
    Mechanism = AUTH_ANY;
  else
    Mechanism = tspSetCapability("AUTH", conf->auth_method);


  if( strcasecmp( conf->auth_method, "anonymous" ) != 0 )
  {
    // Try the most secure authentication methods first:
#ifndef NO_OPENSSL
    if( Mechanism & cap & AUTH_PASSDSS_3DES_1 )
    {
      Display(LOG_LEVEL_3, ELInfo, "tspAuthenticate", HEX_STR_USING_AUTH_PASSDSS_3DES_1);
      rc = AuthPASSDSS_3DES_1(socket, nt, conf, broker_list);
      goto EndAuthenticate;
    }
#endif
    if( Mechanism & cap & AUTH_DIGEST_MD5 )
    {
      Display(LOG_LEVEL_3, ELInfo, "tspAuthenticate", HEX_STR_USING_AUTH_DIGEST_MD5);
      rc = AuthDIGEST_MD5(socket, nt, conf, broker_list, version_index);
      goto EndAuthenticate;
    }

    if( Mechanism & cap & AUTH_PLAIN )
    {
      Display(LOG_LEVEL_3, ELInfo, "tspAuthenticate", HEX_STR_USING_AUTH_PLAIN);
      rc = AuthPLAIN(socket, nt, conf, broker_list);
      goto EndAuthenticate;
    }
  }
  else
  {
    // Finally, try anonymous if possible.
    if( Mechanism & cap & AUTH_ANONYMOUS )
    {
      Display(LOG_LEVEL_3, ELInfo, "tspAuthenticate", HEX_STR_USING_AUTH_ANONYMOUS);
      rc = AuthANONYMOUS(socket, nt, conf, broker_list);
      goto EndAuthenticate;
    }
  }

EndAuthenticate:
  if(rc == -100)
  {
    const char* szStrings[] = { "Server Authentication Capabilities: ",
                                "Your Configured Authentication:     " };
    size_t nWritten;
    char bufDisplay[256];

    // Display server authentication capabilities.
    snprintf( bufDisplay, sizeof(bufDisplay), "%s", szStrings[0] );
    nWritten = strlen( szStrings[0] );
    tspFormatCapabilities( bufDisplay + nWritten, sizeof(bufDisplay) - nWritten, cap );
    Display( LOG_LEVEL_1, ELWarning, "tspAuthenticate", bufDisplay );

    // Display user authentication choice.
    snprintf( bufDisplay, sizeof(bufDisplay), "%s", szStrings[1] );
    nWritten = strlen( szStrings[1] );
    tspFormatCapabilities( bufDisplay + nWritten, sizeof(bufDisplay) - nWritten, Mechanism );
    Display( LOG_LEVEL_1, ELWarning, "tspAuthenticate", bufDisplay );

    // Failed to find a common authentication method.
    Display(LOG_LEVEL_1, ELError, "tspAuthenticate", HEX_STR_NO_COMMON_AUTH_MECH);
  }

  return rc;
}






