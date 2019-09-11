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

#ifndef __NAND_H__
#define __NAND_H__

#include "sys/types.h"
#include "linux/mtd/mtd.h"

int nand_init(void);

int nand_wakeup_lockresume(void);

int nand_write_yaffs_skip_bad(struct mtd_info *mtd, loff_t offset, size_t *length,
        const char *buffer);

int nand_read_yaffs_skip_bad(struct mtd_info *mtd, loff_t offset, size_t *length,
    const char *buffer);

int nand_write_skip_bad(struct mtd_info *mtd, loff_t offset, size_t *length,
        const char *buffer);

int nand_read_skip_bad(struct mtd_info *mtd, loff_t offset, size_t *length,
        const char *buffer);

int hinand_erase(unsigned long start, unsigned long size);

int hinand_write(void* memaddr, unsigned long start, unsigned long size);

int hinand_read(void* memaddr, unsigned long start, unsigned long size);

int hinand_yaffs_write(void* memaddr, unsigned long start, unsigned long size);

int hinand_yaffs_read(void* memaddr, unsigned long start, unsigned long size);

loff_t hinand_addr_cal(loff_t addr, size_t size);

int hinand_yaffs_nand_block_isbad(loff_t ofs);

int hinand_yaffs_nand_block_markbad(loff_t ofs);

extern struct mtd_info *nand_mtd;

#endif /* End of __NAND_H__ */

