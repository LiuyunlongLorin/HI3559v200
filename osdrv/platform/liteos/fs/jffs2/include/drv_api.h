#ifndef JFFS2ONCE_DRV_API_H
#define JFFS2ONCE_DRV_API_H
//=============================================================================
//
//      fileio.h
//
//      Fileio header
//
//=============================================================================
//*   Copyright (c) <2014-2017>, <Huawei Technologies Co., Ltd>
//*   All rights reserved.
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later
// version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with eCos; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// As a special exception, if other files instantiate templates or use
// macros or inline functions from this file, or you compile this file
// and link it with other works to produce a work based on this file,
// this file does not by itself cause the resulting work to be covered by
// the GNU General Public License. However the source code for this file
// must still be made available in accordance with section (3) of the GNU
// General Public License v2.
//
// This exception does not invalidate any other reasons why a work based
// on this file might be covered by the GNU General Public License.
// -------------------------------------------
// ####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg
// Date:          2000-05-25
// Purpose:       Fileio header
// Description:   This header contains the external definitions of the general file
//                IO subsystem for POSIX and EL/IX compatability.
//
// Usage:
//              #include <fileio.h>
//              ...
//
//
//####DESCRIPTIONEND####
/****************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country in
 * which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 ****************************************************************************/
//=============================================================================

#include "jffs2_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef JFFS2PKG_KERNEL

/*------------------------------------------------------------------------*/
/* Kernel co-resident version of API                                      */

#include <pkgconf/kernel.h>

#ifndef JFFS2FUN_KERNEL_API_C
#error Driver API requres Kernel API to be present
#endif

#define jffs2_drv_mutex_t                     jffs2_mutex_t
#define jffs2_drv_mutex_init                  jffs2_mutex_init
#define jffs2_drv_mutex_destroy              jffs2_mutex_destroy
#define jffs2_drv_mutex_lock                  jffs2_mutex_lock
#define jffs2_drv_mutex_trylock               jffs2_mutex_trylock
#define jffs2_drv_mutex_unlock                jffs2_mutex_unlock
#define jffs2_drv_mutex_release               jffs2_mutex_release

#else /* JFFS2PKG_KERNEL */

/*------------------------------------------------------------------------*/
/* Non-kernel version of API                                              */
typedef struct
{
    jffs2_atomic          lock;
} jffs2_drv_mutex_t;

externC void jffs2_drv_mutex_init( jffs2_drv_mutex_t *mutex );
externC void jffs2_drv_mutex_destroy( jffs2_drv_mutex_t *mutex );
externC int jffs2_drv_mutex_lock( jffs2_drv_mutex_t *mutex );
externC int jffs2_drv_mutex_trylock( jffs2_drv_mutex_t *mutex );
externC void jffs2_drv_mutex_unlock( jffs2_drv_mutex_t *mutex );
externC void jffs2_drv_mutex_release( jffs2_drv_mutex_t *mutex );


#endif /* JFFS2PKG_KERNEL */

/*------------------------------------------------------------------------*/
/* EOF drv_api.h                                                          */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif // JFFS2ONCE_DRV_API_H
