/*---------------------------------------------------------------
 * Copyright (c) 1999,2000,2001,2002,2003
 * The Board of Trustees of the University of Illinois
 * All Rights Reserved.
 *---------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software (Iperf) and associated
 * documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 *
 * Redistributions of source code must retain the above
 * copyright notice, this list of conditions and
 * the following disclaimers.
 *
 *
 * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimers in the documentation and/or other materials
 * provided with the distribution.
 *
 *
 * Neither the names of the University of Illinois, NCSA,
 * nor the names of its contributors may be used to endorse
 * or promote products derived from this Software without
 * specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTIBUTORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ________________________________________________________________
 * National Laboratory for Applied Network Research
 * National Center for Supercomputing Applications
 * University of Illinois at Urbana-Champaign
 * http://www.ncsa.uiuc.edu
 * ________________________________________________________________
 *
 * headers.h
 * by Mark Gates "mgates@nlanr.net"
 * -------------------------------------------------------------------
 * All system headers required by iperf.
 * This could be processed to form a single precompiled header,
 * to avoid overhead of compiling it multiple times.
 * This also verifies a few things are defined, for portability.
 * ------------------------------------------------------------------- */

#ifndef HEADERS_H
#define HEADERS_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#define  HAVE_CONFIG_H
#ifdef HAVE_CONFIG_H
    #include "config.pthread.h"
    #include "compiler.h"
/* OSF1 (at least the system I use) needs extern C
 * around the "netdb.h" and "arpa/inet.h" files. */
    #if defined( SPECIAL_OSF1_EXTERN ) && defined( __cplusplus )
        #define SPECIAL_OSF1_EXTERN_C_START extern "C" {
        #define SPECIAL_OSF1_EXTERN_C_STOP  }
    #else
        #define SPECIAL_OSF1_EXTERN_C_START
        #define SPECIAL_OSF1_EXTERN_C_STOP
    #endif
#endif /* HAVE_CONFIG_H */

/* turn off assert debugging */
#define NDEBUG

/* standard C headers */
#include "os_support.h"
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"
#include "ctype.h"
#include "errno.h"
#include "string.h"
#include "time.h"
#include "math.h"

#undef HAVE_THREAD
#undef HAVE_MULTICAST

#undef HAVE_INET_NTOP
#undef HAVE_INET_PTON

/* unix headers */
    #include "sys/types.h"
    //#include "sys/select.h"
    #include "lwip/sockets.h"
    #include "sys/time.h"
    //#include "netinet/in.h"
    //#include "arpa/inet.h"   /* netinet/in.h must be before this on SunOS */
    //#include "net/netdb.h"
    #include "unistd.h"

    #include "time.h"
    //#include "sys/time.h"

//#include "cyg/kernel/kapi.h"
    #define SOCKET_ERROR   -1
    #define INVALID_SOCKET -1
    #define Socklen_t socklen_t

#define delay_loop(usec)    oss_udelay(usec)
#define usleep(usec)        oss_usleep(usec)

#ifndef INET6_ADDRSTRLEN
    #define INET6_ADDRSTRLEN 40
#endif
#ifndef INET_ADDRSTRLEN
    #define INET_ADDRSTRLEN 15
#endif

//#ifdef __cplusplus
    #ifdef HAVE_IPV6
        #define REPORT_ADDRLEN (INET6_ADDRSTRLEN + 1)
typedef struct sockaddr_storage iperf_sockaddr;
    #else
        #define REPORT_ADDRLEN (INET_ADDRSTRLEN + 1)
typedef struct sockaddr_in iperf_sockaddr;
    #endif
//#endif

typedef uint64_t max_size_t;

/* in case the OS doesn't have these, we provide our own implementations */
#include "sys/time.h"
#include "gettimeofday.h"
#include "inet_aton.h"
#include "snprintf.h"

#ifndef SHUT_RD
    #define SHUT_RD   0
    #define SHUT_WR   1
    #define SHUT_RDWR 2
#endif // SHUT_RD

extern int select(int nfds, FAR fd_set *readfds, FAR fd_set *writefds,
                  FAR fd_set *exceptfds, FAR struct timeval *timeout);
extern int close(int d);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* HEADERS_H */

