/**
 * @file      ui_common.c
 * @brief     ui internal common function code
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include "hi_gv_input.h"
#include "ui_common.h"
#include "ui_common_playback.h"
#include "hi_keymng.h"
#include "ui_powercontrol.h"
#include "hi_product_ui.h"
#include "mpi_sys.h"
#include "hi_hal_touchpad.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


static HI_U32 s_u32TotalBitRate = 0;
static HI_U32 s_u32WarningStage = 0;
static HI_BOOL s_bUIFirstRun = HI_TRUE;
static HI_BOOL s_bUSB_In = HI_FALSE;
static HI_BOOL s_bSDAvailable = HI_FALSE;
static HI_BOOL s_bUpgradeSuccess = HI_FALSE;
static PDT_UI_OUTPUT_STATE_E s_enUiState = UI_OUTPUT_LCD;
static HI_PDT_WORKMODE_E s_enSwitchUSBMode = HI_PDT_WORKMODE_BUTT;
static HI_PDT_WORKMODE_E s_enWorkMode = HI_PDT_WORKMODE_BUTT;
static PDT_UI_MESSAGE_CONTEXT s_stMessageCtx = {.bMsgProcessed = HI_TRUE, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
static HI_STORAGEMNG_CFG_S s_stStorageCfg = {};
HIGV_HANDLE s_u32WinCallDialog = 0;
static HI_EVENT_ID s_LastSDEventID = 0;
static HI_S32 s_s32LastStringID = 0;

#if defined(CONFIG_WIFI_ON)
static PDT_UI_WIFI_STATUS_S s_stWiFiStatus = {.bInit = HI_FALSE, .bStart = HI_FALSE, .bResume = HI_FALSE,};

HI_S32 PDT_UI_COMM_GetWiFiStatus(PDT_UI_WIFI_STATUS_S *pstWiFiStatus)
{
    pstWiFiStatus->bInit = s_stWiFiStatus.bInit;
    pstWiFiStatus->bStart = s_stWiFiStatus.bStart;
    pstWiFiStatus->bResume = s_stWiFiStatus.bResume;

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_SetWiFiStatus(PDT_UI_WIFI_STATUS_S *pstWiFiStatus)
{
    s_stWiFiStatus.bInit = pstWiFiStatus->bInit;
    s_stWiFiStatus.bStart = pstWiFiStatus->bStart;
    s_stWiFiStatus.bResume = pstWiFiStatus->bResume;

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_StartWiFi(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HAL_WIFI_APMODE_CFG_S stApCfg;
    HI_HAL_WIFI_CFG_S stCfg;
    memset(&stApCfg, '\0', sizeof(HI_HAL_WIFI_APMODE_CFG_S));

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stApCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    stCfg.enMode = HI_HAL_WIFI_MODE_AP;
    memcpy(&stCfg.unCfg.stApCfg, &stApCfg, sizeof(HI_HAL_WIFI_APMODE_CFG_S));

    s32Ret = HI_HAL_WIFI_Start(&stCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGD("HI_HAL_WIFI_Start\n");

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_StopWiFi(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_HAL_WIFI_Stop();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGD("HI_HAL_WIFI_Stop\n");

    return HI_SUCCESS;
}
#endif

#define KEYTONE_RES_PATH  "/app/bin/res/sound/keytone.pcm"
static HI_U64 s_u64PhyAddr = 0;
static HI_VOID* s_pVitAddr = NULL;
static HI_U32 s_u32DataLen = 0;

static HI_S32 PDT_UI_COMM_GetMtdPartInfo(const HI_CHAR* pszBootargs, HI_UPGRADE_MTDPART_INFO_S* pstMtdPartInfo)
{
    HI_CHAR szMtdPartInfo[HI_UPGRADE_MAX_ENV_LEN] = {0,};
    HI_CHAR* pszStr = NULL;
    pszStr = strstr(pszBootargs, "mtdparts=");
    if (!pszStr)
    {
        MLOGE("Invalid Bootargs[%s]\n", pszBootargs);
        return HI_FAILURE;
    }

    snprintf(szMtdPartInfo, HI_UPGRADE_MAX_ENV_LEN, "%s", pszStr);
    MLOGD(YELLOW"MtdInfo[%s]"NONE"\n", szMtdPartInfo);

    HI_CHAR* pSave = NULL;
    HI_CHAR* pToken = NULL;
    HI_CHAR* pTokenStart = NULL;
    HI_CHAR* pTokenEnd = NULL;
    memset(pstMtdPartInfo, 0, sizeof(HI_UPGRADE_MTDPART_INFO_S));
    for (pszStr = szMtdPartInfo; ; pszStr = NULL)
    {
        pToken = strtok_r(pszStr, ",", &pSave);
        if (!pToken)
        {
            break;
        }

        pTokenStart = strchr(pToken, '(');
        pTokenEnd = strchr(pToken, ')');
        if (!pTokenStart || !pTokenEnd)
        {
            continue;
        }

        if (HI_UPGRADE_MAX_PART_CNT <= pstMtdPartInfo->s32PartitionCnt)
        {
            MLOGE("too many partition\n");
            return HI_FAILURE;
        }
        snprintf(pstMtdPartInfo->aszPartitionName[pstMtdPartInfo->s32PartitionCnt],
                 (pTokenEnd - pTokenStart), "%s", pTokenStart + 1);
        MLOGD(GREEN"Part[%d] Name[%s]"NONE"\n", pstMtdPartInfo->s32PartitionCnt,
              pstMtdPartInfo->aszPartitionName[pstMtdPartInfo->s32PartitionCnt]);
        pstMtdPartInfo->s32PartitionCnt++;
    }

    return HI_SUCCESS;
}


static HI_S32 PDT_UI_COMM_GetBootArgs(HI_CHAR *pszBootargs, HI_U32 u32Len)
{
    FILE* fp = NULL;
    fp = fopen("/proc/cmdline", "r");
    if(fp != NULL)
    {
        if(!feof(fp))
        {
            while(!fgets(pszBootargs, (u32Len - 1), fp))
            {
               MLOGD("Bootargs[%s]\n", pszBootargs);
               break;
            }
        }
        fclose(fp);
        return HI_SUCCESS;
    }
    else
    {
        MLOGE("fopen /proc/cmdline failed!\n");
        return HI_FAILURE;
    }
}

static HI_VOID PDT_UI_COMM_DelUpgradeFiles(HI_VOID)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szBootArgs[HI_UPGRADE_MAX_ENV_LEN] = {0,};
    HI_UPGRADE_MTDPART_INFO_S stMtdPartInfo;
    HI_STORAGEMNG_CFG_S stStorageCfg;

    s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"HI_PDT_PARAM_GetStorageCfg");

    s32Ret = PDT_UI_COMM_GetBootArgs(szBootArgs, HI_UPGRADE_MAX_ENV_LEN);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGW("GetBootArgs failed\n");
        return;
    }

    s32Ret = PDT_UI_COMM_GetMtdPartInfo(szBootArgs, &stMtdPartInfo);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGW("GetMtdInfo failed\n");
        return;
    }

    HI_CHAR szImgPath[HI_APPCOMM_MAX_PATH_LEN] = {0,};
    HI_S32 i;
    for (i = 0; i < stMtdPartInfo.s32PartitionCnt; ++i)
    {
        snprintf(szImgPath, HI_APPCOMM_MAX_PATH_LEN, "%s/%s", stStorageCfg.szMntPath, stMtdPartInfo.aszPartitionName[i]);
        if (0 == access(szImgPath, F_OK))
        {
            (HI_VOID)remove(szImgPath);
            MLOGD(GREEN"Delete image file[%s]"NONE"\n", szImgPath);
        }
        else
        {
            MLOGD(YELLOW"image file[%s] not exist, ignore"NONE"\n", szImgPath);
        }
    }
    snprintf(szImgPath, HI_APPCOMM_MAX_PATH_LEN, "%s/config", stStorageCfg.szMntPath);
    if (0 == access(szImgPath, F_OK))
    {
        (HI_VOID)remove(szImgPath);
        MLOGD(GREEN"Delete config file[%s]"NONE"\n", szImgPath);
    }
}

HI_S32 PDT_UI_COMM_InitKeyTone(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    FILE *fp = NULL;
    HI_U32 u32CurPos=0;
    if(NULL != s_pVitAddr)
    {
        MLOGE("multiple open\n");
        return HI_FAILURE;
    }
    fp=fopen(KEYTONE_RES_PATH,"rb");
    if(fp)
    {
        u32CurPos = ftell(fp);
        fseek(fp, 0L, SEEK_END);
        s_u32DataLen = ftell(fp);
        fseek(fp, u32CurPos, SEEK_SET);
    }
    else
    {
        MLOGE("open fail\n");
        s_u32DataLen = 0;
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&s_u64PhyAddr, &s_pVitAddr, "keytone", "anonymous", s_u32DataLen);
    if (HI_SUCCESS != s32Ret)
    {
        fclose(fp);
        MLOGE("AllocBuffer failed\n\n");
        return HI_FAILURE;
    }
    if(fread(s_pVitAddr,s_u32DataLen,1,fp)<1)
    {
        fclose(fp);
        MLOGE("read fail\n");
        return HI_FAILURE;
   }
    fclose(fp);

    return s32Ret;
}

HI_S32 PDT_UI_COMM_DeinitKeyTone(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(s_pVitAddr)
    {
        s32Ret = HI_MPI_SYS_MmzFree(s_u64PhyAddr, s_pVitAddr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        s_pVitAddr = NULL;
        s_u32DataLen = 0;
        s_u64PhyAddr = 0;

    }
    return s32Ret;
}

static HI_S32 PDT_UI_COMM_KeyTone(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bKeyTone = HI_FALSE;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {.enWorkMode = HI_PDT_WORKMODE_NORM_REC,};

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_KEYTONE, &bKeyTone);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    if(HI_FALSE == bKeyTone)
    {
        return HI_SUCCESS;
    }

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    if(HI_PDT_WORKMODE_USB_STORAGE == stWorkState.enWorkMode
        || HI_PDT_WORKMODE_UVC == stWorkState.enWorkMode)
    {
        return HI_SUCCESS;
    }

    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    HI_PDT_MEDIA_CFG_S stMediaCfg = {};
    HI_PDT_SCENE_MODE_S stSceneMode = {};
    if (HI_PDT_WORKMODE_HDMI_PLAYBACK == stWorkState.enWorkMode)
    {
        s32Ret = HI_PDT_PARAM_GetWorkModeParam(stWorkState.enWorkMode,
            HI_PDT_PARAM_TYPE_MEDIAMODE, &enMediaMode);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = HI_PDT_PARAM_GetMediaCfg(HI_PDT_WORKMODE_HDMI_PLAYBACK, enMediaMode, &stMediaCfg, &stSceneMode);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        HI_PDT_WORKMODE_E enPowerOnWorkMode = HI_PDT_WORKMODE_BUTT;
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enPowerOnWorkMode);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = HI_PDT_PARAM_GetWorkModeParam(enPowerOnWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE, &enMediaMode);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = HI_PDT_PARAM_GetMediaCfg(HI_PDT_WORKMODE_PLAYBACK, enMediaMode, &stMediaCfg, &stSceneMode);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    s32Ret = HI_SYSTEM_Sound(stMediaCfg.stAudioOutCfg.astAoCfg[0].AoHdl,
        &stMediaCfg.stAudioOutCfg.astAoCfg[0].stAoAttr, s_u64PhyAddr, s_u32DataLen);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_VOID PDT_UI_COMM_EnableUSB(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Value = -1;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_USB_MODE, &s32Value);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "GetUsbMode");

    s32Ret = HI_USB_SetMode((HI_USB_MODE_E)s32Value);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "SetUsbMode");
}

HI_VOID PDT_UI_COMM_DisableUSB(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_USB_SetMode(HI_USB_MODE_CHARGE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "SetUsbMode");
}

HI_BOOL PDT_UI_COMM_IsUSBIn(HI_VOID)
{
    return s_bUSB_In;
}

static HI_S32 PDT_UI_COMM_SwitchToUSB(HI_VOID)
{
    HI_S32 s32Ret;
    HI_MESSAGE_S stMessage = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if(HI_PDT_STATEMNG_ENOTINIT == s32Ret)
    {
        MLOGD("statemng not init\n");
        return HI_SUCCESS;
    }
    else
    {
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if(HI_FALSE == stWorkModeState.bStateMngInProgress && HI_FALSE == stWorkModeState.bRunning)
    {
        HI_PLAYBACK_Stop();
        HI_PLAYBACK_Deinit();

        stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
        stMessage.arg2 = s_enSwitchUSBMode;
        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s_enSwitchUSBMode = HI_PDT_WORKMODE_BUTT;

        return HI_SUCCESS;
    }
    else if(HI_FALSE == stWorkModeState.bStateMngInProgress && HI_TRUE  == stWorkModeState.bRunning)
    {
        if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_LOOP_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_LPSE_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_SLOW_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_RECSNAP == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_LPSE_PHOTO == stWorkModeState.enWorkMode)
        {
            stMessage.what = HI_EVENT_STATEMNG_STOP;
            stMessage.arg2 = stWorkModeState.enWorkMode;
            s32Ret =  HI_PDT_STATEMNG_SendMessage(&stMessage);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            return HI_SUCCESS;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_COMM_SwitchToHDMI(HI_VOID)
{
    HI_S32 s32Ret;
    HI_MESSAGE_S stMessage = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if(HI_PDT_STATEMNG_ENOTINIT == s32Ret)
    {
        MLOGD("statemng not init\n");
        return HI_SUCCESS;
    }
    else
    {
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if(HI_PDT_WORKMODE_HDMI_PREVIEW == stWorkModeState.enWorkMode && UI_OUTPUT_HDMI != s_enUiState)
    {
        s32Ret = PDT_UI_StartHDMI();
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

        s_enUiState = UI_OUTPUT_HDMI;
    }
    else if(HI_FALSE == stWorkModeState.bStateMngInProgress && HI_FALSE == stWorkModeState.bRunning)
    {
        PDT_UI_DeinitHigv();

        HI_PLAYBACK_Deinit();
        stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
        stMessage.arg2 = HI_PDT_WORKMODE_HDMI_PREVIEW;

        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s_enUiState = UI_OUTPUT_SWITCHING_TO_HDMI;

        return HI_SUCCESS;
    }
    else if(HI_FALSE == stWorkModeState.bStateMngInProgress && HI_TRUE  == stWorkModeState.bRunning)
    {
        if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_LOOP_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_LPSE_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_SLOW_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_RECSNAP == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_LPSE_PHOTO == stWorkModeState.enWorkMode)
        {
            stMessage.what = HI_EVENT_STATEMNG_STOP;
            stMessage.arg2 = stWorkModeState.enWorkMode;
            s32Ret =  HI_PDT_STATEMNG_SendMessage(&stMessage);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            return HI_SUCCESS;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_COMM_SwitchToLCD(HI_VOID)
{
    HI_S32 s32Ret;
    HI_MESSAGE_S stMessage = {};
    HI_PDT_WORKMODE_E enWorkMode = HI_PDT_WORKMODE_BUTT;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if(HI_PDT_STATEMNG_ENOTINIT == s32Ret)
    {
        MLOGD("statemng not init\n");
        return HI_SUCCESS;
    }
    else
    {
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if(HI_PDT_WORKMODE_HDMI_PREVIEW != stWorkModeState.enWorkMode
        && HI_PDT_WORKMODE_HDMI_PLAYBACK != stWorkModeState.enWorkMode
        && UI_OUTPUT_LCD != s_enUiState)
    {
        s32Ret = PDT_UI_StartLCD();
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

        s_enUiState = UI_OUTPUT_LCD;
    }
    else if(HI_FALSE == stWorkModeState.bStateMngInProgress && HI_FALSE == stWorkModeState.bRunning)
    {
        HI_PLAYBACK_Deinit();
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enWorkMode);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

        stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
        stMessage.arg2 = enWorkMode;
        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

        s_enUiState = UI_OUTPUT_SWITCHING_TO_LCD;

        return HI_SUCCESS;
    }

    return HI_SUCCESS;
}

HI_VOID PDT_UI_COMM_StartWindow(HIGV_HANDLE WinHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(!PDT_UI_WINMNG_WindowIsShow(WinHdl))
    {
        s32Ret = PDT_UI_WINMNG_StartWindow(WinHdl, HI_FALSE);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "StartWindow");
    }
}

HI_VOID PDT_UI_COMM_FinishWindow(HIGV_HANDLE WinHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(PDT_UI_WINMNG_WindowIsShow(WinHdl))
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(WinHdl);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "FinishWindow");
    }
}

HI_S32 PDT_UI_COMM_GetAVHdl(HI_PDT_WORKMODE_E enWorkMode, HI_HANDLE* pAoHdl, HI_HANDLE* pAoChnHdl, HI_HANDLE* pVdispHdl, HI_HANDLE* pWndHdl)
{
    HI_S32 s32Ret;
    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE, &enMediaMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    HI_PDT_MEDIA_CFG_S stMediaCfg = {};
    HI_PDT_SCENE_MODE_S stSceneMode = {};
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enWorkMode, enMediaMode, &stMediaCfg, &stSceneMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if (pAoHdl)
    {
        *pAoHdl = stMediaCfg.stAudioOutCfg.astAoCfg->AoHdl;
        MLOGI(YELLOW"AoHdl[%d]\n"NONE, *pAoHdl);
    }

    if (pAoChnHdl)
    {
        *pAoChnHdl = stMediaCfg.stAudioOutCfg.astAoCfg->AoChnHdl;
        MLOGI(YELLOW"pAoChnHdl[%d]\n"NONE, *pAoChnHdl);
    }

    if (pVdispHdl)
    {
        *pVdispHdl = stMediaCfg.stVideoOutCfg.astDispCfg->VdispHdl;
        MLOGI(YELLOW"pVdispHdl[%d]\n"NONE, *pVdispHdl);
    }

    if (pWndHdl)
    {
        *pWndHdl = stMediaCfg.stVideoOutCfg.astDispCfg->astWndCfg->WndHdl;
    }
    return HI_SUCCESS;
}

/*Get execute time since started, return by Second */
HI_U64 PDT_UI_COMM_GetExecTime(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_EVENT_S stEvent = {};
    struct timespec stCurTime = {};
    HI_U64 u64ExecTime = 0;

    s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STATEMNG_START, &stEvent);
    if(HI_SUCCESS != s32Ret)
    {
        return 0;
    }

    clock_gettime(CLOCK_MONOTONIC, &stCurTime);
    u64ExecTime = (stCurTime.tv_sec*1000 + stCurTime.tv_nsec/1000000) - stEvent.u64CreateTime;
    u64ExecTime = u64ExecTime/1000 + (u64ExecTime%1000 > 500 ? 1 : 0);

    return u64ExecTime;
}
/*Get remaining time of DELAY_PHOTO mode since started,  return by Second */
HI_U64 PDT_UI_COMM_GetRemainDelayTime(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_U64 u64ExecTime = 0;
    HI_U32 u32DelayTime = 0;
    HI_U64 u64DelayTime = 0;
    HI_U64 u64RemainTime = 0;
    HI_EVENT_S stEvent = {};
    struct timespec stCurTime = {};

    s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STATEMNG_START, &stEvent);
    HI_APPCOMM_CHECK_RETURN(s32Ret, 0);

    clock_gettime(CLOCK_MONOTONIC, &stCurTime);
    u64ExecTime = (stCurTime.tv_sec*1000 + stCurTime.tv_nsec/1000000) - stEvent.u64CreateTime;

    s32Ret = HI_PDT_PARAM_GetWorkModeParam(HI_PDT_WORKMODE_DLAY_PHOTO, HI_PDT_PARAM_TYPE_DELAY_TIME, &u32DelayTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret, 0);

    u64DelayTime = (HI_U64)u32DelayTime*1000;  /*second to microsecond*/

    if(u64DelayTime > u64ExecTime)
    {
        u64RemainTime = u64DelayTime - u64ExecTime;
        u64RemainTime = u64RemainTime/1000 + (u64RemainTime%1000 > 500 ? 1 : 0);
        MLOGD("Delay:%llu ms, ExecTime: %llu ms, RemainTime = %llu s\n",u64DelayTime, u64ExecTime, u64RemainTime);
    }
    else
    {
        u64RemainTime = 0;
    }

    return u64RemainTime;
}

HI_S32 PDT_UI_COMM_UpdateTotalBitRate(HI_VOID)
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_U32 u32VencIdx;
    HI_HANDLE aVencHdl[3]= {PDT_UI_MAIN_STREAM_VENC_HDL, PDT_UI_SUB_STREAM_VENC_HDL, PDT_UI_THM_VENC_HANDLE};
    HI_MEDIA_VENC_ATTR_S stVencAttr = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {.enWorkMode = HI_PDT_WORKMODE_NORM_REC,};

    s_u32TotalBitRate = 0;

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
    if(s32Ret != HI_SUCCESS)
    {
        return HI_SUCCESS; /** GetState failed, skip refresh home */
    }

    switch(stWorkState.enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_RECSNAP:
            for(u32VencIdx = 0; u32VencIdx < 2; u32VencIdx++)
            {
                s32Ret = HI_PDT_MEDIA_GetVencAttr(aVencHdl[u32VencIdx], &stVencAttr);
                if(s32Ret != HI_SUCCESS)
                {
                    return HI_SUCCESS; /** GetVencAttr failed, skip refresh home */
                }

                switch(stVencAttr.stTypeAttr.enType)
                {
                    case HI_MAPI_PAYLOAD_TYPE_H264:
                        if(HI_MAPI_VENC_RC_MODE_CBR == stVencAttr.stRcAttr.enRcMode)
                        {
                            s_u32TotalBitRate += stVencAttr.stRcAttr.unAttr.stH264Cbr.stAttr.u32BitRate;
                        }
                        else if(HI_MAPI_VENC_RC_MODE_VBR == stVencAttr.stRcAttr.enRcMode)
                        {
                            s_u32TotalBitRate += stVencAttr.stRcAttr.unAttr.stH264Vbr.stAttr.u32MaxBitRate;
                        }
                        break;

                    case HI_MAPI_PAYLOAD_TYPE_H265:
                        if(HI_MAPI_VENC_RC_MODE_CBR == stVencAttr.stRcAttr.enRcMode)
                        {
                            s_u32TotalBitRate += stVencAttr.stRcAttr.unAttr.stH265Cbr.stAttr.u32BitRate;
                        }
                        else if(HI_MAPI_VENC_RC_MODE_VBR == stVencAttr.stRcAttr.enRcMode)
                        {
                            s_u32TotalBitRate += stVencAttr.stRcAttr.unAttr.stH265Vbr.stAttr.u32MaxBitRate;
                        }
                        break;

                    default:
                        break;
                }
            }
            s_u32TotalBitRate = s_u32TotalBitRate << 10;
            break;

        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            s32Ret = HI_PDT_MEDIA_GetVencAttr(PDT_UI_PHOTO_VENC_HDL, &stVencAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s_u32TotalBitRate = stVencAttr.stTypeAttr.u32Width * stVencAttr.stTypeAttr.u32Height * 3 / 2 * 8 / 7;
            break;

        case HI_PDT_WORKMODE_BURST:
            s32Ret = HI_PDT_MEDIA_GetVencAttr(PDT_UI_PHOTO_VENC_HDL, &stVencAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s_u32TotalBitRate = stVencAttr.stTypeAttr.u32Width * stVencAttr.stTypeAttr.u32Height * 3 / 2 * 8 / 12;
            break;

        default:
            break;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_GetRemainPhotoCount(HI_U32 *pu32RemainCount)
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;
    HI_STORAGE_FS_INFO_S stFsInfo = {};

    s32Ret = PDT_UI_COMM_GetStorageState(&enStorageState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(HI_STORAGE_STATE_MOUNTED != enStorageState)
    {
        *pu32RemainCount = 0;
        return HI_SUCCESS;
    }

    s32Ret = HI_STORAGEMNG_GetFSInfo(s_stStorageCfg.szMntPath, &stFsInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if(0 == s_u32TotalBitRate)
    {
        s32Ret = PDT_UI_COMM_UpdateTotalBitRate();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if(0 == s_u32TotalBitRate)
    {
        *pu32RemainCount = 0;
        return HI_SUCCESS;
    }

    if(PDT_UI_COMM_SdIsFull())
    {
        *pu32RemainCount = 0;
        return HI_SUCCESS;
    }
    *pu32RemainCount = (HI_U32)(8 * (stFsInfo.u64AvailableSize - (((HI_U64)s_u32WarningStage) << 20)) / ((HI_U64)s_u32TotalBitRate));

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_GetRemainRecordTime(HI_U32 *pu32RemainTime)
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;
    HI_STORAGE_FS_INFO_S stFsInfo = {};

    s32Ret = PDT_UI_COMM_GetStorageState(&enStorageState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(HI_STORAGE_STATE_MOUNTED != enStorageState)
    {
        *pu32RemainTime = 0;
        return HI_SUCCESS;
    }

    s32Ret = HI_STORAGEMNG_GetFSInfo(s_stStorageCfg.szMntPath, &stFsInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if(0 == s_u32TotalBitRate)
    {
        s32Ret = PDT_UI_COMM_UpdateTotalBitRate();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if(0 == s_u32TotalBitRate)
    {
        *pu32RemainTime = 0;
        return HI_SUCCESS;
    }

    if(PDT_UI_COMM_SdIsFull())
    {
        *pu32RemainTime = 0;
        return HI_SUCCESS;
    }

    *pu32RemainTime = (HI_U32)(8 * (stFsInfo.u64AvailableSize - (((HI_U64)s_u32WarningStage) << 20)) / ((HI_U64)s_u32TotalBitRate));

    return HI_SUCCESS;
}

HI_VOID PDT_UI_COMM_Second2String(HI_U64 second, HI_CHAR * pszBuffer, HI_U32 size)
{
    if (NULL == pszBuffer)
    {
        MLOGE("null pointer\n");
        return;
    }

    HI_U64 hour, min;

    hour = second/3600;
    min = (second%3600)/60;
    second = second%60;
    snprintf(pszBuffer, size, "%02llu:%02llu:%02llu", hour, min, second);
}

/*Get current date and time*/
HI_S32 PDT_UI_COMM_GetDateTime(PDT_UI_DATETIME_S * pstDateTime)
{
    time_t now = 0;
    struct tm result = {};
    HI_APPCOMM_CHECK_POINTER(pstDateTime, -1);

    now = time(NULL);
    if(NULL == localtime_r(&now, &result) )
    {
        MLOGE("localtime_r failed\n");
        return HI_FAILURE;
    }

    pstDateTime->year = result.tm_year + 1900;
    pstDateTime->month = result.tm_mon + 1;
    pstDateTime->day = result.tm_mday;
    pstDateTime->hour = result.tm_hour;
    pstDateTime->minute = result.tm_min;
    MLOGD("%u-%02u-%02u %02u:%02u\n", pstDateTime->year, pstDateTime->month, pstDateTime->day, pstDateTime->hour, pstDateTime->minute);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_SetDateTime(PDT_UI_DATETIME_S * pstDateTime)
{
    HI_APPCOMM_CHECK_POINTER(pstDateTime, -1);

    HI_S32 s32Ret;
    HI_SYSTEM_TM_S stDateTime;
    stDateTime.s32year = (HI_S32)pstDateTime->year;
    stDateTime.s32mon = (HI_S32)pstDateTime->month;
    stDateTime.s32mday = (HI_S32)pstDateTime->day;
    stDateTime.s32hour = (HI_S32)pstDateTime->hour;
    stDateTime.s32min = (HI_S32)pstDateTime->minute;
    stDateTime.s32sec = 0;

    s32Ret = HI_SYSTEM_SetDateTime(&stDateTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_GetDateString(PDT_UI_DATETIME_S * pstDateTime, HI_CHAR * pszBuffer, HI_U32 size)
{
    HI_APPCOMM_CHECK_POINTER(pstDateTime, -1);
    HI_APPCOMM_CHECK_POINTER(pszBuffer, -1);
    snprintf(pszBuffer, size, "%u-%u-%u", pstDateTime->year, pstDateTime->month, pstDateTime->day);

    return HI_SUCCESS;
}
HI_S32 PDT_UI_COMM_GetTimeString(PDT_UI_DATETIME_S * pstDateTime, HI_CHAR * pszBuffer, HI_U32 size)
{
    HI_APPCOMM_CHECK_POINTER(pstDateTime, -1);
    HI_APPCOMM_CHECK_POINTER(pszBuffer, -1);
    snprintf(pszBuffer, size, "%02u:%02u", pstDateTime->hour, pstDateTime->minute);

    return HI_SUCCESS;
}

/** wrapper for HI_GV_Lan_GetLangString */
HI_S32 PDT_UI_COMM_GetStringByID(const HI_U32 u32StrID, HI_CHAR* pszBuffer, HI_U32 u32Size)
{
    HI_S32 s32Ret = 0;
    HI_CHAR * pszLangID = NULL;
    HI_CHAR * pszContent = NULL;

    HI_APPCOMM_CHECK_POINTER(pszBuffer, -1);

    s32Ret = HI_GV_Lan_GetCurLangID( &pszLangID );
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_GV_Lan_GetLangString(pszLangID, u32StrID, &pszContent);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    snprintf(pszBuffer, u32Size, "%s", pszContent);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_GetStringOfMode(const HI_PDT_WORKMODE_E enWorkMode, HI_CHAR* pszBuffer, HI_U32 u32Size)
{
    HI_S32 s32Ret = 0;
    HI_S32 i = 0;
    HI_S32 aModeStr[][2] =
    {
        {HI_PDT_WORKMODE_NORM_REC,   STRING_MODE_NORM_REC},
        {HI_PDT_WORKMODE_LOOP_REC,   STRING_MODE_LOOP_REC},
        {HI_PDT_WORKMODE_LPSE_REC,   STRING_MODE_LAPSE_REC},
        {HI_PDT_WORKMODE_SLOW_REC,   STRING_MODE_SLOW_REC},
        {HI_PDT_WORKMODE_RECSNAP,    STRING_MODE_RECSNAP},
        {HI_PDT_WORKMODE_SING_PHOTO, STRING_MODE_SINGLE_PHOTO},
        {HI_PDT_WORKMODE_DLAY_PHOTO, STRING_MODE_DELAY_PHOTO},
        {HI_PDT_WORKMODE_LPSE_PHOTO, STRING_MODE_LAPSE_PHOTO},
        {HI_PDT_WORKMODE_BURST,      STRING_MODE_BURST},
    };

    HI_APPCOMM_CHECK_POINTER(pszBuffer, -1);

    for(i=0; i<ARRAY_SIZE(aModeStr); i++)
    {
        if(enWorkMode == aModeStr[i][0])
        {
            s32Ret = PDT_UI_COMM_GetStringByID(aModeStr[i][1], pszBuffer, u32Size);
            return s32Ret;
        }
    }
    MLOGE("not found string of workmode %d\n", enWorkMode);

    return HI_FAILURE;
}

typedef struct tagPDT_UI_SRC_ID_S
{
    HI_CHAR szContent[SETTING_CONTENT_SIZE];
    HI_U32 u32StringId;
} PDT_UI_SRC_ID_S;

static const PDT_UI_SRC_ID_S s_astSrcId[] = {
    {"AUTO",      STRING_AUTO},
    {"LL",        STRING_LL},
    {"Center",    STRING_CENTER},
    {"Average",   STRING_AVERAGE},
    {"Spot",      STRING_SPOT},
    {"CHARGE",    STRING_USB_CHARGE},
    {"UVC",       STRING_USB_UVC},
    {"STORAGE",   STRING_USB_SRORAGE},
    {"en",        STRING_EN},
    {"zh",        STRING_ZH},
    {"IDLE",      STRING_IDLE},
    {"NORMREC",   STRING_MODE_NORM_REC},
    {"LOOPREC",   STRING_MODE_LOOP_REC},
    {"LPSEREC",   STRING_MODE_LAPSE_REC},
    {"SLOWREC",   STRING_MODE_SLOW_REC},
    {"RECSNAP",   STRING_MODE_RECSNAP},
    {"LPSEPHOTO", STRING_MODE_LAPSE_PHOTO},
    {"SINGLE",    STRING_MODE_SINGLE_PHOTO},
    {"OFF",       STRING_OFF},
    {"1MIN",      STRING_1MIN},
    {"3MIN",      STRING_3MIN},
    {"5MIN",      STRING_5MIN},
    {"10MIN",     STRING_10MIN},
    {"0.5S",      STRING_500MS},
    {"1S",        STRING_1S},
    {"2S",        STRING_2S},
    {"3S",        STRING_3S},
    {"5S",        STRING_5S},
    {"10S",       STRING_10S},
    {"20S",       STRING_20S},
    {"30S",       STRING_30S},
    {"60S",       STRING_60S},
    {"3000K",     STRING_3000K},
    {"6500K",     STRING_6500K},
};

HI_S32 PDT_UI_COMM_GetStringByString(const HI_CHAR* pszSrcBuffer, HI_CHAR* pszDestBuffer, HI_U32 u32Size)
{
    HI_U32 u32Index;
    HI_S32 s32Ret;
    HI_CHAR szContent[SETTING_CONTENT_SIZE];

    for (u32Index = 0; u32Index < sizeof(s_astSrcId) / sizeof(PDT_UI_SRC_ID_S); ++u32Index)
    {
        s32Ret = PDT_UI_COMM_GetStringByID(s_astSrcId[u32Index].u32StringId, szContent, u32Size);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        if(0 == strncmp(szContent, pszSrcBuffer, u32Size))
        {
            snprintf(pszDestBuffer, u32Size, "%s", s_astSrcId[u32Index].szContent);
            return HI_SUCCESS;
        }
    }
    return HI_FAILURE;
}

HI_S32 PDT_UI_COMM_Translate(const HI_CHAR* pszSrcBuffer, HI_CHAR* pszDestBuffer, HI_U32 u32Size)
{
    HI_U32 u32Index;
    HI_S32 s32Ret;

    for (u32Index = 0; u32Index < sizeof(s_astSrcId) / sizeof(PDT_UI_SRC_ID_S); ++u32Index)
    {
        if(0 == strncmp(s_astSrcId[u32Index].szContent, pszSrcBuffer, u32Size))
        {
            s32Ret = PDT_UI_COMM_GetStringByID(s_astSrcId[u32Index].u32StringId, pszDestBuffer, u32Size);
            return s32Ret;
        }
    }
    return HI_FAILURE;
}

static HI_S32 PDT_UI_COMM_GetDescFromValueSet(HI_S32 s32Value, const HI_PDT_ITEM_VALUESET_S * pstValueSet,
    HI_CHAR* pszBuffer, HI_U32 u32Size)
{
    HI_U32 i;
    HI_S32 s32Ret;
    HI_APPCOMM_CHECK_POINTER(pstValueSet, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pszBuffer,   HI_PDT_PARAM_EINVAL);

    for(i=0; i<pstValueSet->s32Cnt && i<HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
    {
        if(s32Value == pstValueSet->astValues[i].s32Value)
        {
            s32Ret = PDT_UI_COMM_Translate(pstValueSet->astValues[i].szDesc, pszBuffer, u32Size);
            if(HI_SUCCESS != s32Ret)
            {
                snprintf(pszBuffer, u32Size, "%s", pstValueSet->astValues[i].szDesc);
            }
            return HI_SUCCESS;
        }
    }
    return HI_FAILURE;
}


/** Get  param type's  current value(string)  */
HI_S32 PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_TYPE_E enType,
    HI_CHAR* pszBuffer, HI_U32 u32Size)
{
    HI_S32 s32Value = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_ITEM_VALUESET_S stValueSet = {};
    HI_TIMEDTASK_ATTR_S stTimedTaskAttr = {};

    HI_APPCOMM_CHECK_POINTER(pszBuffer, HI_PDT_PARAM_EINVAL);

    switch(enType)
    {
        /*workmode dependent, have value set*/
        case HI_PDT_PARAM_TYPE_DELAY_TIME:
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:

            s32Ret = HI_PDT_PARAM_GetWorkModeParam(enWorkMode, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = HI_PDT_PARAM_GetWorkModeItemValues(enWorkMode, enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = PDT_UI_COMM_GetDescFromValueSet(s32Value, &stValueSet, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_PDT_PARAM_TYPE_AUDIO:
        case HI_PDT_PARAM_TYPE_FLIP:
        case HI_PDT_PARAM_TYPE_DIS:
        case HI_PDT_PARAM_TYPE_LDC:
        case HI_PDT_PARAM_TYPE_OSD:
            s32Ret = HI_PDT_PARAM_GetWorkModeParam(enWorkMode, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = PDT_UI_COMM_GetStringByID( (HI_BOOL)s32Value ? STRING_ON : STRING_OFF, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = HI_PDT_PARAM_GetWorkModeParam(enWorkMode, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = PDT_UI_COMM_GetStringByID(
                (HI_MAPI_PAYLOAD_TYPE_H264 == (HI_MAPI_PAYLOAD_TYPE_E)s32Value) ? STRING_H264 : STRING_H265,
                    pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        /*items below are workmode independent*/
        case HI_PDT_PARAM_TYPE_VOLUME:
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            snprintf(pszBuffer, u32Size, "%d", s32Value);
            break;

        case HI_PDT_PARAM_TYPE_KEYTONE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = PDT_UI_COMM_GetStringByID( (HI_BOOL)s32Value ? STRING_ON : STRING_OFF, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_PDT_PARAM_TYPE_POWERON_ACTION:
        case HI_PDT_PARAM_TYPE_USB_MODE:
        case HI_PDT_PARAM_TYPE_LANGUAGE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = PDT_UI_COMM_GetDescFromValueSet(s32Value, &stValueSet, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE:
        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &stTimedTaskAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            if( !stTimedTaskAttr.bEnable )
            {
                s32Ret = PDT_UI_COMM_GetStringByID(STRING_OFF, pszBuffer, u32Size);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                return HI_SUCCESS;
            }
            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = PDT_UI_COMM_GetDescFromValueSet(stTimedTaskAttr.u32Time_sec, &stValueSet, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_PDT_PARAM_TYPE_WIFI_AP:
        case HI_PDT_PARAM_TYPE_DEV_INFO:
        case HI_PDT_PARAM_TYPE_POWERON_WORKMODE:
        default:
            MLOGE("Unprocessed ParamType[%d]\n", enType);
            return HI_FAILURE;
    }

    return  HI_SUCCESS;
}

HI_VOID PDT_UI_COMM_ShowAlarm(HIGV_HANDLE WinHdl, HIGV_HANDLE TitleHdl, HI_U32 u32StringID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_UI_COMM_StartWindow(WinHdl);
    s32Ret = HI_GV_Widget_SetTextByID(TitleHdl, u32StringID);
    s32Ret |= HI_GV_Widget_Paint(WinHdl, 0);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "SetText or Paint");
}

HI_S32 PDT_UI_COMM_ActiveNextWidget(HIGV_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HIGV_HANDLE CurWidgetHdl)
{
    HI_U32 u32NextIndex = 0;
    HI_U32 u32CurIndex = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    for (u32CurIndex = 0;  u32CurIndex < u32ArraySize; u32CurIndex++)
    {
        if (CurWidgetHdl == aWidgetHdl[u32CurIndex])
        {
            u32NextIndex = (u32CurIndex + 1) % u32ArraySize;
            s32Ret = HI_GV_Widget_Active( aWidgetHdl[u32NextIndex] );
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            return HI_SUCCESS;
        }
    }

    MLOGE("no widget %d\n", CurWidgetHdl);
    return HI_FAILURE;
}

static HI_S32 PDT_UI_COMM_UpdateWarningStage(HI_STORAGE_FS_INFO_S *pstFSInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_FILEMNG_CFG_S stFileMngCfg = {};

    if(0 == s_u32WarningStage)
    {
        s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stFileMngCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

#if defined(CONFIG_FILEMNG_DTCF)
        stFileMngCfg.stDtcfCfg.u32WarningStage =
            pstFSInfo->u64TotalSize * stFileMngCfg.stDtcfCfg.u32WarningStage / 100 >> 20;
        MLOGD(BLUE"DTCF: WarningStage[%u]\n"NONE, stFileMngCfg.stDtcfCfg.u32WarningStage);
        s_u32WarningStage = stFileMngCfg.stDtcfCfg.u32WarningStage;
#else
        stFileMngCfg.stDcfCfg.u32WarningStage =
            pstFSInfo->u64TotalSize * stFileMngCfg.stDcfCfg.u32WarningStage / 100 >> 20;
        MLOGD(BLUE"DCF: WarningStage[%u]\n"NONE,stFileMngCfg.stDcfCfg.u32WarningStage);
        s_u32WarningStage = stFileMngCfg.stDcfCfg.u32WarningStage;
#endif
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_GetFSInfo(HI_STORAGE_FS_INFO_S *pstFSInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(strnlen(s_stStorageCfg.szMntPath, HI_APPCOMM_MAX_PATH_LEN) <= 0)
    {
        s32Ret = HI_PDT_PARAM_GetStorageCfg(&s_stStorageCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret,HI_FALSE);
    }

    s32Ret = HI_STORAGEMNG_GetFSInfo(s_stStorageCfg.szMntPath, pstFSInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_GetStorageState(HI_STORAGE_STATE_E* penStorageState)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(strnlen(s_stStorageCfg.szMntPath, HI_APPCOMM_MAX_PATH_LEN) <= 0)
    {
        s32Ret = HI_PDT_PARAM_GetStorageCfg(&s_stStorageCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    s32Ret = HI_STORAGEMNG_GetState(s_stStorageCfg.szMntPath, penStorageState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

HI_BOOL PDT_UI_COMM_SdIsReady(HI_VOID)
{
    if(HI_TRUE == s_bSDAvailable)
    {
        return HI_TRUE;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32StringID = 0;

    HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;
    HI_EVENT_S stEvent= {};

    s32Ret = PDT_UI_COMM_GetStorageState(&enStorageState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);

    switch (enStorageState)
    {
        case HI_STORAGE_STATE_DEV_UNPLUGGED:
            u32StringID = STRING_NO_SD_CARD;
            break;

        case HI_STORAGE_STATE_DEV_ERROR:
            u32StringID = STRING_SD_ERROR_PLUG;
            break;

        case HI_STORAGE_STATE_FS_CHECK_FAILED:
            s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_FS_CHECK_FAILED, &stEvent);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);
            switch(stEvent.arg1)
            {
                case HI_STORAGE_FSTOOL_ERR_NOT_ENOUGH_MEMORY:
                case HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT:
                case HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION:
                    u32StringID = STRING_SD_ERROR_FORMAT;
                    break;

                case HI_STORAGE_FSTOOL_ERR_OPEN_FAIL:
                case HI_STORAGE_FSTOOL_ERR_READ_FAIL:
                case HI_STORAGE_FSTOOL_ERR_WRITE_FAIL:
                case HI_STORAGE_FSTOOL_ERR_ILLEGAL_PARAM:
                case HI_STORAGE_FSTOOL_ERR_SYSTEM_CALL_FAILURE:
                    u32StringID = STRING_SD_CHANGE;
                    break;

                default:
                    u32StringID = STRING_SD_ERROR;
                    break;
            }
            break;

        case HI_STORAGE_STATE_FS_EXCEPTION:
            s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_FS_EXCEPTION, &stEvent);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);
            switch(stEvent.arg1)
            {
                case HI_STORAGE_FSTOOL_ERR_FS_EXCEPTION:
                case HI_STORAGE_FSTOOL_ERR_FS_NOT_64K_CLUSTER:
                    u32StringID = STRING_SD_ERROR_FORMAT;
                    break;

                default:
                    u32StringID = STRING_SD_ERROR;
                    break;
            }
            break;

        case HI_STORAGE_STATE_MOUNTED:
            s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_MOUNTED, &stEvent);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);

            if(HI_TRUE == s_bSDAvailable)
            {
                return HI_TRUE;
            }
            else
            {
                s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_FILEMNG_UNIDENTIFICATION, &stEvent);
                if(HI_SUCCESS == s32Ret) // the number of unidentified files beyond limited number
                {
                    u32StringID = STRING_SD_ERROR_FORMAT;
                    break;
                }
                else
                {
                    return HI_TRUE;
                }
            }

            break;

        case HI_STORAGE_STATE_MOUNT_FAILED:
            u32StringID = STRING_SD_CHANGE;
            break;

        case HI_STORAGE_STATE_DEV_CONNECTING:
        case HI_STORAGE_STATE_FS_CHECKING:
            u32StringID = STRING_SD_PREPARING;
            break;

        default:
            u32StringID = STRING_SD_ERROR;
            break;
    }

    if(UI_OUTPUT_LCD == s_enUiState || UI_OUTPUT_SWITCHING_TO_LCD == s_enUiState)
    {
        PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, u32StringID);
    }
    else if(UI_OUTPUT_HDMI == s_enUiState || UI_OUTPUT_SWITCHING_TO_HDMI == s_enUiState)
    {
        if(PDT_UI_WINMNG_WindowIsShow(HDMI_HOME))
        {
            s32Ret = HI_GV_Widget_SetTextByID(HDMI_HOME_NOTIC, u32StringID);
            s32Ret |= HI_GV_Widget_Paint(HDMI_HOME, NULL);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "SetText");
        }
        else
        {
            PDT_UI_COMM_ShowAlarm(HDMI_ALARM_WINDOW, HDMI_ALARM_WINDOW_LABEL_INFO, u32StringID);
        }
    }

    return HI_FALSE;
}

static HI_S32 PDT_UI_COMM_OnReceiveMsgResult(HI_EVENT_S * pstEvent)
{
    return HI_SUCCESS;
}

HI_BOOL PDT_UI_COMM_SdIsFull(HI_VOID)
{
    HI_S32 s32Ret;
    HI_STORAGE_FS_INFO_S stFsInfo = {};

    s32Ret = PDT_UI_COMM_GetFSInfo(&stFsInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);

    s32Ret = PDT_UI_COMM_UpdateWarningStage(&stFsInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);

    if((stFsInfo.u64AvailableSize >> 20) < s_u32WarningStage)
    {
        HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
        s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
        HI_MESSAGE_S stMessage = {};
        if(stWorkModeState.bRunning && (HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_LOOP_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_LPSE_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_SLOW_REC == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_RECSNAP == stWorkModeState.enWorkMode))
        {
            stMessage.what = HI_EVENT_STATEMNG_STOP;
            stMessage.arg2 = stWorkModeState.enWorkMode;
            s32Ret =  PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_COMM_OnReceiveMsgResult);
            if(s32Ret)
            {
                MLOGE("s32Ret %x\n", s32Ret);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
            }
        }

        return HI_TRUE;
    }

    return HI_FALSE;
}

/* process the callback which is given to PDT_UI_COMM_SendAsyncMsg */
static HI_VOID PDT_UI_COMM_ProcessMsgResult(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_MUTEX_LOCK(s_stMessageCtx.MsgMutex);
    if (!s_stMessageCtx.bMsgProcessed)
    {
        if((s_stMessageCtx.stMsg.what == pstEvent->EventID)
            && (s_stMessageCtx.stMsg.arg1== pstEvent->arg1)
            && (s_stMessageCtx.stMsg.arg2== pstEvent->arg2))
        {
            s32Ret = s_stMessageCtx.pfnMsgResultProc(pstEvent);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("pfnMsgResultProc() Error:%#x\n", s32Ret);
            }
            s_stMessageCtx.bMsgProcessed = HI_TRUE;
        }
    }
    HI_MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);
}

HI_VOID PDT_UI_COMM_SDEventProc(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32StringID = 0;

    if(HI_EVENT_STATEMNG_SD_AVAILABLE == s_LastSDEventID
        && HI_EVENT_STORAGEMNG_DEV_CONNECTING == pstEvent->EventID)
    {
        s_LastSDEventID = pstEvent->EventID;
        return;
    }

    s_LastSDEventID = pstEvent->EventID;
    s_bSDAvailable = HI_FALSE;

    switch(pstEvent->EventID)
    {
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            u32StringID = STRING_NO_SD_CARD;
            break;

        case HI_EVENT_STORAGEMNG_DEV_ERROR:
            u32StringID = STRING_SD_ERROR_PLUG;
            break;

        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
            switch(pstEvent->arg1)
            {
                case HI_STORAGE_FSTOOL_ERR_NOT_ENOUGH_MEMORY:
                case HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT:
                case HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION:
                    u32StringID = STRING_SD_ERROR_FORMAT;
                    break;

                case HI_STORAGE_FSTOOL_ERR_OPEN_FAIL:
                case HI_STORAGE_FSTOOL_ERR_READ_FAIL:
                case HI_STORAGE_FSTOOL_ERR_WRITE_FAIL:
                case HI_STORAGE_FSTOOL_ERR_ILLEGAL_PARAM:
                case HI_STORAGE_FSTOOL_ERR_SYSTEM_CALL_FAILURE:
                    u32StringID = STRING_SD_CHANGE;
                    break;

                default:
                    u32StringID = STRING_SD_ERROR;
                    break;
            }
            break;

        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
            switch(pstEvent->arg1)
            {
                case HI_STORAGE_FSTOOL_ERR_FS_EXCEPTION:
                case HI_STORAGE_FSTOOL_ERR_FS_NOT_64K_CLUSTER:
                    u32StringID = STRING_SD_ERROR_FORMAT;
                    break;

                default:
                    u32StringID = STRING_SD_ERROR;
                    break;
            }
            break;

        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
            u32StringID = STRING_SD_CHANGE;
            break;

        case HI_EVENT_STORAGEMNG_DEV_CONNECTING:
        case HI_EVENT_STORAGEMNG_FS_CHECKING:
            u32StringID = STRING_SD_PREPARING;
            break;

        case HI_EVENT_FILEMNG_SPACE_FULL:
            if(UI_OUTPUT_LCD == s_enUiState)
            {
                u32StringID = (STRING_SD_ERROR_FORMAT == s_s32LastStringID ? 0 : STRING_SD_FULL);
            }
            else if(UI_OUTPUT_HDMI== s_enUiState)
            {
                PDT_UI_COMM_FinishWindow(HDMI_ALARM_WINDOW);
                if(PDT_UI_WINMNG_WindowIsShow(HDMI_HOME))
                {
                    u32StringID = STRING_HDMI_HOME_NOTIC;
                }
            }
            s_bSDAvailable = HI_TRUE;
            break;

        case HI_EVENT_STATEMNG_SD_AVAILABLE:
            if(UI_OUTPUT_LCD == s_enUiState && HI_FALSE == s_bUpgradeSuccess)
            {
                PDT_UI_COMM_FinishWindow(ALARM_WINDOW);
            }
            else if(UI_OUTPUT_HDMI== s_enUiState)
            {
                PDT_UI_COMM_FinishWindow(HDMI_ALARM_WINDOW);
                if(PDT_UI_WINMNG_WindowIsShow(HDMI_HOME))
                {
                    u32StringID = STRING_HDMI_HOME_NOTIC;
                }
            }
            s_bUpgradeSuccess = HI_FALSE;
            s_bSDAvailable = HI_TRUE;
            break;

         case HI_EVENT_STATEMNG_SD_UNAVAILABLE:
            u32StringID = STRING_SD_ERROR;
            break;

        case HI_EVENT_FILEMNG_UNIDENTIFICATION:
            u32StringID = STRING_SD_ERROR_FORMAT;
            break;

        default:
            break;
    }

    s_s32LastStringID = u32StringID;
    if(0 == u32StringID)
    {
        return;
    }

    if(UI_OUTPUT_LCD == s_enUiState)
    {
        PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, u32StringID);
    }
    else if(UI_OUTPUT_HDMI== s_enUiState)
    {
        if(PDT_UI_WINMNG_WindowIsShow(HDMI_HOME))
        {
            s32Ret = HI_GV_Widget_SetTextByID(HDMI_HOME_NOTIC, u32StringID);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "SetText");
            HI_GV_Widget_Paint(HDMI_HOME_NOTIC, HI_NULL);
        }
        else
        {
            PDT_UI_COMM_ShowAlarm(HDMI_ALARM_WINDOW, HDMI_ALARM_WINDOW_LABEL_INFO, u32StringID);
            HI_GV_Widget_SetTextByID(HDMI_HOME_NOTIC, u32StringID);
        }
    }
}


/**called by EVENTHUB module directly*/
HI_S32 PDT_UI_COMM_OnEvent(HI_EVENT_S* pstEvent, HI_VOID* pvArgv)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {};
    HIGV_INPUTEVENT_S stInputEvent = {};
    HI_PDT_WORKMODE_E enWorkMode = HI_PDT_WORKMODE_BUTT;
    HI_PDT_MEDIA_HDMI_STATE_E enState = HI_PDT_MEDIA_HDMI_STATE_BUTT;

    if(HI_EVENT_UI_TOUCH == pstEvent->EventID)
    {
        MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2,pstEvent->s32Result);
    }
    else
    {
        MLOGI("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2,pstEvent->s32Result);
    }

    switch(pstEvent->EventID)
    {
        case HI_EVENT_USB_INSERT:
            s_bUSB_In = HI_TRUE;
            break;

        case HI_EVENT_USB_OUT:
            s_bUSB_In = HI_FALSE;
            break;

        default:
            break;
    }

    /*filter out the failed start and stop, because multi STOP may be launched at the same time*/
    if (!((HI_EVENT_STATEMNG_START  == pstEvent->EventID || HI_EVENT_STATEMNG_STOP == pstEvent->EventID)
        && HI_FALSE == pstEvent->s32Result))
    {
        HI_BOOL bEventContinueHandle  = HI_FALSE;
        s32Ret = PDT_UI_POWERCTRL_PreProcessEvent(pstEvent, &bEventContinueHandle);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        if(!bEventContinueHandle)
        {
            MLOGI("Event %x has been processed by POWERCTRL Module\n", pstEvent->EventID);
            return HI_SUCCESS;
        }
    }

    /** do not respond any event in state of system dormant */
    if(PDT_UI_POWERCTRL_IsSystemDormant())
    {
        return HI_SUCCESS;
    }

    if(HI_EVENT_UI_TOUCH == pstEvent->EventID)
    {
        return HI_SUCCESS;
    }

    switch(pstEvent->EventID)
    {
        case HI_EVENT_KEYMNG_SHORT_CLICK:
        case HI_EVENT_KEYMNG_HOLD_DOWN:
            PDT_UI_COMM_KeyTone();
            stInputEvent.msg = HIGV_MSG_KEYDOWN;
            switch(pstEvent->arg1)
            {
#ifdef CFG_ONEKEY_CONSOLE
                case HI_KEYMNG_KEY_IDX_0:
                    stInputEvent.value = PDT_UI_KEY_ENTER;
                    break;
#else
                case HI_KEYMNG_KEY_IDX_0:
                    stInputEvent.value = PDT_UI_KEY_MENU;
                    break;
                case HI_KEYMNG_KEY_IDX_1:
                    stInputEvent.value = PDT_UI_KEY_BACK;
                    break;
                case HI_KEYMNG_KEY_IDX_2:
                    stInputEvent.value = PDT_UI_KEY_ENTER;
                    break;
#endif
                default:
                    MLOGE("key value %u not processed\n", pstEvent->arg1);
                    return HI_SUCCESS;
            }
            MLOGD("HI_GV_SendInputEvent[ value: %d ]\n", stInputEvent.value);
            s32Ret = HI_GV_SendInputEvent(&stInputEvent);
            if(s32Ret)
            {
                MLOGE("SendInputEvent failed %x\n", s32Ret);
            }
            return HI_SUCCESS;

        case HI_EVENT_KEYMNG_LONG_CLICK:
            stInputEvent.msg = HIGV_MSG_KEYDOWN;
            switch(pstEvent->arg1)
            {
                case HI_KEYMNG_KEY_IDX_0:
                    stMessage.what = HI_EVENT_STATEMNG_POWEROFF;

                    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
                    break;

                case HI_KEYMNG_KEY_IDX_1:
                    PDT_UI_COMM_KeyTone();
                    stInputEvent.msg = HIGV_MSG_KEYDOWN;
                    stInputEvent.value = PDT_UI_HOME_KEY_WIFI;
                    MLOGD("HI_GV_SendInputEvent[ value: %d ]\n", stInputEvent.value);
                    s32Ret = HI_GV_SendInputEvent(&stInputEvent);
                    if(s32Ret)
                    {
                        MLOGE("SendInputEvent failed %x\n", s32Ret);
                    }

                    break;

                case HI_KEYMNG_KEY_IDX_2:
                    break;

                default:
                    return HI_SUCCESS;
            }

            return HI_SUCCESS;

        case HI_EVENT_PDT_MEDIA_HDMI_IN:
            /** not in usb storage or uvc mode */
            if(HI_PDT_WORKMODE_USB_STORAGE != s_enWorkMode && HI_PDT_WORKMODE_UVC != s_enWorkMode
                && UI_OUTPUT_HDMI != s_enUiState && UI_OUTPUT_SWITCHING_TO_HDMI != s_enUiState)
            {
                PDT_UI_COMM_DisableUSB();

                s_enUiState = UI_OUTPUT_PREPARE_SWITCH_TO_HDMI;
                PDT_UI_COMM_DeinitKeyTone();
                s32Ret = HI_GV_SetRefreshCombine(HI_FALSE);
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
                s32Ret = PDT_UI_COMM_SwitchToHDMI();
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            return HI_SUCCESS;

        case HI_EVENT_PDT_MEDIA_HDMI_OUT:
            if(UI_OUTPUT_LCD != s_enUiState && UI_OUTPUT_SWITCHING_TO_LCD != s_enUiState)
            {
                s_enUiState = UI_OUTPUT_PREPARE_SWITCH_TO_LCD;
                PDT_UI_COMM_DeinitKeyTone();
                s32Ret = HI_GV_SetRefreshCombine(HI_FALSE);
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
                PDT_UI_DeinitHigv();
                s32Ret = PDT_UI_COMM_SwitchToLCD();
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                PDT_UI_COMM_EnableUSB();
            }
            return HI_SUCCESS;

        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            s32Ret = HI_PDT_MEDIA_GetHDMIState(&enState);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            if (s_bUIFirstRun)
            {
                /* if LCD UI run for the first time, and HDMI is connected, switch to HDMI*/
                s_bUIFirstRun = HI_FALSE;
                if (HI_PDT_MEDIA_HDMI_STATE_CONNECTTED == enState)
                {
                    if(HI_PDT_WORKMODE_USB_STORAGE != s_enWorkMode && HI_PDT_WORKMODE_UVC != s_enWorkMode
                        && UI_OUTPUT_HDMI != s_enUiState && UI_OUTPUT_SWITCHING_TO_HDMI != s_enUiState)
                    {
                        PDT_UI_COMM_DisableUSB();
                        s_enUiState = UI_OUTPUT_PREPARE_SWITCH_TO_LCD;
                        PDT_UI_COMM_DeinitKeyTone();
                        s32Ret = HI_GV_SetRefreshCombine(HI_FALSE);
                        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
                        PDT_UI_DeinitHigv();

                        MLOGD("HDMI plug in, switch to HDMI mode\n");
                        stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                        stMessage.arg2 = HI_PDT_WORKMODE_HDMI_PREVIEW;

                        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

                        s_enUiState = UI_OUTPUT_SWITCHING_TO_HDMI;
                        MLOGD("s_enUiState = UI_OUTPUT_SWITCHING_TO_HDMI\n");
                    }
                    return HI_SUCCESS;
                }
            }

            if(UI_OUTPUT_SWITCHING_TO_LCD != s_enUiState && UI_OUTPUT_SWITCHING_TO_HDMI != s_enUiState)
            {
                MLOGD("s_enUiState != UI_OUTPUT_SWITCHING\n");
                break;
            }

            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enWorkMode);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            if(HI_PDT_WORKMODE_HDMI_PREVIEW == pstEvent->arg2 && HI_PDT_MEDIA_HDMI_STATE_CONNECTTED == enState)
            {
                MLOGD("Switch to HDMI UI\n");
                s32Ret = PDT_UI_StartHDMI();
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

                s_enUiState = UI_OUTPUT_HDMI;
            }
            else if(enWorkMode == pstEvent->arg2 && HI_PDT_MEDIA_HDMI_STATE_CONNECTTED != enState)
            {
                MLOGD("Switch to LCD UI\n");
                s32Ret = PDT_UI_StartLCD();
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

                s_enUiState = UI_OUTPUT_LCD;
            }
            return HI_SUCCESS;

        case HI_EVENT_UPGRADE_NEWPKT:
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
            if(HI_PDT_WORKMODE_UPGRADE != stWorkState.enWorkMode)
            {
                PDT_UI_COMM_FinishWindow(ALARM_WINDOW);

                s_u32WinCallDialog = STRING_UPGRADE_AT_ONCE;

                s32Ret = HI_GV_Widget_SetTextByID(DIALOG_WINDOW_LABEL_ASK, STRING_UPGRADE_AT_ONCE);
                if(s32Ret)
                {
                    MLOGE("SetTextByID failed. %x\n", s32Ret);
                }

                s32Ret = PDT_UI_WINMNG_StartWindow(DIALOG_WINDOW, HI_FALSE);
            }
            else
            {
                /*apk upgrade*/
                MLOGI("enter board upgrade!\n");
                return HI_SUCCESS;
            }
            break;

        case HI_EVENT_UPGRADE_SUCCESS:
            PDT_UI_COMM_DelUpgradeFiles();
            s_bUpgradeSuccess = HI_TRUE;
            PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, STRING_UPGRADE_SUCCESS);
            break;

        case HI_EVENT_UPGRADE_FAILURE:
            PDT_UI_COMM_DelUpgradeFiles();
            PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, STRING_UPGRADE_FAILURE);
            break;

        case HI_EVENT_STATEMNG_UPGRADE_ABNORMAL:
            PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, STRING_UPGRADE_FAILURE);

            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enWorkMode);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
            stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
            stMessage.arg2 = enWorkMode;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            break;

        case HI_EVENT_USB_OUT:
            s32Ret = HI_PDT_MEDIA_GetHDMIState(&enState);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            if((HI_PDT_WORKMODE_UVC == stWorkState.enWorkMode || HI_PDT_WORKMODE_USB_STORAGE == stWorkState.enWorkMode)
                && (HI_PDT_MEDIA_HDMI_STATE_CONNECTTED == enState))
            {
                s32Ret = HI_USB_SetMode(HI_USB_MODE_CHARGE);

                PDT_UI_COMM_DeinitKeyTone();
                s32Ret = HI_GV_SetRefreshCombine(HI_FALSE);
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

                s_enUiState = UI_OUTPUT_PREPARE_SWITCH_TO_HDMI;
                s32Ret = PDT_UI_COMM_SwitchToHDMI();
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                s_enWorkMode = HI_PDT_WORKMODE_HDMI_PREVIEW;
                return HI_SUCCESS;
            }
            break;

        default:
            break;
    }

    if(UI_OUTPUT_LCD == s_enUiState)
    {
        s32Ret = HI_GV_Msg_SendAsyncWithData(HOME_PAGE, HIGV_MSG_EVENT, (HI_VOID *)pstEvent, sizeof(HI_EVENT_S));
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    else if(UI_OUTPUT_HDMI == s_enUiState)
    {
        s32Ret = HI_GV_Msg_SendAsyncWithData(HDMI_HOME, HIGV_MSG_EVENT, (HI_VOID *)pstEvent, sizeof(HI_EVENT_S));
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    else
    {
        MLOGD("Receive events while switching UI !\n");
    }

    if(HI_EVENT_PDT_MEDIA_HDMI_IN != pstEvent->EventID
        && HI_EVENT_PDT_MEDIA_HDMI_OUT != pstEvent->EventID)
    {
        if(UI_OUTPUT_PREPARE_SWITCH_TO_HDMI == s_enUiState)
        {
            s32Ret = PDT_UI_COMM_SwitchToHDMI();
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "SwitchToHDMI");
        }
        else if(UI_OUTPUT_PREPARE_SWITCH_TO_LCD == s_enUiState)
        {
            s32Ret = PDT_UI_COMM_SwitchToLCD();
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "SwitchToLCD");
        }
    }

    return HI_SUCCESS;
}


/**this is called in UI thread*/
HI_S32 PDT_UI_COMM_LCDEventProc(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMsg = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {};
    HI_PDT_WORKMODE_E enPoweronWorkmode = HI_PDT_WORKMODE_BUTT;
    HI_PDT_MEDIA_HDMI_STATE_E enState = HI_PDT_MEDIA_HDMI_STATE_BUTT;

    HI_APPCOMM_CHECK_POINTER(pstEvent, -1);

    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2, pstEvent->s32Result);

    PDT_UI_COMM_ProcessMsgResult(pstEvent);

    if(HI_EVENT_USB_UVC_READY != pstEvent->EventID && HI_EVENT_USB_STORAGE_READY != pstEvent->EventID
        && HI_PDT_WORKMODE_BUTT != s_enSwitchUSBMode)
    {
        s32Ret = PDT_UI_COMM_SwitchToUSB();
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "reswitchToUSB");
    }

    /*common event process*/
    switch(pstEvent->EventID)
    {
        case HI_EVENT_USB_INSERT:
#if 0
            PDT_UI_WINMNG_StartWindow(USB_CHOICES_WINDOW, HI_TRUE);
#endif
            break;

        case HI_EVENT_USB_UVC_READY:
            MLOGD("UVC PC Ready\n");
            s_enWorkMode = HI_PDT_WORKMODE_UVC;
            s_enSwitchUSBMode = HI_PDT_WORKMODE_UVC;

            PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);

            s32Ret = PDT_UI_COMM_SwitchToUSB();
            if(s32Ret)
            {
                MLOGE("SendMessage failed %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            }
            return HI_SUCCESS;

        case HI_EVENT_USB_STORAGE_READY:
            MLOGD("USB STORAGE PC Ready\n");
            s_enWorkMode = HI_PDT_WORKMODE_USB_STORAGE;
            s_enSwitchUSBMode = HI_PDT_WORKMODE_USB_STORAGE;
            PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);

            s32Ret = PDT_UI_COMM_SwitchToUSB();
            if(s32Ret)
            {
                MLOGE("SendMessage failed %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            }
            return HI_SUCCESS;

        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            if(HI_PDT_WORKMODE_USB_STORAGE == pstEvent->arg2
                || HI_PDT_WORKMODE_UVC == pstEvent->arg2)
            {
                s32Ret = PDT_UI_WINMNG_StartWindow(USB_STATE_WINDOW, HI_TRUE);
                s32Ret |= PDT_UI_WINMNG_FinishWindow(PLAYBACK_PAGE);
                s32Ret |= PDT_UI_WINMNG_FinishWindow(FILELIST_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            else if(HI_PDT_WORKMODE_UPGRADE == pstEvent->arg2)
            {
                PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, STRING_UPGRADE_MODE);
            }
            else
            {
                s32Ret = PDT_UI_WINMNG_FinishWindow(USB_STATE_WINDOW);
                if(s32Ret)
                {
                    MLOGE("Window process failed %x\n", s32Ret);
                }
            }
            break;

        case HI_EVENT_USB_OUT:
            s32Ret = HI_PDT_MEDIA_GetHDMIState(&enState);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enPoweronWorkmode);
            s_enWorkMode = enPoweronWorkmode;

            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            if((HI_PDT_WORKMODE_UVC == stWorkState.enWorkMode || HI_PDT_WORKMODE_USB_STORAGE == stWorkState.enWorkMode)
                && (HI_PDT_MEDIA_HDMI_STATE_CONNECTTED != enState))
            {
                s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
#if 0
                s32Ret |= PDT_UI_WINMNG_FinishWindow(USB_CHOICES_WINDOW);
#endif
                if(s32Ret)
                {
                    MLOGE("Window process failed %x\n", s32Ret);
                }

                stMsg.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                stMsg.arg2 = enPoweronWorkmode;
                s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
                if(s32Ret)
                {
                    MLOGE("SendMessage failed %x\n", s32Ret);
                    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
                }
            }

            break;

        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STORAGEMNG_DEV_CONNECTING:
        case HI_EVENT_STATEMNG_SD_AVAILABLE:
        case HI_EVENT_STATEMNG_SD_UNAVAILABLE:
        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
        case HI_EVENT_STORAGEMNG_MOUNTED:
        case HI_EVENT_FILEMNG_SPACE_FULL:
        case HI_EVENT_FILEMNG_UNIDENTIFICATION:
            PDT_UI_COMM_SDEventProc(pstEvent);
            break;

        case HI_EVENT_STATEMNG_SD_FORMAT:
            break;

        case HI_EVENT_STATEMNG_START:
            if((HI_FALSE == PDT_UI_WINMNG_WindowIsShow(HOME_PAGE)) && (1 == pstEvent->s32Result))
            {
                s32Ret = PDT_UI_WINMNG_StartWindow(HOME_PAGE, HI_TRUE);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "StartHomeWindow");
            }
            break;

        case HI_EVENT_NETCTRL_CLIENT_CONNECTED:
        case HI_EVENT_NETCTRL_CLIENT_DISCONNECTED:
            if(PDT_UI_WINMNG_WindowIsShow(SET_PAGE))
            {
                s32Ret = PDT_UI_WINMNG_StartWindow(HOME_PAGE, HI_TRUE);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "StartHomeWindow");
            }
            break;

        default:
            break;
    }

    /*process specific msg by WINDOW(PAGE) */
    if(PDT_UI_WINMNG_WindowIsShow(HOME_PAGE))
    {
        s32Ret = PDT_UI_HOME_EventProc(pstEvent);
    }

    if(PDT_UI_WINMNG_WindowIsShow(FILELIST_PAGE))
    {
        s32Ret = PDT_UI_FILELIST_OnEventProc(pstEvent, DATAMODEL_FILELIST, FILELIST_BUTTON_BACK);
    }

    if(PDT_UI_WINMNG_WindowIsShow(PLAYBACK_PAGE))
    {
        s32Ret = PDT_UI_COMM_PLAYBACK_OnEventProc(pstEvent, PLAYBACK_PAGE);
    }

    return HI_SUCCESS;
}

/**this is called in UI thread*/
HI_S32 PDT_UI_COMM_HDMIEventProc(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pstEvent, -1);

    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2, pstEvent->s32Result);

    PDT_UI_COMM_ProcessMsgResult(pstEvent);

    /*common event process*/
    switch(pstEvent->EventID)
    {
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STORAGEMNG_DEV_CONNECTING:
        case HI_EVENT_FILEMNG_SPACE_FULL:
        case HI_EVENT_STATEMNG_SD_AVAILABLE:
        case HI_EVENT_STATEMNG_SD_UNAVAILABLE:
        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case HI_EVENT_FILEMNG_UNIDENTIFICATION:
            PDT_UI_COMM_SDEventProc(pstEvent);
            break;

        default:
            break;
    }

    /*process specific msg by WINDOW(PAGE) */
    if(PDT_UI_WINMNG_WindowIsShow(HDMI_HOME))
    {
        s32Ret = PDT_HDMI_HOME_EventProc(pstEvent);
    }
    else if(PDT_UI_WINMNG_WindowIsShow(HDMI_FILELIST))
    {
        s32Ret = PDT_UI_HDMI_FILELIST_OnEventProc(pstEvent, DATAMODEL_HDMI_FILELIST, HDMI_FILELIST_BUTTON_BACK);
    }
    else if(PDT_UI_WINMNG_WindowIsShow(HDMI_PLAYBACK_PAGE))
    {
        s32Ret = PDT_UI_COMM_PLAYBACK_OnEventProc(pstEvent, HDMI_PLAYBACK_PAGE);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_SUCCESS);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_SendAsyncMsg(HI_MESSAGE_S* pstMsg, PDT_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pstMsg, -1);

    HI_MUTEX_LOCK(s_stMessageCtx.MsgMutex);
    if( !s_stMessageCtx.bMsgProcessed)
    {
        MLOGE("Current Msg not finished\n");
        HI_MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);
        return HI_FAILURE;
    }

    s_stMessageCtx.bMsgProcessed = HI_FALSE;
    s_stMessageCtx.stMsg.what = pstMsg->what;
    s_stMessageCtx.stMsg.arg1 = pstMsg->arg1;
    s_stMessageCtx.stMsg.arg2 = pstMsg->arg2;
    memcpy(s_stMessageCtx.stMsg.aszPayload, pstMsg->aszPayload, sizeof(s_stMessageCtx.stMsg.aszPayload));
    s_stMessageCtx.pfnMsgResultProc = pfnMsgResultProc;

    MLOGD("[what:%#x, arg1:%#x, arg2:%#x]\n", pstMsg->what, pstMsg->arg1, pstMsg->arg2);
    s32Ret = HI_PDT_STATEMNG_SendMessage(pstMsg);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Error:%#x\n", s32Ret);
        s_stMessageCtx.bMsgProcessed = HI_TRUE;
        HI_MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);
        return HI_FAILURE;
    }

    HI_MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);
    return HI_SUCCESS;
}


HI_S32 PDT_UI_COMM_SubscribeEvents(HI_VOID)
{
    static HI_BOOL bSubscribeFinish = HI_FALSE;
    HI_U32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_SUBSCRIBER_S stSubscriber = {"UI", PDT_UI_COMM_OnEvent, NULL, HI_FALSE};
    HI_MW_PTR SubscriberHdl = NULL;
    HI_EVENT_ID aEvents[] =
    {
        HI_EVENT_KEYMNG_SHORT_CLICK,
        HI_EVENT_KEYMNG_LONG_CLICK,
        HI_EVENT_KEYMNG_HOLD_DOWN,
        HI_EVENT_KEYMNG_HOLD_UP,
        HI_EVENT_KEYMNG_GROUP,

        HI_EVENT_STATEMNG_SWITCH_WORKMODE,
        HI_EVENT_STATEMNG_START,
        HI_EVENT_STATEMNG_STOP,
        HI_EVENT_STATEMNG_SETTING,
        HI_EVENT_STATEMNG_POWEROFF,
        HI_EVENT_STATEMNG_SD_AVAILABLE,
        HI_EVENT_STATEMNG_SD_UNAVAILABLE,
        HI_EVENT_STATEMNG_SD_FORMAT,
        HI_EVENT_STATEMNG_DEBUG_SWITCH,

        HI_EVENT_PHOTOMNG_TASK_STARTED,
        HI_EVENT_STATEMNG_UPGRADE_ABNORMAL,
        HI_EVENT_PHOTOMNG_TASK_END,

        HI_EVENT_USB_OUT,
        HI_EVENT_USB_INSERT,
        HI_EVENT_USB_STORAGE_READY,
        HI_EVENT_USB_UVC_READY,

        HI_EVENT_FILEMNG_SCAN_COMPLETED,
        HI_EVENT_FILEMNG_SPACE_FULL,
        HI_EVENT_FILEMNG_UNIDENTIFICATION,

        HI_EVENT_STORAGEMNG_DEV_UNPLUGED,
        HI_EVENT_STORAGEMNG_DEV_CONNECTING,
        HI_EVENT_STORAGEMNG_DEV_ERROR,
        HI_EVENT_STORAGEMNG_FS_CHECKING,
        HI_EVENT_STORAGEMNG_FS_CHECK_FAILED,
        HI_EVENT_STORAGEMNG_FS_EXCEPTION,
        HI_EVENT_STORAGEMNG_MOUNTED,
        HI_EVENT_STORAGEMNG_MOUNT_FAILED,

        HI_EVENT_UI_TOUCH,

        HI_EVENT_PDT_MEDIA_HDMI_IN,
        HI_EVENT_PDT_MEDIA_HDMI_OUT,

        HI_EVENT_UPGRADE_NEWPKT,
        HI_EVENT_UPGRADE_SUCCESS,
        HI_EVENT_UPGRADE_FAILURE,

        HI_EVENT_NETCTRL_CLIENT_CONNECTED,
        HI_EVENT_NETCTRL_CLIENT_DISCONNECTED,

        HI_EVENT_LIVESVR_CLIENT_CONNECT,
        HI_EVENT_LIVESVR_CLIENT_DISCONNECT,

#ifdef CONFIG_GAUGE_ON
        HI_EVENT_GAUGEMNG_LEVEL_CHANGE,
        HI_EVENT_GAUGEMNG_LEVEL_LOW,
        HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW,
        HI_EVENT_GAUGEMNG_LEVEL_NORMAL,
#endif
    };


    if(bSubscribeFinish)
    {
        MLOGD("Events has been subscribed\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_EVTHUB_Register(HI_EVENT_UI_TOUCH);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_EVTHUB_CreateSubscriber(&stSubscriber, &SubscriberHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_U32 u32ArraySize = ARRAY_SIZE(aEvents);
    for(i = 0; i<u32ArraySize; i++)
    {
        s32Ret = HI_EVTHUB_Subscribe(SubscriberHdl, aEvents[i]);
        if(s32Ret)
        {
            MLOGE("Subscribe Event(%#x) failed. %#x\n", aEvents[i], s32Ret);
            continue;
        }
    }
    bSubscribeFinish = HI_TRUE;
    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

