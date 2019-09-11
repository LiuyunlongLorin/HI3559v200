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

/** @defgroup filesystem FileSystem
 *  @defgroup mtd_partition Multi Partition
 *  @ingroup filesystem
*/
#ifndef _MTD_PARTITION_H
#define _MTD_PARTITION_H

#include "sys/types.h"
#include "los_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct mtd_node{
    unsigned start_block;
    unsigned end_block;
    unsigned patitionnum;
    char *blockdriver_name;
    char *chardriver_name;
    char *mountpoint_name;
    void * mtd_info;//Driver used by a partition
    LOS_DL_LIST node_info;
    UINT32 lock;
    unsigned int user_num;
} mtd_partition;

typedef struct par_param{
    mtd_partition *partition_head;
    struct mtd_info *flash_mtd;
    struct block_operations *flash_ops;
    const struct file_operations_vfs *char_ops;
    char *blockname;
    char *charname;
    unsigned int block_size;
}partition_param;

#define CONFIG_MTD_PATTITION_NUM 20

#define CHECK_ALIGN(len, saddr, sblk, eblk, par_end, blk_size) \
do{ \
    if(len == 0 || len < blk_size || (saddr + len) > par_end) \
        return -EINVAL; \
    len = (((len) + (blk_size) - 1) & ~((blk_size) - 1)); \
    saddr = ((saddr) & ~((blk_size) - 1)); \
    sblk = saddr /blk_size; \
    eblk = len/blk_size + sblk - 1; \
}while(0)
#define PAR_ASSIGNMENT(node, len, saddr, num, mtd, blk_size) \
do{ \
    node->start_block = saddr/blk_size; \
    node->end_block = len/blk_size + node->start_block - 1; \
    node->patitionnum = num; \
    node->mtd_info = mtd; \
    node->mountpoint_name = (char *)NULL; \
}while(0)

/**
 *@ingroup mtd_partition
 *@brief Add a partition.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to add a partition according to the passed-in parameters.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param type           [IN] Storage medium type, support "nand" and "spinor" currently.
 *@param start_addr     [IN] Starting address of a partition.
 *@param length         [IN] Partition size.
 *@param partition_num  [IN] Partition number, less than the value defined by CONFIG_MTD_PATTITION_NUM.
 *
 *@retval #-ENODEV      The driver is not found.
 *@retval #-EINVAL      Invalid parameter.
 *@retval #-ENOMEM      Insufficient memory.
 *@retval #ENOERR       The partition is successfully created.
 *
 *@par Dependency:
 *<ul><li>mtd_partition.h: the header file that contains the API declaration.</li></ul>
 *@see check_mtd_pattition | delete_mtd_partition
 *@since Huawei LiteOS V100R001C00
 */
extern int add_mtd_partition( char *type, UINT32 start_addr, UINT32 length, UINT32 partition_num);

/**
 *@ingroup mtd_partition
 *@brief Delete a partition.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to delete a partition according to its partition number and storage medium type.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param partition_num  [IN] Partition number, less than the value defined by CONFIG_MTD_PATTITION_NUM.
 *@param type           [IN] Storage medium type, support "nand" and "spinor" currently.
 *
 *@retval #-EINVAL    Invalid parameter.
 *@retval #ENOERR     The partition is successfully deleted.
 *
 *@par Dependency:
 *<ul><li>mtd_partition.hthe header file that contains the API declaration.</li></ul>
 *@see add_mtd_partition
 *@since Huawei LiteOS V100R001C00
 */
extern int delete_mtd_partition(UINT32 partition_num, char *type);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /*_MTD_PARTITION_H */
