/**
 * @file    hi_product_scene.c
 * @brief   photo picture.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/2
 * @version   1.0

 */
#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "hi_product_scene_specparam.h"
#include "hi_product_scene_setparam.h"
#include "product_scene_setparam_inner.h"
#include "mpi_vpss.h"
#include "mpi_vi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCENE_CHIP */
/** @{ */  /** <!-- [SCENE_CHIP] */

HI_S32 PDT_SCENE_SetVI3DNRParam(HI_HANDLE ModHdl, HI_U8 u8Index, HI_PDT_SCENE_PIPE_PARAM_S* pstSceneParam)
{
    #ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i, j = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 au32Iso[HI_PDT_SCENE_3DNR_MAX_COUNT] = {};
    VI_PIPE_NRX_PARAM_V2_S astNrxParamV2[HI_PDT_SCENE_3DNR_MAX_COUNT] = {};
    VI_PIPE_ATTR_S stViPipeAttr;

    s32Ret = HI_MPI_VI_GetPipeAttr(ModHdl, &stViPipeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_VI_GetPipeAttr failed.");
        return HI_FAILURE;
    }
    stViPipeAttr.bNrEn = HI_TRUE;

    s32Ret = HI_MPI_VI_SetPipeAttr(ModHdl, &stViPipeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_VI_SetPipeAttr failed.");
        return HI_FAILURE;
    }

    VI_PIPE_NRX_PARAM_S stNRX = {0};
    stNRX.stNRXParamV2.stNRXAutoV2.u32ParamNum = pstSceneParam[u8Index].stStatic3DNR.u323DNRCount;
    stNRX.stNRXParamV2.stNRXAutoV2.pau32ISO = au32Iso;
    HI_APPCOMM_CHECK_POINTER(stNRX.stNRXParamV2.stNRXAutoV2.pau32ISO, HI_FAILURE);
    stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2 = astNrxParamV2;
    HI_APPCOMM_CHECK_POINTER(stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2, HI_FAILURE);

    stNRX.enNRVersion = VI_NR_V2;
    stNRX.stNRXParamV2.enOptMode = OPERATION_MODE_AUTO;

    s32Ret = HI_MPI_VI_GetPipeNRXParam(ModHdl, &stNRX);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_VI_GetPipeNRXParam failed.");
        return HI_FAILURE;
    }

    memcpy(stNRX.stNRXParamV2.stNRXAutoV2.pau32ISO, pstSceneParam[u8Index].stStatic3DNR.au323DNRIso,
                    sizeof(HI_U32) * pstSceneParam[u8Index].stStatic3DNR.u323DNRCount);
    stNRX.stNRXParamV2.stNRXAutoV2.u32ParamNum = pstSceneParam[u8Index].stStatic3DNR.u323DNRCount;

    for (i = 0; i < pstSceneParam[u8Index].stStatic3DNR.u323DNRCount; i++)
    {
        memcpy(&(stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i]),
             &(stNRX.stNRXParamV2.stNRXManualV2.stNRXParamV2), sizeof(VI_PIPE_NRX_PARAM_V2_S));
    }

    for (i = 0; i < stNRX.stNRXParamV2.stNRXAutoV2.u32ParamNum; i++)
    {
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].IEy.IEDZ =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].IEy.IEDZ;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].IEy.IES0 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].IEy.IES0;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].IEy.IES1 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].IEy.IES1;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].IEy.IES2 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].IEy.IES2;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].IEy.IES3 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].IEy.IES3;


        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.BWSF4 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.BWSF4;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.DeIdx =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.DeIdx;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.DeRate =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.DeRate;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.JMODE =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.JMODE;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.PBR6 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.PBR6;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SBN6 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SBN6;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SBR1 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SBR1;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SBR2 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SBR2;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SBR4 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SBR4;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFN0 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFN0;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFN1 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFN1;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFN3 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFN3;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFR =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFR;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFS1 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFS1;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFS2 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFS2;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFS4 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFS4;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFT1 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFT1;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFT2 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFT2;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFT4 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFT4;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SPN6 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SPN6;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SRT0 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SRT0;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SRT1 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SRT1;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.STH1 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.STH1;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.STH3 =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.STH3;
        stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.TriTh =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.TriTh;
        for (j = 0; j < 3; j++)
        {
            stNRX.stNRXParamV2.stNRXAutoV2.pastNRXParamV2[i].SFy.SFR6[j] =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVIValue[i].SFy.SFR6[j];
        }
    }

    stNRX.enNRVersion = VI_NR_V2;
    stNRX.stNRXParamV2.enOptMode = OPERATION_MODE_AUTO;
    s32Ret = HI_MPI_VI_SetPipeNRXParam(ModHdl, &stNRX);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_VI_SetPipeNRXParam failed.");
        return HI_FAILURE;
    }
#endif
    return HI_SUCCESS;
}


HI_S32 PDT_SCENE_SetVPSS3DNRParam(HI_HANDLE ModHdl, HI_U8 u8Index, HI_PDT_SCENE_PIPE_PARAM_S* pstSceneParam)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i, j, k = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 au32Iso[HI_PDT_SCENE_3DNR_MAX_COUNT] = {};
    VPSS_NRX_V2_S astNRxParam[HI_PDT_SCENE_3DNR_MAX_COUNT] = {};

    VPSS_GRP_NRX_PARAM_S stNRX;
    stNRX.stNRXParam_V2.stNRXAuto.u32ParamNum = pstSceneParam[u8Index].stStatic3DNR.u323DNRCount;
    stNRX.stNRXParam_V2.stNRXAuto.pau32ISO = au32Iso;
    HI_APPCOMM_CHECK_POINTER(stNRX.stNRXParam_V2.stNRXAuto.pau32ISO, HI_FAILURE);
    stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam = astNRxParam;
    HI_APPCOMM_CHECK_POINTER(stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam, HI_FAILURE);

    stNRX.enNRVer = VPSS_NR_V2;
    stNRX.stNRXParam_V2.enOptMode = OPERATION_MODE_AUTO;

#if 1
    s32Ret = HI_MPI_VPSS_GetGrpNRXParam(ModHdl, &stNRX);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_VPSS_GetGrpNRXParam failed.");
        return HI_FAILURE;
    }
#endif

    for (i = 0; i < pstSceneParam[u8Index].stStatic3DNR.u323DNRCount; i++)
    {
        memcpy(&(stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i]),
             &(stNRX.stNRXParam_V2.stNRXManual.stNRXParam), sizeof(VPSS_NRX_V2_S));
    }

    memcpy(stNRX.stNRXParam_V2.stNRXAuto.pau32ISO, pstSceneParam[u8Index].stStatic3DNR.au323DNRIso,
                    sizeof(HI_U32) * pstSceneParam[u8Index].stStatic3DNR.u323DNRCount);
    stNRX.stNRXParam_V2.stNRXAuto.u32ParamNum = pstSceneParam[u8Index].stStatic3DNR.u323DNRCount;

#if 1
    for (i = 0; i < stNRX.stNRXParam_V2.stNRXAuto.u32ParamNum; i++)
    {
        for (j = 0; j < 3; j++)
        {
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].IEy[j].IEDZ =
               pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].IEy[j].IEDZ;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].IEy[j].IES0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].IEy[j].IES0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].IEy[j].IES1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].IEy[j].IES1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].IEy[j].IES2 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].IEy[j].IES2;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].IEy[j].IES3 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].IEy[j].IES3;
        }

        for (j = 0; j < 2; j++)
        {
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].biPath =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].biPath;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MABW0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MABW0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MABW1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MABW1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MABR0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MABR0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MABR1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MABR1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MADZ0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MADZ0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MADZ1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MADZ1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MAI00 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MAI00;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MAI01 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MAI01;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MAI02 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MAI02;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MAI10 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MAI10;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MAI11 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MAI11;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MAI12 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MAI12;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MASW =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MASW;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MATE0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MATE0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MATE1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MATE1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MATH0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MATH0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MATH1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MATH1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].MDy[j].MATW =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].MDy[j].MATW;
        }

        for (j = 0; j < 3; j++)
        {
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[0].BWSF4 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[0].BWSF4;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].JMODE =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].JMODE;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].kMode =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].kMode;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].NRyEn =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].NRyEn;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].PBR6 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].PBR6;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SBN6 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SBN6;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SBR1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SBR1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SBR2 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SBR2;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SBR4 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SBR4;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFN0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFN0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFN1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFN1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFN3 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFN3;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFR =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFR;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFS1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFS1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFS2 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFS2;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFS4 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFS4;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFT1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFT1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFT2 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFT2;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFT4 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFT4;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SPN6 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SPN6;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].STH1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].STH1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].STH3 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].STH3;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].TriTh =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].TriTh;

            for (k = 0; k < 3; k++)
            {
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].SFy[j].SFR6[k] =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].SFy[j].SFR6[k];
            }
        }

        stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].pNRc.CTFS =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].pNRc.CTFS;
        stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].pNRc.SFC =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].pNRc.SFC;
        stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].pNRc.TFC =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].pNRc.TFC;
        stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].RFs.advMATH =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].RFs.advMATH;
        stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].RFs.RFDZ =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].RFs.RFDZ;
        stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].RFs.RFSLP =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].RFs.RFSLP;
        stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].RFs.RFUI =
            pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].RFs.RFUI;

        for (j = 0; j < 2; j++)
        {
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].bRef =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].bRef;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].DZMode0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].DZMode0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].DZMode1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].DZMode1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].RFI =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].RFI;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].SDZ0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].SDZ0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].SDZ1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].SDZ1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].STR0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].STR0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].STR1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].STR1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TDX0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TDX0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TDX1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TDX1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TDZ0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TDZ0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TDZ1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TDZ1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].tEdge =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].tEdge;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TFS0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TFS0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TFS1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TFS1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TSI0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TSI0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TSI1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TSI1;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TSS0 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TSS0;
            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TSS1 =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TSS1;

            for (k = 0; k < 6; k++)
            {
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TFR0[k] =
                    pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TFR0[k];
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].TFy[j].TFR1[k] =
                    pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].TFy[j].TFR1[k];
            }

            stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.NRcEn =
                pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.NRcEn;
            if (stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.NRcEn)
            {
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFS1 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFS1;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFS2 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFS2;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFS4 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFS4;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFT1 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFT1;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFT2 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFT2;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFT4 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFT4;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SBR1 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SBR1;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SBR2 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SBR2;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SBR4 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SBR4;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.BWSF4 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.BWSF4;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SPN6 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SPN6;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SBN6 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SBN6;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.PBR6 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.PBR6;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.JMODE =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.JMODE;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFR6[0] =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFR6[0];
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFR6[1] =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFR6[1];
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFR6[2] =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFR6[2];
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SRT0 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SRT0;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SRT1 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SRT1;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.DeRate =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.DeRate;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.DeIdx =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.DeIdx;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.TriTh =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.TriTh;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFN0 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFN0;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFN1 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFN1;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFN3 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFN3;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.STH1 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.STH1;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.STH3 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.STH3;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.SFy.SFR =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.SFy.SFR;

                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.IEy.IES0 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.IEy.IES0;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.IEy.IES1 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.IEy.IES1;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.IEy.IES2 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.IEy.IES2;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.IEy.IES3 =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.IEy.IES3;
                stNRX.stNRXParam_V2.stNRXAuto.pastNRXParam[i].NRc.IEy.IEDZ =
                     pstSceneParam[u8Index].stStatic3DNR.ast3DNRVPSSValue[i].NRc.IEy.IEDZ;
            }
        }
    }
#endif

    stNRX.enNRVer = VPSS_NR_V2;
    stNRX.stNRXParam_V2.enOptMode = OPERATION_MODE_AUTO;
    s32Ret = HI_MPI_VPSS_SetGrpNRXParam(ModHdl, &stNRX);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_VPSS_SetGrpNRXParam failed.");
        return HI_FAILURE;
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStatic3DNR(const HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    HI_S32 i = 0;
    HI_S32 s32Ret = 0;


    HI_PDT_SCENE_PIPE_PARAM_S* pstSceneParam = PDT_SCENE_GetParam();

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != pstSceneMode->astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetVI3DNRParam(pstSceneMode->astPipeAttr[i].VcapPipeHdl,
            pstSceneMode->astPipeAttr[i].u8PipeParamIndex, pstSceneParam);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != pstSceneMode->astPipeAttr[i].bEnable)
        {
            continue;
        }

        if (HI_TRUE == pstSceneMode->astPipeAttr[i].bBypassVpss)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetVPSS3DNRParam(pstSceneMode->astPipeAttr[i].VpssHdl,
            pstSceneMode->astPipeAttr[i].u8PipeParamIndex, pstSceneParam);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    return HI_SUCCESS;
}

/** @}*/  /** <!-- ==== SCENE_CHIP End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
