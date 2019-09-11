/**
 * @file      ui_common.h
 * @brief     ui internal common interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
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
#include "hi_gv_scrollgrid.h"
#include "hi_gv_listbox.h"

#include "hi_appcomm.h"
#include "hi_upgrade.h"
#include "hi_product_ui.h"
#include "hi_eventhub.h"
#include "hi_message.h"
#include "hi_product_statemng.h"
#include "hi_product_param_define.h"
#include "hi_product_param.h"
#include "hi_liveserver.h"
#include "hi_playback.h"
#include "hi_liteplayer.h"
#include "hi_product_media.h"
#include "hi_timestamp.h"
#include "hi_system.h"
#include "hi_product_netctrl.h"

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

#define LCD_XML_LAYOUT_WIDTH    (320)
#define LCD_XML_LAYOUT_HEIGHT   (240)
#define HDMI_XML_LAYOUT_WIDTH   (1920)
#define HDMI_XML_LAYOUT_HEIGHT  (1080)

/*temp define HDMI*/
#define HI_EVENT_HDMI_OUT (0x31234000)
#define HI_EVENT_HDMI_IN  (0x31234001)

#define SQUARE(x)  ((x)*(x))
#define DISTANCE(x1,y1,x2,y2) ( (HI_U32)sqrt(SQUARE((x1)-(x2)) + SQUARE((y1)-(y2))) )
#define UI_SCREEN_WIDTH  (320)
#define UI_SCREEN_HEIGHT (240)
#define INVALID_HANDLE   (0x0)

#define BACK_GROUP_INDEX  (0xFFFFFFFC)
#define SELLECT_ALL_GROUP_INDEX  (0xFFFFFFFD)
#define DELETE_GROUP_INDEX  (0xFFFFFFFE)

#define INVALID_GROUP_INDEX  (0xFFFFFFFF)
#define ICON_HAVE_SUB_MENU_PATH "res/pic/set_have_sub_menu.png"
#define ICON_NO_SUB_MENU_PATH   "res/pic/set_no_sub_menu.png"

#define ICON_SPOT_METERING_WIDTH  (48)
#define ICON_SPOT_METERING_HEIGHT (48)

#define ICON_SPOT_METERING_WIDTH_PERCENT  (15)
#define ICON_SPOT_METERING_HEIGHT_PERCENT (20)

#define PDT_UI_KEY_BACK      HIGV_KEY_F3
#define PDT_UI_KEY_MENU      HIGV_KEY_F2
#define PDT_UI_KEY_ENTER     HIGV_KEY_F1
#define PDT_UI_HOME_KEY_WIFI HIGV_KEY_P1

#define PDT_UI_SET_AND_SHOW_OFFSET (100)

#define TIMER_REFRESH_HOME (0x20180122)
#define TIMER_PLAYBACK_HIDE_UI (0x20180123)
#define TIMER_HDMI_PLAYBACK_HIDE_UI (0x20180124)

/** TODO:define in param */
#define PDT_UI_MAIN_STREAM_VENC_HDL (0)
#define PDT_UI_SUB_STREAM_VENC_HDL  (1)
#define PDT_UI_PHOTO_VENC_HDL       (2)
#define PDT_UI_THM_VENC_HANDLE      (3)

/** function ret value check without return */
#define PDT_UI_CHECK_RET_WITHOUT_RETURN(RetValue,ErrString)\
do{\
    if(HI_SUCCESS != RetValue)\
    {\
        MLOGE(RED"%s failed, Ret[%x]\n"NONE, ErrString, RetValue);\
    }\
  }while(0)

/** function ret value check without return value */
#define PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(RetValue,ErrString)\
do{\
    if(HI_SUCCESS != RetValue)\
    {\
        MLOGE(RED"%s failed, Ret[%x]\n"NONE, ErrString, RetValue);\
        return;\
    }\
  }while(0)

#define FILE_GROUP_TYPE_NORMMP4  "normmp4"

#define FILE_GROUP_TYPE_SLOWMP4  "slowmp4"

#define FILE_GROUP_TYPE_LPSEMP4  "lpsemp4"

#define FILE_GROUP_TYPE_LOOPMP4  "lmp4"

#define FILE_GROUP_TYPE_SINGJPG  "singjpg"
#define FILE_GROUP_TYPE_SINRJPG  "sinrjpg"

#define FILE_GROUP_TYPE_DLAYJPG  "dlayjpg"
#define FILE_GROUP_TYPE_DLYRJPG  "dlyrjpg"

#define FILE_GROUP_TYPE_LPSEJPG  "ljpg"

#define FILE_GROUP_TYPE_BURSTJPG "bjpg"



/**
 * used in set page, every row must bind an action which indicates the next page or setting operation!
 * PDT_UI_ACTION_SHOW_XX means jump to a new page,
 * PDT_UI_ACTION_SET_XX     means set something.
 */
typedef enum tagPDT_UI_ACTION_E
{
    /** show action */
    PDT_UI_ACTION_SHOW_FIRST_VIEW = 0,
    PDT_UI_ACTION_SHOW_MODE_SETTINGS,
    PDT_UI_ACTION_SHOW_MEDIA_SETTINGS,
    PDT_UI_ACTION_SHOW_DATE_PAGE,
    PDT_UI_ACTION_SHOW_TIME_PAGE,
    PDT_UI_ACTION_SHOW_FORMAT,
    PDT_UI_ACTION_SHOW_RESET,
    PDT_UI_ACTION_SET_DEBUG,
    PDT_UI_ACTION_SHOW_ABOUT,

    PDT_UI_ACTION_SHOW_MEDIAMODE = PDT_UI_SET_AND_SHOW_OFFSET, /**<All Rec/All Photo/RecSnap, HI_PDT_MEDIAMODE_E */
    PDT_UI_ACTION_SHOW_PHOTO_SCENE,       /**<Single/Delay */
    PDT_UI_ACTION_SHOW_PHOTO_OUTPUT_FMT,  /**<Single/Delay */
    PDT_UI_ACTION_SHOW_DELAY_TIME,        /**<Delay */
    PDT_UI_ACTION_SHOW_LAPSE_INTERVAL,    /**<LapseRec/LapsePhoto/RecSnap */
    PDT_UI_ACTION_SHOW_BURST_TYPE,
    PDT_UI_ACTION_SHOW_LOOP_TIME,         /**<LoopRec */
    PDT_UI_ACTION_SHOW_ENC_PAYLOAD_TYPE,
    PDT_UI_ACTION_SHOW_PROTUNE_EXP_EV,
    PDT_UI_ACTION_SHOW_PROTUNE_EXP_TIME,
    PDT_UI_ACTION_SHOW_PROTUNE_ISO,
    PDT_UI_ACTION_SHOW_PROTUNE_WB,
    PDT_UI_ACTION_SHOW_PROTUNE_METRY,

    PDT_UI_ACTION_SHOW_OSD,
    PDT_UI_ACTION_SHOW_AUDIO,
    PDT_UI_ACTION_SHOW_DIS,
    PDT_UI_ACTION_SHOW_LDC,
    PDT_UI_ACTION_SHOW_KEYTONE,

    /** WorkMode Independent */
    PDT_UI_ACTION_SHOW_POWERON_ACTION,    /**<HI_PDT_POWERON_ACTION_E */
    PDT_UI_ACTION_SHOW_VOLUME,            /**<HI_S32 */
    PDT_UI_ACTION_SHOW_SCREEN_BRIGHTNESS, /**<HI_S32 */
    PDT_UI_ACTION_SHOW_SCREEN_DORMANT,    /**<HI_TIMEDTASK_ATTR_S */
    PDT_UI_ACTION_SHOW_SYS_DORMANT,       /**<HI_TIMEDTASK_ATTR_S */
    PDT_UI_ACTION_SHOW_USB_MODE,
    PDT_UI_ACTION_SHOW_LANGUAGE,

    /** set action */
    PDT_UI_ACTION_SET_MEDIAMODE         = (PDT_UI_ACTION_SHOW_MEDIAMODE + PDT_UI_SET_AND_SHOW_OFFSET),         /**<All Rec/All Photo/RecSnap, HI_PDT_MEDIAMODE_E */
    PDT_UI_ACTION_SET_PHOTO_SCENE       = (PDT_UI_ACTION_SHOW_PHOTO_SCENE + PDT_UI_SET_AND_SHOW_OFFSET),       /**<Single/Delay */
    PDT_UI_ACTION_SET_PHOTO_OUTPUT_FMT  = (PDT_UI_ACTION_SHOW_PHOTO_OUTPUT_FMT + PDT_UI_SET_AND_SHOW_OFFSET),  /**<Single/Delay */
    PDT_UI_ACTION_SET_DELAY_TIME        = (PDT_UI_ACTION_SHOW_DELAY_TIME + PDT_UI_SET_AND_SHOW_OFFSET),        /**<Delay */
    PDT_UI_ACTION_SET_LAPSE_INTERVAL    = (PDT_UI_ACTION_SHOW_LAPSE_INTERVAL + PDT_UI_SET_AND_SHOW_OFFSET),    /**<LapseRec/LapsePhoto/RecSnap */
    PDT_UI_ACTION_SET_BURST_TYPE        = (PDT_UI_ACTION_SHOW_BURST_TYPE + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_LOOP_TIME         = (PDT_UI_ACTION_SHOW_LOOP_TIME + PDT_UI_SET_AND_SHOW_OFFSET),         /**<LoopRec */
    PDT_UI_ACTION_SET_ENC_PAYLOAD_TYPE  = (PDT_UI_ACTION_SHOW_ENC_PAYLOAD_TYPE + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_PROTUNE_EXP_EV    = (PDT_UI_ACTION_SHOW_PROTUNE_EXP_EV + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_PROTUNE_EXP_TIME  = (PDT_UI_ACTION_SHOW_PROTUNE_EXP_TIME + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_PROTUNE_ISO       = (PDT_UI_ACTION_SHOW_PROTUNE_ISO + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_PROTUNE_WB        = (PDT_UI_ACTION_SHOW_PROTUNE_WB + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_PROTUNE_METRY     = (PDT_UI_ACTION_SHOW_PROTUNE_METRY + PDT_UI_SET_AND_SHOW_OFFSET),

    PDT_UI_ACTION_SET_OSD               = (PDT_UI_ACTION_SHOW_OSD + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_AUDIO             = (PDT_UI_ACTION_SHOW_AUDIO + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_DIS               = (PDT_UI_ACTION_SHOW_DIS + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_LDC               = (PDT_UI_ACTION_SHOW_LDC + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_KEYTONE           = (PDT_UI_ACTION_SHOW_KEYTONE + PDT_UI_SET_AND_SHOW_OFFSET),

    /** WorkMode Independent */
    PDT_UI_ACTION_SET_POWERON_ACTION    = (PDT_UI_ACTION_SHOW_POWERON_ACTION + PDT_UI_SET_AND_SHOW_OFFSET),    /**<HI_PDT_POWERON_ACTION_E */
    PDT_UI_ACTION_SET_SCREEN_BRIGHTNESS = (PDT_UI_ACTION_SHOW_SCREEN_BRIGHTNESS + PDT_UI_SET_AND_SHOW_OFFSET), /**<HI_S32 */
    PDT_UI_ACTION_SET_SCREEN_DORMANT    = (PDT_UI_ACTION_SHOW_SCREEN_DORMANT + PDT_UI_SET_AND_SHOW_OFFSET),    /**<HI_TIMEDTASK_ATTR_S */
    PDT_UI_ACTION_SET_SYS_DORMANT       = (PDT_UI_ACTION_SHOW_SYS_DORMANT + PDT_UI_SET_AND_SHOW_OFFSET),       /**<HI_TIMEDTASK_ATTR_S */
    PDT_UI_ACTION_SET_USB_MODE          = (PDT_UI_ACTION_SHOW_USB_MODE + PDT_UI_SET_AND_SHOW_OFFSET),
    PDT_UI_ACTION_SET_LANGUAGE          = (PDT_UI_ACTION_SHOW_LANGUAGE + PDT_UI_SET_AND_SHOW_OFFSET),

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
    HIGV_HANDLE hSkin;
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
    HI_U32 u32WorkMode;     /* work mode(message arg2),  */
    HI_U32 s32Value;        /* set value  (message payload), enum value or others */
} PDT_UI_SET_ROW_S;


typedef enum tagPDT_UI_OUTPUT_STATE_E
{
    UI_OUTPUT_PREPARE_SWITCH_TO_LCD,
    UI_OUTPUT_SWITCHING_TO_LCD,
    UI_OUTPUT_LCD,
    UI_OUTPUT_PREPARE_SWITCH_TO_HDMI,
    UI_OUTPUT_SWITCHING_TO_HDMI,
    UI_OUTPUT_HDMI,
    UI_OUTPUT_BUTT
}PDT_UI_OUTPUT_STATE_E;

typedef struct tagPDT_UI_DATETIME_S
{
    HI_U32 year;   /*2018-2037*/
    HI_U32 month;  /*1-12*/
    HI_U32 day;    /*1-31*/
    HI_U32 hour;   /*0-23*/
    HI_U32 minute; /*0-59*/
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

#if defined(CONFIG_WIFI_ON)
typedef struct tagPDT_UI_WIFI_STATUS_S
{
    HI_BOOL bInit;  /** WiFi init */
    HI_BOOL bStart; /** WiFi start */
    HI_BOOL bResume; /** stop wifi when system dormant, resume wifi state when wakeup system dormant */
} PDT_UI_WIFI_STATUS_S;

HI_S32 PDT_UI_COMM_GetWiFiStatus(PDT_UI_WIFI_STATUS_S *pstWiFiStatus);
HI_S32 PDT_UI_COMM_SetWiFiStatus(PDT_UI_WIFI_STATUS_S *pstWiFiStatus);
HI_S32 PDT_UI_COMM_StartWiFi(HI_VOID);
HI_S32 PDT_UI_COMM_StopWiFi(HI_VOID);
#endif

HI_S32 PDT_UI_COMM_InitKeyTone(HI_VOID);
HI_S32 PDT_UI_COMM_DeinitKeyTone(HI_VOID);

HI_VOID PDT_UI_COMM_EnableUSB(HI_VOID);
HI_VOID PDT_UI_COMM_DisableUSB(HI_VOID);

HI_BOOL PDT_UI_COMM_IsUSBIn(HI_VOID);

HI_S32 PDT_UI_StartLCD(HI_VOID);
HI_S32 PDT_UI_StartHDMI(HI_VOID);
HI_VOID PDT_UI_DeinitHigv(HI_VOID);

HI_VOID PDT_UI_COMM_StartWindow(HIGV_HANDLE WinHdl);
HI_VOID PDT_UI_COMM_FinishWindow(HIGV_HANDLE WinHdl);

HI_S32 PDT_UI_COMM_GetAVHdl(HI_PDT_WORKMODE_E enWorkMode, HI_HANDLE* pAoHdl, HI_HANDLE* pAoChnHdl, HI_HANDLE* pVdispHdl, HI_HANDLE* pWndHdl);
HI_U64 PDT_UI_COMM_GetExecTime(HI_VOID);
HI_U64 PDT_UI_COMM_GetRemainDelayTime(HI_VOID);
HI_S32 PDT_UI_COMM_GetRemainPhotoCount(HI_U32 *pu32RemainCount);
HI_S32 PDT_UI_COMM_UpdateTotalBitRate(HI_VOID);
HI_S32 PDT_UI_COMM_GetRemainRecordTime(HI_U32 *pu32RemainTime);


HI_VOID PDT_UI_COMM_Second2String(HI_U64 second, HI_CHAR * buffer, HI_U32 size);


HI_S32 PDT_UI_COMM_GetDateTime(PDT_UI_DATETIME_S * pstDateTime);
HI_S32 PDT_UI_COMM_SetDateTime(PDT_UI_DATETIME_S * pstDateTime);
HI_S32 PDT_UI_COMM_GetDateString(PDT_UI_DATETIME_S * pstDateTime, HI_CHAR * pszBuffer, HI_U32 size);
HI_S32 PDT_UI_COMM_GetTimeString(PDT_UI_DATETIME_S * pstDateTime, HI_CHAR * pszBuffer, HI_U32 size);
HI_VOID PDT_UI_COMM_ShowAlarm(HIGV_HANDLE WinHdl, HIGV_HANDLE TitleHdl, HI_U32 u32StringID);

HI_S32 PDT_UI_WINMNG_Init(HI_VOID);
HI_S32 PDT_UI_WINMNG_Deinit(HI_VOID);
HI_S32 PDT_UI_WINMNG_StartWindow(HIGV_HANDLE WinHdl,HI_BOOL bHideCurWin);
HI_S32 PDT_UI_WINMNG_FinishWindow(HIGV_HANDLE WinHdl);
HI_BOOL PDT_UI_WINMNG_WindowIsShow(HIGV_HANDLE WinHdl);
HI_S32 PDT_UI_WINMNG_HideAllWindow(HI_VOID);


HI_S32 PDT_UI_RegisterTouchDevice(HI_VOID);


HI_S32 PDT_UI_COMM_GetStringOfMode(const HI_PDT_WORKMODE_E enWorkMode, HI_CHAR* pszBuffer, HI_U32 u32Size);
HI_S32 PDT_UI_COMM_GetStringByID(const HI_U32 u32StrID, HI_CHAR* pszBuffer, HI_U32 u32Size);
HI_S32 PDT_UI_COMM_GetStringByString(const HI_CHAR* pszSrcBuffer, HI_CHAR* pszDestBuffer, HI_U32 u32Size);
HI_S32 PDT_UI_COMM_Translate(const HI_CHAR* pszSrcBuffer, HI_CHAR* pszDestBuffer, HI_U32 u32Size);
HI_S32 PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_TYPE_E enType, HI_CHAR* pszBuffer, HI_U32 u32Size);
HI_S32 PDT_UI_COMM_ActiveNextWidget(HIGV_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HIGV_HANDLE CurWidgetHdl);
HI_S32 PDT_UI_COMM_GetFSInfo(HI_STORAGE_FS_INFO_S *pstFSInfo);
HI_S32 PDT_UI_COMM_GetStorageState(HI_STORAGE_STATE_E* penStorageState);
HI_BOOL PDT_UI_COMM_SdIsReady(HI_VOID);
HI_BOOL PDT_UI_COMM_SdIsFull(HI_VOID);

HI_S32 PDT_UI_COMM_LCDEventProc(HI_EVENT_S* pEvent);
HI_S32 PDT_UI_COMM_HDMIEventProc(HI_EVENT_S* pstEvent);

HI_S32 PDT_UI_COMM_SendAsyncMsg(HI_MESSAGE_S* pstMsg, PDT_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc);
HI_S32 PDT_UI_COMM_SubscribeEvents(HI_VOID);

HI_S32 PDT_UI_HOME_EventProc(HI_EVENT_S * pstEvent);
HI_S32 PDT_HDMI_HOME_EventProc(HI_EVENT_S * pstEvent);


HI_VOID PDT_UI_FILELIST_DIALOG_Delete(HI_VOID);
HI_VOID PDT_HDMI_FILELIST_DIALOG_Delete(HI_VOID);
HI_S32 PDT_UI_FILELIST_OnEventProc(HI_EVENT_S * pstEvent, HIGV_HANDLE ADMHdl, HIGV_HANDLE BackBtnHdl);
HI_S32 PDT_UI_HDMI_FILELIST_OnEventProc(HI_EVENT_S * pstEvent, HIGV_HANDLE ADMHdl, HIGV_HANDLE BackBtnHdl);
HI_S32 PDT_UI_COMM_FILELIST_OnEventProc(HI_EVENT_S * pstEvent, HIGV_HANDLE ADMHdl, HIGV_HANDLE BackBtnHdl);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
