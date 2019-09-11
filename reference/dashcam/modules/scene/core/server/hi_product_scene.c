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
static HI_S32 PDT_SCENE_SetMainPipeState(const HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    HI_S32 i, j = 0;
    HI_U32 u32MainPipeCnt = 0;
    /**if not use, set to 0*/
    memset(s_stSceneCTX.stSceneState.astMainPipe, 0, sizeof(PDT_SCENE_MAINPIPE_STATE_S) * HI_PDT_SCENE_PIPE_MAX_NUM);

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        s_stSceneCTX.stSceneState.astMainPipe[i].bDISEnable = HI_FALSE;
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

#ifdef CONFIG_SCENEAUTO_SUPPORT
static HI_S32 PDT_SCENE_CalculateExp(VI_PIPE ViPipe, HI_U32* pu32Iso, HI_U64* pu64Exposure)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64SysGainValue = 0;

    ISP_EXP_INFO_S stIspExpInfo;
    ISP_PUB_ATTR_S stPubAttr;

    PDT_SCENE_CHECK_PAUSE();

    s32Ret = HI_MPI_ISP_QueryExposureInfo(ViPipe, &stIspExpInfo);
    if (s32Ret != HI_SUCCESS)
    {
        MLOGW("Query IspInfo error.\n");
        return HI_PDT_SCENE_EINTER;
    }


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

static HI_S32 PDT_SCENE_CalculateWdrParam(VI_PIPE ViPipe, HI_U32 *pu32ActRation)
{
    HI_S32 s32Ret = HI_SUCCESS;

    ISP_INNER_STATE_INFO_S stInnerStateInfo;

    PDT_SCENE_CHECK_PAUSE();
    s32Ret = HI_MPI_ISP_QueryInnerStateInfo(ViPipe, &stInnerStateInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);

    *pu32ActRation = stInnerStateInfo.u32WDRExpRatioActual[0];
    return HI_SUCCESS;
}

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
        s32Ret = PDT_SCENE_SetStaticAERoute(ViPipe, u8Index);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        s32Ret = PDT_SCENE_SetStaticSFAERoute(ViPipe, u8Index);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        s32Ret = PDT_SCENE_SetStaticAWB(ViPipe, u8Index);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        s32Ret = PDT_SCENE_SetStaticSaturation(ViPipe, u8Index);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        s32Ret = PDT_SCENE_SetStaticCCM(ViPipe, u8Index);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        s32Ret = PDT_SCENE_SetStaticStatisticsCfg(ViPipe, u8Index, bMetryFixed);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

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

            s32Ret = PDT_SCENE_SetStaticAERoute(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
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

            s32Ret = PDT_SCENE_SetStaticSFAERoute(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
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

            s32Ret = PDT_SCENE_SetStaticSaturation(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
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

            s32Ret = PDT_SCENE_SetStaticCCM(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
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

        s32Ret = PDT_SCENE_SetStaticCAC(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
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

        s32Ret = PDT_SCENE_SetStaticClut(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
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

        s32Ret = PDT_SCENE_SetStaticSharpen(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
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

        s32Ret = PDT_SCENE_SetStaticDemosaic(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        /**no matter in WDR or in Linear, Always set StaticDRC. Linear Set Enable to HI_FALSE*/
        s32Ret = PDT_SCENE_SetStaticDRC(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        if (HI_PDT_SCENE_PIPE_MODE_WDR != s_stSceneCTX.stSceneMode.astPipeAttr[i].enPipeMode)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticWDRExposure(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        if (HI_PDT_SCENE_PIPE_MODE_WDR != s_stSceneCTX.stSceneMode.astPipeAttr[i].enPipeMode)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticFSWDR(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_TRUE != s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = PDT_SCENE_SetStaticDEHAZE(s_stSceneCTX.stSceneMode.astPipeAttr[i].VcapPipeHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_SetPipeDynamicParam(HI_VOID)
{
    HI_S32 i = 0;
    HI_U32 u32Iso = 0;
    HI_U64 u64Exposure = 0;

    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        VI_PIPE ViPipe = s_stSceneCTX.stSceneState.astMainPipe[i].MainPipeHdl;
        PDT_SCENE_CalculateExp(ViPipe, &u32Iso, &u64Exposure);
        if (HI_TRUE!= s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable
            || s_stSceneCTX.stSceneMode.astPipeAttr[i].bBypassVenc)
        {
            continue;
        }

        PDT_SCENE_SetDynamicQVBRVENC(s_stSceneCTX.stSceneMode.astPipeAttr[i].VencHdl,
        s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex,
        u64Exposure, s_stSceneCTX.stSceneState.astMainPipe[i].u64LastNormalExposure);
    }

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
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_SetVPSSParam(HI_VOID)
{
    return HI_SUCCESS;
}

static HI_S32 PDT_SCENE_SetVENCParam(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_TRUE != s_stSceneCTX.stSceneState.bRefreshIgnore)
    {
        for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
        {
            if (HI_TRUE!= s_stSceneCTX.stSceneMode.astPipeAttr[i].bEnable
                || s_stSceneCTX.stSceneMode.astPipeAttr[i].bBypassVenc)
            {
                continue;
            }
            s32Ret = PDT_SCENE_SetStaticQP(s_stSceneCTX.stSceneMode.astPipeAttr[i].VencHdl, s_stSceneCTX.stSceneMode.astPipeAttr[i].u8PipeParamIndex);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
        }
    }
    return HI_SUCCESS;
}

/**If MainPipe Has more then one SubPipe, Then Use SubPipe's Param to set MainPipe.*/
/**DashCam don't need to concern it. For two pipe Snap*/
static HI_S32 PDT_SCENE_GetSubepipeIndexFromMainPipeState(HI_S32 s32MainPipeArrayIdx,
    HI_HANDLE VcapPipeHdl, HI_U8* pu8Index)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32HdlIndex = -1;
    HI_HANDLE SubPipeHdl = 0;
    HI_S32 j = 0;
    s32Ret = PDT_SCENE_GetPipeIndexFromSceneMode(VcapPipeHdl, &s32HdlIndex);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);

    if (s_stSceneCTX.stSceneState.astMainPipe[s32MainPipeArrayIdx].u32SubPipeNum <= 1)
    {
        *pu8Index = s_stSceneCTX.stSceneMode.astPipeAttr[s32HdlIndex].u8PipeParamIndex;
    }
    else
    {
        for (j = 0; j < s_stSceneCTX.stSceneState.astMainPipe[s32MainPipeArrayIdx].u32SubPipeNum; j++)
        {
            if (VcapPipeHdl != s_stSceneCTX.stSceneState.astMainPipe[s32MainPipeArrayIdx].aSubPipeHdl[j])
            {
                SubPipeHdl = s_stSceneCTX.stSceneState.astMainPipe[s32MainPipeArrayIdx].aSubPipeHdl[j];

                s32Ret = PDT_SCENE_GetPipeIndexFromSceneMode(SubPipeHdl, &s32HdlIndex);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);

                *pu8Index = s_stSceneCTX.stSceneMode.astPipeAttr[s32HdlIndex].u8PipeParamIndex;
                break;
            }
        }
    }
    return HI_SUCCESS;
}

HI_VOID* PDT_SCENE_NormalAutoThread(HI_VOID* pVoid)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    HI_U32 u32ISO = 0;
    HI_U32 u32LastISO = 0;
    HI_U64 u64Exposure = 0;
    HI_U64 u64LastExposure = 0;
    HI_U8 u8PipeIndex = 0;

    prctl(PR_SET_NAME, "HI_PDT_SCENE_NormalThread", 0, 0, 0);

    while (HI_TRUE == s_stSceneCTX.stSceneState.stThreadNormal.bThreadFlag)
    {
        /**Set Dynamic Param in every pipe*/
        s32Ret = PDT_SCENE_SetPipeDynamicParam();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);

        /**Set Dynamic Param in main pipe*/
        for (i = 0; i < s_stSceneCTX.stSceneState.u32MainPipeNum; i++)
        {
            HI_HANDLE MainPipeHdl = s_stSceneCTX.stSceneState.astMainPipe[i].MainPipeHdl;
            PDT_SCENE_CalculateExp(MainPipeHdl, &u32ISO, &u64Exposure);
            u32LastISO = s_stSceneCTX.stSceneState.astMainPipe[i].u32LastNormalIso;
            u64LastExposure = s_stSceneCTX.stSceneState.astMainPipe[i].u64LastNormalExposure;
            s_stSceneCTX.stSceneState.astMainPipe[i].u64LastNormalExposure = u64Exposure;
            s_stSceneCTX.stSceneState.astMainPipe[i].u32LastNormalIso = u32ISO;

            s32Ret = PDT_SCENE_GetSubepipeIndexFromMainPipeState(i, MainPipeHdl, &u8PipeIndex);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);

            PDT_SCENE_SetDynamicClut(MainPipeHdl, u8PipeIndex, u32ISO, u32LastISO);
            PDT_SCENE_SetDynamicDEHAZE(MainPipeHdl, u8PipeIndex, u64Exposure, u64LastExposure);
            PDT_SCENE_SetDynamicNR(MainPipeHdl, u8PipeIndex, u32ISO, u32LastISO);
            PDT_SCENE_SetDynamicCAC(MainPipeHdl, u8PipeIndex, u32ISO, u32LastISO);
            PDT_SCENE_SetDynamicBlackLevel(MainPipeHdl, u8PipeIndex, u32ISO, u32LastISO);
            PDT_SCENE_SetDynamicShading(MainPipeHdl, u8PipeIndex, u64Exposure, u64LastExposure);
            PDT_SCENE_SetDynamicDRC(MainPipeHdl, u8PipeIndex, u32ISO, u32LastISO);
        }

        HI_usleep(30000);
    }

    return HI_NULL;
}

HI_VOID* PDT_SCENE_LuminanceAutoThread(HI_VOID* pVoid)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    HI_U32 u32ISO = 0;
    HI_U64 u64Exposure = 0;
    HI_U64 u64LastExposure = 0;
    HI_U8 u8PipeIndex = 0;

    prctl(PR_SET_NAME, "HI_PDT_SCENE_LuminanceThread", 0, 0, 0);

    while (HI_TRUE == s_stSceneCTX.stSceneState.stThreadLuminance.bThreadFlag)
    {
        for (i = 0; i < s_stSceneCTX.stSceneState.u32MainPipeNum; i++)
        {
            HI_HANDLE MainPipeHdl = s_stSceneCTX.stSceneState.astMainPipe[i].MainPipeHdl;

            PDT_SCENE_CalculateExp(MainPipeHdl, &u32ISO, &u64Exposure);
            u64LastExposure = s_stSceneCTX.stSceneState.astMainPipe[i].u64LastLuminanceExposure;
            s_stSceneCTX.stSceneState.astMainPipe[i].u64LastLuminanceExposure = u64Exposure;
            s_stSceneCTX.stSceneState.astMainPipe[i].u32LastLuminanceISO = u32ISO;

            s32Ret = PDT_SCENE_GetSubepipeIndexFromMainPipeState(i, MainPipeHdl, &u8PipeIndex);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);

            PDT_SCENE_SetDynamicAE(MainPipeHdl, u64Exposure, u64LastExposure, u8PipeIndex);
            PDT_SCENE_SetDynamicGamma(MainPipeHdl, u64Exposure, u64LastExposure, u8PipeIndex);
        }

        HI_usleep(30000);
    }
   return HI_NULL;
}

HI_VOID* PDT_SCENE_WDRAutoThread(HI_VOID* pVoid)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    HI_U32 u32ISO = 0;
    HI_U32 u32LastISO = 0;
    HI_U64 u64Exposure = 0;
    HI_U64 u64LastExposure = 0;
    HI_U8 u8PipeIndex = 0;
    HI_S32 s32HdlIndex = 0;

    prctl(PR_SET_NAME, "HI_PDT_SCENE_WDRThread", 0, 0, 0);

    while (HI_TRUE == s_stSceneCTX.stSceneState.stThreadWdr.bThreadFlag)
    {
        for (i = 0; i < s_stSceneCTX.stSceneState.u32MainPipeNum; i++)
        {
            HI_HANDLE MainPipeHdl = s_stSceneCTX.stSceneState.astMainPipe[i].MainPipeHdl;

            PDT_SCENE_CalculateExp(MainPipeHdl, &u32ISO, &u64Exposure);
            u32LastISO = s_stSceneCTX.stSceneState.astMainPipe[i].u32LastWdrISO;
            u64LastExposure = s_stSceneCTX.stSceneState.astMainPipe[i].u64LastWdrExposure;
            s_stSceneCTX.stSceneState.astMainPipe[i].u64LastWdrExposure = u64Exposure;
            s_stSceneCTX.stSceneState.astMainPipe[i].u32LastWdrISO = u32ISO;

            s32Ret = PDT_SCENE_CalculateWdrParam(MainPipeHdl, &(s_stSceneCTX.stSceneState.astMainPipe[i].u32ActRation));
            if (HI_PDT_SCENE_EINTER == s32Ret)
            {
                s_stSceneCTX.stSceneState.astMainPipe[i].u32ActRation = 0;
            }
            else if (HI_SUCCESS == s32Ret)
            {
            }
            else
            {
                MLOGE("Error.\n");
                return HI_NULL;
            }

            s32Ret = PDT_SCENE_GetPipeIndexFromSceneMode(MainPipeHdl, &s32HdlIndex);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);

            if (HI_PDT_SCENE_PIPE_MODE_WDR != s_stSceneCTX.stSceneMode.astPipeAttr[s32HdlIndex].enPipeMode)
            {
                continue;
            }

            s32Ret = PDT_SCENE_GetSubepipeIndexFromMainPipeState(i, MainPipeHdl, &u8PipeIndex);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_NULL);

            PDT_SCENE_SetDynamicFsWdr(MainPipeHdl, u8PipeIndex, s_stSceneCTX.stSceneState.astMainPipe[i].u32ActRation,
                u32ISO, u32LastISO);
            PDT_SCENE_SetDynamicWdrExposure(MainPipeHdl, u8PipeIndex, s_stSceneCTX.stSceneState.astMainPipe[i].u32ActRation,
                u64Exposure, u64LastExposure);
        }
        HI_usleep(30000);
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
#if (!defined(AMP_LINUX))
        s_stSceneCTX.stSceneState.stThreadnormattr.inheritsched = PTHREAD_EXPLICIT_SCHED;
        s_stSceneCTX.stSceneState.stThreadnormattr.schedparam.sched_priority = 10;
#endif
        s_stSceneCTX.stSceneState.stThreadNormal.bThreadFlag = HI_TRUE;
        s32Ret = pthread_create(&s_stSceneCTX.stSceneState.stThreadNormal.pThread, &(s_stSceneCTX.stSceneState.stThreadnormattr), PDT_SCENE_NormalAutoThread, NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_FALSE == s_stSceneCTX.stSceneState.stThreadLuminance.bThreadFlag)
    {
        pthread_attr_init(&(s_stSceneCTX.stSceneState.stThreadluminanceattr));
        pthread_attr_setdetachstate(&(s_stSceneCTX.stSceneState.stThreadnormattr), PTHREAD_CREATE_DETACHED);
#if (!defined(AMP_LINUX))
        s_stSceneCTX.stSceneState.stThreadluminanceattr.inheritsched = PTHREAD_EXPLICIT_SCHED;
        s_stSceneCTX.stSceneState.stThreadluminanceattr.schedparam.sched_priority = 10;
#endif
        s_stSceneCTX.stSceneState.stThreadLuminance.bThreadFlag = HI_TRUE;
        s32Ret = pthread_create(&s_stSceneCTX.stSceneState.stThreadLuminance.pThread, &(s_stSceneCTX.stSceneState.stThreadluminanceattr), PDT_SCENE_LuminanceAutoThread, NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
    }

    if (HI_FALSE == s_stSceneCTX.stSceneState.stThreadWdr.bThreadFlag)
    {
        pthread_attr_init(&(s_stSceneCTX.stSceneState.stThreadWdrattr));
        pthread_attr_setdetachstate(&(s_stSceneCTX.stSceneState.stThreadWdrattr), PTHREAD_CREATE_DETACHED);
#if (!defined(AMP_LINUX))
        s_stSceneCTX.stSceneState.stThreadWdrattr.inheritsched = PTHREAD_EXPLICIT_SCHED;
        s_stSceneCTX.stSceneState.stThreadWdrattr.schedparam.sched_priority = 10;
#endif
        s_stSceneCTX.stSceneState.stThreadWdr.bThreadFlag = HI_TRUE;
        s32Ret = pthread_create(&s_stSceneCTX.stSceneState.stThreadWdr.pThread, &(s_stSceneCTX.stSceneState.stThreadWdrattr), PDT_SCENE_WDRAutoThread, NULL);
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

    if (HI_TRUE == s_stSceneCTX.stSceneState.stThreadWdr.bThreadFlag)
    {
        s_stSceneCTX.stSceneState.stThreadWdr.bThreadFlag = HI_FALSE;
        pthread_attr_destroy(&(s_stSceneCTX.stSceneState.stThreadWdrattr));
    }

    return HI_SUCCESS;
}
#endif

static HI_S32 PDT_SCENE_LoadParam(HI_VOID)
{
#if (defined(CONFIG_DEBUG) && defined(AMP_LINUX_HUAWEILITE))
    HI_S32 s32Fd = -1;

    s32Fd = open(SCENEAUTO_PARAM_PATH, O_RDWR, 777);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s32Fd >= 0, HI_FAILURE, "Open "SCENEAUTO_PARAM_PATH);

    MLOGD("Load param from sceneauto.bin\n");
    ssize_t readCount = read(s32Fd, s_stSceneCTX.pstSceneParam, sizeof(PDT_SCENE_PARAM_S));
    close(s32Fd);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(0 < readCount, HI_FAILURE, "Read "SCENEAUTO_PARAM_PATH);
#else
    extern unsigned char sceneparam_bin[];
    s_stSceneCTX.pstSceneParam = (PDT_SCENE_PARAM_S*)sceneparam_bin;
#endif
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
            MLOGE("Pipe param index is out of range!\n");
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
                memcpy(stStatisticsCfg.stAECfg.au8Weight, au8AEWeight, sizeof(stStatisticsCfg.stAECfg.au8Weight));
                s32Ret = HI_MPI_ISP_SetStatisticsConfig(s_stSceneCTX.stSceneState.astMainPipe[s32Index].MainPipeHdl, &stStatisticsCfg);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_SCENE_EINTER);
                s_stSceneCTX.stSceneState.astMainPipe[s32Index].bMetryFixed = HI_TRUE;
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
