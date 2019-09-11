/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef _IFADDRS_H_
#define _IFADDRS_H_

#include <sys/cdefs.h>
#include <netinet/in.h>
#include <sys/socket.h>

__BEGIN_DECLS

struct ifaddrs {
  struct ifaddrs* ifa_next;
  char* ifa_name;
  unsigned int ifa_flags;
  struct sockaddr* ifa_addr;
  struct sockaddr* ifa_netmask;
  union {
    struct sockaddr* ifu_broadaddr;
    struct sockaddr* ifu_dstaddr;
  } ifa_ifu;
  void* ifa_data;
};

#define ifa_broadaddr ifa_ifu.ifu_broadaddr
#define ifa_dstaddr ifa_ifu.ifu_dstaddr
/**
* @defgroup Network_Interfaces_Info Network Interfaces Info
* This section contains the interfaces to get information about Network interfaces in LwIP.
* @ingroup User_interfaces
*/


/*
Func Name:  freeifaddrs
*/
/**
* @ingroup Network_Interfaces_Info
* @par Prototype
* @code
* void freeifaddrs(struct ifaddrs *ifa);
* @endcode
*
* @par Purpose
* This function is to free the memory of struct ifaddrs * provided by getifaddrs.
*
* @par Description
* The function getifaddrs, provides the list of network interfaces in struct ifaddrs*.
* The application has to free the memory of struct ifaddrs * by using this function.
*
* @param[in]    ifap                Pointer to struct ifaddrs [N/A]
*
* @par Return values
* None
*
* @par Required Header File
* ifaddrs.h.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
void freeifaddrs(struct ifaddrs *ifa);

/*
Func Name:  getifaddrs
*/
/**
* @ingroup Network_Interfaces_Info
* @par Prototype
* @code
* int getifaddrs(struct ifaddrs **ifap);
* @endcode
*
* @par Purpose
* This function creates a linked list of structures describing the network
* interfaces of the local system, and stores the address of the first item
* of the list in *ifap.
*
* @par Description
* This function creates a linked list of struct ifaddrs, which holds the information
* about the network interfaces of the local system. The ifa_next field contains a
* pointer to the next structure on the list, or NULL if this is the last item of the list.
* The ifa_name points to the null-terminated interface name. The ifa_flags field
* contains the interface flags. The ifa_addr field points to a structure containing
* the interface address. The ifa_netmask field points to a structure containing
* the netmask associated with ifa_addr, if applicable for the address family.
* Depending on whether the bit IFF_brcast or IFF_POINTOPOINT is set
* in ifa_flags (only one can be set at a time), either ifa_broadaddr will contain
* the brcast address associated with ifa_addr (if applicable for the address
* family) or ifa_dstaddr will contain the destination address of the point-to-point
* interface.
*
* @param[in]    ifap                Double Pointer to struct ifaddrs [N/A]
*
* @par Return values
*  ERR_OK : on success
*  Non zero value : on failure
*
* @par Required Header File
* ifaddrs.h.h
*
* @par Note
* The data returned by getifaddrs() is dynamically allocated and should be freed
* using freeifaddrs() when no longer needed.\n
*
* @par Related Topics
* \n
* N/A
*/
int getifaddrs(struct ifaddrs **ifap);

__END_DECLS

#endif