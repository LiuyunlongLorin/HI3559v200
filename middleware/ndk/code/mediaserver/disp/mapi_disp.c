/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_disp.c
 * @brief   disp server function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#include <string.h>
#include <pthread.h>

#include "hi_mapi_comm_define.h"
#include "hi_mapi_vproc_define.h"
#include "hi_mapi_disp_define.h"
#include "hi_comm_vo.h"
#include "hi_comm_region.h"
#include "mpi_vo.h"
#include "mpi_sys.h"
#include "hi_defines.h"
#include "mpi_region.h"
#include "hi_mapi_log.h"
#include "mapi_disp_inner.h"
#include "mapi_comm_inner.h"
#include "mapi_disp_adapt.h"
#include "mapi_disp_arch.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static MAPI_DISP_CONTEXT_S g_astDispCtx[HI_MAPI_DISP_MAX_NUM];
static pthread_mutex_t g_Disp_FuncLock[HI_MAPI_DISP_MAX_NUM] = { PTHREAD_MUTEX_INITIALIZER };

static HI_S32 MAPI_DISP_ResetStatus(HI_VOID)
{
    HI_U32 i, j, k;

    for (i = 0; i < HI_MAPI_DISP_MAX_NUM; i++) {
        g_astDispCtx[i].bDispStarted = HI_FALSE;
        for (j = 0; j < HI_MAPI_DISP_WND_MAX_NUM; j++) {
            g_astDispCtx[i].astWndAttr[j].bWindowStarted = HI_FALSE;
            g_astDispCtx[i].astWndAttr[j].bWindowBinded = HI_FALSE;
            for (k = 0; k < HI_MAPI_DISP_OSD_MAX_NUM; k++) {
                g_astDispCtx[i].astWndAttr[j].g_astDispOsdAttr[k].bOsdStarted = HI_FALSE;
            }
        }
    }
    return HI_SUCCESS;
}

HI_S32 MAPI_DISP_Init(HI_VOID)
{
    return MAPI_DISP_ResetStatus();
}

HI_S32 MAPI_DISP_Deinit(HI_VOID)
{
    return MAPI_DISP_ResetStatus();
}

static HI_S32 MAPI_DISP_GetVideoLayerSize(const VO_PUB_ATTR_S *pstPubAttr, HI_U32 *pu32Width, HI_U32 *pu32Height,
                                          HI_U32 *pu32FrameRate)
{
    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;
    HI_U32 u32FrameRate = 0;

    switch (pstPubAttr->enIntfSync) {
        case VO_OUTPUT_PAL:
            u32Width = 720;
            u32Height = 576;
            u32FrameRate = 25;
            break;
        case VO_OUTPUT_NTSC:
            u32Width = 720;
            u32Height = 480;
            u32FrameRate = 30;
            break;
        case VO_OUTPUT_1080P24:
            u32Width = 1920;
            u32Height = 1080;
            u32FrameRate = 24;
            break;
        case VO_OUTPUT_1080P25:
            u32Width = 1920;
            u32Height = 1080;
            u32FrameRate = 25;
            break;
        case VO_OUTPUT_1080P30:
            u32Width = 1920;
            u32Height = 1080;
            u32FrameRate = 30;
            break;
        case VO_OUTPUT_720P50:
            u32Width = 1280;
            u32Height = 720;
            u32FrameRate = 50;
            break;
        case VO_OUTPUT_720P60:
            u32Width = 1280;
            u32Height = 720;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1080I50:
            u32Width = 1920;
            u32Height = 1080;
            u32FrameRate = 50;
            break;
        case VO_OUTPUT_1080I60:
            u32Width = 1920;
            u32Height = 1080;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1080P50:
            u32Width = 1920;
            u32Height = 1080;
            u32FrameRate = 50;
            break;
        case VO_OUTPUT_1080P60:
            u32Width = 1920;
            u32Height = 1080;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_576P50:
            u32Width = 720;
            u32Height = 576;
            u32FrameRate = 50;
            break;
        case VO_OUTPUT_480P60:
            u32Width = 720;
            u32Height = 480;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_800x600_60:
            u32Width = 800;
            u32Height = 600;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1024x768_60:
            u32Width = 1024;
            u32Height = 768;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1280x1024_60:
            u32Width = 1280;
            u32Height = 1024;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1366x768_60:
            u32Width = 1366;
            u32Height = 768;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1440x900_60:
            u32Width = 1440;
            u32Height = 900;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1280x800_60:
            u32Width = 1280;
            u32Height = 800;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1600x1200_60:
            u32Width = 1600;
            u32Height = 1200;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1680x1050_60:
            u32Width = 1680;
            u32Height = 1050;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1920x1200_60:
            u32Width = 1920;
            u32Height = 1200;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_640x480_60:
            u32Width = 640;
            u32Height = 480;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_960H_PAL:
            u32Width = 960;
            u32Height = 576;
            u32FrameRate = 50;
            break;
        case VO_OUTPUT_960H_NTSC:
            u32Width = 960;
            u32Height = 480;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1920x2160_30:
            u32Width = 1920;
            u32Height = 2160;
            u32FrameRate = 30;
            break;
        case VO_OUTPUT_2560x1440_30:
            u32Width = 2560;
            u32Height = 1440;
            u32FrameRate = 30;
            break;
        case VO_OUTPUT_2560x1600_60:
            u32Width = 2560;
            u32Height = 1600;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_3840x2160_24:
            u32Width = 3840;
            u32Height = 2160;
            u32FrameRate = 24;
            break;
        case VO_OUTPUT_3840x2160_25:
            u32Width = 3840;
            u32Height = 2160;
            u32FrameRate = 25;
            break;
        case VO_OUTPUT_3840x2160_30:
            u32Width = 3840;
            u32Height = 2160;
            u32FrameRate = 30;
            break;
        case VO_OUTPUT_3840x2160_50:
            u32Width = 3840;
            u32Height = 2160;
            u32FrameRate = 50;
            break;
        case VO_OUTPUT_3840x2160_60:
            u32Width = 3840;
            u32Height = 2160;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_4096x2160_24:
            u32Width = 4096;
            u32Height = 2160;
            u32FrameRate = 24;
            break;
        case VO_OUTPUT_4096x2160_25:
            u32Width = 4096;
            u32Height = 2160;
            u32FrameRate = 25;
            break;
        case VO_OUTPUT_4096x2160_30:
            u32Width = 4096;
            u32Height = 2160;
            u32FrameRate = 30;
            break;
        case VO_OUTPUT_4096x2160_50:
            u32Width = 4096;
            u32Height = 2160;
            u32FrameRate = 50;
            break;
        case VO_OUTPUT_4096x2160_60:
            u32Width = 4096;
            u32Height = 2160;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_320x240_60:
            u32Width = 320;
            u32Height = 240;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_320x240_50:
            u32Width = 320;
            u32Height = 240;
            u32FrameRate = 50;
            break;
        case VO_OUTPUT_240x320_50:
            u32Width = 240;
            u32Height = 320;
            u32FrameRate = 50;
            break;
        case VO_OUTPUT_240x320_60:
            u32Width = 240;
            u32Height = 320;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_800x600_50:
            u32Width = 800;
            u32Height = 600;
            u32FrameRate = 50;
            break;
        case VO_OUTPUT_720x1280_60:
            u32Width = 720;
            u32Height = 1280;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_1080x1920_60:
            u32Width = 1080;
            u32Height = 1920;
            u32FrameRate = 60;
            break;
        case VO_OUTPUT_7680x4320_30:
            u32Width = 7680;
            u32Height = 4320;
            u32FrameRate = 30;
            break;
        case VO_OUTPUT_USER:
            u32Width = pstPubAttr->stSyncInfo.u16Hact;  // stSyncInfo.u16Hact;
            u32Height = (pstPubAttr->stSyncInfo.bIop) ? pstPubAttr->stSyncInfo.u16Vact :
                pstPubAttr->stSyncInfo.u16Vact * 2;
            u32FrameRate = 60;  // todo
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MAPI_GetVideoLayerSize ERR.not support enIntfSync[%d]\n",
                           pstPubAttr->enIntfSync);
            return HI_FAILURE;
    }

    *pu32Width = u32Width;
    *pu32Height = u32Height;
    *pu32FrameRate = u32FrameRate;

    return HI_SUCCESS;
}

static HI_S32 MAPI_DISP_SetUserIntfSyncInfo(HI_HANDLE DispHdl,
    HI_MAPI_DISP_USERINFO_ATTR_S *pstUserInfoAttr)
{
    HI_S32 s32Ret;

    if (pstUserInfoAttr->u32DevFrameRate == 0) {
        pstUserInfoAttr->u32DevFrameRate = 60;
    }

    s32Ret = HI_MPI_VO_SetDevFrameRate(DispHdl, pstUserInfoAttr->u32DevFrameRate);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SetDevFrameRate fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_SetUserIntfSyncInfo(DispHdl, &pstUserInfoAttr->stUserInfo);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SetUserIntfSyncInfo fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_Init(HI_HANDLE DispHdl, const HI_MAPI_DISP_ATTR_S *pstDispAttr)
{
    HI_MAPI_DISP_ATTR_S stDispAttr;
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_NULL_PTR(pstDispAttr);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    if (g_astDispCtx[DispHdl].bDispInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Disp is already inited\n");
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return HI_SUCCESS;
    }

    memcpy(&stDispAttr, pstDispAttr, sizeof(HI_MAPI_DISP_ATTR_S));

    s32Ret = HI_MPI_VO_SetPubAttr(DispHdl, &stDispAttr.stPubAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SetPubAttr fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    if (stDispAttr.stPubAttr.enIntfSync == VO_OUTPUT_USER) {
        s32Ret = MAPI_DISP_SetUserIntfSyncInfo(DispHdl, &stDispAttr.stUserInfoAttr);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MAPI_DISP_SetUserIntfSyncInfo fail s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
            return s32Ret;
        }
    }

    s32Ret = HI_MPI_VO_Enable(DispHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_Enable fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    s32Ret = MAPI_DISP_GetVideoLayerSize(&stDispAttr.stPubAttr, &g_astDispCtx[DispHdl].u32VLWidth,
                                         &g_astDispCtx[DispHdl].u32VLHeight, &g_astDispCtx[DispHdl].u32VLFrameRate);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MAPI_DISP_GetVideoLayerSize fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    memcpy(&g_astDispCtx[DispHdl].stDispAttr, &stDispAttr, sizeof(HI_MAPI_DISP_ATTR_S));

    g_astDispCtx[DispHdl].bDispInited = HI_TRUE;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_DISP_Deinit(HI_HANDLE DispHdl)
{
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    s32Ret = HI_MPI_VO_Disable(DispHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_Disable fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    g_astDispCtx[DispHdl].bDispInited = HI_FALSE;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_DISP_SetAttrEx(HI_HANDLE DispHdl, HI_MAPI_DISP_CMD_E enCMD, HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_NULL_PTR(pAttr);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    switch (enCMD) {
        case HI_MAPI_DISP_CMD_VIDEO_CSC: {
            DISP_CHECK_VIDEO_CSC_HANDLE(DispHdl);
            if (u32Len != sizeof(HI_MAPI_DISP_CSCATTREX_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Disp_SETCSC u32Len:%x is not the size of HI_MAPI_DISP_CSCATTREX_S\n",
                               u32Len);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }

            VO_CSC_S stVoCSC;
            HI_MAPI_DISP_CSCATTREX_S *pstDispAttrEx = (HI_MAPI_DISP_CSCATTREX_S *)pAttr;
            stVoCSC.enCscMatrix = pstDispAttrEx->stVoCSC.enCscMatrix;
            stVoCSC.u32Luma = pstDispAttrEx->stVoCSC.u32Luma;
            stVoCSC.u32Contrast = pstDispAttrEx->stVoCSC.u32Contrast;
            stVoCSC.u32Hue = pstDispAttrEx->stVoCSC.u32Hue;
            stVoCSC.u32Satuature = pstDispAttrEx->stVoCSC.u32Satuature;

            s32Ret = HI_MPI_VO_SetVideoLayerCSC(DispHdl, &stVoCSC);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SetVideoLayerCSC fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return s32Ret;
            }
            break;
        }

        case HI_MAPI_DISP_CMD_GRAPHIC_CSC: {
            DISP_CHECK_GRAPHIC_CSC_HANDLE(DispHdl);
            if (u32Len != sizeof(HI_MAPI_DISP_CSCATTREX_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Disp_SETCSC u32Len:%x is not the size of HI_MAPI_DISP_CSCATTREX_S\n",
                               u32Len);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }

            VO_CSC_S stVoCSC;
            HI_MAPI_DISP_CSCATTREX_S *pstDispAttrEx = (HI_MAPI_DISP_CSCATTREX_S *)pAttr;
            stVoCSC.enCscMatrix = pstDispAttrEx->stVoCSC.enCscMatrix;
            stVoCSC.u32Luma = pstDispAttrEx->stVoCSC.u32Luma;
            stVoCSC.u32Contrast = pstDispAttrEx->stVoCSC.u32Contrast;
            stVoCSC.u32Hue = pstDispAttrEx->stVoCSC.u32Hue;
            stVoCSC.u32Satuature = pstDispAttrEx->stVoCSC.u32Satuature;

            s32Ret = HI_MPI_VO_SetGraphicLayerCSC(DispHdl, &stVoCSC);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SetVideoLayerCSC fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return s32Ret;
            }
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Do not support cmd %d.\n", enCMD);
            MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
            return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_GetAttrEx(HI_HANDLE DispHdl, HI_MAPI_DISP_CMD_E enCMD, HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_NULL_PTR(pAttr);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    switch (enCMD) {
        case HI_MAPI_DISP_CMD_VIDEO_CSC: {
            DISP_CHECK_VIDEO_CSC_HANDLE(DispHdl);
            if (u32Len != sizeof(HI_MAPI_DISP_CSCATTREX_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Disp_SETCSC u32Len:%x is not the size of HI_MAPI_DISP_CSCATTREX_S\n",
                               u32Len);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }

            VO_CSC_S stVoCSC;
            HI_MAPI_DISP_CSCATTREX_S stDispAttrEx;

            s32Ret = HI_MPI_VO_GetVideoLayerCSC(DispHdl, &stVoCSC);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_GetVideoLayerCSC fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return s32Ret;
            }

            stDispAttrEx.stVoCSC.enCscMatrix = stVoCSC.enCscMatrix;
            stDispAttrEx.stVoCSC.u32Luma = stVoCSC.u32Luma;
            stDispAttrEx.stVoCSC.u32Contrast = stVoCSC.u32Contrast;
            stDispAttrEx.stVoCSC.u32Hue = stVoCSC.u32Hue;
            stDispAttrEx.stVoCSC.u32Satuature = stVoCSC.u32Satuature;

            memcpy(pAttr, &stDispAttrEx, sizeof(HI_MAPI_DISP_CSCATTREX_S));

            break;
        }
        case HI_MAPI_DISP_CMD_GRAPHIC_CSC: {
            DISP_CHECK_GRAPHIC_CSC_HANDLE(DispHdl);
            if (u32Len != sizeof(HI_MAPI_DISP_CSCATTREX_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Disp_SETCSC u32Len:%x is not the size of HI_MAPI_DISP_CSCATTREX_S\n",
                               u32Len);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }

            VO_CSC_S stVoCSC;
            HI_MAPI_DISP_CSCATTREX_S stDispAttrEx;

            s32Ret = HI_MPI_VO_GetGraphicLayerCSC(DispHdl, &stVoCSC);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_GetVideoLayerCSC fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return s32Ret;
            }

            stDispAttrEx.stVoCSC.enCscMatrix = stVoCSC.enCscMatrix;
            stDispAttrEx.stVoCSC.u32Luma = stVoCSC.u32Luma;
            stDispAttrEx.stVoCSC.u32Contrast = stVoCSC.u32Contrast;
            stDispAttrEx.stVoCSC.u32Hue = stVoCSC.u32Hue;
            stDispAttrEx.stVoCSC.u32Satuature = stVoCSC.u32Satuature;

            memcpy(pAttr, &stDispAttrEx, sizeof(HI_MAPI_DISP_CSCATTREX_S));

            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Do not support cmd %d.\n", enCMD);
            MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
            return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_SetWindowAttrEx(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_MAPI_DISP_WINDOW_CMD_E enCMD,
                                    HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pAttr);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    switch (enCMD) {
        case HI_MAPI_DISP_WINDOW_CMD_ROTATE: {
            if (u32Len != sizeof(HI_MAPI_DISP_WINDOW_ROTATE_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP,
                    "Disp_SETROTATE u32Len:%x is not the size of HI_MAPI_DISP_WINDOW_ROTATE_S\n",
                               u32Len);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }

            ROTATION_E enRotate;
            HI_MAPI_DISP_WINDOW_ROTATE_S *penDispRotate = (HI_MAPI_DISP_WINDOW_ROTATE_S *)pAttr;
            enRotate = (ROTATION_E)penDispRotate->enDispRorate;
            s32Ret = HI_MPI_VO_SetChnRotation(DispHdl, WndHdl, enRotate);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SetChnRotation fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return s32Ret;
            }
            break;
        }
        case HI_MAPI_DISP_WINDOW_CMD_ASPECTRATIO: {
            if (u32Len != sizeof(HI_MAPI_DISP_WINDOW_ASPECTRATIO_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP,
                    "Disp_SETROTATE u32Len:%x is not the size of HI_MAPI_DISP_WINDOW_ASPECTRATIO_S\n",
                               u32Len);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }

            HI_MAPI_DISP_WINDOW_ASPECTRATIO_S *pstAspectRatioAttr = (HI_MAPI_DISP_WINDOW_ASPECTRATIO_S *)pAttr;
            s32Ret = HAL_MAPI_SetWindow_AspectRatioAttr(DispHdl, WndHdl, pstAspectRatioAttr);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MAPI_SetWindow_AspectRatioAttr fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return s32Ret;
            }
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Do not support cmd %d.\n", enCMD);
            MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
            return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_GetWindowAttrEx(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_MAPI_DISP_WINDOW_CMD_E enCMD,
                                    HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pAttr);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    switch (enCMD) {
        case HI_MAPI_DISP_WINDOW_CMD_ROTATE: {
            if (u32Len != sizeof(HI_MAPI_DISP_WINDOW_ROTATE_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP,
                    "Disp_SETROTATE u32Len:%x is not the size of HI_MAPI_DISP_WINDOW_ROTATE_S\n", u32Len);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }

            ROTATION_E enRotate;
            HI_MAPI_DISP_WINDOW_ROTATE_S *penDispRotate = (HI_MAPI_DISP_WINDOW_ROTATE_S *)pAttr;
            s32Ret = HI_MPI_VO_GetChnRotation(DispHdl, WndHdl, &enRotate);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_GetChnRotation fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return s32Ret;
            }
            penDispRotate->enDispRorate = enRotate;

            break;
        }
        case HI_MAPI_DISP_WINDOW_CMD_ASPECTRATIO: {
            if (u32Len != sizeof(HI_MAPI_DISP_WINDOW_ASPECTRATIO_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP,
                    "Disp_SETROTATE u32Len:%x is not the size of HI_MAPI_DISP_WINDOW_PARAM_S\n", u32Len);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }

            HI_MAPI_DISP_WINDOW_ASPECTRATIO_S stAspectRatioAttr;
            s32Ret = HAL_MAPI_GetWindow_AspectRatioAttr(DispHdl, WndHdl, &stAspectRatioAttr);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MAPI_GetWindow_AspectRatioAttr fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
                return s32Ret;
            }

            memcpy(pAttr, &stAspectRatioAttr, sizeof(HI_MAPI_DISP_WINDOW_ASPECTRATIO_S));

            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Do not support cmd %d.\n", enCMD);
            MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
            return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_Start(HI_HANDLE DispHdl, HI_MAPI_DISP_VIDEOLAYER_ATTR_S *pstVideoLayerAttr)
{
    HI_S32 s32Ret;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    HI_U32 u32BufLen;

    DISP_CHECK_HANDLE(DispHdl);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    if (g_astDispCtx[DispHdl].bDispStarted) {
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return HI_SUCCESS;
    }

    u32BufLen = pstVideoLayerAttr->u32BufLen;
    s32Ret = HI_MPI_VO_SetDisplayBufLen(DispHdl, u32BufLen);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SetDisplayBufLen fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    stLayerAttr.stDispRect.s32X = DISP_VIDEOLAYER_DISPRECT_X;
    stLayerAttr.stDispRect.s32Y = DISP_VIDEOLAYER_DISPRECT_Y;
    stLayerAttr.stDispRect.u32Width = g_astDispCtx[DispHdl].u32VLWidth;
    stLayerAttr.stDispRect.u32Height = g_astDispCtx[DispHdl].u32VLHeight;

    stLayerAttr.stImageSize.u32Width = pstVideoLayerAttr->stImageSize.u32Width;
    stLayerAttr.stImageSize.u32Height = pstVideoLayerAttr->stImageSize.u32Height;
    if (stLayerAttr.stImageSize.u32Width == 0 && stLayerAttr.stImageSize.u32Height == 0) {
        stLayerAttr.stImageSize.u32Width = g_astDispCtx[DispHdl].u32VLWidth;
        ;
        stLayerAttr.stImageSize.u32Height = g_astDispCtx[DispHdl].u32VLHeight;
    }

    stLayerAttr.u32DispFrmRt = pstVideoLayerAttr->u32VLFrameRate;
    if (stLayerAttr.u32DispFrmRt == 0) {
        stLayerAttr.u32DispFrmRt = g_astDispCtx[DispHdl].u32VLFrameRate;
    }

    stLayerAttr.enPixFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.bClusterMode = HI_FALSE;
    stLayerAttr.enDstDynamicRange = DYNAMIC_RANGE_SDR8;
    s32Ret = HI_MPI_VO_SetVideoLayerAttr(DispHdl, &stLayerAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SetVideoLayerAttr fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_EnableVideoLayer(DispHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_EnableVideoLayer fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    g_astDispCtx[DispHdl].bDispStarted = HI_TRUE;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_Stop(HI_HANDLE DispHdl)
{
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    s32Ret = HI_MPI_VO_DisableVideoLayer(DispHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_DisableVideoLayer fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    g_astDispCtx[DispHdl].bDispStarted = HI_FALSE;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_SetWindowAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                                  const HI_MAPI_DISP_WINDOW_ATTR_S *pstWndAttr)
{
    VO_CHN_ATTR_S stVoChnAttr;
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pstWndAttr);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    stVoChnAttr.u32Priority = pstWndAttr->u32Priority;
    stVoChnAttr.stRect.s32X = pstWndAttr->stRect.s32X;
    stVoChnAttr.stRect.s32Y = pstWndAttr->stRect.s32Y;
    stVoChnAttr.stRect.u32Width = pstWndAttr->stRect.u32Width;
    stVoChnAttr.stRect.u32Height = pstWndAttr->stRect.u32Height;
    stVoChnAttr.bDeflicker = HI_FALSE;
    s32Ret = HI_MPI_VO_SetChnAttr(DispHdl, WndHdl, &stVoChnAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SetChnAttr fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_GetWindowAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl,
    HI_MAPI_DISP_WINDOW_ATTR_S *pstWndAttr)
{
    VO_CHN_ATTR_S stVoChnAttr;
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pstWndAttr);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    s32Ret = HI_MPI_VO_GetChnAttr(DispHdl, WndHdl, &stVoChnAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_GetChnAttr fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    pstWndAttr->u32Priority = stVoChnAttr.u32Priority;
    pstWndAttr->stRect.s32X = stVoChnAttr.stRect.s32X;
    pstWndAttr->stRect.s32Y = stVoChnAttr.stRect.s32Y;
    pstWndAttr->stRect.u32Width = stVoChnAttr.stRect.u32Width;
    pstWndAttr->stRect.u32Height = stVoChnAttr.stRect.u32Height;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_StartWindow(HI_HANDLE DispHdl, HI_HANDLE WndHdl)
{
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    if (g_astDispCtx[DispHdl].astWndAttr[WndHdl].bWindowStarted) {
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return HI_SUCCESS;
    }
    s32Ret = HI_MPI_VO_EnableChn(DispHdl, WndHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_EnableChn fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    g_astDispCtx[DispHdl].astWndAttr[WndHdl].bWindowStarted = HI_TRUE;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_StopWindow(HI_HANDLE DispHdl, HI_HANDLE WndHdl)
{
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    if (!g_astDispCtx[DispHdl].astWndAttr[WndHdl].bWindowStarted) {
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_VO_DisableChn(DispHdl, WndHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_DisableChn fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    g_astDispCtx[DispHdl].astWndAttr[WndHdl].bWindowStarted = HI_FALSE;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_ClearWindow(HI_HANDLE DispHdl, HI_HANDLE WndHdl)
{
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    s32Ret = HI_MPI_VO_ClearChnBuf(DispHdl, WndHdl, HI_TRUE);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_ClearChnBuf fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_Bind_VProc(HI_HANDLE GrpHdl, HI_HANDLE VPortHdl, HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                               HI_BOOL bStitch)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_VPSSDVE_HANDLE(GrpHdl);
    DISP_CHECK_VPSSVPORT_HANDLE(VPortHdl);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    // in fastboot version litos system has already binded,  if binding again in linux system will cause problem
    DISP_CHECK_BIND(g_astDispCtx[DispHdl].astWndAttr[WndHdl].bWindowBinded);

    if (bStitch) {
        stSrcChn.enModId = HI_ID_AVS;
        stSrcChn.s32DevId = GrpHdl;
        stSrcChn.s32ChnId = VPortHdl;
    } else {
        stSrcChn.enModId = HI_ID_VPSS;
        stSrcChn.s32DevId = GrpHdl;
        stSrcChn.s32ChnId = VPortHdl;
    }
    stDestChn.enModId = HI_ID_VO;
    stDestChn.s32DevId = DispHdl;
    stDestChn.s32ChnId = WndHdl;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "DispHdl(%d, %d) bind to Vproc (%d, %d) ERR.\n", DispHdl, WndHdl, GrpHdl,
                       VPortHdl);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    g_astDispCtx[DispHdl].astWndAttr[WndHdl].bWindowBinded = HI_TRUE;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_UnBind_VProc(HI_HANDLE GrpHdl, HI_HANDLE VPortHdl, HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                                 HI_BOOL bStitch)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_VPSSDVE_HANDLE(GrpHdl);
    DISP_CHECK_VPSSVPORT_HANDLE(VPortHdl);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    if (bStitch) {
        stSrcChn.enModId = HI_ID_AVS;
        stSrcChn.s32DevId = GrpHdl;
        stSrcChn.s32ChnId = VPortHdl;
    } else {
        stSrcChn.enModId = HI_ID_VPSS;
        stSrcChn.s32DevId = GrpHdl;
        stSrcChn.s32ChnId = VPortHdl;
    }

    stDestChn.enModId = HI_ID_VO;
    stDestChn.s32DevId = DispHdl;
    stDestChn.s32ChnId = WndHdl;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "DispHdl(%d, %d) unbind to Vproc (%d, %d) ERR.\n", DispHdl, WndHdl, GrpHdl,
                       VPortHdl);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    g_astDispCtx[DispHdl].astWndAttr[WndHdl].bWindowBinded = HI_FALSE;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_Bind_VCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE DispHdl,
    HI_HANDLE WndHdl)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_VCAPPIPE_HANDLE(VcapPipeHdl);
    DISP_CHECK_VCAPCHN_HANDLE(PipeChnHdl);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    // in fastboot version litos system has already binded,  if binding again in linux system will cause problem
    DISP_CHECK_BIND(g_astDispCtx[DispHdl].astWndAttr[WndHdl].bWindowBinded);

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    stSrcChn.enModId = HI_ID_VI;
    stSrcChn.s32DevId = ViPipe;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VO;
    stDestChn.s32DevId = DispHdl;
    stDestChn.s32ChnId = WndHdl;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "DispHdl(%d, %d) bind to Vcap (%d, %d) ERR.\n", DispHdl, WndHdl, VcapPipeHdl,
                       PipeChnHdl);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    g_astDispCtx[DispHdl].astWndAttr[WndHdl].bWindowBinded = HI_TRUE;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_UnBind_VCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE DispHdl,
    HI_HANDLE WndHdl)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_VCAPPIPE_HANDLE(VcapPipeHdl);
    DISP_CHECK_VCAPCHN_HANDLE(PipeChnHdl);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    stSrcChn.enModId = HI_ID_VI;
    stSrcChn.s32DevId = ViPipe;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VO;
    stDestChn.s32DevId = DispHdl;
    stDestChn.s32ChnId = WndHdl;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "DispHdl(%d, %d) unbind to Vcap (%d, %d) ERR.\n", DispHdl, WndHdl, VcapPipeHdl,
                       PipeChnHdl);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    g_astDispCtx[DispHdl].astWndAttr[WndHdl].bWindowBinded = HI_FALSE;

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_SendFrame(HI_HANDLE DispHdl, HI_HANDLE WndHdl, const HI_MAPI_FRAME_DATA_S *pstFramedata)
{
    VIDEO_FRAME_INFO_S stVideoFrame;
    HI_S32 s32Ret;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pstFramedata);

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    DISP_CHECK_INIT(g_astDispCtx[DispHdl].bDispInited);

    stVideoFrame.u32PoolId = pstFramedata->u32PoolId;
    stVideoFrame.enModId = HI_ID_USER;

    if (pstFramedata->enFrameDataType != HI_FRAME_DATA_TYPE_YUV) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "DispHdl(%d, %d) sendFrame is not YUV.\n", DispHdl, WndHdl);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    stVideoFrame.stVFrame.u32Width = pstFramedata->u32Width;
    stVideoFrame.stVFrame.u32Height = pstFramedata->u32Height;
    stVideoFrame.stVFrame.enField = VIDEO_FIELD_FRAME;

    stVideoFrame.stVFrame.enPixelFormat = pstFramedata->enPixelFormat;
    stVideoFrame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
    stVideoFrame.stVFrame.enCompressMode = pstFramedata->enCompressMode;

    stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVideoFrame.stVFrame.enColorGamut = COLOR_GAMUT_BT709;

    stVideoFrame.stVFrame.u32HeaderStride[0] = 0;
    stVideoFrame.stVFrame.u32HeaderStride[1] = 0;
    stVideoFrame.stVFrame.u32HeaderStride[2] = 0;

    stVideoFrame.stVFrame.u32Stride[0] = pstFramedata->u32Stride[0];
    stVideoFrame.stVFrame.u32Stride[1] = pstFramedata->u32Stride[1];
    stVideoFrame.stVFrame.u32Stride[2] = pstFramedata->u32Stride[2];

    stVideoFrame.stVFrame.u32ExtStride[0] = 0;
    stVideoFrame.stVFrame.u32ExtStride[1] = 0;
    stVideoFrame.stVFrame.u32ExtStride[2] = 0;

    stVideoFrame.stVFrame.u64HeaderPhyAddr[0] = 0;
    stVideoFrame.stVFrame.u64HeaderPhyAddr[1] = 0;
    stVideoFrame.stVFrame.u64HeaderPhyAddr[2] = 0;

    stVideoFrame.stVFrame.u64HeaderVirAddr[0] = 0;
    stVideoFrame.stVFrame.u64HeaderVirAddr[1] = 0;
    stVideoFrame.stVFrame.u64HeaderVirAddr[2] = 0;

    stVideoFrame.stVFrame.u64PhyAddr[0] = pstFramedata->u64PhyAddr[0];
    stVideoFrame.stVFrame.u64PhyAddr[1] = pstFramedata->u64PhyAddr[1];
    stVideoFrame.stVFrame.u64PhyAddr[2] = pstFramedata->u64PhyAddr[2];

    stVideoFrame.stVFrame.u64VirAddr[0] = pstFramedata->u64VirAddr[0];
    stVideoFrame.stVFrame.u64VirAddr[1] = pstFramedata->u64VirAddr[1];
    stVideoFrame.stVFrame.u64VirAddr[2] = pstFramedata->u64VirAddr[2];

    stVideoFrame.stVFrame.u64ExtPhyAddr[0] = 0;
    stVideoFrame.stVFrame.u64ExtPhyAddr[1] = 0;
    stVideoFrame.stVFrame.u64ExtPhyAddr[2] = 0;

    stVideoFrame.stVFrame.u64ExtVirAddr[0] = 0;
    stVideoFrame.stVFrame.u64ExtVirAddr[1] = 0;
    stVideoFrame.stVFrame.u64ExtVirAddr[2] = 0;

    stVideoFrame.stVFrame.s16OffsetTop = 0;
    stVideoFrame.stVFrame.s16OffsetBottom = 0;
    stVideoFrame.stVFrame.s16OffsetLeft = 0;
    stVideoFrame.stVFrame.s16OffsetRight = 0;

    stVideoFrame.stVFrame.u32MaxLuminance = DISP_VIDEOFRAME_LUMINANCE_MAX;
    stVideoFrame.stVFrame.u32MinLuminance = DISP_VIDEOFRAME_LUMINANCE_MIN;

    stVideoFrame.stVFrame.u32TimeRef = 0;

    stVideoFrame.stVFrame.u64PTS = pstFramedata->u64pts;

    stVideoFrame.stVFrame.u64PrivateData = 0;

    stVideoFrame.stVFrame.u32FrameFlag = 0;

    memcpy(&stVideoFrame.stVFrame.stSupplement, &pstFramedata->stVideoSupplement, sizeof(VIDEO_SUPPLEMENT_S));

    s32Ret = HI_MPI_VO_SendFrame(DispHdl, WndHdl, &stVideoFrame, 0);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SendFrame fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_SetWindowOSDAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                     const HI_MAPI_OSD_ATTR_S *pstOsdAttr)
{
    HI_S32 s32Ret;

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    s32Ret = HAL_MAPI_DISP_SetWindowOSDAttr(DispHdl, WndHdl, OsdHdl, pstOsdAttr,
                                            &g_astDispCtx[DispHdl].astWndAttr[WndHdl].g_astDispOsdAttr[OsdHdl]);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HAL_MAPI_DISP_SetWindowOSDAttr return err:%x\n", s32Ret);
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_DISP_GetWindowOSDAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                     HI_MAPI_OSD_ATTR_S *pstOsdAttr)
{
    HI_S32 s32Ret;

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    s32Ret = HAL_MAPI_DISP_GetWindowOSDAttr(DispHdl, WndHdl, OsdHdl, pstOsdAttr,
                                            &g_astDispCtx[DispHdl].astWndAttr[WndHdl].g_astDispOsdAttr[OsdHdl]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HAL_MAPI_DISP_GetWindowOSDAttr return err:%x\n", s32Ret);
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_DISP_StartWindowOSD(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl)
{
    HI_S32 s32Ret;

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    s32Ret = HAL_MAPI_DISP_StartWindowOSD(DispHdl, WndHdl, OsdHdl,
                                          &g_astDispCtx[DispHdl].astWndAttr[WndHdl].g_astDispOsdAttr[OsdHdl]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HAL_MAPI_DISP_StartWindowOSD return err:%x\n", s32Ret);
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_DISP_StopWindowOSD(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl)
{
    HI_S32 s32Ret;

    MUTEX_LOCK(g_Disp_FuncLock[DispHdl]);

    s32Ret = HAL_MAPI_DISP_StopWindowOSD(DispHdl, WndHdl, OsdHdl,
                                         &g_astDispCtx[DispHdl].astWndAttr[WndHdl].g_astDispOsdAttr[OsdHdl]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HAL_MAPI_DISP_StartWindowOSD return err:%x\n", s32Ret);
    }

    MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


