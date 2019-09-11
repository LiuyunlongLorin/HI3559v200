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

#ifndef _KERNEL_LOS_LD_MACRO_INC
#define _KERNEL_LOS_LD_MACRO_INC

#include "los_ld_type.inc"

#include "stdarg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#define GET_MODULE_BY_ID(uwModuleId)    ((DYN_MODULE_INFO *)&stSysModuleInfo[(uwModuleId)])

/* symLib.c */
#define LD_SYM_HFUNC_SEED   1370364821  /* Magic seed */

/* Macro to check if the alignment is a power of 2 */
#define LD_CHECK_2_ALIGN(x) (((x) & ((x) - 1)) == 0)

/* New load flags */
#define LD_NO_SYMBOLS       0x1     /* No symbols added to the target
                                       symbol table
                                       */
#define LD_LOCAL_SYMBOLS    0x2     /* Only internal symbols added to
                                       the target symbol table
                                       */
#define LD_GLOBAL_SYMBOLS   0x4     /* Only external symbols added to
                                       the target symbol table
                                       */
#define LD_ALL_SYMBOLS      (LD_LOCAL_SYMBOLS | LD_GLOBAL_SYMBOLS)  /* All symbols added to the target
                                                                       symbol table
                                                                       */

/* Symbol types */
#define LD_SYM_UNDF     0x0     /* Undefined */
#define LD_SYM_LOCAL    0x0     /* Local */
#define LD_SYM_GLOBAL   0x1     /* Global (external) (ORed) */
#define LD_SYM_ABS      0x2     /* Absolute */
#define LD_SYM_TEXT     0x4     /* Text */
#define LD_SYM_DATA     0x6     /* Data */
#define LD_SYM_BSS      0x8     /* Bss */
#define LD_SYM_COMM     0x10    /* Common symbol */

#define LD_SYM_BAL_ENTRY    0x20    /* I80960 BAL entry point to function */
#define LD_SYM_SDA          0x40    /* Symbols related to a PowerPC SDA section */
#define LD_SYM_SDA2         0x80    /* Symbols related to a PowerPC SDA2 section */

#define LD_SYM_THUMB        0x40    /* Thumb function - unlikely clash with SYM_SDA */

#define LD_SYM_MIPS16       0x80    /* Mips16 function - unlikely clash with SYM_SDA2, arch. specific */

#define LD_SeekAddr(swFd, swOffset) ((swFd) + (swOffset))

#ifndef _ALLOC_ALIGN_SIZE
#define _ALLOC_ALIGN_SIZE   4       /* 4 byte boundary */
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _KERNEL_LOS_LD_MACRO_INC */
