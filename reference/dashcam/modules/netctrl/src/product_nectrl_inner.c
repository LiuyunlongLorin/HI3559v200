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
#include "hi_powercontrol.h"
#include "hi_gsensormng.h"
#include "hi_dtcf.h"
#include "hi_product_media.h"

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
        (HI_U64)pstFileInfo->s64FileSize>>20, (HI_U64)pstFileInfo->s64FileSize>>10&0x3ff, (HI_U64)pstFileInfo->s64FileSize&0x3ff);
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

    HI_CHAR *pSuffix = NULL;
    pSuffix = strrchr(pazFileName, '.');
    if (!pSuffix)
    {
        MLOGW("<<<<<<<<<<<<Invalid File:"RED"%s"NONE"\n", pazFileName);
        return HI_FAILURE;
    }
    MLOGD("File:%s, Suffix:%s\n", pazFileName, pSuffix);
    HI_DEMUX_S *pDemuxerOps = NULL;
    if (!strncmp(pSuffix, ".MP4", sizeof(".MP4")) || !strncmp(pSuffix, ".LRV", sizeof(".LRV")))
    {
        /* Get Demuxer OpsSet */
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



HI_S32 PDT_NETCTRL_SetTime(const HISNET_TIME_ATTR_S *pstSysTime)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR tmp[8] = {0};
    HI_SYSTEM_TM_S stDateTime;

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


/** get eventID desc string  by eventID value */
HI_S32 PDT_NETCTRL_GetEventIDDescByValue(HI_U32 u32EventID, HI_CHAR* pszDesc, HI_U32 u32BufLen)
{
    HI_APPCOMM_CHECK_EXPR(pszDesc, HI_PDT_NETCTRL_EINVAL);

    PDT_NETCTRL_ITEM_VALUE_S astEventDescTable[] = {
        {"STATEMNG_SWITCH_WORKMODE", HI_EVENT_STATEMNG_SWITCH_WORKMODE},
        {"STATEMNG_LIVE_SWITCH",     HI_EVENT_STATEMNG_LIVE_SWITCH},
        {"STATEMNG_VO_SWITCH",       HI_EVENT_STATEMNG_VO_SWITCH},
        {"STATEMNG_START",           HI_EVENT_STATEMNG_START},
        {"STATEMNG_STOP",            HI_EVENT_STATEMNG_STOP},
        {"STATEMNG_TRIGGER",         HI_EVENT_STATEMNG_TRIGGER},
        {"STATEMNG_SETTING",         HI_EVENT_STATEMNG_SETTING},
        {"STATEMNG_POWEROFF",        HI_EVENT_STATEMNG_POWEROFF},
        {"STATEMNG_SD_AVAILABLE",    HI_EVENT_STATEMNG_SD_AVAILABLE},
        {"STATEMNG_SD_UNAVAILABLE",  HI_EVENT_STATEMNG_SD_UNAVAILABLE},
        {"STATEMNG_SD_FORMAT",       HI_EVENT_STATEMNG_SD_FORMAT},
        {"STATEMNG_EMR_END",         HI_EVENT_STATEMNG_EMR_END},
        {"STATEMNG_EMR_BEGIN",       HI_EVENT_STATEMNG_EMR_BEGIN},
        {"STATEMNG_PREVIEW_PIP",     HI_EVENT_STATEMNG_PREVIEW_PIP},

        {"PDT_NETCTRL_UPGRADE",   HI_PDT_WORKMODE_UPGRADE},

        {"PDT_PARAM_CHANGE",         HI_EVENT_PDT_PARAM_CHANGE},

        {"GSENSORMNG_COLLISION",     HI_EVENT_GSENSORMNG_COLLISION},

        {"STORAGEMNG_DEV_UNPLUGED",   HI_EVENT_STORAGEMNG_DEV_UNPLUGED},
        {"STORAGEMNG_DEV_CONNECTING", HI_EVENT_STORAGEMNG_DEV_CONNECTING},
        {"STORAGEMNG_DEV_ERROR",      HI_EVENT_STORAGEMNG_DEV_ERROR},
        {"STORAGEMNG_FS_CHECKING",    HI_EVENT_STORAGEMNG_FS_CHECKING},
        {"STORAGEMNG_FS_CHECK_FAILED",HI_EVENT_STORAGEMNG_FS_CHECK_FAILED},
        {"STORAGEMNG_FS_EXCEPTION",   HI_EVENT_STORAGEMNG_FS_EXCEPTION},
        {"STORAGEMNG_MOUNTED",        HI_EVENT_STORAGEMNG_MOUNTED},
        {"STORAGEMNG_MOUNT_FAILED",   HI_EVENT_STORAGEMNG_MOUNT_FAILED},
#ifdef CONFIG_MOTIONDETECT_ON
        {"MD_MOTIONSTATE_CHANGE",   HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE},
#endif
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

    PDT_NETCTRL_ITEM_VALUE_S astWorkModeValueSet[ ] = {
        {PDT_NETCTRL_WORKMODE_NORM_REC,    HI_PDT_WORKMODE_NORM_REC},
        {PDT_NETCTRL_WORKMODE_PHOTO,       HI_PDT_WORKMODE_PHOTO},
        {PDT_NETCTRL_WORKMODE_PLAYBACK,    HI_PDT_WORKMODE_PLAYBACK},
        {PDT_NETCTRL_WORKMODE_UVC,         HI_PDT_WORKMODE_UVC},
        {PDT_NETCTRL_WORKMODE_USB_STORAGE, HI_PDT_WORKMODE_USB_STORAGE},
        {PDT_NETCTRL_WORKMODE_PARKING_REC, HI_PDT_WORKMODE_PARKING_REC},
    };


    HI_U32 i = 0;
    for (i=0; i < NETCTRL_ARRAY_SIZE(astWorkModeValueSet); i++)
    {
        if (0 == strncasecmp(astWorkModeValueSet[i].szDesc, pDesc, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN))
        {
            *ps32WorkMode = astWorkModeValueSet[i].s32Value;
            return HI_SUCCESS;
        }
    }

    MLOGE("the workmode:%d not support!\n", *ps32WorkMode);
    return HI_FAILURE;
}

/** get workmode description by workmode value */
HI_S32 PDT_NETCTRL_GetWorkModeDescByValue(HI_CHAR* pDesc, HI_S32 s32WorkMode, HI_S32 u32SiZe)
{
    HI_APPCOMM_CHECK_EXPR(pDesc, HI_PDT_NETCTRL_EINVAL);

    PDT_NETCTRL_ITEM_VALUE_S astWorkModeValueSet[ ] = {
        {PDT_NETCTRL_WORKMODE_NORM_REC,    HI_PDT_WORKMODE_NORM_REC},
        {PDT_NETCTRL_WORKMODE_PHOTO,       HI_PDT_WORKMODE_PHOTO},
        {PDT_NETCTRL_WORKMODE_PLAYBACK,    HI_PDT_WORKMODE_PLAYBACK},
        {PDT_NETCTRL_WORKMODE_UVC,         HI_PDT_WORKMODE_UVC},
        {PDT_NETCTRL_WORKMODE_USB_STORAGE, HI_PDT_WORKMODE_USB_STORAGE},
        {PDT_NETCTRL_WORKMODE_PARKING_REC, HI_PDT_WORKMODE_PARKING_REC},
        {PDT_NETCTRL_WORKMODE_UPGRADE,     HI_PDT_WORKMODE_UPGRADE},
    };


    HI_U32 i = 0;
    for (i=0; i < NETCTRL_ARRAY_SIZE(astWorkModeValueSet); i++)
    {
        if (astWorkModeValueSet[i].s32Value == s32WorkMode)
        {
            snprintf(pDesc,u32SiZe,"%s",astWorkModeValueSet[i].szDesc);
            return HI_SUCCESS;
        }
    }

    MLOGE("the workmode:%d not support!\n",s32WorkMode);
    return HI_FAILURE;

}

/** get paramtype value by paramtype description */
HI_S32 PDT_NETCTRL_GetParamTypeValueByDesc(HI_CHAR* pDesc, HI_S32* ps32ParamType)
{
    HI_APPCOMM_CHECK_EXPR(pDesc, HI_PDT_NETCTRL_EINVAL);
    HI_APPCOMM_CHECK_EXPR(ps32ParamType, HI_PDT_NETCTRL_EINVAL);

    HI_U32 i;
    PDT_NETCTRL_ITEM_VALUE_S astParamTypeValueSet[ ] = {
        {PDT_NETCTRL_PARAM_TYPE_MEDIAMODE,          HI_PDT_PARAM_TYPE_MEDIAMODE},
        {PDT_NETCTRL_PARAM_TYPE_ENC_PAYLOAD_TYPE,   HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE},
        {PDT_NETCTRL_PARAM_TYPE_FLIP,               HI_PDT_PARAM_TYPE_FLIP},
        {PDT_NETCTRL_PARAM_TYPE_MIRROR,             HI_PDT_PARAM_TYPE_MIRROR},
        {PDT_NETCTRL_PARAM_TYPE_LDC,                HI_PDT_PARAM_TYPE_LDC},
        {PDT_NETCTRL_PARAM_TYPE_BAKE_REC,           HI_PDT_PARAM_TYPE_REC},
        {PDT_NETCTRL_PARAM_TYPE_OSD,                HI_PDT_PARAM_TYPE_OSD},
        {PDT_NETCTRL_PARAM_TYPE_AUDIO,              HI_PDT_PARAM_TYPE_AUDIO},
        {PDT_NETCTRL_PARAM_TYPE_PREVIEW_CAMID,      HI_PDT_PARAM_TYPE_PREVIEW_CAMID},
        {PDT_NETCTRL_PARAM_TYPE_WIFI_AP,            HI_PDT_PARAM_TYPE_WIFI_AP},
        {PDT_NETCTRL_PARAM_TYPE_DEV_INFO,           HI_PDT_PARAM_TYPE_DEV_INFO},
        {PDT_NETCTRL_PARAM_TYPE_VOLUME,             HI_PDT_PARAM_TYPE_VOLUME},
        {PDT_NETCTRL_PARAM_TYPE_SCREEN_BRIGHTNESS,  HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS},
        {PDT_NETCTRL_PARAM_TYPE_SCREEN_DORMANT_MODE,HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE},
        {PDT_NETCTRL_PARAM_TYPE_KEYTONE,            HI_PDT_PARAM_TYPE_KEYTONE},
        {PDT_NETCTRL_PARAM_TYPE_BOOTSOUND,          HI_PDT_PARAM_TYPE_BOOTSOUND},
        {PDT_NETCTRL_PARAM_TYPE_GSENSOR_SENSITIVITY,HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY},
        {PDT_NETCTRL_PARAM_TYPE_WDR,                HI_PDT_PARAM_TYPE_WDR},
        {PDT_NETCTRL_PARAM_TYPE_GSENSOR_PARKING,    HI_PDT_PARAM_TYPE_GSENSOR_PARKING},

        {PDT_NETCTRL_PARAM_TYPE_VIDEOMODE,          HI_PDT_PARAM_TYPE_VIDEOMODE},
        {PDT_NETCTRL_PARAM_TYPE_PREVIEWPIP,         HI_PDT_PARAM_TYPE_PREVIEWPIP},
        {PDT_NETCTRL_PARAM_TYPE_SPLITTIME,          HI_PDT_PARAM_TYPE_SPLITTIME},
        {PDT_NETCTRL_PARAM_TYPE_REC_TYPE,           HI_PDT_PARAM_RECORD_TYPE},
        {PDT_NETCTRL_PARAM_TYPE_LAPSE_INTERVAL,     HI_PDT_PARAM_TYPE_LAPSE_INTERVAL},
        {PDT_NETCTRL_PARAM_TYPE_MD_SENSITIVITY,     HI_PDT_PARAM_MD_SENSITIVITY},
    };

    HI_S32 s32ArryaSize = NETCTRL_ARRAY_SIZE(astParamTypeValueSet);
    for (i=0; i<s32ArryaSize; i++)
    {
        if (0 == strncasecmp(astParamTypeValueSet[i].szDesc, pDesc, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN))
        {
            *ps32ParamType = astParamTypeValueSet[i].s32Value;
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
HI_S32 PDT_NETCTRL_GetParamTypeDescByValue(HI_CHAR* pDesc, HI_S32 s32ParamType, HI_S32 u32SiZe)
{
    HI_APPCOMM_CHECK_EXPR(pDesc, HI_PDT_NETCTRL_EINVAL);

    HI_U32 i;
    PDT_NETCTRL_ITEM_VALUE_S astParamTypeValueSet[ ] = {
        {PDT_NETCTRL_PARAM_TYPE_MEDIAMODE,          HI_PDT_PARAM_TYPE_MEDIAMODE},
        {PDT_NETCTRL_PARAM_TYPE_ENC_PAYLOAD_TYPE,   HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE},
        {PDT_NETCTRL_PARAM_TYPE_FLIP,               HI_PDT_PARAM_TYPE_FLIP},
        {PDT_NETCTRL_PARAM_TYPE_MIRROR,             HI_PDT_PARAM_TYPE_MIRROR},
        {PDT_NETCTRL_PARAM_TYPE_LDC,                HI_PDT_PARAM_TYPE_LDC},
        {PDT_NETCTRL_PARAM_TYPE_BAKE_REC,           HI_PDT_PARAM_TYPE_REC},
        {PDT_NETCTRL_PARAM_TYPE_OSD,                HI_PDT_PARAM_TYPE_OSD},
        {PDT_NETCTRL_PARAM_TYPE_AUDIO,              HI_PDT_PARAM_TYPE_AUDIO},
        {PDT_NETCTRL_PARAM_TYPE_PREVIEW_CAMID,      HI_PDT_PARAM_TYPE_PREVIEW_CAMID},
        {PDT_NETCTRL_PARAM_TYPE_WIFI_AP,            HI_PDT_PARAM_TYPE_WIFI_AP},
        {PDT_NETCTRL_PARAM_TYPE_DEV_INFO,           HI_PDT_PARAM_TYPE_DEV_INFO},
        {PDT_NETCTRL_PARAM_TYPE_VOLUME,             HI_PDT_PARAM_TYPE_VOLUME},
        {PDT_NETCTRL_PARAM_TYPE_SCREEN_BRIGHTNESS,  HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS},
        {PDT_NETCTRL_PARAM_TYPE_SCREEN_DORMANT_MODE,HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE},
        {PDT_NETCTRL_PARAM_TYPE_KEYTONE,            HI_PDT_PARAM_TYPE_KEYTONE},
        {PDT_NETCTRL_PARAM_TYPE_BOOTSOUND,          HI_PDT_PARAM_TYPE_BOOTSOUND},
        {PDT_NETCTRL_PARAM_TYPE_GSENSOR_SENSITIVITY,HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY},
        {PDT_NETCTRL_PARAM_TYPE_USB_MODE,           HI_PDT_PARAM_TYPE_USB_MODE},
        {PDT_NETCTRL_PARAM_TYPE_GSENSOR_PARKING,    HI_PDT_PARAM_TYPE_GSENSOR_PARKING},
        {PDT_NETCTRL_PARAM_TYPE_PARKING_COUNT,      HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT},
        {PDT_NETCTRL_PARAM_TYPE_CROP,               HI_PDT_PARAM_TYPE_CROP},
        {PDT_NETCTRL_PARAM_TYPE_WDR,                HI_PDT_PARAM_TYPE_WDR},
        {PDT_NETCTRL_PARAM_TYPE_VIDEOMODE,          HI_PDT_PARAM_TYPE_VIDEOMODE},
        {PDT_NETCTRL_PARAM_TYPE_LANGUAGE,           HI_PDT_PARAM_TYPE_LANGUAGE},
        {PDT_NETCTRL_PARAM_TYPE_SPLITTIME,          HI_PDT_PARAM_TYPE_SPLITTIME},
        {PDT_NETCTRL_PARAM_TYPE_PREVIEWPIP,         HI_PDT_PARAM_TYPE_PREVIEWPIP},
        {PDT_NETCTRL_PARAM_TYPE_REC_TYPE,           HI_PDT_PARAM_RECORD_TYPE},
        {PDT_NETCTRL_PARAM_TYPE_LAPSE_INTERVAL,     HI_PDT_PARAM_TYPE_LAPSE_INTERVAL},
        {PDT_NETCTRL_PARAM_TYPE_POWERON_WORKMODE,   HI_PDT_PARAM_TYPE_POWERON_WORKMODE},

#ifdef CONFIG_MOTIONDETECT_ON
        {PDT_NETCTRL_PARAM_TYPE_MD_SENSITIVITY,     HI_PDT_PARAM_MD_SENSITIVITY},
#endif
    };


    HI_S32 s32ArryaSize = NETCTRL_ARRAY_SIZE(astParamTypeValueSet);
    for (i=0; i<s32ArryaSize; i++)
    {
        if (astParamTypeValueSet[i].s32Value == s32ParamType)
        {
            snprintf(pDesc,u32SiZe,"%s",astParamTypeValueSet[i].szDesc);
            break;
        }
    }

    if (i >= s32ArryaSize)
    {
        MLOGE("the paramtype:%d not support!\n",s32ParamType);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_GetDescFromValueSet(HI_S32 s32Value, const HI_PDT_ITEM_VALUESET_S * pstValueSet,
    HI_CHAR* pDesc, HI_U32 u32Size)
{
    HI_U32 i = 0;
    HI_APPCOMM_CHECK_POINTER(pstValueSet, HI_PDT_NETCTRL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pDesc,       HI_PDT_NETCTRL_EINVAL);

    for (i=0; i<pstValueSet->s32Cnt && i<HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
    {
        if (s32Value == pstValueSet->astValues[i].s32Value)
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
    /*HI_PDT_GSENSOR_CFG_S stGSensorCfg = {};*/

    HI_APPCOMM_CHECK_POINTER(pDesc, HI_PDT_NETCTRL_EINVAL);

    HI_S32 s32PreViewCamID = -1;
    s32PreViewCamID = HI_PDT_STATEMNG_GetPreViewCamID();

    switch (enType)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
        case HI_PDT_PARAM_TYPE_WDR:
        case HI_PDT_PARAM_TYPE_VIDEOMODE:
        case HI_PDT_PARAM_TYPE_FLIP:
        case HI_PDT_PARAM_TYPE_MIRROR:
        case HI_PDT_PARAM_TYPE_LDC:
        case HI_PDT_PARAM_RECORD_TYPE:
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
        case HI_PDT_PARAM_TYPE_SPLITTIME:
#ifdef CONFIG_MOTIONDETECT_ON
        case HI_PDT_PARAM_MD_SENSITIVITY:
            if(HI_PDT_PARAM_MD_SENSITIVITY == enType)
            {
                s32PreViewCamID = Front_CAM_ID;
            }
#endif
            s32Ret = HI_PDT_PARAM_GetCamParam(enWorkMode, s32PreViewCamID, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_PDT_PARAM_GetCamParam");

            s32Ret = HI_PDT_PARAM_GetCamItemValues(enWorkMode, s32PreViewCamID, enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_PDT_PARAM_GetCamItemValues");

            s32Ret = PDT_NETCTRL_GetDescFromValueSet(s32Value, &stValueSet, pDesc, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetDescFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_CROP:
        case HI_PDT_PARAM_TYPE_OSD:
        case HI_PDT_PARAM_TYPE_REC:
            if(HI_PDT_PARAM_TYPE_REC == enType)
            {
                s32PreViewCamID = BACK_CAM_ID;
            }
            s32Ret = HI_PDT_PARAM_GetCamParam(enWorkMode, s32PreViewCamID, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_PDT_PARAM_GetCamParam");
            snprintf(pDesc,u32Size,"%d",s32Value);
            break;

        case HI_PDT_PARAM_TYPE_AUDIO:
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
        case HI_PDT_PARAM_TYPE_KEYTONE:
        case HI_PDT_PARAM_TYPE_BOOTSOUND:
        case HI_PDT_PARAM_TYPE_PREVIEWPIP:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");
            snprintf(pDesc, u32Size, "%d", s32Value);
            break;

        case HI_PDT_PARAM_TYPE_VOLUME:
        case HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY:
        case HI_PDT_PARAM_TYPE_GSENSOR_PARKING:
        case HI_PDT_PARAM_TYPE_LANGUAGE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");

            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommItemValues");

            s32Ret = PDT_NETCTRL_GetDescFromValueSet(s32Value, &stValueSet, pDesc, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetDescFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
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
    HI_APPCOMM_CHECK_POINTER(pDesc,       HI_PDT_NETCTRL_EINVAL);

    for (i=0; i<pstValueSet->s32Cnt && i<HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
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

    HI_S32 s32PreViewCamID = -1;
    s32PreViewCamID = HI_PDT_STATEMNG_GetPreViewCamID();

    switch (enType)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
        case HI_PDT_PARAM_TYPE_WDR:
        case HI_PDT_PARAM_TYPE_LDC:
        case HI_PDT_PARAM_TYPE_FLIP:
        case HI_PDT_PARAM_TYPE_MIRROR:
        case HI_PDT_PARAM_MD_SENSITIVITY:
        case HI_PDT_PARAM_RECORD_TYPE:
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
        case HI_PDT_PARAM_TYPE_SPLITTIME:
        case HI_PDT_PARAM_TYPE_VIDEOMODE:
            s32Ret = HI_PDT_PARAM_GetCamItemValues(enWorkMode, s32PreViewCamID, enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_PDT_PARAM_GetCamItemValues");

            s32Ret = PDT_NETCTRL_GetValueFromValueSet(ps32Value, &stValueSet, pDesc);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetValueFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_OSD:
        case HI_PDT_PARAM_TYPE_AUDIO:
        case HI_PDT_PARAM_TYPE_KEYTONE:
        case HI_PDT_PARAM_TYPE_BOOTSOUND:
        case HI_PDT_PARAM_TYPE_REC:
        case HI_PDT_PARAM_TYPE_PREVIEWPIP:
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

        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            *ps32Value = atoi(pDesc);
            break;

        case HI_PDT_PARAM_TYPE_VOLUME:
        case HI_PDT_PARAM_TYPE_GSENSOR_PARKING:
        case HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY:
            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommItemValues");

            s32Ret = PDT_NETCTRL_GetValueFromValueSet(ps32Value, &stValueSet, pDesc);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetValueFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommItemValues");

            s32Ret = PDT_NETCTRL_GetValueFromValueSet(ps32Value, &stValueSet, pDesc);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_NETCTRL_GetValueFromValueSet");
            break;

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

    switch (enType)
    {
        case HI_PDT_PARAM_TYPE_WIFI_AP:
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
        case HI_PDT_PARAM_TYPE_AUDIO:
        case HI_PDT_PARAM_TYPE_KEYTONE:
        case HI_PDT_PARAM_TYPE_BOOTSOUND:
        case HI_PDT_PARAM_TYPE_GSENSOR_PARKING:
            s32Ret = HI_PDT_PARAM_SetCommParam(enType, pvParam);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        case HI_PDT_PARAM_TYPE_VOLUME:
            {
                HI_PDT_PARAM_MEDIA_COMM_CFG_S stMediaCommCfg = {};
                s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stMediaCommCfg);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                s32Ret = HI_PDT_MEDIA_SetAoVolume(
                    stMediaCommCfg.stAudioOutCfg.astAoCfg[0].AoHdl, *(HI_S32*)pvParam);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                s32Ret = HI_PDT_PARAM_SetCommParam(enType, pvParam);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
            break;

        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            if (*(HI_S32*)pvParam > 0)
            {
                stTimedTaskAttr.bEnable = HI_TRUE;
            }
            else
            {
                stTimedTaskAttr.bEnable = HI_FALSE;
            }
            stTimedTaskAttr.u32Time_sec = *(HI_S32*)pvParam;
            s32Ret = HI_PDT_PARAM_SetCommParam(enType, &stTimedTaskAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#ifndef NONESCREEN
            HI_POWERCTRL_TASK_E enPwrCtrlType = HI_PWRCTRL_TASK_SCREENDORMANT;
            s32Ret = HI_POWERCTRL_SetTaskAttr(enPwrCtrlType, &stTimedTaskAttr);
            if(s32Ret)
            {
                MLOGE("SetTaskAttr failed. PwrCtrlType:%d, value:%d\n", enPwrCtrlType, stTimedTaskAttr.u32Time_sec);
            }
#endif
            break;

        case HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY:
            {
                s32Ret = HI_GSENSORMNG_SetSensitity(*(HI_GSENSORMNG_SENSITITY_E*)pvParam);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                s32Ret = HI_PDT_PARAM_SetCommParam(enType, pvParam);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
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


/** get file dir  value by file dir description */
HI_S32 PDT_NETCTRL_GetFileDirValueByDesc(HI_CHAR* pDesc, HI_DTCF_DIR_E* penFileDir)
{
    HI_APPCOMM_CHECK_EXPR(pDesc, HI_PDT_NETCTRL_EINVAL);
    HI_APPCOMM_CHECK_EXPR(penFileDir, HI_PDT_NETCTRL_EINVAL);

    HI_S32 i;
    HI_S32 s32Ret;

    HI_PDT_FILEMNG_CFG_S stCfg = {};
    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    for(i = 0; i < DTCF_DIR_BUTT; i++)
    {
        if (0 == strncasecmp(stCfg.stDtcfCfg.aszDirNames[i], pDesc, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN))
        {
            *penFileDir = i; /*  i means HI_DTCF_DIR_E */
            return HI_SUCCESS;
        }
    }

    return HI_FAILURE;
}

HI_S32 PDT_NETCTRL_GetSdPromptInfo(HI_CHAR* description, HI_S32 descriptionSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_EVENT_S stEvent = {};
    HI_STORAGE_STATE_E enState = HI_STORAGE_STATE_IDEL;
    HI_STORAGEMNG_CFG_S stStorageCfg = {};
    HI_STORAGE_DEV_INFO_S stDevInfo = {0};
    s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_PARAM_GetStorageCfg");
    s32Ret = HI_STORAGEMNG_GetState(stStorageCfg.szMntPath, &enState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_STORAGEMNG_GetState");

    switch (enState)
    {
        case HI_STORAGE_STATE_MOUNTED:
        {
            HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {0};
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            if (HI_TRUE == stWorkModeState.bSDAvailable)
            {
                memcpy(description, "SDPROMPT_AVAILABLE", descriptionSize);
                return HI_SUCCESS; /**statemng file scan ok ,and also speed normal ,fragement normal*/
            }

            s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STATEMNG_SD_UNAVAILABLE, &stEvent);

            if (s32Ret == HI_SUCCESS)
            {
                MLOGE("SD file scan error\n");
                memcpy(description, "SDPROMPT_NEED_FORMAT", descriptionSize); /** statemng file scan fail */
                return HI_SUCCESS;
            }
            else
            {
                /* check SD fragemnet seriously */
                s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_MOUNTED, &stEvent);

                if (s32Ret == HI_SUCCESS)
                {
                    if (stEvent.arg1 == HI_STORAGE_FSTOOL_ERR_FRAGMENT_SEVERELY)
                    {
                        memcpy(description, "SDPROMPT_NEED_FORMAT", descriptionSize);/**statemng file scan ok,but fragemnet seriously */
                        MLOGE("storage mount event arrived,and fragment servious\n");
                        return HI_SUCCESS;
                    }
                }
                else
                {
                    memcpy(description, "SDPROMPT_NEED_FORMAT", descriptionSize);
                    return HI_SUCCESS;
                }

                /* check SD speed */
                s32Ret = HI_STORAGEMNG_GetDevInfo(stStorageCfg.szMntPath, &stDevInfo);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "get dev info failed");

                if ((NULL == strstr(stDevInfo.aszDevType, "MMC"))
                    && (HI_STORAGE_TRANSMISSION_SPEED_1_4M == stDevInfo.enTranSpeed
                        || HI_STORAGE_TRANSMISSION_SPEED_4_10M == stDevInfo.enTranSpeed))
                {
                    memcpy(description, "SDPROMPT_SPEED_LOW", descriptionSize);/**statemng file scan ok,but speed slow */
                    return HI_SUCCESS;
                }

                /* not PowerAction  */
                s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_DEV_ERROR, &stEvent);

                if (s32Ret != HI_SUCCESS)
                {
                    memcpy(description, "SDPROMPT_AVAILABLE", descriptionSize);
                    return HI_SUCCESS;
                }

                if (stEvent.arg1 > 3)
                {
                    memcpy(description, "SDPROMPT_DAMAGED_CHANGE", descriptionSize);
                    return HI_SUCCESS;
                }

            }
        }

        case HI_STORAGE_STATE_FS_CHECK_FAILED:
            s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_FS_CHECK_FAILED, &stEvent);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_EVTHUB_GetEventHistory");

            switch (stEvent.arg1)
            {
                case HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION:
                case HI_STORAGE_FSTOOL_ERR_NOT_ENOUGH_MEMORY:
                case HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT:
                {
                    memcpy(description, "SDPROMPT_NEED_FORMAT", descriptionSize);
                    return HI_SUCCESS;
                }

                case HI_STORAGE_FSTOOL_ERR_OPEN_FAIL:
                case HI_STORAGE_FSTOOL_ERR_READ_FAIL:
                case HI_STORAGE_FSTOOL_ERR_WRITE_FAIL:
                case HI_STORAGE_FSTOOL_ERR_ILLEGAL_PARAM:
                case HI_STORAGE_FSTOOL_ERR_SYSTEM_CALL_FAILURE:
                {
                    memcpy(description, "SDPROMPT_DAMAGED_CHANGE", descriptionSize);
                    return HI_SUCCESS;
                }

                default:
                    memcpy(description, "SDPROMPT_BUTT", descriptionSize);
                    return HI_SUCCESS;
            }

        case HI_STORAGE_STATE_FS_EXCEPTION:
            s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_FS_EXCEPTION, &stEvent);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_EVTHUB_GetEventHistory");

            switch (stEvent.arg1)
            {
                case HI_STORAGE_FSTOOL_ERR_FS_EXCEPTION:
                case HI_STORAGE_FSTOOL_ERR_FS_NOT_64K_CLUSTER:
                    memcpy(description, "SDPROMPT_NEED_FORMAT", descriptionSize);
                    return HI_SUCCESS;

                default:
                    memcpy(description, "SDPROMPT_BUTT", descriptionSize);
                    return HI_SUCCESS;
            }

        case HI_STORAGE_STATE_MOUNT_FAILED:

            memcpy(description, "SDPROMPT_DAMAGED_CHANGE", descriptionSize);
            return HI_SUCCESS;

        case HI_STORAGE_STATE_DEV_CONNECTING:
        case HI_STORAGE_STATE_FS_CHECKING:
            memcpy(description, "SDPROMPT_PREPARING", descriptionSize);
            return HI_SUCCESS;

        case HI_STORAGE_STATE_DEV_UNPLUGGED:
        default:
            memcpy(description, "SDPROMPT_OUT", descriptionSize);
            return HI_SUCCESS;
    }

    memcpy(description, "SDPROMPT_BUTT", descriptionSize);
    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

