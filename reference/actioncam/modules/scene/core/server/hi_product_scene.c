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
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include "product_scene_inner.h"
#include "product_scene_setparam_inner.h"

#include "hi_appcomm.h"
#include "mpi_vpss.h"
#include "mpi_awb.h"
#include "mpi_ae.h"
#include "mpi_isp.h"

#include "hi_product_scene.h"

#if (!defined(CONFIG_DEBUG) || !defined(AMP_LINUX_HUAWEILITE))
#include "sceneparam.dat"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCENE */
/** @{ */  /** <!-- [SCENE] */
/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "PDT_SCENE_SERVER"

#if (defined(CONFIG_DEBUG) && defined(AMP_LINUX_HUAWEILITE))
#define SCENEAUTO_PARAM_PATH HI_SHAREFS_ROOT_PATH"/sceneparam.bin"
#endif


typedef struct tagPDT_SCENE_CTX_S
{
    PDT_SCENE_STATE_S stSceneState;                     /**notes scene state*/
    HI_PDT_SCENE_MODE_S stSceneMode;                    /**notes scene mode*/
    PDT_SCENE_PARAM_S* pstSceneParam;                /**scene param address*/
    pthread_mutex_t SceneModuleMutex;                   /**module mutex*/
} PDT_SCENE_CTX_S;
static PDT_SCENE_CTX_S s_stSceneCTX =
{
    .SceneModuleMutex = PTHREAD_MUTEX_INITIALIZER,
};

/**use to check if the module init*/
#define PDT_SCENE_CHECK_INIT()\
    do{\
        HI_MUTEX_LOCK(s_stSceneCTX.SceneModuleMutex);\
        if (HI_FALSE == s_stSceneCTX.stSceneState.bSceneInit)\
        {\
            MLOGE("please init sceneauto first!\n");\
            HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);\
            return HI_PDT_SCENE_ENOTINIT;\
        }\
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);\
    }while(0);

/**use to check if scene was pause*/
#define PDT_SCENE_CHECK_PAUSE()\
    do{\
        if(HI_TRUE == s_stSceneCTX.stSceneState.bPause)\
        {\
            return HI_SUCCESS;\
        }\
    }while(0);

/**-------------------------internal function interface------------------------- */
static HI_S32 PDT_SCENE_GetMainPipeIndex(HI_HANDLE VcapMainPipeHdl, HI_S32* ps32Index)
{
    HI_APPCOMM_CHECK_POINTER(ps32Index, HI_PDT_SCENE_EINVAL);
    HI_S32 i = 0;

    for (i = 0; i < s_stSceneCTX.stSceneState.u32MainPipeNum; i++)
    {
        if (VcapMainPipeHdl == s_stSceneCTX.stSceneState.astMainPipe[i].MainPipeHdl)
        {
            *ps32Index = i;
            break;
        }
    }

    if (i == s_stSceneCTX.stSceneState.u32MainPipeNum)
    {
        MLOGE("Invalid pipe.\n");
        return HI_PDT_SCENE_EINVAL;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_SubPipeToMainPipe(HI_HANDLE VcapSubPipeHdl, HI_HANDLE* pstVcapMainPipeHdl)
{
    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (VcapSubPipeHdl == s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl)
        {
            if (HI_FALSE == s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
            {
                MLOGE("Invalid pipe.\n");
                return HI_PDT_SCENE_EINVAL;
            }

            if (HI_PDT_SCENE_PIPE_TYPE_SNAP != s_stSceneCTX.stSceneMode.astPipeAttr[i].enPipeType)
            {
                MLOGE("Invalid pipe.\n");
                return HI_PDT_SCENE_EINVAL;
            }

            *pstVcapMainPipeHdl = s_stSceneCTX.stSceneMode.astPipeAttr[i].MainPipeHdl;
            break;
        }
    }

    if (HI_PDT_SCENE_PIPE_MAX_NUM == i)
    {
        MLOGE("Invalid pipe.\n");
        return HI_PDT_SCENE_EINVAL;
    }
    return HI_SUCCESS;
}

#ifdef CONFIG_SCENEAUTO_SUPPORT
static HI_S32 PDT_SCENE_GetPipeIndexFromSceneMode(HI_HANDLE VcapPipeHdl, HI_S32* ps32Index)
{
    HI_APPCOMM_CHECK_POINTER(ps32Index, HI_PDT_SCENE_EINVAL);
    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (VcapPipeHdl == s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl)
        {
            *ps32Index = i;
            break;
        }
    }

    if (i == HI_PDT_SCENE_PIPE_MAX_NUM)
    {
        MLOGE("Invalid pipe.\n");
        return HI_PDT_SCENE_EINVAL;
    }
    return HI_SUCCESS;
}
#endif
static HI_S32 PDT_SCENE_SetLongExp_AeRoute(HI_PDT_SCENE_SNAP_LONGEXP_TYPE_E enSnapLongExpType)
{
    HI_S32 i = 0;

    switch (enSnapLongExpType)
    {
        case HI_PDT_SCENE_SNAP_LONGEXP_TYPE_POSTPROCESS:
            s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count = s_stSceneCTX.pstSceneParam->stSnapAeRouteParam.stPostProcessRoute.u32Count;
            if(HI_PDT_SCENE_LONGEXP_AEROUTE_MAX_NUM < s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count)
            {
                return HI_PDT_SCENE_EINVAL;
            }
            for (i = 0; i < s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count; i++)
            {

                s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[i].u32SysGain = s_stSceneCTX.pstSceneParam->stSnapAeRouteParam.stPostProcessRoute.astLongExpAERoute[i].u32SysGain;
                s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[i].u32ExpTime = s_stSceneCTX.pstSceneParam->stSnapAeRouteParam.stPostProcessRoute.astLongExpAERoute[i].u32ExpTime;
            }
            break;

        case HI_PDT_SCENE_SNAP_LONGEXP_TYPE_NORMAL:
            s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count = s_stSceneCTX.pstSceneParam->stSnapAeRouteParam.stNormalRoute.u32Count;
            if(HI_PDT_SCENE_LONGEXP_AEROUTE_MAX_NUM < s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count)
            {
                return HI_PDT_SCENE_EINVAL;
            }
            for (i = 0; i < s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count; i++)
            {
                s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[i].u32SysGain = s_stSceneCTX.pstSceneParam->stSnapAeRouteParam.stNormalRoute.astLongExpAERoute[i].u32SysGain;
                s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[i].u32ExpTime = s_stSceneCTX.pstSceneParam->stSnapAeRouteParam.stNormalRoute.astLongExpAERoute[i].u32ExpTime;
            }
            break;

        case HI_PDT_SCENE_SNAP_LONGEXP_TYPE_NIGHTMODE:
            s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count = s_stSceneCTX.pstSceneParam->stSnapAeRouteParam.stNightRoute.u32Count;
            if(HI_PDT_SCENE_LONGEXP_AEROUTE_MAX_NUM < s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count)
            {
                return HI_PDT_SCENE_EINVAL;
            }
            for (i = 0; i < s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count; i++)
            {
                s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[i].u32SysGain = s_stSceneCTX.pstSceneParam->stSnapAeRouteParam.stNightRoute.astLongExpAERoute[i].u32SysGain;
                s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[i].u32ExpTime = s_stSceneCTX.pstSceneParam->stSnapAeRouteParam.stNightRoute.astLongExpAERoute[i].u32ExpTime;
            }
            break;
        default:
            MLOGE("LongExp Choice is illegal.\n");
            return HI_PDT_SCENE_EINVAL;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_SetMainPipeState(const HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    HI_S32 i, j = 0;
    HI_U32 u32MainPipeCnt = 0;
    /**if not use, set to 0*/
    memset(s_stSceneCTX.stSceneState.astMainPipe, 0, sizeof(PDT_SCENE_MAINPIPE_STATE_S) * HI_PDT_SCENE_PIPE_MAX_NUM);

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        s_stSceneCTX.stSceneState.astMainPipe[i].bDISEnable = HI_FALSE;
        s_stSceneCTX.stSceneState.astMainPipe[i].bLongExp= HI_FALSE;
        s_stSceneCTX.stSceneState.astMainPipe[i].bMetryFixed= HI_FALSE;
    }

    /**get mainpipe array*/
    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        if (0 == u32MainPipeCnt)
        {
            s_stSceneCTX.stSceneState.astMainPipe[u32MainPipeCnt].MainPipeHdl = s_stSceneCTX.stSceneMode.astPipeAttr[i].MainPipeHdl;
            s_stSceneCTX.stSceneState.astMainPipe[u32MainPipeCnt].enPipeMode = s_stSceneCTX.stSceneMode.astPipeAttr[i].enPipeMode;
            u32MainPipeCnt++;
            continue;
        }

        for (j = 0; j < u32MainPipeCnt; j++)
        {
            if (s_stSceneCTX.stSceneState.astMainPipe[j].MainPipeHdl == s_stSceneCTX.stSceneMode.astPipeAttr[i].MainPipeHdl)
            {
                break;
            }
        }

        if (u32MainPipeCnt == j)
        {
            s_stSceneCTX.stSceneState.astMainPipe[u32MainPipeCnt].MainPipeHdl = s_stSceneCTX.stSceneMode.astPipeAttr[i].MainPipeHdl;
            s_stSceneCTX.stSceneState.astMainPipe[u32MainPipeCnt].enPipeMode = s_stSceneCTX.stSceneMode.astPipeAttr[i].enPipeMode;
            u32MainPipeCnt++;
        }
    }

    /**set subpipe in certain mainpipe*/
    for (i = 0; i < u32MainPipeCnt; i++)
    {
        HI_U32 u32SubPipeCnt = 0;

        for (j = 0; j < HI_PDT_SCENE_PIPE_MAX_NUM; j++)
        {
            if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[j].bEnable)
            {
                continue;
            }

            if (s_stSceneCTX.stSceneState.astMainPipe[i].MainPipeHdl == s_stSceneCTX.stSceneMode.astPipeAttr[j].MainPipeHdl)
            {
               s_stSceneCTX.stSceneState.astMainPipe[i].aSubPipeHdl[u32SubPipeCnt] = s_stSceneCTX.stSceneMode.astPipeAttr[j].VcapPipeHdl;
               u32SubPipeCnt++;
            }
        }

        s_stSceneCTX.stSceneState.astMainPipe[i].u32SubPipeNum = u32SubPipeCnt;
    }

    s_stSceneCTX.stSceneState.u32MainPipeNum = u32MainPipeCnt;

    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_CalculateExp(VI_PIPE ViPipe, HI_U32* pu32Iso, HI_U64* pu64Exposure)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64SysGainValue = 0;

    ISP_EXP_INFO_S stIspExpInfo;
    ISP_PUB_ATTR_S stPubAttr;

    PDT_SCENE_CHECK_PAUSE();

    s32Ret = HI_MPI_ISP_QueryExposureInfo(ViPipe, &stIspExpInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe,&stPubAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    u64SysGainValue = (HI_U64)stIspExpInfo.u32AGain * (HI_U64)stIspExpInfo.u32DGain * (HI_U64)stIspExpInfo.u32ISPDGain;

    *pu32Iso = (u64SysGainValue * 100) >> 30;

    if(WDR_MODE_4To1_LINE == stPubAttr.enWDRMode)
    {
        *pu64Exposure = (u64SysGainValue * (HI_U64)stIspExpInfo.u32LongExpTime) >> 30;
    }
    else if(WDR_MODE_3To1_LINE == stPubAttr.enWDRMode)
    {
        *pu64Exposure = (u64SysGainValue * (HI_U64)stIspExpInfo.u32MedianExpTime) >> 30;
    }
    else if(WDR_MODE_2To1_LINE == stPubAttr.enWDRMode)
    {
        *pu64Exposure = (u64SysGainValue * (HI_U64)stIspExpInfo.u32ShortExpTime) >> 30;
    }
    else
    {
        *pu64Exposure = (u64SysGainValue * (HI_U64)stIspExpInfo.u32ExpTime) >> 30;
    }

    return HI_SUCCESS;
}

#ifdef CONFIG_SCENEAUTO_SUPPORT
static HI_S32 PDT_SCENE_SetMainPipeSpecialParam(VI_PIPE ViPipe, HI_U8 u8Index, HI_BOOL bMetryFixed)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        s32Ret = PDT_SCENE_SetStaticAE(ViPipe, u8Index);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        s32Ret = PDT_SCENE_SetStaticAERouteEX(ViPipe, u8Index);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        s32Ret = PDT_SCENE_SetStaticAWB(ViPipe, u8Index);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        s32Ret = PDT_SCENE_SetStaticStatisticsCfg(ViPipe, u8Index, bMetryFixed);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    s32Ret = PDT_SCENE_SetStaticCCM(ViPipe, u8Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_SetStaticLDCI(ViPipe, u8Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_SetPipeStaticParam(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
        {
            if (HI_TRUE!= s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
            {
                continue;
            }

            s32Ret = PDT_SCENE_SetStaticAE(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
        }
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
        {
            if (HI_TRUE!= s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
            {
                continue;
            }

            s32Ret = PDT_SCENE_SetStaticAERouteEX(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
        }
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
        {
            if (HI_TRUE!= s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
            {
                continue;
            }

            s32Ret = PDT_SCENE_SetStaticAWB(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
        }
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
        {
            if (HI_TRUE!= s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
            {
                continue;
            }

            HI_S32 s32Index = 0;
            HI_HANDLE MainPipeHdl = s_stSceneCTX.stSceneMode.astPipeAttr[i].MainPipeHdl;
            HI_BOOL bMetryFixed = HI_FALSE;

            s32Ret = PDT_SCENE_GetMainPipeIndex(MainPipeHdl, &s32Index);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

            bMetryFixed = s_stSceneCTX.stSceneState.astMainPipe[s32Index].bMetryFixed;

            s32Ret = PDT_SCENE_SetStaticStatisticsCfg(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex, bMetryFixed);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
        }
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE!= s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticCCM(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticNr(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticCSC(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticSaturation(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticLDCI(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticSharpen(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticClut(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticCA(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticCrossTalk(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticSaturation(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticShading(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticDemosaic(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticDPC(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticDE(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_SetPipeDynamicParam(HI_VOID)
{
    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_SetVIPipeParam(HI_VOID)
{
    HI_S32 i, j = 0;
    HI_U8  u8Index = 0;
    HI_HANDLE SubPipeHdl = -1;
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_SCENE_SetPipeStaticParam();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    /*set mainIsp param*/
    for (i = 0; i < s_stSceneCTX.stSceneState.u32MainPipeNum ; i++)
    {
        VI_PIPE ViPipe = s_stSceneCTX.stSceneState.astMainPipe[i].MainPipeHdl;

        /**if MainIsp Pipe have SubIsp, Then Use SubIsp's param to set MainIsp*/
        /**Here, Only AE AWB*/
        /**The reason to do this, because of reducing the pipeType Number*/
        if (s_stSceneCTX.stSceneState.astMainPipe[i].u32SubPipeNum > 1)
        {
            for (j = 0; j < s_stSceneCTX.stSceneState.astMainPipe[i].u32SubPipeNum; j++)
            {
                if (ViPipe != s_stSceneCTX.stSceneState.astMainPipe[i].aSubPipeHdl[j])
                {
                    SubPipeHdl = s_stSceneCTX.stSceneState.astMainPipe[i].aSubPipeHdl[j];
                    u8Index = s_stSceneCTX.stSceneMode.astPipeAttr[SubPipeHdl].u8PipeParamIndex;
                    break;
                }
            }

            s32Ret = PDT_SCENE_SetMainPipeSpecialParam(ViPipe, u8Index, s_stSceneCTX.stSceneState.astMainPipe[i].bMetryFixed);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
        }

        s32Ret = PDT_SCENE_CalculateExp(ViPipe, &(s_stSceneCTX.stSceneState.astMainPipe[i].u32Iso), &(s_stSceneCTX.stSceneState.astMainPipe[i].u64Exposure));
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_SetVPSSParam(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        if (HI_TRUE == s_stSceneCTX.stSceneMode.astPipeAttr[i].bBypassVpss)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticPostSharpen(s_stSceneCTX.stSceneMode.astPipeAttr[i].VpssHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_SetVENCParam(HI_VOID)
{
    // to do
    return HI_SUCCESS;
}

HI_VOID* PDT_SCENE_NormalAutoThread(HI_VOID* pVoid)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j = 0;
    HI_U32 u32Iso = 0;
    HI_U64 u64Exposure = 0;
    HI_U64 u64LastExposure = 0;

    prctl(PR_SET_NAME, (unsigned long)"HI_PDT_SCENE_NormalThread", 0, 0, 0);

    while (HI_TRUE == s_stSceneCTX.stSceneState.stThreadNormal.bThreadFlag)
    {

        s32Ret = PDT_SCENE_SetPipeDynamicParam();
        for (i = 0; i < s_stSceneCTX.stSceneState.u32MainPipeNum; i++)
        {
            VI_PIPE ViPipe = s_stSceneCTX.stSceneState.astMainPipe[i].MainPipeHdl;
            s32Ret = PDT_SCENE_CalculateExp(ViPipe, &u32Iso, &u64Exposure);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);
            u64LastExposure = s_stSceneCTX.stSceneState.astMainPipe[i].u64LastNormalExposure;
            s_stSceneCTX.stSceneState.astMainPipe[i].u64LastNormalExposure = u64Exposure;
            s_stSceneCTX.stSceneState.astMainPipe[i].u32LastNormalIso = u32Iso;
            for (j = 0; j < s_stSceneCTX.stSceneState.astMainPipe[i].u32SubPipeNum; j++)
            {
                HI_HANDLE PipeHdl = s_stSceneCTX.stSceneState.astMainPipe[i].aSubPipeHdl[j];
                HI_U8 u8PipeIndex = s_stSceneCTX.stSceneMode.astPipeAttr[PipeHdl].u8PipeParamIndex;
                PDT_SCENE_SetDynamicShading(PipeHdl, u64Exposure, u64LastExposure, u8PipeIndex);
            }
        }
        HI_usleep(300000);
    }

    return HI_NULL;
}

HI_VOID* PDT_SCENE_LuminanceAutoThread(HI_VOID* pVoid)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i, j = 0;
    HI_U64 u64Exposure = 0;
    HI_U32 u32Iso = 0;
    HI_U64 u64LastExposure = 0;
    HI_HANDLE  SubPipeHdl = -1;
    HI_U8 u8PipeIndex = 0;
    HI_S32 s32HdlIndex = 0;

    prctl(PR_SET_NAME, (unsigned long)"HI_PDT_SCENE_LuminanceThread", 0, 0, 0);

    while (HI_TRUE == s_stSceneCTX.stSceneState.stThreadLuminance.bThreadFlag)
    {
        for (i = 0; i < s_stSceneCTX.stSceneState.u32MainPipeNum; i++)
        {
            HI_HANDLE MainPipeHdl = s_stSceneCTX.stSceneState.astMainPipe[i].MainPipeHdl;
            s32Ret = PDT_SCENE_CalculateExp(MainPipeHdl, &u32Iso, &u64Exposure);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);
            u64LastExposure = s_stSceneCTX.stSceneState.astMainPipe[i].u64LastLuminanceExposure;
            s_stSceneCTX.stSceneState.astMainPipe[i].u64LastLuminanceExposure = u64Exposure;
            s_stSceneCTX.stSceneState.astMainPipe[i].u32LastLuminanceISO = u32Iso;

            s32Ret = PDT_SCENE_GetPipeIndexFromSceneMode(MainPipeHdl, &s32HdlIndex);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);

            if (s_stSceneCTX.stSceneState.astMainPipe[i].u32SubPipeNum <= 1)
            {
                u8PipeIndex = s_stSceneCTX.stSceneMode.astPipeAttr[s32HdlIndex].u8PipeParamIndex;
            }
            else
            {
                for (j = 0; j < s_stSceneCTX.stSceneState.astMainPipe[i].u32SubPipeNum; j++)
                {
                    if (MainPipeHdl != s_stSceneCTX.stSceneState.astMainPipe[i].aSubPipeHdl[j])
                    {
                        SubPipeHdl = s_stSceneCTX.stSceneState.astMainPipe[i].aSubPipeHdl[j];

                        s32Ret = PDT_SCENE_GetPipeIndexFromSceneMode(SubPipeHdl, &s32HdlIndex);
                        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);

                        u8PipeIndex = s_stSceneCTX.stSceneMode.astPipeAttr[s32HdlIndex].u8PipeParamIndex;
                        break;
                    }
                }
            }

            PDT_SCENE_SetDynamicAE(MainPipeHdl, u64Exposure, u64LastExposure, u8PipeIndex);
            PDT_SCENE_SetDynamicGamma(MainPipeHdl, u64Exposure, u64LastExposure, u8PipeIndex);
        }
        HI_usleep(100000);
    }
   return HI_NULL;
}

static HI_S32 PDT_SCENE_StartAutoThread(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_FALSE == s_stSceneCTX.stSceneState.stThreadNormal.bThreadFlag)
    {
        pthread_attr_init(&(s_stSceneCTX.stSceneState.stThreadnormattr));
        pthread_attr_setdetachstate(&(s_stSceneCTX.stSceneState.stThreadnormattr), PTHREAD_CREATE_DETACHED);
        s_stSceneCTX.stSceneState.stThreadnormattr.inheritsched = PTHREAD_EXPLICIT_SCHED;
        s_stSceneCTX.stSceneState.stThreadnormattr.schedparam.sched_priority = 10;
        s_stSceneCTX.stSceneState.stThreadNormal.bThreadFlag = HI_TRUE;
        s32Ret = pthread_create(&s_stSceneCTX.stSceneState.stThreadNormal.pThread, &(s_stSceneCTX.stSceneState.stThreadnormattr), PDT_SCENE_NormalAutoThread, NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_FALSE == s_stSceneCTX.stSceneState.stThreadLuminance.bThreadFlag)
    {
        pthread_attr_init(&(s_stSceneCTX.stSceneState.stThreadluminanceattr));
        pthread_attr_setdetachstate(&(s_stSceneCTX.stSceneState.stThreadnormattr), PTHREAD_CREATE_DETACHED);
        s_stSceneCTX.stSceneState.stThreadluminanceattr.inheritsched = PTHREAD_EXPLICIT_SCHED;
        s_stSceneCTX.stSceneState.stThreadluminanceattr.schedparam.sched_priority = 10;
        s_stSceneCTX.stSceneState.stThreadLuminance.bThreadFlag = HI_TRUE;
        s32Ret = pthread_create(&s_stSceneCTX.stSceneState.stThreadLuminance.pThread, &(s_stSceneCTX.stSceneState.stThreadluminanceattr), PDT_SCENE_LuminanceAutoThread, NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_StopAutoThread(HI_VOID)
{
    if (HI_TRUE == s_stSceneCTX.stSceneState.stThreadNormal.bThreadFlag)
    {
        s_stSceneCTX.stSceneState.stThreadNormal.bThreadFlag = HI_FALSE;
        pthread_attr_destroy(&(s_stSceneCTX.stSceneState.stThreadnormattr));
    }

    if (HI_TRUE == s_stSceneCTX.stSceneState.stThreadLuminance.bThreadFlag)
    {
        s_stSceneCTX.stSceneState.stThreadLuminance.bThreadFlag = HI_FALSE;
        pthread_attr_destroy(&(s_stSceneCTX.stSceneState.stThreadluminanceattr));
    }

    return HI_SUCCESS;
}
#endif
static HI_S32 PDT_SCENE_CalculateLongExp(HI_S32 s32Index, HI_PDT_SCENE_LONGEXP_PARAM_S* pstLongExpParam)
{
    HI_U32 u32TimeIndex = 0;
    HI_U32 u32GainIndex = 0;
    HI_U32 u32Time = 0;
    HI_U32 u32Gain = 0;

    if (s_stSceneCTX.stSceneState.astMainPipe[s32Index].u32Iso < ((s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[0].u32SysGain >> 10) * 100))
    {
        pstLongExpParam->bLongExp = HI_FALSE;
        s_stSceneCTX.stSceneState.astMainPipe[s32Index].bLongExp = HI_FALSE;
    }
    else
    {
        pstLongExpParam->bLongExp = HI_TRUE;
        s_stSceneCTX.stSceneState.astMainPipe[s32Index].bLongExp = HI_TRUE;

        while((s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count > u32TimeIndex) && (s_stSceneCTX.stSceneState.stLongExpAeRoute.u32Count > u32GainIndex))
        {
            u32Time = s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[u32TimeIndex].u32ExpTime;
            u32Gain = s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[u32GainIndex].u32SysGain;
            if (0 == u32Time || 0 == u32Gain)
            {
                MLOGE("The route number should not be zero.\n");
                return HI_PDT_SCENE_EINVAL;
            }

            if (u32TimeIndex == u32GainIndex)
            {
                if (s_stSceneCTX.stSceneState.astMainPipe[s32Index].u64Exposure < (u32Time * (u32Gain >> 10)))
                {
                    pstLongExpParam->stAeRoute.u32SysGain = u32Gain;
                    pstLongExpParam->stAeRoute.u32ExpTime = s_stSceneCTX.stSceneState.astMainPipe[s32Index].u64Exposure / (u32Gain >> 10);
                    s_stSceneCTX.stSceneState.astMainPipe[s32Index].u32LongExpIso = (u32Gain >> 10) * 100;
                    return HI_SUCCESS;
                }
                u32GainIndex++;
            }
            else
            {
                if (s_stSceneCTX.stSceneState.astMainPipe[s32Index].u64Exposure < (u32Time * (u32Gain >> 10)))
                {
                    pstLongExpParam->stAeRoute.u32SysGain = 1024 * (s_stSceneCTX.stSceneState.astMainPipe[s32Index].u64Exposure / u32Time);
                    pstLongExpParam->stAeRoute.u32ExpTime = u32Time;
                    s_stSceneCTX.stSceneState.astMainPipe[s32Index].u32LongExpIso = ((pstLongExpParam->stAeRoute.u32SysGain) >> 10) * 100;
                    return HI_SUCCESS;
                }
                u32TimeIndex++;
            }
        }

        if (0 == s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[u32TimeIndex].u32ExpTime)
        {
            MLOGE("The number should not be zero.\n");
            return HI_PDT_SCENE_EINVAL;
        }

        pstLongExpParam->stAeRoute.u32SysGain = 1024 * (s_stSceneCTX.stSceneState.astMainPipe[s32Index].u64Exposure / s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[u32TimeIndex].u32ExpTime);
        pstLongExpParam->stAeRoute.u32ExpTime = s_stSceneCTX.stSceneState.stLongExpAeRoute.astLongExpAERoute[u32TimeIndex].u32ExpTime;
        s_stSceneCTX.stSceneState.astMainPipe[s32Index].u32LongExpIso = ((pstLongExpParam->stAeRoute.u32SysGain) >> 10) * 100;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_LoadParam()
{
#ifdef CONFIG_DEBUG
    HI_S32 s32Fd = -1;

    s32Fd = open(SCENEAUTO_PARAM_PATH, O_RDWR, 777);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s32Fd >= 0, HI_FAILURE, "Open "SCENEAUTO_PARAM_PATH);

    MLOGD("Load param from sceneauto.bin\n");
    ssize_t readCount = read(s32Fd, s_stSceneCTX.pstSceneParam, sizeof(PDT_SCENE_PARAM_S));
    close(s32Fd);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(0 < readCount, HI_FAILURE, "Read "SCENEAUTO_PARAM_PATH);
#elif defined (CONFIG_RELEASE)
    extern unsigned char sceneparam_bin[];
    s_stSceneCTX.pstSceneParam = (PDT_SCENE_PARAM_S*)sceneparam_bin;
#endif
    MLOGI("The number is %d.\n", s_stSceneCTX.pstSceneParam->stSpecialParam.u32MinimumSysgain);
    return HI_SUCCESS;
}

/**-------------------------external function interface-------------------------*/
HI_S32 HI_PDT_SCENE_Init()
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_MUTEX_LOCK(s_stSceneCTX.SceneModuleMutex);

    if (HI_TRUE == s_stSceneCTX.stSceneState.bSceneInit)
    {
        MLOGE("SCENE module has already been inited.\n");
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
        return HI_SUCCESS;
    }

#if (defined(CONFIG_DEBUG) && defined(AMP_LINUX_HUAWEILITE))
    s_stSceneCTX.pstSceneParam = (PDT_SCENE_PARAM_S*)malloc(sizeof(PDT_SCENE_PARAM_S));
    if (HI_NULL == s_stSceneCTX.pstSceneParam)
    {
        MLOGE("malloc scene param failed.\n");
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
        return HI_SUCCESS;
    }
#endif
    s32Ret = PDT_SCENE_LoadParam();
    if (HI_SUCCESS != s32Ret)
    {
#if (defined(CONFIG_DEBUG) && defined(AMP_LINUX_HUAWEILITE))
        HI_APPCOMM_SAFE_FREE(s_stSceneCTX.pstSceneParam);
#endif
        MLOGE("PDT_SCENE_LoadParam failed.\n");
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_SetPipeParam(s_stSceneCTX.pstSceneParam->astPipeParam, PDT_SCENE_PIPETYPE_CNT);
    if (HI_SUCCESS != s32Ret)
    {
#if (defined(CONFIG_DEBUG) && defined(AMP_LINUX_HUAWEILITE))
        HI_APPCOMM_SAFE_FREE(s_stSceneCTX.pstSceneParam);
#endif
        MLOGE("HI_PDT_SCENE_SetPipeParam failed.\n");
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
        return HI_FAILURE;
    }
    MLOGD("The PDT_SCENE_PIPETYPE_CNT is %d.\n", PDT_SCENE_PIPETYPE_CNT);

    s_stSceneCTX.stSceneState.bRefreshIgnore = HI_FALSE;
    s_stSceneCTX.stSceneState.bSceneInit = HI_TRUE;
    HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
    MLOGD("SCENE module has been inited successfully.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetSceneMode(const HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    //dprintf("Lation@ HI_PDT_SCENE_SetSceneMode %d, %d, %d\n", pstSceneMode->astPipeAttr[0].bEnable, pstSceneMode->astPipeAttr[0].MainPipeHdl, pstSceneMode->astPipeAttr[0].u8PipeParamIndex);

    PDT_SCENE_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(pstSceneMode, HI_PDT_SCENE_EINVAL);

    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    HI_MUTEX_LOCK(s_stSceneCTX.SceneModuleMutex);

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != pstSceneMode->astPipeAttr[i].bEnable)
        {
            continue;
        }

        if (HI_PDT_SCENE_PIPE_TYPE_SNAP != pstSceneMode->astPipeAttr[i].enPipeType && HI_PDT_SCENE_PIPE_TYPE_VIDEO != pstSceneMode->astPipeAttr[i].enPipeType)
        {
            MLOGE("Pipe Type is not video or snap!\n");
            HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
            return HI_PDT_SCENE_EINVAL;
        }

        if (pstSceneMode->astPipeAttr[i].u8PipeParamIndex < 0 || pstSceneMode->astPipeAttr[i].u8PipeParamIndex >= PDT_SCENE_PIPETYPE_CNT)
        {
            MLOGE("Pipe param index is out of range! i(%d) index(%d)\n", i, pstSceneMode->astPipeAttr[i].u8PipeParamIndex);
            HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
            return HI_PDT_SCENE_EINVAL;
        }
    }

    memcpy(&s_stSceneCTX.stSceneMode, pstSceneMode, sizeof(HI_PDT_SCENE_MODE_S));

    s32Ret = PDT_SCENE_SetMainPipeState(pstSceneMode);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENE_SetMainIspState failed.\n");
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
        return HI_FAILURE;
    }
#ifdef CONFIG_SCENEAUTO_SUPPORT
    s32Ret = PDT_SCENE_SetVIPipeParam();
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENE_SetIspParam failed.\n");
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
        return HI_FAILURE;
    }

    extern HI_S32 PDT_SCENE_SetStatic3DNR(const HI_PDT_SCENE_MODE_S* pstSceneMode);
    s32Ret = PDT_SCENE_SetStatic3DNR(pstSceneMode);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_SetStaticThreeDNR failed.\n");
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_SetVPSSParam();
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENE_SetVPSSParam failed.\n");
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_SetVENCParam();
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENE_SetVPSSParam failed.\n");
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_StartAutoThread();
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENE_StartThread failed.\n");
        HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
        return HI_FAILURE;
    }
#endif
    HI_MUTEX_UNLOCK(s_stSceneCTX.SceneModuleMutex);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_GetSceneMode(HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    PDT_SCENE_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(pstSceneMode, HI_PDT_SCENE_EINVAL);

    memcpy(pstSceneMode, &s_stSceneCTX.stSceneMode, sizeof(HI_PDT_SCENE_MODE_S));

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_Pause(HI_BOOL bEnable)
{
    PDT_SCENE_CHECK_INIT();

    HI_S32 s32Ret = HI_SUCCESS;

    s_stSceneCTX.stSceneState.bPause = bEnable;

    s32Ret = PDT_SCENE_SetPause(bEnable);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    return HI_SUCCESS;
}

#ifdef CFG_POST_PROCESS
#define HDR_DEBUG 0
static HI_S32 PDT_SCENE_CalculateHDR(HI_S32 s32Index, HI_PDT_SCENE_HDR_PARAM_S* pstHDRParam)
{
    HI_U32 i = 0;
    HI_U32 u32DarkPercent = 0;
    HI_U32 u32HighPercent = 0;

    HI_U32 u32Tmp = 0;
    HI_U32 u32DarkMax = 0;
    HI_U32 u32HighMax = HIST_NUM-1;

    HI_U64 u64NormIdx = 0;
    HI_U64 u64DarkIdx = 0;
    HI_U64 u64HighIdx = 0;

    HI_U64 u64HistSum = 0;
    HI_U64 u64DarkSum = 0;
    HI_U64 u64HighSum = 0;
    HI_U64 u64DarkClip = 0;
    HI_U64 u64HighClip = 0;

    HI_S32 s32Ret = HI_SUCCESS;

    ISP_AE_STATISTICS_S stAEStatistics;
    ISP_EXP_INFO_S stIspExpInfo;

    VI_PIPE ViPipe = s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl;

    s32Ret = HI_MPI_ISP_QueryExposureInfo(ViPipe, &stIspExpInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    HI_U32 u32Exptime = stIspExpInfo.u32ExpTime;
    HI_U32 u32Sysgain = ((stIspExpInfo.u32AGain >> 10) * (stIspExpInfo.u32DGain >> 10)* (stIspExpInfo.u32ISPDGain >> 10)) << 10;
    //HI_U32 u32ISO = ((HI_U64)stIspExpInfo.u32AGain * (HI_U64)stIspExpInfo.u32DGain * (HI_U64)stIspExpInfo.u32ISPDGain * 100) >> 30;

    s32Ret = HI_MPI_ISP_GetAEStatistics(ViPipe, &stAEStatistics);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    for (i = 0; i < HIST_NUM; i++)
    {
        u32Tmp = stAEStatistics.au32BEHist1024Value[i];

        u64HistSum += u32Tmp;
        u64NormIdx += u32Tmp * i;

        if (PDT_SCENE_HDR_DARK_LINE > i)
        {
            u64DarkSum += u32Tmp;
            u64DarkIdx += u32Tmp * i;
            if (stAEStatistics.au32BEHist1024Value[u32DarkMax] <= u32Tmp)
            {
                u32DarkMax = i;
            }
        }

        if (PDT_SCENE_HDR_HIGH_LINE < i)
        {
            u64HighSum += u32Tmp;
            u64HighIdx += u32Tmp * i;
            if (stAEStatistics.au32BEHist1024Value[u32HighMax] <= u32Tmp)
            {
                u32HighMax = i;
            }
        }
    }

    if (0 == u64HistSum)
    {
        MLOGD("Histogram Sum is zero => hist(%lld), dark(%lld), high(%lld).\n", u64HistSum, u64DarkSum, u64HighSum);

        pstHDRParam->astHDRAeRoute[0].u32SysGain = u32Sysgain;
        pstHDRParam->astHDRAeRoute[1].u32SysGain = u32Sysgain;
        pstHDRParam->astHDRAeRoute[2].u32SysGain = u32Sysgain;

        pstHDRParam->astHDRAeRoute[0].u32ExpTime = u32Exptime;
        pstHDRParam->astHDRAeRoute[1].u32ExpTime = u32Exptime;
        pstHDRParam->astHDRAeRoute[2].u32ExpTime = u32Exptime;

        return HI_SUCCESS;
    }

    /**obtain the norm (or mean)*/
    u64NormIdx = u64NormIdx / (u64HistSum+1);
    u64DarkIdx = u64DarkIdx / (u64DarkSum+1);
    u64HighIdx = u64HighIdx / (u64HighSum+1);

    /** estimate of clipped dark intensities*/
    for (i = 0; i < 8; i++)
        u64DarkClip+= stAEStatistics.au32BEHist1024Value[i];

    u64HighClip = stAEStatistics.au32BEHist1024Value[HIST_NUM-1];

    u32DarkPercent = ((u64DarkSum * 10000) / u64HistSum);
    u32HighPercent = ((u64HighSum * 10000) / u64HistSum);

    if ((u32DarkPercent >= PDT_SCENE_HDR_DARK_PERCENT) && (u32HighPercent >= PDT_SCENE_HDR_HIGH_PERCENT))
    {
        pstHDRParam->bHDR = HI_TRUE;
    }
    else
    {
        pstHDRParam->bHDR = HI_FALSE;
    }
#if HDR_DEBUG
    MLOGI("[HDR] u32ExpTime = %d, u32Sysgain = %d, u32ISO = %d\n", u32Exptime, u32Sysgain, u32ISO);
    MLOGI("[HDR] u32DarkPercent = %d, u32BrightPercent = %d\n", u32DarkPercent, u32HighPercent);

    MLOGD("[HDR] LINEAR...\n");
    MLOGD("[HDR] u64DarkIdx = %lld, u64NormIdx = %lld, u64HighIdx = %lld\n", u64DarkIdx, u64NormIdx, u64HighIdx);
    MLOGD("[HDR] u32DarkMax = %d, u32HighMax = %d\n", u32DarkMax, u32HighMax);
    MLOGD("[HDR] u64DarkSum = %lld, u64HistSum = %lld, u64HighSum = %lld\n", u64DarkSum, u64HistSum, u64HighSum);
    MLOGD("[HDR] u64DarkClip = %lld, u64HighClip = %lld\n", u64DarkClip, u64HighClip);
#endif/*HDR_DEBUG*/

    /* scale to [0, 255] range + assume gamma 2.2*/
    u64DarkIdx = 255 * pow(u64DarkIdx / (HI_FLOAT)HIST_NUM, 0.45);
    u64NormIdx = 255 * pow(u64NormIdx / (HI_FLOAT)HIST_NUM, 0.45);
    u64HighIdx = 255 * pow(u64HighIdx / (HI_FLOAT)HIST_NUM, 0.45);

    u32DarkMax = 255 * pow(u32DarkMax / (HI_FLOAT)HIST_NUM, 0.45);
    u32HighMax = 255 * pow(u32HighMax / (HI_FLOAT)HIST_NUM, 0.45);

#if HDR_DEBUG
    MLOGI("[HDR] NON-LINEAR\n");
    MLOGI("[HDR] u64DarkIdx = %lld, u64NormIdx = %lld, u64HighIdx = %lld\n", u64DarkIdx, u64NormIdx, u64HighIdx);
    MLOGI("[HDR] u32DarkMax = %d, u32HighMax = %d\n", u32DarkMax, u32HighMax);
    MLOGI("[HDR] u64DarkSum = %lld, u64HistSum = %lld, u64HighSum = %lld\n", u64DarkSum, u64HistSum, u64HighSum);
    MLOGI("[HDR] u64DarkClip = %lld, u64HighClip = %lld\n", u64DarkClip, u64HighClip);
#endif/*HDR_DEBUG*/

    HI_FLOAT fDarkKey = (u64NormIdx - u64DarkIdx) / 128.0; /* almost linear response*/
    HI_FLOAT fHighKey = (u64HighIdx - u64NormIdx) / 128.0; /* almost linear response*/

   /* modeled as sigmoid(n), 4. is a scaler
     fEV_BIAS -> 1, likely dark
     fEV_BIAS -> 0, likely over-exposed*/
    HI_FLOAT fEV_BIAS = 1./(1. + exp(4. * (u64HighClip / (HI_FLOAT)(u64DarkClip+1) - 1.)));
    /* shift and scale to Exposure Value [-1, 1]*/
    fEV_BIAS = 2.0 * fEV_BIAS - 1.0;

#if HDR_DEBUG
    MLOGI("[HDR] BIAS = %.2f, Dark = %.2f, High = %.2f\n", fEV_BIAS, fDarkKey, fHighKey);
#endif/*HDR_DEBUG*/

   /*
     * empirical factor
     * boost shadow amplification to match the case of LDCI enabled
     */
    const HI_FLOAT fDarkGain = 1.8;
    /* compensate for any shift in middle exposure*/
    if (fEV_BIAS < 0)fDarkKey = fDarkKey - fEV_BIAS;
    if (fEV_BIAS > 0)fHighKey = fHighKey + fEV_BIAS;

    /* translate key and compensate clipped regions*/
    fDarkKey = pow(2.0, fDarkGain * fDarkKey) * pow(2.0, 2.0 * u64DarkClip / (HI_FLOAT)(u64DarkSum+1)); /*estimate clipped region*/
    fHighKey = pow(2.0, fHighKey) * pow(2.0, 2.0 * u64HighClip / (HI_FLOAT)(u64HighSum+1)); /* estimate clipped region*/
    fEV_BIAS = pow(2.0, fEV_BIAS);

    /* clamp from over-compensation: apprx. +/-2.3 EV*/
    HI_FLOAT fPOS_EVBIAS = fDarkKey > 5.0?5.0:fDarkKey;
    HI_FLOAT fNEG_EVBIAS = fHighKey > 5.0?0.2:(1./fHighKey);

    pstHDRParam->astHDRAeRoute[0].u32SysGain = u32Sysgain;
    pstHDRParam->astHDRAeRoute[1].u32SysGain = u32Sysgain;
    pstHDRParam->astHDRAeRoute[2].u32SysGain = u32Sysgain;

    pstHDRParam->astHDRAeRoute[0].u32ExpTime = u32Exptime * fEV_BIAS * fNEG_EVBIAS;
    pstHDRParam->astHDRAeRoute[1].u32ExpTime = u32Exptime * fEV_BIAS;
    pstHDRParam->astHDRAeRoute[2].u32ExpTime = u32Exptime * fEV_BIAS * fPOS_EVBIAS;
#if HDR_DEBUG
    MLOGI("[HDR] SHIFT = %.2f, POS_BIAS = %.2f, NEG_BIAS = %.2f\n", fEV_BIAS, fPOS_EVBIAS, fNEG_EVBIAS);
#endif/*HDR_DEBUG*/
    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_CalculateSFNR(HI_S32 s32Index, HI_PDT_SCENE_SFNR_PARAM_S* pstSFNRParam)
{
    HI_U32 u32Iso = 0;

    u32Iso = s_stSceneCTX.stSceneState.astMainPipe[s32Index].u32Iso;

    if (PDT_SCENE_SFNR_ISO_THRESH <= u32Iso)
    {
        pstSFNRParam->bSFNR = HI_TRUE;
    }
    else
    {
        pstSFNRParam->bSFNR = HI_FALSE;
    }

    /**Currently, Do nou use SFNR*/
    pstSFNRParam->bSFNR = HI_FALSE;
    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_CalculateMFNR(HI_S32 s32Index, HI_PDT_SCENE_MFNR_PARAM_S* pstMFNRParam)
{
    HI_U32 u32Iso = 0;

    u32Iso = s_stSceneCTX.stSceneState.astMainPipe[s32Index].u32Iso;

    if (PDT_SCENE_MFNR_ISO_THRESH <= u32Iso)
    {
        pstMFNRParam->bMFNR = HI_TRUE;
    }
    else
    {
        pstMFNRParam->bMFNR = HI_FALSE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_GetDetectPostProcessInfo(HI_HANDLE VcapPipeHdl, HI_PDT_SCENE_POSTPROCESS_INFO_S* pstDetectInfo)
{
    PDT_SCENE_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(pstDetectInfo, HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;

    HI_HANDLE MainPipeHdl = HI_INVALID_HANDLE;

    s32Ret = PDT_SCENE_SubPipeToMainPipe(VcapPipeHdl, &MainPipeHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_GetMainPipeIndex(MainPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_CalculateExp(MainPipeHdl, &(s_stSceneCTX.stSceneState.astMainPipe[s32Index].u32Iso), &(s_stSceneCTX.stSceneState.astMainPipe[s32Index].u64Exposure));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_CalculateHDR(s32Index, &pstDetectInfo->stHDRParam);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_CalculateSFNR(s32Index, &pstDetectInfo->stSFNRParam);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_CalculateMFNR(s32Index, &pstDetectInfo->stMFNRParam);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    /**To make sure Three PhotoAlg can only be one true at same time.*/
    if (pstDetectInfo->stMFNRParam.bMFNR && pstDetectInfo->stSFNRParam.bSFNR)
    {
        pstDetectInfo->stSFNRParam.bSFNR= HI_FALSE;
    }

    if (pstDetectInfo->stMFNRParam.bMFNR && pstDetectInfo->stHDRParam.bHDR)
    {
        /** Means At night, Then Do MFNR*/
        if (s_stSceneCTX.stSceneState.astMainPipe[s32Index].u32Iso >= PDT_SCENE_DAYNIGHT_ISO_THRESH)
        {
            pstDetectInfo->stHDRParam.bHDR = HI_FALSE;
        }
        else
        {
            pstDetectInfo->stMFNRParam.bMFNR = HI_FALSE;
        }
    }

    if (pstDetectInfo->stSFNRParam.bSFNR && pstDetectInfo->stHDRParam.bHDR)
    {
        /** Means At night, Then Do MFNR*/
        if (s_stSceneCTX.stSceneState.astMainPipe[s32Index].u32Iso >= PDT_SCENE_DAYNIGHT_ISO_THRESH)
        {
            pstDetectInfo->stHDRParam.bHDR = HI_FALSE;
        }
        else
        {
            pstDetectInfo->stSFNRParam.bSFNR = HI_FALSE;
        }
    }

    return HI_SUCCESS;
}
#endif

HI_S32 HI_PDT_SCENE_GetDetectLongExpInfo(HI_HANDLE VcapPipeHdl, HI_PDT_SCENE_SNAP_LONGEXP_TYPE_E enSnapLongExpType, HI_PDT_SCENE_LONGEXP_PARAM_S* pstLongExpParam)
{
    PDT_SCENE_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(pstLongExpParam, HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;

    HI_HANDLE MainPipeHdl = HI_INVALID_HANDLE;

    s32Ret = PDT_SCENE_SubPipeToMainPipe(VcapPipeHdl, &MainPipeHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_GetMainPipeIndex(MainPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_SetLongExp_AeRoute(enSnapLongExpType);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_CalculateExp(MainPipeHdl, &(s_stSceneCTX.stSceneState.astMainPipe[s32Index].u32Iso), &(s_stSceneCTX.stSceneState.astMainPipe[s32Index].u64Exposure));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_CalculateLongExp(s32Index, pstLongExpParam);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_RefreshPhotoPipe(HI_HANDLE VcapPipeHdl, HI_U8 u8PipeParamIndex)
{
    PDT_SCENE_CHECK_INIT();

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (VcapPipeHdl == s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl)
        {
            if (HI_FALSE == s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
            {
                MLOGE("Invalid pipe.\n");
                return HI_PDT_SCENE_EINVAL;
            }

            s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex = u8PipeParamIndex;
            break;
        }
    }

    if (HI_PDT_SCENE_PIPE_MAX_NUM == i)
    {
        MLOGE("Invalid pipe.\n");
        return HI_PDT_SCENE_EINVAL;
    }

    s_stSceneCTX.stSceneState.bRefreshIgnore = HI_TRUE;

    s32Ret = HI_PDT_SCENE_SetSceneMode(&s_stSceneCTX.stSceneMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s_stSceneCTX.stSceneState.bRefreshIgnore = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_GetLongExpGain(HI_HANDLE VcapPipeHdl, HI_U32 u32ExpTime_us, HI_U32* pu32SysGain)
{
    PDT_SCENE_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(pu32SysGain, HI_PDT_SCENE_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == HI_APPCOMM_CHECK_RANGE(u32ExpTime_us, PDT_SCENE_EXPTIME_LOWERLIMIT_US, PDT_SCENE_EXPTIME_LIMIT_US), HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Iso = 0;
    HI_U64 u64Exposure = 0;
    HI_U32 u32ExpTime = 0;
    HI_S32 s32Index = 0;

    s32Ret = PDT_SCENE_GetMainPipeIndex(VcapPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_CalculateExp(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &u32Iso, &u64Exposure);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    /** The time from input is ms, but we use it in us*/
    u32ExpTime = u32ExpTime_us;

    if (0 == u32ExpTime)
    {
        MLOGE("Do not need set sysgain!!\n");
    }
    else
    {
        *pu32SysGain = (u64Exposure / u32ExpTime) << 10;

        if (*pu32SysGain < s_stSceneCTX.pstSceneParam->stSpecialParam.u32MinimumSysgain)
        {

            /** the gain must be 1 times, if lower than this, the pixel can not get to the maximum value */
            /** the color in white area will be pink, most of the sensor has the characteristic */
            /** but the gain mostly lager than 1 times, becauseof the sensro characteristic, sony maybe 1025 * 1.5 */
            *pu32SysGain = s_stSceneCTX.pstSceneParam->stSpecialParam.u32MinimumSysgain;
        }
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_GetLongExpTime(HI_HANDLE VcapPipeHdl, HI_U32 u32ISO, HI_U32* pu32ExpTime_us)
{
    PDT_SCENE_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(pu32ExpTime_us, HI_PDT_SCENE_EINVAL);

    if (u32ISO != 0 && (u32ISO < 100 || u32ISO > 6400))
    {
        return HI_PDT_SCENE_EINVAL;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurIso = 0;
    HI_U64 u64Exposure = 0;
    HI_U32 u32SysGain = 0;
    HI_S32 s32Index = 0;

    s32Ret = PDT_SCENE_GetMainPipeIndex(VcapPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_CalculateExp(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &u32CurIso, &u64Exposure);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    /** The time from input is ms, but we use it in us*/
    u32SysGain = u32ISO / 100 * 1024;

    if (0 == u32SysGain)
    {
        MLOGE("Do not need set sysgain!!\n");
    }
    else
    {
        *pu32ExpTime_us = (u64Exposure / u32SysGain) << 10;
    }

    return HI_SUCCESS;
}


HI_S32 HI_PDT_SCENE_SetDIS(HI_HANDLE VcapPipeHdl, HI_BOOL bEnable)
{
    PDT_SCENE_CHECK_INIT();
    HI_S32 i = 0;

    for (i = 0; i < s_stSceneCTX.stSceneState.u32MainPipeNum; i++)
    {
        if (s_stSceneCTX.stSceneState.astMainPipe[i].MainPipeHdl == VcapPipeHdl)
        {
            s_stSceneCTX.stSceneState.astMainPipe[i].bDISEnable = bEnable;
            return HI_SUCCESS;
        }
    }

    return HI_PDT_SCENE_EINVAL;
}

HI_S32 HI_PDT_SCENE_SetISO(HI_HANDLE VcapPipeHdl, HI_U32 u32ISO)
{
    PDT_SCENE_CHECK_INIT();

    if (u32ISO != 0 && (u32ISO < 100 || u32ISO > 6400))
    {
        return HI_PDT_SCENE_EINVAL;
    }

    ISP_EXPOSURE_ATTR_S stExposure;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;
    static HI_U32 u32AutoMax = 0;
    static HI_U32 u32AutoMin = 0;
    static HI_U32 u32LastIso = 0;
    static HI_BOOL bAERouteExValid = HI_FALSE;

    s32Ret = PDT_SCENE_GetMainPipeIndex(VcapPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = HI_MPI_ISP_GetExposureAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stExposure);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    if (0 == u32LastIso)
    {
        u32AutoMax = stExposure.stAuto.stSysGainRange.u32Max;
        u32AutoMin = stExposure.stAuto.stSysGainRange.u32Min;
        bAERouteExValid = stExposure.bAERouteExValid;
    }

    u32LastIso = u32ISO;

    if (0 != u32ISO)
    {
        stExposure.enOpType = OP_TYPE_AUTO;
        stExposure.bAERouteExValid = HI_FALSE;
        stExposure.stAuto.stSysGainRange.u32Max = u32ISO / 100 * 1024;
        stExposure.stAuto.stSysGainRange.u32Min = u32ISO / 100 * 1024;
    }
    else
    {
        stExposure.enOpType = OP_TYPE_AUTO;
        stExposure.bAERouteExValid = bAERouteExValid;
        stExposure.stAuto.stSysGainRange.u32Max = u32AutoMax;
        stExposure.stAuto.stSysGainRange.u32Min = u32AutoMin;
    }

    s32Ret = HI_MPI_ISP_SetExposureAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stExposure);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetEV(HI_HANDLE VcapPipeHdl, HI_PDT_SCENE_EV_E enEV)
{
    PDT_SCENE_CHECK_INIT();

    ISP_EXPOSURE_ATTR_S stExposure;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;

    s32Ret = PDT_SCENE_GetMainPipeIndex(VcapPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = HI_MPI_ISP_GetExposureAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stExposure);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    stExposure.enOpType = OP_TYPE_AUTO;

    switch (enEV)
    {

        case HI_PDT_SCENE_EV_0:
            stExposure.stAuto.u16EVBias = PDT_SCENE_ORIGINAL_EVBIAS * pow(2, 0);
            break;

        case HI_PDT_SCENE_EV_0_5:
            stExposure.stAuto.u16EVBias = PDT_SCENE_ORIGINAL_EVBIAS * pow(2, 0.5);
            break;

        case HI_PDT_SCENE_EV_B0_5:
            stExposure.stAuto.u16EVBias = PDT_SCENE_ORIGINAL_EVBIAS * pow(2, -0.5);
            break;

        case HI_PDT_SCENE_EV_1:
            stExposure.stAuto.u16EVBias = PDT_SCENE_ORIGINAL_EVBIAS * pow(2, 1);
            break;

        case HI_PDT_SCENE_EV_B1:
            stExposure.stAuto.u16EVBias = PDT_SCENE_ORIGINAL_EVBIAS * pow(2, -1);
            break;

        case HI_PDT_SCENE_EV_1_5:
            stExposure.stAuto.u16EVBias = PDT_SCENE_ORIGINAL_EVBIAS * pow(2, 1.5);
            break;

        case HI_PDT_SCENE_EV_B1_5:
            stExposure.stAuto.u16EVBias = PDT_SCENE_ORIGINAL_EVBIAS * pow(2, -1.5);
            break;

        case HI_PDT_SCENE_EV_2:
            stExposure.stAuto.u16EVBias = PDT_SCENE_ORIGINAL_EVBIAS * pow(2, 2);
            break;

        case HI_PDT_SCENE_EV_B2:
            stExposure.stAuto.u16EVBias = PDT_SCENE_ORIGINAL_EVBIAS * pow(2, -2);
            break;
        default:
            MLOGE("Error Ev!\n");
            return HI_PDT_SCENE_EINVAL;
    }

    s32Ret = HI_MPI_ISP_SetExposureAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stExposure);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetWB(HI_HANDLE VcapPipeHdl, HI_U32 u32WB)
{
    PDT_SCENE_CHECK_INIT();
    if (u32WB != 0 && (u32WB < 1500 || u32WB > 15000))
    {
        return HI_PDT_SCENE_EINVAL;
    }

    ISP_WB_ATTR_S stWbAttr;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;

    HI_U16 u16ColorTemp = (HI_U16)u32WB;

    s32Ret = PDT_SCENE_GetMainPipeIndex(VcapPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);


    s32Ret = HI_MPI_ISP_GetWBAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stWbAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    if (u32WB == 0)
    {
        stWbAttr.bByPass = HI_FALSE;
        stWbAttr.enOpType =  OP_TYPE_AUTO;
    }
    else
    {
        HI_U16 u16AwbGain[4];

        s32Ret = HI_MPI_ISP_CalGainByTemp(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stWbAttr, u16ColorTemp, 0, u16AwbGain);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

        stWbAttr.stManual.u16Rgain = u16AwbGain[0];
        stWbAttr.stManual.u16Grgain = u16AwbGain[1];
        stWbAttr.stManual.u16Gbgain = u16AwbGain[2];
        stWbAttr.stManual.u16Bgain = u16AwbGain[3];
        stWbAttr.bByPass = HI_FALSE;
        stWbAttr.enOpType = OP_TYPE_MANUAL;
    }

    s32Ret = HI_MPI_ISP_SetWBAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stWbAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetExpTime(HI_HANDLE VcapPipeHdl, HI_U32 u32ExpTime_us)
{
    PDT_SCENE_CHECK_INIT();

    /**The Time must smaller than 1/FrameRate*/
    if (u32ExpTime_us != 0 && u32ExpTime_us > PDT_SCENE_EXPTIME_VIDEOLIMIT_US)
    {
        return HI_PDT_SCENE_EINVAL;
    }

    ISP_EXPOSURE_ATTR_S stExposure;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;
    static HI_U32 u32AutoMax = 0;
    static HI_U32 u32AutoMin = 0;
    static HI_U32 u32LastExpTime = 0;

    s32Ret = PDT_SCENE_GetMainPipeIndex(VcapPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = HI_MPI_ISP_GetExposureAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stExposure);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    if (0 == u32LastExpTime)
    {
        u32AutoMax = stExposure.stAuto.stExpTimeRange.u32Max;
        u32AutoMin = stExposure.stAuto.stExpTimeRange.u32Min;
    }

    u32LastExpTime = u32ExpTime_us;

    if (0 != u32ExpTime_us)
    {
        stExposure.enOpType = OP_TYPE_AUTO;
        stExposure.stAuto.stExpTimeRange.u32Max = u32ExpTime_us;
        stExposure.stAuto.stExpTimeRange.u32Min = u32ExpTime_us;
    }
    else
    {
        stExposure.enOpType = OP_TYPE_AUTO;
        stExposure.stAuto.stExpTimeRange.u32Max = u32AutoMax;
        stExposure.stAuto.stExpTimeRange.u32Min = u32AutoMin;
    }

    s32Ret = HI_MPI_ISP_SetExposureAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stExposure);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetMetry(HI_HANDLE VcapPipeHdl, const HI_PDT_SCENE_METRY_S* pstMetry)
{
    PDT_SCENE_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(pstMetry,HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;
    HI_U32 u32ArrayIndex = 0;
    ISP_STATISTICS_CFG_S stStatisticsCfg;

    s32Ret = PDT_SCENE_GetMainPipeIndex(VcapPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = HI_MPI_ISP_GetStatisticsConfig(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stStatisticsCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    switch(pstMetry->enMetryType)
    {
        case HI_PDT_SCENE_METRY_TYPE_CENTER:
            u32ArrayIndex = pstMetry->unParam.stCenterMetryParam.u32Index;
            memcpy(stStatisticsCfg.stAECfg.au8Weight, s_stSceneCTX.pstSceneParam->stExParam.astCenterMetryAeWeight[u32ArrayIndex].au8AEWeight, \
                               sizeof(stStatisticsCfg.stAECfg.au8Weight));
            s32Ret = HI_MPI_ISP_SetStatisticsConfig(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stStatisticsCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
            s_stSceneCTX.stSceneState.astMainPipe[s32Index].bMetryFixed = HI_TRUE;
            break;

        case HI_PDT_SCENE_METRY_TYPE_AVERAGE:
            u32ArrayIndex = pstMetry->unParam.stAverageMetryParam.u32Index;
            memcpy(stStatisticsCfg.stAECfg.au8Weight, s_stSceneCTX.pstSceneParam->stExParam.astAvergeMetryAeWeight[u32ArrayIndex].au8AEWeight, \
                               sizeof(stStatisticsCfg.stAECfg.au8Weight));
            s32Ret = HI_MPI_ISP_SetStatisticsConfig(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stStatisticsCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
            s_stSceneCTX.stSceneState.astMainPipe[s32Index].bMetryFixed = HI_FALSE;
            break;

        case HI_PDT_SCENE_METRY_TYPE_SPOT:
            {
#if 0
                HI_U8 au8AEWeight[AE_ZONE_ROW][AE_ZONE_COLUMN] = {0};

                HI_APPCOMM_CHECK_EXPR(HI_TRUE == HI_APPCOMM_CHECK_RANGE(pstMetry->unParam.stSpotMetryParam.stPoint.s32X, 0, 100), HI_PDT_SCENE_EINVAL);
                HI_APPCOMM_CHECK_EXPR(HI_TRUE == HI_APPCOMM_CHECK_RANGE(pstMetry->unParam.stSpotMetryParam.stPoint.s32Y, 0, 100), HI_PDT_SCENE_EINVAL);
                HI_APPCOMM_CHECK_EXPR(HI_TRUE == HI_APPCOMM_CHECK_RANGE(pstMetry->unParam.stSpotMetryParam.stSize.u32Height, 0, 100), HI_PDT_SCENE_EINVAL);
                HI_APPCOMM_CHECK_EXPR(HI_TRUE == HI_APPCOMM_CHECK_RANGE(pstMetry->unParam.stSpotMetryParam.stSize.u32Width, 0, 100), HI_PDT_SCENE_EINVAL);
                HI_S32 s32X = pstMetry->unParam.stSpotMetryParam.stPoint.s32X;
                HI_S32 s32Y = pstMetry->unParam.stSpotMetryParam.stPoint.s32Y;
                HI_U32 u32Height = pstMetry->unParam.stSpotMetryParam.stSize.u32Height;
                HI_U32 u32Weight = pstMetry->unParam.stSpotMetryParam.stSize.u32Width;

                HI_U8 u8StartRowPos = (HI_U8)((HI_FLOAT)(s32X - u32Weight/2) / 100.0 * AE_ZONE_ROW);
                HI_U8 u8EndRowPos = (HI_U8)((HI_FLOAT)(s32X + u32Weight/2) / 100.0 * AE_ZONE_ROW) + 1;
                HI_U8 u8StartColumnPos = (HI_U8)((HI_FLOAT)(s32Y - u32Height/2) / 100.0 * AE_ZONE_COLUMN);
                HI_U8 u8EndColumnPos = (HI_U8)((HI_FLOAT)(s32Y + u32Height/2) / 100.0 * AE_ZONE_COLUMN) + 1;

                HI_S32 i, j = 0;

                /**Margin Check*/
                if ((u8StartRowPos - 1) < 0 ||
                    (u8StartColumnPos - 1) < 0 ||
                    u8EndRowPos < (u8StartRowPos - 1) ||
                    u8EndColumnPos < (u8StartColumnPos - 1) ||
                    (u8StartRowPos - 1)  > AE_ZONE_ROW ||
                    u8EndRowPos > AE_ZONE_ROW ||
                    (u8StartColumnPos - 1)  > AE_ZONE_COLUMN ||
                    u8EndColumnPos > AE_ZONE_ROW
                    )
                {
                    MLOGE("Spot Metry param is error.\n");
                    return HI_PDT_SCENE_EINTER;
                }

                for (i = u8StartRowPos - 1; i < u8EndRowPos; i++)
                {
                    for (j = u8StartColumnPos - 1; j < u8EndColumnPos; j++)
                    {
                        au8AEWeight[i][j] = 1;
                    }
                }
                HI_S32 k, w = 0;
                printf("ExpWeight is: \n");
                for (k = 0; k < AE_ZONE_ROW; k++)
                {
                    for (w = 0; w < AE_ZONE_COLUMN; w++)
                    {
                        printf("%3d ",au8AEWeight[k][w]);
                    }
                    printf("\n");
                }
                memcpy(stStatisticsCfg.stAECfg.au8Weight, au8AEWeight, sizeof(stStatisticsCfg.stAECfg.au8Weight));
                s32Ret = HI_MPI_ISP_SetStatisticsConfig(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stStatisticsCfg);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
                s_stSceneCTX.stSceneState.astMainPipe[s32Index].bMetryFixed = HI_TRUE;
#else
                HI_U8 au8AEWeight[AE_ZONE_ROW][AE_ZONE_COLUMN] = {0};

                HI_APPCOMM_CHECK_EXPR(HI_TRUE == HI_APPCOMM_CHECK_RANGE(pstMetry->unParam.stSpotMetryParam.stPoint.s32X, 0, 100), HI_PDT_SCENE_EINVAL);
                HI_APPCOMM_CHECK_EXPR(HI_TRUE == HI_APPCOMM_CHECK_RANGE(pstMetry->unParam.stSpotMetryParam.stPoint.s32Y, 0, 100), HI_PDT_SCENE_EINVAL);
                HI_S32 s32X = pstMetry->unParam.stSpotMetryParam.stPoint.s32X;
                HI_S32 s32Y = pstMetry->unParam.stSpotMetryParam.stPoint.s32Y;

                HI_U8 u8RowPos = (HI_U8)((HI_FLOAT)(s32X) / 100.0 * AE_ZONE_ROW);
                HI_U8 u8ColumnPos = (HI_U8)((HI_FLOAT)(s32Y) / 100.0 * AE_ZONE_COLUMN);

                /**Margin Check*/
                if ((u8RowPos) < 0 ||
                    (u8ColumnPos) < 0 ||
                    (u8RowPos)  >= AE_ZONE_ROW ||
                    (u8ColumnPos) >= AE_ZONE_COLUMN
                    )
                {
                    MLOGE("Spot Metry param is error.\n");
                    return HI_PDT_SCENE_EINTER;
                }
                au8AEWeight[u8RowPos][u8ColumnPos] = 1;

                memcpy(stStatisticsCfg.stAECfg.au8Weight, au8AEWeight, sizeof(stStatisticsCfg.stAECfg.au8Weight));
                s32Ret = HI_MPI_ISP_SetStatisticsConfig(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stStatisticsCfg);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
                s_stSceneCTX.stSceneState.astMainPipe[s32Index].bMetryFixed = HI_TRUE;
#endif
            }
        default:
            s_stSceneCTX.stSceneState.astMainPipe[s32Index].bMetryFixed = HI_FALSE;
            break;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_Deinit(HI_VOID)
{
    PDT_SCENE_CHECK_INIT();

#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_SCENE_StopAutoThread();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
#endif

#if (defined(CONFIG_DEBUG) && defined(AMP_LINUX_HUAWEILITE))
    HI_APPCOMM_SAFE_FREE(s_stSceneCTX.pstSceneParam);
#endif

    s_stSceneCTX.stSceneState.bSceneInit = HI_FALSE;

    MLOGD("SCENE Module has been deinited successfully!\n");

    return HI_SUCCESS;
}

#ifdef CFG_POST_PROCESS
HI_S32 HI_PDT_SCENE_LockAE(HI_HANDLE VcapPipeHdl, const HI_BOOL bLock)
{
    PDT_SCENE_CHECK_INIT();

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;

    ISP_EXPOSURE_ATTR_S stExposureAttr;

    HI_HANDLE MainPipeHdl = HI_INVALID_HANDLE;

    /** COMMENT: photo mode is dual-pipe, must get handle to main pipe which controls sensor*/
    s32Ret = PDT_SCENE_SubPipeToMainPipe(VcapPipeHdl, &MainPipeHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_GetMainPipeIndex(MainPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = HI_MPI_ISP_GetExposureAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stExposureAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    if (HI_TRUE == bLock)
    {
        stExposureAttr.bByPass = HI_TRUE;
    }
    else
    {
        stExposureAttr.bByPass = HI_FALSE;
    }

    s32Ret = HI_MPI_ISP_SetExposureAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stExposureAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_LockAWB(HI_HANDLE VcapPipeHdl, const HI_BOOL bLock)
{
    PDT_SCENE_CHECK_INIT();

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;

    ISP_WB_ATTR_S stWbAttr;
    ISP_WB_INFO_S stWbInfo;
    static ISP_OP_TYPE_E  enOpType = OP_TYPE_BUTT;
    static HI_BOOL bLockFlag = HI_FALSE;
    HI_HANDLE MainPipeHdl = HI_INVALID_HANDLE;

    /** COMMENT: photo mode is dual-pipe, must get handle to main pipe which controls sensor*/
    s32Ret = PDT_SCENE_SubPipeToMainPipe(VcapPipeHdl, &MainPipeHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    s32Ret = PDT_SCENE_GetMainPipeIndex(MainPipeHdl, &s32Index);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    if (HI_TRUE ==  bLock)
    {
        /**lock awb */
        s32Ret = HI_MPI_ISP_QueryWBInfo(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stWbInfo);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

        s32Ret = HI_MPI_ISP_GetWBAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stWbAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

        enOpType = stWbAttr.enOpType;
        stWbAttr.enOpType = OP_TYPE_MANUAL;
        stWbAttr.stManual.u16Bgain  = stWbInfo.u16Bgain;
        stWbAttr.stManual.u16Gbgain = stWbInfo.u16Gbgain;
        stWbAttr.stManual.u16Grgain = stWbInfo.u16Grgain;
        stWbAttr.stManual.u16Rgain  = stWbInfo.u16Rgain;
        bLockFlag = HI_TRUE;

    }
    else
    {
        /** unlock awb */
        s32Ret = HI_MPI_ISP_GetWBAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stWbAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

        if (bLockFlag == HI_TRUE)
        {
            stWbAttr.enOpType = enOpType;;
        }
        bLockFlag = HI_FALSE;
    }

    s32Ret = HI_MPI_ISP_SetWBAttr(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stWbAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    return HI_SUCCESS;
}
#endif

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
HI_S32 HI_PDT_SCENE_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = HI_EVTHUB_Register(HI_EVENT_PDT_SCENE_FRAMERATE_CHANGE);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_EVENT_PDT_SCENE_FRAMERATE_CHANGE register fail \n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
#endif

/** @}*/  /** <!-- ==== SCENE End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
