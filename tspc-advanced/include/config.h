/*
---------------------------------------------------------------------------
 $Id: config.h,v 1.25 2007/11/28 17:27:06 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/
#ifndef CONFIG_H
#define CONFIG_H


/* these globals are defined by US used by alot of things in  */
#define STR_CONFIG_TUNNELMODE_V6ANYV4   "v6anyv4"
#define STR_CONFIG_TUNNELMODE_V6V4      "v6v4"
#define STR_CONFIG_TUNNELMODE_V6UDPV4   "v6udpv4"
#define STR_CONFIG_TUNNELMODE_V4V6      "v4v6"

#define STR_XML_TUNNELMODE_V6ANYV4    "v6anyv4"
#define STR_XML_TUNNELMODE_V6V4       "v6v4"
#define STR_XML_TUNNELMODE_V6UDPV4    "v6udpv4"
#define STR_XML_TUNNELMODE_V4V6       "v4v6"

#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

#define STR_CONFIG_BOOLEAN_FALSE  "no"
#define STR_CONFIG_BOOLEAN_TRUE   "yes"


typedef enum {
  FALSE=0,
  TRUE
} tBoolean;

typedef enum {
  V6V4=1,
  V6UDPV4=2,
  V6ANYV4=3,
  V4V6=4
} tTunnelMode;


typedef struct stConf {
  char
    *tsp_dir,
    *server,
    *userid,
    *passwd,
    *auth_method,
    *client_v4,
    *client_v6,
    *protocol,
    *if_tunnel_v6v4,
    *if_tunnel_v6udpv4,
    *if_tunnel_v4v6,
    *dns_server,
    *routing_info,
    *if_prefix,
    *template,
    *host_type,
    *log_filename,
    *last_server,
    *hap6_document_root,
    *broker_list;
  int keepalive_interval;
  int prefixlen;
  int retry;
  int syslog_facility;
  int transport;
  int log_rotation_size;
  short log_level_stderr;
  short log_level_syslog;
  short log_level_console;
  short log_level_file;
  tBoolean keepalive;
  tBoolean syslog;
  tBoolean proxy_client;
  tBoolean log_rotation;
  tBoolean log_rotation_delete;
  tBoolean always_use_same_server;
  tBoolean auto_retry_connect;
  tTunnelMode tunnel_mode;
  tBoolean hap6_web_enabled;
  tBoolean hap6_proxy_enabled;

  // These are run-time, dynamically computed values
  //
  char* addr_local_v4[47];     // INET6_ADDRSTRLEN
  unsigned short port_local_v4;
  char* addr_remote_v4[47];    // INET6_ADDRSTRLEN
  unsigned short port_remote_v4;
} tConf;


typedef struct syslog_facility {
  char *string;
  int value;
} syslog_facility_t;


/* Valid syslog_facility values */
#define STR_CONFIG_SLOG_FACILITY_USER   "USER"
#define STR_CONFIG_SLOG_FACILITY_LOCAL0 "LOCAL0"
#define STR_CONFIG_SLOG_FACILITY_LOCAL1 "LOCAL1"
#define STR_CONFIG_SLOG_FACILITY_LOCAL2 "LOCAL2"
#define STR_CONFIG_SLOG_FACILITY_LOCAL3 "LOCAL3"
#define STR_CONFIG_SLOG_FACILITY_LOCAL4 "LOCAL4"
#define STR_CONFIG_SLOG_FACILITY_LOCAL5 "LOCAL5"
#define STR_CONFIG_SLOG_FACILITY_LOCAL6 "LOCAL6"
#define STR_CONFIG_SLOG_FACILITY_LOCAL7 "LOCAL7"


/* Valid log values */
#define STR_CONFIG_LOG_DESTINATION_STDERR   "stderr"
#define STR_CONFIG_LOG_DESTINATION_SYSLOG   "syslog"
#define STR_CONFIG_LOG_DESTINATION_CONSOLE  "console"
#define STR_CONFIG_LOG_DESTINATION_FILE     "file"


/* imports defined in the platform dependant file */
extern char *FileName;
extern char *LogFile;
extern char *ScriptInterpretor;
extern char *ScriptExtension;
extern char *ScriptDir;
extern char *TspHomeDir;
extern char DirSeparator;
extern int RootUid;


/* functions exported */
int tspReadConfigFile(char *, tConf *);
int tspInitialize(int, char *[], tConf *);

#ifdef _USES_SYS_SOCKET_H_
int tspUpdateSourceAddr(tConf *Conf, SOCKET fd);
#endif

#endif



