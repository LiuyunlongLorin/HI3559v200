/**
* @file    product_nectrl_inner.c
* @brief   Function implementation
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
* @version   1.0

*/
#include <sys/stat.h>
#include "hi_dl_adapt.h"
#include "product_netctrl_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/** ffmpeg demuxer symbol */
HI_DEMUX_S s_stNETCTRLFFmpegDemuxerOps =
{
    .aszDemuxerName       = "ffmpeg_demuxer",
    .aszSupportFormat     = "mp4,lrv,mov",
    .u32Priority          = 2,
    .stFmtFun.fmt_open    = NULL,
    .stFmtFun.fmt_close   = NULL,
    .stFmtFun.fmt_probe   = NULL,
    .stFmtFun.fmt_getinfo = NULL,
};

/**
 * @brief  get specified file size
 * @param[in]pazFileName : filename
 * @param[out]ps64Size : file size
 * @return 0 success
 * @return -1 failure
 */
static HI_S32 PDT_FILE_GetSize(const HI_CHAR* pazFileName, HI_S64* ps64Size)
{
    HI_APPCOMM_CHECK_POINTER(pazFileName,-1);
    HI_APPCOMM_CHECK_POINTER(ps64Size,-1);

    struct stat stStatBuf;
    HI_S32 s32Ret = stat(pazFileName, &stStatBuf);
    if (s32Ret)
    {
        MLOGE("get file[%s] stat failed\n", pazFileName);
        return HI_FAILURE;
    }
    *ps64Size = (HI_S64)stStatBuf.st_size;
    return HI_SUCCESS;
}

/**
 * @brief  print fileinfo
 * @param[in]pstFileInfo : file information
 * @return 0 success
 */
static HI_S32 PDT_FILE_PrintFileInfo(const HI_FORMAT_FILE_INFO_S *pstFileInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstFileInfo,-1);

    HI_S32 s32Idx = 0;
    MLOGD("file type[%d][0:h264,1:h265,2:jpeg]\n", pstFileInfo->enVideoType);
    MLOGD("filesize[%lld][%lldMB%lldKB%lldB]\n", pstFileInfo->s64FileSize,
        pstFileInfo->s64FileSize>>20, pstFileInfo->s64FileSize>>10&0x3ff, pstFileInfo->s64FileSize&0x3ff);
    MLOGD("video num[%d]\n", pstFileInfo->s32UsedVideoStreamIndex + 1);
    for (s32Idx = 0; s32Idx <= pstFileInfo->s32UsedVideoStreamIndex; ++s32Idx)
    {
        MLOGD("VideoIdx[%d], Res[%ux%u]\n", pstFileInfo->stSteamResolution[s32Idx].s32VideoStreamIndex,
            pstFileInfo->stSteamResolution[s32Idx].u32Width, pstFileInfo->stSteamResolution[s32Idx].u32Height);
    }
    if (HI_FORMAT_VIDEO_TYPE_JPEG != pstFileInfo->enVideoType)
    {
        MLOGD("duration[%lldms]\n", pstFileInfo->s64Duration);
        MLOGD("bitrate[%uMb%uKb%ubit/s]\n", pstFileInfo->u32Bitrate>>20, pstFileInfo->u32Bitrate>>10&0x3ff, pstFileInfo->u32Bitrate&0x3ff);
        MLOGD("framerate[%f]\n", pstFileInfo->fFrameRate);
    }
    else if (pstFileInfo->s32UsedAudioStreamIndex >= 0)
    {
        MLOGD("audio num[%d]\n", pstFileInfo->s32UsedAudioStreamIndex);
        MLOGD("audio chn cnt[%u],samplerate[%u]\n", pstFileInfo->u32AudioChannelCnt, pstFileInfo->u32SampleRate);
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetDlsym(HI_VOID)
{
#define FFMPEG_DEMUXER  (HI_APP_LIB_PATH"/libffmpeg_demuxer.so")
    HI_VOID* pLibhandle = Hi_dlopen(FFMPEG_DEMUXER,RTLD_LAZY|RTLD_LOCAL);
    if(pLibhandle==NULL)
    {
        MLOGE("Hi_dlopen(%s) fail.\n",FFMPEG_DEMUXER);
        return;
    }

    s_stNETCTRLFFmpegDemuxerOps.stFmtFun.fmt_open = Hi_dlsym(pLibhandle, "HI_FFMPEG_Open");
    if(s_stNETCTRLFFmpegDemuxerOps.stFmtFun.fmt_open == NULL)
    {
        MLOGE("find HI_FFMPEG_Open symbol error!\n");
        goto exit;
    }

    s_stNETCTRLFFmpegDemuxerOps.stFmtFun.fmt_close = Hi_dlsym(pLibhandle, "HI_FFMPEG_Close");
    if(s_stNETCTRLFFmpegDemuxerOps.stFmtFun.fmt_close == NULL)
    {
        MLOGE("find HI_FFMPEG_Close symbol error!\n");
        goto exit;
    }

    s_stNETCTRLFFmpegDemuxerOps.stFmtFun.fmt_probe = Hi_dlsym(pLibhandle, "HI_FFMPEG_Probe");
    if(s_stNETCTRLFFmpegDemuxerOps.stFmtFun.fmt_probe == NULL)
    {
        MLOGE("find HI_FFMPEG_Probe symbol error!\n");
        goto exit;
    }

    s_stNETCTRLFFmpegDemuxerOps.stFmtFun.fmt_getinfo = Hi_dlsym(pLibhandle, "HI_FFMPEG_Getinfo");
    if(s_stNETCTRLFFmpegDemuxerOps.stFmtFun.fmt_getinfo == NULL)
    {
        MLOGE("find HI_FFMPEG_Getinfo symbol error!\n");
        goto exit;
    }
exit:
    Hi_dlclose(pLibhandle);
}

HI_S32 PDT_NETCTRL_GetVideoFileExhaustiveInfo(const HI_CHAR* pazFileName, HI_FORMAT_FILE_INFO_S *pstFileInfo)
{
    /* Check Input Param */
    HI_APPCOMM_CHECK_POINTER(pazFileName, -1);
    HI_APPCOMM_CHECK_POINTER(pstFileInfo,-1);

    /* Get Demuxer OpsSet */
    HI_DEMUX_S *pDemuxerOps = NULL;
    HI_CHAR *pSuffix = NULL;
    pSuffix = strrchr(pazFileName, '.');
    if (!pSuffix)
    {
        MLOGW("<<<<<<<<<<<<Invalid File:"RED"%s"NONE"\n", pazFileName);
        return HI_FAILURE;
    }
    MLOGD("File:%s, Suffix:%s\n", pazFileName, pSuffix);

    if (!strncmp(pSuffix, ".MP4", sizeof(".MP4")) || !strncmp(pSuffix, ".LRV", sizeof(".LRV")))
    {
        pDemuxerOps = &s_stNETCTRLFFmpegDemuxerOps;
    }
    else
    {
        MLOGW("<<<<<<<<<<<<Invalid File Suffix:"RED"%s"NONE"\n", pSuffix);
        return HI_FAILURE;
    }

    /* Get File Info */
    HI_HANDLE* pFmtHandle = NULL;
    HI_S32 s32Ret;

    s32Ret = pDemuxerOps->stFmtFun.fmt_open((HI_VOID **)&pFmtHandle, pazFileName);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = pDemuxerOps->stFmtFun.fmt_probe((HI_VOID *)pFmtHandle);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGD("fmt_probe failed\n");
        goto exit;
    }
    s32Ret = pDemuxerOps->stFmtFun.fmt_getinfo((HI_VOID *)pFmtHandle, pstFileInfo);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGD("fmt_getinfo failed\n");
        goto exit;
    }

exit:
    s32Ret = pDemuxerOps->stFmtFun.fmt_close((HI_VOID *)pFmtHandle);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    PDT_FILE_GetSize(pazFileName, &pstFileInfo->s64FileSize);
    PDT_FILE_PrintFileInfo(pstFileInfo);

    return HI_SUCCESS;
}



/** get eventID desc string  by eventID value */
HI_S32 PDT_NETCTRL_GetEventIDDescByValue(HI_U32 u32EventID, HI_CHAR* pszDesc, HI_U32 u32BufLen)
{
    HI_APPCOMM_CHECK_EXPR(pszDesc, HI_PDT_NETCTRL_EINVAL);

    PDT_NETCTRL_ITEM_VALUE_S astEventDescTable[] = {
        {"STATEMNG_SWITCH_WORKMODE", HI_EVENT_STATEMNG_SWITCH_WORKMODE},
        {"STATEMNG_START",           HI_EVENT_STATEMNG_START},
        {"STATEMNG_STOP",            HI_EVENT_STATEMNG_STOP},
        {"STATEMNG_SETTING",         HI_EVENT_STATEMNG_SETTING},
        {"STATEMNG_POWEROFF",        HI_EVENT_STATEMNG_POWEROFF},
        {"STATEMNG_SD_AVAILABLE",    HI_EVENT_STATEMNG_SD_AVAILABLE},
        {"STATEMNG_SD_UNAVAILABLE",  HI_EVENT_STATEMNG_SD_UNAVAILABLE},
        {"STATEMNG_SD_FORMAT",       HI_EVENT_STATEMNG_SD_FORMAT},
        {"STATEMNG_VO_SWITCH",       HI_EVENT_STATEMNG_VO_SWITCH},
        {"STATEMNG_FACTORY_RESET",   HI_EVENT_STATEMNG_FACTORY_RESET},

        {"PDT_PARAM_CHANGE",         HI_EVENT_PDT_PARAM_CHANGE},

        {"STORAGEMNG_DEV_UNPLUGED",   HI_EVENT_STORAGEMNG_DEV_UNPLUGED},
        {"STORAGEMNG_DEV_CONNECTING", HI_EVENT_STORAGEMNG_DEV_CONNECTING},
        {"STORAGEMNG_DEV_ERROR",      HI_EVENT_STORAGEMNG_DEV_ERROR},
        {"STORAGEMNG_FS_CHECKING",    HI_EVENT_STORAGEMNG_FS_CHECKING},
        {"STORAGEMNG_FS_CHECK_FAILED",HI_EVENT_STORAGEMNG_FS_CHECK_FAILED},
        {"STORAGEMNG_FS_EXCEPTION",   HI_EVENT_STORAGEMNG_FS_EXCEPTION},
        {"STORAGEMNG_MOUNTED",        HI_EVENT_STORAGEMNG_MOUNTED},
        {"STORAGEMNG_MOUNT_FAILED",   HI_EVENT_STORAGEMNG_MOUNT_FAILED},
    };

    HI_U32 i = 0;
    for (i=0; i < NETCTRL_ARRAY_SIZE(astEventDescTable); i++)
    {
        if (astEventDescTable[i].s32Value == u32EventID)
        {
            snprintf(pszDesc, u32BufLen, "%s", astEventDescTable[i].szDesc);
            return HI_SUCCESS;
        }
    }

    MLOGE("the event ID: %#x not support!\n", u32EventID);
    return HI_FAILURE;
}


/** get workmode value by workmode description */
HI_S32 PDT_NETCTRL_GetWorkModeValueByDesc(HI_CHAR* pDesc, HI_S32* ps32WorkMode)
{
    HI_APPCOMM_CHECK_EXPR(pDesc, HI_PDT_NETCTRL_EINVAL);
    HI_APPCOMM_CHECK_EXPR(ps32WorkMode, HI_PDT_NETCTRL_EINVAL);

    HI_U32 i;
    PDT_NETCTRL_ITEM_VALUE_S astWorkModeValueSet[ ] = {
        {PDT_NETCTRL_NORM_REC,  HI_PDT_WORKMODE_NORM_REC},
        {PDT_NETCTRL_LOOP_REC,  HI_PDT_WORKMODE_LOOP_REC},
        {PDT_NETCTRL_LPSE_REC,  HI_PDT_WORKMODE_LPSE_REC},
        {PDT_NETCTRL_SLOW_REC,  HI_PDT_WORKMODE_SLOW_REC},
        {PDT_NETCTRL_SING_PHOTO,HI_PDT_WORKMODE_SING_PHOTO},
        {PDT_NETCTRL_DLAY_PHOTO,HI_PDT_WORKMODE_DLAY_PHOTO},
        {PDT_NETCTRL_LPSE_PHOTO,HI_PDT_WORKMODE_LPSE_PHOTO},
        {PDT_NETCTRL_BURST,     HI_PDT_WORKMODE_BURST},
        {PDT_NETCTRL_RECSNAP,   HI_PDT_WORKMODE_RECSNAP},
        {PDT_NETCTRL_PLAYBACK,  HI_PDT_WORKMODE_PLAYBACK},
        {PDT_NETCTRL_UVC,       HI_PDT_WORKMODE_UVC},
        {PDT_NETCTRL_UPGRADE,   HI_PDT_WORKMODE_UPGRADE},
        {PDT_NETCTRL_USB_STORAGE,   HI_PDT_WORKMODE_USB_STORAGE},
        {PDT_NETCTRL_SUSPEND,       HI_PDT_WORKMODE_SUSPEND},
        {PDT_NETCTRL_HDMI_PREVIEW,  HI_PDT_WORKMODE_HDMI_PREVIEW},
        {PDT_NETCTRL_HDMI_PLAYBACK, HI_PDT_WORKMODE_HDMI_PLAYBACK},
    };

    HI_S32 s32ArryaSize = NETCTRL_ARRAY_SIZE(astWorkModeValueSet);
    for (i=0; i<s32ArryaSize; i++)
    {
        if (0 == strncasecmp(astWorkModeValueSet[i].szDesc, pDesc, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN))
        {
            *ps32WorkMode = astWorkModeValueSet[i].s32Value;
            break;
        }
    }

    if (i >= s32ArryaSize)
    {
        MLOGE("the workmode:%s not support!\n",pDesc);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/** get workmode description by workmode value */
HI_S32 PDT_NETCTRL_GetWorkModeDescByValue(HI_CHAR* pDesc, HI_S32 s32WorkMode, HI_S32 u32SiZe)
{
    HI_APPCOMM_CHECK_EXPR(pDesc, HI_PDT_NETCTRL_EINVAL);

    HI_U32 i;
    PDT_NETCTRL_ITEM_VALUE_S astWorkModeValueSet[ ] = {
        {PDT_NETCTRL_NORM_REC,  HI_PDT_WORKMODE_NORM_REC},
        {PDT_NETCTRL_LOOP_REC,  HI_PDT_WORKMODE_LOOP_REC},
        {PDT_NETCTRL_LPSE_REC,  HI_PDT_WORKMODE_LPSE_REC},
        {PDT_NETCTRL_SLOW_REC,  HI_PDT_WORKMODE_SLOW_REC},
        {PDT_NETCTRL_SING_PHOTO,HI_PDT_WORKMODE_SING_PHOTO},
        {PDT_NETCTRL_DLAY_PHOTO,HI_PDT_WORKMODE_DLAY_PHOTO},
        {PDT_NETCTRL_LPSE_PHOTO,HI_PDT_WORKMODE_LPSE_PHOTO},
        {PDT_NETCTRL_BURST,     HI_PDT_WORKMODE_BURST},
        {PDT_NETCTRL_RECSNAP,   HI_PDT_WORKMODE_RECSNAP},
        {PDT_NETCTRL_PLAYBACK,  HI_PDT_WORKMODE_PLAYBACK},
        {PDT_NETCTRL_UVC,       HI_PDT_WORKMODE_UVC},
        {PDT_NETCTRL_UPGRADE,   HI_PDT_WORKMODE_UPGRADE},
        {PDT_NETCTRL_USB_STORAGE,   HI_PDT_WORKMODE_USB_STORAGE},
        {PDT_NETCTRL_SUSPEND,       HI_PDT_WORKMODE_SUSPEND},
        {PDT_NETCTRL_HDMI_PREVIEW,  HI_PDT_WORKMODE_HDMI_PREVIEW},
        {PDT_NETCTRL_HDMI_PLAYBACK, HI_PDT_WORKMODE_HDMI_PLAYBACK},
    };

    HI_S32 s32ArryaSize = NETCTRL_ARRAY_SIZE(astWorkModeValueSet);
    for (i=0; i<s32ArryaSize; i++)
    {
        if (astWorkModeValueSet[i].s32Value == s32WorkMode)
        {
            snprintf(pDesc,u32SiZe,"%s",astWorkModeValueSet[i].szDesc);
            break;
        }
    }

    if (i >= s32ArryaSize)
    {
        MLOGE("the workmode:%s not support!\n",pDesc);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


/** get paramtype value by paramtype description */
HI_S32 PDT_NETCTRL_GetParamTypeValueByDesc(HI_CHAR* pDesc, HI_S32* ps32WorkMode)
{
    HI_APPCOMM_CHECK_EXPR(pDesc, HI_PDT_NETCTRL_EINVAL);
    HI_APPCOMM_CHECK_EXPR(ps32WorkMode, HI_PDT_NETCTRL_EINVAL);

    HI_U32 i;
    PDT_NETCTRL_ITEM_VALUE_S astParamTypeValueSet[ ] = {
        {PDT_NETCTRL_PARAM_TYPE_MEDIAMODE,          HI_PDT_PARAM_TYPE_MEDIAMODE},
        {PDT_NETCTRL_PARAM_TYPE_PHOTO_SCENE,        HI_PDT_PARAM_TYPE_PHOTO_SCENE},
        {PDT_NETCTRL_PARAM_TYPE_PHOTO_OUTPUT_FMT,   HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT},
        {PDT_NETCTRL_PARAM_TYPE_DELAY_TIME,         HI_PDT_PARAM_TYPE_DELAY_TIME},
        {PDT_NETCTRL_PARAM_TYPE_LAPSE_INTERVAL,     HI_PDT_PARAM_TYPE_LAPSE_INTERVAL},
        {PDT_NETCTRL_PARAM_TYPE_BURST_TYPE,         HI_PDT_PARAM_TYPE_BURST_TYPE},
        {PDT_NETCTRL_PARAM_TYPE_LOOP_TIME,          HI_PDT_PARAM_TYPE_LOOP_TIME},
        {PDT_NETCTRL_PARAM_TYPE_ENC_PAYLOAD_TYPE,   HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE},
        {PDT_NETCTRL_PARAM_TYPE_PROTUNE_EXP_EV,     HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV},
        {PDT_NETCTRL_PARAM_TYPE_PROTUNE_EXP_TIME,   HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME},
        {PDT_NETCTRL_PARAM_TYPE_PROTUNE_ISO,        HI_PDT_PARAM_TYPE_PROTUNE_ISO},
        {PDT_NETCTRL_PARAM_TYPE_PROTUNE_WB,         HI_PDT_PARAM_TYPE_PROTUNE_WB},
        {PDT_NETCTRL_PARAM_TYPE_PROTUNE_METRY,      HI_PDT_PARAM_TYPE_PROTUNE_METRY},
        {PDT_NETCTRL_PARAM_TYPE_OSD,                HI_PDT_PARAM_TYPE_OSD},
        {PDT_NETCTRL_PARAM_TYPE_AUDIO,              HI_PDT_PARAM_TYPE_AUDIO},
        {PDT_NETCTRL_PARAM_TYPE_FLIP,               HI_PDT_PARAM_TYPE_FLIP},
        {PDT_NETCTRL_PARAM_TYPE_DIS,                HI_PDT_PARAM_TYPE_DIS},
        {PDT_NETCTRL_PARAM_TYPE_LDC,                HI_PDT_PARAM_TYPE_LDC},
        {PDT_NETCTRL_PARAM_TYPE_POWERON_WORKMODE,   HI_PDT_PARAM_TYPE_POWERON_WORKMODE},
        {PDT_NETCTRL_PARAM_TYPE_POWERON_ACTION,     HI_PDT_PARAM_TYPE_POWERON_ACTION},
        {PDT_NETCTRL_PARAM_TYPE_WIFI_AP,            HI_PDT_PARAM_TYPE_WIFI_AP},
        {PDT_NETCTRL_PARAM_TYPE_DEV_INFO,           HI_PDT_PARAM_TYPE_DEV_INFO},
        {PDT_NETCTRL_PARAM_TYPE_VOLUME,             HI_PDT_PARAM_TYPE_VOLUME},
        {PDT_NETCTRL_PARAM_TYPE_SCREEN_BRIGHTNESS,  HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS},
        {PDT_NETCTRL_PARAM_TYPE_SCREEN_DORMANT_MODE,HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE},
        {PDT_NETCTRL_PARAM_TYPE_SYS_DORMANT_MODE,   HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE}
    };

    HI_S32 s32ArryaSize = NETCTRL_ARRAY_SIZE(astParamTypeValueSet);
    for (i=0; i<s32ArryaSize; i++)
    {
        if (0 == strncasecmp(astParamTypeValueSet[i].szDesc, pDesc, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN))
        {
            *ps32WorkMode = astParamTypeValueSet[i].s32Value;
            break;
        }
    }

    if (i >= s32ArryaSize)
    {
        MLOGE("the paramtype:%s not support!\n",pDesc);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/** get paramtype description by workmode value */
HI_S32 PDT_NETCTRL_GetParamTypeDescByValue(HI_CHAR* pDesc, HI_S32 s32WorkMode, HI_S32 u32SiZe)
{
    HI_APPCOMM_CHECK_EXPR(pDesc, HI_PDT_NETCTRL_EINVAL);

    HI_U32 i;
    PDT_NETCTRL_ITEM_VALUE_S astParamTypeValueSet[ ] = {
        {PDT_NETCTRL_PARAM_TYPE_MEDIAMODE,          HI_PDT_PARAM_TYPE_MEDIAMODE},
        {PDT_NETCTRL_PARAM_TYPE_PHOTO_SCENE,        HI_PDT_PARAM_TYPE_PHOTO_SCENE},
        {PDT_NETCTRL_PARAM_TYPE_PHOTO_OUTPUT_FMT,   HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT},
        {PDT_NETCTRL_PARAM_TYPE_DELAY_TIME,         HI_PDT_PARAM_TYPE_DELAY_TIME},
        {PDT_NETCTRL_PARAM_TYPE_LAPSE_INTERVAL,     HI_PDT_PARAM_TYPE_LAPSE_INTERVAL},
        {PDT_NETCTRL_PARAM_TYPE_BURST_TYPE,         HI_PDT_PARAM_TYPE_BURST_TYPE},
        {PDT_NETCTRL_PARAM_TYPE_LOOP_TIME,          HI_PDT_PARAM_TYPE_LOOP_TIME},
        {PDT_NETCTRL_PARAM_TYPE_ENC_PAYLOAD_TYPE,   HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE},
        {PDT_NETCTRL_PARAM_TYPE_PROTUNE_EXP_EV,     HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV},
        {PDT_NETCTRL_PARAM_TYPE_PROTUNE_EXP_TIME,   HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME},
        {PDT_NETCTRL_PARAM_TYPE_PROTUNE_ISO,        HI_PDT_PARAM_TYPE_PROTUNE_ISO},
        {PDT_NETCTRL_PARAM_TYPE_PROTUNE_WB,         HI_PDT_PARAM_TYPE_PROTUNE_WB},
        {PDT_NETCTRL_PARAM_TYPE_PROTUNE_METRY,      HI_PDT_PARAM_TYPE_PROTUNE_METRY},
        {PDT_NETCTRL_PARAM_TYPE_OSD,                HI_PDT_PARAM_TYPE_OSD},
        {PDT_NETCTRL_PARAM_TYPE_AUDIO,              HI_PDT_PARAM_TYPE_AUDIO},
        {PDT_NETCTRL_PARAM_TYPE_FLIP,               HI_PDT_PARAM_TYPE_FLIP},
        {PDT_NETCTRL_PARAM_TYPE_DIS,                HI_PDT_PARAM_TYPE_DIS},
        {PDT_NETCTRL_PARAM_TYPE_LDC,                HI_PDT_PARAM_TYPE_LDC},
        {PDT_NETCTRL_PARAM_TYPE_POWERON_WORKMODE,   HI_PDT_PARAM_TYPE_POWERON_WORKMODE},
        {PDT_NETCTRL_PARAM_TYPE_POWERON_ACTION,     HI_PDT_PARAM_TYPE_POWERON_ACTION},
        {PDT_NETCTRL_PARAM_TYPE_WIFI_AP,            HI_PDT_PARAM_TYPE_WIFI_AP},
        {PDT_NETCTRL_PARAM_TYPE_DEV_INFO,           HI_PDT_PARAM_TYPE_DEV_INFO},
        {PDT_NETCTRL_PARAM_TYPE_VOLUME,             HI_PDT_PARAM_TYPE_VOLUME},
        {PDT_NETCTRL_PARAM_TYPE_SCREEN_BRIGHTNESS,  HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS},
        {PDT_NETCTRL_PARAM_TYPE_SCREEN_DORMANT_MODE,HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE},
        {PDT_NETCTRL_PARAM_TYPE_SYS_DORMANT_MODE,   HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE},
        {PDT_NETCTRL_PARAM_TYPE_USB_MODE,           HI_PDT_PARAM_TYPE_USB_MODE},
        {PDT_NETCTRL_PARAM_TYPE_KEYTONE,            HI_PDT_PARAM_TYPE_KEYTONE},
        {PDT_NETCTRL_PARAM_TYPE_LANGUAGE,           HI_PDT_PARAM_TYPE_LANGUAGE}
    };

    HI_S32 s32ArryaSize = NETCTRL_ARRAY_SIZE(astParamTypeValueSet);
    for (i=0; i<s32ArryaSize; i++)
    {
        if (astParamTypeValueSet[i].s32Value == s32WorkMode)
        {
            snprintf(pDesc,u32SiZe,"%s",astParamTypeValueSet[i].szDesc);
            break;
        }
    }

    if (i >= s32ArryaSize)
    {
        MLOGE("the paramtype:%s not support!\n",pDesc);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_GetDescFromValueSet(HI_S32 s32Value, const HI_PDT_ITEM_VALUESET_S * pstValueSet,
    HI_CHAR* pDesc, HI_U32 u32Size)
{
    HI_U32 i = 0;
    HI_APPCOMM_CHECK_POINTER(pstValueSet, HI_PDT_NETCTRL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pDesc,   HI_PDT_NETCTRL_EINVAL);

    for(i=0; i<pstValueSet->s32Cnt && i<HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
    {
        if(s32Value == pstValueSet->astValues[i].s32Value)
        {
            snprintf(pDesc, u32Size, "%s", pstValueSet->astValues[i].szDesc);
            return HI_SUCCESS;
        }
    }
    return HI_FAILURE;
}


/** Get  param type's  current value(string)  */
HI_S32 PDT_NETCTRL_GetParamValueDesc(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_TYPE_E enType,
    HI_CHAR* pDesc, HI_U32 u32Size)
{
    HI_S32 s32Value = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_ITEM_VALUESET_S stValueSet = {};
    HI_TIMEDTASK_ATTR_S stTimedTaskAttr = {};

    HI_APPCOMM_CHECK_POINTER(pDesc, HI_PDT_NETCTRL_EINVAL);

    switch(enType)
    {
        /*workmode dependent, have value set*/
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
        case HI_PDT_PARAM_TYPE_DELAY_TIME:
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = HI_PDT_PARAM_GetWorkModeParam(enWorkMode, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetWorkModeParam");

            s32Ret = HI_PDT_PARAM_GetWorkModeItemValues(enWorkMode, enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetWorkModeItemValues");

            s32Ret = PDT_NETCTRL_GetDescFromValueSet(s32Value, &stValueSet, pDesc, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetDescFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_AUDIO:
        case HI_PDT_PARAM_TYPE_DIS:
        case HI_PDT_PARAM_TYPE_LDC:
        case HI_PDT_PARAM_TYPE_OSD:
            s32Ret = HI_PDT_PARAM_GetWorkModeParam(enWorkMode, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");
            snprintf(pDesc,u32Size,"%d",s32Value);
            break;

        /*items below are workmode independent*/
        case HI_PDT_PARAM_TYPE_VOLUME:
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");
            snprintf(pDesc, u32Size, "%d", s32Value);
            break;

        case HI_PDT_PARAM_TYPE_POWERON_ACTION:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");

            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommItemValues");

            s32Ret = PDT_NETCTRL_GetDescFromValueSet(s32Value, &stValueSet, pDesc, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetDescFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
        case HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &stTimedTaskAttr);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");

            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommItemValues");

            if( !stTimedTaskAttr.bEnable )
            {
                s32Ret = PDT_NETCTRL_GetDescFromValueSet(0, &stValueSet, pDesc, u32Size);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetDescFromValueSet");
                return HI_SUCCESS;
            }

            s32Ret = PDT_NETCTRL_GetDescFromValueSet(stTimedTaskAttr.u32Time_sec, &stValueSet, pDesc, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetDescFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_WIFI_AP:
        case HI_PDT_PARAM_TYPE_DEV_INFO:
        case HI_PDT_PARAM_TYPE_POWERON_WORKMODE:
        default:
            MLOGE("not support ParamType[%d]\n", enType);
            return HI_FAILURE;
    }

    return  HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_GetValueFromValueSet(HI_S32* ps32Value, const HI_PDT_ITEM_VALUESET_S * pstValueSet,
    HI_CHAR* pDesc)
{
    HI_U32 i = 0;
    HI_APPCOMM_CHECK_POINTER(pstValueSet, HI_PDT_NETCTRL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pDesc,   HI_PDT_NETCTRL_EINVAL);

    for(i=0; i<pstValueSet->s32Cnt && i<HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
    {
        if (0 == strncmp(pDesc,pstValueSet->astValues[i].szDesc,HI_PDT_ITEM_VALUE_DESCRIPTION_LEN))
        {
            *ps32Value = pstValueSet->astValues[i].s32Value;
            return HI_SUCCESS;
        }
    }
    return HI_FAILURE;
}


/** Get  param type's  current value by description  */
HI_S32 PDT_NETCTRL_GetParamValueByDesc(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_TYPE_E enType,
    HI_CHAR* pDesc, HI_U32 u32Size, HI_S32* ps32Value)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_ITEM_VALUESET_S stValueSet = {};

    HI_APPCOMM_CHECK_POINTER(pDesc, HI_PDT_NETCTRL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(ps32Value, HI_PDT_NETCTRL_EINVAL);

    switch(enType)
    {
        /*workmode dependent, have value set*/
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
        case HI_PDT_PARAM_TYPE_DELAY_TIME:
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = HI_PDT_PARAM_GetWorkModeItemValues(enWorkMode, enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetWorkModeItemValues");

            s32Ret = PDT_NETCTRL_GetValueFromValueSet(ps32Value, &stValueSet, pDesc);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetValueFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_AUDIO:
        case HI_PDT_PARAM_TYPE_DIS:
        case HI_PDT_PARAM_TYPE_LDC:
        case HI_PDT_PARAM_TYPE_OSD:
            if (0 == strncmp(PDT_NETCTRL_OFF, pDesc, strnlen(pDesc, u32Size)))
            {
                *(HI_BOOL*)ps32Value = HI_FALSE;
            }
            else if (0 == strncmp(PDT_NETCTRL_ON, pDesc, strnlen(pDesc, u32Size)))
            {
                *(HI_BOOL*)ps32Value = HI_TRUE;
            }
            else
            {
                MLOGE("Param Type[%d], not support param value[%s]\n",  enType, pDesc);
                return HI_FAILURE;
            }
            break;

        /*items below are workmode independent*/
        case HI_PDT_PARAM_TYPE_VOLUME:
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            *ps32Value = atoi(pDesc);
            break;

        case HI_PDT_PARAM_TYPE_POWERON_ACTION:
            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommItemValues");

            s32Ret = PDT_NETCTRL_GetValueFromValueSet(ps32Value, &stValueSet, pDesc);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetValueFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
        case HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE:
            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommItemValues");

            s32Ret = PDT_NETCTRL_GetValueFromValueSet(ps32Value, &stValueSet, pDesc);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetValueFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_WIFI_AP:
        case HI_PDT_PARAM_TYPE_DEV_INFO:
        case HI_PDT_PARAM_TYPE_POWERON_WORKMODE:
        default:
            MLOGE("not support ParamType[%d]\n", enType);
            return HI_FAILURE;
    }

    return  HI_SUCCESS;
}

HI_S32 PDT_NETCTRL_SetCommParam(HI_PDT_PARAM_TYPE_E enType, HI_VOID *pvParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TIMEDTASK_ATTR_S stTimedTaskAttr = {};
    HI_APPCOMM_CHECK_POINTER(pvParam, HI_PDT_NETCTRL_EINVAL);

    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_POWERON_ACTION:
        case HI_PDT_PARAM_TYPE_WIFI_AP:
        case HI_PDT_PARAM_TYPE_VOLUME:
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            s32Ret = HI_PDT_PARAM_SetCommParam(enType, pvParam);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            if(HI_PDT_PARAM_TYPE_VOLUME == enType)
            {
                HI_PDT_WORKMODE_E enPowerOnWorkMode = HI_PDT_WORKMODE_BUTT;
                s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enPowerOnWorkMode);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
                s32Ret = HI_PDT_PARAM_GetWorkModeParam(enPowerOnWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE, &enMediaMode);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                HI_PDT_MEDIA_CFG_S stMediaCfg = {};
                HI_PDT_SCENE_MODE_S stSceneMode = {};
                s32Ret = HI_PDT_PARAM_GetMediaCfg(HI_PDT_WORKMODE_PLAYBACK, enMediaMode, &stMediaCfg, &stSceneMode);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                s32Ret = HI_PDT_MEDIA_SetAoVolume(stMediaCfg.stAudioOutCfg.astAoCfg->AoHdl, *(HI_U32 *)pvParam);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            break;

        case HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE:
        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            if (*(HI_S32*)pvParam > 0)
            {
                stTimedTaskAttr.bEnable = HI_TRUE;
            }
            stTimedTaskAttr.u32Time_sec = *(HI_S32*)pvParam;
            s32Ret = HI_PDT_PARAM_SetCommParam(enType, &stTimedTaskAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            HI_POWERCTRL_TASK_E enPwrCtrlType = (HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE == enType
                ? HI_PWRCTRL_TASK_SYSTEMDORMANT : HI_PWRCTRL_TASK_SCREENDORMANT);
            s32Ret = HI_POWERCTRL_SetTaskAttr(enPwrCtrlType, &stTimedTaskAttr);
            if(s32Ret)
            {
                MLOGE("SetTaskAttr failed. PwrCtrlType:%d\n", enType);
            }
            break;

        default:
            MLOGE("Unsupport Common ParamType[%d]\n\n", enType);
            return HI_PDT_NETCTRL_EUNSUPPORT;
    }

    return HI_SUCCESS;
}

/** get log level value by description */
HI_S32 PDT_NETCTRL_GetLogLevelValueByDesc(HI_CHAR* pDesc, HI_S32* ps32LogLevel)
{
    HI_APPCOMM_CHECK_EXPR(pDesc, HI_PDT_NETCTRL_EINVAL);
    HI_APPCOMM_CHECK_EXPR(ps32LogLevel, HI_PDT_NETCTRL_EINVAL);

    HI_U32 i;
    PDT_NETCTRL_ITEM_VALUE_S astLogLevelTypeValueSet[ ] = {
        {PDT_NETCTRL_LOG_FATAL,     HI_LOG_LEVEL_FATAL},
        {PDT_NETCTRL_LOG_ERROR,     HI_LOG_LEVEL_ERROR},
        {PDT_NETCTRL_LOG_WARNING,   HI_LOG_LEVEL_WARNING},
        {PDT_NETCTRL_LOG_INFO,      HI_LOG_LEVEL_INFO},
        {PDT_NETCTRL_LOG_DEBUG,     HI_LOG_LEVEL_DEBUG}
    };

    HI_S32 s32ArryaSize = NETCTRL_ARRAY_SIZE(astLogLevelTypeValueSet);
    for (i=0; i<s32ArryaSize; i++)
    {
        if (0 == strncasecmp(astLogLevelTypeValueSet[i].szDesc, pDesc, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN))
        {
            *ps32LogLevel = astLogLevelTypeValueSet[i].s32Value;
            break;
        }
    }

    if (i >= s32ArryaSize)
    {
        MLOGE("the log level type:%s not support!\n",pDesc);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_NETCTRL_SetTime(const HISNET_TIME_ATTR_S *pstSysTime)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR tmp[8] = {0};
    HI_SYSTEM_TM_S stDateTime;

    if(HI_NULL == pstSysTime->aszTime)
    {
        MLOGE("azTime is NULL!\n");
        return HI_FAILURE;
    }

    memset(&stDateTime, 0, sizeof(HI_SYSTEM_TM_S));

    strncpy(tmp, &pstSysTime->aszTime[0], 4);
    stDateTime.s32year = atoi(tmp);

    memset(tmp, 0x0, sizeof(tmp));
    strncpy(tmp, &pstSysTime->aszTime[4], 2);
    stDateTime.s32mon = atoi(tmp);

    memset(tmp, 0x0, sizeof(tmp));
    strncpy(tmp, &pstSysTime->aszTime[6], 2);
    stDateTime.s32mday = atoi(tmp);

    memset(tmp, 0x0, sizeof(tmp));
    strncpy(tmp, &pstSysTime->aszTime[8], 2);
    stDateTime.s32hour = atoi(tmp);

    memset(tmp, 0x0, sizeof(tmp));
    strncpy(tmp, &pstSysTime->aszTime[10], 2);
    stDateTime.s32min = atoi(tmp);

    memset(tmp, 0x0, sizeof(tmp));
    strncpy(tmp, &pstSysTime->aszTime[12], 2);
    stDateTime.s32sec = atoi(tmp);

    MLOGD("%d-%02d-%02d  %02d:%02d\n", stDateTime.s32year, stDateTime.s32mon,
        stDateTime.s32mday, stDateTime.s32hour, stDateTime.s32sec);

    s32Ret = HI_SYSTEM_SetDateTime(&stDateTime);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "SetDateTime");

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

