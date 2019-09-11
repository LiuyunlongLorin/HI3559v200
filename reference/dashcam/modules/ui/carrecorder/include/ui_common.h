/**
* @file    ui_common.h
* @brief   ui internal common interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/7
* @version   1.0

*/


#ifndef __UI_COMMON_H__
#define __UI_COMMON_H__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>


#include "higv_language.h"
#include "higv_cextfile.h"
#include "hi_gv_widget.h"
#include "hi_gv_parser.h"
#include "hi_gv_ddb.h"

#include "hi_appcomm.h"
#include "hi_product_ui.h"
#include "hi_eventhub.h"
#include "hi_message.h"
#include "hi_product_statemng.h"
#include "hi_product_param_define.h"
#include "hi_product_param.h"
#include "hi_liveserver.h"
#include "hi_timedtask.h"
#include "hi_system.h"
#include "hi_gsensormng.h"
#include "hi_upgrade.h"
#include "hi_playback.h"
#include "hi_product_usbctrl.h"
#include "hi_product_netctrl.h"
#include "hi_timestamp.h"

#ifdef CONFIG_MOTIONDETECT_ON
#include "hi_videodetect.h"
#endif

#if defined(CONFIG_WIFI_ON)
#include "hi_hal_wifi.h"
#endif


#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "UI"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/*temp define HDMI*/
#define HI_EVENT_HDMI_OUT (0x31234000)
#define HI_EVENT_HDMI_IN  (0x31234001)

#define UI_ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))
#define UI_SCREEN_WIDTH  (320)
#define UI_SCREEN_HEIGHT (240)
#define INVALID_HANDLE   (0x0)
#define ICON_HAVE_SUB_MENU_PATH "/app/bin/res/pic/set_have_sub_menu.png"

#define TIMER_REFRESH_HOME (0x20180122)
#define TIMER_REFRESH_HOME_EMR (0x20180813)
#define TIMER_REFRESH_ALARM_POWEROFF (0x20180814)

#define UI_HOME_EMR_TIME_INTERVAL (50)

#define PDT_UI_KEY_EXPAND  HIGV_KEY_F4
#define PDT_UI_KEY_BACK      HIGV_KEY_F3
#define PDT_UI_KEY_MENU      HIGV_KEY_F2
#define PDT_UI_KEY_OK        HIGV_KEY_F1
#define PDT_UI_HOME_KEY_WIFI HIGV_KEY_P1
#define PDT_UI_SET_AND_SHOW_OFFSET (100)
#define CONFIG_SYS_SOUND_ON
#define UI_FILELIST_INVALID_FILE_IDX  (0xFFFFFFFF)
#define Front_CAM_ID (0)
#define BACK_CAM_ID  (1)

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

/** function ret value check without return */
#define PDT_UI_CHECK_RET_WITHOUT_RETURN(RetValue,ErrString)\
do{\
    if(HI_SUCCESS != RetValue)\
    {\
        MLOGE(RED"%s failed, s32Ret(%x)\n\n"NONE, ErrString, RetValue);\
    }\
  }while(0)

/** function ret value check without return value */
#define PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(RetValue,ErrString)\
do{\
    if(HI_SUCCESS != RetValue)\
    {\
        MLOGE(RED"%s failed, s32Ret(%x)\n\n"NONE, ErrString, RetValue);\
        return;\
    }\
  }while(0)

/**
 * used in set page, every row must bind an action which indicates the next page or setting operation!
 * PDT_UI_ACTION_SHOW_XX means jump to a new page,
 * PDT_UI_ACTION_SET_XX     means set something.
 */
typedef enum tagPDT_UI_ACTION_E
{
    PDT_UI_ACTION_SHOW_FIRST_VIEW = 0,
    PDT_UI_ACTION_SHOW_WORKMODE,
    PDT_UI_ACTION_SHOW_VO_SWITCH,
    PDT_UI_ACTION_SHOW_CAM_ATTR,
    PDT_UI_ACTION_SHOW_MEDIA_SETTINGS,
    PDT_UI_ACTION_SHOW_TIME_PAGE,
    PDT_UI_ACTION_SHOW_FORMAT,
    PDT_UI_ACTION_SHOW_RESET,
    PDT_UI_ACTION_SHOW_DEBUG,
    PDT_UI_ACTION_SHOW_ABOUT,

    PDT_UI_ACTION_SHOW_MEDIAMODE = PDT_UI_SET_AND_SHOW_OFFSET,
    PDT_UI_ACTION_SHOW_ENC_PAYLOAD_TYPE,

    PDT_UI_ACTION_SHOW_OSD,
    PDT_UI_ACTION_SHOW_FLIP,
    PDT_UI_ACTION_SHOW_LDC,
    PDT_UI_ACTION_SHOW_WDR,
    PDT_UI_ACTION_SHOW_VIDEOMODE,
    PDT_UI_ACTION_SHOW_MIRROR,

    /** WorkMode Independent */
    PDT_UI_ACTION_SHOW_WIFI_STATUS,
    PDT_UI_ACTION_SHOW_DEV_INFO,
    PDT_UI_ACTION_SHOW_AUDIO,
    PDT_UI_ACTION_SHOW_VOLUME,
    PDT_UI_ACTION_SHOW_KEYTONE,
    PDT_UI_ACTION_SHOW_SCREEN_DORMANT,
    PDT_UI_ACTION_SHOW_SCREEN_BRIGHTNESS,
    PDT_UI_ACTION_SHOW_BOOTSOUND,
    PDT_UI_ACTION_SHOW_USB_MODE,
    PDT_UI_ACTION_SHOW_LANGUAGE,
    PDT_UI_ACTION_SHOW_SENSITIVITY,
    PDT_UI_ACTION_SHOW_PARKING,
    PDT_UI_ACTION_SHOW_BACK_REC,
    PDT_UI_ACTION_SHOW_RECSPLITTIME,
    PDT_UI_ACTION_SHOW_BRECORD_TYPE,
    PDT_UI_ACTION_SHOW_LAPSE_INTERVAL,
    PDT_UI_ACTION_SHOW_PREVIEWPIP,
#ifdef CONFIG_MOTIONDETECT_ON
    PDT_UI_ACTION_SHOW_MD_SENSITIVITY,
#endif
    /** set action */
    PDT_UI_ACTION_SET_WORKMODE = (PDT_UI_ACTION_SHOW_WORKMODE + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_MEDIAMODE = (PDT_UI_ACTION_SHOW_MEDIAMODE + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_ENC_PAYLOAD_TYPE = (PDT_UI_ACTION_SHOW_ENC_PAYLOAD_TYPE + PDT_UI_SET_AND_SHOW_OFFSET),

    PDT_UI_ACTION_SET_OSD = (PDT_UI_ACTION_SHOW_OSD + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_FLIP = (PDT_UI_ACTION_SHOW_FLIP + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_LDC = (PDT_UI_ACTION_SHOW_LDC + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_WDR = (PDT_UI_ACTION_SHOW_WDR + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_VIDEOMODE = (PDT_UI_ACTION_SHOW_VIDEOMODE + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_MIRROR = (PDT_UI_ACTION_SHOW_MIRROR + PDT_UI_SET_AND_SHOW_OFFSET),

    /** WorkMode Independent */
    PDT_UI_ACTION_SET_WIFI_STATUS = (PDT_UI_ACTION_SHOW_WIFI_STATUS + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_DEV_INFO = (PDT_UI_ACTION_SHOW_DEV_INFO + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_AUDIO = (PDT_UI_ACTION_SHOW_AUDIO + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_VOLUME = (PDT_UI_ACTION_SHOW_VOLUME + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_KEYTONE = (PDT_UI_ACTION_SHOW_KEYTONE + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_SCREEN_DORMANT = (PDT_UI_ACTION_SHOW_SCREEN_DORMANT + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_SCREEN_BRIGHTNESS = (PDT_UI_ACTION_SHOW_SCREEN_BRIGHTNESS + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_BOOTSOUND = (PDT_UI_ACTION_SHOW_BOOTSOUND + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_USB_MODE = (PDT_UI_ACTION_SHOW_USB_MODE + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_LANGUAGE  = (PDT_UI_ACTION_SHOW_LANGUAGE + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_SENSITIVITY = (PDT_UI_ACTION_SHOW_SENSITIVITY + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_PARKING = (PDT_UI_ACTION_SHOW_PARKING + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_BACK_REC = (PDT_UI_ACTION_SHOW_BACK_REC + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_RECSPLITTIME = (PDT_UI_ACTION_SHOW_RECSPLITTIME + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_BRECORD_TYPE = (PDT_UI_ACTION_SHOW_BRECORD_TYPE + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_LAPSE_INTERVAL = (PDT_UI_ACTION_SHOW_LAPSE_INTERVAL + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_PREVIEWPIP = (PDT_UI_ACTION_SHOW_PREVIEWPIP + PDT_UI_SET_AND_SHOW_OFFSET),

#ifdef CONFIG_MOTIONDETECT_ON
    PDT_UI_ACTION_SET_MD_SENSITIVITY = (PDT_UI_ACTION_SHOW_MD_SENSITIVITY + PDT_UI_SET_AND_SHOW_OFFSET),
#endif
    PDT_UI_ACTION_BUTT,
}PDT_UI_ACTION_E;


#define MAX_SET_LEVELS (3)

typedef struct tagPDT_UI_SET_LEVEL_S
{
    PDT_UI_ACTION_E enAction;
    HI_U32 u32StartRowIndex;
    HI_U32 u32ActiveRowIndex;
} PDT_UI_SET_LEVEL_S;

typedef struct tagSET_ACTION_AND_PARAM_TYPE_S
{
    PDT_UI_ACTION_E enAction;
    HI_PDT_PARAM_TYPE_E enParamType;
}PDT_UI_SET_ACTION_AND_PARAM_TYPE_S;

typedef struct tagPDT_UI_WORKMODE_AND_SKIN_S
{
    HI_PDT_WORKMODE_E enWorkMode;
    HI_HANDLE hSkin;
}PDT_UI_WORKMODE_AND_SKIN_S;

typedef struct tagPDT_UI_SET_DATA_S
{
    HI_U32 u32Action;       /* set or show something */
    HI_U32 u32ParamType;    /* param type,  HI_PDT_PARAM_TYPE_XXX */
    HI_U32 u32Title;        /* title string id */
    HI_BOOL bHaveSubMenu;   /* has sub menu icon ">"  */
} PDT_UI_SET_DATA_S;


#define SETTING_CONTENT_SIZE  (32)

/* used by DDB, for show setting rows*/
typedef struct tagPDT_UI_SET_ROW_S
{
    HI_CHAR szTitle[SETTING_CONTENT_SIZE];   /* show text, left */
    HI_CHAR szContent[SETTING_CONTENT_SIZE]; /* show text, right */
    HI_RESID resIcon;         /* has sub menu or others */
    HI_U32 u32Action;       /* set or show what */
    HI_U32 u32ParamType;    /* param type    (message arg1) ,    HI_PDT_PARAM_TYPE_XXX */
    HI_U32 s32CamID;     /* s32CamID  */
    HI_U32 s32Value;        /* set value  (message payload), enum value or others */
} PDT_UI_SET_ROW_S;

typedef struct tagPDT_UI_DATETIME_S
{
    HI_U32 year;   /**< 2018-2037 */
    HI_U32 month;  /**< 1-12 */
    HI_U32 day;    /**< 1-31 */
    HI_U32 hour;   /**< 0-23 */
    HI_U32 minute; /**< 0-59 */
    HI_U32 second; /**< 0-59 */
}PDT_UI_DATETIME_S;

typedef HI_S32 (*PDT_UI_MSGRESULTPROC_FN_PTR)(HI_EVENT_S* pstEvent);

/**State machine message context, can not send a msg successfully until last msg is finished*/
typedef struct tagPDT_UI_MESSAGE_CONTEXT
{
    HI_MESSAGE_S stMsg;     /**< the message that has been sent*/
    HI_BOOL bMsgProcessed;  /**< the message sent has been processed or not*/
    PDT_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc;   /**< used to process the response*/
    pthread_mutex_t MsgMutex;  /**< the mutex protect sent msg and bool flag*/
}PDT_UI_MESSAGE_CONTEXT;

HI_S32 PDT_UI_COMM_isMdReady(HI_VOID);

HI_S32 PDT_UI_COMM_InitKeyTone(HI_VOID);
HI_S32 PDT_UI_COMM_DeinitKeyTone(HI_VOID);

HI_S32 PDT_UI_COMM_isBackCamConnecting(HI_VOID);

HI_S32 PDT_UI_InitLCD(HI_VOID);
HI_S32 PDT_UI_InitHDMI(HI_VOID);

HI_S32 PDT_UI_COMM_SendStopMsg(PDT_UI_MSGRESULTPROC_FN_PTR pfnOnReceiveMsgResult);

HI_U64 PDT_UI_COMM_GetExecTime(HI_VOID);
HI_VOID PDT_UI_COMM_Second2String(HI_U64 second, HI_CHAR * buffer, HI_U32 size);
HI_S32 PDT_UI_COMM_GetDateTime(PDT_UI_DATETIME_S * pstDateTime);
HI_S32 PDT_UI_COMM_SetDateTime(PDT_UI_DATETIME_S * pstDateTime);
HI_S32 PDT_UI_COMM_GetDateTimeString(PDT_UI_DATETIME_S * pstDateTime, HI_CHAR * pszBuffer, HI_U32 size);

HI_S32 PDT_UI_WINMNG_Init(HI_VOID);
HI_S32 PDT_UI_WINMNG_Deinit(HI_VOID);
HI_S32 PDT_UI_WINMNG_StartWindow(HI_HANDLE WinHdl,HI_BOOL bHideCurWin);
HI_S32 PDT_UI_WINMNG_FinishWindow(HI_HANDLE WinHdl);
HI_BOOL PDT_UI_WINMNG_WindowIsShow(HI_HANDLE WinHdl);

HI_S32 PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_E enWorkMode, HI_S32 s32CamID, HI_PDT_PARAM_TYPE_E enType,
    HI_CHAR* pszBuffer, HI_U32 u32Size);


HI_S32 PDT_UI_COMM_GetStringByID(const HI_U32 u32StrID, HI_CHAR* pszBuffer, HI_U32 u32Size);
HI_VOID PDT_UI_COMM_ShowAlarm(HI_U32 u32StringID);
HI_S32 PDT_UI_COMM_ActiveNextWidget(HI_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HI_HANDLE CurWidgetHdl);
HI_S32 PDT_UI_COMM_ActivePreviousWidget(HI_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HI_HANDLE CurWidgetHdl);
HI_S32 PDT_UI_COMM_EventProc(HI_EVENT_S* pEvent);
HI_S32 PDT_UI_COMM_SendAsyncMsg(HI_MESSAGE_S* pstMsg, PDT_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc);
HI_S32 PDT_UI_COMM_SubscribeEvents(HI_VOID);
HI_S32 PDT_UI_COMM_POWEROFF(HI_VOID);
HI_S32 PDT_UI_COMM_CheckSd(HI_VOID);

#ifdef  CFG_TIME_STAMP_ON
HI_VOID PDT_UI_COMM_SwitchPageTime_begin(HI_VOID);
HI_VOID PDT_UI_COMM_SwitchPageTime_end(HI_VOID);
#endif

#ifndef ONE_SENSOR_CONNECT
HI_S32 PDT_UI_SET_VoSwitch(HI_VOID);
#endif


HI_S32 PDT_UI_HOME_Refresh(HI_VOID);
HI_S32 PDT_UI_HOME_EventProc(HI_EVENT_S * pstEvent);
HI_VOID PDT_UI_PLAYBACK_VideoPlayerInit(HI_VOID);

HI_S32 PDT_UI_ALBUM_OnEventProc(HI_EVENT_S * pstEvent);
HI_VOID PDT_UI_FILELIST_GetCurFileIdx(HI_U32* pu32CurFileIdx);
HI_VOID PDT_UI_FILELIST_SetCurFileIdx(HI_U32 u32CurFileIdx);
HI_VOID PDT_UI_FILELIST_SetSearchScope(HI_DTCF_DIR_E enDir);
HI_VOID PDT_UI_FILELIST_GetSearchScope(HI_DTCF_DIR_E* penDir);

HI_S32 PDT_UI_SD_UPGRADE_EventProc(HI_EVENT_S * pstEvent);

HI_S32 PDT_UI_COMM_Translate(const HI_CHAR* pszSrcBuffer, HI_CHAR* pszDestBuffer, HI_U32 u32Size);
HI_S32 PDT_UI_COMM_GetStringByString(const HI_CHAR* pszSrcBuffer, HI_CHAR* pszDestBuffer, HI_U32 u32Size);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
