/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "fcntl.h"
#include "sched.h"
#include "errno.h"
#include "assert.h"
#ifdef CONFIG_FILE_MODE
#include "stdarg.h"
#endif
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "local.h"
#ifdef LOSCFG_FS_VFS
#include "fs/fs.h"
#include "inode/inode.h"
#endif
#ifdef LOSCFG_FS_VFS
#include "console.h"
#endif
#include "hisoc/uart.h"
#include "local.h"
#include "virtual_serial.h"

volatile UINT32 g_uwSerialType = 0;
volatile UINT32 g_uwUartMask = 0;

/*lint -e534*/
static int serial_open(struct file *filep)
{
    int ret = 0;
    struct file *pstfilep = (struct file *)NULL;
    const struct file_operations_vfs *pstfops = (struct file_operations_vfs *)NULL;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR){
        ret = -EINVAL;
        goto errout;
    }
    ret = filep_open(pstfilep,pstfops);
    if (ret < 0){
        ret = -EPERM;
        goto errout;
    }

    if (SERIAL_TYPE_UART_DEV == g_uwSerialType){
        hal_interrupt_unmask(NUM_HAL_INTERRUPT_UART);
    }else if (SERIAL_TYPE_USBTTY_DEV == g_uwSerialType){

    }
    return ENOERR;

errout:
    set_errno(-ret);
    return VFS_ERROR;
}

static int serial_close(struct file *filep)
{
    int ret = 0;
    struct file *pstfilep = (struct file *)NULL;
    const struct file_operations_vfs *pstfops = (struct file_operations_vfs *)NULL;

    if (SERIAL_TYPE_UART_DEV == g_uwSerialType){
        g_uwUartMask = 0;
        hal_interrupt_mask(NUM_HAL_INTERRUPT_UART);
    }else if (SERIAL_TYPE_USBTTY_DEV == g_uwSerialType){
        ret = get_filep_ops(filep, &pstfilep, &pstfops);
        if (ret != ENOERR){
            ret = -EINVAL;
            goto errout;
        }
        ret = filep_close(pstfilep,pstfops);
        if (ret < 0){
            ret = -EPERM;
            goto errout;
        }

    }
    return ENOERR;

errout:
    set_errno(-ret);
    return VFS_ERROR;
}

static ssize_t serial_read(struct file *filep, char *buffer, size_t buflen)
{
    int ret = 0;
    struct file *pstfilep = (struct file *)NULL;
    const struct file_operations_vfs *pstfops = NULL;
    if (SERIAL_TYPE_UART_DEV == g_uwSerialType){
        g_uwUartMask = 1;
    }
    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR){
        ret = -EINVAL;
        goto errout;
    }

    ret = filep_read(pstfilep,pstfops,buffer,buflen);
    if (ret < 0){
        goto errout;
    }
    return ret;

errout:
    set_errno(-ret);
    return VFS_ERROR;
}

/* Note: do not add print function in this module! */
static ssize_t serial_write(FAR struct file *filep, FAR const char *buffer, size_t buflen)
{
    int ret = 0;
    struct file *pstfilep = (struct file *)NULL;
    const struct file_operations_vfs *pstfops = NULL;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR){
        ret = -EINVAL;
        goto errout;
    }

    ret = filep_write(pstfilep,pstfops,buffer,buflen);
    if (ret < 0){
        goto errout;
    }
    return ret;

errout:
    set_errno(-ret);
    return VFS_ERROR;
}

static int serial_ioctl(struct file *filep, int cmd, unsigned long arg)
{
    int ret = 0;
    struct file *pstfilep = (struct file *)NULL;
    const struct file_operations_vfs *pstfops = (struct file_operations_vfs *)NULL;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR){
        ret = -EINVAL;
        goto errout;
    }

    ret = filep_ioctl(pstfilep,pstfops,cmd,arg);
    if (ret < 0){
        goto errout;
    }
    return ret;

errout:
     set_errno(-ret);
     return VFS_ERROR;
}

static int serial_poll(struct file *filep, poll_table *fds)
{
    int ret = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR)
    {
        ret = -EINVAL;
        goto errout;
    }
    ret = filep_poll(pstfilep,pstfops,fds);
    if (ret < 0){
        goto errout;
    }
    return ret;

errout:
    set_errno(-ret);
    return VFS_ERROR;
}

static const struct file_operations_vfs  serial_dev_ops =
{
    serial_open,                 /* open */
    serial_close,                /* close */
    serial_read,                 /* read */
    serial_write,
    NULL,
    serial_ioctl,
#ifndef CONFIG_DISABLE_POLL
    serial_poll,
#endif
    NULL
};

int virtual_serial_init(const char *device_name)
{
    int ret = 0;
    char *fullpath = (char *)NULL;
    struct file *filep = (struct file *)NULL;
    struct inode *inode = (struct inode *)NULL;


    if (!strncmp(device_name,SERIAL_UARTDEV,12)){
        g_uwSerialType = SERIAL_TYPE_UART_DEV;
    }
    else if (!strncmp(device_name,SERIAL_TTYGS0,11)){
        g_uwSerialType = SERIAL_TYPE_USBTTY_DEV;
    }

    filep = (struct file *)LOS_MemAlloc(m_aucSysMem0, sizeof(struct file));
    if (!filep)
    {
        ret = ENOMEM;
        goto errout;
    }

    ret = vfs_normalize_path(NULL, device_name, &fullpath);
    if (ret < 0)
    {
        ret = EINVAL;
        goto errout_with_filep;
    }

    inode = inode_find(fullpath, NULL);
    if (!inode)
    {
        ret = ENOENT;
        goto errout_with_fullpath;
    }

    filep->f_oflags = O_RDWR;
    filep->f_pos    = 0;
    filep->f_inode  = inode;
    filep->f_path   = NULL;
    filep->f_priv   = NULL;

    if (inode->u.i_ops->open){
        (void)inode->u.i_ops->open(filep);
    }else{
        ret = EFAULT;
        goto errout_with_inode;
    }

    (void)register_driver(SERIAL, &serial_dev_ops, 0666, (void *)filep);
    inode_release(inode);
    free(fullpath);
    return ENOERR; /*lint !e429*/

errout_with_inode:
    inode_release(inode);
errout_with_fullpath:
    free(fullpath);
errout_with_filep:
    (VOID)LOS_MemFree(m_aucSysMem0, filep);/*lint !e424*/
errout:
    set_errno(ret);
    return VFS_ERROR;
}

int virtual_serial_deinit(void)
{
    int ret = 0;
    struct file *filep = NULL;
    struct inode *inode = NULL;
    char *fullpath = NULL;
    /* It's a process opposite virtual_serial_init */
    ret = vfs_normalize_path(NULL, SERIAL, &fullpath);
    if (ret < 0){
        ret = EINVAL;
        goto errout;
    }

    inode = inode_find(fullpath, NULL);
    if (!inode){
        ret = ENOENT;
        goto errout_with_fullpath;
    }

    filep = inode->i_private;
    if (NULL != filep && NULL != inode->u.i_ops){
        inode->u.i_ops->close(filep);               /* close filep */
        (VOID)LOS_MemFree(m_aucSysMem0, filep);     /* free filep what you malloc from console_init */
    }
    else if (NULL != filep){
        ret = EPERM;
        goto errout_with_filep;
    }else{
        ret = EBADF;
        goto errout_with_inode;
    }
    inode_release(inode);
    free(fullpath);
    (VOID)unregister_driver(SERIAL);
    return ENOERR;

errout_with_filep:
    (VOID)LOS_MemFree(m_aucSysMem0, filep);
errout_with_inode:
    inode_release(inode);
errout_with_fullpath:
    free(fullpath);
errout:
    set_errno(ret);
    return VFS_ERROR;
}
