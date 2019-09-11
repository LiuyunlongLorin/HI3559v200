/**
 * @file      hi_product_param_debug.c
 * @brief     param debug interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/15
 * @version   1.0

 */

#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "hi_product_param.h"
#include "hi_product_param_debug.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/** Param Debug Pointer Check */
#define PDT_PARAM_CHECK_POINTER(p) \
    do { if (!(p)) { \
            MLOGE("null pointer\n");\
            return; \
        } }while(0)

const HI_CHAR* PDT_PARAM_GetWorkModeStr(HI_PDT_WORKMODE_E enWorkMode)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            return "NORM_REC";
        case HI_PDT_WORKMODE_PHOTO:
            return "PHOTO";
        case HI_PDT_WORKMODE_PLAYBACK:
            return "PLAYBACK";
        case HI_PDT_WORKMODE_UVC:
            return "UVC";
        case HI_PDT_WORKMODE_USB_STORAGE:
            return "USB_STORAGE";
        case HI_PDT_WORKMODE_PARKING_REC:
            return "PARKING_REC";
        default:
            return "Unknown";
    }
}

const HI_CHAR* PDT_PARAM_GetMediaModeStr(HI_PDT_MEDIAMODE_E enMediaMode)
{
    switch(enMediaMode)
    {
        case HI_PDT_MEDIAMODE_1080P_30:
            return "1080P_30";
        case HI_PDT_MEDIAMODE_1080P_25:
            return "1080P_25";
        case HI_PDT_MEDIAMODE_1296P_30:
            return "1296P_30";
        case HI_PDT_MEDIAMODE_1440P_30:
            return "1440P_30";
        case HI_PDT_MEDIAMODE_1520P_30:
            return "1520P_30";
        case HI_PDT_MEDIAMODE_1600P_30:
            return "1600P_30";
        case HI_PDT_MEDIAMODE_1944P_30:
            return "1944P_30";
        case HI_PDT_MEDIAMODE_2160P_30:
            return "2160P_30";
        case HI_PDT_MEDIAMODE_PHOTO_720P:
            return "720P";
        case HI_PDT_MEDIAMODE_PHOTO_1080P:
            return "1080P";
        case HI_PDT_MEDIAMODE_PHOTO_1296P:
            return "1296P";
        case HI_PDT_MEDIAMODE_PHOTO_1440P:
            return "1440P";
        case HI_PDT_MEDIAMODE_PHOTO_1600P:
            return "1600P";
        case HI_PDT_MEDIAMODE_PHOTO_1520P:
            return "1520P";
        case HI_PDT_MEDIAMODE_PHOTO_1944P:
            return "1944P";
        case HI_PDT_MEDIAMODE_PHOTO_2160P:
             return "2160P";
        default:
            return "Unknown";
    }
}
const HI_CHAR* PDT_PARAM_GetParamTypeStr(HI_PDT_PARAM_TYPE_E enType)
{
    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            return "MediaMode";
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            return "EncPayloadType";
        case HI_PDT_PARAM_TYPE_FLIP:
            return "Flip";
        case HI_PDT_PARAM_TYPE_MIRROR:
            return "Mirror";
        case HI_PDT_PARAM_TYPE_LDC:
            return "LDC";
        case HI_PDT_PARAM_TYPE_WDR:
            return "WDR";
        case HI_PDT_PARAM_TYPE_REC:
            return "Record";
        case HI_PDT_PARAM_TYPE_OSD:
            return "OSD";
        case HI_PDT_PARAM_TYPE_AUDIO:
            return "Audio";
        case HI_PDT_PARAM_TYPE_PREVIEW_CAMID:
            return "PREVIEW_CAMID";
        case HI_PDT_PARAM_TYPE_WIFI_AP:
            return "WiFiAp";
        case HI_PDT_PARAM_TYPE_DEV_INFO:
            return "DevInfo";
        case HI_PDT_PARAM_TYPE_VOLUME:
            return "Volume";
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            return "ScreenBrightness";
        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            return "ScreenDormantMode";
        case HI_PDT_PARAM_TYPE_KEYTONE:
            return "KEYTONE";
        case HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY:
            return "GSENSOR_SENSITIVITY";
        case HI_PDT_PARAM_TYPE_GSENSOR_PARKING:
            return "GSENSOR_PARKINGLEVEL";
        case HI_PDT_PARAM_TYPE_USB_MODE:
            return "Usb_Mode";
        case HI_PDT_PARAM_TYPE_LANGUAGE:
            return "SystemLanguage";
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            return "LapseInterval";
        case HI_PDT_PARAM_RECORD_TYPE:
            return "RecordType";

        default:
            return "Unknown";
    }
}

#ifdef CFG_PARAM_DEBUG_LOG_ON

HI_VOID PDT_PARAM_DebugFileMngCfg(const HI_PDT_FILEMNG_CFG_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("Common Configure:\n");
    MLOGD("MountPath[%s]\n", pstCfg->stCommCfg.szMntPath);
    MLOGD("FileRepair: enable[%d] parseExtraMdat[%d] RootDir[%s] TopDirName[%s]\n",
        pstCfg->stCommCfg.stRepairCfg.bEnable, pstCfg->stCommCfg.stRepairCfg.bParseExtraMdat,
        pstCfg->stCommCfg.stRepairCfg.szRootPath, pstCfg->stCommCfg.stRepairCfg.szTopDirName);


    MLOGD("DTCF Configure:\n");
    MLOGD("PreAllocCnt[%u] PreAllocUnit[%uKB] SharePercent[%d] WarningStage[%u] GuaranteedStage[%u] RootDir[%s] \n",
        pstCfg->stDtcfCfg.u32PreAllocCnt,pstCfg->stDtcfCfg.u32PreAllocUnit,pstCfg->stDtcfCfg.u8SharePercent,pstCfg->stDtcfCfg.u32WarningStage
        ,pstCfg->stDtcfCfg.u32GuaranteedStage,pstCfg->stDtcfCfg.szRootDir);

    for(int i = 0 ;i < DTCF_DIR_BUTT;i++)
    {
        MLOGD("aszDirNames[%d] = %s \n",i,pstCfg->stDtcfCfg.aszDirNames[i]);
    }

    MLOGD("\n\n");
}
#ifdef CONFIG_MOTIONDETECT_ON
HI_VOID PDT_PARAM_DebugVideoDetectCfg(const HI_PDT_PARAM_VIDEODETECT_CFG_S* pstVideoDetectCfg)
{
    HI_S32 j = 0;

    /** debug*/
     MLOGD("CAMID[%d] VideoDetect config: bEnable[%d] algCnt[%d]\nyuvsrc.yuvMode[%d]\n yuvsrc.modHdl[%d]\n yuvsrc.chnl[%d]\n yuvSrc.dumpDepth[%d]\n",
            s32CamID,pstVideoDetectCfg->bEnable, pstVideoDetectCfg->algCnt, pstVideoDetectCfg->yuvSrc.yuvMode, pstVideoDetectCfg->yuvSrc.modHdl, pstVideoDetectCfg->yuvSrc.chnHdl,
            pstVideoDetectCfg->yuvSrc.dumpDepth);

     for(j = 0; j < pstVideoDetectCfg->algCnt; j++)
     {
         MLOGD("algAttr[%d].algType[%d]\n", j, pstVideoDetectCfg->algAttr[j].algType);
         MLOGD("algAttr[%d].enAlgMode[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.enAlgMode);
         MLOGD("algAttr[%d].enSadMode[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.enSadMode);
         MLOGD("algAttr[%d].enSadMode[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.enSadMode);
         MLOGD("algAttr[%d].enSadOutCtrl[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.enSadOutCtrl);
         MLOGD("algAttr[%d].u32Width[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.u32Width);
         MLOGD("algAttr[%d].u32Height[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.u32Height);
         MLOGD("algAttr[%d].u16SadThr[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.u16SadThr);
         MLOGD("algAttr[%d].stCclCtrl.enMode[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.stCclCtrl.enMode);
         MLOGD("algAttr[%d].stCclCtrl.u16InitAreaThr[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.stCclCtrl.u16InitAreaThr);
         MLOGD("algAttr[%d].stCclCtrl.u16Step[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.stCclCtrl.u16Step);
         MLOGD("algAttr[%d].stAddCtrl.stAddCtrl.u0q16X[%d]\n",j,pstVideoDetectCfg->algAttr[j].mdAttr.stAddCtrl.u0q16X);
         MLOGD("algAttr[%d].stAddCtrl.stAddCtrl.u0q16Y[%d]",j,pstVideoDetectCfg->algAttr[j].mdAttr.stAddCtrl.u0q16Y);

         MLOGD("algAttr[%d].mdSensitivity[%d]\n", j, pstVideoDetectCfg->algAttr[j].algProcMdAttr.mdSensitivity);
         MLOGD("algAttr[%d].enAlgMode[%d]\n",j,pstVideoDetectCfg->algAttr[j].algProcMdAttr.mdStableCnt);
     }
}
#endif
HI_VOID PDT_PARAM_DebugKeyMngCfg(const HI_KEYMNG_CFG_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    HI_U32 u32Idx;
    MLOGD("KeyCount[%u]\n", pstCfg->stKeyCfg.u32KeyCnt);
    for (u32Idx = 0; u32Idx < pstCfg->stKeyCfg.u32KeyCnt; ++u32Idx)
    {
        MLOGD("Key[%u], type[%d, 0:click, 1:hold] Id[%d]\n", u32Idx,
            pstCfg->stKeyCfg.astKeyAttr[u32Idx].enType, pstCfg->stKeyCfg.astKeyAttr[u32Idx].s32Id);
        if (HI_KEYMNG_KEY_TYPE_CLICK == pstCfg->stKeyCfg.astKeyAttr[u32Idx].enType)
        {
            MLOGD("      Click Attr: LongKeyEnable[%d], LongClickTime[%ums]\n",
                pstCfg->stKeyCfg.astKeyAttr[u32Idx].unAttr.stClickKeyAttr.bLongClickEnable,
                pstCfg->stKeyCfg.astKeyAttr[u32Idx].unAttr.stClickKeyAttr.u32LongClickTime_msec);
        }
    }
    MLOGD("KeyGrp Enable[%u]\n", pstCfg->stGrpKeyCfg.bEnable);
    for(u32Idx = 0;u32Idx < HI_KEYMNG_KEY_NUM_EACH_GRP; u32Idx++)
    {
        MLOGD("idx[%u] KeyIdx[%u]\n",u32Idx, pstCfg->stGrpKeyCfg.au32GrpKeyIdx[u32Idx]);
    }
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugGsensorCfg(const HI_PDT_GSENSOR_CFG_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("gsensor Configure:\n");
    MLOGD("SampleRate[%u] XDirValue[%d]  YDirValue[%d]  ZDirValue[%d] \n",pstCfg->stAttr.u32SampleRate,pstCfg->stThreshold.s32XDirValue,
        pstCfg->stThreshold.s32YDirValue,pstCfg->stThreshold.s32ZDirValue);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugWiFiInfo(const HI_PDT_WIFI_INFO_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("bEnable[%d]\n", pstCfg->bEnable);
    MLOGD("enMode[%d]\n", pstCfg->enMode);
}

HI_VOID PDT_PARAM_DebugWiFiAPCfg(const HI_HAL_WIFI_APMODE_CFG_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("bHideSSID[%d]\n", pstCfg->bHideSSID);
    MLOGD("SSID[%s], Chn[%d]\n\n", pstCfg->stCfg.szWiFiSSID, pstCfg->s32Channel);
}

HI_VOID PDT_PARAM_DebugDevInfo(const HI_PDT_DEV_INFO_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("system version[%s], software version[%s], model[%s]\n\n",
        pstCfg->szSysVersion, pstCfg->szSoftVersion, pstCfg->szModel);
}

HI_VOID PDT_PARAM_DebugTimedTask(const HI_CHAR* pszName, const HI_TIMEDTASK_ATTR_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pszName);
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("Name[%s] enable[%d] time[%uS]\n\n", pszName, pstCfg->bEnable, pstCfg->u32Time_sec);
}

HI_VOID PDT_PARAM_DebugFileAttr(const HI_RECMNG_FILE_ATTR_S *pstCfg)
{
    MLOGD("PlayFps[%d] MuxerType[%d,0-MP4,1-MOV,2-TS] PreAllocUnit[%u] VBufSize[%u] ",
        pstCfg->s32PlayFps,pstCfg->stMuxerCfg.enMuxerType,pstCfg->stMuxerCfg.u32PreAllocUnit,pstCfg->stMuxerCfg.u32VBufSize);
    switch(pstCfg->stMuxerCfg.enMuxerType)
    {
        case HI_RECMNG_MUXER_MP4:
            MLOGD("RepairUnit[%u] ",pstCfg->stMuxerCfg.unMuxerCfg.stMp4Cfg.u32RepairUnit);
            break;
        case HI_RECMNG_MUXER_MOV:
            MLOGD("RepairUnit[%u] ",pstCfg->stMuxerCfg.unMuxerCfg.stMovCfg.u32RepairUnit);
            break;
        case HI_RECMNG_MUXER_TS:
            break;
        default:
            MLOGE("unsupport muxer type[%d] \n",pstCfg->stMuxerCfg.enMuxerType);
            return;
    }

    HI_S32 i = 0;
    MLOGD("VencCnt[%u] \n",pstCfg->stDataSource.u32VencCnt);
    for(i = 0;i < pstCfg->stDataSource.u32VencCnt && i < HI_RECMNG_FILE_MAX_VIDEO_TRACK_CNT;i++)
    {
        MLOGD("VencHdl%d[%u] \n",i,pstCfg->stDataSource.aVencHdl[i]);
    }

    MLOGD("AencCnt[%u] \n",pstCfg->stDataSource.u32AencCnt);
    for(i = 0;i < pstCfg->stDataSource.u32AencCnt&& i < HI_RECMNG_FILE_MAX_AUDIO_TRACK_CNT;i++)
    {
        MLOGD("AencCnt%d[%u] \n",i,pstCfg->stDataSource.aAencHdl[i]);
    }
    MLOGD("ThmHdl[%d] \n",pstCfg->stDataSource.ThmHdl);

}

HI_VOID PDT_PARAM_DebugNormRecCfg(const HI_PDT_NORM_REC_ATTR_S *pstCfg)
{
    HI_S32 s32Idx = 0,j = 0;

    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("Normal Record Configure:\n");
    for(s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        MLOGD("Photo source[%d]: Enable[%d] VcapPipeHdl[%d] VcapPipeChnHdl[%d] VpssHdl[%d] VportHdl[%d] VencHdl[%d] \n",
            s32Idx,pstCfg->stPhotoSrc[s32Idx].bEnable,pstCfg->stPhotoSrc[s32Idx].VcapPipeHdl,pstCfg->stPhotoSrc[s32Idx].VcapPipeChnHdl,
            pstCfg->stPhotoSrc[s32Idx].VpssHdl,pstCfg->stPhotoSrc[s32Idx].VportHdl,pstCfg->stPhotoSrc[s32Idx].VencHdl);
    }

    for(s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT;s32Idx++)
    {
        MLOGD("Enable[%d],SplitType[%d],SplitTimeLenSec[%u] \n",
            pstCfg->astRecMngAttr[s32Idx].bEnable,pstCfg->astRecMngAttr[s32Idx].stSplitAttr.enSplitType,
            pstCfg->astRecMngAttr[s32Idx].stSplitAttr.u32SplitTimeLenSec);
        MLOGD("RecThmType[%d] BufferTimeSec[%d] PreRecTimeSec[%d] \n",
            pstCfg->astRecMngAttr[s32Idx].enRecThmType,pstCfg->astRecMngAttr[s32Idx].u32BufferTime_ms,
            pstCfg->astRecMngAttr[s32Idx].u32PreRecTimeSec);

        MLOGD("Thm Attr: ThmCnt[%d] \n",pstCfg->astRecMngAttr[s32Idx].stThmAttr.u32ThmCnt);
        for(j = 0;j < pstCfg->astRecMngAttr[s32Idx].stThmAttr.u32ThmCnt && j < HI_RECMNG_TASK_MAX_FILE_CNT;j++)
        {
            MLOGD("ThmHdl%d[%d] \n",j,pstCfg->astRecMngAttr[s32Idx].stThmAttr.aThmHdl[j]);
        }

        MLOGD("FileCnt[%d] \n",pstCfg->astRecMngAttr[s32Idx].u32FileCnt);
        for(j = 0;j < pstCfg->astRecMngAttr[s32Idx].u32FileCnt && j < HI_PDT_RECTASK_FILE_MAX_CNT;j++)
        {
            MLOGD("rec File attr: idx[%d] \n",j);
            PDT_PARAM_DebugFileAttr(&pstCfg->astRecMngAttr[s32Idx].astFileAttr[j]);
        }
    }
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugPhotoCfg(const HI_PDT_PARAM_PHOTO_CFG_S *pstCfg)
{
    HI_S32 s32Idx = 0;

    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("Photo Configure:\n");
    for(s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        MLOGD("Photo source[%d]: Enable[%d] VcapPipeHdl[%d] VcapPipeChnHdl[%d] VpssHdl[%d] VportHdl[%d] VencHdl[%d] \n",
            s32Idx,pstCfg->stPhotoSrc[s32Idx].bEnable,pstCfg->stPhotoSrc[s32Idx].VcapPipeHdl,pstCfg->stPhotoSrc[s32Idx].VcapPipeChnHdl,
            pstCfg->stPhotoSrc[s32Idx].VpssHdl,pstCfg->stPhotoSrc[s32Idx].VportHdl,pstCfg->stPhotoSrc[s32Idx].VencHdl);
    }

    MLOGD("\n\n");
}


static HI_VOID PDT_PARAM_DebugVBCfg(const HI_PDT_MEDIA_VB_CFG_S *pstCfg)
{
    HI_U32 u32Idx;
    MLOGD("VB PoolCnt[%u]\n", pstCfg->u32MaxPoolCnt);
    for (u32Idx = 0; u32Idx < pstCfg->u32MaxPoolCnt; ++u32Idx)
    {
        MLOGD("VB Pool[%u] BlkSize[%u] BlkCnt[%u]\n", u32Idx,
            pstCfg->astCommPool[u32Idx].u32BlkSize, pstCfg->astCommPool[u32Idx].u32BlkCnt);
    }
}

HI_VOID PDT_PARAM_DebugPlayBackCfg(const HI_PDT_PARAM_PLAYBACK_CFG_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("playback Configure:\n");
    HI_U32 i;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        MLOGD("s32CamID[%d] MediaMode[%s]\n",pstCfg->astMediaModeCfg[i].s32CamID,
            PDT_PARAM_GetMediaModeStr(pstCfg->astMediaModeCfg[i].enMediaMode));
    }
    MLOGD("VoutOpt Type[%d:0-vpss,1-vo] ModHdl[%d] ChnHdl[%d]\n",
        pstCfg->stVoutOpt.enVoutType, pstCfg->stVoutOpt.hModHdl, pstCfg->stVoutOpt.hChnHdl);
    MLOGD("AoutOpt AudTrackHdl[%d]\n", pstCfg->stAoutOpt.hAudTrackHdl);
}

static HI_VOID PDT_PARAM_DebugUVCCap(const HI_UVC_FORMAT_CAP_S *pstCap)
{
    HI_U32 u32Idx;
    MLOGD("ModeCnt[%u]\n", pstCap->u32Cnt);
    for (u32Idx = 0; u32Idx < pstCap->u32Cnt; ++u32Idx)
    {
        MLOGD("Mode[%u] Enum[%d]\n", u32Idx, pstCap->astModes[u32Idx].enMode);
    }
    MLOGD("Default ModeEnum[%d]\n", pstCap->enDefMode);
}

HI_VOID PDT_PARAM_DebugUVCCfg(const HI_PDT_PARAM_UVC_CFG_S *pstCfg)
{
    MLOGD("DevPath[%s]\n", pstCfg->stUvcCfg.szDevPath);
    MLOGD("DataSource: hVprocHdl[%d] hVportHdl[%d] hVencHdl[%d] hAcapHdl[%d]\n",
        pstCfg->stUvcCfg.stDataSource.VprocHdl, pstCfg->stUvcCfg.stDataSource.VportHdl,
        pstCfg->stUvcCfg.stDataSource.VencHdl, pstCfg->stUvcCfg.stDataSource.AcapHdl);
    MLOGD("YUV420 Cap:\n");
    PDT_PARAM_DebugUVCCap(&pstCfg->stUvcCfg.stDevCap.astFmtCaps[HI_UVC_STREAM_FORMAT_YUV420]);
    MLOGD("MJPEG Cap:\n");
    PDT_PARAM_DebugUVCCap(&pstCfg->stUvcCfg.stDevCap.astFmtCaps[HI_UVC_STREAM_FORMAT_MJPEG]);
    MLOGD("H264 Cap:\n");
    PDT_PARAM_DebugUVCCap(&pstCfg->stUvcCfg.stDevCap.astFmtCaps[HI_UVC_STREAM_FORMAT_H264]);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugUSBStorageCfg(const HI_USB_STORAGE_CFG_S *pstCfg)
{
    MLOGD("DevPath[%s]\n\n", pstCfg->szDevPath);
}

static HI_VOID PDT_PARAM_DebugViVpssMode(const HI_MEDIA_VI_VPSS_MODE_S* pstCfg)
{
    HI_S32 s32VcapPipeIdx;
    for (s32VcapPipeIdx = 0; s32VcapPipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32VcapPipeIdx)
    {
        MLOGD("Pipe[%d] PipeHdl[%d] ViVpssMode[%d:0-vioffline_vpssoffline]\n",
            s32VcapPipeIdx,
            pstCfg[s32VcapPipeIdx].VcapPipeHdl,
            pstCfg[s32VcapPipeIdx].enMode);
    }

}

static HI_VOID PDT_PARAM_DebugVcapDevCfg(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstCfg)
{
    HI_S32 s32PipeIdx, s32PipeChnIdx;
    const HI_PDT_MEDIA_VCAP_PIPE_ATTR_S*     pstPipeAttr = NULL;
    const HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChnAttr = NULL;
    MLOGD("VcapDev Enable[%d] handle[%d]\n", pstCfg->bEnable, pstCfg->VcapDevHdl);
    MLOGD("Sensor: Resolution[%ux%u] WdrMode[%d] SnsMode[%u]\n",
        pstCfg->stSnsAttr.stResolution.u32Width, pstCfg->stSnsAttr.stResolution.u32Height,
        pstCfg->stSnsAttr.enWdrMode,pstCfg->stSnsAttr.u32SnsMode);
    MLOGD("DevAttr: Resolution[%ux%u] WdrMode[%d]\n",
        pstCfg->stResolution.u32Width, pstCfg->stResolution.u32Height, pstCfg->enWdrMode);
    for (s32PipeIdx = 0; s32PipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32PipeIdx)
    {
        pstPipeAttr = &pstCfg->astVcapPipeAttr[s32PipeIdx];
        MLOGD("  Pipe[%d] Enable[%d] PipeHdl[%d] PipeType[%d:0:video,1:snap] IspBypass[%d]\n",
            s32PipeIdx, pstPipeAttr->bEnable, pstPipeAttr->VcapPipeHdl, pstPipeAttr->enPipeType,
            pstPipeAttr->bIspBypass);
        MLOGD("    SrcFrmRate[%dfps] DstFrmRate[%dfps]\n",
            pstPipeAttr->stFrameRate.s32SrcFrameRate, pstPipeAttr->stFrameRate.s32DstFrameRate);
        MLOGD("  ISPAttr: Resolution[%ux%u] FrameRate[%ffps]\n",
            pstPipeAttr->stIspPubAttr.stSize.u32Width,
            pstPipeAttr->stIspPubAttr.stSize.u32Height, pstPipeAttr->stIspPubAttr.f32FrameRate);
        for (s32PipeChnIdx = 0; s32PipeChnIdx < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++s32PipeChnIdx)
        {
            pstPipeChnAttr = &pstPipeAttr->astPipeChnAttr[s32PipeChnIdx];
            MLOGD("    PiPeChn[%d] Enable[%d] PipeChnHdl[%d] DstResolution[%ux%u]\n",
                s32PipeChnIdx, pstPipeChnAttr->bEnable, pstPipeChnAttr->PipeChnHdl,
                pstPipeChnAttr->stDestResolution.u32Width, pstPipeChnAttr->stDestResolution.u32Height);
            MLOGD("      SrcFrmRate[%dfps] DstFrmRate[%dfps] PixedFmt[%d:0-420,1-422]\n",
                pstPipeChnAttr->stFrameRate.s32SrcFrameRate, pstPipeChnAttr->stFrameRate.s32DstFrameRate,
                pstPipeChnAttr->enPixelFormat);
            MLOGD("      Rotate[%d] Flip[%d] Mirror[%d]\n",
                pstPipeChnAttr->enRotate, pstPipeChnAttr->bFlip, pstPipeChnAttr->bMirror);
            MLOGD("      Brightness[%d] Saturation[%d]\n",
                pstPipeChnAttr->s32Brightness, pstPipeChnAttr->s32Saturation);
        }
    }
}

static HI_VOID PDT_PARAM_DebugVpssAttr(const HI_PDT_MEDIA_VPSS_ATTR_S *pstCfg)
{
    HI_S32 s32VpssPortIdx;
    const HI_PDT_MEDIA_VPSS_PORT_ATTR_S* pstVpssPortAttr = NULL;

    MLOGD("enable[%d] VpssHdl[%d] VcapPipeHdl[%d] VcapPipeChnHdl[%d]\n",
        pstCfg->bEnable, pstCfg->VpssHdl,
        pstCfg->VcapPipeHdl, pstCfg->VcapPipeChnHdl);
    MLOGD("  MaxSize[%ux%u] SrcFrmRate[%dfps] DstFrmRate[%dfps] PixelFmt[%d:0-YUV420]\n",
        pstCfg->stVpssAttr.u32MaxW, pstCfg->stVpssAttr.u32MaxH,
        pstCfg->stVpssAttr.stFrameRate.s32SrcFrameRate,
        pstCfg->stVpssAttr.stFrameRate.s32DstFrameRate,
        pstCfg->stVpssAttr.enPixelFormat);
    MLOGD("  bNrEn[%d] CompressMode[%d] NrMotionMode[%d]\n",
        pstCfg->stVpssAttr.bNrEn, pstCfg->stVpssAttr.stNrAttr.enCompressMode,
        pstCfg->stVpssAttr.stNrAttr.enNrMotionMode);
    for (s32VpssPortIdx = 0; s32VpssPortIdx < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++s32VpssPortIdx)
    {
        pstVpssPortAttr = &pstCfg->astVportAttr[s32VpssPortIdx];
        MLOGD("    Port[%d] enable[%d] VpssPortHdl[%d] Resoution[%ux%u] SrcFrmRate[%dfps] DstFrmRate[%dfps]\n",
            s32VpssPortIdx, pstVpssPortAttr->bEnable, pstVpssPortAttr->VportHdl,
            pstVpssPortAttr->stResolution.u32Width, pstVpssPortAttr->stResolution.u32Height,
            pstVpssPortAttr->stFrameRate.s32SrcFrameRate, pstVpssPortAttr->stFrameRate.s32DstFrameRate);
        MLOGD("      Rotate[%d] Flip[%d] Mirror[%d]\n",
            pstVpssPortAttr->enRotate, pstVpssPortAttr->bFlip, pstVpssPortAttr->bMirror);
        MLOGD("      VideoFmt[%d] PixedFmt[%d]\n",
            pstVpssPortAttr->enVideoFormat, pstVpssPortAttr->enPixelFormat);
    }

}

static HI_VOID PDT_PARAM_DebugVencAttr(const HI_MEDIA_VENC_ATTR_S *pstAttr)
{
    MLOGD("PayloadType[%d:0-h264,1-h265,2-mjpeg,3-jpeg] Resolution[%ux%u] BufSize[%u]\n",
        pstAttr->stTypeAttr.enType, pstAttr->stTypeAttr.u32Width,
        pstAttr->stTypeAttr.u32Height, pstAttr->stTypeAttr.u32BufSize);
    MLOGD("  SceneMode[%d:0-norm,1-dv,2-car] rcMode[%d:0-cbr,1-vbr]\n",
        pstAttr->stTypeAttr.enSceneMode, pstAttr->stRcAttr.enRcMode);
    if (HI_MAPI_PAYLOAD_TYPE_H264 == pstAttr->stTypeAttr.enType)
    {
        MLOGD("H264: Profile[%u:0-baseline,1-main,2-high]\n", pstAttr->stTypeAttr.u32Profile);
        if (HI_MAPI_VENC_RC_MODE_CBR == pstAttr->stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H264_CBR_S* pstH264Cbr = &pstAttr->stRcAttr.unAttr.stH264Cbr;
            MLOGD("  CBR: BitRate[%uKbps] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
                pstH264Cbr->stAttr.u32BitRate, pstH264Cbr->stAttr.u32SrcFrameRate,
                pstH264Cbr->stAttr.fr32DstFrameRate, pstH264Cbr->stAttr.u32Gop,
                pstH264Cbr->stAttr.u32StatTime);
            MLOGD("    MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n", pstH264Cbr->u32MaxQp,
                pstH264Cbr->u32MinQp, pstH264Cbr->u32MaxIQp, pstH264Cbr->u32MinIQp);
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == pstAttr->stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H264_VBR_S* pstH264Vbr = &pstAttr->stRcAttr.unAttr.stH264Vbr;
            MLOGD("  VBR: MaxBitRate[%uKbps] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
                pstH264Vbr->stAttr.u32MaxBitRate,
                pstH264Vbr->stAttr.u32SrcFrameRate, pstH264Vbr->stAttr.fr32DstFrameRate,
                pstH264Vbr->stAttr.u32Gop, pstH264Vbr->stAttr.u32StatTime);
            MLOGD("    MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n", pstH264Vbr->u32MaxQp,
                pstH264Vbr->u32MinQp, pstH264Vbr->u32MaxIQp, pstH264Vbr->u32MinIQp);
        }
        else if (HI_MAPI_VENC_RC_MODE_QVBR == pstAttr->stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H264_QVBR_S* pstH264QVbr = &pstAttr->stRcAttr.unAttr.stH264QVbr;
            MLOGI("  QVBR: MaxBitRate[%uKbps] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
                pstH264QVbr->stAttr.u32TargetBitRate,
                pstH264QVbr->stAttr.u32SrcFrameRate, pstH264QVbr->stAttr.fr32DstFrameRate,
                pstH264QVbr->stAttr.u32Gop, pstH264QVbr->stAttr.u32StatTime);
            MLOGI("    MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n", pstH264QVbr->u32MaxQp,
                pstH264QVbr->u32MinQp, pstH264QVbr->u32MaxIQp, pstH264QVbr->u32MinIQp);
            MLOGI("    BitPercentUL[%d] BitPercentLL[%d] PsnrFluctuateUL[%d] PsnrFluctuateLL[%d]\n",
                pstH264QVbr->s32BitPercentUL, pstH264QVbr->s32BitPercentLL,
                pstH264QVbr->s32PsnrFluctuateUL, pstH264QVbr->s32PsnrFluctuateLL);
        }
    }
    else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstAttr->stTypeAttr.enType)
    {
        MLOGD("H265: Profile[%u:0-baseline]\n", pstAttr->stTypeAttr.u32Profile);
        if (HI_MAPI_VENC_RC_MODE_CBR == pstAttr->stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H265_CBR_S* pstH265Cbr = &pstAttr->stRcAttr.unAttr.stH265Cbr;
            MLOGD("  CBR: BitRate[%uKbps] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
                pstH265Cbr->stAttr.u32BitRate, pstH265Cbr->stAttr.u32SrcFrameRate,
                pstH265Cbr->stAttr.fr32DstFrameRate, pstH265Cbr->stAttr.u32Gop,
                pstH265Cbr->stAttr.u32StatTime);
            MLOGD("    MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n", pstH265Cbr->u32MaxQp,
                pstH265Cbr->u32MinQp, pstH265Cbr->u32MaxIQp, pstH265Cbr->u32MinIQp);
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == pstAttr->stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H265_VBR_S* pstH265Vbr = &pstAttr->stRcAttr.unAttr.stH265Vbr;
            MLOGD("  VBR: MaxBitRate[%uKbps] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
                pstH265Vbr->stAttr.u32MaxBitRate,
                pstH265Vbr->stAttr.u32SrcFrameRate, pstH265Vbr->stAttr.fr32DstFrameRate,
                pstH265Vbr->stAttr.u32Gop, pstH265Vbr->stAttr.u32StatTime);
            MLOGD("    MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n", pstH265Vbr->u32MaxQp,
                pstH265Vbr->u32MinQp, pstH265Vbr->u32MaxIQp, pstH265Vbr->u32MinIQp);
        }
        else if (HI_MAPI_VENC_RC_MODE_QVBR == pstAttr->stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H264_QVBR_S* pstH265QVbr = &pstAttr->stRcAttr.unAttr.stH265QVbr;
            MLOGI("  QVBR: MaxBitRate[%uKbps] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
                pstH265QVbr->stAttr.u32TargetBitRate,
                pstH265QVbr->stAttr.u32SrcFrameRate, pstH265QVbr->stAttr.fr32DstFrameRate,
                pstH265QVbr->stAttr.u32Gop, pstH265QVbr->stAttr.u32StatTime);
            MLOGI("    MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n", pstH265QVbr->u32MaxQp,
                pstH265QVbr->u32MinQp, pstH265QVbr->u32MaxIQp, pstH265QVbr->u32MinIQp);
            MLOGI("    BitPercentUL[%d] BitPercentLL[%d] PsnrFluctuateUL[%d] PsnrFluctuateLL[%d]\n",
                pstH265QVbr->s32BitPercentUL, pstH265QVbr->s32BitPercentLL,
                pstH265QVbr->s32PsnrFluctuateUL, pstH265QVbr->s32PsnrFluctuateLL);
        }
    }
    else if (HI_MAPI_PAYLOAD_TYPE_MJPEG == pstAttr->stTypeAttr.enType)
    {
        MLOGD("MJPEG: Profile[%u]\n", pstAttr->stTypeAttr.u32Profile);
        if (HI_MAPI_VENC_RC_MODE_CBR == pstAttr->stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_MJPEG_CBR_S* pstMjpegCbr = &pstAttr->stRcAttr.unAttr.stMjpegCbr;
            MLOGD("  CBR: BitRate[%uKbps] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
                pstMjpegCbr->stAttr.u32BitRate, pstMjpegCbr->stAttr.u32SrcFrameRate,
                pstMjpegCbr->stAttr.fr32DstFrameRate,
                pstMjpegCbr->stAttr.u32Gop, pstMjpegCbr->stAttr.u32StatTime);
            MLOGD("    MaxQfactor[%u] MinQfactor[%u]\n",
                pstMjpegCbr->u32MaxQfactor, pstMjpegCbr->u32MinQfactor);
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == pstAttr->stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_MJPEG_VBR_S* pstMjpegVbr = &pstAttr->stRcAttr.unAttr.stMjpegVbr;
            MLOGD("  VBR: MaxBitRate[%uKbps] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
                pstMjpegVbr->stAttr.u32MaxBitRate, pstMjpegVbr->stAttr.u32SrcFrameRate,
                pstMjpegVbr->stAttr.fr32DstFrameRate, pstMjpegVbr->stAttr.u32Gop,
                pstMjpegVbr->stAttr.u32StatTime);
        }
    }
    else if (HI_MAPI_PAYLOAD_TYPE_JPEG == pstAttr->stTypeAttr.enType)
    {
        HI_U8 u8Idx;
        MLOGD("JPEG: Profile[%u] EncodeMode[%d] EnableDCF[%d] Qfactor[%u]\n",
            pstAttr->stTypeAttr.u32Profile,
            pstAttr->stTypeAttr.stAttrJpege.enJpegEncodeMode,
            pstAttr->stTypeAttr.stAttrJpege.bEnableDCF,
            pstAttr->stTypeAttr.stAttrJpege.u32Qfactor);
        MLOGD("  MPF Num[%u]\n",
            pstAttr->stTypeAttr.stAttrJpege.stAttrMPF.u8LargeThumbNailNum);
        for (u8Idx = 0; u8Idx < pstAttr->stTypeAttr.stAttrJpege.stAttrMPF.u8LargeThumbNailNum; ++u8Idx)
        {
            MLOGD("    [%d] Size[%ux%u]\n", u8Idx,
                pstAttr->stTypeAttr.stAttrJpege.stAttrMPF.astLargeThumbNailSize[u8Idx].u32Width,
                pstAttr->stTypeAttr.stAttrJpege.stAttrMPF.astLargeThumbNailSize[u8Idx].u32Height);
        }
    }
    MLOGD("HierarchicalQp: \n");
    MLOGD("  QPDelta[%d, %d, %d, %d]\n",
        pstAttr->stRcAttr.stHierarchicalQp.s32HierarchicalQpDelta[0],
        pstAttr->stRcAttr.stHierarchicalQp.s32HierarchicalQpDelta[1],
        pstAttr->stRcAttr.stHierarchicalQp.s32HierarchicalQpDelta[2],
        pstAttr->stRcAttr.stHierarchicalQp.s32HierarchicalQpDelta[3]);
    MLOGD("  FrameNum[%d, %d, %d, %d]\n",
        pstAttr->stRcAttr.stHierarchicalQp.s32HierarchicalFrameNum[0],
        pstAttr->stRcAttr.stHierarchicalQp.s32HierarchicalFrameNum[1],
        pstAttr->stRcAttr.stHierarchicalQp.s32HierarchicalFrameNum[2],
        pstAttr->stRcAttr.stHierarchicalQp.s32HierarchicalFrameNum[3]);
}

HI_VOID PDT_PARAM_DebugVencCfg(const HI_PDT_MEDIA_VENC_CFG_S *pstCfg)
{
    MLOGD("Enable[%d] enBindedMod[%d],ModHdl[%d] ChnHdl[%d] Venchdl[%d]\n",
        pstCfg->bEnable, pstCfg->enBindedMod,pstCfg->ModHdl, pstCfg->ChnHdl, pstCfg->VencHdl);
    PDT_PARAM_DebugVencAttr(&pstCfg->stVencAttr);
}

static HI_VOID PDT_PARAM_DebugOsdAttr(const HI_PDT_MEDIA_OSD_ATTR_S* pstAttr)
{
    MLOGD("OSD: BindModule[%d:0-vcap,1-venc]\n", pstAttr->enBindMod);
    MLOGD("  Time: show[%d] format[%d:0-24h] Color[%08x] BgColor[%08x]\n",
        pstAttr->stTimeOsd.bShow, pstAttr->stTimeOsd.enFormat, pstAttr->stTimeOsd.u32Color,
        pstAttr->stTimeOsd.u32BgColor);
    MLOGD("    FontSize[%ux%u], StartPos[%d,%d] u32FgAlpha[%x] u32BgAlpha[%x]\n",
        pstAttr->stTimeOsd.stFontSize.u32Width, pstAttr->stTimeOsd.stFontSize.u32Height,
        pstAttr->stTimeOsd.stStartPos.s32X, pstAttr->stTimeOsd.stStartPos.s32Y,
        pstAttr->stTimeOsd.u32FgAlpha,pstAttr->stTimeOsd.u32BgAlpha);
    MLOGD("  Logo: Show[%u], StartPos[%d,%d] u32FgAlpha[%x] u32BgAlpha[%x]\n",
        pstAttr->stLogoOsd.bShow,
        pstAttr->stLogoOsd.stStartPos.s32X, pstAttr->stLogoOsd.stStartPos.s32Y,
        pstAttr->stLogoOsd.u32FgAlpha,pstAttr->stLogoOsd.u32BgAlpha);
}

static HI_VOID PDT_PARAM_DebugDispCfg(const HI_PDT_PARAM_DISP_CFG_S *pstCfg)
{
    HI_S32 s32WndIdx;
    MLOGD("Display: enable[%d] Hdl[%d] BgColor[%u]\n", pstCfg->bEnable, pstCfg->VdispHdl,pstCfg->u32BgColor);
    MLOGD("  CSC enable[%d]\n", pstCfg->stCscAttr.bEnable);
    MLOGD("    Matrix Type[%d:0-Identity,1-BT601ToBT709,2-BT709ToBT601,3-BT601ToRGB,4-BT709ToRGB,5-RGBToBT601,6-RGBToBT709]\n",
        pstCfg->stCscAttr.stAttrEx.stVoCSC.enCscMatrix);
    MLOGD("    luma[%u], contrast[%u], hue[%u], satuature[%u]\n",
        pstCfg->stCscAttr.stAttrEx.stVoCSC.u32Luma,
        pstCfg->stCscAttr.stAttrEx.stVoCSC.u32Contrast,
        pstCfg->stCscAttr.stAttrEx.stVoCSC.u32Hue,
        pstCfg->stCscAttr.stAttrEx.stVoCSC.u32Satuature);
    for (s32WndIdx = 0; s32WndIdx < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++s32WndIdx)
    {
        MLOGD("  Wnd[%d] Enable[%d] Hdl[%d] enBindedMod[%d],ModHdl[%d] ChnHdl[%d] Rotate[%d:0-0,1-90,2-180,3-270]\n",
            s32WndIdx, pstCfg->astWndCfg[s32WndIdx].bEnable, pstCfg->astWndCfg[s32WndIdx].WndHdl,
            pstCfg->astWndCfg[s32WndIdx].enBindedMod,pstCfg->astWndCfg[s32WndIdx].ModHdl,
             pstCfg->astWndCfg[s32WndIdx].ChnHdl,pstCfg->astWndCfg[s32WndIdx].enRotate);
        MLOGD("    AspectRatio: Mode[%d:0-none,1-auto,2-manual] BgColor[%u] Start[%d,%d] Size[%ux%u]\n",
            pstCfg->astWndCfg[s32WndIdx].stAspectRatio.enMode,
            pstCfg->astWndCfg[s32WndIdx].stAspectRatio.u32BgColor,
            pstCfg->astWndCfg[s32WndIdx].stAspectRatio.stVideoRect.s32X,
            pstCfg->astWndCfg[s32WndIdx].stAspectRatio.stVideoRect.s32Y,
            pstCfg->astWndCfg[s32WndIdx].stAspectRatio.stVideoRect.u32Width,
            pstCfg->astWndCfg[s32WndIdx].stAspectRatio.stVideoRect.u32Height);
        MLOGD("    WndAttr: Start[%d,%d] Size[%ux%u] Priority[%u]\n",
            pstCfg->astWndCfg[s32WndIdx].stWndAttr.stRect.s32X,
            pstCfg->astWndCfg[s32WndIdx].stWndAttr.stRect.s32Y,
            pstCfg->astWndCfg[s32WndIdx].stWndAttr.stRect.u32Width,
            pstCfg->astWndCfg[s32WndIdx].stWndAttr.stRect.u32Height,
            pstCfg->astWndCfg[s32WndIdx].stWndAttr.u32Priority);
    }
}


HI_VOID PDT_PARAM_DebugAudioCfg(HI_PDT_MEDIA_AUDIO_CFG_S *pstCfg)
{
    HI_S32 s32Idx = 0;

    MLOGD("Acap configure \n");
    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_ACAP_MAX_CNT;s32Idx++)
    {
        MLOGD("AcapHdl[%d] Enable[%d] AudioGain[%d] \n",
            pstCfg->astAcapCfg[s32Idx].AcapHdl,pstCfg->astAcapCfg[s32Idx].bEnable,pstCfg->astAcapCfg[s32Idx].s32AudioGain);
        MLOGD("Acap attr: SampleRate[%d] Bitwidth[%d] SoundMode[%d] Workmode[%d] PtNumPerFrm[%u] MixerMicMode[%d] ResampleRate[%d] \n",
            pstCfg->astAcapCfg[s32Idx].stAcapAttr.enSampleRate,pstCfg->astAcapCfg[s32Idx].stAcapAttr.enBitwidth,pstCfg->astAcapCfg[s32Idx].stAcapAttr.enSoundMode,
            pstCfg->astAcapCfg[s32Idx].stAcapAttr.enWorkmode,pstCfg->astAcapCfg[s32Idx].stAcapAttr.u32PtNumPerFrm,
            pstCfg->astAcapCfg[s32Idx].stAcapAttr.enMixerMicMode,pstCfg->astAcapCfg[s32Idx].stAcapAttr.enResampleRate);
    }

    MLOGD("Aenc configure \n");
    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_AENC_MAX_CNT;s32Idx++)
    {
        MLOGD("Enable[%d] AencHdl[%d] AcapHdl[%d] Format[%d] PtNumPerFrm[%u] \n",
        pstCfg->astAencCfg[s32Idx].bEnable,pstCfg->astAencCfg[s32Idx].AencHdl,pstCfg->astAencCfg[s32Idx].AcapHdl,
        pstCfg->astAencCfg[s32Idx].enFormat,pstCfg->astAencCfg[s32Idx].u32PtNumPerFrm);
        if(HI_MAPI_AUDIO_FORMAT_AACLC == pstCfg->astAencCfg[s32Idx].enFormat)
        {
            HI_MAPI_AENC_ATTR_AAC_S *pstAACAttr = &pstCfg->astAencCfg[s32Idx].unAttr.stAACAttr;
            MLOGD("AAC Attr:AACType[%d] BitRate[%d] SmpRate[%d] BitWidth[%d] SoundMode[%d] TransType[%d] BandWidth[%d] \n",
            pstAACAttr->enAACType,pstAACAttr->enBitRate,pstAACAttr->enSmpRate,pstAACAttr->enBitWidth,pstAACAttr->enSoundMode,
            pstAACAttr->enTransType,pstAACAttr->s16BandWidth);
        }
    }

}

static HI_VOID PDT_PARAM_DebugAudioOutCfg(const HI_PDT_MEDIA_AUDIOOUT_CFG_S *pstCfg)
{
    HI_S32 s32Idx = 0;
    MLOGD("Audio out configure \n");
    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_ACAP_MAX_CNT;s32Idx++)
    {
        MLOGD("Enable[%d] AoHdl[%d] Volume[%d] \n",pstCfg->astAoCfg[s32Idx].bEnable,pstCfg->astAoCfg[s32Idx].AoHdl,pstCfg->astAoCfg[s32Idx].s32Volume);
        MLOGD("Audio out Attr: SampleRate[%d] InSampleRate[%d] Bitwidth[%d] SoundMode[%d] Workmode[%d] PtNumPerFrm[%u] \n",
            pstCfg->astAoCfg[s32Idx].stAoAttr.enSampleRate,pstCfg->astAoCfg[s32Idx].stAoAttr.enInSampleRate,pstCfg->astAoCfg[s32Idx].stAoAttr.enBitwidth,
            pstCfg->astAoCfg[s32Idx].stAoAttr.enSoundMode,pstCfg->astAoCfg[s32Idx].stAoAttr.enWorkmode,pstCfg->astAoCfg[s32Idx].stAoAttr.u32PtNumPerFrm);
    }
}

HI_VOID PDT_PARAM_DebugCamMediaCfg(const HI_PDT_PARAM_CAM_MEDIA_CFG_S *pstCfg)
{
    HI_S32 s32Idx;
    PDT_PARAM_DebugViVpssMode(pstCfg->astMode);
    PDT_PARAM_DebugVBCfg(&pstCfg->stVBCfg);

    MLOGD("Vcapture Configure:\n");
    PDT_PARAM_DebugVcapDevCfg(&pstCfg->stVcapDevAttr);

    MLOGD("Vprocessor Configure:\n");
    MLOGD("VpssCnt[%d]\n",pstCfg->u32VpssCnt);
    for (s32Idx = 0;s32Idx < pstCfg->u32VpssCnt && s32Idx < HI_PDT_VCAPDEV_VPSS_MAX_CNT; ++s32Idx)
    {
        MLOGD("VpssIdx[%d]\n",s32Idx);
        PDT_PARAM_DebugVpssAttr(&pstCfg->astVpssCfg[s32Idx]);
    }

    MLOGD("Vencoder Configure:\n");
    MLOGD("VencCnt[%d]\n",pstCfg->u32VencCnt);
    for (s32Idx = 0;s32Idx< pstCfg->u32VencCnt && s32Idx < HI_PDT_VCAPDEV_VENC_MAX_CNT; ++s32Idx)
    {
        MLOGD("Vencoder[%d] Configure:\n", s32Idx);
        PDT_PARAM_DebugVencCfg(&pstCfg->astVencCfg[s32Idx]);
    }
    PDT_PARAM_DebugOsdAttr(&pstCfg->stOsdAttr);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugSceneCamVcapCfg(const HI_PDT_PARAM_SCENE_CAM_VCAP_ATTR_S *pstCfg)
{
    MLOGD("Scene Vcap Attr:\n");
    MLOGD("IspBypass[%d] MainPipeHdl[%d] VcapPipeHdl[%d] PipeChnHdl[%d] PipeParamIndex[%u] PipeType[%d] PipeMode[%d]\n",
        pstCfg->bIspBypass,pstCfg->MainPipeHdl,pstCfg->VcapPipeHdl,pstCfg->PipeChnHdl,pstCfg->u8PipeParamIndex,
        pstCfg->enPipeType,pstCfg->enPipeMode);
}

HI_VOID PDT_PARAM_DEBUGMediaCommCfg(HI_PDT_PARAM_MEDIA_COMM_CFG_S *pstCfg)
{
    HI_S32 s32Idx = 0;
    MLOGD("Disp configures\n");
    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_DISP_MAX_CNT;s32Idx++)
    {
        MLOGD("Disp Index[%d]\n",s32Idx);
        PDT_PARAM_DebugDispCfg(&pstCfg->stVideoOutCfg.astDispCfg[s32Idx]);
    }

    PDT_PARAM_DebugAudioCfg(&pstCfg->stAudioCfg);
    PDT_PARAM_DebugAudioOutCfg(&pstCfg->stAudioOutCfg);

}

HI_VOID PDT_PARAM_DebugItemValues(const HI_PDT_ITEM_VALUESET_S* pstValues)
{
    HI_S32 s32Idx;
    MLOGD("value count[%d]\n", pstValues->s32Cnt);
    for (s32Idx = 0; s32Idx < pstValues->s32Cnt; ++s32Idx)
    {
        MLOGD("[%d] Description[%s] Value[%d]\n", s32Idx,
            pstValues->astValues[s32Idx].szDesc, pstValues->astValues[s32Idx].s32Value);
    }
    MLOGD("\n\n");
}


HI_VOID PDT_PARAM_DebugOSDVideoAttr(HI_PDT_MEDIA_OSD_VIDEO_CFG_S* pstVideoCfg)
{
    HI_S32 i, j, k;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                MLOGD("Vcap[%d] Pipe[%d] Chn[%d] MaxSize[%ux%u]\n", i, j, k,
                    pstVideoCfg->astAttr[i].stVcapDevAttr.astPipeAttr[j].astPipeChnAttr[k].stMaxSize.u32Width,
                    pstVideoCfg->astAttr[i].stVcapDevAttr.astPipeAttr[j].astPipeChnAttr[k].stMaxSize.u32Height);
            }
        }

        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            MLOGD("Vcapdev[%d] Port[%d] MaxSize[%ux%u]\n", i, j,
                    pstVideoCfg->astAttr[i].stVpssAttr.astPortAttr[j].stMaxSize.u32Width,
                    pstVideoCfg->astAttr[i].stVpssAttr.astPortAttr[j].stMaxSize.u32Height);
        }
        for (j = 0; j < HI_PDT_MEDIA_VENC_MAX_CNT; ++j)
        {
            MLOGD("Venc[%d] MaxSize[%ux%u]\n", j,
                    pstVideoCfg->astAttr[i].astVencAttr[j].stMaxSize.u32Width,
                    pstVideoCfg->astAttr[i].astVencAttr[j].stMaxSize.u32Height);
        }
    }

}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

