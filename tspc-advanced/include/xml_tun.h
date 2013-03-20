/*
---------------------------------------------------------------------------
 $Id: xml_tun.h,v 1.9 2007/05/02 13:32:23 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef XML_TUN_H
#define XML_TUN_H

#ifdef XMLTUN
# define ACCESS
#else
# define ACCESS extern
#endif

typedef struct stLinkedList {
  char *Value;
  struct stLinkedList *next;
} tLinkedList;

typedef struct stTunnel {
  char
    *action,
    *type,
    *lifetime,
    *proxy,
    *mtu,
    *client_address_ipv4,
    *client_address_ipv6,
    *client_dns_name,
    *server_address_ipv4,
    *server_address_ipv6,
    *router_protocol,
    *prefix_length,
    *prefix,
    *client_as,
    *server_as,
    *keepalive_interval,
    *keepalive_address;
  tLinkedList
    *dns_server_address_ipv4,
    *dns_server_address_ipv6,
    *broker_address_ipv4,
    *broker_address_ipv6,
  *broker_redirect_ipv4,
  *broker_redirect_ipv6,
  *broker_redirect_dn;
} tTunnel;


ACCESS int  tspXMLParse(char *Data, tTunnel *Tunnel);
ACCESS void tspClearTunnelInfo(tTunnel *Tunnel);

#undef ACCESS
#endif

/*----- xmlparse.h --------------------------------------------------------------------*/










