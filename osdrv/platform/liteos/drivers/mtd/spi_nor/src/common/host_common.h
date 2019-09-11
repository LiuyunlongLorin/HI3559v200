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

#ifndef __HOST_COMMON_H__
#define __HOST_COMMON_H__

#include "los_mux.h"

#include "mtd_common.h"
#include "spinor_common.h"
#include "spi_common.h"

#define reg_read(_host, _reg) \
        mtd_readl((AARCHPTR)((char *)_host->regbase + (_reg)))

#define reg_write(_host, _value, _reg) \
        mtd_writel((unsigned)(_value), (AARCHPTR)((char *)_host->regbase + (_reg)))

#define get_host(_host) \
        if(LOS_OK != LOS_MuxPend(_host->lock, LOS_WAIT_FOREVER)) \
            return -1;

#define put_host(_host) \
        if(LOS_OK != LOS_MuxPost(_host->lock)) \
            return -1;
struct spinor_host {
    struct spinor_info *spinor;

    char     *regbase;
    char     *membase;

    void (*set_system_clock)(unsigned clock, int clk_en);
    void (*set_host_addr_mode)(struct spinor_host *host, int enable);

    char *buffer;
    char *dma_buffer;
    char *dma_buffer_bak;

    int num_chip;
    struct spi spi[1];

    UINT32 lock;
};

#endif /* End of __HOST_COMMON_H__ */

