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
#ifndef __MEDIA_H__
#define __MEDIA_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */
/*
 * macro define
 */
 /* opp table define */

/* 1080p@30fps profile */
#define HI_VDD_MEDIA_OPP1_MV            1030
#define HI_VDD_MEDIA_OPP1_PROFILE       0
#define HI_VDD_MEDIA_OPP1_AVS_HPM       245
#define HI_VDD_MEDIA_OPP1_AVS_VMIN_MV   930
#define HI_VDD_MEDIA_OPP1_AVS_HPM_DIV   3

/* 3M@30fps profile */
#define HI_VDD_MEDIA_OPP2_MV            1030
#define HI_VDD_MEDIA_OPP2_PROFILE       1
#define HI_VDD_MEDIA_OPP2_AVS_HPM       245
#define HI_VDD_MEDIA_OPP2_AVS_VMIN_MV   930
#define HI_VDD_MEDIA_OPP2_AVS_HPM_DIV   3

/* 1080p@60fps profile */
#define HI_VDD_MEDIA_OPP3_MV            1030
#define HI_VDD_MEDIA_OPP3_PROFILE       2
#define HI_VDD_MEDIA_OPP3_AVS_HPM       245
#define HI_VDD_MEDIA_OPP3_AVS_VMIN_MV   930
#define HI_VDD_MEDIA_OPP3_AVS_HPM_DIV   4

/* 5M@30fps profile */
#define HI_VDD_MEDIA_OPP4_MV            1100
#define HI_VDD_MEDIA_OPP4_PROFILE       3
#define HI_VDD_MEDIA_OPP4_AVS_HPM       260
#define HI_VDD_MEDIA_OPP4_AVS_VMIN_MV   930
#define HI_VDD_MEDIA_OPP4_AVS_HPM_DIV   5

#define MIN_MEDIA_PROFILE HI_VDD_MEDIA_OPP1_PROFILE
#define MAX_MEDIA_PROFILE HI_VDD_MEDIA_OPP4_PROFILE
#define DEFAULT_HW_PROFILE HI_VDD_MEDIA_OPP4_PROFILE

#define MEDIA_VMAX 1310 /*mv*/
#define MEDIA_VMIN 800 /*mv*/

int media_Handler(void);
int media_AvsInit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
