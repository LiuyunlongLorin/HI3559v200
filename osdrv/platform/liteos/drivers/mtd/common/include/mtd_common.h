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

#ifndef __MTD_COMMON_H__
#define __MTD_COMMON_H__

#include "stdint.h"

/*---------------------------------------------------------------------------*/
/* base type macros */
/*---------------------------------------------------------------------------*/
#ifndef u16
#define u16         unsigned short
#endif
#ifndef u32
#define u32         unsigned int
#endif
#ifndef ulong
#define ulong       unsigned long
#endif

/*---------------------------------------------------------------------------*/
/* frequently-used macros */
/*---------------------------------------------------------------------------*/
#ifndef min
#define min(x,y) (x<y?x:y)
#endif
#ifndef max
#define max(x,y) (x<y?y:x)
#endif
#ifndef min_t
#define min_t(t, x,y) ((t)x<(t)y?(t)x:(t)y)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef likely
#define likely(x)       (x)
#endif
#ifndef unlikely
#define unlikely(x)     (x)
#endif

/*---------------------------------------------------------------------------*/
/* mtd device capacity mcaros */
/*---------------------------------------------------------------------------*/
#define _256B       (256)
#define _512B       (512)
#define _1K         (1024)
#define _2K         (2048)
#define _4K         (4096)
#define _8K         (8192)
#define _16K        (16384)
#define _32K        (32768)
#define _64K        (0x10000UL)
#define _128K       (0x20000UL)
#define _256K       (0x40000UL)
#define _512K       (0x80000UL)
#define _1M         (0x100000UL)
#define _2M         (0x200000UL)
#define _4M         (0x400000UL)
#define _8M         (0x800000UL)
#define _16M        (0x1000000UL)
#define _32M        (0x2000000UL)
#define _64M        (0x4000000UL)
#define _128M       (0x8000000UL)
#define _256M       (0x10000000UL)
#define _512M       (0x20000000UL)
#define _1G         (0x40000000ULL)
#define _2G         (0x80000000ULL)
#define _4G         (0x100000000ULL)
#define _8G         (0x200000000ULL)
#define _16G        (0x400000000ULL)
#define _64G        (0x1000000000ULL)
#define INFINITE    (0xFFFFFFFF)
/*---------------------------------------------------------------------------*/
/* mtd device print control mcaros */
/*---------------------------------------------------------------------------*/
#include "los_printf.h"
#include "asm/io.h"

#define DISABLE     0
#define ENABLE      1

#define READ        0
#define WRITE       1

#define MTD_REG_DEBUG DISABLE
//#define MTD_REG_DEBUG ENABLE

#define mtd_trace(debug, msg...) do { \
    if (debug == ENABLE) { \
        dprintf("%s:%d: ", __func__, __LINE__); \
        dprintf(msg); \
        dprintf("\n"); \
    } \
} while (0)

#define mtd_readl(addr) ({unsigned int reg = readl((AARCHPTR)addr); \
        mtd_trace(MTD_REG_DEBUG, "readl(0x%p) = 0x%08X", (AARCHPTR)addr, reg); \
        reg; })

#define mtd_writel(v, addr) do { \
    writel(v, (AARCHPTR)addr); \
    mtd_trace(MTD_REG_DEBUG, "writel(0x%p) = 0x%08X",\
            (AARCHPTR)addr, (unsigned int)(v)); \
} while (0)

/*****************************************************************************/
#define INIT_DBG      0        /* Init  debug print */
#define ER_DBG        0        /* Erase debug print */
#define WR_DBG        0        /* Write debug print */
#define RD_DBG        0        /* Read  debug print */

#define DB_BUG(fmt, args...) \
    do { \
        dprintf("%s(%d): BUG: " fmt, __FILE__, __LINE__, ##args); \
        __asm("b ."); \
    } while (0)

#define DBG_MSG(_fmt, arg...) \
    dprintf("%s(%d): " _fmt, __func__, __LINE__, ##arg);

#define ERR_MSG(_fmt, arg...) \
    dprintf("%s(%d): Error:" _fmt, __func__, __LINE__, ##arg);

#define WARN_MSG(_fmt, arg...) \
    dprintf("%s(%d): Warning:" _fmt, __func__, __LINE__, ##arg);

#define INFO_MSG(_fmt, arg...) \
    dprintf(_fmt, ##arg);

#define MTD_PR(_type, _fmt, arg...) \
    do { \
        if (_type) \
            DBG_MSG(_fmt, ##arg) \
    } while (0)

/* function and variable declaration */

char *ulltostr(unsigned long long size);
int ffs(int x);
void mtd_dma_cache_inv(void *addr, unsigned int size);
void mtd_dma_cache_clean(void *addr, unsigned int size);

extern const struct file_operations_vfs g_mtdchar_ops;
extern const struct block_operations g_mtdblock_ops;

#endif /* End of __MTD_COMMON_H__ */

