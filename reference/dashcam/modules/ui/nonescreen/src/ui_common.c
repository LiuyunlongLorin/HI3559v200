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
#include <pthread.h>

#include "hi_appcomm.h"
#include "hi_keymng.h"
#include "hi_system.h"
#include "hi_gsensormng.h"
#include "hi_eventhub.h"
#include "hi_message.h"
#include "hi_voiceplay.h"
#include "hi_upgrade.h"
#include "hi_product_media.h"
#include "hi_product_statemng.h"
#include "hi_product_netctrl.h"
#include "hi_product_param_define.h"
#include "hi_product_param.h"
#include "hi_product_media.h"
#include "hi_product_ui.h"
#include "mpi_sys.h"
#ifdef CONFIG_ACC_ON
#include "hi_accmng.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "UI"

#define CONFIG_SYS_SOUND_ON

#define PDT_UI_VOICE_MAX_NUM            (5)
#define PDT_UI_VOICE_SD_NO_EXIST_IDX    (0)
#define PDT_UI_VOICE_SD_NO_EXIST_SRC    "/app/bin/res/voice/sd_no_exist.m4a"
#define PDT_UI_VOICE_EMR_START_IDX      (1)
#define PDT_UI_VOICE_EMR_START_SRC      "/app/bin/res/voice/emr_start.m4a"
#define PDT_UI_VOICE_WIFI_IDX           (2)
#define PDT_UI_VOICE_WIFI_SRC           "/app/bin/res/voice/wifi.m4a"
#define PDT_UI_VOICE_OPEN_IDX           (3)
#define PDT_UI_VOICE_OPEN_SRC           "/app/bin/res/voice/open.m4a"
#define PDT_UI_VOICE_CLOSE_IDX          (4)
#define PDT_UI_VOICE_CLOSE_SRC          "/app/bin/res/voice/close.m4a"

#define PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(RetValue,ErrString)\
do{\
    if(HI_SUCCESS != RetValue)\
    {\
        MLOGE(RED"%s failed, s32Ret(%x)\n\n"NONE, ErrString, RetValue);\
        return RetValue;\
    }\
  }while(0)

static PDT_UI_MESSAGE_CONTEXT s_stMessageCtx = {.s32Workmode = HI_PDT_WORKMODE_BUTT, .bMsgProcessed = HI_TRUE, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};

#ifdef CONFIG_SYS_SOUND_ON

#define KEYTONE_RES_PATH  "/app/bin/res/keytone/0.pcm"
static HI_U64 s_u64PhyAddr = 0;
static HI_VOID* s_pVitAddr = NULL;
static HI_S32 s_s32DataLen = 0;
static HI_BOOL s_bShuttingDown = HI_FALSE;

HI_S32 PDT_UI_COMM_InitKeyTone(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    FILE* fp = NULL;
    HI_U32 u32CurPos = 0;

    if (NULL != s_pVitAddr)
    {
        MLOGE("KeyTone do not allow multiple opens!\n");
        return HI_FAILURE;
    }

    fp = fopen(KEYTONE_RES_PATH, "rb");

    if (fp)
    {
        u32CurPos = ftell(fp);
        fseek(fp, 0L, SEEK_END);
        s_s32DataLen = ftell(fp);
        fseek(fp, u32CurPos, SEEK_SET);
    }
    else
    {
        MLOGE("open file fail!\n");
        s_s32DataLen = 0;
        return HI_FAILURE;
    }

    if (s_s32DataLen < 0)
    {
        s_s32DataLen = 0;
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&s_u64PhyAddr, &s_pVitAddr, "keytone", "anonymous", (HI_U32)s_s32DataLen);

    if (HI_SUCCESS != s32Ret)
    {
        fclose(fp);
        MLOGE("HI_MAPI_AllocBuffer failed!\n\n");
        return HI_FAILURE;
    }

    if (fread(s_pVitAddr, s_s32DataLen, 1, fp) < 1)
    {
        fclose(fp);
        MLOGE("read file  fail! \n");
        return HI_FAILURE;
    }

    fclose(fp);

    return s32Ret;
}

HI_S32 PDT_UI_COMM_DeinitKeyTone(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (s_pVitAddr)
    {
        s32Ret = HI_MPI_SYS_MmzFree(s_u64PhyAddr, s_pVitAddr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        s_pVitAddr = NULL;
        s_s32DataLen = 0;
        s_u64PhyAddr = 0;

    }

    return s32Ret;
}

static HI_S32 PDT_UI_COMM_KeyTone(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_PARAM_MEDIA_COMM_CFG_S stMediaCommCfg;

    s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stMediaCommCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s32Ret = HI_SYSTEM_Sound(stMediaCommCfg.stAudioOutCfg.astAoCfg[0].AoHdl,
                             &stMediaCommCfg.stAudioOutCfg.astAoCfg[0].stAoAttr, s_u64PhyAddr, (HI_U32)s_s32DataLen);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return s32Ret;
}

#endif

HI_S32 PDT_UI_COMM_POWEROFF(HI_VOID)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};
    HI_S32  s32ParkingLevel = 0;
    HI_SYSTEM_WAKEUP_LEVEL_E enLevel = HI_SYSTEM_WAKEUP_LEVEL_LOW;
#ifdef HI3559V200
#ifdef BOARD_DASHCAM_REFB
        enLevel = HI_SYSTEM_WAKEUP_LEVEL_HIGH;
#endif
#endif

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_GSENSOR_PARKING, &s32ParkingLevel);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"get parkinglevel failed");

    HI_STORAGEMNG_CFG_S stStorageCfg = {};
    HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;
    s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"GetStorageCfg failed");

    s32Ret = HI_STORAGEMNG_GetState(stStorageCfg.szMntPath, &enStorageState);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"Get Storagemng State failed");

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState ={};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"Get Statemng State failed");

    if ((0 == s32ParkingLevel)  || ((HI_STORAGE_STATE_DEV_UNPLUGGED == enStorageState)  &&
        (HI_PDT_WORKMODE_PARKING_REC == stWorkModeState.enWorkMode)))
    {
        HI_GSENSORMNG_SetSensitity(HI_GSENSORMNG_SENSITITY_OFF);
        HI_SYSTEM_SetWakeUpEnable(HI_FALSE,enLevel);
        stMessage.what = HI_EVENT_STATEMNG_POWEROFF;
    }
    else
    {
        s32Ret = HI_GSENSORMNG_SetSensitity(s32ParkingLevel);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        HI_SYSTEM_SetWakeUpEnable(HI_TRUE,enLevel);
        stMessage.what = HI_EVENT_STATEMNG_POWEROFF;
    }
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "Send poweroff msg");

    s_bShuttingDown = HI_TRUE;
    return HI_SUCCESS;
}


HI_S32 PDT_UI_COMM_SendAsyncMsg(HI_MESSAGE_S* pstMsg, PDT_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pstMsg, -1);

    HI_MUTEX_LOCK(s_stMessageCtx.MsgMutex);

    if ( !s_stMessageCtx.bMsgProcessed)
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

HI_S32 PDT_UI_COMM_SwitchToUVC(HI_EVENT_S* pstEvent)
{
    HI_MESSAGE_S stMsg = {};
    stMsg.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMsg.arg2 = HI_PDT_WORKMODE_UVC;
    return HI_PDT_STATEMNG_SendMessage(&stMsg);
}

HI_S32 PDT_UI_COMM_SwitchToUsbStorage(HI_EVENT_S* pstEvent)
{
    HI_MESSAGE_S stMsg = {};
    stMsg.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMsg.arg2 = HI_PDT_WORKMODE_USB_STORAGE;
    s_stMessageCtx.s32Workmode = HI_PDT_WORKMODE_USB_STORAGE;
    return HI_PDT_STATEMNG_SendMessage(&stMsg);
}

HI_S32 PDT_UI_COMM_USBSwitchToPrepare(HI_EVENT_S* pstEvent)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_MESSAGE_S stMsg = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    memset(&stMsg, 0, sizeof(HI_MESSAGE_S));

    if (stWorkModeState.bRunning)
    {
        stMsg.what = HI_EVENT_STATEMNG_STOP;
        stMsg.arg1 = HI_TRUE;
        stMsg.arg2 = stWorkModeState.enWorkMode;
        if (HI_EVENT_USB_UVC_READY == pstEvent->EventID)
        {
            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMsg, PDT_UI_COMM_SwitchToUVC);
        }
        else if (HI_EVENT_USB_STORAGE_READY == pstEvent->EventID)
        {
            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMsg, PDT_UI_COMM_SwitchToUsbStorage);
        }
    }
    else
    {
        if (HI_EVENT_USB_UVC_READY == pstEvent->EventID)
        {
            s32Ret = PDT_UI_COMM_SwitchToUVC(pstEvent);
        }
        else if (HI_EVENT_USB_STORAGE_READY == pstEvent->EventID)
        {
            s32Ret = PDT_UI_COMM_SwitchToUsbStorage(pstEvent);
        }
    }

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_PDT_STATEMNG_GetState failed :%#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_SwitchToUpgrade(HI_VOID)
{
    HI_MESSAGE_S stMsg = {};
    stMsg.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMsg.arg2 = HI_PDT_WORKMODE_UPGRADE;
    s_stMessageCtx.s32Workmode = HI_PDT_WORKMODE_UPGRADE;
    return HI_PDT_STATEMNG_SendMessage(&stMsg);
}

/**called by EVENTHUB module directly*/
HI_S32 PDT_UI_COMM_OnEvent(HI_EVENT_S* pstEvent, HI_VOID* pvArgv)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bKeyToneEnable;

    if(s_bShuttingDown)
    {
        MLOGI("Shutting down, ignore Event %x\n", pstEvent->EventID);
        return HI_SUCCESS;
    }

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2,pstEvent->s32Result);

    HI_MUTEX_LOCK(s_stMessageCtx.MsgMutex);
    if (!s_stMessageCtx.bMsgProcessed)
    {
        if ((s_stMessageCtx.stMsg.what == pstEvent->EventID)
            && (s_stMessageCtx.stMsg.arg1 == pstEvent->arg1)
            && (s_stMessageCtx.stMsg.arg2 == pstEvent->arg2))
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

#ifdef CONFIG_SYS_SOUND_ON

    if ((HI_EVENT_KEYMNG_SHORT_CLICK == pstEvent->EventID) || (HI_EVENT_KEYMNG_LONG_CLICK == pstEvent->EventID))
    {
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_KEYTONE, &bKeyToneEnable);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        if (bKeyToneEnable)
        {
            s32Ret = PDT_UI_COMM_KeyTone();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }

#endif

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if(s32Ret)
    {
        MLOGE("GetWorkState\n");
        return HI_FAILURE;
    }
    switch(pstEvent->EventID)
    {
        case HI_EVENT_KEYMNG_SHORT_CLICK:
            switch(pstEvent->arg1)
            {
                case HI_KEYMNG_KEY_IDX_2:  /*PDT_UI_KEY_BACK*/
                    if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode)
                    {
                        if(stWorkModeState.bEMRRecord)
                        {
                            MLOGE("EMRRecording, not support keys\n");
                            return HI_SUCCESS;
                        }

                        if(!stWorkModeState.bSDAvailable)
                        {
                            return HI_SUCCESS;
                        }

                        HI_MESSAGE_S stMessage = {};

                        stMessage.what = HI_EVENT_STATEMNG_EMR_BEGIN;
                        stMessage.arg2 = stWorkModeState.enWorkMode;
                        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                        if(s32Ret)
                        {
                            MLOGE("s32Ret %x\n", s32Ret);
                            return HI_SUCCESS;
                        }
                    }
                    break;
                default:
                    MLOGE("key value %u not processed\n", pstEvent->arg1);
                    return HI_SUCCESS;
            }
            return HI_SUCCESS;

        case HI_EVENT_KEYMNG_LONG_CLICK:
            switch(pstEvent->arg1)
            {
                case HI_KEYMNG_KEY_IDX_1:
#ifdef CONFIG_WIFI_ON
                {
                    HI_S32 s32CurValue = 0;
                    s32Ret = HI_HAL_WIFI_GetStartedStatus((HI_BOOL*)&s32CurValue);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_GetStartedStatus");

                    if(HI_FALSE == s32CurValue)
                    {
                        HI_HAL_WIFI_APMODE_CFG_S stApCfg = {};
                        HI_HAL_WIFI_CFG_S stCfg = {};

                        stCfg.enMode = HI_HAL_WIFI_MODE_AP;
                        s32Ret = HI_HAL_WIFI_Init(stCfg.enMode);
                        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_Init");

                        /* after init wifi,do get ap param and start wifi */
                        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stApCfg);
                        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"GetCommParam");

                        memcpy(&stCfg.unCfg.stApCfg, &stApCfg, sizeof(HI_HAL_WIFI_APMODE_CFG_S));
                        s32Ret = HI_HAL_WIFI_Start(&stCfg);
                        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_Start");

                        HI_VOICEPLAY_VOICE_S stVoice=
                        {
                            .au32VoiceIdx={PDT_UI_VOICE_WIFI_IDX,PDT_UI_VOICE_OPEN_IDX},
                            .u32VoiceCnt=2,
                            .bDroppable=HI_TRUE,
                        };
                        HI_VOICEPLAY_Push(&stVoice, 0);
                        MLOGD("HI_HAL_WIFI_Start\n");
                    }
                    else
                    {
                        s32Ret = HI_HAL_WIFI_Stop();
                        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_Stop");

                        s32Ret = HI_HAL_WIFI_Deinit();
                        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_Deinit");

                        HI_VOICEPLAY_VOICE_S stVoice=
                        {
                            .au32VoiceIdx={PDT_UI_VOICE_WIFI_IDX,PDT_UI_VOICE_CLOSE_IDX},
                            .u32VoiceCnt=2,
                            .bDroppable=HI_TRUE,
                        };
                        HI_VOICEPLAY_Push(&stVoice, 0);
                        MLOGD("HI_HAL_WIFI_Stop\n");
                    }
                }
#endif
                    break;

                case HI_KEYMNG_KEY_IDX_2:
                    break;

                case HI_KEYMNG_KEY_IDX_0:
                    s32Ret = PDT_UI_COMM_POWEROFF();
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                    break;

                default:
                    return HI_SUCCESS;
            }
            return HI_SUCCESS;
        case HI_EVENT_STATEMNG_SHUTDOWN_PROC:
            {
                s32Ret = PDT_UI_COMM_POWEROFF();
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            break;

        case HI_EVENT_USB_INSERT:
            MLOGW("USB INSERT\n");
            return HI_SUCCESS;

        case HI_EVENT_USB_STORAGE_READY:
            {
                MLOGW("USB STORAGE PC Ready\n");
                s32Ret = PDT_UI_COMM_USBSwitchToPrepare(pstEvent);
                if(s32Ret)
                {
                    MLOGE("SendMessage failed. %x\n", s32Ret);
                }
            }
            return HI_SUCCESS;

        case HI_EVENT_USB_UVC_READY:
            {
               s32Ret = PDT_UI_COMM_USBSwitchToPrepare(pstEvent);
                if(s32Ret)
                {
                    MLOGE("SendMessage failed. %x\n", s32Ret);
                }
            }
            return HI_SUCCESS;

        case HI_EVENT_USB_OUT:
            {
                MLOGW("USB OUT\n");
                HI_MESSAGE_S stMessage = {};
                if(HI_PDT_WORKMODE_UVC == stWorkModeState.enWorkMode || HI_PDT_WORKMODE_USB_STORAGE == stWorkModeState.enWorkMode)
                {
                    HI_PDT_WORKMODE_E enPoweronWorkmode = HI_PDT_WORKMODE_BUTT;
                    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enPoweronWorkmode);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                    MLOGD("enPoweronWorkmode(%d)\n", enPoweronWorkmode);

                    stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                    stMessage.arg2 = enPoweronWorkmode;
                    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                    if(s32Ret)
                    {
                        MLOGE("SendMessage failed. %x\n", s32Ret);
                    }
                }
            }
            return HI_SUCCESS;
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            {
                MLOGW(RED"SD unpluged  !! \n"NONE);
            }
            break;

        case HI_EVENT_STATEMNG_SD_UNAVAILABLE:
        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
        case HI_EVENT_FILEMNG_UNIDENTIFICATION:
            {
                MLOGW(RED"SD can not use  !! \n"NONE);
                HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_BUTT;
                s32Ret = HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "get start_up source failed");

                if (HI_SYSTEM_STARTUP_SRC_WAKEYP == enStartupSrc)
                {
                    MLOGE(" SD error, power off when PowerOn source is wakeup\n");
                    PDT_UI_COMM_POWEROFF();
                }
            }
            break;

        case HI_EVENT_GSENSORMNG_COLLISION:
            {
                HI_MESSAGE_S stMessage = {};
                HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
                s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
                if (HI_SUCCESS == s32Ret)
                {
                    if ((stWorkModeState.bEMRRecord) || (stWorkModeState.bStateMngInProgress))
                    {
                        MLOGE("statemng busy !\n");
                        return HI_FAILURE;
                    }
                    if(!stWorkModeState.bSDAvailable)
                    {
                        MLOGE(" SD state is not normal \n");
                        return HI_FAILURE;
                    }
                    stMessage.what = HI_EVENT_STATEMNG_EMR_BEGIN;
                    stMessage.arg2 = stWorkModeState.enWorkMode;
                    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
            }
            return HI_SUCCESS;
        case HI_EVENT_STATEMNG_EMR_BEGIN:
            if (HI_SUCCESS == pstEvent->s32Result)
            {
                if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode)
                {
                    HI_VOICEPLAY_VOICE_S stVoice=
                    {
                        .au32VoiceIdx={PDT_UI_VOICE_EMR_START_IDX},
                        .u32VoiceCnt=1,
                        .bDroppable=HI_TRUE,
                    };
                    HI_VOICEPLAY_Push(&stVoice, 0);

                    s32Ret = HI_USB_SetMode(HI_USB_MODE_CHARGE);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"set usb mode failed");

                }
            }
            break;
        case HI_EVENT_STATEMNG_EMR_END:
            if (HI_SUCCESS == pstEvent->s32Result)
            {
                HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_BUTT;
                HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
                s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                if (HI_PDT_WORKMODE_PARKING_REC != stWorkModeState.enWorkMode)
                {
                    if(HI_FALSE == PDT_NETCTRL_IsClientConnecting())
                    {
                        HI_USB_MODE_E enUsbMode;

                        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_USB_MODE, &enUsbMode);
                        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"GetCommParam failed");

                        s32Ret = HI_USB_SetMode(enUsbMode);
                        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"set usb mode failed");
                    }
                }

                if (stWorkModeState.bEMRRecord == HI_FALSE)
                {
                    HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
                    if (enStartupSrc == HI_SYSTEM_STARTUP_SRC_WAKEYP)
                    {
                        s32Ret = PDT_UI_COMM_POWEROFF();
                        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                    }
                }
            }
            break;

        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            if (HI_SUCCESS == pstEvent->s32Result)
            {
                HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
                HI_EVENT_S stEvent = {};
                HI_MESSAGE_S stMessage = {};
                if (HI_PDT_WORKMODE_NORM_REC == pstEvent->arg2)
                {
                    s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STATEMNG_SD_AVAILABLE, &stEvent);
                    if (HI_SUCCESS == s32Ret && s_stMessageCtx.s32Workmode != HI_PDT_WORKMODE_USB_STORAGE) /* not  PowerAction   */
                    {
                        s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
                        if (HI_SUCCESS == s32Ret)
                        {
                            if ((stWorkModeState.bEMRRecord) || (stWorkModeState.bStateMngInProgress) || (stWorkModeState.bRunning))
                            {
                                MLOGE("statemng busy !\n");
                                return HI_FAILURE;
                            }
                            if(!(PDT_NETCTRL_IsClientConnecting()))
                            {
                                stMessage.what = HI_EVENT_STATEMNG_START;
                                stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
                                s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                                if (s32Ret)
                                {
                                    MLOGE("SendMessage failed. %x\n", s32Ret);
                                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                                }
                            }
                        }
                    }
                    else
                    {
                        MLOGI("HI_EVTHUB_GetEventHistory fail %x\n", s32Ret);
                        s_stMessageCtx.s32Workmode = HI_PDT_WORKMODE_BUTT;
                    }
                }
                else if(HI_PDT_WORKMODE_UPGRADE == pstEvent->arg2)
                {
                    if (s_stMessageCtx.s32Workmode == HI_PDT_WORKMODE_UPGRADE)
                    {
                        MLOGI(RED"System will automatically reboot in 5 seconds for upgrade.\n"NONE);
                        sleep(5);
                        stMessage.what = HI_EVENT_STATEMNG_UPGRADE_START;
                        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                    }
                    else
                    {
                        /*apk switch wokemode to upgrade,do nothing*/
                        MLOGI(RED"System will reboot soon after receiving the upgrade package....\n"NONE);
                    }
                }
                return HI_SUCCESS;
            }
            break;

        case HI_EVENT_STATEMNG_SD_AVAILABLE:
            {
                HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_BUTT;
                HI_MESSAGE_S stMessage = {};

                if(pstEvent->arg1 == 1)
                {
                    MLOGI("the event from format sd\n");
                    return HI_SUCCESS;
                }

                HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
                if (enStartupSrc == HI_SYSTEM_STARTUP_SRC_STARTUP)
                {
                    if (HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
                    {
                        MLOGI(RED"Photo Mode!\n"NONE);
                    }
                    else
                    {
                        MLOGI(RED"Normal REC \n"NONE);
                        stMessage.what = HI_EVENT_STATEMNG_START;
                        stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
                        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                        if (s32Ret)
                        {
                            MLOGE("SendMessage failed. %x\n", s32Ret);
                            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                        }
                    }
                }
                else if (enStartupSrc == HI_SYSTEM_STARTUP_SRC_WAKEYP)
                {
                    MLOGI(RED"parking REC \n"NONE);
                }
            }
            break;
        case HI_EVENT_UPGRADE_NEWPKT:
            {
                HI_MESSAGE_S stMessage = {};
                s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                if (HI_PDT_WORKMODE_UPGRADE != stWorkModeState.enWorkMode)
                {
                    /*board upgrade*/
                    MLOGW("enter board upgrade\n");
                    HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_BUTT;
                    HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);

                    HI_S32  s32CollisionCnt = 0;
                    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT, &s32CollisionCnt);
                    if (s32Ret == HI_SUCCESS)
                    {
                        if (s32CollisionCnt > 0)
                        {
                            MLOGW("not support SD upgrade,because have PARKING_COLLISION record\n");
                            return HI_SUCCESS;
                        }
                    }
                    else
                    {
                        MLOGE("get param fialed \n");
                        return HI_FAILURE;
                    }

                    /*temporary only consider startup state , not upgrade in parking mode */
                    if (HI_SYSTEM_STARTUP_SRC_STARTUP == enStartupSrc)
                    {
                        if ((stWorkModeState.bEMRRecord) || (stWorkModeState.bStateMngInProgress))
                        {
                            MLOGE("statemng busy \n");
                            return HI_FAILURE;
                        }

                        if (stWorkModeState.bRunning)
                        {
                            stMessage.what = HI_EVENT_STATEMNG_STOP;
                            stMessage.arg1 = HI_TRUE;
                            stMessage.arg2 = stWorkModeState.enWorkMode;
                            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                            if (s32Ret)
                            {
                                MLOGE("SendMessage failed. %x\n", s32Ret);
                                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                            }
                        }

                        s32Ret = PDT_UI_COMM_SwitchToUpgrade();
                        if (s32Ret)
                        {
                            MLOGE("PDT_UI_COMM_SwitchToUpgrade. %x\n", s32Ret);
                            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                        }
                    }
                    else
                    {
                        MLOGW("not support SD upgrade in ohter PowerAction mode\n");
                    }
                }
                else
                {
                    /*apk upgrade*/
                    MLOGI("enter board upgrade!\n");
                    return HI_SUCCESS;
                }
            }
            break;
#ifdef CONFIG_TEMPRATURE_CYCLE_ON
        case HI_EVENT_TEMPCYCLE_TEMP_CHANGE:
        {
            HI_MESSAGE_S stMessage = {};
            stMessage.what = HI_EVENT_TEMPCYCLE_TEMP_CHANGE;
            stMessage.arg1 = pstEvent->arg1;
            stMessage.arg2 = stWorkModeState.enWorkMode;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
            if (s32Ret)
            {
                MLOGE("SendMessage failed. %x\n", s32Ret);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }

        }
        break;
#endif
#ifdef CONFIG_ACC_ON
        case HI_EVENT_ACCMNG_POWEROFF:
            MLOGI("start to power off!\n");
            s32Ret = PDT_UI_COMM_POWEROFF();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
#endif

        default:
            MLOGD("Event %#x not processed \n", pstEvent->EventID);
            break;
    }

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
        HI_EVENT_STATEMNG_POWEROFF,
        HI_EVENT_STATEMNG_SD_AVAILABLE,
        HI_EVENT_STATEMNG_SD_UNAVAILABLE,
        HI_EVENT_STATEMNG_SD_FORMAT,
        HI_EVENT_STATEMNG_EMR_BEGIN,
        HI_EVENT_STATEMNG_EMR_END,
        HI_EVENT_STATEMNG_SHUTDOWN_PROC,
        HI_EVENT_USB_OUT,
        HI_EVENT_USB_INSERT,
        HI_EVENT_USB_STORAGE_READY,
        HI_EVENT_USB_UVC_READY,
        HI_EVENT_FILEMNG_UNIDENTIFICATION,
        HI_EVENT_STORAGEMNG_DEV_UNPLUGED,
        HI_EVENT_STORAGEMNG_DEV_CONNECTING,
        HI_EVENT_STORAGEMNG_DEV_ERROR,
        HI_EVENT_STORAGEMNG_FS_CHECKING,
        HI_EVENT_STORAGEMNG_FS_CHECK_FAILED,
        HI_EVENT_STORAGEMNG_FS_EXCEPTION,
        HI_EVENT_STORAGEMNG_MOUNTED,
        HI_EVENT_STORAGEMNG_MOUNT_FAILED,
        HI_EVENT_GSENSORMNG_COLLISION,
        HI_EVENT_UPGRADE_NEWPKT,
#ifdef CONFIG_ACC_ON
        HI_EVENT_ACCMNG_POWEROFF,
#endif

    };

    if(bSubscribeFinish)
    {
        MLOGD("Events has been subscribed\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_EVTHUB_CreateSubscriber(&stSubscriber, &SubscriberHdl);
    if(s32Ret)
    {
        MLOGE("CreateSubscriber failed. %#x\n", s32Ret);
        return HI_FAILURE;
    }

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

HI_S32 HI_PDT_UI_Init(HI_VOID)
{
    HI_S32 s32Ret;
    s32Ret = PDT_UI_COMM_SubscribeEvents();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"PDT_UI_COMM_SubscribeEvents");

    #ifdef CONFIG_SYS_SOUND_ON
    s32Ret = PDT_UI_COMM_InitKeyTone();
    if(s32Ret!=HI_SUCCESS)
    {
        MLOGE(" PDT_UI_COMM_InitKeyTone error ! \n");
    }
    MLOGI("Init KeyTone OK.\n");
    #endif

    HI_PDT_PARAM_MEDIA_COMM_CFG_S stMediaCommCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stMediaCommCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"HI_PDT_PARAM_GetMediaCommCfg");

    HI_VOICEPLAY_CFG_S stVoicePlayCfg;
    stVoicePlayCfg.stAoutOpt.hAudDevHdl = stMediaCommCfg.stAudioOutCfg.astAoCfg[0].AoHdl;
    stVoicePlayCfg.stAoutOpt.hAudTrackHdl = (HI_MAPI_AO_CHN_MAX_NUM - 1);
    stVoicePlayCfg.u32MaxVoiceCnt = PDT_UI_VOICE_MAX_NUM;
    HI_VOICEPLAY_VOICETABLE_S astVoiceTab[PDT_UI_VOICE_MAX_NUM]=
    {
        {PDT_UI_VOICE_SD_NO_EXIST_IDX,PDT_UI_VOICE_SD_NO_EXIST_SRC},
        {PDT_UI_VOICE_EMR_START_IDX,PDT_UI_VOICE_EMR_START_SRC},
        {PDT_UI_VOICE_WIFI_IDX,PDT_UI_VOICE_WIFI_SRC},
        {PDT_UI_VOICE_OPEN_IDX,PDT_UI_VOICE_OPEN_SRC},
        {PDT_UI_VOICE_CLOSE_IDX,PDT_UI_VOICE_CLOSE_SRC}
    };
    stVoicePlayCfg.pstVoiceTab = astVoiceTab;
    s32Ret = HI_VOICEPLAY_Init(&stVoicePlayCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"HI_VOICEPLAY_Init");
    return HI_SUCCESS;
}


HI_S32 HI_PDT_UI_Deinit(HI_VOID)
{
#if defined(CONFIG_WIFI_ON)
    HI_S32 s32Ret;
    HI_BOOL bWiFiOn = HI_FALSE;

    s32Ret = HI_HAL_WIFI_GetStartedStatus(&bWiFiOn);
    MLOGD("HI_HAL_WIFI_GetStartedStatus. %#x\n", s32Ret);

    if(bWiFiOn)
    {
        s32Ret = HI_HAL_WIFI_Stop();
        MLOGD("HI_HAL_WIFI_Stop. %#x\n", s32Ret);

        s32Ret = HI_HAL_WIFI_Deinit();
        MLOGD("HI_HAL_WIFI_Deinit. %#x\n", s32Ret);
    }
#endif

    #ifdef CONFIG_SYS_SOUND_ON
    PDT_UI_COMM_DeinitKeyTone();
    #endif
    HI_VOICEPLAY_Deinit();
    return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

