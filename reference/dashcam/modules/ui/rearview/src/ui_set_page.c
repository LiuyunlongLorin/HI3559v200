/**
* @file    ui_set_page.c
* @brief   ui set page. show all settings.
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/14
* @version   1.0

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ui_common.h"
#include "hi_voiceplay.h"
#include "hi_product_media.h"
#include "hi_timedtask.h"
#include "hi_powercontrol.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define SETTING_STRING_SIZE  (32)

typedef enum tagPDT_UI_SET_ITEM_E
{
    PDT_UI_SET_MEDIA_MODE,
    PDT_UI_SET_VENC_TYPE,
    PDT_UI_SET_AUDIO_STATUS,
    PDT_UI_SET_FLIP_STATUS,
    PDT_UI_SET_MIRROR_STATUS,
    PDT_UI_SET_LDC_STATUS,
    PDT_UI_SET_TIME_OSD_STATUS,
    PDT_UI_SET_VOLUME,
    PDT_UI_SET_USBMODE,
    PDT_UI_SET_BOOTSOUND,
    PDT_UI_SET_WIFI_STATUS,
    PDT_UI_SET_TIME_SETTING,
    PDT_UI_SET_RESET_FACTORY,
    PDT_UI_SET_FORMAT_SD,
    PDT_UI_SET_PARKING,
    PDT_UI_SET_DEBUG,
    PDT_UI_SET_SENSITIVITY,
    PDT_UI_SET_ABOUT,
    PDT_UI_SET_BACK,
    PDT_UI_SET_LANGUAGE,
    PDT_UI_SET_BACK_REC,
    PDT_UI_SET_VOSWITCH,
    PDT_UI_SET_WDR,
    PDT_UI_SET_SPLITTIME,
    PDT_UI_SET_BRECORD_TYPE,
    PDT_UI_SET_LAPSE_INTERVAL,
    PDT_UI_SET_PREVIEWPIP,
    PDT_UI_SET_VIDEOMODE,
    PDT_UI_SET_SCREEN_DORMANT,
    PDT_UI_SET_WORK_MODE,
} PDT_UI_SET_ITEM_E;

typedef struct tagPDT_UI_SET_ROW_S
{
    HI_S32  s32Item;
    HI_S32  s32ParamType;
    HI_CHAR szIconPath[HI_APPCOMM_MAX_PATH_LEN];
    HI_CHAR szDesc[SETTING_STRING_SIZE];
    HI_CHAR szValue[SETTING_STRING_SIZE];
} PDT_UI_SET_ROW_S;

typedef struct tagPDT_UI_SET_DATA_S
{
    HI_S32 s32Item;
    HI_S32 s32ParamType;
    HI_U32 u32StringID;
} PDT_UI_SET_DATA_S;

static HI_S32 s_s32DebugOn = 0;
static HI_S32  s_s32CurGridIndex = 0;
HI_BOOL g_bSaveCurGridIndex = HI_FALSE;


static PDT_UI_SET_DATA_S s_stRecordSetItems[]= {
    {PDT_UI_SET_WORK_MODE,          HI_PDT_PARAM_TYPE_BUTT,                 STRING_WORK_MODE},
    {PDT_UI_SET_VOSWITCH,           HI_PDT_PARAM_TYPE_BUTT,                 STRING_VOSWITCH},
    {PDT_UI_SET_MEDIA_MODE,         HI_PDT_PARAM_TYPE_MEDIAMODE,            STRING_RESOLUTION},
    {PDT_UI_SET_VENC_TYPE,          HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE,     STRING_VIDEO_ENCODE},
    {PDT_UI_SET_FLIP_STATUS,        HI_PDT_PARAM_TYPE_FLIP,                 STRING_FLIP},
    {PDT_UI_SET_MIRROR_STATUS,      HI_PDT_PARAM_TYPE_MIRROR,               STRING_MIRROR},
    {PDT_UI_SET_LDC_STATUS,         HI_PDT_PARAM_TYPE_LDC,                  STRING_LDC},
    {PDT_UI_SET_TIME_OSD_STATUS,    HI_PDT_PARAM_TYPE_OSD,                  STRING_OSD},
    {PDT_UI_SET_WDR,                HI_PDT_PARAM_TYPE_WDR,                  STRING_WDR},
    {PDT_UI_SET_BACK,               HI_PDT_PARAM_TYPE_BUTT,                 STRING_BACK},

    {PDT_UI_SET_BACK_REC,           HI_PDT_PARAM_TYPE_REC,                  STRING_BACK_REC},
    {PDT_UI_SET_VIDEOMODE,          HI_PDT_PARAM_TYPE_VIDEOMODE,            STRING_VIDEOMODE},
    {PDT_UI_SET_SPLITTIME,          HI_PDT_PARAM_TYPE_SPLITTIME,            STRING_SPLITTIME},
    {PDT_UI_SET_BRECORD_TYPE,       HI_PDT_PARAM_RECORD_TYPE,               STRING_REC_TYPE},
    {PDT_UI_SET_LAPSE_INTERVAL,     HI_PDT_PARAM_TYPE_LAPSE_INTERVAL,       STRING_LAPSE_REC_TIME},
    {PDT_UI_SET_USBMODE,            HI_PDT_PARAM_TYPE_USB_MODE,             STRING_SELECT_USB_MODE},
    {PDT_UI_SET_TIME_SETTING,       HI_PDT_PARAM_TYPE_BUTT,                 STRING_TIME_SETTING},
    {PDT_UI_SET_RESET_FACTORY,      HI_PDT_PARAM_TYPE_BUTT,                 STRING_RESET_FACTORY},
    {PDT_UI_SET_FORMAT_SD,          HI_PDT_PARAM_TYPE_BUTT,                 STRING_FORMAT_SD},
    {PDT_UI_SET_PARKING,            HI_PDT_PARAM_TYPE_GSENSOR_PARKING,      STRING_PARKING},
    {PDT_UI_SET_SENSITIVITY,        HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY,  STRING_SENSITIVITY},
    {PDT_UI_SET_AUDIO_STATUS,       HI_PDT_PARAM_TYPE_AUDIO,                STRING_AUDIO},
    {PDT_UI_SET_VOLUME,             HI_PDT_PARAM_TYPE_VOLUME,               STRING_VOLUME},

#ifdef CONFIG_WIFI_ON
    {PDT_UI_SET_WIFI_STATUS,        HI_PDT_PARAM_TYPE_BUTT,                 STRING_WIFI},
#endif
#ifdef CONFIG_RAWCAP_ON
    {PDT_UI_SET_DEBUG,              HI_PDT_PARAM_TYPE_BUTT,                 STRING_DEBUG},
#endif
    {PDT_UI_SET_SCREEN_DORMANT,     HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE,  STRING_SCREEN_DORMANT},
    {PDT_UI_SET_PREVIEWPIP,         HI_PDT_PARAM_TYPE_BUTT,                 STRING_PREVIEWPIP},
    {PDT_UI_SET_BOOTSOUND,          HI_PDT_PARAM_TYPE_BOOTSOUND,            STRING_BOOTSOUND},
    {PDT_UI_SET_LANGUAGE,           HI_PDT_PARAM_TYPE_LANGUAGE,             STRING_LANGUAGE},
    {PDT_UI_SET_ABOUT,              HI_PDT_PARAM_TYPE_BUTT,                 STRING_ABOUT},
};

static PDT_UI_SET_DATA_S s_stPhotoSetItems[]= {
    {PDT_UI_SET_WORK_MODE,          HI_PDT_PARAM_TYPE_BUTT,                 STRING_WORK_MODE},
    {PDT_UI_SET_VOSWITCH,           HI_PDT_PARAM_TYPE_BUTT,                 STRING_VOSWITCH},
    {PDT_UI_SET_MEDIA_MODE,         HI_PDT_PARAM_TYPE_MEDIAMODE,            STRING_RESOLUTION},
    {PDT_UI_SET_FLIP_STATUS,        HI_PDT_PARAM_TYPE_FLIP,                 STRING_FLIP},
    {PDT_UI_SET_MIRROR_STATUS,      HI_PDT_PARAM_TYPE_MIRROR,               STRING_MIRROR},
    {PDT_UI_SET_LDC_STATUS,         HI_PDT_PARAM_TYPE_LDC,                  STRING_LDC},
    {PDT_UI_SET_TIME_OSD_STATUS,    HI_PDT_PARAM_TYPE_OSD,                  STRING_OSD},
    {PDT_UI_SET_USBMODE,            HI_PDT_PARAM_TYPE_USB_MODE,             STRING_SELECT_USB_MODE},
    {PDT_UI_SET_TIME_SETTING,       HI_PDT_PARAM_TYPE_BUTT,                 STRING_TIME_SETTING},

    {PDT_UI_SET_BACK,               HI_PDT_PARAM_TYPE_BUTT,                 STRING_BACK},
    {PDT_UI_SET_RESET_FACTORY,      HI_PDT_PARAM_TYPE_BUTT,                 STRING_RESET_FACTORY},
    {PDT_UI_SET_FORMAT_SD,          HI_PDT_PARAM_TYPE_BUTT,                 STRING_FORMAT_SD},
    {PDT_UI_SET_PARKING,            HI_PDT_PARAM_TYPE_GSENSOR_PARKING,      STRING_PARKING},
    {PDT_UI_SET_SENSITIVITY,        HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY,  STRING_SENSITIVITY},
    {PDT_UI_SET_VOLUME,             HI_PDT_PARAM_TYPE_VOLUME,               STRING_VOLUME},

#ifdef CONFIG_WIFI_ON
    {PDT_UI_SET_WIFI_STATUS,        HI_PDT_PARAM_TYPE_BUTT,                 STRING_WIFI},
#endif
    {PDT_UI_SET_SCREEN_DORMANT,     HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE,  STRING_SCREEN_DORMANT},
    {PDT_UI_SET_PREVIEWPIP,         HI_PDT_PARAM_TYPE_BUTT,                 STRING_PREVIEWPIP},
    {PDT_UI_SET_BOOTSOUND,          HI_PDT_PARAM_TYPE_BOOTSOUND,            STRING_BOOTSOUND},
    {PDT_UI_SET_LANGUAGE,           HI_PDT_PARAM_TYPE_LANGUAGE,             STRING_LANGUAGE},
    {PDT_UI_SET_ABOUT,              HI_PDT_PARAM_TYPE_BUTT,                 STRING_ABOUT},
};


static HI_S32 UI_SET_OnReceiveMsgResult(HI_EVENT_S* pstEvent)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    /* avoid  voswitch  from touch */
    if (PDT_UI_WINMNG_WindowIsShow(HOME_PAGE))
    {
        return HI_SUCCESS;
    }

    PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);

    s32Ret = HI_GV_ScrollGrid_SetSelCell(SET_SCROLLGRID, s_s32CurGridIndex);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ScrollGrid_SetSelCell");

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_SETTINGS);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    return HI_SUCCESS;
}

HI_S32 PDT_UI_SET_Volume(HI_S32 s32Volume)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_PDT_PARAM_MEDIA_COMM_CFG_S stMediaCommCfg = {};

    s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_VOLUME, &s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stMediaCommCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s32Ret = HI_PDT_MEDIA_SetAoVolume(
                 stMediaCommCfg.stAudioOutCfg.astAoCfg[0].AoHdl, s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return s32Ret;
}

HI_S32 PDT_UI_SET_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    if(!g_bSaveCurGridIndex)
    {
        s_s32CurGridIndex = 0;
        g_bSaveCurGridIndex = HI_FALSE;
    }
    s32Ret = HI_GV_Widget_Active(SET_SCROLLGRID);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
    s32Ret = HI_GV_ScrollGrid_SetSelCell(SET_SCROLLGRID, s_s32CurGridIndex);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ScrollGrid_SetSelCell");

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_SETTINGS);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

#ifdef  CFG_TIME_STAMP_ON
    PDT_UI_COMM_SwitchPageTime_end();
#endif
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SET_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("SET_PAGE OnHide\n");
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SET_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

static HI_S32 PDT_UI_SET_Back(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    g_bSaveCurGridIndex = HI_FALSE;

#ifdef CFG_TIME_STAMP_ON
    PDT_UI_COMM_SwitchPageTime_begin();
#endif

    s32Ret = PDT_UI_WINMNG_FinishWindow(SET_PAGE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s_s32CurGridIndex = 0;

    HI_EVENT_S stEvent = {0};
    stEvent.arg1 = 0;
    s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_RECMNG_MUXER_CNT_CHANGE, &stEvent);
    if((HI_SUCCESS != s32Ret) ||(-1 == stEvent.arg1))
    {
        if(!PDT_UI_COMM_SdIsReady())
        {
            MLOGE(" SD state is not normal \n");
            return HI_FAILURE;
        }

        HI_MESSAGE_S stMessage = {};
        stMessage.what = HI_EVENT_STATEMNG_START;
        stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else
    {
        MLOGI("task[%d] is not zero ,can not start rec!\n",stEvent.arg1);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_SET_OnGridSelect(HI_U32 u32GridIndex)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    HI_MESSAGE_S stMessage = {};
    HI_CHAR * pcMsgPayload = (HI_CHAR*)(stMessage.aszPayload);
    HI_PDT_ITEM_VALUESET_S stValueSet = {};
    HI_S32 s32CamID = 0;
    HI_S32 s32CurValue = 0;
    HI_S32 s32NewValue = 0;
    HI_CHAR szLanId[SETTING_CONTENT_SIZE];

    s32CamID = HI_PDT_STATEMNG_GetPreViewCamID();

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    PDT_UI_SET_DATA_S  *pstSetItems;
    HI_S32 s32SetItemsCnt = 0;

    if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
    {
        s32SetItemsCnt = UI_ARRAY_SIZE(s_stPhotoSetItems);
        pstSetItems = s_stPhotoSetItems;
    }
    else
    {
        s32SetItemsCnt = UI_ARRAY_SIZE(s_stRecordSetItems);
        pstSetItems = s_stRecordSetItems;
    }

    if( 0 == s32SetItemsCnt || u32GridIndex >= s32SetItemsCnt)
    {
        MLOGE(" select index [%d]\n",u32GridIndex);
        return HI_SUCCESS;
    }

    s_s32CurGridIndex = u32GridIndex;
    MLOGD("GridIndex = %u\n", u32GridIndex);

    switch(pstSetItems[u32GridIndex].s32Item)
    {

        case PDT_UI_SET_TIME_SETTING:
            g_bSaveCurGridIndex = HI_TRUE;
            s32Ret = PDT_UI_WINMNG_StartWindow(TIME_SET_WINDOW, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        case PDT_UI_SET_FORMAT_SD:
        {
            HI_STORAGEMNG_CFG_S stStorageCfg = {};
            HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;

            s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s32Ret = HI_STORAGEMNG_GetState(stStorageCfg.szMntPath, &enStorageState);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            if(HI_STORAGE_STATE_DEV_UNPLUGGED == enStorageState)
            {
                PDT_UI_COMM_ShowAlarm(STRING_NO_SD_CARD);
                MLOGD("sd not exit, can not format sd!\n");
            }
            else
            {

                g_bSaveCurGridIndex = HI_TRUE;
                s32Ret = PDT_UI_WINMNG_StartWindow(SD_FORMAT_WINDOW, HI_TRUE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
        }
            break;

        case PDT_UI_SET_RESET_FACTORY:
            g_bSaveCurGridIndex = HI_TRUE;
            s32Ret = PDT_UI_WINMNG_StartWindow(RESET_WINDOW, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        case PDT_UI_SET_ABOUT:
            g_bSaveCurGridIndex = HI_TRUE;
            s32Ret = PDT_UI_WINMNG_StartWindow(ABOUT_WINDOW, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        case PDT_UI_SET_BACK:
            s32Ret = PDT_UI_SET_Back();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        case PDT_UI_SET_FLIP_STATUS:
        case PDT_UI_SET_MIRROR_STATUS:
        case PDT_UI_SET_MEDIA_MODE:
        case PDT_UI_SET_WDR:
        case PDT_UI_SET_LDC_STATUS:
        case PDT_UI_SET_LAPSE_INTERVAL:
        case PDT_UI_SET_BRECORD_TYPE:
        case PDT_UI_SET_VIDEOMODE:
        case PDT_UI_SET_SPLITTIME:
            if(PDT_UI_SET_LAPSE_INTERVAL == pstSetItems[u32GridIndex].s32Item)
            {
                HI_REC_TYPE_E enRecType;
                s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, s32CamID, HI_PDT_PARAM_RECORD_TYPE, &enRecType);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCamParam");

                if(HI_REC_TYPE_LAPSE != enRecType)
                {
                    MLOGI("normal record type  unsupport change lapse_interval\n");
                    return HI_SUCCESS;
                }
            }

            s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode,
                s32CamID, pstSetItems[u32GridIndex].s32ParamType, &s32CurValue);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s32Ret = HI_PDT_PARAM_GetCamItemValues(stWorkModeState.enWorkMode, s32CamID, pstSetItems[u32GridIndex].s32ParamType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            if(stValueSet.s32Cnt<=1)
            {
                MLOGE("only 1 value, not set\n");
                return HI_SUCCESS;
            }

            stMessage.what = HI_EVENT_STATEMNG_SETTING;
            stMessage.arg1 = pstSetItems[u32GridIndex].s32ParamType;
            stMessage.arg2 = stWorkModeState.enWorkMode;

            s32NewValue = stValueSet.astValues[0].s32Value;
            for(i=0; i<stValueSet.s32Cnt && i<HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
            {
                if(s32CurValue == stValueSet.astValues[i].s32Value)
                {
                    s32NewValue = stValueSet.astValues[ (i+1)%stValueSet.s32Cnt ].s32Value ;
                    break;
                }
            }
            *(HI_PDT_MEDIAMODE_E *)pcMsgPayload  = s32NewValue;

            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            MLOGD("Setting -----> [mode:%d, type:%d, value:%d]\n", stMessage.arg2, stMessage.arg1, s32NewValue);
            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, UI_SET_OnReceiveMsgResult);
            if(s32Ret)
            {
                MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                return HI_FAILURE;
            }
            return HI_SUCCESS;


        case PDT_UI_SET_VENC_TYPE:
            s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode,
                s32CamID, pstSetItems[u32GridIndex].s32ParamType, &s32CurValue);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetCamParam");

            stMessage.what = HI_EVENT_STATEMNG_SETTING;
            stMessage.arg1 = pstSetItems[u32GridIndex].s32ParamType;
            stMessage.arg2 = stWorkModeState.enWorkMode;


            if(HI_MAPI_PAYLOAD_TYPE_H264 == (HI_MAPI_PAYLOAD_TYPE_E)s32CurValue)
            {
                s32NewValue = HI_MAPI_PAYLOAD_TYPE_H265;
            }
            else
            {
                s32NewValue = HI_MAPI_PAYLOAD_TYPE_H264;
            }
            *(HI_MAPI_PAYLOAD_TYPE_E *)pcMsgPayload = (HI_MAPI_PAYLOAD_TYPE_E)s32NewValue;

            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            MLOGD("Setting -----> [mode:%d, type:%d, value:%d]\n", stMessage.arg2, stMessage.arg1, s32NewValue);
            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, UI_SET_OnReceiveMsgResult);
            if(s32Ret)
            {
                MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                return HI_FAILURE;
            }
            return HI_SUCCESS;

        case PDT_UI_SET_TIME_OSD_STATUS:
        case PDT_UI_SET_AUDIO_STATUS:
        case PDT_UI_SET_BACK_REC:
            if(PDT_UI_SET_AUDIO_STATUS == pstSetItems[u32GridIndex].s32Item)
            {
                s32Ret = HI_PDT_PARAM_GetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s32CurValue);
            }
            else if(PDT_UI_SET_BACK_REC == pstSetItems[u32GridIndex].s32Item)
            {
                s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, BACK_CAM_ID, pstSetItems[u32GridIndex].s32ParamType, &s32CurValue);
            }
            else
            {
                s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, pstSetItems[u32GridIndex].s32ParamType, &s32CurValue);
            }
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            stMessage.what = HI_EVENT_STATEMNG_SETTING;
            stMessage.arg1 = pstSetItems[u32GridIndex].s32ParamType;
            stMessage.arg2 = stWorkModeState.enWorkMode;

            s32NewValue = (HI_TRUE == (HI_BOOL)s32CurValue) ? HI_FALSE : HI_TRUE;

            *(HI_BOOL *)pcMsgPayload = (HI_BOOL)s32NewValue;

            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            MLOGD("Setting -----> [mode:%d, type:%d, value:%d]\n", stMessage.arg2, stMessage.arg1, s32NewValue);
            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, UI_SET_OnReceiveMsgResult);
            if(s32Ret)
            {
                MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                return HI_FAILURE;
            }
            return HI_SUCCESS;

        case PDT_UI_SET_VOLUME:
            s32Ret = HI_PDT_PARAM_GetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s32CurValue);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetCommParam");

            s32Ret = HI_PDT_PARAM_GetCommItemValues(pstSetItems[u32GridIndex].s32ParamType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetCommItemValues");

            for(i=0; i<stValueSet.s32Cnt && i<HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
            {
                if (s32CurValue <=  stValueSet.astValues[i].s32Value)
                {
                    s32NewValue = stValueSet.astValues[(i+1)%stValueSet.s32Cnt].s32Value;
                    break;
                }
            }

            if (s32CurValue > stValueSet.astValues[stValueSet.s32Cnt - 1].s32Value)
            {
                s32NewValue = stValueSet.astValues[0].s32Value;
            }

            s32Ret = PDT_UI_SET_Volume(s32NewValue);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            break;

        case PDT_UI_SET_BOOTSOUND:
            s32Ret = HI_PDT_PARAM_GetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s32CurValue);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s32NewValue = (HI_TRUE == (HI_BOOL)s32CurValue) ? HI_FALSE : HI_TRUE;
            s32Ret = HI_PDT_PARAM_SetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s32NewValue);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        case PDT_UI_SET_PARKING:
        case PDT_UI_SET_SENSITIVITY:
        case PDT_UI_SET_USBMODE:
            s32Ret = HI_PDT_PARAM_GetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s32CurValue);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s32Ret = HI_PDT_PARAM_GetCommItemValues(pstSetItems[u32GridIndex].s32ParamType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            for (i = 0; i < stValueSet.s32Cnt && i < HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
            {
                if (s32CurValue == stValueSet.astValues[i].s32Value)
                {
                    s32NewValue = stValueSet.astValues[(i + 1) % stValueSet.s32Cnt].s32Value;
                    break;
                }
            }

            if (i == stValueSet.s32Cnt)
            {
                MLOGE("not found value %d in valueset\n", s32CurValue);
                return HI_FAILURE;
            }

            if (PDT_UI_SET_SENSITIVITY == pstSetItems[u32GridIndex].s32Item)
            {
                s32Ret = HI_GSENSORMNG_SetSensitity(s32NewValue);
                s32Ret |= HI_PDT_PARAM_SetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s32NewValue);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            else if (PDT_UI_SET_PARKING == pstSetItems[u32GridIndex].s32Item )
            {
                s32Ret = HI_PDT_PARAM_SetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s32NewValue);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            }
            else if (PDT_UI_SET_USBMODE == pstSetItems[u32GridIndex].s32Item)
            {
                HI_USB_MODE_E enUsbMode = HI_USB_MODE_CHARGE;
                s32Ret = HI_USB_GetMode(&enUsbMode);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"get usb mode");

                HI_USB_STATE_E enUsbState = HI_USB_STATE_OUT;
                s32Ret = HI_USB_GetState(&enUsbState);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"get usb state");

                if ((HI_USB_MODE_CHARGE != enUsbMode) && (HI_USB_STATE_OUT != enUsbState))
                {
                    MLOGW("current usb_mode[%d] usb_state[%d]; can not set usb mode!\n", enUsbMode, enUsbState);
                    return HI_SUCCESS;
                }

                s32Ret = HI_USB_SetMode((HI_USB_MODE_E)s32NewValue);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                if (HI_SUCCESS == s32Ret)
                {
                    s32Ret = HI_PDT_PARAM_SetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s32NewValue);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
            }

            break;

#ifdef CONFIG_WIFI_ON
        case PDT_UI_SET_WIFI_STATUS:

            s32Ret = HI_HAL_WIFI_GetStartedStatus((HI_BOOL*)&s32CurValue);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            if(HI_FALSE == s32CurValue)
            {
                HI_HAL_WIFI_APMODE_CFG_S stApCfg = {};
                HI_HAL_WIFI_CFG_S stCfg = {};

                stCfg.enMode = HI_HAL_WIFI_MODE_AP;
                s32Ret = HI_HAL_WIFI_Init(stCfg.enMode);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

                /* after init wifi,do get ap param and start wifi */
                s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stApCfg);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                memcpy(&stCfg.unCfg.stApCfg, &stApCfg, sizeof(HI_HAL_WIFI_APMODE_CFG_S));
                s32Ret = HI_HAL_WIFI_Start(&stCfg);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

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
                HI_APPCOMM_CHECK_RETURN(s32Ret,HI_FAILURE);
                MLOGD("HI_HAL_WIFI_Stop\n");

                s32Ret = HI_HAL_WIFI_Deinit();
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

                HI_VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={PDT_UI_VOICE_WIFI_IDX,PDT_UI_VOICE_CLOSE_IDX},
                    .u32VoiceCnt=2,
                    .bDroppable=HI_TRUE,
                };
                HI_VOICEPLAY_Push(&stVoice, 0);
            }
            break;
#endif

#ifdef CONFIG_RAWCAP_ON
        case PDT_UI_SET_DEBUG:
            memset(&stMessage, 0x0, sizeof(HI_MESSAGE_S));
            stMessage.what = HI_EVENT_STATEMNG_DEBUG_SWITCH;
            stMessage.arg2 = stWorkModeState.enWorkMode;

            s_s32DebugOn = (s_s32DebugOn == 0 ? 1 : 0);
            HI_CHAR* payload = (stMessage.aszPayload);
            *(HI_S32 *)payload = s_s32DebugOn;

            PDT_UI_WINMNG_StartWindow(BUSY_PAGE,HI_FALSE);
            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage,UI_SET_OnReceiveMsgResult);
            if(HI_SUCCESS != s32Ret)
            {
                MLOGE("SendMessage failed. %x\n", s32Ret);
                PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                s_s32DebugOn = (s_s32DebugOn == 0 ? 1 : 0);
                return HI_FAILURE;
            }
            return HI_SUCCESS;
#endif

        case PDT_UI_SET_LANGUAGE:
        {
            s32Ret = HI_PDT_PARAM_GetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s32CurValue);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s32NewValue = (HI_TRUE == (HI_BOOL)s32CurValue) ? HI_FALSE : HI_TRUE;
            s32Ret = HI_PDT_PARAM_SetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s32NewValue);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            HI_CHAR szContent[SETTING_CONTENT_SIZE] = {0};
            HI_S32 s32CamID = 0;

            s32CamID = HI_PDT_STATEMNG_GetPreViewCamID();
            s32Ret = PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_BUTT, s32CamID, HI_PDT_PARAM_TYPE_LANGUAGE,
                                                   szContent, SETTING_CONTENT_SIZE);

            s32Ret = PDT_UI_COMM_GetStringByString(szContent, szLanId, SETTING_CONTENT_SIZE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            MLOGI("szLanId:%s\n", szLanId);

            s32Ret = HI_GV_Lan_Change(szLanId);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
            break;

        case PDT_UI_SET_VOSWITCH:
            s32Ret = PDT_UI_SET_VoSwitch();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            return HI_SUCCESS;
        case PDT_UI_SET_WORK_MODE:
            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            if (s32Ret)
            {
                MLOGE("Window process failed. %x\n", s32Ret);
            }

            stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;

            if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode)
            {
                stMessage.arg2 = HI_PDT_WORKMODE_PHOTO;
            }
            else
            {
                stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
            }

            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, UI_SET_OnReceiveMsgResult);
            if(s32Ret)
            {
                MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "close busy page");
            }
            return HI_SUCCESS;

        case PDT_UI_SET_PREVIEWPIP:
        {
            HI_PDT_PARAM_MEDIA_COMM_CFG_S stMediaCommCfg = {};
            s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stMediaCommCfg);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_PDT_PARAM_GetMediaCommCfg");
            if(stMediaCommCfg.stVideoOutCfg.astDispCfg[0].u32WindowCnt <= 1)
            {
                MLOGW("preview pipe can not support!!\n");
                return HI_SUCCESS;
            }
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PREVIEWPIP,&s32CurValue);
            s32NewValue = (HI_TRUE == (HI_BOOL)s32CurValue) ? HI_FALSE : HI_TRUE;
            memset(&stMessage, 0, sizeof(stMessage));
            stMessage.what = HI_EVENT_STATEMNG_PREVIEW_PIP;
            stMessage.arg2 = stWorkModeState.enWorkMode;
            *(HI_BOOL*)pcMsgPayload = (HI_BOOL)s32NewValue;

            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "show busy page");

            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, UI_SET_OnReceiveMsgResult);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "close busy page");
                return HI_FAILURE;
            }
            return HI_SUCCESS;
        }
        break;

            case PDT_UI_SET_SCREEN_DORMANT:
            {
                HI_TIMEDTASK_ATTR_S s_ScreenDormantAttr = {0};
                s32Ret = HI_PDT_PARAM_GetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s_ScreenDormantAttr);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get comm param failed\n");
                s32Ret = HI_PDT_PARAM_GetCommItemValues(pstSetItems[u32GridIndex].s32ParamType, &stValueSet);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get comm Item values failed\n");

                for (i = 0; i < stValueSet.s32Cnt && i < HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
                {
                    if (s_ScreenDormantAttr.u32Time_sec == stValueSet.astValues[i].s32Value)
                    {
                        s_ScreenDormantAttr.u32Time_sec = stValueSet.astValues[(i + 1) % stValueSet.s32Cnt].s32Value;
                        s_ScreenDormantAttr.bEnable = s_ScreenDormantAttr.u32Time_sec > 0 ? HI_TRUE : HI_FALSE;
                        break;
                    }
                }

                if (i == stValueSet.s32Cnt)
                {
                    MLOGE("not found value %d in valueset\n", s_ScreenDormantAttr.u32Time_sec);
                    return HI_FAILURE;
                }

                HI_POWERCTRL_TASK_E enPwrCtrlType = HI_PWRCTRL_TASK_SCREENDORMANT;
                s32Ret = HI_POWERCTRL_SetTaskAttr(enPwrCtrlType, &s_ScreenDormantAttr);
                if (s32Ret)
                {
                    MLOGE("SetTaskAttr screen_dormant failed");
                    return HI_FAILURE;
                }

                s32Ret = HI_PDT_PARAM_SetCommParam(pstSetItems[u32GridIndex].s32ParamType, &s_ScreenDormantAttr);
                if (s32Ret)
                {
                    MLOGE("SetCommParam screen_dormant failed\n");
                }
            }
            break;

        default:
            MLOGE("not support the item\n");
            break;
    }

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_SETTINGS);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_SET_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("WidgetHdl: %u, wParam: %#lx, lParam = %#lx\n", WidgetHdl, (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32SetItemsCnt = 0;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
    {
        s32SetItemsCnt = UI_ARRAY_SIZE(s_stPhotoSetItems);
    }
    else
    {
        s32SetItemsCnt = UI_ARRAY_SIZE(s_stRecordSetItems);
    }

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_SET_Back();
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_SET_Back");
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        s_s32CurGridIndex = (s_s32CurGridIndex+1) % s32SetItemsCnt;
        s32Ret = HI_GV_ADM_Sync(DATAMODEL_SETTINGS);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ADM_Sync");
        s32Ret = HI_GV_ScrollGrid_SetSelCell(SET_SCROLLGRID, s_s32CurGridIndex);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ScrollGrid_SetSelCell");
    }
    else if (PDT_UI_KEY_EXPAND == wParam)
    {
        s_s32CurGridIndex = s_s32CurGridIndex ? (s_s32CurGridIndex - 1) : (s32SetItemsCnt - 1);
        s32Ret = HI_GV_ADM_Sync(DATAMODEL_SETTINGS);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ADM_Sync");
        s32Ret = HI_GV_ScrollGrid_SetSelCell(SET_SCROLLGRID, s_s32CurGridIndex);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ScrollGrid_SetSelCell");
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        s32Ret = PDT_UI_SET_OnGridSelect(s_s32CurGridIndex);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_SET_OnClick");
    }

    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_SET_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SET_GRID_OnCellColSelect(HIGV_HANDLE WidgetHdl, HI_PARAM u32GridIndex, HI_PARAM u32ChildIndex)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("WidgetHdl: %u, u32GridIndex: %#lx, u32ChildIndex = %#lx\n", WidgetHdl, (HI_UL)u32GridIndex, (HI_UL)u32ChildIndex);

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
    {
        if (u32GridIndex >= UI_ARRAY_SIZE(s_stPhotoSetItems))
        {
            MLOGI("clicking out of the range\n");
            return HIGV_PROC_GOON;
        }
    }
    else
    {
        if (u32GridIndex >= UI_ARRAY_SIZE(s_stRecordSetItems))
        {
            MLOGI("clicking out of the range\n");
            return HIGV_PROC_GOON;
        }
    }



    s_s32CurGridIndex = u32GridIndex;
    s32Ret = HI_GV_ADM_Sync(DATAMODEL_SETTINGS);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ADM_Sync");
    s32Ret = HI_GV_ScrollGrid_SetSelCell(SET_SCROLLGRID, s_s32CurGridIndex);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ScrollGrid_SetSelCell");

    s32Ret = PDT_UI_SET_OnGridSelect(s_s32CurGridIndex);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_SET_OnClick");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SET_GRID_OnFocusMove(HIGV_HANDLE WidgetHdl, HI_PARAM u32GridIndex, HI_PARAM lParam)
{
    MLOGD("GridIndex = %lu, lParam = %lu\n", (HI_UL)u32GridIndex, (HI_UL)lParam);

    return HIGV_PROC_GOON;
}

/**
 * @brief  given data from row value,  output  FOCUS state image's path.
 * @param u32GridChild  grid's child's index.
 * @param u32Grid  grid index
 * @param pSrcStr  we don't use it.
 * @param pDstStr  output FOCUS box image's path
 * @return HI_SUCCESS, HI_FAILURE
 */
HI_S32 PDT_UI_SET_GRID_GetFocusIcon(HI_HANDLE GridHdl, HI_U32 u32GridChild, HI_U32 u32Grid,
                                        const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 u32Length)
{
    if (s_s32CurGridIndex == u32Grid)
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/set_focus_box.png");
    }
    else
    {
        snprintf(pDstStr, u32Length, "%s", "");
    }

    return HI_SUCCESS;
}


HI_S32 PDT_UI_SET_GRID_GetIcon(HI_HANDLE GridHdl, HI_U32 u32GridChild, HI_U32 u32Grid,
                                        const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 u32Length)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Value = 0;
    HI_S32 s32CamID = 0;

    if (HI_NULL == pSrcStr || HI_NULL == pDstStr)
    {
        MLOGE("Null Pointer\n");
        return HI_FAILURE;
    }

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_U32 u32GridIndex;
    u32GridIndex = atoi(pSrcStr);

    s32CamID = HI_PDT_STATEMNG_GetPreViewCamID();

    switch(u32GridIndex)
    {
        case PDT_UI_SET_MEDIA_MODE :
        {
            s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, HI_PDT_PARAM_TYPE_MEDIAMODE, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetWorkModeParam");
            switch((HI_PDT_MEDIAMODE_E)s32Value)
            {
                case HI_PDT_MEDIAMODE_1080P_30:
                case HI_PDT_MEDIAMODE_1080P_25:
                case HI_PDT_MEDIAMODE_PHOTO_1080P:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/1080P30.png");
                    break;
                case HI_PDT_MEDIAMODE_2160P_30:
                case HI_PDT_MEDIAMODE_PHOTO_2160P:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/2160P30.png");
                    break;
                case HI_PDT_MEDIAMODE_1440P_30:
                case HI_PDT_MEDIAMODE_1440P_25:
                case HI_PDT_MEDIAMODE_PHOTO_1440P:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/1440P30.png");
                    break;
                case HI_PDT_MEDIAMODE_720P_30:
                case HI_PDT_MEDIAMODE_PHOTO_720P:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/720P30.png");
                    break;
                case HI_PDT_MEDIAMODE_1944P_30:
                case HI_PDT_MEDIAMODE_PHOTO_1944P:

                default :
                    MLOGE("Relolution error!");
                    snprintf(pDstStr, u32Length, "%s", "");
                break;
            }
        }
            break;

        case PDT_UI_SET_VENC_TYPE :
        {
            s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");

            if(HI_MAPI_PAYLOAD_TYPE_H264 == s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/h264.png");
            }
            else if(HI_MAPI_PAYLOAD_TYPE_H265 == s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/h265.png");
            }
        }
            break;

        case PDT_UI_SET_AUDIO_STATUS :
        {
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_AUDIO, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");

            if((HI_BOOL)s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/audio_on.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/audio_off.png");
            }
        }
            break;

        case PDT_UI_SET_FLIP_STATUS :
        {
            s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, HI_PDT_PARAM_TYPE_FLIP, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");

            if((HI_BOOL)s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/flip_on.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/flip_off.png");
            }
        }
            break;

        case PDT_UI_SET_MIRROR_STATUS :
        {
            s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, HI_PDT_PARAM_TYPE_MIRROR, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");
            if((HI_BOOL)s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/flip_on.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/flip_off.png");
            }
        }
            break;

        case PDT_UI_SET_LDC_STATUS :
        {
            s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, HI_PDT_PARAM_TYPE_LDC, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");

            if((HI_BOOL)s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/ldc_on.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/ldc_off.png");
            }
        }
            break;

        case PDT_UI_SET_TIME_OSD_STATUS :
        {

            s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, HI_PDT_PARAM_TYPE_OSD, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");

            if(HI_FALSE == s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/osd_off.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/osd_on.png");
            }
        }
            break;

        case PDT_UI_SET_VOLUME :
        {
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_VOLUME, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");
            switch(s32Value)
            {
                case 0:

                    snprintf(pDstStr, u32Length, "%s", "./res/pic/valume_mute.png");
                    break;

                case 33:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/valume_low.png");
                    break;

                case 66:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/valume_middle.png");
                    break;

                case 100:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/valume_high.png");
                    break;

                default :
                    snprintf(pDstStr, u32Length, "%s", "");
                    break;
            }
        }
            break;

        case PDT_UI_SET_USBMODE :
        {
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_USB_MODE, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");
            switch((HI_USB_MODE_E)s32Value)
            {
                case HI_USB_MODE_CHARGE:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/usb_charge.png");
                    break;

                case HI_USB_MODE_UVC:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/usb_uvc.png");
                    break;

                case HI_USB_MODE_STORAGE:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/usb_storage.png");
                    break;

                default :
                    snprintf(pDstStr, u32Length, "%s", "");
                    break;
            }
        }
            break;

        case PDT_UI_SET_BOOTSOUND :
        {
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_BOOTSOUND, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");

            if((HI_BOOL)s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/bootsound_on.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/bootsound_off.png");
            }

        }
            break;

#ifdef CONFIG_WIFI_ON
        case PDT_UI_SET_WIFI_STATUS:
        {
            HI_BOOL bCurValue = HI_FALSE;
            s32Ret = HI_HAL_WIFI_GetStartedStatus((HI_BOOL*)&bCurValue);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            if(bCurValue)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/wifi_on.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/wifi_off.png");
            }
        }
            break;
#endif


        case PDT_UI_SET_TIME_SETTING :
            snprintf(pDstStr, u32Length, "%s", "./res/pic/time_set.png");
            break;

        case  PDT_UI_SET_RESET_FACTORY :
            snprintf(pDstStr, u32Length, "%s", "./res/pic/reset.png");
            break;

        case  PDT_UI_SET_FORMAT_SD :
            snprintf(pDstStr, u32Length, "%s", "./res/pic/sd_formate.png");
            break;

        case  PDT_UI_SET_PARKING :
        {
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_GSENSOR_PARKING, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");
            switch(s32Value)
            {
                case 0:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/parking_off.png");
                    break;

                case 1:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/parking_low.png");
                    break;

                case 2:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/parking_middle.png");
                    break;

                case 3:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/parking_high.png");
                    break;

                default :
                    snprintf(pDstStr, u32Length, "%s", "");
                    break;

            }
        }
            break;

        case  PDT_UI_SET_DEBUG :
        {
            if(0 == s_s32DebugOn)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/debug_off.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/debug_on.png");
            }
        }
            break;


        case  PDT_UI_SET_SENSITIVITY :
        {
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetCommParam");
            switch(s32Value)
            {
                case 0:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/sensitivity_off.png");
                    break;

                case 1:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/sensitivity_low.png");
                    break;

                case 2:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/sensitivity_middle.png");
                    break;

                case 3:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/sensitivity_high.png");
                    break;

                default :

                    snprintf(pDstStr, u32Length, "%s", "");
                break;

            }
        }
            break;

        case  PDT_UI_SET_ABOUT :
            snprintf(pDstStr, u32Length, "%s", "./res/pic/info.png");
            break;

        case PDT_UI_SET_BACK :
            snprintf(pDstStr, u32Length, "%s", "./res/pic/back.png");
            break;

        case PDT_UI_SET_LANGUAGE :
        {
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_LANGUAGE, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetCommParam");
            if(0 == s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/language_chinese.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/language_english.png");
            }
        }
            break;

        case PDT_UI_SET_BACK_REC:
        {
            s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, BACK_CAM_ID, HI_PDT_PARAM_TYPE_REC, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetCamParam");

            if(0 == s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/back_rec_off.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/back_rec_on.png");
            }
        }
            break;

        case PDT_UI_SET_VOSWITCH:
            if (0 == s32CamID)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/voswitch_front.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/voswitch_back.png");
            }

            break;
        case PDT_UI_SET_WORK_MODE:
        {
            HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/workmode_photo.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/workmode_rec.png");
            }
        }
            break;


        case PDT_UI_SET_WDR:
            s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, s32CamID, HI_PDT_PARAM_TYPE_WDR, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "get CamParam");

            if (0 == s32Value)
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/wdr_off.png");
            }
            else
            {
                snprintf(pDstStr, u32Length, "%s", "./res/pic/wdr_on.png");
            }

            break;
        case PDT_UI_SET_SPLITTIME:
        {
            s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, s32CamID, HI_PDT_PARAM_TYPE_SPLITTIME, &s32Value);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get cam param");
            switch (s32Value)
            {
                case 60:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/loop_time_1min.png");
                    break;
                case 180:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/loop_time_3min.png");
                    break;
                case 300:
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/loop_time_5min.png");
                    break;
                default :
                    snprintf(pDstStr, u32Length, "%s", "");
                    break;
            }
        }
        break;

        case PDT_UI_SET_LAPSE_INTERVAL:
            {
                HI_REC_TYPE_E enRecType;
                s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, HI_PDT_PARAM_RECORD_TYPE, &enRecType);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCamParam");

                if(HI_REC_TYPE_LAPSE != enRecType)
                {
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/lapse_interval_off.png");
                    break;
                }
                else
                {
                    s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, s32CamID, HI_PDT_PARAM_TYPE_LAPSE_INTERVAL, &s32Value);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get cam param");
                    switch (s32Value)
                    {
                        case 500:
                            snprintf(pDstStr, u32Length, "%s", "./res/pic/lapse_interval_0.5.png");
                            break;
                        case 1000:
                            snprintf(pDstStr, u32Length, "%s", "./res/pic/lapse_interval_1.png");
                            break;
                        case 5000:
                            snprintf(pDstStr, u32Length, "%s", "./res/pic/lapse_interval_5.png");
                            break;
                        default :
                            snprintf(pDstStr, u32Length, "%s", "");
                            break;
                    }
                }

            }
            break;
        case PDT_UI_SET_BRECORD_TYPE:
            {
                s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, s32CamID, HI_PDT_PARAM_RECORD_TYPE, &s32Value);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get cam param");
                switch (s32Value)
                {
                    case HI_REC_TYPE_NORMAL:
                        snprintf(pDstStr, u32Length, "%s", "./res/pic/rec_type_norm.png");
                        break;
                    case HI_REC_TYPE_LAPSE:
                        snprintf(pDstStr, u32Length, "%s", "./res/pic/rec_type_lapse.png");
                        break;
                    default :
                        snprintf(pDstStr, u32Length, "%s", "");
                        break;
                }
            }
            break;
        case PDT_UI_SET_PREVIEWPIP:
            {
                HI_BOOL bPreviewP2P = HI_FALSE;
                s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PREVIEWPIP,&bPreviewP2P);
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
                if (HI_TRUE == bPreviewP2P)
                {
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/window_overlay_on.png");
                }
                else
                {
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/window_overlay_off.png");
                }
            }
            break;
        case PDT_UI_SET_VIDEOMODE:
            {
                s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, s32CamID, HI_PDT_PARAM_TYPE_VIDEOMODE, &s32Value);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                if (s32Value)
                {
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/videoformate_ntsc.png");
                }
                else
                {
                    snprintf(pDstStr, u32Length, "%s", "./res/pic/videoformate_pal.png");
                }
            }
            break;
        case PDT_UI_SET_SCREEN_DORMANT:
            {
                HI_TIMEDTASK_ATTR_S s_ScreenDormantAttr = {0};
                s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE,&s_ScreenDormantAttr);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get comm param");
                switch (s_ScreenDormantAttr.u32Time_sec)
                {
                    case 0:
                        snprintf(pDstStr, u32Length, "%s", "./res/pic/sleep_off.png");
                        break;
                    case 60:
                        snprintf(pDstStr, u32Length, "%s", "./res/pic/sleep_1.png");
                        break;
                    case 180:
                        snprintf(pDstStr, u32Length, "%s", "./res/pic/sleep_3min.png");
                        break;
                    default :
                        snprintf(pDstStr, u32Length, "%s", "");
                        break;
                }
            }
            break;
        default :
            MLOGE("u32GridIndex Error!\n");
            break;
    }

    return HI_SUCCESS;
}



HI_S32 PDT_UI_SET_GRID_Register(HI_U32 u32DBSource, HI_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

/** used by datamodel, unregisterdatachange */
HI_S32 PDT_UI_SET_GRID_Unregister(HI_U32 u32DBSource, HI_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

/**
 * @brief used  by datamodel, get filemgr's group count. A grid's data come from a row, which corresponds a group.
 */
HI_S32 PDT_UI_SET_GRID_GetRowCount(HI_U32 u32DBSource, HI_U32* pu32RowCnt)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pu32RowCnt, HI_FAILURE);

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
    {
        *pu32RowCnt = UI_ARRAY_SIZE(s_stPhotoSetItems);
    }
    else
    {
        *pu32RowCnt = UI_ARRAY_SIZE(s_stRecordSetItems);
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_SET_GRID_GetRowValue(HI_U32 u32DBSource, HI_U32 u32StartIndex, HI_U32 u32ReqNumber, HI_VOID* pData, HI_U32* pu32ResCnt)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    HI_S32 s32CamID = 0;
    PDT_UI_SET_ROW_S * pstRow = NULL;
    PDT_UI_SET_ROW_S stRowData = {};

    if(HI_NULL == pData || HI_NULL == pu32ResCnt)
    {
        MLOGE("Null Pointer\n");
        return HI_FAILURE;
    }
    MLOGD("u32StartIndex:%u, u32ReqNumber:%u\n", u32StartIndex, u32ReqNumber);

    s32CamID = HI_PDT_STATEMNG_GetPreViewCamID();

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_S32 s32Itemsize;
    PDT_UI_SET_DATA_S* pstSetItems = HI_NULL;
    if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
    {
        s32Itemsize = UI_ARRAY_SIZE(s_stPhotoSetItems);
        pstSetItems  = s_stPhotoSetItems;
    }
    else
    {
        s32Itemsize = UI_ARRAY_SIZE(s_stRecordSetItems);
        pstSetItems = s_stRecordSetItems;
    }

    *pu32ResCnt = 0;
    pstRow = (PDT_UI_SET_ROW_S *)pData;
    for(i = u32StartIndex; i < u32ReqNumber+u32StartIndex && i<s32Itemsize; i++)
    {
        memset(&stRowData, 0, sizeof(stRowData));

        stRowData.s32Item = pstSetItems[i].s32Item;
        stRowData.s32ParamType = pstSetItems[i].s32ParamType;
        s32Ret = PDT_UI_COMM_GetStringByID(pstSetItems[i].u32StringID, stRowData.szDesc, sizeof(stRowData.szDesc));
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        switch( stRowData.s32Item )
        {
            case PDT_UI_SET_MEDIA_MODE:
            case PDT_UI_SET_VENC_TYPE:
            case PDT_UI_SET_AUDIO_STATUS:
            case PDT_UI_SET_FLIP_STATUS:
            case PDT_UI_SET_MIRROR_STATUS:
            case PDT_UI_SET_LDC_STATUS:
            case PDT_UI_SET_TIME_OSD_STATUS:
            case PDT_UI_SET_SENSITIVITY:
            case PDT_UI_SET_PARKING:
            case PDT_UI_SET_VOLUME:
            case PDT_UI_SET_USBMODE:
            case PDT_UI_SET_BOOTSOUND:
            case PDT_UI_SET_LANGUAGE:
            case PDT_UI_SET_BACK_REC:
            case PDT_UI_SET_WDR:
            case PDT_UI_SET_SPLITTIME:
            case PDT_UI_SET_BRECORD_TYPE:
            case PDT_UI_SET_VIDEOMODE:
            case PDT_UI_SET_SCREEN_DORMANT:
            case PDT_UI_SET_LAPSE_INTERVAL:
                if(PDT_UI_SET_LAPSE_INTERVAL == stRowData.s32Item)
                {
                    HI_REC_TYPE_E enRecType;
                    s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, HI_PDT_PARAM_RECORD_TYPE, &enRecType);
                    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCamParam");

                    if(HI_REC_TYPE_LAPSE != enRecType)
                    {
                        snprintf(stRowData.szValue,sizeof(stRowData.szValue),"%s","--");
                        break;
                    }
                }

                s32Ret = PDT_UI_COMM_GetParamValueDesc(stWorkModeState.enWorkMode, s32CamID,
                    stRowData.s32ParamType, stRowData.szValue, sizeof(stRowData.szValue));
                if(s32Ret)
                {
                    MLOGE("GetParamValueDesc(Mode:%d,s32Item:%d, ParamType:%u) failed\n", stWorkModeState.enWorkMode, stRowData.s32Item,stRowData.s32ParamType);
                    continue;
                }
                MLOGD("GetParamValueDesc(Mode:%d, ParamType:%u, Desc:%s, Value:%s)\n",
                    stWorkModeState.enWorkMode, stRowData.s32ParamType,stRowData.szDesc, stRowData.szValue);
                break;

            case PDT_UI_SET_VOSWITCH:
                if (0 == s32CamID)
                {
                    PDT_UI_COMM_GetStringByID(STRING_FRONT_CAM, stRowData.szValue, sizeof(stRowData.szValue));
                }
                else
                {
                    PDT_UI_COMM_GetStringByID(STRING_BACK_CAM, stRowData.szValue, sizeof(stRowData.szValue));
                }
                break;
            case PDT_UI_SET_WORK_MODE:
            {
                if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
                {
                    PDT_UI_COMM_GetStringByID(STRING_TAKE_PHOTO, stRowData.szValue, sizeof(stRowData.szValue));
                }
                else if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode)
                {
                    PDT_UI_COMM_GetStringByID(STRING_RECORD, stRowData.szValue, sizeof(stRowData.szValue));
                }
            }
                break;

#ifdef CONFIG_WIFI_ON
            case PDT_UI_SET_WIFI_STATUS:
            {
                HI_BOOL bCurValue = HI_FALSE;
                s32Ret = HI_HAL_WIFI_GetStartedStatus((HI_BOOL*)&bCurValue);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                if(bCurValue)
                {
                    PDT_UI_COMM_GetStringByID(STRING_ON, stRowData.szValue, sizeof(stRowData.szValue));
                }
                else
                {
                    PDT_UI_COMM_GetStringByID(STRING_OFF, stRowData.szValue, sizeof(stRowData.szValue));
                }
            }
                break;
#endif


#ifdef CONFIG_RAWCAP_ON
            case PDT_UI_SET_DEBUG:
                s32Ret = PDT_UI_COMM_GetStringByID(0 == s_s32DebugOn ? STRING_OFF : STRING_ON,
                    stRowData.szValue, sizeof(stRowData.szValue));
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                break;
#endif
            case PDT_UI_SET_PREVIEWPIP:
                {
                    HI_BOOL bPreviewP2P = HI_FALSE;
                    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PREVIEWPIP,&bPreviewP2P);
                    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
                    s32Ret = PDT_UI_COMM_GetStringByID(HI_TRUE == bPreviewP2P ? STRING_ON : STRING_OFF,
                                                       stRowData.szValue , sizeof(stRowData.szValue));
                    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get string ID");
                }
                break;
        }
        memcpy(pstRow, &stRowData, sizeof(PDT_UI_SET_ROW_S));
        pstRow++;
        (*pu32ResCnt)++;

        if (s_s32CurGridIndex == i)
        {
            s32Ret = HI_GV_Widget_SetText(SET_LABEL_VALUE, stRowData.szValue);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_Widget_SetText");
            s32Ret = HI_GV_Widget_Paint(SET_LABEL_VALUE, NULL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_Widget_Paint");

            s32Ret = HI_GV_Widget_SetText(SET_LABEL_DESCRIPTION, stRowData.szDesc);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_Widget_SetText");
            s32Ret = HI_GV_Widget_Paint(SET_LABEL_DESCRIPTION, NULL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_Widget_Paint");
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_SET_VoSwitch(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {0};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if (stWorkModeState.bStateMngInProgress)
    {
        MLOGE("WorkMode: %u, StateMngInProgress!\n", stWorkModeState.enWorkMode);
        return HI_FAILURE;
    }

    s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "StartWindow");

    memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
    HI_S32 s32PreviewCamID = -1;

    s32PreviewCamID = HI_PDT_STATEMNG_GetPreViewCamID();
    s32PreviewCamID = (s32PreviewCamID + 1) % HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;

    stMessage.what = HI_EVENT_STATEMNG_VO_SWITCH;
    memcpy(stMessage.aszPayload,&s32PreviewCamID,sizeof(HI_S32));

    MLOGD("VoSwitch -----> [CamID:%d]\n", s32PreviewCamID);
    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage,UI_SET_OnReceiveMsgResult);
    if(s32Ret)
    {
        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "FinishWindow");
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

