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


#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOSCFG_FS_VFS
#include "fs/fs.h"
#include "virtual_serial.h"
#ifdef LOSCFG_NET_TELNET
#include "telnet_dev.h"
#endif
#include "asm/termbits.h"

#define LOSCFG_PLATFORM_CONSOLE
#define STDIN                               0
#define STDOUT                              1
#define STDERR                              2

#define CONSOLE                             "/dev/console"
#define CONSOLE_NAMELEN                     16

#define CONSOLE_CMD_RD_BLOCK_SERIAL         0x104
#define CONSOLE_CMD_RD_BLOCK_TELNET         101
#define CONSOLE_RD_BLOCK                    1
#define CONSOLE_RD_NONBLOCK                 0
#define CONSOLE_SHELL_KEY_EVENT             0x112
#define CONSOLE_CONTROL_RIGHTS_CAPTURE      201
#define CONSOLE_CONTROL_RIGHTS_RELEASE      202
#define CONSOLE_UART_BAUDRATE               115200
#define CONSOLE_FIFO_SIZE                   1024

typedef struct tagConsoleCB
{
    UINT32   uwConsoleId;
    UINT32   uwConsoleType;
    UINT32   uwConsoleSem;
    UINT32   uwSystaskid;
    UINT32   uwConsoleMask;
    struct inode *dev_inode;
    char     name[CONSOLE_NAMELEN];
    int      fd;
    BOOL     uwRefCount;
    BOOL     bisNonBlock;
#ifdef LOSCFG_SHELL
    void     *pStShellHandle;
#endif
    UINT8    ucFifo[CONSOLE_FIFO_SIZE];
    UINT32   rp;
    UINT32   wp;
    UINT32   uwCurrentLen;
    struct termios stConsoleTermios;
} CONSOLE_CB;

extern int system_console_init(const char *device_name);
extern int system_console_deinit(const char *device_name);
extern int stdio_fd_init(int fd, char *fullpath);
extern BOOL set_serial_nonblock(CONSOLE_CB * pStConsole);
extern BOOL set_serial_block(CONSOLE_CB * pStConsole);
extern BOOL set_telnet_nonblock(void * pStConsole);
extern BOOL set_telnet_nonblock(void * pStConsole);
extern CONSOLE_CB * get_console_by_id(int console_id);
extern void console_task_reg(int console_id, UINT32 taskid);
extern BOOL is_nonblock(void * pStConsole);
extern BOOL console_is_occupied(CONSOLE_CB * pStConsole);
extern int filep_open(struct file * filep, const struct file_operations_vfs * fops);
extern int filep_close(struct file * filep, const struct file_operations_vfs * fops);
extern int user_filep_read(CONSOLE_CB * pStConsole,struct file * filep, const struct file_operations_vfs * fops,char * buffer,size_t buflen);
extern int filep_read(struct file *filep, const struct file_operations_vfs *fops, char *buffer, size_t buflen);
extern int filep_write(struct file * filep, const struct file_operations_vfs * fops,const char * buffer,size_t buflen);
extern int filep_poll(struct file * filep,const struct file_operations_vfs * fops,poll_table * fds);
extern int filep_ioctl(struct file * filep,const struct file_operations_vfs * fops,int cmd,unsigned long arg);
extern int get_filep_ops(struct file * filep,struct file * * pst_filep,const struct file_operations_vfs ** pst_filep_ops);
extern BOOL console_enable(void);
extern int update_console_fd(void);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __CONSOLE_H__ */
