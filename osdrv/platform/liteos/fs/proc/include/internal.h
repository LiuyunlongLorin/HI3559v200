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

#ifndef __LITEOS_PROC_INTERNAL_H__
#define __LITEOS_PROC_INTERNAL_H__

#include "proc_fs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

extern spinlock_t procfs_subdir_lock;
#define	MAX_NON_LFS	((1UL<<31) - 1)

int proc_match(unsigned int len, const char *name, struct proc_dir_entry *pde);
struct proc_dir_entry * proc_find_entry(const char *pcPath);
void pde_put(struct proc_dir_entry *pde);

extern int proc_stat(const char *file, struct proc_stat *buf);
//Interface for Client(Shell);
extern void proc_mounts_init(void);

/**
 * @ingroup  procfs
 * @brief open a proc node
 *
 * @par Description:
 * This API is used to open the  node by 'filename' and flags,
 *
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  filename [IN] Type #const char * the filename of the node to  be opened
 * @param  flags [IN] Type  #int the flags of open's node
 *
 * @retval #NULL                open failed
 * @retval #NOT NULL            open successfully
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see proc_close
 *
 * @since Huawei LiteOS V100R001C00
 */
extern struct proc_dir_entry *proc_open(const char *filename, int flags, ...);

/**
 * @ingroup  procfs
 * @brief close a proc node
 *
 * @par Description:
 * This API is used to close the node by pHandle
 *
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  pHandle [IN] Type #struct proc_dir_entry * pointer of the node structure to be closed
 *
 * @retval #-1                close failed
 * @retval #0                 close successfully
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see proc_open
 *
 * @since Huawei LiteOS V100R001C00
 */
extern int proc_close(struct proc_dir_entry * pHandle);

/**
 * @ingroup  procfs
 * @brief read a proc node
 *
 * @par Description:
 * This API is used to read the node by pHandle
 *
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  pHandle [IN] Type #struct proc_dir_entry * pointer of the node structure to be read
 * @param  buf [IN] Type #void *  user-provided to save the data
 * @param  len [IN] Type #size_t  the length of want to read
 *
 * @retval #-1                read failed
 * @retval #>0                Number of bytes read success
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see proc_open
 *
 * @since Huawei LiteOS V100R001C00
 */
extern int proc_read(struct proc_dir_entry * pHandle, void *buf, size_t len);

/**
 * @ingroup  procfs
 * @brief write a proc node
 *
 * @par Description:
 * This API is used to write the node by pHandle
 *
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  pHandle [IN] Type #struct proc_dir_entry * pointer of the node structure to be wrote
 * @param  buf [IN] Type #const void *    data to write
 * @param  len [IN] Type #size_t    length of data to write
 *
 * @retval #-1                write failed
 * @retval #>0                Number of bytes write successfully
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see proc_open
 *
 * @since Huawei LiteOS V100R001C00
 */
extern int proc_write(struct proc_dir_entry * pHandle, const void *buf, size_t len);

/**
 * @ingroup  procfs
 * @brief File migration
 *
 * @par Description:
 * This API is used to set the proc file migration
 *
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  pHandle [IN] Type #struct proc_dir_entry *   pointer of the node structure to be deviation
 * @param  offset [IN] Type #loff_t    the number of deviation
 * @param  whence [IN] Type #int       the begin of  deviation
 *
 * @retval #<0                deviation failed
 * @retval #>=0               deviation successfully
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see proc_open
 *
 * @since Huawei LiteOS V100R001C00
 */
extern loff_t proc_lseek(struct proc_dir_entry * pHandle, loff_t offset, int whence);

/**
 * @ingroup  procfs
 * @brief directory migration
 *
 * @par Description:
 * This API is used to set the proc directory migration
 *
 * @attention
 * <ul>
 * <li>Only allow SEEK_SET to zero.</li>
 * </ul>
 *
 * @param  pHandle [IN] Type #struct proc_dir_entry *   pointer of the node structure to be deviated
 * @param  pos [IN] Type #off_t *      the number of deviation
 * @param  whence [IN] Type #int       the begin of deviation
 *
 * @retval #EINVAL            deviation failed
 * @retval #ENOERR            deviation successfully
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see proc_open
 *
 * @since Huawei LiteOS V100R001C00
 */
int proc_dirlseek(struct proc_dir_entry *pHandle, off_t * pos, int whence);


/* * This is the "filldir" function type, used by readdir() to let
* the kernel specify what kind of dirent layout it wants to have.
* This allows the kernel to read directories into kernel space or
* to have different dirent layouts depending on the binary type. */
typedef int (*filldir_t)(void *, const char *, int, loff_t, unsigned long long, unsigned);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
