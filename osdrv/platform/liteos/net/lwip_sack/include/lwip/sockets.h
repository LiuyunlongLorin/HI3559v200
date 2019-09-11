/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 *********************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which
 * might include those applicable to Huawei LiteOS of U.S. and the country in which you
 * are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance
 * with such applicable export control laws and regulations.
 *********************************************************************************
 */

#ifndef __LWIP_SOCKETS_H__
#define __LWIP_SOCKETS_H__

#include "lwip/opt.h"

#if LWIP_SOCKET /* don't build if not configured for use in lwipopts.h */

#include <stddef.h> /* for size_t */

#include "lwip/ip_addr.h"
#include "lwip/inet.h"
#include <poll.h>
#include <netinet/tcp.h>
#include <net/if_packet.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
* @file sockets.h
*/

/**
*  @defgroup  Socket_Interfaces  Socket Interfaces
*  @ingroup User_interfaces
*   This file contains socket interfaces.
*/

#define SOCK_MAX (SOCK_PACKET + 1)
/* Mask which covers at least up to SOCK_MASK-1.  The
 * remaining bits are used as flags. */
#define SOCK_TYPE_MASK 0xf

#if LWIP_UDP && LWIP_UDPLITE
/*
 * Options for level IPPROTO_UDPLITE
 */
#define UDPLITE_SEND_CSCOV 0x01 /* sender checksum coverage */
#define UDPLITE_RECV_CSCOV 0x02 /* minimal receiver checksum coverage */
#endif /* LWIP_UDP && LWIP_UDPLITE*/

#define lwip_accept         accept
#define lwip_bind           bind
#define lwip_shutdown       shutdown
#define lwip_close          closesocket
#define lwip_connect        connect
#define lwip_getsockname    getsockname
#define lwip_getpeername    getpeername
#define lwip_setsockopt     setsockopt
#define lwip_getsockopt     getsockopt
#define lwip_listen         listen
#define lwip_recv           recv
#define lwip_recvfrom       recvfrom
#define lwip_send           send
#define lwip_sendto         sendto
#define lwip_socket         socket

/*
Func Name:  lwip_accept
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
* @endcode
*
* @par Purpose
*  This API is used to accept a connection on socket.
*
* @par Description
*  This API extracts the first connection request on the queue of pending connections
*  for the listening socket 's', creates a new connected socket, and returns a new
*  file descriptor  referring to that  socket. The newly created socket is not in the
*  the listening state. The original socket 's' is unaffected by this call.
*
* @param[in]     s        Specifies a socket that was created with lwip_socket(), has been bound to an address with lwip_bind(), and has issued a successful call to lwip_listen().
* @param[out]    addr     Either a null pointer, or a pointer to a sockaddr structure where the address of the connecting socket shall be returned.
* @param[out]    addrlen  Either a null pointer, if address is a null pointer, or a pointer to a socklen_t object which on input specifies the length of the supplied sockaddr structure, and on output specifies the length of the stored address.
*
* @par Return values
*  New socket file descriptor: On success \n
*  -1: On failure \n
*
* @par Errors
*  \li The lwip_accept() function fails if:
*     - <b> [EWOULDBLOCK]</b> : \n O_NONBLOCK is set for the socket file descriptor and no connections are present to be accepted.
*     - <b> [EWOULDBLOCK]</b> : \n O_NONBLOCK is not set but SO_RCVTIMEO is set, and no connections are present within acception timeout.
*     - <b> [EBADF] </b>: \n The socket argument is not a valid file descriptor.
*     - <b> [ECONNABORTED] </b>: \n A connection has been aborted.
*     - <b> [EINVAL] </b>: \n The socket is not accepting connections.
*     - <b> [ENFILE] </b>: \n The maximum number of sockets in the system are already open.
*     - <b> [ENOBUFS] </b>: \n No buffer space is available.
*     - <b> [ENOMEM] </b>: \n There was insufficient memory available to complete the operation.
*     - <b> [EOPNOTSUPP] </b>: \n The referenced socket is not of type SOCK_STREAM.
*  \li The lwip_accept() function may fail if:
*      - <b> [EIO] </b>: \n Internal errors
* @par POSIX Conformance:
*    @li Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are the exceptions to conformance:
*         - The new socket return by lwip_accept() does inherit socket option such SOF_REUSEADDR,SOF_KEEPALIVE, this behaviour may differ from other implementation. Portable programs should not rely on inheritance or non inheritance of file status flags or socket options, and always explicityly set all required flags or options on the socket returned from the accept().
*         - EIO are lwIP specific errno, not defined by POSIX, portable programs should not use these options in applications.
*
* @par Required Header File
* sockets.h
*
* @par Note
*  - This API does not support the PF_PACKET option.
*
* @par Related Topics
* lwip_connect
*/
int lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);



/*
Func Name:  lwip_bind
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_bind(int s, const struct sockaddr *name, socklen_t namelen);
* @endcode
*
* @par Purpose
*  This API is used to associate a local address or name with a socket.
*
* @par Description
*  This API assigns the address specified by name to the socket referred to
*  by the file descriptor 's'. namelen specifies the size, in bytes, of the address
*  structure pointed to by name.
*
* @param[in]    s        Specifies the file descriptor of the socket to be bound. [N/A]
* @param[in]    name     Points to a sockaddr structure containing the address to be bound to the socket. The length and format of the address depend on the address family of the socket.[N/A]
* @param[in]    namelen  Specifies the length of the sockaddr structure pointed to by the address argument.[N/A]
*
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Errors
*    @li The lwip_bind() function shall fail if:
*     - <b> [EADDRINUSE] </b>: \n The specified address is already in use.
*     - <b> [EADDRINUSE] </b>: For AF_INET socket, the port number was specified as zero in the socket address structure, but, upon attempting to bind to an ephemeral port, it was determined that all port numbers in the ephemeral port range are currently in use.
*     - <b> [EADDRNOTAVAIL] </b>: \n The specified address is not available from the local machine.
*     - <b> [EAFNOSUPPORT] </b>: \n The specified address is not a valid address for the address family of the specified socket.
*     - <b> [EBADF] </b> : \n The socket argument is not a valid file descriptor.
*     - <b> [EINVAL] </b> : \n The socket is already bound to an address, and the protocol does not support binding to a new address.
*     - <b> [EINVAL] </b> : \n The namelen argument is not a valid length for the address family.
*     - <b> [EINVAL] </b> : \n For AF_PACKET socket, the ifindex is out of system netif index range.
*     - <b> [EISCONN] </b>: \n The specified socket is already connected.
*     - <b> [ENOBUFS] </b>: \n Insufficient resources were available to complete the call.
*     - <b> [ENODEV] </b>: \n For AF_PACKET socket, the netif can not be determined by the ifindex.
*     - <b> [ENETDOWN] </b>: \n For AF_PACKET socket, the netif determined by the ifindex was down.
*     - <b> [EOPNOTSUPP] </b>: \n The socket type of the specified socket does not support binding to an address.
*  @par POSIX Conformance:
*     Confirming to POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition)
*
* @par Required Header File
* sockets.h
*
* @par Note
* - Bind must not be called multiple times.\n
*
* @par Related Topics
* \n
* N/A
*/
int lwip_bind(int s, const struct sockaddr *name, int namelen);



/*
Func Name:  lwip_shutdown
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_shutdown(int s, int how);
* @endcode
*
* @par Purpose
*  This is used to shut down socket send and receive operations.
*
* @par Description
*  This API is used to shut down the send and receive operations. lwip_bind() assigns the address specified by name to the socket referred to
*  by the file descriptor 's'. namelen specifies the size, in bytes, of the address
*  structure pointed to by name.
*
* @param[in]    s       Specifies a file descriptor referring to the socket.
* @param[in]    how     Specifies the type of shut-down. The values are as follows: \n SHUT_RD Disables further receive operations. SHUT_WR: Disables further send operations. SHUT_RDWR: Disables further send and receive operations.
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Errors
*    @li The lwip_shutdown() function shall fail if:
*     - <b> [EBADF] </b> : \n The socket argument is not a valid file descriptor.
*     - <b> [EINVAL] </b>: \n The how argument is invalid.
*     - <b> [ENOTCONN] </b>: \n lwIP does not support half close, ENOTCONN is returned.
*     - <b> [EINPROGRESS] </b>: \n If WRITE/CONNECTING/CLOSE is in progress, lwip may fail.
*     - <b> [EINVAL] </b>: \n If socket is not connected, lwIP shall fail with EINVAL.
*     - <b> [ENOMEM] </b>: \n There was insufficient memory available to complete the operation.
*     - <b> [EOPNOTSUPP] </b>: \n Socket is not TCP, lwIP shall fail. \n
*
* @par POSIX Conformance:
* Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are exceptions to conformance:
*  - lwIP does not support half shutdown, i.e. with either SHUT_RD or SHUT_WR alone, and if tried returns error ENOTCONN.
*  - lwIP does not return failure with errno ENOTCONN as expected by POSIX,intead set errno to EINVAL.
*
* @par Required Header File
* sockets.h
*
* @par Note
* - Only "SHUT_RDWR" is supported for the "how" parameter in this API. lwIP does not support closing one end of the full-duplex connection.\n
* - This API does not support the PF_PACKET option.
* - The listen socket does not support half shutdown.
* - lwIP does not support half shutdown. Any half shut-down is treated as full connection shut-down.
*
*
* @par Related Topics
* \n
* N/A
*/
int lwip_shutdown(int s, int how);



/*
Func Name:  lwip_getpeername
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);
* @endcode
*
* @par Purpose
*  This API is used to get name of connected peer socket.
*
* @par Description
*  This API returns the address of the peer connected to the socket 's', in the buffer pointed to by name.
*  The namelen argument should be initialized  to  indicate the amount of space pointed to by name.
*  On return it contains the actual size of the name returned (in bytes). The name is truncated if the
*  buffer provided is too small.
*
* @param[in]    s          Specifies the file descriptor referring to the connected socket.
* @param[out]    name       Indicates the pointer to the sockaddr structure that identifies the connection.
* @param[in, out]    namelen  Specifies the size name structure.
*
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Errors
*    @li The lwip_getpeername() function fails if:
*     - <b> [EBADF] </b>: The socket argument is not a valid file descriptor.
*     - <b> [ENOTCONN] </b>: The socket is not connected or otherwise has not had the peer pre-specified.
*     - <b> [EOPNOTSUPP] </b>: The operation is not supported for the socket protocol.
*     - <b> [EINVAL] </b>: The Name or Namelen are NULL, then EINVAL is returned.
* @par POSIX Conformance:
* Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are the exceptions to conformance:
* - lwIP does not check if connection-mode socket have been shutdown, and does not return failure with  errno [EINVAL].
*
*
* @par Required Header File
* sockets.h
*
* @par Note
* - This API does not support the PF_PACKET option.\n
*
*
* @par Related Topics
* lwip_getsockname()
*/
int lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);



/*
Func Name:  lwip_getsockname
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);
* @endcode
*
* @par Purpose
*  This API is used to get name of socket.
*
* @par Description
*  This API returns the current address to which the socket 's'  is bound, in the buffer pointed to by name.
*  The namelen argument should be initialized to indicate the amount of space(in bytes) pointed to by
*  name.The returned address is truncated if the buffer provided is too small; in this case, namelen will
*  return a value greater than was supplied to the call.
*
* @param[in]        s             Specifies the file descriptor referring to connected socket.
* @param[out]       name          Indicates a pointer to sockaddr structure that identifies the connection.
* @param[in,out]    namelen       Specifies the size name structure.
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Errors
* @li The lwip_getsockname() function shall fail if:
*    - <b> [EBADF] </b>: \n The socket argument is not a valid file descriptor.
*    - <b> [EOPNOTSUPP] </b>: \n The operation is not supported for this socket's protocol.For PF_RAW sockets, this error is returned.
*    - <b> [EINVAL] </b>: \n The Name or Namelen are NULL, then EINVAL is returned.
* @par POSIX conformance:
*  Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are the exceptions to conformance:
*  - lwIP does not check if connection-mode socket have been shutdown, and does not return failure with  errno [EINVAL].
*
* @par Required Header File
* sockets.h
*
* @par Note
*\n
*
* @par Related Topics
* lwip_getpeername()
*/
int lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);



/*
Func Name:  lwip_getsockopt
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
* @endcode
*
* @par Purpose
*  This API is used to get options set on socket.
*
* @par Description
*  This API retrieves the value for the option specified by the optname argument for the socket
*  specified by 's'. If the size of the optval is greater than optlen, the value stored in the object
*  pointed to by the optval argument shall be silently truncated.
*
* @param[in]        s           Specifies a socket file descriptor.  [N/A]
* @param[in]        level       Specifies the protocol level at which the option resides.  [N/A]
* @param[in]        optname     Specifies a single option to be retrieved.  [N/A]
* @param[out]       optval      Indicates an address to store option value.  [N/A]
* @param[in,out]    optlen      Specifies the size of the option value. [N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Errors
*   @li The lwip_getsockopt() function fails in the following conditions:
*     - <b> [EBADF]   </b>: \n The socket argument is not a valid file descriptor.
*     - <b> [EINVAL] </b>: \n The specified option is invalid at the specified socket level.
*     - <b> [EINVAL] </b>: \n If opt or optlen are NULL,option or option length is incorrect.
*     - <b> [ENOPROTOOPT] </b>: \n The option is not supported by the protocol.
*     - <b> [ENOMEM] </b>: \n There was insufficient memory available to complete the operation.
* @par POSIX Conformance:
*  Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition) with below exceptions:
*  - lwIP does not check if connection-mode socket have been shutdown, and does not return failure with  errno [EINVAL].
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. Supported protocol levels are: SOL_SOCKET, IPPROTO_IP, IPPROTO_TCP,IPPROTO_UDPLITE.\n
* 2. Under SOL_SOCKET the options supported are:
* SO_ACCEPTCONN
* SO_BROADCAST
* SO_ERROR
* SO_KEEPALIVE
* SO_SNDTIMEO
* SO_RCVTIMEO
* SO_RCVBUF
* SO_SNDBUF
* SO_REUSEADDR
* SO_TYPE
* SO_NO_CHECK
* SO_DONTROUTE
* SO_BINDTODEVICE
* For SO_SNDTIMEO, SO_RCVTIMEO, SO_RCVBUF, the macros LWIP_SO_SNDTIMEO, LWIP_SO_RCVTIMEO and LWIP_SO_RCVBUF should have been defined at compile time.\n
* For SO_REUSEADDR, the macro SO_REUSE should have been defined at compile time.\n
* For SO_BINDTODEVICE, the macro LWIP_SO_BINDTODEVICE should have been defined at compile time.\n
* For SO_SNDBUF, the macro LWIP_SO_SNDBUF should have been defined at compile time.\n
* 3. Under IPPROTO_IP the options supported are:
* IP_MULTICAST_TTL
* IP_MULTICAST_LOOP
* IP_MULTICAST_IF
* IP_HDRINCL
* IP_TTL
* IP_TOS.\n
* 4. Under IPPROTO_TCP the options supported are:
* TCP_NODELAY
* TCP_KEEPIDLE
* TCP_KEEPINTVL
* TCP_KEEPCNT
* TCP_INFO
* TCP_QUEUE_SEQ.
* For TCP_KEEPIDLE, TCP_KEEPINTVL, TCP_KEEPCNT, the macro LWIP_TCP_KEEPALIVE should have been defined at compile time.\n
* For TCP_INFO, Only tcpi_state, tcpi_retransmits, tcpi_probes, tcpi_backoff, tcpi_options, tcpi_rto, tcpi_snd_mss, tcpi_rcv_mss, tcpi_unacked,
* tcpi_rtt, tcpi_rttvar, tcpi_snd_ssthresh, tcpi_snd_cwnd, tcpi_reordering in
* struct tcp_info are supported, other fields are keep 0\n
* 5. Under IPPROTO_UDPLITE the options supported are:
* UDPLITE_SEND_CSCOV
* UDPLITE_RECV_CSCOV\n
* @par Related Topics
* lwip_setsockopt()
*/
int lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);



/*
Func Name:  lwip_setsockopt
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);
* @endcode
*
* @par Purpose
*  This API is used to set options on socket.
*
* @par Description
*  This API sets the option specified by the optname, at the protocol level specified by the level,
*  to the value pointed to by the optval for the socket associated with the file descriptor specified by 's'.
*
* @param[in]     s             Specifies a socket file descriptor. [N/A]
* @param[in]     level         Specifies the protocol level at which the option resides. [N/A]
* @param[in]     optname       Specifies a single option to set. [N/A]
* @param[out]    optval        Indicates the address to store the option value.  [N/A]
* @param[in]     optlen        Specifies the size of option value. [N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Errors
*   @li The lwip_setsockopt() function fails in the following conditions:
*     - <b> [EBADF] </b>: \n The socket argument is not a valid file descriptor.
*     - <b> [EDOM] </b>: \n The send and receive timeout values are too big to fit into the timeout fields in the socket structure.
*     - <b> [ENOENT] </b>: \n the option is SO_DETACH_FILTER while no previous socket filter was attached.
*     - <b> [ENOMEM] </b>: \n There was insufficient memory available for the operation to complete.
*     - <b> [ENOPROTOOPT] </b>: \n The option is not supported by the protocol.
*     - <b> [EINVAL]  </b>: \n If optval is NULL pointer, lwIP shall fail.
*     - <b> [EINVAL] </b>: \n If for PF_PACKET socket type, level is neither SOL_PACKET nor SOL_SOCKET, lwIP shall fail.
*     - <b> [EINVAL] </b>: \n If optlen does not match optval for corresponding optname, lwIP shall fail.
*     - <b> [EINVAL] </b>: \n If the send and receive timeout values are smaller than 10000 microseconds, lwIP shall fail.
* @par POSIX Conformance:
*  Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are exceptions to conformance:
*  - if connection oriented socket is shutdown, lwip_setsockopt does not check for this and does not return failure with errno EINVAL as expected by POSIX.
*  - If socket option is INVALID at specified socket level, lwip_setsockopt return failure with errno set to ENOPROTOOPT, which is not in conformance to POSIX.
*  - lwip_setsockopt does not check if connection is already connected for specified options if can not be set while socket is connected.
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. Supported protocol levels are: SOL_SOCKET, IPPROTO_IP, IPPROTO_TCP,IPPROTO_UDPLITE.\n
* 2. Under SOL_SOCKET the options supported are:
* SO_BROADCAST
* SO_KEEPALIVE
* SO_SNDTIMEO
* SO_RCVTIMEO
* SO_RCVBUF
* SO_SNDBUF
* SO_REUSEADDR
* SO_REUSEPORT
* SO_NO_CHECK.
* SO_ATTACH_FILTER
* SO_DETACH_FILTER
* SO_DONTROUTE
* SO_BINDTODEVICE
* For SO_SNDTIMEO, SO_RCVTIMEO, SO_RCVBUF, the macros LWIP_SO_SNDTIMEO, LWIP_SO_RCVTIMEO and LWIP_SO_RCVBUF should have been defined at compile time.
* For SO_REUSEADDR, the macro SO_REUSE should have been defined at compile time.\n
* For SO_BINDTODEVICE, the macro LWIP_SO_BINDTODEVICE should have been defined at compile time.\n
* For SO_ATTACH_FILTER, SO_DETACH_FILTER, the macro LWIP_SOCKET_FILTER should have been defined at compile time.\n
* Only PF_PACKET RAW socket supports SO_ATTACH_FILTER and SO_DETACH_FILTER now.
* For SO_SNDBUF, the macro LWIP_SO_SNDBUF should have been defined at compile time.\n
* Only TCP socket in listen or closed stated supports SO_SNDBUF.\n
* 3. Under IPPROTO_IP the options supported are:
* IP_ADD_MEMBERSHIP
* IP_DROP_MEMBERSHIP
* IP_MULTICAST_TTL
* IP_MULTICAST_LOOP
* IP_MULTICAST_IF
* IP_HDRINCL
* IP_TTL
* IP_TOS.\n
* 4. Under IPPROTO_TCP the options supported are:
* TCP_NODELAY
* TCP_KEEPIDLE
* TCP_KEEPINTVL
* TCP_KEEPCNT.
* For TCP_KEEPIDLE, TCP_KEEPINTVL, TCP_KEEPCNT, the macro LWIP_TCP_KEEPALIVE should have been defined at compile time.\n
* 5. Under IPPROTO_UDPLITE the options supported are:
* UDPLITE_SEND_CSCOV
* UDPLITE_RECV_CSCOV\n
* For IPPROTO_UDPLITE, the macro LWIP_UDPLITE should have been defined at compile time.\n
* @par Related Topics
* lwip_getsockopt()
*/
int lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);



/*
Func Name:  lwip_close
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_close(int s);
* @endcode
*
* @par Purpose
*  This API is used to close the socket.
*
* @par Description
*  This API closes the socket file descriptor.
*
* @param[in]    s      Specifies a socket file descriptor.  [N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
* @par Errors
*   @li The lwip_close() function shall fail if:
*     - <b> [EBADF] </b>: \n The files argument is not a open file descriptor. Invalid file descriptor.
*
* @par POSIX Conformance:
* Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are the exceptions to conformance:
* - does not support SO_LINGER socket option
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. The close() API must not be called concurrently with other BSD API on the same socket,
* because this scenario maybe causes other BSD API to return with unexpected behavior.

* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
int lwip_close(int s);



/*
Func Name:  lwip_connect
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen);
* @endcode
*
* @par Purpose
*  This is used to initiate a connection on the socket.
*
* @par Description
*  This API connects the socket referred to by the file descriptor 's' to the address specified by name.
*
* @param[in]    s          Specifies a socket file descriptor.  [N/A]
* @param[in]    name       Specifies a pointer to the sockaddr structure which identifies the connection.  [N/A]
* @param[in]    namelen    Specifies a size name structure.[N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Errors
*    @li The lwip_connect() function shall fail if:
*     - <b> [EACCES] </b>: \n The user tried to connect to a broadcast address without having the socket broadcast flag enabled.
*     - <b> [EADDRNOTAVAIL] </b>: \n For AF_IENT socket, the socket had not previously been bound to an address and, upon attempting to bind it to an ephemeral port, it was determined that all port numbers in the ephemeral port range are currently in use.
*     - <b> [EAFNOSUPPORT] </b>: \n The specified address is not a valid address for the address family of the specified socket.
*     - <b> [EALREADY] </b>: \n The socket is nonblocking and a previous connection attempt has not yet been completed.
*     - <b> [EBADF] </b>: \n The socket argument is not a valid file descriptor.
*     - <b> [ECONNREFUSED] </b>: \n The target address was not listening for connections or refused the connection request.
*     - <b> [ECONNRESET] </b>: \n Remote host reset the connection request.
*     - <b> [EINPROGRESS] </b>: \n O_NONBLOCK is set for the file descriptor for the socket and the connection cannot be immediately established; the connection shall be established asynchronously.
*     - <b> [EINVAL] </b>: \n The address_len argument is not a valid length for the address family.
*     - <b> [EISCONN] </b>: \n The specified socket is connection-mode and is already connected.
*     - <b> [ENETUNREACH] </b>: \n No route to the network is present.
*     - <b> [ENOBUFS] </b>: \n No buffer space is available.
*     - <b> [EOPNOTSUPP] </b>: \n The referenced socket is not of domain AF_INET.
*     - <b> [EOPNOTSUPP] </b>: \n The socket is listening and cannot be connected.
*     - <b> [ETIMEDOUT] </b>: \n The attempt to connect timed out before a connection was made.
* @par POSIX Conformance:
*     POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition)
*
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen);



/*
Func Name:  lwip_listen
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_listen(int s, int backlog);
* @endcode
*
* @par Purpose
*  This API is used to set a socket into listen mode.
*
* @par Description
*  This API marks the socket referred to by 's' as a passive socket, that is, as a socket that will be used
*  to accept incoming connection requests using lwip_accept().
*
* @param[in]    s          Specifies a file descriptor that refers to a socket of type SOCK_STREAM.  [N/A]
* @param[in]    backlog    Defines the maximum length to which the queue of pending connections for sockfd may grow. If a connection request arrives when the queue is full, the client may receive an error with
*      an indication of ECONNREFUSED or, if the underlying protocol supports retransmission, the request may be ignored so that a later reattempt at connection succeeds. [N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Errors
*   @li The lwip_listen() function shall fail if:
*     - <b> [EBADF] </b>: \n The socket argument is not a valid file descriptor.
*     - <b> [EDESTADDRREQ] </b>: \n The socket is not bound to a local address, and the protocol does not support listening on an unbound socket.
*     - <b> [EINVAL] </b>: \n The socket is in state which is not acceptable for listen.
*     - <b> [ENOBUFS] </b>: \n Insufficient resources are available in the system to complete the call.
*     - <b> [EOPNOTSUPP] </b>: \n The socket protocol does not support listen().
* @par POSIX Conformance:
*     POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition)
*
* @par Required Header File
* sockets.h
*
* @par Note
* - Max value of backlog is 0xFF, and min value of backlog is 0.
* - If backlog value is 0 or less than zero, backlog value of 1 is used.
* - This API does not support the PF_PACKET socket.
* - lwIP do not support auto binding in listen operation, so bind must be called before listen.
* - Listen can only be called once, and the backlog value can not be changed by later calling.
* @par Related Topics
* \n
* N/A
*/
int lwip_listen(int s, int backlog);



/*
Func Name:  lwip_recv
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_recv(int s, void *mem, size_t len, int flags);
* @endcode
*
* @par Purpose
*  This API is used to recieve a message from connected socket.
*
* @par Description
*  This API can be used to receive messages from a connection-oriented sockets only
*  because it doesnot permits the application to retrieve the source address of received data.
*
* @param[in]    s         Specifies the socket file descriptor. [N/A]
* @param[in]    mem       Points to a buffer where the message should be stored.  [N/A]
* @param[in]    len       Specifies the length in bytes of the buffer pointed to by the 'mem' argument. [N/A]
* @param[out]   flags     Specifies the type of message reception. Values of this argument are formed by logically OR'ing zero or more of the following values: MSG_PEEK
*                             Peeks at an incoming message. The data is treated as unread and the next recv() or similar function shall still return this data. [N/A]
*
* @par Return values
*  Number of bytes recieved: On success \n
*  -1: On failure \n
*
* @par Errors
*    @li The lwip_recv() function fails if:
*     - <b> [EWOULDBLOCK] </b>: \n The socket's file descriptor is marked O_NONBLOCK or MSG_DONTWAIT flag is set and no data is waiting to be received.
*     - <b> [EWOULDBLOCK] </b>: \n The socket was not marked with O_NONBLOCK, but set with option SO_RCVTIMEO, and elapsed time is more than timeout value.
*     - <b> [EBADF] </b>: \n The socket argument is not a valid file descriptor.
*     - <b> [ECONNRESET] </b>: \n A connection was forcibly closed by a peer.
*     - <b> [EINVAL]</b>: \n Invalid input parameters. If mem, len is null or flags is less than zero, lwip return failure.
*     - <b> [ENOBUFS] </b>: \n Insufficient resources were available in the system to perform the operation.
*     - <b> [ENOMEM] </b>: \n Insufficient memory was available to fulfill the request.
*     - <b> [ENOTCONN] </b>: \n A receive is attempted on a connection-mode socket that is not connected.
*     - <b> [EOPNOTSUPP] </b>: \n Some bit in the flags argument is unsupported for the socket type.
*    @li The lwip_recv() function may fail if:
*     - <b> [EIO]</b>: \n Internal errors.
* @par POSIX Conformance:
*  Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are exceptions to conformance:
*  - Return type is int instead of ssize_t.\n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. LwIP updates this receive buffer list, once it Gets the next expected segment.
* If there is any out of order segment which is next to the received segment, means
* it merges and puts that as one segemnt into receive buffer list.\n
* 2. If the apps's not read the packet form the socket and the recv buffered
* packets up to MAX_MBOX_SIZE, the incoming packet may be discard and the tcp
* connection may rst by the remote.\n
* 3. Only flag MSG_PEEK is supported, other flags, such as MSG_TRUNC/MSG_WAITALL/MSG_OOB is not supported
*\n
* @par Related Topics
* lwip_read() \n
* lwip_recvfrom()
*/
ssize_t lwip_recv(int s, void *mem, size_t len, int flags);



/*
Func Name:  lwip_read
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_read(int s, void *mem, size_t len);
* @endcode
*
* @par Purpose
*  This API is used to read bytes from a socket file descriptor.
*
* @par Description
*  This API can be used to receive messages from a connection-oriented sockets only
*  because it doesnot permits the application to retrieve the source address of received data.
*
* @param[in]     s          Specifies a socket file descriptor. [N/A]
* @param[out]    mem        Specifies the buffer to store the received data. [N/A]
* @param[in]     len        Specifies the number of bytes of data to receive. [N/A]
*
* @par Return values
*  Number of bytes recieved: On success \n
*  -1: On failure \n
*
* @par Errors
* - Same with lwip_recv except that EOPNOTSUPP doesn't occur as no flags could be provided.
*
* @par POSIX Conformance:
* - Return type is int instead of ssize_t.
*
* @par Required Header File
* sockets.h
*
* @par Note
*\n
* N/A
*
* @par Related Topics
* lwip_recv() \n
* lwip_recvfrom()
*/
int lwip_read(int s, void *mem, size_t len);



/*
Func Name:  lwip_recvfrom
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
* @endcode
*
* @par Purpose
*  This API is used to recieve a message from a connected and non-connected sockets.
*
* @par Description
*  This API can be used to receive messages from a connection-oriented and connectionless sockets
*  because it permits the application to retrieve the source address of received data.
*
* @param[in]    s          Specifies the socket file descriptor. [N/A]
* @param[out]   mem        Points to the buffer where the message should be stored.   [N/A]
* @param[in]    len        Specifies the length in bytes of the buffer pointed to by the buffer argument.[N/A]
* @param[in]    flags      Specifies the type of message reception. Values of this argument are formed by logically OR'ing zero or more of the following values: MSG_PEEK Peeks at an incoming message. The data is treated as unread and the next recvfrom() or similar function shall still return this data. [N/A]
* @param[out]   from       A null pointer, or points to a sockaddr structure in which the sending address is to be stored. The length and format of the address depend on the address family of the socket. [N/A]
* @param[out]   fromlen    Either a null pointer, if address is a null pointer, or a pointer to a socklen_t object which on input specifies the length of the supplied sockaddr structure, and on output specifies the length of the stored address. [N/A]
*
* @par Return values
*  Number of bytes recieved: On success \n
*  -1: On failure \n
*
* @par Errors
* - Same with lwip_recv.
*
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. if the apps's not read the packet form the socket and the recv buffered
* packets up to MAX_MBOX_SIZE, the incoming packet may be discard by lwIP.
* \n
* N/A
*
* @par Related Topics
* lwip_read
* lwip_recv
*/
ssize_t lwip_recvfrom(int s, void *mem, size_t len, int flags,
      struct sockaddr *from, socklen_t *fromlen);



/*
Func Name:  lwip_send
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_send(int s, const void *dataptr, size_t size, int flags);
* @endcode
*
* @par Purpose
*  This API is used to send a message to connected socket.
*
* @par Description
*  This API initiate transmission of a message from the specified socket to its peer.
*  This API will send a message only when the socket is connected.
*
* @param[in]    s          Specifies the socket file descriptor.  [N/A]
* @param[in]    dataptr    Specifies a buffer containing message to send.   [N/A]
* @param[in]    size       Specifies the length of the message to send. [N/A]
* @param[in]    flags      Indicates the types of message transmission. [N/A]
*
* @par Return values
*  Number of bytes sent: On success \n
*  -1: On failure \n
*
* @par Errors
*   @li The lwip_send() function fails if:
*     - <b> [EWOULDBLOCK] </b>: The socket's file descriptor is marked O_NONBLOCK or MSG_DONWAIT flag is set and the requested operation would block.
*     - <b> [EWOULDBLOCK] </b>: The socket was not marked with O_NONBLOCK, but set with option SO_SNDTIMEO, and elapsed time is more than timeout value.
*     - <b> [EADDINUSE] </b>: For PF_INET/SOCK_DGRAM socket without local port bond, when attempting to bind to an ephemeral port, it was determined that all port numbers in the ephemeral port range are currently in use.
*     - <b> [EBADF] </b>: The socket argument is not a valid file descriptor.
*     - <b> [ECONNRESET] </b>: A connection was forcibly closed by a peer.
*     - <b> [EDESTADDRREQ] </b>: The socket is not connection-mode and does not have its peer address set.
*     - <b> [EINPROGRESS] </b>: For PF_INET/SOCK_STREAM, there was one send operation in progress, and concurrent sending was not supported.
*     - <b> [EINVAL] </b>: Invalid argument passed, e.g. dataptr is NULL, size is zero.
*     - <b> [EMSGSIZE] </b>: The socket type requires that message be sent atomically, and the size of the message to be sent made this impossible. For PF_PACKET/SOCK_RAW socket, it means the packet is larger than the MTU of out network interface. For PF_INET/(SOCK_RAW,SOCK_DGRAM) socket, it means the packet is larger than 65000 bytes.
*     - <b> [ENETDOWN] </b>: For PF_PACKET/SOCK_RAW socket, the binding network interface was down.
*     - <b> [ENOBUFS] </b>: Insufficient resources were available in the system to perform the operation.
*     - <b> [ENOMEM] </b>: Insufficient memory was available to fulfill the request.
*     - <b> [ENODEVADDR] </b>: For PF_PACKET/SOCK_RAW socket, the binding network interface does not exist.
*     - <b> [ENOTCONN] </b>: The socket is not connected.
*     - <b> [EOPNOTSUPP] </b>: Some bit in the flags argument is unsupported for the socket type.
*   @li The lwip_send() function may fail if:
*     - <b> [EIO] </b>: Internal errors.
* @par POSIX Conformance:
*  Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are exceptions to conformance:
*   - LwIP do not support half-close, if sending data on the socket that was shut down for writing, it would failed and errno set to ENOTCONN, while it is EPIPE in POSIX.
*   - Return type is int instead of ssize_t
*   - If the socket is in connecting LwIP would return EINPROGRESS, which is ENOTCONN in POSIX.
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. UDP & RAW connection can send only a maximum data of length 65000.\n
* 2. Only flag MSG_MORE and MSG_DONTWAIT is supported, other flags, such as MSG_OOB/MSG_NOSIGNAL/MSG_EOR is not supported.\n
*\n
* @par Related Topics
* lwip_write()  \n
* lwip_sendto()
*/
ssize_t lwip_send(int s, const void *dataptr, size_t size, int flags);



/*
Func Name:  lwip_sendto
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
* @endcode
*
* @par Purpose
*  This API is used to send a message to a connected or non-connected sockets.
*
* @par Description
*  This API can be used to send messages to a connection-oriented or connectionless socket
*  If the socket is connectionless-mode, the message shall be sent to the address specified by 'to'.
*  If the socket is connection-mode, destination address in 'to' shall be ignored.
*
* @param[in]    s          Specifies a socket file descriptor. [N/A]
* @param[in]    dataptr    Specifies a buffer containing the message to send. [N/A]
* @param[in]    size       Specifies the length of the message to send. [N/A]
* @param[in]    flags      Indicates the types of message transmission. [N/A]
* @param[in]    to         Specifies a pointer to the sockaddr structure that contains the destination address. [N/A]
* @param[in]    tolen      Specifies the size of the 'to' structure.[N/A]
*
* @par Return values
*  Number of bytes sent: On success \n
*  -1: On failure \n
*
* @par Errors
*    @li The lwip_sendto() function fails if:
*     - <b> [EWOULDBLOCK] </b>: The socket's file descriptor is marked O_NONBLOCK or MSG_DONTWAIT flag is set and the requested operation would block.
*     - <b> [EWOULDBLOCK] </b>: The socket was not marked with O_NONBLOCK, but set with option SO_SNDTIMEO, and elapsed time is more than timeout value.
*     - <b> [EACCES] </b>: For PF_INET/(SOCK_RAW,SOCK_DGRAM) socket, the destination address is broadcast but SO_BROADCAST option was not enabled.
*     - <b> [EADDINUSE] </b>: For PF_INET/SOCK_DGRAM socket without local port bond, when attempting to bind to an ephemeral port, it was determined that all port numbers in the ephemeral port range are currently in use.
*     - <b> [EAFNOSUPPORT] </b>: Addresses in the specified address family cannot be used with this socket.
*     - <b> [EBADF] </b>: The socket argument is not a valid file descriptor.
*     - <b> [ECONNRESET] </b>: A connection was forcibly closed by a peer.
*     - <b> [EDESTADDRREQ] </b>: The socket is not connection-mode and does not have its peer address set, and no destination address was specified.
*     - <b> [EINPROGRESS] </b>: For PF_INET/SOCK_STREAM, there was one send operation in progress, and concurrent sending was not supported.
*     - <b> [EINVAL] </b>: Invalid argument passed, e.g. dataptr is NULL, size is zero.
*     - <b> [EMSGSIZE] </b>: The socket type requires that message be sent atomically, and the size of the message to be sent made this impossible. For PF_PACKET/SOCK_RAW socket, it means the packet is larger than the MTU of out network interface. For PF_INET/(SOCK_RAW,SOCK_DGRAM) socket, it means the packet is larger than 65000 bytes.
*     - <b> [ENETDOWN] </b>: For PF_PACKET/SOCK_RAW socket, the out network interface was down.
*     - <b> [ENETUNREACH] </b>: No route to the destination is present.
*     - <b> [ENOBUFS] </b>: Insufficient resources were available in the system to perform the operation.
*     - <b> [ENXIO] </b>: For PF_PACKET/SOCK_RAW socket, the given network interface does not exist.
*     - <b> [ENOMEM] </b>: Insufficient memory was available to fulfill the request.
*     - <b> [ENOTCONN] </b>: The socket is not connected.
*     - <b> [EOPNOTSUPP] </b>: Some bit in the flags argument is unsupported for the socket type.
*   @li The lwip_send() function may fail if:
*     - <b> [EIO] </b>: Internal error.
*
* @par POSIX Conformance:
*  Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are exceptions to conformance:
*  - For UDP/RAW socket, lwip_sendto() override peer address set in connect by address provided in sendto, and does not return failure with EISCONN.
*  - TCP ignore destination address provided in lwip_sendto(), and use connection set up by lwip_connect()
*  - return type is int instead of ssize_t
*  - LwIP do not support half-close, if sending data on the socket that was shut down for writing, it would failed and errno set to ENOTCONN, while it is EPIPE in POSIX.
*  - For PF_PACKET SOCK_RAW socket, if given network interface index does not exist, lwIP return failure with ENXIO.
*  - If the socket is in connecting LwIP would return EINPROGRESS, which is ENOTCONN in POSIX.
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. UDP & RAW connection can send only a maximum data of length 65000.
* 2. Only flag MSG_MORE and MSG_DONTWAIT is supported, other flags, such as MSG_OOB/MSG_NOSIGNAL/MSG_EOR is not supported
*
* @par Related Topics
* lwip_write() \n
* lwip_send()
*/
ssize_t lwip_sendto(int s, const void *dataptr, size_t size, int flags,
    const struct sockaddr *to, socklen_t tolen);



/*
Func Name:  lwip_socket
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_socket(int domain, int type, int protocol);
* @endcode
*
* @par Purpose
*  This API is used to allocate a socket.
*
* @par Description
*  This API is used to create an endpoint for communication and returns a file descriptor.
*
* @param[in]    domain     Specifies a protocol family. [N/A]
* @param[in]    type       Specifies the socket type.  [SOCK_RAW|SOCK_DGRAM|SOCK_STREAM] [N/A]
* @param[in]    protocol   Specifies the protocol to be used with the socket. [N/A]
*
* @par Return values
* Valid socket file descriptor: On success \n
*  -1: On failure \n
*
* @par Errors
*    @li The lwip_socket() function fails if:
*     - <b>[EAFNOSUPPORT] </b>: \n The implementation does not support the specified address family.
*     - <b>[EINVAL] </b>:  \n Invalid type or invalid flags in type.
*     - <b>[ENFILE] </b>: \n The maximum number of sockets in the system are already open.
*     - <b>[ENOBUFS] </b>: \n Insufficient resources were available in the system to perform the operation.
*     - <b>[EPROTONOSUPPORT] </b>: \n The specified protocol is not supported within this domain & type.
*     - <b>[ESOCKTNOSUPPORT] </b>: \n The socket type is not supported within this domain.
* @par POSIX Conformance:
*  Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are exceptions to conformance:
*  - For socket type not recognised by lwip_socket it returns failure with errno ESOCKTNOSUPPORT instead of EPROTOTYPE.
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. Only domain AF_INET and AF_PACKET are supported. For AF_INET, type SOCK_RAW|SOCK_DGRAM|SOCK_STREAM is supported.
* For AF_PACKET, only type SOCK_RAW is supported.
* 2. The type argument can only be used to specify the socket type, no flags is supported, such as SOCK_NONBLOCK and SOCK_CLOEXEC.\n
* @par Related Topics
* \n
* N/A
*/
int lwip_socket(int domain, int type, int protocol);



/*
Func Name:  lwip_write
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_write(int s, const void *dataptr, size_t size);
* @endcode
*
* @par Purpose
*  This API is used to write data bytes to a socket file descriptor.
*
* @par Description
*  This API is used on a connection-oriented sockets to send data.
*
* @param[in]    s          Specifies a socket file descriptor. [N/A]
* @param[in]    dataptr    Specifies a buffer containing the message to send. [N/A]
* @param[in]    size       Specifies the length of the message to send. [N/A]
*
* @par Return values
*  Number of bytes sent: On success \n
*  -1: On failure \n
*
* @par Errors
* - Same with lwip_send except that EOPNOSUPP doesn't occur as no flags could be provided.
*
* @par POSIX Conformance:
* Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are exceptions to conformance:
* - Return type is int instead of ssize_t
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* lwip_send() \n
* lwip_sendto()
*/
int lwip_write(int s, const void *dataptr, size_t size);

/*
Func Name:  lwip_select
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
* @endcode
*
* @par Purpose
*  This API is used to keeep tabs on multiple file descriptors, waiting until one or more of the file descriptors
*  become "ready" for some of I/O operations.
*
* @par Description
*  This API is  used to examine  the file descriptor sets whose addresses are passed in the readset, writeset, and exceptset
*  parameters to see whether some of their descriptors are ready for reading, are ready for writing, or have an exceptional
*  condition pending, respectively.
*
* @param[in]        maxfdp1       Specifies a range of file descriptors [N/A]
* @param[in,out]    readset       Specifies a pointer to struct fd_set, and specifies the descriptor to check for being ready to read.  [N/A]
* @param[in,out]    writeset      Specifies a pointer to struct fd_set, and specifies the descriptor to check for being ready to write.  [N/A]
* @param[in,out]    exceptset     Specifies a pointer to struct fd_set, and specifies the descriptor to check for pending error conditions.  [N/A]
* @param[in]        timeout       Specifies a pointer to struct timeval, for timeout application. [N/A]
*
* @par Return values
*  Socket file descriptor: On success \n
*  -1: On failure \n
*
* @par POSIX Conformance:
*  Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are exceptions to conformance:
*   - As per POSIX, Implementations may place limitations on the maximum timeout interval supported. All implementations shall support a maximum timeout interval of at least 31 days, but  lwIP limits value until any overflow happens,  timer with very high value might result in unspecified behavior.

* @par Errors
* The lwip_select() function shall fail if:
*     - <b>[EBADF] </b>: \n One or more of the file descriptor sets specified a file descriptor that is not a valid open file descriptor.
*     - <b>[EINVAL] </b>: \n An invalid timeout interval was specified. \n
*     - <b>[ENOMEM] </b>: \n Insufficient resources or memory, memory allocation failed.
*
* @par Required Header File
* sockets.h
*
* @par Note
*- lwip_select() does not update the timeout argument to indicate how much time was left.
*- FD_SETSIZE is compile time configurable in lwIP, and application must ensure it does not violate this boundary, lwIP does not validate this at runtime. **to be fixed in next version.
*
* @par Multithreaded Application
*      If a file descriptor being monitored by select() is closed in another
*      thread, the result is unspecified. lwIP may return without setting any fdset.
*      On some UNIX systems, select()  unblocks and returns, with an indication that the file descriptor is
*      ready (a subsequent I/O operation will likely fail with an error,
*      unless another the file descriptor reopened between the time select()
*      returned and the I/O operations was performed).  On Linux (and some
*      other systems), closing the file descriptor in another thread has no
*      effect on select().  In summary, any application that relies on a
*      particular behaviour in this scenario must be considered buggy.
*
* @par Related Topics
* \n
* N/A
*/
int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
                struct timeval *timeout);

/*
Func Name:  lwip_ioctl
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_ioctl(int s, long cmd, void *argp);
* @endcode
*
* @par Purpose
*  This API is used as a control device.
*
* @par Description
*  This API is used as a control device.
*
* @param[in]    s        Specifies an open socket file descriptor. [N/A]
* @param[in]    cmd    Specifies a device-dependent request code.  [FIONREAD|FIONBIO|N/A]
* @param[in]    argp   Specifies additional information, if required.  [N/A]
*
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
*
* @par Errors
*    @li The lwip_ioctl() function shall fail if:
*     - <b> [EBADF] </b> : The fd argument is not a valid open file descriptor.
*     - <b> [EINVAL] </b> : The request or arg argument is not valid for this device.
*     - <b> [ENOMEM] </b> : If any resource allocation fails, ENOMEM is set as errno.
*    @li The lwip_ioctl() function may fail if:
*     - <b> [EAFNOSUPPORT] </b>: If socket created with PF_PACKET, SOCK_RAW is called with SIOCADDRT.
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. Linux API supports variable argument support. But LwIP API supports only one void * as
* 3rd argument. \n
* 2. Options supported by this api are : \n
* SIOCADDRT: set default gateway only.\n
* SIOCGIFADDR: get ifnet address.\n
* SIOCGIFFLAGS: get ifnet flags.\n
* SIOCSIFFLAGS: set ifnet flags.\n
*   IFF_UP interface is up.\n
*   IFF_BROADCAST broadcast address valid.\n
*   IFF_LOOPBACK is a loopback net.\n
*   IFF_POINTOPOINT is a point-to-point link.\n
*   IFF_DRV_RUNNING resources allocated.\n
*   IFF_NOARP no address resolution protocol.\n
*   IFF_MULTICAST supports multicast.\n
*   IFF_DYNAMIC dialup device with changing addresses.\n
*   IFF_DYNAMIC_S dialup device with changing addresses.\n
* SIOCGIFADDR: get ifnet address.\n
* SIOCSIFADDR: set ifnet address.\n
* SIOCGIFNETMASK: get net addr mask.\n
* SIOCSIFNETMASK : set net addr mask.\n
* SIOCSIFHWADDR: set IF mac_address.\n
* SIOCGIFHWADDR: get IF mac_address\n
* SIOCGIFNAME: get IF name.\n
* SIOCSIFNAME: set IF name.\n
* SIOCGIFMTU: get IF mtu size.\n
* SIOCSIFMTU: set IF mtu size.\n
* SIOCGIFINDEX: get IF index.\n
* SIOCGIFCONF: get ifnet config. \n
* SIOCETHTOOL:detect eth link status. \n
* FIONBIO: set/clear non-blocking i/o.\n
* FIONREAD:get the size of buffer.\n
* 3. For SIOCSIFNAME, the new name must be consisted by letters and numbers, and the letters must be infront of numbers.
* The number range is from 0 to 255.\n
* 4. For FIONREAD option, argp should point to an application variable of type signed int. \n
* 5. For PF_PACKET sockets SIOCADDRT option is not supported. The option supported are
* FIONBIO,
* SIOCGIFADDR,
* SIOCSIFADDR,
* SIOCGIFNETMASK,
* SIOCSIFNETMASK,
* SIOCSIFHWADDR,
* SIOCGIFHWADDR,
* SIOCGIFFLAGS,
* SIOCSIFFLAGS,
* SIOCGIFNAME,
* SIOCSIFNAME,
* SIOCGIFINDEX,
* SIOCGIFCONF
* 6. For SIOCSIFADDR/SIOSIFNETMASK option, the gateway address is reset if the changed\n
* netif was the default netif and the gateway was unreachable after changing.\n
* Also duplicate network is forbidden.\n
* 7. For SIOCADDRT, only gateway setting is supported\n
* (flags in "struct rtentry" must be RTF_GATEWAY).\n
*
* @par Related Topics
* \n
* N/A
*/
int lwip_ioctl(int s, long cmd, void *argp);



/*
Func Name:  lwip_fcntl
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_fcntl(int s, int cmd, int val);
* @endcode
*
* @par Purpose
*  This API is used to manipulate file descriptor.
*
* @par Description
*  This API is used to manipulate file descriptor.
*
* @param[in]    s        Indicates the socket file descriptor. [N/A]
* @param[in]    cmd   Indicates a command to select an operation [F_GETFL|F_SETFL|N/A]
* @param[in]    val     Indicates an additional flag, to set non-blocking. [N/A]
*
*
* @par Return values
*  Postitive value: On success \n
*  -1: On failure \n
*
* @par Errors
*    @li The fcntl() function shall fail if:
*     - <b> [EBADF] </b> : \n The field argument is not a valid open file descriptor, o.
*     - <b> [EINVAL] </b> : \n The cmd argument is invalid, or cmd not supported by implementation. Only F_GETFL and F_SETFL are supported by lwIP.
* @par POSIX Conformance:
*  Implementation deviate from POSIX.1-2008 (IEEE Std 1003.1-2008, 2016 Edition), below are the exceptions to conformance:
*   - Function prototype does not support variable arguments like POSIX or linux fcntl API.
*   - The arg values to F_GETFL, and F_SETFL all represent flag values to allow for future growth. Applications using these functions should do a read-modify-write operation on them, rather than assuming that only the values defined by POSIX.1-2008 are valid. It is a common error to forget this, particularly in the case of F_SETFD. Some implementations set additional file status flags to advise the application of default behavior, even though the application did not request these flags.
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1.  Function prototype does not support variable arguments like linux fcntl API.\n
* 2. Only F_GETFL & F_SETFL commands are supported. For F_SETFL, only O_NONBLOCK is supported for val. \n
* 3. PF_PACKET sockets supports the F_SETFL and F_GETFL option.
*
* @par Related Topics
* \n
* N/A
*/
int lwip_fcntl(int s, int cmd, int val);

/* internal function,
   Call this function to intialise global socket resources
*/
int sock_init(void);

/*
Func Name:  lwip_get_conn_info
*/
/**
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_get_conn_info (int s, struct tcpip_conn * conninfo);
* @endcode
*
* @par Purpose
*  This API is used to get tcp or udp connection information.
*
* @par Description
*   This API is used to get tcp or udp connection information.
*
* @param[in]    s        socket file descriptor [N/A]
* @param[out]  conninfo  Connection information  details of given socket
*
* @par Return values
*    0: On success \n
*    Negative value: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. This function called to get tcp or udp connection information.\n
* 2. The void pointer is of type tcpip_conn.\n
* 3. This API does not support getting connection information of the TCP socket in LISTEN state.\n
*
* @par Related Topics
* \n
* N/A
*/
int lwip_get_conn_info (int s, void *conninfo);

#ifdef __cplusplus
}
#endif

#endif /* LWIP_SOCKET */

#endif /* __LWIP_SOCKETS_H__ */
