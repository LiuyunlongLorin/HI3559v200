/**
 * @file      hi_product_init_service.c
 * @brief     service init source
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 * @version   1.0

 */

#include "asm/io.h"

#include "hi_product_init_os.h"
#include "hi_product_init_service.h"
#include "hi_product_media.h"
#include "hi_product_param.h"
#include "hi_product_scene_msg_server.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_disp.h"
#include "hi_mapi_aenc.h"
#include "hi_avplay_server_ipcmsg.h"
#include "hi_hal_screen.h"
#include "hi_appcomm_msg_server.h"
#include "hi_timestamp.h"
#include "hi_system.h"
#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifdef CFG_RAW_PARAM_ON

#ifdef CFG_BOOT_LOGO_ON
#include "bootlogo.dat"
#endif

#ifdef CFG_BOOT_SOUND_ON
#include "bootsound.dat"
#endif

/** ae converence maximum framerate */
#define AECONVERGENCE_MAX_FRAMERATE 120.0f

/** ae converence timeout value, ms */
#define AECONVERGENCE_TIME_OUT    (550)

/** recmng play fps(for lapse) */
#define PDT_INIT_RECMNG_LAPSE_PLAY_FPS         (30)

/** venc handle */
#define PDT_INIT_MAIN_STREAM_VENC_HDL          (0)
#define PDT_INIT_SUB_STREAM_VENC_HDL           (1)

/**
 * @brief     get current system time
 * @return    system in ms
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/31
 */
static HI_U64 PDT_INIT_GetCurTime(HI_VOID)
{
    struct timeval tv;
    int64_t us;
    gettimeofday(&tv, NULL);
    us = (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return us;
}

/**
 * @brief     update ae converence video configure,
 *            update vcap resolution and framerate according sensor mode, and disable other modules
 * @param[in/out] pstVideoCfg : video configure
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/9/25
 */
static HI_S32 PDT_INIT_UpdateAEConvVideoCfg(HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr = NULL;

    HI_S32 s32SnsModesCnt = 0;
    HI_MAPI_SENSOR_MODE_S* pstSnsModes = NULL;
    HI_MEDIA_SENSOR_ATTR_S stSnsAttr;
    HI_FLOAT fMaxFrameRate = 0.0f;

    HI_S32 i, j, k;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        pstVcapDevAttr = &pstVideoCfg->stVcapCfg.astVcapDevAttr[i];
        if (!pstVcapDevAttr->bEnable)
        {
            continue;
        }
        memcpy(&stSnsAttr, &pstVcapDevAttr->stSnsAttr, sizeof(HI_MEDIA_SENSOR_ATTR_S));
        MLOGI(YELLOW"MediaMode Res[%ux%u] SnsMode[%u]"NONE"\n",
                    stSnsAttr.stResolution.u32Width, stSnsAttr.stResolution.u32Height,
                    stSnsAttr.u32SnsMode);

        s32Ret = HI_MAPI_VCAP_GetSensorModesCnt(pstVcapDevAttr->VcapDevHdl, &s32SnsModesCnt);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "GetSnsModesCnt");

        pstSnsModes = (HI_MAPI_SENSOR_MODE_S*)malloc(sizeof(HI_MAPI_SENSOR_MODE_S) * s32SnsModesCnt);
        HI_APPCOMM_CHECK_POINTER(pstSnsModes, HI_FAILURE);

        s32Ret = HI_MAPI_VCAP_GetSensorAllModes(pstVcapDevAttr->VcapDevHdl, s32SnsModesCnt, pstSnsModes);
        if (HI_SUCCESS != s32Ret)
        {
            HI_APPCOMM_SAFE_FREE(pstSnsModes);
            MLOGE("GetSnsModes Fail\n");
            return HI_FAILURE;
        }
        for (j = 0; j < s32SnsModesCnt; ++j)
        {
            if (fMaxFrameRate < pstSnsModes[j].f32SnsMaxFrameRate)
            {
                fMaxFrameRate = pstSnsModes[j].f32SnsMaxFrameRate;
                stSnsAttr.stResolution.u32Width  = pstSnsModes[j].s32Width;
                stSnsAttr.stResolution.u32Height = pstSnsModes[j].s32Height;
                stSnsAttr.enWdrMode  = pstSnsModes[j].enWdrMode;
                stSnsAttr.u32SnsMode = pstSnsModes[j].u8SnsMode;
                MLOGI("Res[%ux%u] MaxFrmRate[%f] SnsMode[%u]\n",
                    stSnsAttr.stResolution.u32Width, stSnsAttr.stResolution.u32Height,
                    fMaxFrameRate, stSnsAttr.u32SnsMode);
            }
        }
        HI_APPCOMM_SAFE_FREE(pstSnsModes);

        if (AECONVERGENCE_MAX_FRAMERATE < fMaxFrameRate)
        {
            fMaxFrameRate = AECONVERGENCE_MAX_FRAMERATE;
        }
        MLOGI(YELLOW"AE Res[%ux%u] MaxFrmRate[%f] SnsMode[%u]"NONE"\n",
                    stSnsAttr.stResolution.u32Width, stSnsAttr.stResolution.u32Height,
                    fMaxFrameRate, stSnsAttr.u32SnsMode);

        /* update vcap attribute */
        memcpy(&pstVcapDevAttr->stSnsAttr, &stSnsAttr, sizeof(stSnsAttr));
        pstVcapDevAttr->enWdrMode = stSnsAttr.enWdrMode;
        pstVcapDevAttr->stResolution = stSnsAttr.stResolution;

        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            if (!pstVcapDevAttr->astVcapPipeAttr[j].bEnable)
            {
                continue;
            }
            pstVcapDevAttr->astVcapPipeAttr[j].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;
            pstVcapDevAttr->astVcapPipeAttr[j].stFrameRate.s32DstFrameRate = -1;
            pstVcapDevAttr->astVcapPipeAttr[j].stFrameRate.s32SrcFrameRate = -1;

            pstVcapDevAttr->astVcapPipeAttr[j].stIspPubAttr.stSize = stSnsAttr.stResolution;
            pstVcapDevAttr->astVcapPipeAttr[j].stIspPubAttr.f32FrameRate = fMaxFrameRate;

            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if (!pstVcapDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].bEnable)
                {
                    continue;
                }

                pstVcapDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDestResolution = stSnsAttr.stResolution;
                pstVcapDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stFrameRate.s32DstFrameRate = -1;
                pstVcapDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stFrameRate.s32SrcFrameRate = -1;
            }
        }
    }

    /* disable venc */
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        pstVideoCfg->astVencCfg[i].bEnable = HI_FALSE;
    }

    /* disable vpss */
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        pstVideoCfg->stVprocCfg.astVpssAttr[i].bEnable = HI_FALSE;
    }

    /* disable osd */
    pstVideoCfg->stOsdCfg.stTimeOsd.bShow = HI_FALSE;

    return HI_SUCCESS;
}


/**
 * @brief     get first video vcaphdl
 * @param[in] pstVideoCfg : video configure
 * @param[out]VcapHdl : first video vcaphdl
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/31
 */
static HI_S32 PDT_INIT_GetFirstVideoVcapHdl(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg, VI_PIPE* VcapHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        if (!pstVideoCfg->stVcapCfg.astVcapDevAttr[i].bEnable)
        {
            continue;
        }
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            const HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstPipeAttr =
                &pstVideoCfg->stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j];
            if (!pstPipeAttr->bEnable || (HI_MAPI_PIPE_TYPE_SNAP == pstPipeAttr->enPipeType))
            {
                continue;
            }
            *VcapHdl = (VI_PIPE)pstPipeAttr->VcapPipeHdl;
            MLOGI("FirstVideo VcapHdl[%d] FrameRate[%f]\n", *VcapHdl,
                pstPipeAttr->stIspPubAttr.f32FrameRate);
            break;
        }
    }
    return s32Ret;
}

/**
 * @brief     check ae converence status
 * @param[in] VcapHdl : video vcaphdl
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/31
 */
static HI_S32 PDT_INIT_CheckAEConverence(VI_PIPE VcapHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64TimeStart = PDT_INIT_GetCurTime();
    HI_U64 u64TimeCur   = 0;

    if (VcapHdl < 0)
    {
        MLOGE("Illegal VcapPipe[%d] for AE converence!\n", VcapHdl);
        return HI_FAILURE;
    }

    ISP_EXP_INFO_S stIspExpInfo;
    ISP_EXPOSURE_ATTR_S stExposureAttr;
    s32Ret = HI_MPI_ISP_GetExposureAttr(VcapHdl, &stExposureAttr);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MPI_ISP_GetExposureAttr");
    stExposureAttr.stAuto.u8Speed =120;
    s32Ret = HI_MPI_ISP_SetExposureAttr(VcapHdl, &stExposureAttr);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MPI_ISP_SetExposureAttr");
    ISP_VD_TYPE_E eVdType=ISP_VD_FE_START;
    while (1)
    {
        s32Ret = HI_MPI_ISP_QueryExposureInfo(VcapHdl, &stIspExpInfo);
        if (HI_SUCCESS != s32Ret)
        {
#if 0
            if (++s32WaitReadyCnt > 5) /**< 5frames */
            {
                MLOGE(RED"wait ISP AE ready failed!!!"NONE"\n");
                return HI_FAILURE;
            }
            MLOGW(RED"HI_MPI_ISP_QueryExposureInfo failed[%x]!!!"NONE"\n", s32Ret);
#endif
        }

        u64TimeCur = PDT_INIT_GetCurTime();
        //MLOGD(YELLOW"FirstStableTime: %u"NONE"\n", stIspExpInfo.u32FirstStableTime);

        if (0 != stIspExpInfo.u32FirstStableTime) /* finish */
        {
            MLOGI(YELLOW"FirstStableTime: %u"NONE"\n", stIspExpInfo.u32FirstStableTime);
            dprintf("--> u32Exposure:%u u32LinesPer500ms:%u \n",
                    stIspExpInfo.u32Exposure, stIspExpInfo.u32LinesPer500ms);
            return HI_SUCCESS;
        }

        if ((u64TimeCur - u64TimeStart) > AECONVERGENCE_TIME_OUT) /* timeout */
        {
            MLOGE(RED"wait time out!!!"NONE"\n");
            return HI_SUCCESS;
        }
         /* timeout 40 interrup*/
        HI_MPI_ISP_GetVDTimeOut(VcapHdl,eVdType,40);
    }
    return HI_FAILURE;
}

/**
 * @brief     ae converence
 * @param[in] pstVideoCfg : video configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/31
 */
static HI_S32 PDT_INIT_AEConverence(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* TODO:  */
    HI_usleep(50000);

    HI_TIME_STAMP;
    s32Ret = HI_PDT_MEDIA_VideoInit(pstVideoCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_VideoInit");
    HI_TIME_STAMP;

    VI_PIPE VcapHdl = -1;
    s32Ret = PDT_INIT_GetFirstVideoVcapHdl(pstVideoCfg, &VcapHdl);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_INIT_GetFirstVideoVcapHdl");

    s32Ret = PDT_INIT_CheckAEConverence(VcapHdl);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_INIT_CheckAEConverence");
    HI_TIME_STAMP;
    return s32Ret;
}

#if !defined(CFG_LITEOS_NO_PRE_RECORD)
/**
 * @brief     Power On Pre Rec
 * @param[in] pstMediaCfg : Media configure
 * @param[in] enWorkMode  : WorkMode
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/31
 */
static HI_S32 PDT_INIT_PowerOnPreRec(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_PDT_WORKMODE_E enWorkMode)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    HI_S32 j = 0;

    HI_PDT_WORKMODE_CFG_S stWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enWorkMode, &stWorkModeCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_PDT_REC_SRC_S* pstRecDataSrc = NULL;
    switch (enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            pstRecDataSrc = &(stWorkModeCfg.unModeAttr.stNormRecAttr.stDataSrc);
            break;
        case HI_PDT_WORKMODE_LOOP_REC:
            pstRecDataSrc = &(stWorkModeCfg.unModeAttr.stLoopRecAttr.stDataSrc);
            break;
        case HI_PDT_WORKMODE_LPSE_REC:
            pstRecDataSrc = &(stWorkModeCfg.unModeAttr.stLapseRecAttr.stDataSrc);
            break;
        case HI_PDT_WORKMODE_SLOW_REC:
            pstRecDataSrc = &(stWorkModeCfg.unModeAttr.stSlowRecAttr.stDataSrc);
            break;
        default:
        {
            MLOGE(RED"the workmode parameter error"NONE);
            return HI_FAILURE;
        }
    }

    /**< Start venc */
    for (j = 0; j < HI_PDT_REC_VSTREAM_MAX_CNT; j++)
    {

        for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; i++)
        {
            if ((pstRecDataSrc->aVencHdl[j] == pstMediaCfg->stVideoCfg.astVencCfg[i].VencHdl) &&
                (HI_TRUE == pstMediaCfg->stVideoCfg.astVencCfg[i].bEnable))
            {
                s32Ret = HI_MAPI_VPROC_StartPort(pstMediaCfg->stVideoCfg.astVencCfg[i].ModHdl,
                    pstMediaCfg->stVideoCfg.astVencCfg[i].ChnHdl);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                s32Ret = HI_MAPI_VENC_Start(pstRecDataSrc->aVencHdl[j], -1);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                break;
            }
        }
    }

    /**< Start aenc  */
    for (i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; i++)
    {
        if ((pstRecDataSrc->AencHdl == pstMediaCfg->stAudioCfg.astAencCfg[i].AencHdl) &&
            (HI_TRUE == pstMediaCfg->stAudioCfg.astAencCfg[i].bEnable))
        {
            s32Ret = HI_PDT_MEDIA_AudioInit(&pstMediaCfg->stAudioCfg);
            HI_APPCOMM_CHECK_EXPR((HI_PDT_MEDIA_EINITIALIZED == s32Ret || HI_SUCCESS == s32Ret), HI_FAILURE);
            s32Ret = HI_MAPI_AENC_BindACap(pstMediaCfg->stAudioCfg.astAencCfg[i].AcapHdl,
                pstRecDataSrc->AencHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            s32Ret = HI_MAPI_AENC_Start(pstRecDataSrc->AencHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;
        }
    }

    return s32Ret;
}
#endif

#ifdef CFG_BOOT_LOGO_ON

/**
 * @brief     videoout start without bind, for bootlogo
 * @param[in] pstVideoOutCfg : video out configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/31
 */
static HI_S32 PDT_INIT_VideoOutStart(const HI_PDT_MEDIA_VIDEOOUT_CFG_S* pstVideoOutCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;

    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
    {
        if (!pstVideoOutCfg->astDispCfg[i].bEnable)
        {
            continue;
        }
        s32Ret = HI_MAPI_DISP_Start(pstVideoOutCfg->astDispCfg[i].VdispHdl,
            (HI_MAPI_DISP_VIDEOLAYER_ATTR_S *)&pstVideoOutCfg->astDispCfg[i].stVideoLayerAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        if (pstVideoOutCfg->astDispCfg[i].stCscAttr.bEnable)
        {
            MLOGI(YELLOW"set csc in video out start"NONE"\n");
            s32Ret = HI_MAPI_DISP_SetAttrEx(pstVideoOutCfg->astDispCfg[i].VdispHdl,
                HI_MAPI_DISP_CMD_VIDEO_CSC,
                (HI_VOID*)&pstVideoOutCfg->astDispCfg[i].stCscAttr.stAttrEx,
                sizeof(HI_MAPI_DISP_CSCATTREX_S));
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }

        for (j = 0; j < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++j)
        {
            if (!pstVideoOutCfg->astDispCfg[i].astWndCfg[j].bEnable)
            {
                continue;
            }
            HI_MAPI_DISP_WINDOW_ROTATE_S stDispRotate;
            stDispRotate.enDispRorate = pstVideoOutCfg->astDispCfg[i].astWndCfg[j].enRotate;
            s32Ret = HI_MAPI_DISP_SetWindowAttrEx(pstVideoOutCfg->astDispCfg[i].VdispHdl,
                pstVideoOutCfg->astDispCfg[i].astWndCfg[j].WndHdl, HI_MAPI_DISP_WINDOW_CMD_ROTATE,
                (HI_VOID*)&stDispRotate, sizeof(HI_MAPI_DISP_WINDOW_ROTATE_S));
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s32Ret = HI_MAPI_DISP_SetWindowAttr(pstVideoOutCfg->astDispCfg[i].VdispHdl,
                pstVideoOutCfg->astDispCfg[i].astWndCfg[i].WndHdl,
                (HI_MAPI_DISP_WINDOW_ATTR_S*)&pstVideoOutCfg->astDispCfg[i].astWndCfg[i].stWndAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            if (ASPECT_RATIO_BUTT != pstVideoOutCfg->astDispCfg[i].astWndCfg[j].stAspectRatio.enMode)
            {
                HI_MAPI_DISP_WINDOW_ASPECTRATIO_S stAspectRatio = {};
                stAspectRatio.stAspectRatio.enMode = pstVideoOutCfg->astDispCfg[i].astWndCfg[j].stAspectRatio.enMode;
                stAspectRatio.stAspectRatio.u32BgColor = pstVideoOutCfg->astDispCfg[i].astWndCfg[j].stAspectRatio.u32BgColor;
                stAspectRatio.stAspectRatio.stVideoRect = pstVideoOutCfg->astDispCfg[i].astWndCfg[j].stAspectRatio.stVideoRect;
                MLOGI("[%d,%d] AspectMode[%d] BgColor[%#x]\n", pstVideoOutCfg->astDispCfg[i].VdispHdl,
                    pstVideoOutCfg->astDispCfg[i].astWndCfg[j].WndHdl,
                    stAspectRatio.stAspectRatio.enMode, stAspectRatio.stAspectRatio.u32BgColor);

                s32Ret = HI_MAPI_DISP_SetWindowAttrEx(pstVideoOutCfg->astDispCfg[i].VdispHdl,
                        pstVideoOutCfg->astDispCfg[i].astWndCfg[j].WndHdl, HI_MAPI_DISP_WINDOW_CMD_ASPECTRATIO,
                        (HI_VOID*)&stAspectRatio, sizeof(HI_MAPI_DISP_WINDOW_ASPECTRATIO_S));
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
            s32Ret = HI_MAPI_DISP_StartWindow(pstVideoOutCfg->astDispCfg[i].VdispHdl,
                                              pstVideoOutCfg->astDispCfg[i].astWndCfg[i].WndHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
    return HI_SUCCESS;
}

/** BootLogo Entry */
static HI_S32 PDT_INIT_BootLogo(const HI_PDT_MEDIA_VIDEOOUT_CFG_S* pstVideoOutCfg)
{
#define BOOT_LOGO_SIZE_WIDTH  (320)
#define BOOT_LOGO_SIZE_HEIGHT (240)

    SIZE_S stRes;
    stRes.u32Width  = BOOT_LOGO_SIZE_WIDTH;
    stRes.u32Height = BOOT_LOGO_SIZE_HEIGHT;
    MLOGD("BootLogo Size[%ux%u]\n", stRes.u32Width, stRes.u32Height);
    return HI_SYSTEM_BootLogo(pstVideoOutCfg->astDispCfg[0].VdispHdl,
              pstVideoOutCfg->astDispCfg[0].astWndCfg[0].WndHdl, stRes, bootlogo_yuv);
}

#endif

#ifdef CFG_BOOT_SOUND_ON

/** BootSound Entry  */
HI_S32 PDT_INIT_BootSound(const HI_PDT_MEDIA_CFG_S* pstMediaModeCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_U8* pu8PCMData = bootsound_pcm;
    HI_U32 u32DataLen = sizeof(bootsound_pcm);
    MLOGD("BootSound Size[%u]\n", u32DataLen);
    s32Ret = HI_SYSTEM_BootSound(pstMediaModeCfg->stAudioOutCfg.astAoCfg[0].AoHdl, pu8PCMData, u32DataLen);

    return s32Ret;
}

#endif

static HI_S32 PDT_INIT_GetPoweronWorkMode(HI_PDT_WORKMODE_E* penPoweronWorkMode)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_WORKMODE_E  enPoweronWorkMode = HI_PDT_WORKMODE_BUTT;
    HI_PDT_POWERON_ACTION_E enPoweronAction = HI_PDT_POWERON_ACTION_BUTT;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_ACTION, &enPoweronAction);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    if (HI_PDT_POWERON_ACTION_NORM_REC == enPoweronAction)
    {
        enPoweronWorkMode = HI_PDT_WORKMODE_NORM_REC;
    }
    else if (HI_PDT_POWERON_ACTION_LOOP_REC == enPoweronAction)
    {
        enPoweronWorkMode = HI_PDT_WORKMODE_LOOP_REC;
    }
    else if (HI_PDT_POWERON_ACTION_LPSE_REC == enPoweronAction)
    {
        enPoweronWorkMode = HI_PDT_WORKMODE_LPSE_REC;
    }
    else if (HI_PDT_POWERON_ACTION_SLOW_REC == enPoweronAction)
    {
        enPoweronWorkMode = HI_PDT_WORKMODE_SLOW_REC;
    }
    else if (HI_PDT_POWERON_ACTION_RECSNAP == enPoweronAction)
    {
        enPoweronWorkMode = HI_PDT_WORKMODE_RECSNAP;
    }
    else if (HI_PDT_POWERON_ACTION_LPSE_PHOTO == enPoweronAction)
    {
        enPoweronWorkMode = HI_PDT_WORKMODE_LPSE_PHOTO;
    }
    else if (HI_PDT_POWERON_ACTION_SING_PHOTO == enPoweronAction)
    {
        enPoweronWorkMode = HI_PDT_WORKMODE_SING_PHOTO;
    }
    else
    {
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enPoweronWorkMode);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    *penPoweronWorkMode = enPoweronWorkMode;
    MLOGI("PoweronWorkMode[%d]\n", enPoweronWorkMode);
    return HI_SUCCESS;
}

static HI_U32 PDT_INIT_GCD(HI_U32 x,HI_U32 y)
{
    HI_U32 u32Max = (x>y)?x:y;
    HI_U32 u32Min = (x>y)?y:x;
    HI_U32 z = u32Min;
    while(u32Max%u32Min!=0)
    {
        z = u32Max%u32Min;
        u32Max = u32Min;
        u32Min = z;
    }
    return z;
}

static HI_S32 PDT_INIT_UpdateLapseRecVencAttr(HI_U32 u32Interval_ms,
    HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_S32 s32VencIndex)
{
    HI_U32 u32GCD = PDT_INIT_GCD(1000,u32Interval_ms);
    MLOGD(YELLOW"VencIndex(%d), IntervalMs(%d)\n"NONE, s32VencIndex, u32Interval_ms);

    HI_PDT_MEDIA_VENC_CFG_S* pstStreamVencCfg = &pstMediaCfg->stVideoCfg.astVencCfg[s32VencIndex];
    if(pstStreamVencCfg->bEnable)
    {
        HI_MAPI_VENC_ATTR_CBR_S* pstCBRAttr = NULL;
        HI_MAPI_PAYLOAD_TYPE_E  enStreamType = pstStreamVencCfg->stVencAttr.stTypeAttr.enType;
        switch(enStreamType)
        {
            case HI_MAPI_PAYLOAD_TYPE_H264:
                pstCBRAttr = &(pstStreamVencCfg->stVencAttr.stRcAttr.unAttr.stH264Cbr.stAttr);
                break;
            case HI_MAPI_PAYLOAD_TYPE_H265:
                pstCBRAttr = &(pstStreamVencCfg->stVencAttr.stRcAttr.unAttr.stH265Cbr.stAttr);
                break;
            default:
               MLOGD(YELLOW"StreamType(%d) err\n"NONE, enStreamType);
        }

        if(pstCBRAttr)
        {
            HI_FR32* pu32srcFrameRate = &pstCBRAttr->u32SrcFrameRate;
            HI_FR32* pu32dstFrameRate = &pstCBRAttr->fr32DstFrameRate;
            HI_U32* pu32BitRate = &pstCBRAttr->u32BitRate;
            MLOGI(YELLOW"BitRate(%d), srcFrameRate(%d), dstFrameRate(%d)\n"NONE,
                *pu32BitRate, *pu32srcFrameRate, *pu32dstFrameRate);

            *pu32BitRate = (*pu32BitRate)/(u32Interval_ms/1000.0 * PDT_INIT_RECMNG_LAPSE_PLAY_FPS);
            *pu32dstFrameRate = (1000/u32GCD)+((u32Interval_ms/u32GCD) << 16);

            if(*pu32BitRate < 2)
            {
                MLOGD(YELLOW"BitRate(%d) small than 2, set BitRate(2)\n"NONE, *pu32BitRate);
                *pu32BitRate = 2;
            }

            MLOGI(YELLOW"updated, BitRate(%d), srcFrameRate(%d), dstFrameRate(%d)\n"NONE,
                *pu32BitRate, *pu32srcFrameRate, *pu32dstFrameRate);
        }
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIT_MediaInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_CFG_S  stMediaCfg;
    HI_PDT_SCENE_MODE_S stSceneModeCfg;
    HI_PDT_WORKMODE_E  enPoweronWorkMode = HI_PDT_WORKMODE_BUTT;
    HI_PDT_MEDIAMODE_E enPoweronMediaMode = HI_PDT_MEDIAMODE_BUTT;
    memset(&stMediaCfg, 0, sizeof(HI_PDT_MEDIA_CFG_S));
    memset(&stSceneModeCfg, 0, sizeof(HI_PDT_SCENE_MODE_S));

    /* get poweron mediamode and mediacfg */
    HI_TIME_STAMP;
    s32Ret = PDT_INIT_GetPoweronWorkMode(&enPoweronWorkMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enPoweronWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE,
                                           (HI_VOID*)&enPoweronMediaMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGI("Poweron MediaMode[%d]\n", enPoweronMediaMode);

    s32Ret = HI_PDT_PARAM_GetMediaCfg(enPoweronWorkMode, enPoweronMediaMode, &stMediaCfg, &stSceneModeCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "GetPoweronMediaCfg");

    /* init media module and osd resource */
    HI_TIME_STAMP;
    s32Ret = HI_PDT_MEDIA_Init(&stMediaCfg.stViVpssMode, &stMediaCfg.stVBCfg, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MEDIA_Init");

#ifdef CONFIG_MOTIONSENSOR
    s32Ret = HI_PDT_MEDIA_InitMotionsensor();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "init motionsensor");
#endif

    HI_PDT_MEDIA_OSD_VIDEO_ATTR_S stOsdVideoAttr;
    HI_PDT_PARAM_GetOSDVideoAttr(&stOsdVideoAttr);
    s32Ret = HI_PDT_MEDIA_InitOSD(&stOsdVideoAttr, &stMediaCfg.stVideoCfg.stOsdCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_InitOSD");

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_VOLUME, &stMediaCfg.stAudioOutCfg.astAoCfg[0].s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGD("ao handle [%d],sound on\n",stMediaCfg.stAudioOutCfg.astAoCfg[0].AoHdl);
    s32Ret = HI_PDT_MEDIA_AudioOutStart(&stMediaCfg.stAudioOutCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_AudioOutStart");

    /* boot sound */
#ifdef CFG_BOOT_SOUND_ON
    HI_TIME_STAMP;
    HI_BOOL bBootSoundEnable = HI_TRUE;
    HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_BOOTSOUND,&bBootSoundEnable);
    if(HI_TRUE == bBootSoundEnable)
    {
        s32Ret = PDT_INIT_BootSound(&stMediaCfg);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_INIT_BootSound");
    }
#endif
    /* boot logo */
#ifdef CFG_BOOT_LOGO_ON
    ASPECT_RATIO_E enMode = stMediaCfg.stVideoOutCfg.astDispCfg[0].astWndCfg[0].stAspectRatio.enMode;
    stMediaCfg.stVideoOutCfg.astDispCfg[0].astWndCfg[0].stAspectRatio.enMode = ASPECT_RATIO_NONE;
#endif
    HI_TIME_STAMP;
    s32Ret = HI_PDT_Media_UpdateDispCfg(&stMediaCfg, &stMediaCfg.stVideoOutCfg.astDispCfg[0]);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_Media_UpdateDispCfg");

    s32Ret = HI_PDT_MEDIA_VideoOutInit(&stMediaCfg.stVideoOutCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_VideoOutInit");
#ifdef CFG_BOOT_LOGO_ON
#ifdef CONFIG_SCREEN
    HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_HAL_SCREEN_SetDisplayState");
#endif

    s32Ret = PDT_INIT_VideoOutStart(&stMediaCfg.stVideoOutCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_INIT_VideoOutStart");

    s32Ret = PDT_INIT_BootLogo(&stMediaCfg.stVideoOutCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_INIT_BootLogo");
#endif

    /* AE converence */
    HI_TIME_STAMP;
    HI_PDT_MEDIA_VIDEO_CFG_S stAEVideoCfg;
    memcpy(&stAEVideoCfg, &stMediaCfg.stVideoCfg, sizeof(stAEVideoCfg));

    s32Ret = PDT_INIT_UpdateAEConvVideoCfg(&stAEVideoCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "UpdateAECfg");

    s32Ret = PDT_INIT_AEConverence(&stAEVideoCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_INIT_AEConverence");
    HI_TIME_STAMP;

#ifdef CFG_BOOT_LOGO_ON
    s32Ret =HI_PDT_MEDIA_VideoOutStop();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_VideoOutStop");
    stMediaCfg.stVideoOutCfg.astDispCfg[0].astWndCfg[0].stAspectRatio.enMode = enMode;
#endif

    s32Ret = HI_PDT_Media_UpdateDispCfg(&stMediaCfg, &stMediaCfg.stVideoOutCfg.astDispCfg[0]);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_Media_UpdateDispCfg");

    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enPoweronWorkMode, &stEnterWorkModeCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "get WorkModeCfg");

    /** update venc attr when poweron workmode is Lapse record */
    if((HI_PDT_WORKMODE_LPSE_REC == enPoweronWorkMode) &&
       (stEnterWorkModeCfg.unModeAttr.stLapseRecAttr.u32Interval_ms < 64*1000))
    {
        s32Ret = PDT_INIT_UpdateLapseRecVencAttr(stEnterWorkModeCfg.unModeAttr.stLapseRecAttr.u32Interval_ms,
            &(stMediaCfg), PDT_INIT_MAIN_STREAM_VENC_HDL);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "update main stream venc attr");

        s32Ret = PDT_INIT_UpdateLapseRecVencAttr(stEnterWorkModeCfg.unModeAttr.stLapseRecAttr.u32Interval_ms,
            &(stMediaCfg), PDT_INIT_SUB_STREAM_VENC_HDL);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "update sub stream venc attr");
    }

    /* TODO: set ae attribute before media reset in case poweron photo(speed) */
    /* reset media */
    s32Ret = HI_PDT_MEDIA_Reset(&stMediaCfg, HI_FALSE, HI_FALSE, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_Reset");

#ifdef CONFIG_RELEASE
    s32Ret = HI_PDT_SCENE_SetSceneMode(&stSceneModeCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_SCENE_SetSceneMode");

    s32Ret = HI_PDT_SCENE_Pause(HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_SCENE_Pause");
#endif

#ifdef CONFIG_SCREEN
    HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_HAL_SCREEN_SetDisplayState");
#endif

    /** start preview */
    s32Ret = HI_PDT_MEDIA_VideoOutStart();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_VideoOutStart");
    HI_TIME_STAMP;

#ifdef CFG_TIME_MEASURE_ON
    HI_PrintBootTime("Preview");
#endif

#if !defined(CFG_LITEOS_NO_PRE_RECORD)
    /** start preRec */
    HI_PDT_POWERON_ACTION_E enPoweronAction = HI_PDT_POWERON_ACTION_BUTT;
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_ACTION, &enPoweronAction);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    if ((HI_PDT_POWERON_ACTION_NORM_REC == enPoweronAction) ||
        (HI_PDT_POWERON_ACTION_LOOP_REC == enPoweronAction) ||
        (HI_PDT_POWERON_ACTION_LPSE_REC == enPoweronAction) ||
        (HI_PDT_POWERON_ACTION_SLOW_REC == enPoweronAction))
    {
        s32Ret = PDT_INIT_PowerOnPreRec(&stMediaCfg, enPoweronWorkMode);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_INIT_PowerOnPreRec");

#ifdef CFG_TIME_MEASURE_ON
        HI_PrintBootTime("PreRec");
#endif
    }
#endif

    return s32Ret;
}

#endif

HI_S32 HI_PDT_INIT_SERVICE_PreInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;


    /* param init */
#ifdef CFG_RAW_PARAM_ON
    s32Ret = HI_PDT_PARAM_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGD("HI_PDT_PARAM_Init success\n");
#endif

    return s32Ret;
}

HI_S32 HI_PDT_INIT_SERVICE_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TIME_STAMP;

    /* mapi log level */
    HI_MAPI_LOG_SetEnabledLevel(HI_MAPI_LOG_LEVEL_WARN);

    s32Ret = HI_MAPI_Sys_Init();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_Sys_Init");
    HI_TIME_STAMP;

#if defined(AMP_LINUX_HUAWEILITE)
    /* msg */
    MLOGD("msg server init ...\n");
    HI_MSG_SERVER_Init();
    HI_TIME_STAMP;
    MLOGD("msg system server init ...\n");
    HI_SYSTEM_ServiceInit();
    HI_TIME_STAMP;
#ifdef CONFIG_SCREEN
    MLOGD("msg hal screen init ...\n");
    extern HI_S32 MSG_HAL_SCREEN_ServiceInit(HI_VOID);
    MSG_HAL_SCREEN_ServiceInit();
    HI_TIME_STAMP;
#endif
    MLOGD("msg scene msg server init ...\n");
    HI_PDT_SCENE_MSG_SERVER_Init();
    HI_TIME_STAMP;
    extern HI_S32 PDT_MEDIA_MSG_RegisterCallback(HI_VOID);
    MLOGI("msg media init ...\n");
    PDT_MEDIA_MSG_RegisterCallback();
#endif

#ifdef CONFIG_RELEASE
    s32Ret = HI_PDT_SCENE_Init();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"init scene");
#endif

#ifdef CFG_RAW_PARAM_ON
    s32Ret = PDT_INIT_MediaInit();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_INIT_PDT_MediaInit");
#endif

    return s32Ret;
}

HI_S32 HI_PDT_INIT_SERVICE_PostInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TIME_STAMP;

    /* cat logmpp */
    extern void CatLogShell();
    CatLogShell();
    HI_TIME_STAMP;

    /* playback svr init */
    s32Ret = HI_AVPLAY_IPCMSG_SVR_Init(NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    HI_TIME_STAMP;
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

