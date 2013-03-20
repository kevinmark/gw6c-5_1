/*
---------------------------------------------------------------------------
 $Id: xml_req.c,v 1.8 2005/10/12 18:52:19 jfboud Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2005 Hexago Inc. All rights reserved

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"

#include "xml_req.h"
#include "config.h"


/*
   Create XML request for tunnel
*/

char *tspBuildCreateRequest(tConf *conf)
{
  static char *Request[5000], Buffer[1024];

  /* XXX: This routine may overflow Request */
  memset(Request, 0, sizeof(Request));
  
  strcpy((char *)Request, "<tunnel action=\"create\" type=\"");
  
  if (conf->tunnel_mode == V6UDPV4)
     strcat((char *)Request, STR_XML_TUNNELMODE_V6UDPV4);
  else if (conf->tunnel_mode == V6V4)
    strcat((char *)Request, STR_XML_TUNNELMODE_V6V4);
#ifdef V4V6_SUPPORT  
  else if (conf->tunnel_mode == V4V6)
    strcat((char *)Request, STR_XML_TUNNELMODE_V4V6);
#endif /* V4V6_SUPPORT */  
  else
    strcat((char *)Request, STR_XML_TUNNELMODE_V6ANYV4);

  if (conf->proxy_client == TRUE)
    strcat((char *)Request, "\" proxy=\"yes\">\r\n");
  else
    strcat((char *)Request, "\" proxy=\"no\">\r\n");
  
  strcat((char *)Request, " <client>\r\n");

  if (conf->tunnel_mode != V4V6)
	snprintf(Buffer, sizeof Buffer,
		 "  <address type=\"ipv4\">%s</address>\r\n", conf->client_v4);
#ifdef V4V6_SUPPORT  
  if (conf->tunnel_mode == V4V6)
	snprintf(Buffer, sizeof Buffer,
		 "  <address type=\"ipv6\">%s</address>\r\n", conf->client_v6);
#endif /* V4V6_SUPPORT */	 
  strcat((char *)Request, Buffer);

  /* ------------------------------------------------------- */
  /*                     KEEPALIVE                           */
  /* ------------------------------------------------------- */
  if (conf->keepalive == TRUE) {
    if (conf->tunnel_mode != V4V6)
	    snprintf(Buffer, sizeof Buffer, "  <keepalive interval=\"%d\">\r\n    <address type=\"ipv6\">::</address>\r\n  </keepalive>\r\n",conf->keepalive_interval);
#ifdef V4V6_SUPPORT    
    if (conf->tunnel_mode == V4V6)
	    snprintf(Buffer, sizeof Buffer, "  <keepalive interval=\"%d\">\r\n    <address type=\"ipv4\">127.0.0.1</address>\r\n  </keepalive>\r\n",conf->keepalive_interval);
#endif /* V4V6_SUPPORT */    
    strcat((char *) Request, Buffer);
  }
  
  /* ------------------------------------------------------- */
  /*                 ROUTER SECTION                          */
  /* ------------------------------------------------------- */
  if (strcmp(conf->host_type, "router") == 0) {

    strcat((char *)Request, "  <router");

    if (strcmp(conf->protocol, "default_route") != 0) {
      snprintf(Buffer, sizeof Buffer,
	      " protocol=\"%s\"",
	      conf->protocol);
      strcat((char *)Request, Buffer);
    }

    strcat((char *)Request, ">\r\n");

    if (conf->prefixlen==0) {
     if (conf->tunnel_mode != V4V6)
      conf->prefixlen = 48; /* default to 48 for v6anyv4 */
#ifdef V4V6_SUPPORT     
     if (conf->tunnel_mode == V4V6)
      conf->prefixlen = 24; /* default to 24 for v4v6 */
#endif /* V4V6_SUPPORT */     
    }
    snprintf(Buffer, sizeof Buffer,
	      "   <prefix length=\"%d\"/>\r\n",
	      conf->prefixlen);
    strcat((char *)Request, Buffer);
    

    /* ------------------------------------------------------- */
    /*                 REVERSE DNS DELEGATION                  */
    /* ------------------------------------------------------- */
    if(strlen(conf->dns_server)) {
       char *Server;
       strcat((char *)Request, "   <dns_server>\r\n");
       for(Server = strtok(conf->dns_server, ":");Server; Server = strtok(NULL, ":")) {
          snprintf(Buffer, sizeof Buffer,
              "     <address type=\"dn\">%s</address>\r\n", Server);
          strcat((char *)Request, Buffer);
       }
       strcat((char *)Request, "   </dns_server>\r\n");
    }

    /* ------------------------------------------------------- */
    /*                 ROUTING PROTOCOL SECTION                */
    /* ------------------------------------------------------- */
    if (strcmp(conf->protocol, "default_route") == 0) {
      if (strlen(conf->routing_info) > 0) {
	snprintf(Buffer, sizeof Buffer,
		"    %s\r\n", 
		conf->routing_info);
	strcat((char *)Request, Buffer);
      }
    }

    strcat((char *)Request, "  </router>\r\n");
  }

  strcat((char *)Request, " </client>\r\n");
  strcat((char *)Request, "</tunnel>\r\n");

  return (char *)Request;
}

/*
  Create XML tunnel acknowledge
*/
char *tspBuildCreateAcknowledge()
{
  /*XXX Based on BuildCreateRequest - this is a reminder to fix memory usage of both functions.*/
  static char *Request[5000];

  memset(Request, 0, sizeof(Request));
  strcpy((char *)Request, "<tunnel action=\"accept\"></tunnel>\r\n");

  return (char *)Request;  
}
