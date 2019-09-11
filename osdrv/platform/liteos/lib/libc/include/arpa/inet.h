/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _ARPA_INET_H_
#define _ARPA_INET_H_

#include <stdint.h>
#include <sys/types.h>
#include <netinet/in.h>

__BEGIN_DECLS


/**
* @defgroup INET_Interfaces INET Interfaces
* This section contains the INET interfaces.
* @ingroup User_interfaces
*/

/*
Func Name:  inet_addr
*/

/**
* @ingroup INET_Interfaces
* @par Prototype
* @code
* in_addr_t inet_addr(const char *cp);
* @endcode
*
* @par Purpose
* This function is used to convert the IPv4 address from string notation (number and dot format)
* to binary notation of network byte order.
*
* @par Description
* This function is used to convert the IPv4 address from string notation (number and dot format)
* to binary notation of network byte order.
*
* @param[in]    cp  Pointer to IPv4 address string [N/A]
*
* @par Return values
*  Valid IP_add: On success, in unsigned int (32bit) format \n
*  IPADDR_NONE : On failure \n
*
* @par Required Header File
* inet.h
*
* @par Note
* [N/A]
*
* @par Related Topics
* N/A
*/
in_addr_t inet_addr(const char *cp);

/*
Func Name:  inet_aton
*/

/**
* @ingroup INET_Interfaces
* @par Prototype
* @code
* int inet_aton(const char *cp, struct in_addr *inp);
* @endcode
*
* @par Purpose
* This function is used to convert the IPv4 address from string notation (number and dot format)
* to binary notation of network byte order and stores it in the structure that addr points to.
*
* @par Description
* This function is used to convert the IPv4 address from string notation (number and dot format)
* to binary notation of network byte order and stores it in the structure that addr points to.
*
* @param[in]    cp     Pointer to IPv4 address string [N/A]
* @param[out]  inp  Generated binary notation of IPv4 address will be udpated here [N/A]
*
* @par Return values
*  1 : On success\n
*  0 : On failure \n
*
* @par Required Header File
* inet.h
*
* @par Note
* 1. This interface is same as inet_addr(). Only difference is that the generated binary notation
* of IPv4 address is updated in the input parameter "inp", instead of returning it.[N/A]
*
* @par Related Topics

* N/A
*/
int inet_aton(const char *cp, struct in_addr *inp);

/*
Func Name:  inet_ntoa
*/

/**
* @ingroup INET_Interfaces
* @par Prototype
* @code
* char *inet_ntoa(struct in_addr in);
* @endcode
*
* @par Purpose
* This function is used to convert the IPv4 address from binary notation (network byte order)
* to string notation (number and dot format).
*
* @par Description
* This function is used to convert the IPv4 address from binary notation (network byte order)
* to string notation (number and dot format).
*
* @param[in]    in     Pointer to binary notation of IPv4 address [N/A]
*
* @par Return values
*  Valid pointer : On success, returns pointer to string notation of IPv4 address\n
*  NULL : On failure \n
*
* @par Required Header File
* inet.h
*
* @par Note
* [N/A]
*
* @par Related Topics

* N/A
*/
char *inet_ntoa(struct in_addr in);

/*
Func Name:  inet_ntop
*/

/**
* @ingroup INET_Interfaces
* @par Prototype
* @code
* const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
* @endcode
*
* @par Purpose
* This  function is used to convert the network address structure src in the af address family into a character string.
* The resulting string is copied to the buffer pointed to by dst, which must be a non-null pointer.
* The caller specifies the number of bytes available in this buffer in the argument size.
*
* @par Description
* This  function is used to convert the network address structure src in the af address family into a character string.
* The resulting string is copied to the buffer pointed to by dst, which must be a non-null pointer.
* The caller specifies the number of bytes available in this buffer in the argument size.
*
* @param[in]   af    Address family [N/A]
* @param[in]   src   Pointer to IPv4 address string [N/A]
* @param[in]   size  Size of IPv4 address string [N/A]
* @param[out]  dst   Result string of IPv4 address will be udpated here [N/A]
*
* @par Return values
*  Valid pointer : On success, returns pointer to string notation of IPv4 address\n
*  NULL : On failure \n
*
* @par Required Header File
* inet.h
*
* @par Related Topics
* N/A
*/
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);

/*
Func Name:  inet_pton
*/

/**
* @ingroup INET_Interfaces
* @par Prototype
* @code
* int inet_pton(int af, const char *src, void *dst);
* @endcode
*
* @par Purpose
* This  function is used to convert the character string src into a network address
* structure in the af address family, then copies the network address structure to dst.
* The af argument must be AF_INET.
*
* @par Description
* This  function is used to convert the character string src into a network address
* structure in the af address family, then copies the network address structure to dst.
* The af argument must be AF_INET.
*
* @param[in]   af    Address family [N/A]
* @param[in]   src   Pointer to IPv4 address string [N/A]
* @param[out]  dst   Result string of IPv4 address will be udpated here [N/A]
*
* @par Return values
*  1 : On success, network address was successfully converted\n
*  0 : On failure, if src does not contain a character string representing a
*      valid network address in the specified address family \n
*  -1: On failure, if af does not contain a valid address family \n
*
* @par Required Header File
* inet.h
*
* @par Related Topics

* N/A
*/
int inet_pton(int af, const char *src, void *dst);

__END_DECLS

#endif /* _ARPA_INET_H_ */
