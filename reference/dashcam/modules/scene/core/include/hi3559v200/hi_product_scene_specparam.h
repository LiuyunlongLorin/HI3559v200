/**
* @file    hi_product_scene_chip.h
* @brief   scene module struct and interface declaration
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date       2017/12/13
* @version   1.0

*/

#ifndef _HI_PRODUCT_SCENE_CHIP_H_
#define _HI_PRODUCT_SCENE_CHIP_H_

#include "hi_appcomm.h"
#include "hi_product_scene_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCENE_CHIP */
/** @{ */  /** <!-- [SCENE_CHIP] */

#define HI_PDT_SCENE_3DNR_MAX_COUNT              (12)

typedef struct hiSCENE_3DNR_VI_IEy
{
    HI_S32 IES0, IES1, IES2, IES3;
    HI_S32 IEDZ;
} HI_SCENE_3DNR_VI_IEy;

typedef struct hiSCENE_3DNR_VI_SFy
{
    HI_S32 SPN6, SFR;
    HI_S32 SBN6, PBR6;
    HI_S32 SRT0, SRT1, JMODE, DeIdx;
    HI_S32 DeRate, SFR6[3];

    HI_S32 SFS1, SFT1, SBR1;
    HI_S32 SFS2, SFT2, SBR2;
    HI_S32 SFS4, SFT4, SBR4;

    HI_S32 STH1,  SFN1, SFN0, NRyEn;
    HI_S32 STH2,  SFN2, BWSF4, kMode;
    HI_S32 STH3,  SFN3, TriTh;
} HI_SCENE_3DNR_VI_SFy;

typedef struct hiSCENE_3DNR_VI_S
{
    HI_SCENE_3DNR_VI_IEy  IEy;
    HI_SCENE_3DNR_VI_SFy  SFy;
} HI_SCENE_3DNR_VI_S;


typedef struct hiSCENE_3DNR_VPSS_IEy
{
    HI_S32 IES0, IES1, IES2, IES3;
    HI_S32 IEDZ;
} HI_SCENE_3DNR_VPSS_IEy;

typedef struct hiSCENE_3DNR_VPSS_SFy
{
    HI_S32 SPN6, SFR;
    HI_S32 SBN6, PBR6;
    HI_S32 SRT0, SRT1, JMODE, DeIdx;
    HI_S32 DeRate, SFR6[3];

    HI_S32 SFS1, SFT1, SBR1;
    HI_S32 SFS2, SFT2, SBR2;
    HI_S32 SFS4, SFT4, SBR4;

    HI_S32 STH1,  SFN1, SFN0, NRyEn;
    HI_S32 STH2,  SFN2, BWSF4, kMode;
    HI_S32 STH3,  SFN3, TriTh;

    HI_S32 SBSk[32], SDSk[32];
} HI_SCENE_3DNR_VPSS_SFy;

typedef struct hiSCENE_3DNR_VPSS_MDy
{
    HI_S32 MADZ0,  MAI00, MAI01, MAI02, biPath;
    HI_S32 MADZ1, MAI10, MAI11, MAI12;
    HI_S32 MABR0, MABR1;

    HI_S32 MATH0,  MATE0,  MATW;
    HI_S32 MATH1,  MATE1;
    HI_S32 MASW;
    HI_S32 MABW0,  MABW1;
} HI_SCENE_3DNR_VPSS_MDy;

typedef struct hiSCENE_3DNR_VPSS_TFy
{
    HI_S32 TFS0, TDZ0, TDX0;
    HI_S32 TFS1, TDZ1, TDX1;
    HI_S32 SDZ0, STR0, DZMode0;
    HI_S32 SDZ1, STR1, DZMode1;

    HI_S32 TFR0[6], TSS0,  TSI0;
    HI_S32 TFR1[6], TSS1,  TSI1;

    HI_S32 RFI, tEdge;
    HI_S32 bRef;
} HI_SCENE_3DNR_VPSS_TFy;

typedef struct hiSCENE_3DNR_VPSS_RFs
{
    HI_S32 advMATH, RFDZ;
    HI_S32 RFUI, RFSLP, bRFU;
} HI_SCENE_3DNR_VPSS_RFs;

typedef struct hiSCENE_3DNR_VPSS_pNRc
{
    HI_S32 SFC, TFC;
    HI_S32 CTFS;
} HI_SCENE_3DNR_VPSS_pNRc;

typedef struct hiSCENE_3DNR_VPSS_NRc
{
    HI_SCENE_3DNR_VPSS_IEy  IEy;
    HI_SCENE_3DNR_VPSS_SFy  SFy;
    HI_S32 NRcEn;
} HI_SCENE_3DNR_VPSS_NRc;

typedef struct hiSCENE_3DNR_VPSS_GMC
{
    HI_S32 GMEMode;
} HI_SCENE_3DNR_VPSS_GMC;

typedef struct hiSCENE_3DNR_VPSS_S
{
    HI_SCENE_3DNR_VPSS_IEy  IEy[3];
    HI_SCENE_3DNR_VPSS_GMC  GMC;
    HI_SCENE_3DNR_VPSS_SFy  SFy[3];
    HI_SCENE_3DNR_VPSS_MDy  MDy[2];
    HI_SCENE_3DNR_VPSS_RFs  RFs;
    HI_SCENE_3DNR_VPSS_TFy  TFy[2];
    HI_SCENE_3DNR_VPSS_pNRc pNRc;
    HI_SCENE_3DNR_VPSS_NRc  NRc;
} HI_SCENE_3DNR_VPSS_S;

typedef struct hiPDT_SCENE_STATIC_3DNR_S
{
    HI_U32 u323DNRCount;
    HI_U32 au323DNRIso[HI_PDT_SCENE_3DNR_MAX_COUNT];
    HI_SCENE_3DNR_VI_S ast3DNRVIValue[HI_PDT_SCENE_3DNR_MAX_COUNT];
    HI_SCENE_3DNR_VPSS_S ast3DNRVPSSValue[HI_PDT_SCENE_3DNR_MAX_COUNT];
} HI_PDT_SCENE_STATIC_3DNR_S;

/** @}*/  /** <!-- ==== SCENE_CHIP End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* End of #ifndef __HI_SCENE_H__ */
