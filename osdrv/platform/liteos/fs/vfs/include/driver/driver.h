/****************************************************************************
 * fs/driver/driver.h
 *
 *   Copyright (C) 2007, 2009, 2012, 2014 Gregory Nutt. All rights reserved.
 *   Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
 *   All rights reserved.
 *
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
/****************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country in
 * which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 ****************************************************************************/

#ifndef __FS_DRIVER_DRIVER_H
#define __FS_DRIVER_DRIVER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "vfs_config.h"
#include "fs/fs.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

 /****************************************************************************
 * Global Variables
 ****************************************************************************/

extern FAR struct inode *root_inode;


/**
 * @ingroup  disk
 * @brief Set usb mode.
 *
 * @par Description:
 * Set the corresponding bit of g_uwUsbMode as usb host mode.
 *
 * @attention
 * <ul>
 * <li>disk_id should be [0,SYS_MAX_DISK)</li>
 * </ul>
 *
 * @param  disk_id      [IN]  Type # unsigned int  disk id.
 *
 * @par Dependency:
 * <ul><li>driver.h</li></ul>
 * @see os_clear_usb_status
 *
 * @since Huawei LiteOS V100R001C00
 */
 void os_set_usb_status(unsigned int disk_id);

/**
 * @ingroup  disk
 * @brief Set usb mode.
 *
 * @par Description:
 * Clear the corresponding bit of g_uwUsbMode as usb host mode.
 *
 * @attention
 * <ul>
 * <li>disk_id should be [0,SYS_MAX_DISK)</li>
 * </ul>
 *
 * @param  disk_id      [IN]  Type # unsigned int  disk id.
 *
 * @par Dependency:
 * <ul><li>driver.h</li></ul>
 * @see os_set_usb_status
 *
 * @since Huawei LiteOS V100R001C00
 */
void os_clear_usb_status(unsigned int disk_id);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* __FS_DRIVER_DRIVER_H */
