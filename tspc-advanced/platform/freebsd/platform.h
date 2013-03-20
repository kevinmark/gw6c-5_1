/*
---------------------------------------------------------------------------
 $Id: platform.h,v 1.25 2007/04/25 19:31:31 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

/* FreeBSD */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#ifdef _USES_PTHREAD_H_
#include <pthread.h>

#define MUTEX	pthread_mutex_t
#define MUTEX_INIT(A) \
	pthread_mutex_init(A, NULL)
#define MUTEX_LOCK(A) \
	pthread_mutex_lock(A)
#define MUTEX_UNLOCK(A) \
	pthread_mutex_unlock(A)
#endif

#ifdef _USES_SYS_TIME_H_
#include <sys/time.h>
#define GETTIMEOFDAY(A, B) \
	gettimeofday(A, B)
#endif

#ifdef _USES_NETDB_H_
#include <netdb.h>
#endif

#ifdef _USES_SYS_IOCTL_H_
#include <sys/ioctl.h>
#endif

#ifdef _USES_SYS_SOCKET_H_
#include <sys/types.h>
#include <sys/socket.h>
#define SOCKET int
#define CLOSESOCKET close
#endif

#ifdef _USES_NETINET_IN_H_
#include <netinet/in.h>
#endif

#ifdef _USES_NETINET_IP6_H_
#include <netinet/ip6.h>
#endif

#ifdef _USES_NETINET_ICMP6_H_
#include <netinet/icmp6.h>
#endif

#ifdef _USES_NETINET_ICMP_H_
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#endif


#ifdef _USES_NET_IF_H_
#include <net/if.h>
#endif

#ifdef _USES_ARPA_INET_H_
#include <arpa/inet.h>
#define INET_PTON(A, B, C)\
	inet_pton(A, B, C)
#endif

#ifdef _USES_SYSLOG_H_
#include <syslog.h>
#define SYSLOG(A,B)\
	syslog(A, B)
#define OPENLOG(A,B,C)\
	openlog(A, B, C)
#define CLOSELOG\
	closelog
#endif


#define SLEEP(A)\
	sleep(A)

#define RANDOM \
	rand
#define SRANDOM \
	srand

#define SCRIPT_TMP_FILE "/tmp/gw6c-tmp.log"

#define DEFAULT_LOG_LEVEL_STDERR LOG_LEVEL_1
#define DEFAULT_LOG_LEVEL_SYSLOG LOG_LEVEL_DISABLED
#define DEFAULT_LOG_LEVEL_CONSOLE LOG_LEVEL_DISABLED
#define DEFAULT_LOG_LEVEL_FILE LOG_LEVEL_DISABLED

#define DEFAULT_PRECFG_LOG_LEVEL_STDERR LOG_LEVEL_MAX
#define DEFAULT_PRECFG_LOG_LEVEL_SYSLOG LOG_LEVEL_MAX
#define DEFAULT_PRECFG_LOG_LEVEL_CONSOLE LOG_LEVEL_DISABLED
#define DEFAULT_PRECFG_LOG_LEVEL_FILE LOG_LEVEL_MAX

#define V4V6_SUPPORT

#endif










