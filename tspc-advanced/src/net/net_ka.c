/*
---------------------------------------------------------------------------
 $Id: net_ka.c,v 1.54 2007/11/28 17:27:33 cnepveu Exp $
---------------------------------------------------------------------------
Copyright (c) 2001-2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

/*
 This implements a keepalive algorythm.

 Should be able to get a ping socket for all the platforms,
 generate the ICMP6 raw data and process the actual ping
 based on internal values.
*/

/*

This implements a keepalive algorythm.

Should be able to get a ping socket for all the platforms,
  generate the ICMP6 raw data and process the actual ping
  based on internal values.

  Algo:

Start from the maximum value suggested by the server.

  Need:

1. Major fuzz factor
  The major fuzz factor is applied to major increases or decreases
  of the timeout value. +/- 0% - 40%, initial +25%

  2. Minor fuzz factor
  Thee minor fuzz factor is applied after we have reached the upper
  timeout value zone.  +/- 0% - 5%, initial +3%

  3. Initial timeout
  Fixed to 5 seconds

  4. Maximal timeout
  Given by the server

  5. Upper timeout value zone
  ( (Maximal timeout - 25%) - Maximal timeout ) (ie, 75% - 100% of 30 seconds).
  This   is meant as a comfort zone in which we can apply small changes to
  the keep alive value - changes in the order of the minor fuzz factor.
  (from Teredo, section 6.7)


     a) Apply +(Major fuzz factor) to the timeout value until we reach the upper
     timeout value zone, then throttle to +/-(Minor fuzz factor) with an adjustment to
     stay in the upper timeout value zone.

     b) When we get no replies, apply -(Major fuzz factor) to the timeout value until
     initial timeout is reached. Forfeit if we get too many timeouts. If reply, go to a)

     So.

     Each time NetKeepaliveDo() is called, we check next_event and see if we have
     to let out a keepalive packet. If so we do, recompute major_fuzz, minor_fuzz,
     apply either +(major_fuzz) or +/-(minor_fuzz) to next_event and exit.

     Apply +(major_fuzz) if outside the comfort zone, +/-(minor_fuzz) otherwise.

     If we have an event ready and we havent got a read from the socket,
     we send the keepalive and recompute major_fuzz, minor_fuzz, apply -(major_fuzz)
     to next_event and exit.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _USES_SYS_TIME_H_
#define _USES_SYS_SOCKET_H_
#define _USES_NETINET_IN_H_
#define _USES_NETINET_IP6_H_
#define _USES_NETINET_ICMP6_H_
#define _USES_NETINET_ICMP_H_
#define _USES_ARPA_INET_H_

#include "platform.h"

#include "net_ka.h"
#include "net_cksm.h"
#include "log.h"	// log levels + display
#include "hex_strings.h"
#include "tsp_lease.h"  /* allocation lease stuff */
#include "errors.h"


struct NetKeepAlive {

  int init;   /* is it initialized? */
  char host[512];   /* host to send ping to */

  SOCKET keepalive_fd;
  int family;
  struct sockaddr_in6 sa6_local; /* our ipv6 */ /* needed for cksum */
  struct sockaddr_in6 sa6;  /* ping6 destination */
  struct sockaddr_in sa_local;   /* ipv4 keepalive */
  struct sockaddr_in sa;         /* ping4 destination */
  int still_up;     // link is still up?
  int doit;       // we had no write out, ping out to keepalive
  int count;        // read counter in between cycles
  int got_read;     // read flag
  int got_write;      // write flag
  int pinged;       // pinged flag
  int consecutive_timeout;  /* number of consecutive timeouts */

  int maximal_keepalive;  /* the maximal wait factor */
  double current_keepalive;

  float minor_fuzz;   /* the minor fuzz factor */
  float major_fuzz;   /* the major fuzz factor */

#define KA_INITIAL_KEEPALIVE 5  // initial timeout value
#define KA_MAXIMAL_CYCLES  5  // maximal number of consecutive keepalive
                // missed before declaring a timeout
  struct timeval next_event;    /* take action at OR after this time */
};
static struct NetKeepAlive ka;

static void internal_adjust_major_fuzz(float *);
static void internal_adjust_minor_fuzz(float *);
static void internal_adjust_next_event(struct NetKeepAlive *, int got_reply, int pinged);
static float internal_trim(float, float, float);

static int internal_do_pingout(struct NetKeepAlive *);
static int internal_do_pingin(struct NetKeepAlive *);

static int seqn;

#ifdef WIN32
// IPHLPAPI ping for windows
static HINSTANCE hndliphlp = NULL;
static HANDLE hHandle = NULL;
static HANDLE hEvent = NULL;
static char ReplyBuffer[1500];

HANDLE (WINAPI *IcmpCreateFile)(VOID);
BOOL   (WINAPI *IcmpCloseHandle)(HANDLE);
DWORD  (WINAPI *IcmpSendEcho2)(HANDLE, HANDLE, FARPROC, PVOID, IPAddr, LPVOID, WORD, PIP_OPTION_INFORMATION, LPVOID, DWORD, DWORD);
DWORD  (WINAPI *IcmpParseReplies)(LPVOID, DWORD);
HANDLE (WINAPI *Icmp6CreateFile)(VOID);
DWORD  (WINAPI *Icmp6SendEcho2)(HANDLE, HANDLE, FARPROC, PVOID, struct sockaddr_in6*, struct sockaddr_in6 *, LPVOID, WORD, PIP_OPTION_INFORMATION, LPVOID, DWORD, DWORD);
DWORD  (WINAPI *Icmp6ParseReplies)(LPVOID, DWORD);
#endif


int
NetKeepaliveInit(char *src, char *dst, int maximal_keepalive, int family) {
	struct timeval tv;
#ifdef WIN32
	hndliphlp = LoadLibrary("IPHLPAPI.DLL");
	
	if (!hndliphlp) {
		Display(LOG_LEVEL_3, ELError, "NetKeepaliveInit",  HEX_STR_CANT_CREATE_ICMP_HNDL_FILE);
		return 1;
	}
	
	if (family == AF_INET6)  {
		// Try to load IPv6
		// support functions
		// from iphlpapi
		//
		Icmp6CreateFile = (HANDLE (WINAPI *)(void)) GetProcAddress(hndliphlp, "Icmp6CreateFile");
    IcmpCloseHandle = (BOOL (WINAPI *)(HANDLE)) GetProcAddress(hndliphlp, "IcmpCloseHandle");
		Icmp6SendEcho2 = (DWORD (WINAPI *)(HANDLE, HANDLE, FARPROC, PVOID, struct sockaddr_in6*, struct sockaddr_in6*, LPVOID, WORD, PIP_OPTION_INFORMATION,
			LPVOID, DWORD, DWORD)) GetProcAddress(hndliphlp, "Icmp6SendEcho2");
		Icmp6ParseReplies = (DWORD (WINAPI *)(LPVOID, DWORD)) GetProcAddress(hndliphlp, "Icmp6ParseReplies"); 

		if( (Icmp6CreateFile == NULL) ||
        (IcmpCloseHandle == NULL) ||
        (Icmp6SendEcho2  == NULL) ||
        (Icmp6ParseReplies == NULL) )
    {
      Display(LOG_LEVEL_3, ELError, "NetKeepaliveInit",  HEX_STR_CANT_CREATE_ICMP_HNDL_FILE);
      return 1;
    }

		hHandle = Icmp6CreateFile();
	}
		
#ifdef V4V6_SUPPORT	
	else if (family == AF_INET) {
		// Try to load IPv4
		// support functions
		// from iphlpapi
		//
		IcmpCreateFile = (HANDLE (WINAPI *)(VOID)) GetProcAddress(hndliphlp, "IcmpCreateFile");
    IcmpCloseHandle = (BOOL (WINAPI *)(HANDLE)) GetProcAddress(hndliphlp, "IcmpCloseHandle");
		IcmpSendEcho2 = (DWORD (WINAPI *)(HANDLE, HANDLE, FARPROC, PVOID, IPAddr, LPVOID, WORD, PIP_OPTION_INFORMATION,
			LPVOID, DWORD, DWORD)) GetProcAddress(hndliphlp, "IcmpSendEcho2");
		IcmpParseReplies = (DWORD (WINAPI *)(LPVOID, DWORD)) GetProcAddress(hndliphlp, "IcmpParseReplies"); 
		
		if ( (IcmpCreateFile == NULL) || 
         (IcmpCloseHandle == NULL) ||
         (IcmpSendEcho2 == NULL) ||
         (IcmpParseReplies == NULL) ) 
    {
      Display(LOG_LEVEL_3, ELError, "NetKeepaliveInit",  HEX_STR_CANT_CREATE_ICMP_HNDL_FILE);
      return 1;
		}
		
		hHandle = IcmpCreateFile();
	}
#endif
	if (hHandle == INVALID_HANDLE_VALUE) {
		Display(LOG_LEVEL_3, ELError, "NetKeepaliveInit", HEX_STR_CANT_CREATE_ICMP_HNDL_FILE);
		return 1;
	}
#endif

	/* Load the structure with passed in AND
	 * initial values
	 */

	seqn = 0;
	
	if (strncpy(ka.host, dst, sizeof(ka.host)) == NULL)
		return 1;

	ka.maximal_keepalive = maximal_keepalive;
	ka.current_keepalive = KA_INITIAL_KEEPALIVE;
	ka.major_fuzz = (float) 0.60;
	ka.minor_fuzz = (float) 0.075;

	/* And initialize the next event */
	/* Initial timeout is set to five seconds */

	GETTIMEOFDAY(&tv, NULL);
	ka.next_event.tv_sec = tv.tv_sec + KA_INITIAL_KEEPALIVE;
	ka.next_event.tv_usec = tv.tv_usec;

	/* initialize the random source */

	SRANDOM(tv.tv_usec);

	/* set the family */
	ka.family = family;
	
	/* get a ping socket and a monitor socket */
	if (family == AF_INET6) {
		INET_PTON(AF_INET6, dst, &ka.sa6.sin6_addr);
		ka.sa6.sin6_family = AF_INET6;

		INET_PTON(AF_INET6, src, &ka.sa6_local.sin6_addr);
		ka.sa6_local.sin6_family = AF_INET6;
#ifndef WIN32
		ka.keepalive_fd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
#endif
	}
#ifdef V4V6_SUPPORT	
	else if (family == AF_INET) {
		INET_PTON(AF_INET, dst, &ka.sa.sin_addr);
		ka.sa.sin_family = AF_INET;

		INET_PTON(AF_INET, src, &ka.sa_local.sin_addr);
		ka.sa_local.sin_family = AF_INET;
#ifndef WIN32
		ka.keepalive_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
#endif
	}
#endif /* V4V6_SUPPORT */	

	ka.consecutive_timeout = 0;
	ka.still_up = 0;
	ka.doit = 1;
	ka.count = 0;
	ka.got_read = 0;
	ka.got_write = 0;
	ka.pinged = 1;

	/* ok, we are inialized */

	ka.init = 1;
	Display(LOG_LEVEL_3, ELInfo, "NetKeepaliveInit", HEX_STR_KEEPALIVE_INITIALIZED, dst, maximal_keepalive);
	return 0;
}

void NetKeepaliveDestroy( void )
{
	shutdown(ka.keepalive_fd, SHUT_RDWR);

#ifdef WIN32
	if( hHandle != NULL )
  {
		IcmpCloseHandle(hHandle);
    hHandle = NULL;
  }

	if( hEvent != NULL )
  {
		CloseHandle(hEvent);
    hEvent = NULL;
  }

  // Free IP Helper API DLL handle, and invalidate function pointers.
  if( hndliphlp != NULL )
  {
    IcmpCreateFile      = NULL;
    IcmpCloseHandle     = NULL;
    IcmpSendEcho2       = NULL;
    IcmpParseReplies    = NULL;
    Icmp6CreateFile     = NULL;
    Icmp6SendEcho2      = NULL;
    Icmp6ParseReplies   = NULL;

    FreeLibrary( hndliphlp );
  }
#endif

	CLOSESOCKET(ka.keepalive_fd);
	memset(&ka, 0, sizeof(struct NetKeepAlive));
}

void
NetKeepaliveGotRead() {
	if (ka.init == 1) {
		ka.got_read = 1;
		//Display(LOG_LEVEL_3, ELNotice, "NetKeepaliveGotRead", HEX_STR_INCOMING_DATA_FROM_TUNNEL);
	}
}

void
NetKeepaliveGotWrite() {
	if (ka.init == 1) {
		ka.got_write = 1;
		//Display(LOG_LEVEL_3, ELNotice, "NetKeepaliveGotWrite", HEX_STR_OUT_DATA_TO_TUNNEL);
	}
}


/* Should be called with a good resolution 
   to allow for good precision. 

   Every 50 to 100 ms.

   return values :
	0 - everything is OK
	1 - warning - timeout
	2 - fatal - too many timeouts
	3 - not initialised
	*/

int
NetKeepaliveDo() {

	struct timeval tv;
	int doit = 0;		// local evaluation of if we should try to ping.
						// will be flagged only if we hit the next event time
	
	if (ka.init != 1)
		return 3;
	
	if (internal_do_pingin(&ka) != 0)	// read echo replies
		ka.got_read = 1;

	if (ka.got_read == 1) {			// if we did get a read in the last cycle,
		ka.consecutive_timeout = 0;	// we can reset the consecutive timeouts counter.
		ka.count++;					// increment the packet counter
		ka.got_read = 0;			// and reset the read flag until next time
	}

	if (ka.got_write == 1) {	// we did get a write in the last cycle?
		ka.doit = 0;			// then no need to ping out
		ka.got_write = 0;		// and also reset the write flag until next time
	}


	/* do we need to ping out ? */

	GETTIMEOFDAY(&tv, NULL);
	
	/* If a ping is needed,
	   then also take a look
	   at the number of consecutive
	   missreads we have.
	   */

	if (tv.tv_sec == ka.next_event.tv_sec) {
		if (tv.tv_sec >= ka.next_event.tv_usec)
			doit = 1; 
	}

	else if (tv.tv_sec > ka.next_event.tv_sec)
		doit = 1;
	
	
	if (doit) {
	
		if (ka.doit == 0 && ka.count > 0)
			ka.still_up = 1;	/* so if we had both read and writes, we are up? */

		if (ka.consecutive_timeout != 0)	/* if we had timeouts, ping */
			ka.still_up = 0;

		if (ka.count == 2 || ka.count == 1)	/* a count of 2 or 1 probably means */
											/* only the keepalive packet was sent */
			ka.still_up = 0;

		if (ka.doit == 1)					/* no write, do it */
			ka.still_up = 0;

		if ( ka.still_up == 0 )  { 
								// if we havent got any
								// traffic for the last cycle,
								// generate a ping (and thus
								// a reply, and thus - traffic)
								// or none if we are down.
								// a count of 2 will mean only our ping got thru since
								// last time and it means we need to continue
								// pinging
			
			if (internal_do_pingout(&ka) > 0) {
				ka.got_write = 1;	// if it worked, flag the write flag
			}
			ka.consecutive_timeout++;		// we had a timeout of sorts
											// since we had to ping
		}

		ka.doit = 1;		// re-ping unless noted otherwise
		ka.count = 0;		// reset packet counter in between pings
		
		/* adjust internal values */
		/* set the next event time */

		internal_adjust_next_event(&ka, ka.still_up, ka.consecutive_timeout);

		/* adjust the major fuzz factor.
	   it needs to vary +/- 10 - 40% and stay
	   in the .40 - .80 range
	   */
		internal_adjust_major_fuzz(&ka.major_fuzz);

		/* then adjust the minor fuzz factor.
		this one needs to vary +/- 1 - 5%
		staying in the .03 - .07 range
		*/
		internal_adjust_minor_fuzz(&ka.minor_fuzz);

		if (ka.consecutive_timeout == KA_MAXIMAL_CYCLES)
			return 2;

		ka.still_up = 0; // until next time, we are down unless noted otherwise
	}


	/* and return */
	return 0;
}


static 
void
internal_adjust_major_fuzz(float *major_fuzz) {
	int i;
	float f;

	/* here we want a randomness of .40 - .80 */

	i = RANDOM()%40;
    f = (float) i / 100;		/* 0 - 100% */
	f += .40f;
	
	*major_fuzz = internal_trim(f, 0.40f, 0.80f);
}

static
void 
internal_adjust_minor_fuzz(float *minor_fuzz) {
	int i;
	float f;

	/* here we want a randomness of .03 - .07 */

	i = RANDOM()%4;
    f = (float) i / 100;
	f += .03f;

	*minor_fuzz = internal_trim (f, 0.03f, 0.07f);
}

static
void
internal_adjust_next_event(struct NetKeepAlive *nka, int got_reply, int ct) {
	/* if we are in the comfort zone, 
	   apply minor_fuzz.
	   Otherwise, apply major_fuzz

	   If we are in the comfort zone and got no
	   reply, force us out of it.
	*/

	int i;
	struct timeval tv;
	float minor_minimum, major_minimum;

	i = RANDOM()&0x01;      /* positive or negative shift? */

	minor_minimum = KA_INITIAL_KEEPALIVE;
	major_minimum = (float) nka->maximal_keepalive * 0.75f;

	/* next_event = current_event * +/- factor */

	if (nka->current_keepalive >= major_minimum) {
			// we are in the comfort zone
			// did we get any reply?
			// if not, bump down outside the zone
			// also ensure that if we
		    // are stuck at either ends of the
		    // spectrum, we get out fast
#ifdef FAST_HANDOVER
		if (got_reply == 0 && ct > 1) 
				nka->current_keepalive = major_minimum - 1;
		else
#endif
		{
			float fuzz_factor;
			if (nka->current_keepalive == nka->maximal_keepalive)
				fuzz_factor = 1 - nka->minor_fuzz;
			else if (nka->current_keepalive == major_minimum)
				    fuzz_factor = 1 + nka->minor_fuzz; 
			else fuzz_factor = i ? (1 - nka->minor_fuzz) : (1 + nka->minor_fuzz);
			
			nka->current_keepalive = internal_trim((float)nka->current_keepalive * fuzz_factor,
				major_minimum, (float) nka->maximal_keepalive);
		}
	}

	/* else we are NOT in the comfort zone */
	if (nka->current_keepalive < major_minimum) {
		/* if we got_reply we go up, otherwise
		   we go down */
#ifdef FAST_HANDOVER
		float fuzz_factor;
		fuzz_factor = (ct < 2) ? (1 + nka->major_fuzz) : ( 1 - nka->major_fuzz);
#else
		float fuzz_factor = 1;
		if (ct < 2)
			fuzz_factor = 1 + nka->major_fuzz;
#endif
		nka->current_keepalive = internal_trim((float)nka->current_keepalive * fuzz_factor,
		minor_minimum, (float) nka->maximal_keepalive);
	}

	/* then put the next event in the timeval */

	GETTIMEOFDAY(&tv, NULL);

	nka->next_event.tv_sec  = tv.tv_sec + (int) nka->current_keepalive;
	nka->next_event.tv_usec = (long)( nka->current_keepalive - (int) nka->current_keepalive ) * 1000000;

	Display(LOG_LEVEL_3, ELInfo, "internal_adjust_next_event", HEX_STR_NEXT_KA_IN, nka->current_keepalive);

	return;
}

static
float
internal_trim(float value, float lower, float higher) {
	if ( value <= lower)
		return lower;
	if (value >= higher)
		return higher;
	return value;
}

/* if this returns 0, no data was written. Otherwise, data was written
*/

static int internal_do_pingout(struct NetKeepAlive *nka) 
{
	int ret = 0;
#ifdef WIN32
	IP_OPTION_INFORMATION ip_option_information;
  int icmp_send_ret = 0;
#else
	int len = 0;
	char sendbuf[1500];
#ifdef V4V6_SUPPORT	
	struct icmp *icmp;
#endif /* V4V6_SUPPORT */	
	struct icmp6_hdr *icmp6;
#endif
	
#ifndef WIN32
	// compute packet 
	// and send it
	//
#ifdef V4V6_SUPPORT 	
	if (nka->family == AF_INET) {
		/* icmp4 packet */
		icmp = (struct icmp *)sendbuf;
		icmp->icmp_type = ICMP_ECHO;
		icmp->icmp_code = 0;
		icmp->icmp_id = getpid();
		icmp->icmp_seq = seqn++;
		gettimeofday((struct timeval *) icmp->icmp_data, NULL);
		len = 8 /* icmp header */ + 56 /* icmp data */;
		icmp->icmp_cksum = 0;
		icmp->icmp_cksum = in_cksum((u_short *) icmp, len);
		ret = sendto(nka->keepalive_fd, sendbuf, len, 0, (struct sockaddr *) &(nka->sa),
			     sizeof(nka->sa));
	}
#endif /* V4V6_SUPPORT */	
	if (nka->family == AF_INET6) {
		icmp6 = (struct icmp6_hdr *) sendbuf;
		icmp6->icmp6_type = ICMP6_ECHO_REQUEST;
		icmp6->icmp6_code = 0;
		icmp6->icmp6_id = getpid();
		icmp6->icmp6_seq = seqn++;
		gettimeofday((struct timeval *) (icmp6+1), NULL);
		len = 8 /* icmp header */ + 56 /* icmp data */;
		ret = sendto(nka->keepalive_fd, sendbuf, len, 0, (struct sockaddr *) &(nka->sa6),
			     sizeof(nka->sa6));
	}
#else
	// WIN32 handling
	//
  memset( &ip_option_information, 0x00, sizeof(IP_OPTION_INFORMATION) );
	ip_option_information.Ttl = 64;
	
	if (hEvent != NULL) {
		CloseHandle(hEvent);
		hEvent = NULL;
	}
	
	hEvent = CreateEvent(NULL, FALSE, FALSE, "Gw6cIcmpEvent");
	
	//
	// Send a ping out then callback in
	// internal_do_pingin when the reply
	// comes in
	//
	if (nka->family == AF_INET6) 
  {
		icmp_send_ret = Icmp6SendEcho2(hHandle, hEvent, NULL, NULL, &nka->sa6_local, &nka->sa6, NULL, 0,
                                   &ip_option_information, ReplyBuffer, sizeof(ReplyBuffer), 1000);

    if( (icmp_send_ret != 0)  ||  ((icmp_send_ret == 0) && (GetLastError() == ERROR_IO_PENDING)) )
    {
      // Success.
      ret = 1;
    }
	}
#ifdef V4V6_SUPPORT
	else if (nka->family == AF_INET) 
  {
		icmp_send_ret = IcmpSendEcho2(hHandle, hEvent, NULL, NULL, (IPAddr) nka->sa.sin_addr.S_un.S_addr, NULL, 0,
                                  &ip_option_information, ReplyBuffer, sizeof(ReplyBuffer), 1000);

    if( (icmp_send_ret != 0)  ||  ((icmp_send_ret == 0) && (GetLastError() == ERROR_IO_PENDING)) )
    {
      // Success.
      ret = 1;
    }
	}
#endif
#endif
	
	return ret;
}
 
/* if this next one returns zero, no ping reply.
   if it returns > 0, ping reply.
   */
static
int
internal_do_pingin(struct NetKeepAlive *nka) {
	int ret = 0;
#ifndef WIN32
	unsigned char buffer[2048];
	fd_set fs;
	struct timeval tv_sel;
	
	memset(buffer, 0, sizeof(buffer));

	FD_ZERO(&fs);
	FD_SET(nka->keepalive_fd, &fs);
	memset(&tv_sel, 0, sizeof(tv_sel));	// set to zero - imitate polling

	ret = select(nka->keepalive_fd + 1, &fs, NULL, NULL, &tv_sel);	
	if (ret > 0)
		ret = recv(nka->keepalive_fd, buffer, sizeof(buffer), 0);
#else
	// Check if windows signaled our event
	// meaning the ping reply came in
	//
	if ((hEvent != NULL) && (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)) 
  {
		if (nka->family == AF_INET6)
    {
			ret = Icmp6ParseReplies(ReplyBuffer, sizeof(ReplyBuffer));
    }
#ifdef V4V6_SUPPORT
		else if (nka->family == AF_INET)
    {
			ret = IcmpParseReplies(ReplyBuffer, sizeof(ReplyBuffer));
    }
#endif

		CloseHandle(hEvent);
		hEvent = NULL;
	}
#endif

	//if (ret > 0) 
	//	Display(LOG_LEVEL_3, ELNotice, "internal_do_pingin", HEX_STR_ICMP_ECHO_REPLY, ret);
	return ret;
}

