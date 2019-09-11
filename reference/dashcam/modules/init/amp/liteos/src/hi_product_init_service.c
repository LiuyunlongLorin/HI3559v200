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
#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_aenc.h"
#include "hi_avplay_server_ipcmsg.h"
#include "hi_hal_screen.h"
#include "hi_appcomm_msg_server.h"
#include "hi_timestamp.h"
#include "hi_system.h"
#ifdef HDMI_TEST
#include "hi_mapi_hdmi_define.h"
#include "hi_mapi_hdmi.h"
#endif
#include "hi_mapi_hal_ahd.h"
#include "hi_ahdmng.h"
#include "hi_tempcycle.h"

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
/**
bCheckCamEnable,HI_FASE mean all cameras are enable when getting cam context
*/
static HI_S32 PDT_INIT_GetWorkmodeInfo(HI_BOOL bCheckCamEnable,HI_PDT_WORKMODE_E* penWorkmode,HI_PDT_PARAM_CAM_CONTEXT_S* pstCamContext)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_PARAM_CAM_MEDIAMODE_CFG_S stCamCfg;
    HI_S32 i = 0;
    HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_STARTUP;

    HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
    if(HI_SYSTEM_STARTUP_SRC_WAKEYP == enStartupSrc)
    {
        *penWorkmode = HI_PDT_WORKMODE_PARKING_REC;
    }
    else if(HI_SYSTEM_STARTUP_SRC_STARTUP == enStartupSrc)
    {
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, penWorkmode);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"HI_PDT_PARAM_GetCommParam");
    }

    s32Ret = HI_PDT_PARAM_GetMediaModeCfg(*penWorkmode,&stCamCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"HI_PDT_PARAM_GetMediaModeCfg");
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        pstCamContext[i].stMediaModeCfg.s32CamID = stCamCfg.astMediaModeCfg[i].s32CamID;
        pstCamContext[i].stMediaModeCfg.enMediaMode = stCamCfg.astMediaModeCfg[i].enMediaMode;
        if(HI_TRUE == bCheckCamEnable)
        {
            s32Ret = HI_PDT_PARAM_GetCamParam(*penWorkmode, pstCamContext[i].stMediaModeCfg.s32CamID,
                HI_PDT_PARAM_TYPE_CAM_STATUS, &pstCamContext[i].bEnable);
            HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret,"get CamEnable");
        }
        else
        {
            pstCamContext[i].bEnable = HI_TRUE;
        }
    }
    return s32Ret;
}
/**
bCheckCamEnable,HI_FASE mean all cameras are enable when getting MediaCfg
bAutoUpdated,HI_TRUE mean update MediaCfg for display or for lapse record
*/
static HI_S32 PDT_INIT_GetMediaCfg(HI_BOOL bCheckCamEnable,HI_BOOL bAutoUpdated,HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_PARAM_CAM_CONTEXT_S astCamCtx[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_WORKMODE_E enWorkmode = HI_PDT_WORKMODE_NORM_REC;

    s32Ret = PDT_INIT_GetWorkmodeInfo(bCheckCamEnable,&enWorkmode,astCamCtx);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"PDT INIT GetWorkmodeInfo");
    HI_TIME_STAMP;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enWorkmode, astCamCtx, pstMediaCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"HI_PDT_PARAM_GetMediaCfg");
    HI_TIME_STAMP;
    if(HI_TRUE == bAutoUpdated)
    {
        HI_S32 s32PreViewCamID = 0;
        HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PREVIEW_CAMID, &s32PreViewCamID);
        for(HI_S32 i = 0;i < HI_PDT_MEDIA_DISP_MAX_CNT;i++)
        {
            s32Ret = HI_PDT_Media_UpdateDispCfg(&pstMediaCfg->stVideoOutCfg.astDispCfg[i],&pstMediaCfg->stVideoCfg.stVprocCfg,
                                                &pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[s32PreViewCamID].stResolution);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"HI_PDT_Media_UpdateDispCfg");
        }
        /* record update media config*/
        HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_STARTUP;
        HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
        if(HI_PDT_WORKMODE_NORM_REC == enWorkmode)
        {
            HI_PDT_PARAM_RecordUpdateMediaCfg(pstMediaCfg);
        }
    }
    return HI_SUCCESS;
}


#ifdef CONFIG_AHD_ON
static HI_S32 PDT_INIT_AhdCamStatusProc(HI_U32 u32CamID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bAhdConnected = HI_TRUE,bCamEnable = HI_TRUE;
    HI_HAL_AHD_STATUS_E aenState[HI_HAL_AHD_CHN_MAX] =
        {HI_HAL_AHD_STATUS_BUIT,HI_HAL_AHD_STATUS_BUIT,HI_HAL_AHD_STATUS_BUIT,HI_HAL_AHD_STATUS_BUIT};
    s32Ret = HI_MAPI_VCAP_AhdGetStatus(u32CamID,aenState,HI_HAL_AHD_CHN_MAX);
    if(HI_SUCCESS == s32Ret)
    {
        for(HI_S32 i=0; i < HI_HAL_AHD_CHN_MAX; i++)
        {
            if(aenState[i] == HI_HAL_AHD_STATUS_DISCONNECT)
            {
                bAhdConnected = HI_FALSE;
            }
        }
    }
    else
    {
        bAhdConnected = HI_FALSE;
    }
    MLOGI("bAhdConnected[%d]\n",bAhdConnected);
    s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, u32CamID, HI_PDT_PARAM_TYPE_CAM_STATUS, &bCamEnable);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret,"set ahd bCamEnable");
    if(bCamEnable != bAhdConnected)
    {
        MLOGI("bAhdConnected[%d],bCamEnable[%d],state change\n",bAhdConnected,bCamEnable);
        s32Ret = HI_PDT_PARAM_SetCamParam(HI_PDT_WORKMODE_NORM_REC, u32CamID, HI_PDT_PARAM_TYPE_CAM_STATUS, &bAhdConnected);
        HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret,"set ahd bCamEnable");
    }
    return s32Ret;
}

static HI_S32 PDT_INIT_AhdStatusProc(HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 u32CamID = 0;
    s32Ret = PDT_INIT_GetMediaCfg(HI_FALSE,HI_TRUE,pstMediaCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT GetMediaCfg");
#ifdef CONFIG_SNS1_BT656
    u32CamID = 1;
#else
    MLOGE("must need point AHD cam\n");
    return HI_FAILURE;
#endif
    s32Ret = HI_PDT_MEDIA_InitSensor(&pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[u32CamID]);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI PDT MEDIA InitSensor");

    s32Ret = PDT_INIT_AhdCamStatusProc(1);
    return s32Ret;
}
#else
static HI_S32 PDT_INIT_AhdStatusProc(HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    return HI_SUCCESS;
}
#endif
#ifndef HDMI_TEST
#ifdef CFG_BOOT_LOGO_ON
/** BootLogo Entry */
static HI_S32 PDT_INIT_BootLogo(const HI_PDT_MEDIA_VIDEOOUT_CFG_S* pstVideoOutCfg)
{
#define BOOT_LOGO_SIZE_WIDTH  (320)
#define BOOT_LOGO_SIZE_HEIGHT (240)

    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_VIDEOOUT_CFG_S stVideoOutCfg;
    memcpy(&stVideoOutCfg, pstVideoOutCfg, sizeof(HI_PDT_MEDIA_VIDEOOUT_CFG_S));

    HI_S32 i = 0, j = 0;
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; i++)
    {
        if (!stVideoOutCfg.astDispCfg[i].bEnable)
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_DISP_WND_MAX_CNT; j++)
        {
            stVideoOutCfg.astDispCfg[i].astWndCfg[j].bEnable = HI_FALSE;
            stVideoOutCfg.astDispCfg[i].astWndCfg[j].enBindedMod = HI_PDT_MEDIA_VIDEOMOD_BUTT;
            stVideoOutCfg.astDispCfg[i].astWndCfg[j].stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
#ifdef CFG_LCD_UISRC_DEST_WIDTH
            stVideoOutCfg.astDispCfg[i].astWndCfg[j].stAspectRatio.enMode = ASPECT_RATIO_NONE;
#endif
        }
        /* only enable first vo_chn  */
        stVideoOutCfg.astDispCfg[i].astWndCfg[0].bEnable = HI_TRUE;

#if (defined(HI3559V200) && defined(CFG_LCD_UISRC_DEST_WIDTH))
        stVideoOutCfg.astDispCfg[i].u32BufferLen = 2;
#endif
    }
    s32Ret = HI_PDT_MEDIA_VideoOutStart(&stVideoOutCfg,NULL);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_VideoOutStart");

    SIZE_S stRes;
    stRes.u32Width  = BOOT_LOGO_SIZE_WIDTH;
    stRes.u32Height = BOOT_LOGO_SIZE_HEIGHT;
#ifdef CFG_LCD_UISRC_DEST_WIDTH
    stRes.u32Width  = CFG_LCD_UISRC_DEST_WIDTH;
#endif
#ifdef CFG_LCD_UISRC_DEST_HEIGHT
    stRes.u32Height = CFG_LCD_UISRC_DEST_HEIGHT;
#endif


    MLOGD("BootLogo Size[%ux%u]\n", stRes.u32Width, stRes.u32Height);
    return HI_SYSTEM_BootLogo(stVideoOutCfg.astDispCfg[0].VdispHdl,
        stVideoOutCfg.astDispCfg[0].astWndCfg[0].WndHdl, stRes, bootlogo_yuv);
}

#endif
#endif

#ifdef CFG_BOOT_SOUND_ON

/** BootSound Entry  */
HI_S32 PDT_INIT_BootSound(const HI_PDT_MEDIA_CFG_S* pstMediaModeCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    MLOGD("ao handle [%d],sound on\n",pstMediaModeCfg->stAudioOutCfg.astAoCfg[0].AoHdl);
    s32Ret = HI_PDT_MEDIA_AudioOutStart(&pstMediaModeCfg->stAudioOutCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_AudioOutStart");

    HI_U8* pu8PCMData = bootsound_pcm;
    HI_U32 u32DataLen = sizeof(bootsound_pcm);
    MLOGD("BootSound Size[%u]\n", u32DataLen);
    s32Ret = HI_SYSTEM_BootSound(pstMediaModeCfg->stAudioOutCfg.astAoCfg[0].AoHdl, pu8PCMData, u32DataLen);

    return s32Ret;
}

#endif

#ifdef HDMI_TEST
HI_VOID HDMI_EventPro(HI_HDMI_EVENT_TYPE_E event, HI_VOID* pPrivateData)
{
    switch (event)
    {
        case HI_HDMI_EVENT_HOTPLUG:
            printf("\033[0;32mhdmi HOTPLUG event! \033[0;39m\n");
            break;

        case HI_HDMI_EVENT_NO_PLUG:
            printf("\033[0;31mhdmi NO_PLUG event! \033[0;39m\n");
            break;

        default :
            printf("\033[0;31minvalid hdmi event! \033[0;39m\n");
            break;
    }
}
#endif
static void PDT_INIT_AutoEncBitRate(HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg)
{
#define PDT_VENC_BUF_MIN_CACHE_TIME_MS 2000
    HI_MEDIA_VENC_ATTR_S* pstVencAttr = &pstVencCfg->stVencAttr;
    HI_U32  u32BufSize = pstVencAttr->stTypeAttr.u32BufSize;
    HI_U32  u32BitRate = 0;
    HI_U32  u32CacheTime_ms = 2000;
    HI_BOOL bAutoalg = HI_FALSE;
    if (HI_MAPI_VENC_RC_MODE_CBR == pstVencAttr->stRcAttr.enRcMode)
    {
        if (HI_MAPI_PAYLOAD_TYPE_H264 == pstVencAttr->stTypeAttr.enType)
        {
            HI_MEDIA_VENC_ATTR_H264_CBR_S* pstAttrCbr = &pstVencAttr->stRcAttr.unAttr.stH264Cbr;
            u32CacheTime_ms = 1000 * u32BufSize/(pstAttrCbr->stAttr.u32BitRate*1024/8);
            if(u32CacheTime_ms < PDT_VENC_BUF_MIN_CACHE_TIME_MS) /**confirm vencbuf Cache time */
            {
                bAutoalg = HI_TRUE;
                u32BitRate =(1000*u32CacheTime_ms/PDT_VENC_BUF_MIN_CACHE_TIME_MS)*pstAttrCbr->stAttr.u32BitRate/1000;
                pstAttrCbr->stAttr.u32BitRate = u32BitRate;
            }
        }
        else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstVencAttr->stTypeAttr.enType)
        {
            HI_MEDIA_VENC_ATTR_H265_CBR_S* pstAttrCbr = &pstVencAttr->stRcAttr.unAttr.stH265Cbr;
            u32CacheTime_ms = 1000 * u32BufSize/(pstAttrCbr->stAttr.u32BitRate*1024/8);
            if(u32CacheTime_ms < PDT_VENC_BUF_MIN_CACHE_TIME_MS) /**confirm vencbuf Cache time */
            {
                bAutoalg = HI_TRUE;
                u32BitRate =(1000*u32CacheTime_ms/PDT_VENC_BUF_MIN_CACHE_TIME_MS)*pstAttrCbr->stAttr.u32BitRate/1000;
                pstAttrCbr->stAttr.u32BitRate = u32BitRate;
            }
        }
    }
    else if (HI_MAPI_VENC_RC_MODE_VBR == pstVencAttr->stRcAttr.enRcMode)
    {
        if (HI_MAPI_PAYLOAD_TYPE_H264 == pstVencAttr->stTypeAttr.enType)
        {
            HI_MEDIA_VENC_ATTR_H264_VBR_S* pstAttrVbr = &pstVencAttr->stRcAttr.unAttr.stH264Vbr;
            u32CacheTime_ms = 1000 * u32BufSize/(pstAttrVbr->stAttr.u32MaxBitRate*1024/8);
            if(u32CacheTime_ms < PDT_VENC_BUF_MIN_CACHE_TIME_MS) /**confirm vencbuf Cache time */
            {
                bAutoalg = HI_TRUE;
                u32BitRate =(1000*u32CacheTime_ms/PDT_VENC_BUF_MIN_CACHE_TIME_MS)*pstAttrVbr->stAttr.u32MaxBitRate/1000;
                pstAttrVbr->stAttr.u32MaxBitRate = u32BitRate;
            }
        }
        else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstVencAttr->stTypeAttr.enType)
        {
            HI_MEDIA_VENC_ATTR_H265_VBR_S* pstAttrVbr = &pstVencAttr->stRcAttr.unAttr.stH265Vbr;
            u32CacheTime_ms = (1000*u32BufSize)/(pstAttrVbr->stAttr.u32MaxBitRate*1024/8);
            if(u32CacheTime_ms < PDT_VENC_BUF_MIN_CACHE_TIME_MS) /**confirm vencbuf Cache time */
            {
                bAutoalg = HI_TRUE;
                u32BitRate =(1000*u32CacheTime_ms/PDT_VENC_BUF_MIN_CACHE_TIME_MS)*pstAttrVbr->stAttr.u32MaxBitRate/1000;
                pstAttrVbr->stAttr.u32MaxBitRate = u32BitRate;
            }
        }
    }
    else if (HI_MAPI_VENC_RC_MODE_QVBR == pstVencAttr->stRcAttr.enRcMode)
    {
        if (HI_MAPI_PAYLOAD_TYPE_H264 == pstVencAttr->stTypeAttr.enType)
        {
            HI_MEDIA_VENC_ATTR_H264_QVBR_S* pstAttrQVbr = &pstVencAttr->stRcAttr.unAttr.stH264QVbr;
            u32CacheTime_ms = (1000*u32BufSize)/(pstAttrQVbr->stAttr.u32TargetBitRate*1024/8);
            if(u32CacheTime_ms < PDT_VENC_BUF_MIN_CACHE_TIME_MS)
            {
                bAutoalg = HI_TRUE;
                u32BitRate =(1000*u32CacheTime_ms/PDT_VENC_BUF_MIN_CACHE_TIME_MS)*pstAttrQVbr->stAttr.u32TargetBitRate/1000;
                pstAttrQVbr->stAttr.u32TargetBitRate = u32BitRate;
            }
        }
        else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstVencAttr->stTypeAttr.enType)
        {
            HI_MEDIA_VENC_ATTR_H265_QVBR_S* pstAttrQVbr = &pstVencAttr->stRcAttr.unAttr.stH265QVbr;
            u32CacheTime_ms = (1000*u32BufSize)/(pstAttrQVbr->stAttr.u32TargetBitRate*1024/8);
            if(u32CacheTime_ms < PDT_VENC_BUF_MIN_CACHE_TIME_MS)
            {
                bAutoalg = HI_TRUE;
                u32BitRate =(1000*u32CacheTime_ms/PDT_VENC_BUF_MIN_CACHE_TIME_MS)*pstAttrQVbr->stAttr.u32TargetBitRate/1000;
                pstAttrQVbr->stAttr.u32TargetBitRate = u32BitRate;
            }
        }
    }
    if(bAutoalg)
    {
        MLOGI("need auto enc[%d] bitrate[%d]\n",pstVencCfg->VencHdl,u32BitRate);
        HI_PDT_MEDIA_VencInit(pstVencCfg);
    }
}


/**
 * @brief     Power On Pre Rec
 * @param[in] pstMediaCfg : Media configure
 * @param[in] enWorkMode  : WorkMode
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/31
 */
static HI_S32 PDT_INIT_PowerOnPreRec(HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_PDT_WORKMODE_E enWorkMode)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0,j = 0,k = 0,s32VencIdx = 0;

    HI_PDT_WORKMODE_CFG_S stWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enWorkMode, &stWorkModeCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_PDT_PARAM_RECMNG_ATTR_S *pstRecAttr = stWorkModeCfg.unModeCfg.stNormRecAttr.astRecMngAttr;

    for(i = 0;i < HI_PDT_RECTASK_MAX_CNT;i++)
    {
        if(pstRecAttr[i].bEnable)
        {
            for(j = 0 ;j < HI_PDT_RECTASK_FILE_MAX_CNT;j++)
            {

                for(k = 0;k < pstRecAttr[i].astFileAttr[j].stDataSource.u32VencCnt && k < HI_RECMNG_FILE_MAX_VIDEO_TRACK_CNT;k++)
                {
                    for(s32VencIdx = 0; s32VencIdx < HI_PDT_MEDIA_VENC_MAX_CNT;s32VencIdx++)
                    {
                        if ((pstRecAttr[i].astFileAttr[j].stDataSource.aVencHdl[k] == pstMediaCfg->stVideoCfg.astVencCfg[s32VencIdx].VencHdl) &&
                        (HI_TRUE == pstMediaCfg->stVideoCfg.astVencCfg[s32VencIdx].bEnable))
                        {
                                s32Ret = HI_MAPI_VPROC_StartPort(pstMediaCfg->stVideoCfg.astVencCfg[s32VencIdx].ModHdl,
                                    pstMediaCfg->stVideoCfg.astVencCfg[s32VencIdx].ChnHdl);
                                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                                PDT_INIT_AutoEncBitRate(&pstMediaCfg->stVideoCfg.astVencCfg[s32VencIdx]);
                                s32Ret = HI_MAPI_VENC_Start(pstMediaCfg->stVideoCfg.astVencCfg[s32VencIdx].VencHdl, -1);
                                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                                MLOGD("Venc[%d] start success\n", pstMediaCfg->stVideoCfg.astVencCfg[s32VencIdx].VencHdl);
                                break;
                        }
                    }
                }
                HI_PrintBootTime("liteos start venc");

                HI_S32 s32AencIdx = 0;
                for(k = 0;k < pstRecAttr[i].astFileAttr[j].stDataSource.u32AencCnt && k < HI_RECMNG_FILE_MAX_AUDIO_TRACK_CNT;k++)
                {
                    for(s32AencIdx = 0; s32AencIdx < HI_PDT_MEDIA_AENC_MAX_CNT;s32AencIdx++)
                    {
                        if ((pstRecAttr[i].astFileAttr[j].stDataSource.aAencHdl[k] == pstMediaCfg->stAudioCfg.astAencCfg[s32AencIdx].AencHdl) &&
                        (HI_TRUE == pstMediaCfg->stAudioCfg.astAencCfg[s32AencIdx].bEnable))
                        {
                            s32Ret = HI_PDT_MEDIA_AudioInit(&pstMediaCfg->stAudioCfg);
                            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                            s32Ret = HI_MAPI_AENC_BindACap(pstMediaCfg->stAudioCfg.astAencCfg[s32AencIdx].AcapHdl,pstMediaCfg->stAudioCfg.astAencCfg[s32AencIdx].AcapChnlHdl,
                                pstMediaCfg->stAudioCfg.astAencCfg[s32AencIdx].AencHdl);
                            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                            s32Ret = HI_MAPI_AENC_Start(pstMediaCfg->stAudioCfg.astAencCfg[s32AencIdx].AencHdl);
                            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                            MLOGD("Aenc[%d] start success\n", pstMediaCfg->stAudioCfg.astAencCfg[s32AencIdx].AencHdl);
                            break;
                        }
                    }
                }
            }
        }
    }

    return s32Ret;
}

static HI_S32 PDT_INIT_MediaInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_CFG_S stMediaCfg;

    /* get ae converence mediamode */
    /* TODO: get from sensor mode */
    HI_TIME_STAMP;
    s32Ret = PDT_INIT_GetMediaCfg(HI_TRUE,HI_FALSE,&stMediaCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT INIT GetMediaCfg");
    s32Ret = HI_PDT_MEDIA_Init(&stMediaCfg.stViVpssMode, &stMediaCfg.stVBCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MEDIA_Init");

    s32Ret = HI_PDT_MEDIA_InitOSD(&stMediaCfg.stVideoCfg.stOsdCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_InitOSD");

    for(HI_S32 i = 0;i < HI_PDT_MEDIA_DISP_MAX_CNT;i++)
    {
        s32Ret = HI_PDT_Media_UpdateDispCfg(&stMediaCfg.stVideoOutCfg.astDispCfg[i],&stMediaCfg.stVideoCfg.stVprocCfg,NULL);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"HI_PDT_Media_UpdateDispCfg");
    }

    s32Ret = HI_PDT_MEDIA_VideoOutInit(&stMediaCfg.stVideoOutCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_VideoOutInit");

#ifdef CONFIG_SCREEN
    HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"HI_HAL_SCREEN_SetDisplayState");
#endif

    HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_STARTUP;
    HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
    if(HI_SYSTEM_STARTUP_SRC_WAKEYP != enStartupSrc)
    {
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
#ifndef HDMI_TEST
        /* boot logo */
#ifdef CFG_BOOT_LOGO_ON
        HI_TIME_STAMP;
        s32Ret = PDT_INIT_BootLogo(&stMediaCfg.stVideoOutCfg);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_INIT_BootLogo");
#endif
#endif
    }

    PDT_INIT_AhdStatusProc(&stMediaCfg);
    s32Ret = PDT_INIT_GetMediaCfg(HI_TRUE,HI_TRUE,&stMediaCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT GetMediaCfg");
    /* reset media */
    s32Ret = HI_PDT_MEDIA_Reset(&stMediaCfg, HI_TRUE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_Reset");

#ifdef CONFIG_RELEASE
    HI_PDT_SCENE_MODE_S stSceneModeCfg;
    HI_PDT_WORKMODE_E enWorkmode = HI_PDT_WORKMODE_NORM_REC;
    HI_PDT_PARAM_CAM_CONTEXT_S astCamCtx[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    s32Ret = PDT_INIT_GetWorkmodeInfo(HI_TRUE,&enWorkmode,astCamCtx);
    memset(&stSceneModeCfg, 0, sizeof(HI_PDT_SCENE_MODE_S));
    s32Ret = HI_PDT_PARAM_GetSceneCfg(enWorkmode, astCamCtx, &stSceneModeCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE, "get scene configure");

    s32Ret = HI_PDT_SCENE_SetSceneMode(&stSceneModeCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_SCENE_SetSceneMode");

    s32Ret = HI_PDT_SCENE_Pause(HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_SCENE_Pause");

#endif

    if(HI_SYSTEM_STARTUP_SRC_WAKEYP == enStartupSrc)
    {
        /*poweron pre record*/
        s32Ret = PDT_INIT_PowerOnPreRec(&stMediaCfg, HI_PDT_WORKMODE_NORM_REC);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"PDT_INIT_PowerOnPreRec");
    }
    /** start preview */
    if (ASPECT_RATIO_NONE!=stMediaCfg.stVideoOutCfg.astDispCfg[0].astWndCfg[0].stAspectRatio.enMode)
    {
        /*stop vo to reset VideoLayerAttr*/
        s32Ret = HI_PDT_MEDIA_VideoOutStop();
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_MEDIA_VideoOutStop");
    }
    s32Ret = HI_PDT_MEDIA_VideoOutStart(NULL,NULL);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"HI_PDT_MEDIA_VideoOutStart");
    HI_PrintBootTime("start vo --preview");

    HI_TIME_STAMP;
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
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE,"HI_INIT_PDT_MediaInit");
#else
    s32Ret = HI_PDT_MEDIA_Init(NULL, NULL);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE,"HI_PDT_MEDIA_Init");
#endif

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
    HI_TEMPCYCLE_MONITORCFG_S stTempMonitorCfg = {0};
    stTempMonitorCfg.highTemperature= 100;
    stTempMonitorCfg.normalTemperature = 85;
    stTempMonitorCfg.tempLastTime_s = 5;   /**5s*/
    stTempMonitorCfg.tempHighAlarmTime_s = 500;
    s32Ret = HI_TEMPCYCLE_Init(&stTempMonitorCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE,"HI_TEMPCYCLE_CreateMonitor");
#endif
    MLOGI("MediaInit success\n");

    return s32Ret;
}

HI_S32 HI_PDT_INIT_SERVICE_PostInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
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

