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
        case HI_PDT_WORKMODE_LOOP_REC:
            return "LOOP_REC";
        case HI_PDT_WORKMODE_LPSE_REC:
            return "LPSE_REC";
        case HI_PDT_WORKMODE_SLOW_REC:
            return "SLOW_REC";
        case HI_PDT_WORKMODE_SING_PHOTO:
            return "SING_PHOTO";
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            return "DLAY_PHOTO";
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            return "LPSE_PHOTO";
        case HI_PDT_WORKMODE_BURST:
            return "BURST";
        case HI_PDT_WORKMODE_RECSNAP:
            return "RECSNAP";
        case HI_PDT_WORKMODE_PLAYBACK:
            return "PLAYBACK";
        case HI_PDT_WORKMODE_UVC:
            return "UVC";
        case HI_PDT_WORKMODE_USB_STORAGE:
            return "USB_STORAGE";
        case HI_PDT_WORKMODE_SUSPEND:
            return "SUSPEND";
        case HI_PDT_WORKMODE_HDMI_PREVIEW:
            return "HDMI_PREVIEW";
        case HI_PDT_WORKMODE_HDMI_PLAYBACK:
            return "HDMI_PLAYBACK";
        default:
            return "Unknown";
    }
}

const HI_CHAR* PDT_PARAM_GetMediaModeStr(HI_PDT_MEDIAMODE_E enMediaMode)
{
    switch(enMediaMode)
    {
        case HI_PDT_MEDIAMODE_PLAYBACK:
            return "PLAYBACK";
        case HI_PDT_MEDIAMODE_HDMIPREVIEW:
            return "HDMIPREVIEW";
        case HI_PDT_MEDIAMODE_HDMIPLAYBACK:
            return "HDMIPLAYBACK";
        case HI_PDT_MEDIAMODE_720P_120_REC:
            return "720P_120_REC";
        case HI_PDT_MEDIAMODE_720P_240_REC:
            return "720P_240_REC";
        case HI_PDT_MEDIAMODE_1080P_30_REC:
            return "1080P_30_REC";
        case HI_PDT_MEDIAMODE_1080P_60_REC:
            return "1080P_60_REC";
        case HI_PDT_MEDIAMODE_1080P_60_RECSNAP:
            return "1080P_60_RECSNAP";
        case HI_PDT_MEDIAMODE_1080P_90_REC:
            return "1080P_90_REC";
        case HI_PDT_MEDIAMODE_1080P_120_REC:
            return "1080P_120_REC";
        case HI_PDT_MEDIAMODE_1080P_240_REC:
            return "1080P_240_REC";
        case HI_PDT_MEDIAMODE_1440P_30_REC:
            return "1440P_30_REC";
        case HI_PDT_MEDIAMODE_1440P_30_RECSNAP:
            return "1440P_30_RECSNAP";
        case HI_PDT_MEDIAMODE_1440P_60_REC:
            return "1440P_60_REC";
        case HI_PDT_MEDIAMODE_1440P_60_RECSNAP:
            return "1440P_60_RECSNAP";
        case HI_PDT_MEDIAMODE_4K2K_25_REC:
            return "4K2K_25_REC";
        case HI_PDT_MEDIAMODE_4K2K_30_REC:
            return "4K2K_30_REC";
        case HI_PDT_MEDIAMODE_4K2K_25_RECSNAP:
            return "4K2K_25_RECSNAP";
        case HI_PDT_MEDIAMODE_4K2K_30_RECSNAP:
            return "4K2K_30_RECSNAP";
        case HI_PDT_MEDIAMODE_4K2K_60_REC:
            return "4K2K_60_REC";
        case HI_PDT_MEDIAMODE_4K2K_60_RECSNAP:
            return "4K2K_60_RECSNAP";
        case HI_PDT_MEDIAMODE_4K2K_120_REC:
            return "4K2K_120_REC";
        case HI_PDT_MEDIAMODE_4K2K_120_RECSNAP:
            return "4K2K_120_RECSNAP";
        case HI_PDT_MEDIAMODE_4K2K_PHOTO:
            return "4K2K_PHOTO";
        case HI_PDT_MEDIAMODE_4K2K_MPHOTO:
            return "4K2K_MPHOTO";
        case HI_PDT_MEDIAMODE_12M_PHOTO:
            return "12M_PHOTO";
        case HI_PDT_MEDIAMODE_12M_MPHOTO:
            return "12M_MPHOTO";
        case HI_PDT_MEDIAMODE_16M_PHOTO:
            return "16M_PHOTO";
        case HI_PDT_MEDIAMODE_16M_MPHOTO:
            return "16M_MPHOTO";
        case HI_PDT_MEDIAMODE_BUTT:
            return "BUTT";
        default:
            return "Unknown";
    }
}

const HI_CHAR* PDT_PARAM_GetPoweronActionStr(HI_PDT_POWERON_ACTION_E enPoweronAction)
{
    switch(enPoweronAction)
    {
        case HI_PDT_POWERON_ACTION_IDLE:
            return "IDLE";
        case HI_PDT_POWERON_ACTION_NORM_REC:
            return "NORM_REC";
        case HI_PDT_POWERON_ACTION_LOOP_REC:
            return "LOOP_REC";
        case HI_PDT_POWERON_ACTION_LPSE_REC:
            return "LPSE_REC";
        case HI_PDT_POWERON_ACTION_SLOW_REC:
            return "SLOW_REC";
        case HI_PDT_POWERON_ACTION_RECSNAP:
            return "RECSNAP";
        case HI_PDT_POWERON_ACTION_LPSE_PHOTO:
            return "LPSE_PHOTO";
        case HI_PDT_POWERON_ACTION_SING_PHOTO:
            return "SING_PHOTO";
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
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
            return "PhotoScene";
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
            return "PhotoOutputFormat";
        case HI_PDT_PARAM_TYPE_DELAY_TIME:
            return "DelayTime";
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            return "LapseInterval";
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
            return "BurstType";
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
            return "LoopTime";
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            return "EncPayloadType";
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
            return "EV";
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
            return "ExpTime";
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
            return "ISO";
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
            return "WB";
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:
            return "MetryType";
        case HI_PDT_PARAM_TYPE_OSD:
            return "OSD";
        case HI_PDT_PARAM_TYPE_AUDIO:
            return "Audio";
        case HI_PDT_PARAM_TYPE_FLIP:
            return "Flip";
        case HI_PDT_PARAM_TYPE_DIS:
            return "DIS";
        case HI_PDT_PARAM_TYPE_LDC:
            return "LDC";
        case HI_PDT_PARAM_TYPE_POWERON_WORKMODE:
            return "PoweronWorkMode";
        case HI_PDT_PARAM_TYPE_POWERON_ACTION:
            return "PoweronAction";
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
        case HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE:
            return "SystemDormantMode";
        case HI_PDT_PARAM_TYPE_USB_MODE:
            return "USBMode";
        case HI_PDT_PARAM_TYPE_LANGUAGE:
            return "SystemLanguage";
        default:
            return "Unknown";
    }
}

#ifdef CFG_DEBUG_LOG_ON

HI_VOID PDT_PARAM_DebugFileMngCfg(const HI_PDT_FILEMNG_CFG_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("Common Configure:\n");
    MLOGD("MountPath[%s]\n", pstCfg->stCommCfg.szMntPath);
    MLOGD("FileRepair: enable[%d] parseExtraMdat[%d] RootDir[%s] TopDirName[%s]\n",
        pstCfg->stCommCfg.stRepairCfg.bEnable, pstCfg->stCommCfg.stRepairCfg.bParseExtraMdat,
        pstCfg->stCommCfg.stRepairCfg.szRootPath, pstCfg->stCommCfg.stRepairCfg.szTopDirName);

#if defined(CONFIG_FILEMNG_DTCF)
    HI_S32 i = 0;
    MLOGD("DCF Configure:\n");
    MLOGD("  PreAllocCnt[%u] PreAllocUnit[%u]\n", pstCfg->stDtcfCfg.u32PreAllocCnt,
        pstCfg->stDtcfCfg.u32PreAllocUnit);
    MLOGD("  SharePercent[%u] WarningStage[%u] GuaranteedStage[%u] RootDir[%s]\n",
        pstCfg->stDtcfCfg.u8SharePercent, pstCfg->stDtcfCfg.u32WarningStage,
        pstCfg->stDtcfCfg.u32GuaranteedStage, pstCfg->stDtcfCfg.szRootDir);
    for(i = 0 ; i < DTCF_DIR_BUTT; i++)
    {
        MLOGD("  DirName%02d[%s] \n", i, pstCfg->stDtcfCfg.aszDirNames[i]);
    }
#else
    HI_U8 u8ObjIdx, u8FileIdx;
    MLOGD("DCF Configure:\n");
    MLOGD("WarningStage[%uMB] DirFreeChar[%s] ObjTypeCnt[%u]\n",
        pstCfg->stDcfCfg.u32WarningStage, pstCfg->stDcfCfg.szDirFreeChar, pstCfg->stDcfCfg.u8ObjTypeCnt);
    for (u8ObjIdx = 0; u8ObjIdx < pstCfg->stDcfCfg.u8ObjTypeCnt; ++u8ObjIdx)
    {
        MLOGD("ObjType[%u]: Type[%d] FileFreeChar[%s] FileCnt[%u]\n", u8ObjIdx,
            pstCfg->stDcfCfg.astObjCfgTbl[u8ObjIdx].enType,
            pstCfg->stDcfCfg.astObjCfgTbl[u8ObjIdx].szFileFreeChar, pstCfg->stDcfCfg.astObjCfgTbl[u8ObjIdx].u8FileCnt);
        for (u8FileIdx = 0; u8FileIdx < pstCfg->stDcfCfg.astObjCfgTbl[u8ObjIdx].u8FileCnt; ++u8FileIdx)
        {
            MLOGD("    FileSuffix[%s]\n", pstCfg->stDcfCfg.astObjCfgTbl[u8ObjIdx].aszExtension[u8FileIdx]);
        }
    }
#endif
    MLOGD("\n\n");
}

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
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugWiFiAPCfg(const HI_HAL_WIFI_APMODE_CFG_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("bHideSSID[%d]\n", pstCfg->bHideSSID);
    MLOGD("SSID[%s], Chn[%d]\n\n", pstCfg->stCfg.szWiFiSSID, pstCfg->s32Channel);
}

HI_VOID PDT_PARAM_DebugDevInfo(const HI_UPGRADE_DEV_INFO_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("software version[%s], model[%s]\n\n", pstCfg->szSoftVersion, pstCfg->szModel);
}

HI_VOID PDT_PARAM_DebugTimedTask(const HI_CHAR* pszName, const HI_TIMEDTASK_ATTR_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pszName);
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("Name[%s] enable[%d] time[%uS]\n\n", pszName, pstCfg->bEnable, pstCfg->u32Time_sec);
}

static HI_VOID PDT_PARAM_DebugRecDataSrc(const HI_PDT_REC_SRC_S *pstDataSrc)
{
    MLOGD("Record Data Source: Venchdl0[%d] Venchdl1[%d] Aenchdl[%d] Thmhdl[%d]\n",
        pstDataSrc->aVencHdl[0], pstDataSrc->aVencHdl[1], pstDataSrc->AencHdl, pstDataSrc->ThmHdl);
}

static HI_VOID PDT_PARAM_DebugPhotoDataSrc(const HI_PHOTOMNG_PHOTO_SRC_S *pstDataSrc)
{
    MLOGD("Photo Data Source: VCap PipeHdl[%d] PipeChnHdl[%d] VProcHdl[%d] VPortHdl[%d] VEncHdl[%d] VEncThmHdl[%d]\n",
        pstDataSrc->VcapPipeHdl, pstDataSrc->VcapPipeChnHdl, pstDataSrc->VprocHdl,
        pstDataSrc->VportHdl, pstDataSrc->VencHdl, pstDataSrc->VencThmHdl);
}

static HI_VOID PDT_PARAM_DebugRecCommAttr(const HI_PDT_REC_COMM_ATTR_S *pstCommAttr)
{
    MLOGD("MediaMode[%s] SplitType[%d] SplitTime[%uS]\n", PDT_PARAM_GetMediaModeStr(pstCommAttr->enMediaMode),
        pstCommAttr->stSplitAttr.enSplitType, pstCommAttr->stSplitAttr.u32SplitTimeLenSec);
    MLOGD("RecBuf[%uMS] PreRec[%uS]\n", pstCommAttr->u32BufferTimeMSec, pstCommAttr->u32PreRecTimeSec);
    MLOGD("MuxerType[%d] ThmType[%d]\n", pstCommAttr->enMuxerType, pstCommAttr->enThmType);
    MLOGD("RepairUnit[%uMB] PreAllocUnit[%uMB] VBufSize[%uKB,%uKB] FileTypeIdx[%u]\n",
        pstCommAttr->u32RepairUnit>>20, pstCommAttr->u32PreAllocUnit>>20,
        pstCommAttr->au32VBufSize[0]>>10, pstCommAttr->au32VBufSize[1]>>10, pstCommAttr->u8FileTypeIdx);
}

static HI_VOID PDT_PARAM_DebugPhotoCommAttr(const HI_PDT_PHOTO_COMM_ATTR_S *pstCommAttr)
{
    MLOGD("MediaMode[%s] JpgFileTypeIdx[%u] DngFileTypeIdx[%u] DngJpgFileTypeIdx[%u]\n",
        PDT_PARAM_GetMediaModeStr(pstCommAttr->enMediaMode), pstCommAttr->u8JpgFileTypeIdx,
        pstCommAttr->u8DngFileTypeIdx, pstCommAttr->u8JpgDngFileTypeIdx);
}

static HI_VOID PDT_PARAM_DebugVideoProtune(const HI_PDT_VIDEO_PROTUNE_ATTR_S *pstProtune)
{
    MLOGD("VideoProtune Configure:\n");
    MLOGD("WB[%u] ISO[%u] Metry[%d] EV[%d]\n",
        pstProtune->u32WB, pstProtune->u32ISO, pstProtune->enMetryType, pstProtune->enEV);
    MLOGD("MetryAverageParamIdx[%u] MetryCenterParamIdx[%u]\n",
        pstProtune->u16MetryAverageParamIdx, pstProtune->u16MetryCenterParamIdx);
}

static HI_VOID PDT_PARAM_DebugPhotoProtune(const HI_PDT_PHOTO_PROTUNE_ATTR_S *pstProtune)
{
    MLOGD("PhotoProtune Configure:\n");
    MLOGD("WB[%u] ISO[%u] Metry[%d] EV[%d] ExpTime[%uus]\n",
        pstProtune->u32WB, pstProtune->u32ISO, pstProtune->enMetryType, pstProtune->enEV, pstProtune->u32ExpTime_us);
    MLOGD("MetryAverageParamIdx[%u] MetryCenterParamIdx[%u]\n",
        pstProtune->u16MetryAverageParamIdx, pstProtune->u16MetryCenterParamIdx);
}

static HI_VOID PDT_PARAM_DebugPhotoProcAlg(const HI_PDT_PHOTO_PROCALG_S *pstProtune)
{
#if 0
    MLOGD("ProcAlg[%d]\n", pstProtune->enProcAlg);
    MLOGD("HDR: NrLuma[%d] NrChrama[%d] Sharpen[%d] Saturation[%d] GlobalContrast[%d] LocalContrast[%d]\n",
        pstProtune->stHdrAttr.s32NrLuma, pstProtune->stHdrAttr.s32NrChroma, pstProtune->stHdrAttr.s32Sharpen,
        pstProtune->stHdrAttr.s32Saturation, pstProtune->stHdrAttr.s32GlobalContrast, pstProtune->stHdrAttr.s32LocalContrast);
    MLOGD("LL: NrLuma[%d] NrChrama[%d] Sharpen[%d] Saturation[%d] Iso[%d]\n",
        pstProtune->stLLAttr.s32NrLuma, pstProtune->stLLAttr.s32NrChroma, pstProtune->stLLAttr.s32Sharpen,
        pstProtune->stLLAttr.s32Saturation, pstProtune->stLLAttr.s32Iso);
#endif
}

HI_VOID PDT_PARAM_DebugNormRecCfg(const HI_PDT_NORM_REC_ATTR_S *pstCfg)
{
    PDT_PARAM_CHECK_POINTER(pstCfg);
    MLOGD("Normal Record Configure:\n");
    PDT_PARAM_DebugRecDataSrc(&pstCfg->stDataSrc);
    PDT_PARAM_DebugRecCommAttr(&pstCfg->stCommAttr);
    PDT_PARAM_DebugVideoProtune(&pstCfg->stProTune);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugLoopRecCfg(const HI_PDT_LOOP_REC_ATTR_S *pstCfg)
{
    MLOGD("Loop Record Configure:\n");
    PDT_PARAM_DebugRecDataSrc(&pstCfg->stDataSrc);
    PDT_PARAM_DebugRecCommAttr(&pstCfg->stCommAttr);
    MLOGD("Loop Time[%uMIN]\n", pstCfg->u32LoopTime_min);
    PDT_PARAM_DebugVideoProtune(&pstCfg->stProTune);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugLapseRecCfg(const HI_PDT_LAPSE_REC_ATTR_S *pstCfg)
{
    MLOGD("Lapse Record Configure:\n");
    PDT_PARAM_DebugRecDataSrc(&pstCfg->stDataSrc);
    PDT_PARAM_DebugRecCommAttr(&pstCfg->stCommAttr);
    MLOGD("Lapse Interval[%ums]\n", pstCfg->u32Interval_ms);
    PDT_PARAM_DebugVideoProtune(&pstCfg->stProTune);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugSlowRecCfg(const HI_PDT_SLOW_REC_ATTR_S *pstCfg)
{
    MLOGD("Slow Record Configure:\n");
    PDT_PARAM_DebugRecDataSrc(&pstCfg->stDataSrc);
    PDT_PARAM_DebugRecCommAttr(&pstCfg->stCommAttr);
    MLOGD("PlayRate[%ufps, %ufps]\n", pstCfg->au32PlayFrmRate[0], pstCfg->au32PlayFrmRate[1]);
    PDT_PARAM_DebugVideoProtune(&pstCfg->stProTune);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugSinglePhotoCfg(const HI_PDT_SINGLE_PHOTO_ATTR_S *pstCfg)
{
    MLOGD("Single Photo Configure:\n");
    PDT_PARAM_DebugPhotoDataSrc(&pstCfg->stDataSrc);
    PDT_PARAM_DebugPhotoCommAttr(&pstCfg->stCommAttr);
    MLOGD("Output Format[%d] Scene[%d]\n", pstCfg->enOutputFmt, pstCfg->enScene);
    PDT_PARAM_DebugPhotoProcAlg(&pstCfg->stProcAlg);
    PDT_PARAM_DebugPhotoProtune(&pstCfg->stProTune);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugDelayPhotoCfg(const HI_PDT_DELAY_PHOTO_ATTR_S *pstCfg)
{
    MLOGD("Delay Photo Configure:\n");
    PDT_PARAM_DebugPhotoDataSrc(&pstCfg->stDataSrc);
    PDT_PARAM_DebugPhotoCommAttr(&pstCfg->stCommAttr);
    MLOGD("Output Format[%d] Scene[%d]\n", pstCfg->enOutputFmt, pstCfg->enScene);
    PDT_PARAM_DebugPhotoProcAlg(&pstCfg->stProcAlg);
    MLOGD("Delay time[%uS]\n", pstCfg->u32DelayTime_s);
    PDT_PARAM_DebugPhotoProtune(&pstCfg->stProTune);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugLapsePhotoCfg(const HI_PDT_LAPSE_PHOTO_ATTR_S *pstCfg)
{
    MLOGD("Lapse Photo Configure:\n");
    PDT_PARAM_DebugPhotoDataSrc(&pstCfg->stDataSrc);
    PDT_PARAM_DebugPhotoCommAttr(&pstCfg->stCommAttr);
    MLOGD("Lapse interval[%ums]\n", pstCfg->u32Interval_ms);
    PDT_PARAM_DebugPhotoProtune(&pstCfg->stProTune);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugBurstCfg(const HI_PDT_BURST_ATTR_S *pstCfg)
{
    MLOGD("Burst Photo Configure:\n");
    PDT_PARAM_DebugPhotoDataSrc(&pstCfg->stDataSrc);
    PDT_PARAM_DebugPhotoCommAttr(&pstCfg->stCommAttr);
    MLOGD("Burst type[%d]\n", pstCfg->enJpgBurstType);
    PDT_PARAM_DebugVideoProtune(&pstCfg->stProTune);
    MLOGD("\n\n");
}

HI_VOID PDT_PARAM_DebugRecSnapCfg(const HI_PDT_RECSNAP_ATTR_S *pstCfg)
{
    MLOGD("Record+Photo Configure:\n");
    PDT_PARAM_DebugRecDataSrc(&pstCfg->stDataSrc.stRecSrc);
    PDT_PARAM_DebugPhotoDataSrc(&pstCfg->stDataSrc.stPhotoSrc);
    PDT_PARAM_DebugRecCommAttr(&pstCfg->stRecAttr);
    MLOGD("PhotoFileTypeIdx[%u]\n", pstCfg->u32SnapFileTypeIndex);
    MLOGD("PhotoInterval[%ums]\n", pstCfg->u32SnapInterval_ms);
    PDT_PARAM_DebugVideoProtune(&pstCfg->stProTune);
    MLOGD("\n\n");
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

HI_VOID PDT_PARAM_DebugUVCCfg(const HI_PDT_PARAM_UVC_ATTR_S *pstCfg)
{
    MLOGD("MediaMode[%s]\n", PDT_PARAM_GetMediaModeStr(pstCfg->enMediaMode));
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
    MLOGD("SysFile[%s]\n\n", pstCfg->szSysFile);
    MLOGD("ProcFile[%s]\n\n", pstCfg->szProcFile);
}

HI_VOID PDT_PARAM_DebugHDMIPreviewCfg(const HI_PDT_PARAM_HDMI_PREVIEW_ATTR_S *pstCfg)
{
    MLOGD("MediaMode[%s]\n", PDT_PARAM_GetMediaModeStr(pstCfg->enMediaMode));
}

HI_VOID PDT_PARAM_DebugHDMIPlaybackCfg(const HI_PDT_PARAM_HDMI_PLAYBACK_ATTR_S *pstCfg)
{
    MLOGD("MediaMode[%s]\n", PDT_PARAM_GetMediaModeStr(pstCfg->enMediaMode));
    MLOGD("VoutOpt Type[%d:0-vpss,1-vo] ModHdl[%d] ChnHdl[%d]\n",
        pstCfg->stVoutOpt.enVoutType, pstCfg->stVoutOpt.hModHdl, pstCfg->stVoutOpt.hChnHdl);
    MLOGD("AoutOpt AudDevHdl[%d] AudTrackHdl[%d]\n",
        pstCfg->stAoutOpt.hAudDevHdl, pstCfg->stAoutOpt.hAudTrackHdl);
}

HI_VOID PDT_PARAM_DebugPlaybackCfg(const HI_PDT_PARAM_PLAYBACK_ATTR_S *pstCfg)
{
    MLOGD("MediaMode[%s]\n", PDT_PARAM_GetMediaModeStr(pstCfg->enMediaMode));
    MLOGD("VoutOpt Type[%d:0-vpss,1-vo] ModHdl[%d] ChnHdl[%d]\n",
        pstCfg->stVoutOpt.enVoutType, pstCfg->stVoutOpt.hModHdl, pstCfg->stVoutOpt.hChnHdl);
    MLOGD("AoutOpt AudDevHdl[%d] AudTrackHdl[%d]\n",
        pstCfg->stAoutOpt.hAudDevHdl, pstCfg->stAoutOpt.hAudTrackHdl);
}

static HI_VOID PDT_PARAM_DebugViVpssMode(const HI_PDT_MEDIA_VI_VPSS_MODE_S* pstCfg)
{
    HI_S32 s32VcapIdx, s32VcapPipeIdx;
    for (s32VcapIdx = 0; s32VcapIdx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32VcapIdx)
    {
        for (s32VcapPipeIdx = 0; s32VcapPipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32VcapPipeIdx)
        {
            MLOGD("Vcap[%d] Pipe[%d] PipeHdl[%d] ViVpssMode[%d:0-vioffline_vpssoffline]\n",
                s32VcapIdx, s32VcapPipeIdx,
                pstCfg->astMode[s32VcapIdx][s32VcapPipeIdx].VcapPipeHdl,
                pstCfg->astMode[s32VcapIdx][s32VcapPipeIdx].enMode);
        }
    }
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

static HI_VOID PDT_PARAM_DebugVcapDevCfg(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstCfg)
{
    HI_S32 s32PipeIdx, s32PipeChnIdx;
    const HI_PDT_MEDIA_VCAP_PIPE_ATTR_S*     pstPipeAttr = NULL;
    const HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChnAttr = NULL;
    MLOGD("VcapDev Enable[%d] handle[%d]\n", pstCfg->bEnable, pstCfg->VcapDevHdl);
    MLOGD("Sensor: Resolution[%ux%u] Mode[%u] WdrMode[%d]\n",
        pstCfg->stSnsAttr.stResolution.u32Width, pstCfg->stSnsAttr.stResolution.u32Height,
        pstCfg->stSnsAttr.u32SnsMode, pstCfg->stSnsAttr.enWdrMode);
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
            MLOGD("      LDC: enable[%d] bAspect[%d] ratio[x:%d,y:%d,xy:%d] offset[%d,%d] distortionRation[%d]\n",
                pstPipeChnAttr->stLDCAttr.bEnable, pstPipeChnAttr->stLDCAttr.stAttr.bAspect,
                pstPipeChnAttr->stLDCAttr.stAttr.s32XRatio,
                pstPipeChnAttr->stLDCAttr.stAttr.s32YRatio,
                pstPipeChnAttr->stLDCAttr.stAttr.s32XYRatio,
                pstPipeChnAttr->stLDCAttr.stAttr.s32CenterXOffset,
                pstPipeChnAttr->stLDCAttr.stAttr.s32CenterYOffset,
                pstPipeChnAttr->stLDCAttr.stAttr.s32DistortionRatio);
            MLOGD("      DIS: enable[%d] MotionType[%d] MotionLevel[%d]\n",
                pstPipeChnAttr->stDISAttr.bEnable,
                pstPipeChnAttr->stDISAttr.stAttr.stDISConfig.enMode,
                pstPipeChnAttr->stDISAttr.stAttr.stDISConfig.enMotionLevel);

            MLOGD("      bEnable[%d] bGdcBypass[%d] u32MovingSubjectLevel[%u] s32RollingShutterCoef[%d]\n",
                pstPipeChnAttr->stDISAttr.stAttr.stDisAttr.bEnable,
                pstPipeChnAttr->stDISAttr.stAttr.stDisAttr.bGdcBypass,
                pstPipeChnAttr->stDISAttr.stAttr.stDisAttr.u32MovingSubjectLevel,
                pstPipeChnAttr->stDISAttr.stAttr.stDisAttr.s32RollingShutterCoef);
            MLOGD("      s32Timelag[%d] u32ViewAngle[%u] u32HorizontalLimit[%u] u32VerticalLimit[%u] bStillCrop[%d]\n",
                pstPipeChnAttr->stDISAttr.stAttr.stDisAttr.s32Timelag,
                pstPipeChnAttr->stDISAttr.stAttr.stDisAttr.u32ViewAngle,
                pstPipeChnAttr->stDISAttr.stAttr.stDisAttr.u32HorizontalLimit,
                pstPipeChnAttr->stDISAttr.stAttr.stDisAttr.u32VerticalLimit,
                pstPipeChnAttr->stDISAttr.stAttr.stDisAttr.bStillCrop);

            MLOGD(YELLOW"      stLDCV2Attr: bEnable[%d] s32FocalLenX[%d] s32FocalLenY[%d] s32CoorShiftX[%d] s32CoorShiftY[%d] s32SrcJunPt[%d]\n"NONE,
                pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr.bEnable,
                pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr.stAttr.s32FocalLenX,
                pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr.stAttr.s32FocalLenY,
                pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr.stAttr.s32CoorShiftX,
                pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr.stAttr.s32CoorShiftY,
                pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr.stAttr.s32SrcJunPt);

            MLOGD(YELLOW"      *** SrcCaliRatio[%d][%d] ***\n", SRC_LENS_COEF_SEG, SRC_LENS_COEF_NUM);
            HI_S32 i,j;
            for(i = 0; i < SRC_LENS_COEF_SEG; i++)
            {
                for(j = 0; j < SRC_LENS_COEF_NUM; j++)
                {
                    MLOGD(YELLOW"      SrcCaliRatio[%d][%d] = [%d]\n"NONE,
                        i, j, pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr.stAttr.as32SrcCaliRatio[i][j]);
                }
            }

            MLOGD(YELLOW"      *** DstCaliRatio[%d][%d] ***\n", DST_LENS_COEF_SEG, DST_LENS_COEF_NUM);
            for(i = 0; i < DST_LENS_COEF_SEG; i++)
            {
                for(j = 0; j < DST_LENS_COEF_NUM; j++)
                {
                    MLOGD(YELLOW"      DstCaliRatio[%d][%d] = [%d]\n"NONE,
                        i, j, pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr.stAttr.as32DstCaliRatio[i][j]);
                }
            }

            MLOGD(YELLOW"      *** DstJunPt[%d] ***\n", DST_LENS_COEF_SEG - 1);
            for(i = 0; i < (DST_LENS_COEF_SEG - 1); i++)
            {
                MLOGD(YELLOW"      DstJunPt[%d] = [%d]\n"NONE,
                        i, pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr.stAttr.as32DstJunPt[i]);
            }
        }
    }
}

static HI_VOID PDT_PARAM_DebugVprocCfg(const HI_PDT_MEDIA_VPROC_CFG_S *pstCfg)
{
    HI_S32 s32VpssIdx, s32VpssPortIdx;
    const HI_PDT_MEDIA_VPSS_ATTR_S*      pstVpssAttr = NULL;
    const HI_PDT_MEDIA_VPSS_PORT_ATTR_S* pstVpssPortAttr = NULL;
    for (s32VpssIdx = 0; s32VpssIdx < HI_PDT_MEDIA_VPSS_MAX_CNT; ++s32VpssIdx)
    {
        pstVpssAttr = &pstCfg->astVpssAttr[s32VpssIdx];
        MLOGD("Vpss[%d] enable[%d] VpssHdl[%d] VcapPipeHdl[%d] VcapPipeChnHdl[%d]\n",
            s32VpssIdx, pstVpssAttr->bEnable, pstVpssAttr->VpssHdl,
            pstVpssAttr->VcapPipeHdl, pstVpssAttr->VcapPipeChnHdl);
        MLOGD("  MaxSize[%ux%u] srcFrmRate[%dfps] dstFrmRate[%dfps] PixelFmt[%d:0-YUV420]\n",
            pstVpssAttr->stVpssAttr.u32MaxW, pstVpssAttr->stVpssAttr.u32MaxH,
            pstVpssAttr->stVpssAttr.stFrameRate.s32SrcFrameRate,
            pstVpssAttr->stVpssAttr.stFrameRate.s32DstFrameRate,
            pstVpssAttr->stVpssAttr.enPixelFormat);
        MLOGD("  bNrEn[%d] CompressMode[%d] NrMotionMode[%d]\n",
            pstVpssAttr->stVpssAttr.bNrEn, pstVpssAttr->stVpssAttr.stNrAttr.enCompressMode,
            pstVpssAttr->stVpssAttr.stNrAttr.enNrMotionMode);
        for (s32VpssPortIdx = 0; s32VpssPortIdx < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++s32VpssPortIdx)
        {
            pstVpssPortAttr = &pstVpssAttr->astVportAttr[s32VpssPortIdx];
            MLOGD("    Port[%d] enable[%d] VpssPortHdl[%d] Resoution[%ux%u] srcFrmRate[%dfps] dstFrmRate[%dfps]\n",
                s32VpssPortIdx, pstVpssPortAttr->bEnable, pstVpssPortAttr->VportHdl,
                pstVpssPortAttr->stResolution.u32Width, pstVpssPortAttr->stResolution.u32Height,
                pstVpssPortAttr->stFrameRate.s32SrcFrameRate, pstVpssPortAttr->stFrameRate.s32DstFrameRate);
            MLOGD("      Rotate[%d] Flip[%d] Mirror[%d]\n",
                pstVpssPortAttr->enRotate, pstVpssPortAttr->bFlip, pstVpssPortAttr->bMirror);
            MLOGD("      VideoFmt[%d] PixedFmt[%d]\n",
                pstVpssPortAttr->enVideoFormat, pstVpssPortAttr->enPixelFormat);
            MLOGD("      aspectMode[%d]\n",
                pstVpssPortAttr->stAspectRatio.enMode);
            MLOGD("      BgColor[%u]\n",
                pstVpssPortAttr->stAspectRatio.u32BgColor);
            MLOGD("      x[%d] y[%d] w[%u] h[%u]\n",
                pstVpssPortAttr->stAspectRatio.stVideoRect.s32X,
                pstVpssPortAttr->stAspectRatio.stVideoRect.s32Y,
                pstVpssPortAttr->stAspectRatio.stVideoRect.u32Width,
                pstVpssPortAttr->stAspectRatio.stVideoRect.u32Height);
        }
    }
}

HI_VOID PDT_PARAM_DebugVencAttr(const HI_MEDIA_VENC_ATTR_S *pstAttr)
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

static HI_VOID PDT_PARAM_DebugVencCfg(const HI_PDT_MEDIA_VENC_CFG_S *pstCfg)
{
    MLOGD("Enable[%d] VencHdl[%d] BindedMod[%d:0-vcap,1-vpss] ModHdl[%d] Chn[%d]\n",
        pstCfg->bEnable, pstCfg->VencHdl, pstCfg->enBindedMod, pstCfg->ModHdl, pstCfg->ChnHdl);
    PDT_PARAM_DebugVencAttr(&pstCfg->stVencAttr);
}

static HI_VOID PDT_PARAM_DebugOsdCfg(const HI_PDT_MEDIA_OSD_CFG_S* pstCfg)
{
    MLOGD("OSD: BindModule[%d:0-vcap,1-vpss,3-venc]\n", pstCfg->enBindMod);
    MLOGD("  Time: show[%d] format[%d:0-24h] Color[%08x] BgColor[%08x]\n",
        pstCfg->stTimeOsd.bShow, pstCfg->stTimeOsd.enFormat, pstCfg->stTimeOsd.u32Color,
        pstCfg->stTimeOsd.u32BgColor);
    MLOGD("    FontSize[%ux%u], StartPos[%d,%d] FgAlpha[%x] BgAlpha[%x]\n",
        pstCfg->stTimeOsd.stFontSize.u32Width, pstCfg->stTimeOsd.stFontSize.u32Height,
        pstCfg->stTimeOsd.stStartPos.s32X, pstCfg->stTimeOsd.stStartPos.s32Y,
        pstCfg->stTimeOsd.u32FgAlpha, pstCfg->stTimeOsd.u32BgAlpha);
}

static HI_VOID PDT_PARAM_DebugDispCfg(const HI_PDT_MEDIA_DISP_CFG_S *pstCfg)
{
    HI_S32 s32WndIdx;
    MLOGD("Display: enable[%d] Hdl[%d]\n", pstCfg->bEnable, pstCfg->VdispHdl);
    MLOGD("  CSC enable[%d]\n", pstCfg->stCscAttr.bEnable);
    MLOGD("    Matrix Type[%d:0-Identity,1-BT601ToBT709,2-BT709ToBT601,3-BT601ToRGB,4-BT709ToRGB,5-RGBToBT601,6-RGBToBT709]\n",
        pstCfg->stCscAttr.stAttrEx.stVoCSC.enCscMatrix);
    MLOGD("    luma[%u], contrast[%u], hue[%u], satuature[%u]\n",
        pstCfg->stCscAttr.stAttrEx.stVoCSC.u32Luma,
        pstCfg->stCscAttr.stAttrEx.stVoCSC.u32Contrast,
        pstCfg->stCscAttr.stAttrEx.stVoCSC.u32Hue,
        pstCfg->stCscAttr.stAttrEx.stVoCSC.u32Satuature);
    MLOGD("  PubAttr: bg_color[%u], interface_type[%d], interface_sync[%d] Vdev framerate[%ufps]\n",
          pstCfg->stDispAttr.stPubAttr.u32BgColor, pstCfg->stDispAttr.stPubAttr.enIntfType,
          pstCfg->stDispAttr.stPubAttr.enIntfSync, pstCfg->stDispAttr.stUserInfoAttr.u32DevFrameRate);
    MLOGD("  stImageSize.u32Width[%u] stImageSize.u32Height[%u] VLframerate[%ufps] u32BufLen[%u]\n",
        pstCfg->stVideoLayerAttr.stImageSize.u32Width,
        pstCfg->stVideoLayerAttr.stImageSize.u32Height,
        pstCfg->stVideoLayerAttr.u32VLFrameRate, pstCfg->stVideoLayerAttr.u32BufLen);
    for (s32WndIdx = 0; s32WndIdx < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++s32WndIdx)
    {
        MLOGD("  Wnd[%d] Enable[%d] Hdl[%d] BindedMod[%d] ModHdl[%d] ChnHdl[%d] Rotate[%d:0-0,1-90,2-180,3-270]\n",
            s32WndIdx, pstCfg->astWndCfg[s32WndIdx].bEnable, pstCfg->astWndCfg[s32WndIdx].WndHdl,
            pstCfg->astWndCfg[s32WndIdx].enBindedMod, pstCfg->astWndCfg[s32WndIdx].ModHdl,
            pstCfg->astWndCfg[s32WndIdx].ChnHdl, pstCfg->astWndCfg[s32WndIdx].enRotate);
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

static HI_VOID PDT_PARAM_DebugAudioCfg(const HI_PDT_MEDIA_AUDIO_CFG_S *pstCfg)
{
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_ACAP_MAX_CNT; ++i)
    {
        MLOGD("acap[%d] enable[%d] hdl[%d] gain[%d]\n", i, pstCfg->astAcapCfg[i].bEnable,
            pstCfg->astAcapCfg[i].AcapHdl, pstCfg->astAcapCfg[i].s32AudioGain);
        MLOGD("  SampleRate[%d] Bitwidth[%d] soundMode[%d] workMode[%d] PtNumPerFrm[%u] MixerMicMode[%d] ResampleRate[%d]\n",
            pstCfg->astAcapCfg[i].stAcapAttr.enSampleRate, pstCfg->astAcapCfg[i].stAcapAttr.enBitwidth,
            pstCfg->astAcapCfg[i].stAcapAttr.enSoundMode, pstCfg->astAcapCfg[i].stAcapAttr.enWorkmode,
            pstCfg->astAcapCfg[i].stAcapAttr.u32PtNumPerFrm, pstCfg->astAcapCfg[i].stAcapAttr.enMixerMicMode,
            pstCfg->astAcapCfg[i].stAcapAttr.enResampleRate);
    }
    for (i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; ++i)
    {
        MLOGD("aenc[%d] enable[%d] hdl[%d] acapHdl[%d] format[%d:0-aac] PtNumPerFrm[%u]\n",
            i, pstCfg->astAencCfg[i].bEnable,
            pstCfg->astAencCfg[i].AencHdl, pstCfg->astAencCfg[i].AcapHdl,
            pstCfg->astAencCfg[i].enFormat, pstCfg->astAencCfg[i].u32PtNumPerFrm);
        MLOGD("  aacattr type[%d] Bitrate[%d] SmpRate[%d] BitWidth[%d] SoundMode[%d] TransType[%d] BandWidth[%d]\n",
            pstCfg->astAencCfg[i].unAttr.stAACAttr.enAACType,
            pstCfg->astAencCfg[i].unAttr.stAACAttr.enBitRate,
            pstCfg->astAencCfg[i].unAttr.stAACAttr.enSmpRate,
            pstCfg->astAencCfg[i].unAttr.stAACAttr.enBitWidth,
            pstCfg->astAencCfg[i].unAttr.stAACAttr.enSoundMode,
            pstCfg->astAencCfg[i].unAttr.stAACAttr.enTransType,
            pstCfg->astAencCfg[i].unAttr.stAACAttr.s16BandWidth);
    }
}

static HI_VOID PDT_PARAM_DebugAudioOutCfg(const HI_PDT_MEDIA_AUDIOOUT_CFG_S* pstCfg)
{
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; ++i)
    {
        MLOGD("ao[%d] enable[%d] hdl[%d] chn_hdl[%d] volume[%d]\n",
            i, pstCfg->astAoCfg[i].bEnable, pstCfg->astAoCfg[i].AoHdl, pstCfg->astAoCfg[i].AoChnHdl, pstCfg->astAoCfg[i].s32Volume);
        MLOGD("  aoattr SmpRate[%d] BitWidth[%d] SoundMode[%d] WorkMode[%d] PtNumPerFrm[%u] InSmpRate[%d]\n",
            pstCfg->astAoCfg[i].stAoAttr.enSampleRate, pstCfg->astAoCfg[i].stAoAttr.enBitwidth,
            pstCfg->astAoCfg[i].stAoAttr.enSoundMode, pstCfg->astAoCfg[i].stAoAttr.enWorkmode,
            pstCfg->astAoCfg[i].stAoAttr.u32PtNumPerFrm, pstCfg->astAoCfg[i].stAoAttr.enInSampleRate);
    }
}

HI_VOID PDT_PARAM_DebugMediaCfg(const HI_PDT_MEDIA_CFG_S *pstCfg)
{
    HI_S32 s32Idx;
    PDT_PARAM_DebugViVpssMode(&pstCfg->stViVpssMode);
    PDT_PARAM_DebugVBCfg(&pstCfg->stVBCfg);
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32Idx)
    {
        MLOGD("Vcapture Device[%d] Configure:\n", s32Idx);
        PDT_PARAM_DebugVcapDevCfg(&pstCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[s32Idx]);
    }
    MLOGD("Vprocessor Configure:\n");
    PDT_PARAM_DebugVprocCfg(&pstCfg->stVideoCfg.stVprocCfg);
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VENC_MAX_CNT; ++s32Idx)
    {
        MLOGD("Vencoder[%d] Configure:\n", s32Idx);
        PDT_PARAM_DebugVencCfg(&pstCfg->stVideoCfg.astVencCfg[s32Idx]);
    }
    PDT_PARAM_DebugOsdCfg(&pstCfg->stVideoCfg.stOsdCfg);
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_DISP_MAX_CNT; ++s32Idx)
    {
        MLOGD("Display[%d] Configure:\n", s32Idx);
        PDT_PARAM_DebugDispCfg(&pstCfg->stVideoOutCfg.astDispCfg[s32Idx]);
    }
    PDT_PARAM_DebugAudioCfg(&pstCfg->stAudioCfg);
    PDT_PARAM_DebugAudioOutCfg(&pstCfg->stAudioOutCfg);
    MLOGD("\n\n");
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
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

