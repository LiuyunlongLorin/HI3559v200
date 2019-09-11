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

#ifndef _NET_IF_H_
#define _NET_IF_H_

#include <sys/socket.h>
#ifndef __LITEOS__
#include <linux/if.h>
#else
#include <liteos/if.h>
#endif
#include <sys/cdefs.h>

#ifndef IF_NAMESIZE
#define IF_NAMESIZE IFNAMSIZ
#endif

__BEGIN_DECLS

struct if_nameindex {
  unsigned if_index;
  char* if_name;
};

/**
* @defgroup IF_Interfaces IF Interfaces
* This section contains the IF interfaces.
* @ingroup User_interfaces
*/

/*
Func Name:  if_indextoname
*/

/**
* @ingroup IF_Interfaces
* @par Prototype
* @code
* char *if_indextoname(unsigned int ifindex, char *ifname);
* @endcode
*
* @par Purpose
* This  function is used to convert the name of the network interface corresponding
* to the interface index ifindex. The name is placed in the buffer pointed to by ifname.
* The buffer must allow for the storage of at least IF_NAMESIZE bytes.
*
* @par Description
* This  function is used to convert the name of the network interface corresponding
* to the interface index ifindex. The name is placed in the buffer pointed to by ifname.
* The buffer must allow for the storage of at least IF_NAMESIZE bytes.
*
* @param[in]    ifindex  The index of the network interface [N/A]
* @param[out]   ifname   The name of the network interface [N/A]
*
* @par Return values
*  ifname : On success, the ifname was successfully converted\n
*  NULL   : On failure, 0 is returned and errno is set appropriately\n
*
* @par Required Header File
* if.h
*
* @par Related Topics

* N/A
*/
char *if_indextoname(unsigned int ifindex, char *ifname);

/*
Func Name:  if_nametoindex
*/

/**
* @ingroup IF_Interfaces
* @par Prototype
* @code
* unsigned int if_nametoindex(const char *ifname);
* @endcode
*
* @par Purpose
* This function is used to convert the index of the network interface corresponding to the name ifname.
*
* @par Description
* This function is used to convert the index of the network interface corresponding to the name ifname.
*
* @param[in]   ifname    The name of the network interface [N/A]
*
* @par Return values
*  index : On success, the index number of the network interface was successfully converted\n
*  0 : On failure, 0 is returned and errno is set appropriately\n
*
* @par Required Header File
* if.h
*
* @par Related Topics

* N/A
*/
unsigned int if_nametoindex(const char *ifname);
struct if_nameindex* if_nameindex(void);
void if_freenameindex(struct if_nameindex*);

__END_DECLS

#endif