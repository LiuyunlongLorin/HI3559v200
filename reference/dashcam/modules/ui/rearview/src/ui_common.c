/**
 * @file      ui_common.c
 * @brief     ui internal common function code
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/7
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
#include "hi_keymng.h"
#include "hi_product_ui.h"
#include "hi_timedtask.h"
#include "hi_system.h"
#include "hi_gsensormng.h"
#include "hi_product_netctrl.h"
#include "hi_server_log.h"
#include "hi_voiceplay.h"
#include "ui_powercontrol.h"
#ifdef CONFIG_ACC_ON
#include "hi_accmng.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


static PDT_UI_MESSAGE_CONTEXT s_stMessageCtx = {.bMsgProcessed = HI_TRUE, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
HI_PDT_WORKMODE_E g_enUsbPreviousMode = HI_PDT_WORKMODE_BUTT;  /*WorkMode before USB*/
static HI_USB_MODE_E s_enUSBRepeatedSendFlg = HI_USB_MODE_BUTT;  /* recording of switch usb workmode failed due to statemng busy */
extern HI_BOOL g_bSaveCurGridIndex;
static HI_BOOL s_bShuttingDown = HI_FALSE;

/* measure  time   */
#ifdef CFG_TIME_STAMP_ON
static struct timespec stUITimespecBegin = {0};
static struct timespec stUITimespecEnd = {0};
#endif

typedef struct tagPDT_UI_SRC_ID_S
{
    HI_CHAR szContent[SETTING_CONTENT_SIZE]; //from valuset.ini
    HI_U32 u32StringId; //is languae string id
} PDT_UI_SRC_ID_S;

static const PDT_UI_SRC_ID_S s_astSrcId[] =
{
    {"NORMAL", STRING_NORMAL_REC},
    {"LAPSE", STRING_LAPSE_REC},
    {"0.5S", STRING_500MS},
    {"1S", STRING_1S},
    {"5S", STRING_5S},
    {"1MIN", STRING_1MIN},
    {"3MIN", STRING_3MIN},
    {"5MIN", STRING_5MIN},
    {"ON", STRING_ON},
    {"OFF", STRING_OFF},
    {"en", STRING_EN},
    {"zh", STRING_ZH},
    {"CHARGE", STRING_SELECT_CHARGE},
    {"UVC", STRING_SELECT_UVC},
    {"STORAGE", STRING_SELECT_STORAGE},
    {"MIDDLE", STRING_MIDDLE},
    {"HIGH", STRING_HIGH},
    {"LOW", STRING_LOW},
    {"MUTE", STRING_MUTE},
    {"H264", STRING_H264},
    {"H265", STRING_H265},
    {"front_emr", STRING_EMR_FRONT},
    {"front_norm", STRING_NORM_FRONT},
    {"back_emr", STRING_EMR_REAR},
    {"back_norm", STRING_NORM_REAR},
    {"front_photo", STRING_PHOTO_FRONT},
    {"back_photo", STRING_PHOTO_BACK},
    {"emr", STRING_SINGLE_EMR},
    {"norm", STRING_SINGLE_NORM},
    {"photo", STRING_PHOTO},
    {"PAL", STRING_PAL},
    {"NTSC", STRING_NTSC},
};

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

/*Get execute time since started, return by Second */
HI_U64 PDT_UI_COMM_GetExecTime(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_EVENT_S stEvent = {};
    struct timespec stCurTime = {};
    HI_U64 u64ExecTime = 0;

    s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STATEMNG_START, &stEvent);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, 0, "GetEventHistory");

    clock_gettime(CLOCK_MONOTONIC, &stCurTime);
    u64ExecTime = (stCurTime.tv_sec * 1000 + stCurTime.tv_nsec / 1000000) - stEvent.u64CreateTime;
    u64ExecTime = u64ExecTime / 1000 + (u64ExecTime % 1000 > 500 ? 1 : 0);

    return u64ExecTime;
}
/*Get remaining time of DELAY_PHOTO mode since started,  return by Second */
HI_U64 PDT_UI_COMM_GetRemainDelayTime(HI_VOID)
{

    return 0;
}


HI_VOID PDT_UI_COMM_Second2String(HI_U64 second, HI_CHAR* pszBuffer, HI_U32 size)
{
    if (NULL == pszBuffer)
    {
        MLOGE("pointer is NULL");
        return;
    }

    HI_U64 hour, min;

    hour = second / 3600;
    min = (second % 3600) / 60;
    second = second % 60;
    snprintf(pszBuffer, size, "%02llu:%02llu:%02llu", hour, min, second);
}

/*Get current date and time*/
HI_S32 PDT_UI_COMM_GetDateTime(PDT_UI_DATETIME_S* pstDateTime)
{
    time_t now = 0;
    struct tm result = {};
    HI_APPCOMM_CHECK_POINTER(pstDateTime, -1);

    now = time(NULL);

    if (NULL == localtime_r(&now, &result) )
    {
        MLOGE("localtime_r failed\n");
        return HI_FAILURE;
    }

    pstDateTime->year = result.tm_year + 1900;
    pstDateTime->month = result.tm_mon + 1;
    pstDateTime->day = result.tm_mday;
    pstDateTime->hour = result.tm_hour;
    pstDateTime->minute = result.tm_min;
    pstDateTime->second = result.tm_sec;

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_SetDateTime(PDT_UI_DATETIME_S* pstDateTime)
{
    HI_APPCOMM_CHECK_POINTER(pstDateTime, -1);

    HI_S32 s32Ret;
    HI_SYSTEM_TM_S stDateTime = {};
    stDateTime.s32year = (HI_S32)pstDateTime->year;
    stDateTime.s32mon = (HI_S32)pstDateTime->month;
    stDateTime.s32mday = (HI_S32)pstDateTime->day;
    stDateTime.s32hour = (HI_S32)pstDateTime->hour;
    stDateTime.s32min = (HI_S32)pstDateTime->minute;
    stDateTime.s32sec = 0;

    s32Ret = HI_SYSTEM_SetDateTime(&stDateTime);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_SYSTEM_SetDateTime");

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_GetDateString(PDT_UI_DATETIME_S* pstDateTime, HI_CHAR* pszBuffer, HI_U32 size)
{
    HI_APPCOMM_CHECK_POINTER(pstDateTime, -1);
    HI_APPCOMM_CHECK_POINTER(pszBuffer, -1);
    snprintf(pszBuffer, size, "%u-%u-%u", pstDateTime->year, pstDateTime->month, pstDateTime->day);

    return HI_SUCCESS;
}
HI_S32 PDT_UI_COMM_GetTimeString(PDT_UI_DATETIME_S* pstDateTime, HI_CHAR* pszBuffer, HI_U32 size)
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
    HI_CHAR* pszLangID = NULL;
    HI_CHAR* pszContent = NULL;

    HI_APPCOMM_CHECK_POINTER(pszBuffer, -1);

    s32Ret = HI_GV_Lan_GetCurLangID( &pszLangID );
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCurLangID");

    s32Ret = HI_GV_Lan_GetLangString(pszLangID, u32StrID, &pszContent);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetLangString");

    snprintf(pszBuffer, u32Size, "%s", pszContent);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_GetDirName(const HI_DTCF_DIR_E enDir, HI_CHAR* pszBuffer, HI_U32 u32Size)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_FILEMNG_CFG_S stFilemngCfg = {};

    if (enDir >= DTCF_DIR_BUTT)
    {
        MLOGE("not found value %d in valueset\n", enDir);
        return HI_FAILURE;
    }

    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stFilemngCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    snprintf(pszBuffer, u32Size, "%s", stFilemngCfg.stDtcfCfg.aszDirNames[enDir]);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_COMM_GetDescFromValueSet(HI_S32 s32Value, const HI_PDT_ITEM_VALUESET_S* pstValueSet,
        HI_CHAR* pszBuffer, HI_U32 u32Size)
{
    HI_U32 i = 0;
    HI_U32 s32Ret;
    HI_APPCOMM_CHECK_POINTER(pstValueSet, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pszBuffer,   HI_PDT_PARAM_EINVAL);

    for (i = 0; i < pstValueSet->s32Cnt && i < HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
    {
        if (s32Value == pstValueSet->astValues[i].s32Value)
        {

            s32Ret = PDT_UI_COMM_Translate(pstValueSet->astValues[i].szDesc, pszBuffer, u32Size);
            if (HI_SUCCESS != s32Ret)
            {
                snprintf(pszBuffer, u32Size, "%s", pstValueSet->astValues[i].szDesc);
            }
            return HI_SUCCESS;
        }
    }

    return HI_FAILURE;
}

/** Get  param type's  current value(string)  */
HI_S32 PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_E enWorkMode, HI_S32 s32CamID, HI_PDT_PARAM_TYPE_E enType,
                                     HI_CHAR* pszBuffer, HI_U32 u32Size)
{
    HI_S32 s32Value = 0;
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_ITEM_VALUESET_S stValueSet = {};
    HI_TIMEDTASK_ATTR_S stTimedTaskAttr = {};
    HI_APPCOMM_CHECK_POINTER(pszBuffer,   HI_PDT_PARAM_EINVAL);

    switch (enType)
    {
            /*workmode dependent, have value set*/
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
        case HI_PDT_PARAM_TYPE_SPLITTIME:
        case HI_PDT_PARAM_RECORD_TYPE:
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
        case HI_PDT_PARAM_TYPE_VIDEOMODE:
            s32Ret = HI_PDT_PARAM_GetCamParam(enWorkMode, s32CamID, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCamParam");

            s32Ret = HI_PDT_PARAM_GetCamItemValues(enWorkMode, s32CamID, enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCamItemValues");

            s32Ret = PDT_UI_COMM_GetDescFromValueSet(s32Value, &stValueSet, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetDescFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = HI_PDT_PARAM_GetCamParam(enWorkMode, s32CamID, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCamParam");

            s32Ret = PDT_UI_COMM_GetStringByID(
                         (HI_MAPI_PAYLOAD_TYPE_H264 == (HI_MAPI_PAYLOAD_TYPE_E)s32Value) ? STRING_H264 : STRING_H265,
                         pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetStringByID");
            break;

        case HI_PDT_PARAM_TYPE_FLIP:
        case HI_PDT_PARAM_TYPE_LDC:
        case HI_PDT_PARAM_TYPE_MIRROR:
        case HI_PDT_PARAM_TYPE_OSD:
        case HI_PDT_PARAM_TYPE_WDR:
            s32Ret = HI_PDT_PARAM_GetCamParam(enWorkMode, s32CamID, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCamParam");
            s32Ret = PDT_UI_COMM_GetStringByID(s32Value ? STRING_ON : STRING_OFF, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetStringByID");
            break;

        case HI_PDT_PARAM_TYPE_REC:
            s32CamID = BACK_CAM_ID;
            s32Ret = HI_PDT_PARAM_GetCamParam(enWorkMode, s32CamID, enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCamParam");
            s32Ret = PDT_UI_COMM_GetStringByID(s32Value ? STRING_ON : STRING_OFF, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetStringByID");
            break;

            /*items below are workmode independent*/
        case HI_PDT_PARAM_TYPE_AUDIO:
        case HI_PDT_PARAM_TYPE_KEYTONE:
        case HI_PDT_PARAM_TYPE_BOOTSOUND:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");

            s32Ret = PDT_UI_COMM_GetStringByID( (HI_BOOL)s32Value ? STRING_ON : STRING_OFF, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetStringByID");
            break;

        case HI_PDT_PARAM_TYPE_PREVIEW_CAMID:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");
            snprintf(pszBuffer, u32Size, "%d", s32Value);
            break;

        case HI_PDT_PARAM_TYPE_VOLUME:
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");

            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommItemValues");

            for (i = 0; i < stValueSet.s32Cnt && i < HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
            {
                if (s32Value <= stValueSet.astValues[i].s32Value)
                {
                    s32Ret = PDT_UI_COMM_Translate(stValueSet.astValues[i].szDesc, pszBuffer, u32Size);

                    if (HI_SUCCESS != s32Ret)
                    {
                        snprintf(pszBuffer, u32Size, "%s", stValueSet.astValues[i].szDesc);
                    }
                    return HI_SUCCESS;
                }
            }

            if (s32Value > stValueSet.astValues[stValueSet.s32Cnt - 1].s32Value )
            {
                snprintf(pszBuffer, u32Size, "%s", stValueSet.astValues[stValueSet.s32Cnt - 1].szDesc);
                return HI_SUCCESS;
            }

            break;

        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &stTimedTaskAttr);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");

            if ( !stTimedTaskAttr.bEnable )
            {
                s32Ret = PDT_UI_COMM_GetStringByID(STRING_OFF, pszBuffer, u32Size);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetStringByID");
                return HI_SUCCESS;
            }

            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommItemValues");

            s32Ret = PDT_UI_COMM_GetDescFromValueSet(stTimedTaskAttr.u32Time_sec, &stValueSet, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetDescFromValueSet");
            break;

        case HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY:
        case HI_PDT_PARAM_TYPE_USB_MODE:
        case HI_PDT_PARAM_TYPE_GSENSOR_PARKING:
        case HI_PDT_PARAM_TYPE_LANGUAGE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");

            s32Ret = HI_PDT_PARAM_GetCommItemValues(enType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommItemValues");

            s32Ret = PDT_UI_COMM_GetDescFromValueSet(s32Value, &stValueSet, pszBuffer, u32Size);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetDescFromValueSet");

            break;

        case HI_PDT_PARAM_TYPE_WIFI_AP:
        case HI_PDT_PARAM_TYPE_DEV_INFO:
        default:
            MLOGE("Unprocessed ParamType[%d]\n", enType);
            return HI_FAILURE;
    }


    return  HI_SUCCESS;
}

HI_VOID PDT_UI_COMM_ShowAlarm(HI_U32 u32StringID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_WINMNG_StartWindow(ALARM_WINDOW, HI_FALSE);

    if (s32Ret)
    {
        MLOGE("Start Window failed. %x\n", s32Ret);
    }

    s32Ret = HI_GV_Widget_SetTextByID(ALARM_WINDOW_LABEL_INFO, u32StringID);

    if (s32Ret)
    {
        MLOGE("SetTextByID failed. %x\n", s32Ret);
    }

    s32Ret = HI_GV_Widget_Paint(ALARM_WINDOW, 0);

    if (s32Ret)
    {
        MLOGE("SetTextByID failed. %x\n", s32Ret);
    }
}

HI_S32 PDT_UI_COMM_ActiveNextWidget(HI_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HI_HANDLE CurWidgetHdl)
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
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_Widget_Active");
            return HI_SUCCESS;
        }
    }

    MLOGE("not found the given widget %d, NO widget activated\n", CurWidgetHdl);
    return HI_FAILURE;
}

HI_BOOL PDT_UI_COMM_SdIsReady(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32StringID = 0;
    HI_EVENT_S stEvent = {};
    HI_STORAGEMNG_CFG_S stStorageCfg = {0};
    HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;
    HI_STORAGE_DEV_INFO_S stDevInfo = {0};

    s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FALSE, "HI_PDT_PARAM_GetStorageCfg");

    s32Ret = HI_STORAGEMNG_GetState(stStorageCfg.szMntPath, &enStorageState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FALSE, "HI_STORAGEMNG_GetState");
    MLOGI("Storage State = %d\n", enStorageState);

    switch (enStorageState)
    {
        case HI_STORAGE_STATE_DEV_UNPLUGGED:
            {
                u32StringID = STRING_NO_SD_CARD;
                HI_VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={PDT_UI_VOICE_SD_NO_EXIST_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=HI_TRUE,
                };
                HI_VOICEPLAY_Push(&stVoice, 0);
                break;
            }

        case HI_STORAGE_STATE_DEV_ERROR:
            u32StringID = STRING_SD_ERROR_PLUG;
            break;

        case HI_STORAGE_STATE_FS_CHECK_FAILED:
            s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_FS_CHECK_FAILED, &stEvent);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);

            switch (stEvent.arg1)
            {
                case HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION:
                case HI_STORAGE_FSTOOL_ERR_NOT_ENOUGH_MEMORY:
                case HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT:
                    u32StringID = STRING_SD_ERROR_FORMAT;
                    break;

                case HI_STORAGE_FSTOOL_ERR_OPEN_FAIL:
                case HI_STORAGE_FSTOOL_ERR_READ_FAIL:
                case HI_STORAGE_FSTOOL_ERR_WRITE_FAIL:
                case HI_STORAGE_FSTOOL_ERR_ILLEGAL_PARAM:
                case HI_STORAGE_FSTOOL_ERR_SYSTEM_CALL_FAILURE:
                    u32StringID = STRING_SD_ERROR_CHANGE;
                    break;

                default:
                    u32StringID = STRING_SD_ERROR;
                    break;
            }

            break;

        case HI_STORAGE_STATE_FS_EXCEPTION:
            s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_FS_EXCEPTION, &stEvent);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);

            switch (stEvent.arg1)
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
            {
                HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {0};
                s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                if (HI_TRUE == stWorkModeState.bSDAvailable)
                {
                    return HI_TRUE; /**statemng file scan ok ,and also speed normal ,fragement normal*/
                }
                s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STATEMNG_SD_UNAVAILABLE, &stEvent);
                if (s32Ret == HI_SUCCESS)
                {
                    MLOGE("SD file scan error\n");
                    u32StringID = STRING_SD_ERROR_FORMAT;/**statemng file scan fail */
                }
                else
                {
                    /* check SD fragemnet seriously */
                    s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_MOUNTED, &stEvent);
                    if (s32Ret == HI_SUCCESS)
                    {
                        if (stEvent.arg1 == HI_STORAGE_FSTOOL_ERR_FRAGMENT_SEVERELY)
                        {
                            u32StringID = STRING_SD_ERROR_FORMAT;/**statemng file scan ok,but fragemnet seriously */
                            MLOGE("storage mount event arrived,and fragment servious\n");
                        }
                     }

                    /* check SD speed */
                    s32Ret = HI_STORAGEMNG_GetDevInfo(stStorageCfg.szMntPath, &stDevInfo);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get dev info failed");
                    if ((NULL == strstr(stDevInfo.aszDevType, "MMC"))
                        &&(HI_STORAGE_TRANSMISSION_SPEED_1_4M == stDevInfo.enTranSpeed
                            || HI_STORAGE_TRANSMISSION_SPEED_4_10M == stDevInfo.enTranSpeed ))
                        {
                            u32StringID = STRING_SD_SPEED_LOW;/**statemng file scan ok,but speed slow */
                        }
                    /**statemng file scan is checking */
                }
            }
            break;

        case HI_STORAGE_STATE_MOUNT_FAILED:
            u32StringID = STRING_SD_ERROR_CHANGE;
            break;

        case HI_STORAGE_STATE_DEV_CONNECTING:
        case HI_STORAGE_STATE_FS_CHECKING:
            u32StringID = STRING_SD_PREPARING;
            break;

        default:
            u32StringID = STRING_SD_ERROR;
            break;
    }

    PDT_UI_COMM_ShowAlarm(u32StringID);

    return HI_FALSE;
}

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
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"get comm param failed");

    HI_STORAGEMNG_CFG_S stStorageCfg = {};
    HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;
    s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"GetStorageCfg failed");
    s32Ret = HI_STORAGEMNG_GetState(stStorageCfg.szMntPath, &enStorageState);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"Get Storagemng State failed");

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"Get Statemng State failed");


    if ((0 == s32ParkingLevel)  || ((HI_STORAGE_STATE_DEV_UNPLUGGED == enStorageState)  &&
        (HI_PDT_WORKMODE_PARKING_REC == stWorkModeState.enWorkMode)))
    {
        HI_GSENSORMNG_SetSensitity(HI_GSENSORMNG_SENSITITY_OFF);
        HI_SYSTEM_SetWakeUpEnable(HI_FALSE,enLevel);
        stMessage.what = HI_EVENT_STATEMNG_POWEROFF;
        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"Send poweroff msg failed");
    }
    else
    {
        s32Ret = HI_GSENSORMNG_SetSensitity(s32ParkingLevel);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"set Gsensor sensitity failed");
        HI_SYSTEM_SetWakeUpEnable(HI_TRUE,enLevel);
        PDT_UI_COMM_ShowAlarm(STRING_Parking);
        s32Ret = HI_GV_Timer_Create(ALARM_WINDOW, TIMER_REFRESH_ALARM_POWEROFF, 100);
        s32Ret |= HI_GV_Timer_Start(ALARM_WINDOW, TIMER_REFRESH_ALARM_POWEROFF);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"start poweroff time failed");
    }
    s_bShuttingDown = HI_TRUE;

    return HI_SUCCESS;
}

/**called by EVENTHUB module directly*/
HI_S32 PDT_UI_COMM_OnEvent(HI_EVENT_S* pstEvent, HI_VOID* pvArgv)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_INPUTEVENT_S stInputEvent = {};
    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2, pstEvent->s32Result);

    /**filter out the failed start and stop, because multi STOP may be launched at the same time*/
    if (!((HI_EVENT_STATEMNG_START  == pstEvent->EventID || HI_EVENT_STATEMNG_STOP == pstEvent->EventID)
          && HI_FAILURE == pstEvent->s32Result))
    {
        HI_BOOL bEventContinueHandle  = HI_FALSE;
        s32Ret = PDT_UI_POWERCTRL_PreProcessEvent(pstEvent, &bEventContinueHandle);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PreProcessEvent");
        if (!bEventContinueHandle)
        {
            MLOGD("Event %x has been processed by Power Control Module\n", pstEvent->EventID);
            return HI_SUCCESS;
        }
    }

    if ((HI_EVENT_UI_TOUCH == pstEvent->EventID) || s_bShuttingDown)
    {
        MLOGD("Touch or Shutting down, ignore Event %x\n", pstEvent->EventID);
        return HI_SUCCESS;
    }

    switch (pstEvent->EventID)
    {
        case HI_EVENT_KEYMNG_SHORT_CLICK:
            stInputEvent.msg = HIGV_MSG_KEYDOWN;

            switch (pstEvent->arg1)
            {
                case HI_KEYMNG_KEY_IDX_0:
                    stInputEvent.value = PDT_UI_KEY_OK;
                    break;

                case HI_KEYMNG_KEY_IDX_1:
                    stInputEvent.value = PDT_UI_KEY_MENU;
                    break;

                case HI_KEYMNG_KEY_IDX_2:
                    stInputEvent.value = PDT_UI_KEY_BACK;
                    break;

                case HI_KEYMNG_KEY_IDX_3:
                    stInputEvent.value = PDT_UI_KEY_EXPAND;
                    break;

                default:
                    MLOGE("key value %u not processed\n", pstEvent->arg1);
                    return HI_SUCCESS;
            }

            s32Ret = HI_GV_SendInputEvent(&stInputEvent);

            if (s32Ret)
            {
                MLOGE("SendInputEvent failed. %x\n", s32Ret);
            }

            return HI_SUCCESS;

        default:
            break;
    }

    s32Ret = HI_GV_Msg_SendAsyncWithData(HOME_PAGE, HIGV_MSG_EVENT, (HI_VOID*)pstEvent, sizeof(HI_EVENT_S));
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "SendAsyncWithData");

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
    return HI_PDT_STATEMNG_SendMessage(&stMsg);
}

HI_S32 PDT_UI_COMM_USBSwitchToPrepare(HI_EVENT_S* pstEvent)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_MESSAGE_S stMsg = {};

    g_enUsbPreviousMode = HI_PDT_WORKMODE_BUTT;
    s_enUSBRepeatedSendFlg = HI_USB_MODE_BUTT;

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if ((stWorkModeState.bEMRRecord) || (stWorkModeState.bStateMngInProgress))
    {
        /* recording  USB  switch  workmode */
        if(HI_EVENT_USB_UVC_READY == pstEvent->EventID)
        {
            s_enUSBRepeatedSendFlg = HI_USB_MODE_UVC;
        }
        else if(HI_EVENT_USB_STORAGE_READY == pstEvent->EventID)
        {
            s_enUSBRepeatedSendFlg = HI_USB_MODE_STORAGE;
        }

        MLOGE("statemng busy !\n");
        return HI_FAILURE;
    }

    if (stWorkModeState.enWorkMode == HI_PDT_WORKMODE_PLAYBACK)
    {
        s32Ret = HI_PLAYBACK_Stop();
        s32Ret |= HI_PLAYBACK_Deinit();

        if (s32Ret)
        {
            MLOGE(" STOP PLAYER failed !\n ");
        }
    }

    g_enUsbPreviousMode = stWorkModeState.enWorkMode;

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
        MLOGE(" enter usb mode send message failed :%#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID PDT_UI_COMM_SDEventProc(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32StringID = 0;
    HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_BUTT;

    switch (pstEvent->EventID)
    {
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            u32StringID = STRING_NO_SD_CARD;
            break;

        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        {
            HI_MESSAGE_S stMsg = {};
            HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

            /* not PowerAction  */
            if (pstEvent->arg1 < 3)
            {
                MLOGW("UI receive SD DEV_ERROR count[%d]\n", pstEvent->arg1);
            }
            else
            {
                MLOGE("UI receive SD DEV_ERROR count[%d]\n", pstEvent->arg1);
                u32StringID = STRING_SD_ERROR_CHANGE;

                s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get current workmode");

                stMsg.what = HI_EVENT_STATEMNG_STOP;
                stMsg.arg2 = stWorkModeState.enWorkMode;
                stMsg.arg1 = HI_TRUE;

                s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "send stop recorder message");
            }
        }
            break;

        case HI_EVENT_STORAGEMNG_FS_CHECKING:
            u32StringID = STRING_SD_PREPARING;
            break;

        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
            switch (pstEvent->arg1)
            {
                case HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION:
                case HI_STORAGE_FSTOOL_ERR_NOT_ENOUGH_MEMORY:
                case HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT:
                    u32StringID = STRING_SD_ERROR_FORMAT;
                    break;

                case HI_STORAGE_FSTOOL_ERR_OPEN_FAIL:
                case HI_STORAGE_FSTOOL_ERR_READ_FAIL:
                case HI_STORAGE_FSTOOL_ERR_WRITE_FAIL:
                case HI_STORAGE_FSTOOL_ERR_ILLEGAL_PARAM:
                case HI_STORAGE_FSTOOL_ERR_SYSTEM_CALL_FAILURE:
                    u32StringID = STRING_SD_ERROR_CHANGE;
                    break;

                default:
                    u32StringID = STRING_SD_ERROR;
                    break;
            }

            break;

        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
            switch (pstEvent->arg1)
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

        case HI_EVENT_STORAGEMNG_MOUNTED:
            {
                if (HI_STORAGE_FSTOOL_ERR_FRAGMENT_SEVERELY == pstEvent->arg1)
                {
                    u32StringID = STRING_SD_ERROR_FORMAT;
                }

                /* check SD speed */
                HI_STORAGEMNG_CFG_S stStorageCfg = {};
                HI_STORAGE_DEV_INFO_S stDevInfo = {};
                s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get storage config failed");
                s32Ret = HI_STORAGEMNG_GetDevInfo(stStorageCfg.szMntPath, &stDevInfo);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get dev info failed");

                if ((NULL == strstr(stDevInfo.aszDevType, "MMC"))
                    && (HI_STORAGE_TRANSMISSION_SPEED_1_4M == stDevInfo.enTranSpeed
                        || HI_STORAGE_TRANSMISSION_SPEED_4_10M == stDevInfo.enTranSpeed) )
                {
                    u32StringID = STRING_SD_SPEED_LOW;
                }
            }
            break;

        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
            u32StringID = STRING_SD_ERROR_CHANGE;
            break;

        case HI_EVENT_STATEMNG_SD_AVAILABLE:
        {
            PDT_UI_WINMNG_FinishWindow(ALARM_WINDOW);
            if (0 == pstEvent->arg1) /*1 : the  event from format sd ; 0 : PowerAction */
            {
                /* SHOW HOME  PAGE */
                if (PDT_UI_WINMNG_WindowIsShow(SET_PAGE))
                {
                    s32Ret = PDT_UI_WINMNG_StartWindow(HOME_PAGE, HI_TRUE);
                    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((s32Ret == HI_SUCCESS), "show home page failed");
                }
            }
        }
        break;

        case HI_EVENT_STATEMNG_SD_UNAVAILABLE:
            u32StringID = STRING_SD_ERROR_FORMAT;
            break;

        default:
            break;

    }

    if (0 == u32StringID)
    {
        return ;
    }

    PDT_UI_COMM_ShowAlarm(u32StringID);

    if ((HI_EVENT_STORAGEMNG_MOUNT_FAILED == pstEvent->EventID) ||
        (HI_EVENT_STORAGEMNG_FS_CHECK_FAILED == pstEvent->EventID)||
        (HI_EVENT_STATEMNG_SD_UNAVAILABLE == pstEvent->EventID))
    {
        s32Ret = HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get start_up source failed");

        if (HI_SYSTEM_STARTUP_SRC_WAKEYP == enStartupSrc)
        {
            MLOGW(" SD error, power off when PowerOn source is wakeup\n");
            PDT_UI_COMM_POWEROFF();
        }
    }
}

/**this is called in UI thread*/
HI_S32 PDT_UI_COMM_EventProc(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {};
    HI_APPCOMM_CHECK_POINTER(pstEvent, -1);
    HI_STORAGEMNG_CFG_S stStorageCfg = {};
    HI_MESSAGE_S stMessage = {};
    HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;
    HI_PDT_WORKMODE_E enWorkMode = HI_PDT_WORKMODE_BUTT;

    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2, pstEvent->s32Result);

    /* process the callback which is given to PDT_UI_COMM_SendAsyncMsg */
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

    /*common event process*/
    switch (pstEvent->EventID)
    {
        case HI_EVENT_USB_INSERT:
            HI_TIME_CLEAN;   /* clean time mark */
            MLOGW("USB INSERT\n");
            return HI_SUCCESS;

        case HI_EVENT_USB_UVC_READY:
            MLOGW("UVC PC Ready\n");
            PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            s32Ret = PDT_UI_COMM_USBSwitchToPrepare(pstEvent);

            if (HI_SUCCESS != s32Ret)
            {
                PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                MLOGE("enter UVC mode failed \n");
                return HIGV_PROC_GOON;
            }

            return HI_SUCCESS;

        case HI_EVENT_USB_STORAGE_READY:
            MLOGW("USB STORAGE PC Ready\n");
            PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            s32Ret = PDT_UI_COMM_USBSwitchToPrepare(pstEvent);

            if (HI_SUCCESS != s32Ret)
            {
                PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                MLOGE("enter usb storage mode failed \n");
                return HIGV_PROC_GOON;
            }

            return HI_SUCCESS;

        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            if (HI_PDT_WORKMODE_USB_STORAGE == pstEvent->arg2
                || HI_PDT_WORKMODE_UVC == pstEvent->arg2)
            {
                s32Ret = PDT_UI_WINMNG_StartWindow(USB_STATE_WINDOW, HI_TRUE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            else if (PDT_UI_WINMNG_WindowIsShow(USB_STATE_WINDOW))
            {
                if (HI_PDT_WORKMODE_PLAYBACK == pstEvent->arg2)
                {
                    PDT_UI_PLAYBACK_VideoPlayerInit(); /* avoid show FILE_DIRECTORY_PAGE, but no init player */
                }

                s32Ret = PDT_UI_WINMNG_FinishWindow(USB_STATE_WINDOW);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            else if (HI_PDT_WORKMODE_UPGRADE == pstEvent->arg2)
            {
                MLOGW("HI_PDT_WORKMODE_UPGRADE \n");
                PDT_UI_COMM_ShowAlarm(STRING_UPGRADE_MODE);

            }

            break;

        case HI_EVENT_USB_OUT:
            HI_TIME_CLEAN;   /* clean  time mark */
            MLOGW("USB OUT\n");
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetWorkModeState");

            if (HI_PDT_WORKMODE_UVC == stWorkState.enWorkMode || HI_PDT_WORKMODE_USB_STORAGE == stWorkState.enWorkMode)
            {
                s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
                if (s32Ret)
                {
                    MLOGE("Window process failed. %x\n", s32Ret);
                }

                stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                stMessage.arg2 = g_enUsbPreviousMode;
                s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                if (s32Ret)
                {
                    MLOGE("SendMessage failed. %x\n", s32Ret);
                    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
            }

            return HI_SUCCESS;

        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STORAGEMNG_DEV_CONNECTING:
        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        case HI_EVENT_STORAGEMNG_FS_CHECKING:
        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case HI_EVENT_STORAGEMNG_MOUNTED:
        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
        case HI_EVENT_STATEMNG_SD_AVAILABLE:
        case HI_EVENT_STATEMNG_SD_UNAVAILABLE:
            PDT_UI_COMM_SDEventProc(pstEvent);
            break;

        case HI_EVENT_STATEMNG_SD_FORMAT:
            break;

        case HI_EVENT_STATEMNG_EMR_BEGIN:
            {
                HI_VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={PDT_UI_VOICE_EMR_START_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=HI_TRUE,
                };
                HI_VOICEPLAY_Push(&stVoice, 0);
                break;
            }

        case HI_EVENT_STATEMNG_VO_SWITCH:
            s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_EVENT_NETCTRL_CLIENT_CONNECTED:
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            g_bSaveCurGridIndex = HI_FALSE;
            s32Ret = PDT_UI_WINMNG_StartWindow(HOME_PAGE, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            if (HI_PDT_WORKMODE_NORM_REC == stWorkState.enWorkMode)
            {
                if (!stWorkState.bRunning)
                {
                    HI_S32 nSndCnt = 0;
                    HI_MESSAGE_S stMessage = {};

                    if (HI_TRUE != stWorkState.bSDAvailable)
                    {
                        MLOGE(" SD state is not normal \n");
                        PDT_UI_COMM_SdIsReady();
                        return HI_FAILURE;
                    }

                    stMessage.what = HI_EVENT_STATEMNG_START;
                    stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
                    /** confirm the start rec cmd can been sent successfully*/
                    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                    {
                        while ((HI_PDT_STATEMNG_EINPROGRESS == s32Ret) && (nSndCnt < 1000))
                        {
                            nSndCnt++;
                            hi_usleep(60 * 1000);
                            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                        }
                    }
                }
            }
            break;

        case HI_EVENT_STATEMNG_SHUTDOWN_PROC:
            {
                s32Ret = PDT_UI_COMM_POWEROFF();
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            return HI_SUCCESS;
        case HI_EVENT_KEYMNG_LONG_CLICK:
            switch (pstEvent->arg1)
            {
                case HI_KEYMNG_KEY_IDX_0:
                    s32Ret = PDT_UI_COMM_POWEROFF();
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                    break;

                case HI_KEYMNG_KEY_IDX_1:
                    break;

                case HI_KEYMNG_KEY_IDX_2:
                    break;

                default:
                    break;
            }

            return HI_SUCCESS;

        case HI_EVENT_UPGRADE_NEWPKT:
            s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_PDT_PARAM_GetStorageCfg");
            s32Ret = HI_STORAGEMNG_GetState(stStorageCfg.szMntPath, &enStorageState);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_STORAGEMNG_GetState");
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);

            if (HI_PDT_WORKMODE_UPGRADE != stWorkState.enWorkMode)
            {
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
                    s32Ret = PDT_UI_WINMNG_FinishWindow(ALARM_WINDOW); /* avoid ALARM page  interference*/
                    s32Ret |= PDT_UI_WINMNG_StartWindow(SD_UPGRADE_WINDOW, HI_TRUE);

                    if (s32Ret)
                    {
                        MLOGE("show sd upgrade window failed \n");
                        return HI_FAILURE;
                    }

                    return HI_SUCCESS;
                }
                else
                {
                    MLOGW("not support SD upgrade in ohter PowerAction mode\n");
                }
            }
            else
            {
                /*apk upgrade*/
                MLOGD("enter board upgrade!\n");
                s32Ret = HI_UPGRADE_DoUpgrade(stStorageCfg.szMntPath, pstEvent->aszPayload);

                if (HI_SUCCESS == s32Ret)
                {
                    MLOGD("Do Upgrade Success, reboot!\n");
                    stMessage.what = HI_EVENT_STATEMNG_SYATEM_REBOOT;
                    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
                else
                {
                    MLOGE("Do Upgrade failed. %x\n", s32Ret);
                    stMessage.what = HI_EVENT_STATEMNG_UPGRADE_FAILED;
                    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
            }

            break;

        case HI_EVENT_UPGRADE_SUCCESS:
            s32Ret = HI_GV_Widget_SetTextByID(SDUpgrade_WINDOW_LABEL_INFO, STRING_UPGRADE_SUCCESS);
            s32Ret |= PDT_UI_WINMNG_StartWindow(SD_Upgrade_result_WINDOW, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_EVENT_UPGRADE_FAILURE:
            s32Ret = HI_GV_Widget_SetTextByID(SDUpgrade_WINDOW_LABEL_INFO, STRING_UPGRADE_FAILED);
            s32Ret |= PDT_UI_WINMNG_StartWindow(SD_Upgrade_result_WINDOW, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_EVENT_STATEMNG_UPGRADE_ABNORMAL:
            MLOGD("HI_EVENT_STATEMNG_UPGRADE_ABNORMAL\n");
            PDT_UI_COMM_ShowAlarm(STRING_UPGRADE_FAILED);
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_REC, &stWorkState);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
            stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
            stMessage.arg2 = enWorkMode;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
#ifdef CONFIG_ACC_ON
        case HI_EVENT_ACCMNG_POWEROFF:
            MLOGI("start to power off!\n");
            s32Ret = PDT_UI_COMM_POWEROFF();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
#endif
        default:
            break;

    }

    /*process msg by pages*/
    if (PDT_UI_WINMNG_WindowIsShow(HOME_PAGE))
    {
        MLOGD("UI current show home_page\n");
        s32Ret = PDT_UI_HOME_EventProc(pstEvent);

        if (s32Ret)
        {
            MLOGE("HOME_EventProc failed. %x\n", s32Ret);
        }
    }
    else if (PDT_UI_WINMNG_WindowIsShow(FILELIST_PAGE))
    {
        MLOGD("UI current show filelist_page\n");
        s32Ret = PDT_UI_FILELIST_EventProc(pstEvent);

        if (s32Ret)
        {
            MLOGE("filelist_EventProc failed. %x\n", s32Ret);
        }
    }
    else if (PDT_UI_WINMNG_WindowIsShow(SD_UPGRADE_WINDOW))
    {
        MLOGD("UI current show SD_upgrade_page\n");
        s32Ret = PDT_UI_SD_UPGRADE_EventProc(pstEvent);

        if (s32Ret)
        {
            MLOGE("SD_Upgrade_EventProc failed. %x\n", s32Ret);
        }
    }

    /* check usbflag  */
    if (HI_USB_MODE_BUTT != s_enUSBRepeatedSendFlg)
    {
        MLOGW(" Reissue  USB message \n");
        HI_EVENT_S stUSBEvent = {};
        if (HI_USB_MODE_UVC == s_enUSBRepeatedSendFlg)
        {
            stUSBEvent.EventID = HI_EVENT_USB_UVC_READY;
        }
        else if (HI_USB_MODE_STORAGE == s_enUSBRepeatedSendFlg)
        {
            stUSBEvent.EventID = HI_EVENT_USB_STORAGE_READY;
        }
        PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
        s32Ret = PDT_UI_COMM_USBSwitchToPrepare(&stUSBEvent);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("Reissue usb message\n");
            return HIGV_PROC_GOON;
        }

    }

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
        HI_EVENT_STATEMNG_EMR_BEGIN,
        HI_EVENT_STATEMNG_EMR_END,
        HI_EVENT_STATEMNG_VO_SWITCH,
        HI_EVENT_STATEMNG_TRIGGER,
        HI_EVENT_STATEMNG_SETTING,
        HI_EVENT_STATEMNG_POWEROFF,
        HI_EVENT_STATEMNG_SD_AVAILABLE,
        HI_EVENT_STATEMNG_SD_UNAVAILABLE,
        HI_EVENT_STATEMNG_SD_FORMAT,
        HI_EVENT_STATEMNG_DEBUG_SWITCH,
        HI_EVENT_STATEMNG_SHUTDOWN_PROC,

        HI_EVENT_PHOTOMNG_TASK_END,
        HI_EVENT_GSENSORMNG_COLLISION,

        HI_EVENT_USB_OUT,
        HI_EVENT_USB_INSERT,
        HI_EVENT_USB_STORAGE_READY,
        HI_EVENT_USB_UVC_READY,

        HI_EVENT_FILEMNG_SCAN_COMPLETED,

        HI_EVENT_STORAGEMNG_DEV_UNPLUGED,
        HI_EVENT_STORAGEMNG_DEV_CONNECTING,
        HI_EVENT_STORAGEMNG_DEV_ERROR,
        HI_EVENT_STORAGEMNG_FS_CHECKING,
        HI_EVENT_STORAGEMNG_FS_CHECK_FAILED,
        HI_EVENT_STORAGEMNG_FS_EXCEPTION,
        HI_EVENT_STORAGEMNG_MOUNTED,
        HI_EVENT_STORAGEMNG_MOUNT_FAILED,

        HI_EVENT_UI_TOUCH,
        HI_EVENT_NETCTRL_CLIENT_CONNECTED,
        HI_EVENT_NETCTRL_CLIENT_DISCONNECTED,

        HI_EVENT_UPGRADE_NEWPKT,
        HI_EVENT_UPGRADE_SUCCESS,
        HI_EVENT_UPGRADE_FAILURE,
        HI_EVENT_STATEMNG_UPGRADE_ABNORMAL,
        HI_EVENT_RECMNG_MUXER_CNT_CHANGE,
        HI_EVENT_STATEMNG_PREVIEW_PIP,
#ifdef CONFIG_GAUGE_ON
        HI_EVENT_GAUGEMNG_LEVEL_CHANGE,
        HI_EVENT_GAUGEMNG_LEVEL_LOW,
        HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW,
        HI_EVENT_GAUGEMNG_LEVEL_NORMAL,
        HI_EVENT_GAUGEMNG_CHARGESTATE_CHANGE,
#endif
#ifdef CONFIG_ACC_ON
        HI_EVENT_ACCMNG_POWEROFF,
#endif
    };



    if (bSubscribeFinish)
    {
        MLOGD("Events has been subscribed\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_EVTHUB_Register(HI_EVENT_UI_TOUCH);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_EVTHUB_CreateSubscriber(&stSubscriber, &SubscriberHdl);

    if (s32Ret)
    {
        MLOGE("CreateSubscriber failed. %#x\n", s32Ret);
        return HI_FAILURE;
    }

    HI_U32 u32ArraySize = UI_ARRAY_SIZE(aEvents);

    for (i = 0; i < u32ArraySize; i++)
    {
        s32Ret = HI_EVTHUB_Subscribe(SubscriberHdl, aEvents[i]);

        if (s32Ret)
        {
            MLOGE("Subscribe Event(%#x) failed. %#x\n", aEvents[i], s32Ret);
            continue;
        }
    }

    bSubscribeFinish = HI_TRUE;
    return HI_SUCCESS;
}

#ifdef CFG_TIME_STAMP_ON
HI_VOID PDT_UI_COMM_SwitchPageTime_begin(HI_VOID)
{
    HI_TIME_CLEAN;
    HI_TIME_STAMP;
    memset(&stUITimespecBegin, 0, sizeof(stUITimespecBegin));
    (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &stUITimespecBegin);
}

HI_VOID PDT_UI_COMM_SwitchPageTime_end(HI_VOID)
{
    if (!(0 == stUITimespecBegin.tv_sec && 0 == stUITimespecBegin.tv_nsec))
    {
        memset(&stUITimespecEnd, 0, sizeof(stUITimespecEnd));
        (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &stUITimespecEnd);

        HI_U32 u32WaitTime_ms = (stUITimespecEnd.tv_sec * 1000 + stUITimespecEnd.tv_nsec / 1000000)
                                - (stUITimespecBegin.tv_sec * 1000 + stUITimespecBegin.tv_nsec / 1000000);
        MLOGI(GREEN"%s %dms\n"NONE, " switch ui_page cost time" , u32WaitTime_ms);

        memset(&stUITimespecBegin, 0, sizeof(stUITimespecBegin));
        memset(&stUITimespecEnd, 0, sizeof(stUITimespecEnd));
        HI_TIME_STAMP;
        HI_TIME_PRINT;
        HI_TIME_CLEAN;
    }
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

