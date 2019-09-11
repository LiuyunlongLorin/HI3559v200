/****************************************************************************
 * include/debug.h
 *
 *   Copyright (C) 2007-2011, 2014 Gregory Nutt. All rights reserved.
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

#ifndef _VFS_CONFIG_H_
#define _VFS_CONFIG_H_

#include "los_config.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define CONFIG_DISABLE_MQUEUE   //disable posix mqueue inode configure

/*file system configur*/
//#define LOSCFG_FS_FAT           //enable fat file system
//#define LOSCFG_FS_YAFFS         //enable yaffs file system
//#define LOSCFG_FS_JFFS          //enable jffs file system
//#define LOSCFG_FS_RAMFS         //enable ramfs file system
//#define LOSCFG_FS_PROC        //enable procfs file system
//#define LOSCFG_FS_NFS              //enable nfs file system
#define CONFIG_FS_WRITABLE      //enable file system can be written
#define CONFIG_FS_READABLE      //enable file system can be read
#define CONFIG_DEBUG_FS         //enable vfs debug function


/*yaffs and fatfs cache configur*/
/*config block size for fat file system, only can be 0,32,64,128,256,512,1024*/
#define CONFIG_FS_FAT_SECTOR_PER_BLOCK  (256)
/*config block num for fat file system*/
#define CONFIG_FS_FAT_BLOCK_NUMS (16)
/*enable function that vfs can write something in connected multi block*/
//#if CONFIG_FS_FAT_SECTOR_PER_BLOCK <= 256
//#define CONFIG_FS_BLIB_WRITE_MULTBLOCK
//#endif


#define CONFIG_FS_FLASH_BLOCK_NUM 1
/*page numbers of per yaffs block*/
#define CONFIG_FS_YLIB_PAGE_PER_BLOCK (64 * CONFIG_FS_FLASH_BLOCK_NUM)
/*config erase size for yaffs file system*/
#define CONFIG_FS_YAFFS_BLOCK_ERASE_SIZE (2048 * CONFIG_FS_YLIB_PAGE_PER_BLOCK)
/*config block size for yaffs file system*/
#define CONFIG_FS_YAFFS_BLOCK_SIZE  ((2048 + 64) * CONFIG_FS_YLIB_PAGE_PER_BLOCK)
/*config cache size for yaffs file system*/
#define CONFIG_FS_YAFFS_BLOCK_CACHE_MEMSIZE (16*CONFIG_FS_YAFFS_BLOCK_SIZE)


/*nfs configure*/
#define CONFIG_NFS_MACHINE_NAME "IPC"   //nfs device name is IPC
#define CONFIG_NFS_MACHINE_NAME_SIZE 3  //size of nfs machine name


/*file descriptors configure*/
#define CONFIG_NFILE_STREAMS        1   //enable file stream
#define CONFIG_STDIO_BUFFER_SIZE    0
#define CONFIG_NUNGET_CHARS         0

#define FD_SETSIZE  512  //total fds
/*net configure*/
#ifdef LOSCFG_NET_LWIP_SACK             //enable socket and net function
extern unsigned int g_lwip_num_sockets;
#define CONFIG_NSOCKET_DESCRIPTORS ((int)g_lwip_num_sockets)   //max numbers of socket descriptor
/*max numbers of other descriptors except socket descriptors*/
#define CONFIG_NFILE_DESCRIPTORS    (256)
#define CONFIG_NET_SENDFILE         1   //enable sendfile function
#define CONFIG_NET_TCP              1   //enable sendfile and send function
#else
#define CONFIG_NSOCKET_DESCRIPTORS 0
#define CONFIG_NFILE_DESCRIPTORS    256
#define CONFIG_NET_SENDFILE         0   //disable sendfile function
#define CONFIG_NET_TCP              0   //disable sendfile and send function
#endif

/*directory configure*/
#define VFS_USING_WORKDIR               //enable current working directory

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
