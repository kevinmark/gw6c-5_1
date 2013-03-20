/*
---------------------------------------------------------------------------
 $Id: net_rudp.h,v 1.10 2007/05/23 19:19:28 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef _rudp_h_
#define _rudp_h_

#define RTTENGINE_G  (float)1/8
#define RTTENGINE_H  (float)1/4
#define RTTENGINE_TMIN  2
#define RTTENGINE_TMAX  30
#define RTTENGINE_MAXRTT 3
#define RTTENGINE_MAXRT  8

#include <sys/types.h>

extern SOCKET   NetRUDPConnect    (char *, unsigned short);
extern int      NetRUDPClose      (SOCKET);

extern int      NetRUDPReadWrite  (SOCKET, char *, int, char *, int);

extern int      NetRUDPWrite      (SOCKET, char *, int);
extern int      NetRUDPPrintf     (SOCKET, char *, int, char *, ...);

extern int      NetRUDPRead       (SOCKET, char *, int);


typedef struct rudp_message_struct {
  uint32_t sequence;
  uint32_t timestamp;
} rudp_msghdr_t;


typedef struct rttengine_statistics {
  /* connected udp host stats */
  struct sockaddr* sai;

  /* stat stats */

  float rtt;
  float srtt;
  float rttvar;
  float rto;

  /* timeline stats */
  uint32_t sequence;
  int retries;
  int32_t last_recv_sequence;
  int32_t initial_timestamp;
  int apply_backoff;
  int has_peer;
  int initiated;
} rttengine_stat_t;

extern rttengine_stat_t rttengine_stats;

/* rudp engine functions */
extern int rttengine_init(rttengine_stat_t *);
extern int rttengine_deinit(rttengine_stat_t *, void *, void *);
extern void *internal_prepare_message(rudp_msghdr_t **, size_t);
extern void internal_discard_message(void *);
extern float rttengine_update(rttengine_stat_t *, uint32_t);
extern uint32_t internal_get_timestamp(rttengine_stat_t *);
extern float internal_get_adjusted_rto(float);
extern int internal_send_recv(SOCKET, void *, int, void *, int);

#endif
