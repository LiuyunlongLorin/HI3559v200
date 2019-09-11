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
#include "local.h"
#include "unistd.h"
#include "inode/inode.h"
#include "console.h"
#include "linux/wait.h"
#include "string.h"
#include "poll.h"
#include "securec.h"
#ifdef LOSCFG_SHELL_DMESG
#include "dmesg.ph"
#endif

#ifdef LOSCFG_SHELL
#include "shcmd.h"
#include "show.h"
#endif

/*lint -e438 -e429 -e534*/

CONSOLE_CB      *g_stConsole[2];
#define min(a, b) ((a) < (b) ? (a) : (b))

/* acquire uart driver function and filep of /dev/console,
 * then store uart driver function in *pst_filep_ops
 * and store filep of /dev/console in *pst_filep.
 */
int get_filep_ops(struct file *filep, struct file **pst_filep, const struct file_operations_vfs **pst_filep_ops )
{
    int ret = 0;

    if (!filep || !(filep->f_inode) || !(filep->f_inode->i_private)){
        ret = EINVAL;
        goto errout;
    }

    /* to find console device's filep(now it is *pst_filep) throught i_private */
    *pst_filep = (struct file *)filep->f_inode->i_private;

    if (!((*pst_filep)->f_inode) || !((*pst_filep)->f_inode->u.i_ops)){
        ret = EINVAL;
        goto errout;
    }


    /*to find uart driver operation function throutht u.i_opss */
    *pst_filep_ops = (*pst_filep)->f_inode->u.i_ops;

    return ENOERR;
errout:
    set_errno(ret);
    return VFS_ERROR;
}

int console_tcgetattr(int fd, struct termios *termios)
{
    struct file * filep;
    CONSOLE_CB * pStConsole;

    if (fd >= 0 && fd < 3){
        fd = update_console_fd();
        if (fd < 0)
        {
          return -EBADF;
        }
    }

    filep = fs_getfilep(fd);
    if (filep == NULL)
    {
        return -EPERM;
    }

    pStConsole = (CONSOLE_CB *)filep->f_priv;
    if (pStConsole == NULL){
        return -EFAULT;
    }

    termios->c_lflag = pStConsole->stConsoleTermios.c_lflag;
    return LOS_OK;
}

int console_tcsetattr(int fd , int actions , const struct termios *termios)
{
    struct file * filep;
    CONSOLE_CB * pStConsole;

    if (fd >= 0 && fd < 3){
        fd = update_console_fd();
        if (fd < 0)
        {
          return -EBADF;
        }
    }

    filep = fs_getfilep(fd);
    if (filep == NULL)
    {
        return -EPERM;
    }

    pStConsole = (CONSOLE_CB *)filep->f_priv;
    if (pStConsole == NULL){
        return -EFAULT;
    }
    pStConsole->stConsoleTermios.c_lflag = termios->c_lflag;
    return LOS_OK;
}


static BOOL console_refcount_get(CONSOLE_CB *pStConsole)
{
    return pStConsole->uwRefCount;
}
static VOID console_refcount_set(CONSOLE_CB *pStConsole, BOOL count)
{
    pStConsole->uwRefCount = count;
}
BOOL console_is_occupied(CONSOLE_CB *pStConsole)
{
    if (FALSE != console_refcount_get(pStConsole))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static int console_ctrl_rights_capture(CONSOLE_CB * pStConsole)
{
    (void)LOS_SemPend(pStConsole->uwConsoleSem,LOS_WAIT_FOREVER);
    if (FALSE == console_refcount_get(pStConsole)){
        LOS_TaskSuspend(pStConsole->uwSystaskid);
        console_refcount_set(pStConsole, TRUE);
        (void)LOS_SemPost(pStConsole->uwConsoleSem);
        return LOS_OK;
    }else{
        PRINT_ERR("console is under control\n");
    }
    (void)LOS_SemPost(pStConsole->uwConsoleSem);
    return LOS_NOK;
}
static int console_ctrl_rights_release(CONSOLE_CB * pStConsole)
{
    (void)LOS_SemPend(pStConsole->uwConsoleSem,LOS_WAIT_FOREVER);

    if (TRUE == console_refcount_get(pStConsole)){
        console_refcount_set(pStConsole, FALSE);
        LOS_TaskResume(pStConsole->uwSystaskid);
        (void)LOS_SemPost(pStConsole->uwConsoleSem);
        return LOS_OK;
    }else{
        PRINT_ERR("console is free\n");
    }
    (void)LOS_SemPost(pStConsole->uwConsoleSem);
    return LOS_NOK;
}

static CONSOLE_CB *get_console_by_device(const char* device_name)
{
    int ret = 0;
    char *fullpath;
    FAR struct inode *inode = NULL;

    ret = vfs_normalize_path(NULL, device_name, &fullpath);
    if (ret < 0)
    {
        set_errno(EINVAL);
        return NULL;
    }

    inode = inode_find(fullpath, NULL);
    free(fullpath);
    if (!inode)
    {
        set_errno(ENOENT);
        return NULL;
    }

    if (g_stConsole[CONSOLE_SERIAL - 1]->dev_inode == inode)
    {
        inode_release(inode);
        return g_stConsole[CONSOLE_SERIAL - 1];
    }
    else if (g_stConsole[CONSOLE_TELNET - 1]->dev_inode == inode)
    {
        inode_release(inode);
        return g_stConsole[CONSOLE_TELNET - 1];
    }
    else
    {
        inode_release(inode);
        set_errno(ENOENT);
        return NULL;
    }
}

static int get_console_id(const char * device_name)
{
    if ((device_name != NULL)
        && (strlen(device_name) == strlen(SERIAL))
        && (!strncmp(device_name,SERIAL,strlen(SERIAL))))
    {
        return CONSOLE_SERIAL;
    }
#ifdef LOSCFG_NET_TELNET
    else if ((device_name != NULL)
        && (strlen(device_name) == strlen(TELNET))
        && (!strncmp(device_name,(TELNET),strlen((TELNET)))))
    {
        return CONSOLE_TELNET;
    }
#endif
    return -1;
}
static BOOL console_fifo_empty(CONSOLE_CB *console)
{
    if (console->rp == console->wp)
        return TRUE;
    return FALSE;
}
static void console_fifo_clearup(CONSOLE_CB *console)
{
    console->rp = 0;
    console->wp = 0;
    (void)memset_s(console->ucFifo,CONSOLE_FIFO_SIZE,0,CONSOLE_FIFO_SIZE);
}
static void console_fifolen_update(CONSOLE_CB *console)
{
    console->uwCurrentLen = console->wp - console->rp;
}
static int console_memcpy(char *buffer, CONSOLE_CB *console, size_t buflen)
{
    int len = 0;
    int readnum = 0;
    readnum = min(buflen,console->uwCurrentLen);
    (void)memcpy_s(buffer,readnum,console->ucFifo + console->rp,readnum);
    console->rp += readnum;
    if (console_fifo_empty(console))
    {
        console_fifo_clearup(console);
    }
    console_fifolen_update(console);
    len = readnum;
    return len;
}
int filep_open(struct file *filep, const struct file_operations_vfs *fops)
{
    int ret;
    if (!fops->open){
        return -EFAULT;
    }

    /* adopt uart open function to open pstfilep (pstfilep is
     * corresponding to filep of /dev/console)
     */
    ret = fops->open(filep);
    if (ret < 0){
        return -EPERM;
    }
    return ret;
}
int user_filep_read(CONSOLE_CB * pStConsole,struct file *filep, const struct file_operations_vfs *fops, char *buffer, size_t buflen)
{
    int ret = 0;
    char ch;

    if (!fops->read){
        return -EFAULT;
    }

    /* adopt uart read function to read data from pstfilep
     * and write data to buffer (pstfilep is
     * corresponding to filep of /dev/console)
     */
    /* 1 store data to console buffer */
    if (pStConsole->stConsoleTermios.c_lflag & ICANON)
    {
        /* 2 read data and stored data into console fifo */
        if (0 == pStConsole->uwCurrentLen)
        {
            while(1){
                ret = fops->read(filep, &ch, 1);
                if (ret <= 0){
                    return ret;
                }

                if ('\r' == ch){
                    ch = '\n';
                }

                if (pStConsole->stConsoleTermios.c_lflag & ECHO){
                    fops->write(filep, &ch, 1);
                }

                /* 2.2 store what you input */
                /* wp should less than CONSOLE_FIFO_SIZE -1 - space of '\n' and '\0' */
                if ((ret == 1) && (ch != '\n') &&(pStConsole->wp <= CONSOLE_FIFO_SIZE - 3)){
                    pStConsole->ucFifo[pStConsole->wp] = ch;
                    pStConsole->wp ++;
                }

                /* 2.3 return what you input */
                if ('\n' == ch)
                {
                    if (pStConsole->stConsoleTermios.c_lflag & ECHO){
                        ch = '\r';
                        fops->write(filep, &ch, 1);
                    }
                    pStConsole->ucFifo[pStConsole->wp++] = '\n';
                    pStConsole->ucFifo[pStConsole->wp] = '\0';
                    pStConsole->uwCurrentLen = pStConsole->wp;
                    ret = console_memcpy(buffer,pStConsole,buflen);
                    break;
                }
            }
        }else{
            /* if data is already in console fifo, we returen them immediately */
            ret = console_memcpy(buffer,pStConsole,buflen);
        }
    }else{
        ret = fops->read(filep, buffer, buflen);
        if (ret < 0){
            return -EPERM;
        }
    }

    return ret;
}

int filep_read(struct file *filep, const struct file_operations_vfs *fops, char *buffer, size_t buflen)
{
    int ret;
    if (!fops->read){
        return -EFAULT;
    }
    /* adopt uart read function to read data from pstfilep
     * and write data to buffer (pstfilep is
     * corresponding to filep of /dev/console)
     */
    ret = fops->read(filep, buffer, buflen);
    if (ret < 0){
        return -EPERM;
    }
    return ret;
}

int filep_write(struct file *filep, const struct file_operations_vfs *fops, const char *buffer, size_t buflen)
{
    int ret;
    if (!fops->write){
        return -EFAULT;
    }

    ret = fops->write(filep, buffer, buflen);
    if (ret < 0){
        return -EPERM;
    }
    return ret;
}
int filep_close(struct file *filep, const struct file_operations_vfs *fops)
{
    int ret;
    if (!fops->close){
        return -EFAULT;
    }

    /* adopt uart close function to open pstfilep (pstfilep is
     * corresponding to filep of /dev/console)
     */
    ret = fops->close(filep);
    if (ret < 0){
        return -EPERM;
    }
    return ret;
}
int filep_ioctl(struct file *filep, const struct file_operations_vfs *fops, int cmd, unsigned long arg)
{
    int ret;
    if (NULL == fops->ioctl){
        return -EFAULT;
    }

    ret = fops->ioctl(filep, cmd, arg);
    if (ret < 0){
        return -EPERM;
    }
    return ret;
}
int filep_poll(struct file *filep, const struct file_operations_vfs *fops, poll_table *fds)
{
    int ret;
    if (!fops->poll){
        return -EFAULT;
    }

    /* adopt uart poll function to poll pstfilep (pstfilep is
     * corresponding to filep of /dev/serial)
     */
    ret = fops->poll(filep,fds);
    if (ret < 0){
        return -EPERM;
    }
    return ret;
}
static int console_open(struct file *filep)
{
    int ret = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;

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
    return ENOERR;

errout:
    set_errno(-ret);
    return VFS_ERROR;
}

static int console_close(struct file *filep)
{
    int ret = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;

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
    return ENOERR;

errout:
    set_errno(-ret);
    return VFS_ERROR;
}

static ssize_t console_read(struct file *filep, char *buffer, size_t buflen)
{
    int ret = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;
    CONSOLE_CB * pStConsole;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR){
        ret = -EINVAL;
        goto errout;
    }
    pStConsole = (CONSOLE_CB *)filep->f_priv;
    if (pStConsole == NULL){
        pStConsole = get_console_by_id(g_stLosTask.pstRunTask->usConsoleID);
        if (pStConsole == NULL){
            return -EFAULT;
        }
    }

    /* shell task use filep_read function to get data,
     * user task use user_filep_read to get data */
    if (pStConsole->uwSystaskid == g_stLosTask.pstRunTask->uwTaskID){
        ret = filep_read(pstfilep,pstfops,buffer,buflen);
        if (ret < 0){
            goto errout;
        }
    }else{
        ret = user_filep_read(pStConsole,pstfilep,pstfops,buffer,buflen);
        if (ret < 0){
            goto errout;
        }
    }

    return ret;

errout:
    set_errno(-ret);
    return VFS_ERROR;
}

static ssize_t console_write(struct file *filep, const char *buffer, size_t buflen)
{
    int ret = 0;
    int cnt = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR){
        ret = EINVAL;
        goto errout;
    }
    if (!pstfops->write){
        ret = EFAULT;
        goto errout;
    }
    /* adopt uart open function to read data from buffer
     * and write data to pstfilep (pstfilep is
     * corresponding to filep of /dev/console)
     */

#ifdef LOSCFG_SHELL_DMESG
    (void)osLogMemcpyRecord(buffer,buflen);
    if(!osCheckConsoleLock())
    {
#endif
    while (cnt < (int)buflen)
    {
        if (('\n' == buffer[cnt]) || ('\r' == buffer[cnt]))
        {
            pstfops->write(pstfilep, "\r", 1);
            pstfops->write(pstfilep, &buffer[cnt], 1);
            cnt++;
            continue;
        }
        pstfops->write(pstfilep, &buffer[cnt], 1);
        cnt++;
    }
#ifdef LOSCFG_SHELL_DMESG
    }
#endif
    return cnt;

errout:
     set_errno(ret);
     return VFS_ERROR;
}

static int console_ioctl(struct file *filep, int cmd, unsigned long arg)
{
    int ret = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;

    CONSOLE_CB * pStConsole;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR){
        ret = EINVAL;
        goto errout;
    }

    if (!pstfops->ioctl){
        ret = EFAULT;
        goto errout;
    }

    pStConsole = (CONSOLE_CB *)filep->f_priv;
    if (pStConsole == NULL)
    {
        ret = EINVAL;
        goto errout;
    }


    switch(cmd){
        case CONSOLE_CONTROL_RIGHTS_CAPTURE:
            ret = console_ctrl_rights_capture(pStConsole);
            break;
        case CONSOLE_CONTROL_RIGHTS_RELEASE:
            ret = console_ctrl_rights_release(pStConsole);
            break;
        default:
            ret = pstfops->ioctl(pstfilep, cmd, arg);
            break;
    }

    if (ret < 0){
        ret = EPERM;
        goto errout;
    }

    return ret;
errout:
    set_errno(ret);
    return VFS_ERROR;
}

static int console_poll(struct file *filep, poll_table *fds)
{
    int ret = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;
    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR){
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

/* console device driver function structure */
static const struct file_operations_vfs  console_dev_ops =
{
    console_open,                 /* open */
    console_close,                /* close */
    console_read,                 /* read */
    console_write,                /* write */
    NULL,
    console_ioctl,
#ifndef CONFIG_DISABLE_POLL
    console_poll,
#endif
    NULL
};

/* Initialized console control platform so that when we operate /dev/console
 * as if we are operating /dev/ttyS0 (uart0).
 */
static int console_init(CONSOLE_CB *pConsole, const char *device_name)
{
    int ret = 0;
    char *fullpath = NULL;
    struct file *filep = NULL;
    struct inode *inode = NULL;/*lint !e578*/

    /* allocate memory for filep,in order to unchange the value of filep */
    filep = (struct file *)LOS_MemAlloc(m_aucSysMem0, sizeof(struct file));
    if (!filep)
    {
        ret = ENOMEM;
        goto errout;
    }

    /* Adopt procedure of open function to allocate 'filep' to /dev/console */
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

    pConsole->dev_inode = inode;

    /* initialize the console filep which is associated with /dev/console,
     * assign the uart0 inode of /dev/ttyS0 to console inod of /dev/console,
     * then we can operate console's filep as if we operate uart0 filep of
     * /dev/ttyS0.
     */
    memset(filep, 0, sizeof(struct file));
    filep->f_oflags = O_RDWR;
    filep->f_pos    = 0;
    filep->f_inode  = inode;
    filep->f_path   = fullpath;
    filep->f_priv   = NULL;

    if (inode->u.i_ops->open){
        inode->u.i_ops->open(filep);
    }else{
        ret = EFAULT;
        goto errout_with_inode;
    }

    /* Use filep to connect console and uart, we can find uart driver function throught filep.
     * now we can operate /dev/console to operate /dev/ttyS0 through filep.
     */
    register_driver(pConsole->name, &console_dev_ops, 0666, (void *)filep);
    inode_release(inode);
    free(fullpath);
    return ENOERR;

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

static int console_deinit(const char *console_name)
{
    int ret = 0;
    struct file *filep = NULL;
    struct inode *inode = NULL;
    char *fullpath = NULL;

    /* It's a process opposite console_init */

    ret = vfs_normalize_path(NULL, console_name, &fullpath);
    if (ret < 0)
    {
        ret = EINVAL;
        goto errout;
    }

    inode = inode_find(fullpath, NULL);
    if (!inode)
    {
        ret = ENOENT;
        goto errout_with_fullpath;
    }

    filep = inode->i_private;
    if (NULL != filep)
    {
        (VOID)LOS_MemFree(m_aucSysMem0, filep); /* free filep what you malloc from console_init */
    }
    else
    {
        ret = EBADF;
        goto errout_with_inode;
    }
    inode_release(inode);
    free(fullpath);
    (VOID)unregister_driver(console_name);
    return ENOERR;

errout_with_inode:
    inode_release(inode);
errout_with_fullpath:
    free(fullpath);
errout:
    set_errno(ret);
    return VFS_ERROR;
}

#if defined (LOSCFG_DRIVERS_USB_SERIAL_GADGET) || defined (LOSCFG_DRIVERS_USB_ETH_SER_GADGET)
extern volatile UINT32 g_usbSerialMask;
#endif
extern volatile UINT32 g_uwSerialType;
extern unsigned int g_uart_fputc_en;

BOOL console_enable(void)
{
    int console_id = 0;
    if (g_stLosTask.pstRunTask)
    {
        console_id = g_stLosTask.pstRunTask->usConsoleID;
        if (g_uart_fputc_en == 0)
        {
            console_id = CONSOLE_TELNET;
        }

        if (console_id == 0)
        {
            return FALSE;
        }
        else if (console_id == CONSOLE_TELNET)
        {
            return TRUE;
        }
#if defined (LOSCFG_DRIVERS_USB_SERIAL_GADGET) || defined (LOSCFG_DRIVERS_USB_ETH_SER_GADGET)
        else if (g_uwSerialType == SERIAL_TYPE_USBTTY_DEV && g_usbSerialMask == 1)
        {
            return TRUE;
        }
#endif
    }

    return FALSE;
}

int system_console_deinit(const char *device_name)
{
    int ret = 0;
    CONSOLE_CB *pStConsole;

    pStConsole = get_console_by_device(device_name);
    if (pStConsole == NULL)
    {
        return VFS_ERROR;
    }

#ifdef LOSCFG_SHELL
    osShellDeinit(pStConsole->uwConsoleId);
#endif

    (void)files_close(pStConsole->fd);

    ret = console_deinit(pStConsole->name);
    if (LOS_OK != ret)
    {
        PRINT_ERR("%s, Failed to system_console_deinit\n",__FUNCTION__);/*lint !e40*/
        return VFS_ERROR;
    }



    g_stConsole[pStConsole->uwConsoleId - 1] = NULL;
    LOS_MemFree((void *)m_aucSysMem0, pStConsole);

    return ENOERR;
}

/* Initialized system console and return stdinfd stdoutfd stderrfd */
int system_console_init(const char *device_name)
{
    int ret = 0;
    FAR struct inode *inode = NULL;/*lint !e578*/
    FAR struct file *filep;
    CONSOLE_CB * pStConsole;
    int console_id = 0;
    struct termios console_termios;

    console_id = get_console_id(device_name);

    if (console_id == -1)
    {
        PRINT_ERR("device is full.\n");
        return VFS_ERROR;
    }

    pStConsole = (CONSOLE_CB *)LOS_MemAlloc((void *)m_aucSysMem0,  sizeof(CONSOLE_CB));
    if (pStConsole == NULL)
    {
        PRINT_ERR("console malloc error.\n");
        return VFS_ERROR;
    }

    pStConsole->uwConsoleId = console_id;
    pStConsole->uwConsoleSem = 0;
    pStConsole->uwConsoleType = 0;
    pStConsole->uwRefCount = 0;
    pStConsole->uwCurrentLen = 0;
    pStConsole->rp = 0;
    pStConsole->wp = 0;
    pStConsole->stConsoleTermios.c_lflag = 0;

    (void)memset_s(pStConsole->ucFifo,CONSOLE_FIFO_SIZE,0,CONSOLE_FIFO_SIZE);

    (void)snprintf(pStConsole->name, CONSOLE_NAMELEN, "%s%d", CONSOLE, pStConsole->uwConsoleId);

    ret = LOS_SemCreate(1, &pStConsole->uwConsoleSem);
    if(ret != LOS_OK)
    {
        LOS_MemFree((void *)m_aucSysMem0, pStConsole);
        PRINT_ERR("creat sem for uart failed\n");
        return VFS_ERROR;
    }


    ret = console_init(pStConsole, device_name);
    if (0 != ret)
    {
        PRINT_ERR("%s, %d",__func__, __LINE__);/*lint !e40*/
        goto errout;
    }

    inode = inode_find(pStConsole->name, NULL);
    if (!inode)
    {
        ret = ENOENT;
        goto errout;
    }

    pStConsole->fd  = files_allocate(inode, O_RDWR, 0, pStConsole,3);
    if (pStConsole->fd < 0)
    {
        ret = EMFILE;
        goto errout_with_inode;
    }

    filep = fs_getfilep(pStConsole->fd);
    if (filep == NULL)
    {
        ret = EPERM;
        goto errout_with_inode;
    }
    filep->f_path = pStConsole->name;
    g_stConsole[console_id - 1] = pStConsole;

    /* Judge which virtual device device_name is */
    if ((device_name != NULL)
        && (strlen(device_name) == strlen(SERIAL))
        && (!strncmp(device_name,SERIAL,strlen(SERIAL)))){
        pStConsole->bisNonBlock = set_serial_block(pStConsole);
    }
#ifdef LOSCFG_NET_TELNET
    else if ((device_name != NULL)
        && (strlen(device_name) == strlen(TELNET))
        && (!strncmp(device_name,TELNET,strlen(TELNET)))){
        pStConsole->bisNonBlock = set_telnet_block(pStConsole);
    }
#endif

    inode_release(inode);
    if (g_stLosTask.pstRunTask)
    {
        g_stLosTask.pstRunTask->usConsoleID = console_id;
    }

    /* set console to have a buffer for user */
    console_tcgetattr(pStConsole->fd,&console_termios);
    console_termios.c_lflag |= ICANON | ECHO;
    console_tcsetattr(pStConsole->fd,0,&console_termios);

#ifdef LOSCFG_SHELL
    if (osShellInit(console_id) != 0)
    {
        PRINTK("osShellInit failed\n");
    }
#endif

    return ret;

errout_with_inode:
 inode_release(inode);

errout:
    LOS_MemFree((void *)m_aucSysMem0, pStConsole);
    set_errno(ret);
    return VFS_ERROR;
}

void console_task_reg(int console_id, UINT32 taskid)
{
    g_stConsole[console_id - 1]->uwSystaskid = taskid;

}

BOOL set_serial_nonblock(CONSOLE_CB * pStConsole)
{
    int ret = 0;

    ret = ioctl(pStConsole->fd, CONSOLE_CMD_RD_BLOCK_SERIAL, CONSOLE_RD_NONBLOCK);
    if (ret != 0)
    {
        return 0;
    }

    return 1;
}

BOOL set_serial_block(CONSOLE_CB * pStConsole)
{
    int ret = 0;

    ret = ioctl(pStConsole->fd, CONSOLE_CMD_RD_BLOCK_SERIAL, CONSOLE_RD_BLOCK);
    if (ret != 0)
    {
        return 1;
    }

    return 0;
}

BOOL set_telnet_nonblock(void * pStConsole)
{
    int ret = 0;

    ret = ioctl(((CONSOLE_CB *)pStConsole)->fd, CONSOLE_CMD_RD_BLOCK_TELNET, CONSOLE_RD_NONBLOCK);
    if (ret != 0)
    {
        return 0;
    }
    return 1;
}

BOOL set_telnet_block(void * pStConsole)
{
    int ret = 0;

    ret = ioctl(((CONSOLE_CB *)pStConsole)->fd, CONSOLE_CMD_RD_BLOCK_TELNET, CONSOLE_RD_BLOCK);
    if (ret != 0)
    {
        return 1;
    }
    return 0;
}

BOOL is_nonblock(void * pStConsole)
{
    return ((CONSOLE_CB *)pStConsole)->bisNonBlock;
}

int update_console_fd(void)
{
    int console_id = 0;

    if (g_stLosTask.pstRunTask)
    {
        console_id = g_stLosTask.pstRunTask->usConsoleID;
    }
    else
    {
        return -1;
    }

    if (g_uart_fputc_en == 0 && (g_stConsole[CONSOLE_TELNET - 1]))
    {
        console_id = CONSOLE_TELNET;
    }
    else if (console_id == 0)
    {
        if (g_stConsole[CONSOLE_SERIAL - 1])
        {
            console_id = CONSOLE_SERIAL;
        }
        else if (g_stConsole[CONSOLE_TELNET - 1])
        {
            console_id = CONSOLE_TELNET;
        }
        else
        {
            PRINT_ERR("No console dev used.\n");
            return -1;
        }
    }

    return g_stConsole[console_id - 1]->fd;
}

CONSOLE_CB * get_console_by_id(int console_id)
{
    if (console_id != CONSOLE_TELNET)
    {
        console_id = CONSOLE_SERIAL;
    }
    return g_stConsole[console_id - 1];
}

/*lint -e438 -e429 -e534*/
