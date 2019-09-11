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


#ifndef    __HISOC_CPU_H__
#define    __HISOC_CPU_H__

#include "asm/platform.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */



#define ARM_GIC_CPU_CTRL                    0x00
#define ARM_GIC_CPU_PRIMASK                 0x04
#define ARM_GIC_CPU_BINPOINT                0x08
#define ARM_GIC_CPU_INTACK                  0x0c
#define ARM_GIC_CPU_EOI                     0x10
#define ARM_GIC_CPU_RUNNINGPRI              0x14
#define ARM_GIC_CPU_HIGHPRI                 0x18

#define ARM_GIC_DIST_CTRL                   0x000
#define ARM_GIC_DIST_CTR                    0x004
#define ARM_GIC_DIST_ENABLE_SET             0x100
#define ARM_GIC_DIST_ENABLE_CLEAR           0x180
#define ARM_GIC_DIST_PENDING_SET            0x200
#define ARM_GIC_DIST_PENDING_CLEAR          0x280
#define ARM_GIC_DIST_ACTIVE_BIT             0x300
#define ARM_GIC_DIST_PRI                    0x400
#define ARM_GIC_DIST_TARGET                 0x800
#define ARM_GIC_DIST_CONFIG                 0xc00
#define ARM_GIC_DIST_SOFTINT                0xf00


#define REG_A7_PERI_GIC_DIST                0x1000
#define REG_A7_PERI_GIC_CPU                 0x2000
#define REG_BASE_A7_PERI                    ARM_REG_BASE
#define CFG_ARM_GIC_CPU_BASE                (IO_ADDRESS(REG_BASE_A7_PERI) + REG_A7_PERI_GIC_CPU)
#define CFG_ARM_GIC_DIST_BASE               (IO_ADDRESS(REG_BASE_A7_PERI) + REG_A7_PERI_GIC_DIST)
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

