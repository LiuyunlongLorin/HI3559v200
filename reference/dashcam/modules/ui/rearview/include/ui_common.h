/**
 * @file      ui_common.h
 * @brief     ui internal common interface
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
#include "hi_hal_screen.h"
#include "hi_gsensormng.h"
#include "hi_upgrade.h"
#include "hi_playback.h"
#include "hi_timestamp.h"
#include "hi_filemng_dtcf.h"


#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "UI"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#ifdef CFG_LCD_UISRC_DEST_WIDTH
#define XML_LAYOUT_WIDTH    CFG_LCD_UISRC_DEST_WIDTH
#else
#define XML_LAYOUT_WIDTH    (1600)
#endif

#ifdef CFG_LCD_UISRC_DEST_HEIGHT
#define XML_LAYOUT_HEIGHT   CFG_LCD_UISRC_DEST_HEIGHT
#else
#define XML_LAYOUT_HEIGHT   (400)
#endif


#define PDT_UI_SCREEN_BRIGHTNESS_LEVEL 400

#define SQUARE(x)  ((x)*(x))
#define DISTANCE(x1,y1,x2,y2) ( (HI_U32)sqrt(SQUARE((x1)-(x2)) + SQUARE((y1)-(y2))) )
#define UI_ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

#define INVALID_HANDLE   (0x0)

#define PDT_UI_KEY_EXPAND  HIGV_KEY_F4
#define PDT_UI_KEY_BACK   HIGV_KEY_F3
#define PDT_UI_KEY_MENU   HIGV_KEY_F2
#define PDT_UI_KEY_OK  HIGV_KEY_F1

#define UI_HOME_EMR_TIME_INTERVAL (50)
#define UI_FILELIST_INVALID_FILE_IDX  (0xFFFFFFFF)

#define SETTING_CONTENT_SIZE  (32)

#define TIMER_REFRESH_HOME (0x20180308)
#define TIMER_REFRESH_HOME_EMR (0x20180813)
#define TIMER_REFRESH_PERIPHERAL (0x20180724)
#define TIMER_REFRESH_ALARM_POWEROFF (0x20180810)
#define TIMER_HOME_HIDE_UI (0x20181010)

#define BACK_CAM_ID (1)

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

#define FILE_GROUP_TYPE_NORMMP4  "normmp4"

#define FILE_GROUP_TYPE_SLOWMP4  "slowmp4"

#define FILE_GROUP_TYPE_LPSEMP4  "lpsemp4"

#define FILE_GROUP_TYPE_LOOPMP4  "lmp4"

#define FILE_GROUP_TYPE_SINGJPG  "singjpg"

#define FILE_GROUP_TYPE_DLAYJPG  "dlayjpg"

#define FILE_GROUP_TYPE_LPSEJPG  "ljpg"

#define FILE_GROUP_TYPE_BURSTJPG "bjpg"



typedef enum tagPDT_UI_OUTPUT_STATE_E
{
    UI_OUTPUT_LCD,
    UI_OUTPUT_HDMI,
    UI_OUTPUT_SWITCHING,
}PDT_UI_OUTPUT_STATE_E;

typedef struct tagPDT_UI_DATETIME_S
{
    HI_U32 year;   /*2018-2037*/
    HI_U32 month;  /*1-12*/
    HI_U32 day;    /*1-31*/
    HI_U32 hour;   /*0-23*/
    HI_U32 minute; /*0-59*/
    HI_U32 second;  /*0-59*/
}PDT_UI_DATETIME_S;

typedef enum tagPDT_UI_VO_VIEWCROP_E
{
    UI_VO_VIEW_UP,
    UI_VO_VIEW_DOWN,
}PDT_UI_VO_VIEWCROP_E;

typedef HI_S32 (*PDT_UI_MSGRESULTPROC_FN_PTR)(HI_EVENT_S* pstEvent);

/**State machine message context, can not send a msg successfully until last msg is finished*/
typedef struct tagPDT_UI_MESSAGE_CONTEXT
{
    HI_MESSAGE_S stMsg;     /**< the message that has been sent*/
    HI_BOOL bMsgProcessed;  /**< the message sent has been processed or not*/
    PDT_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc;   /**< used to process the response*/
    pthread_mutex_t MsgMutex;  /**< the mutex protect sent msg and bool flag*/
}PDT_UI_MESSAGE_CONTEXT;

typedef struct tagPDT_UI_FILE_DIRECTORY_S
{
    HI_DTCF_DIR_E enDirectory;
    HI_CHAR szDirectoryName[HI_DIR_LEN_MAX];
}PDT_UI_DIRECTORY_INFO_S;


HI_S32 PDT_UI_InitLCD(HI_VOID);
HI_S32 PDT_UI_InitHDMI(HI_VOID);

HI_U64 PDT_UI_COMM_GetExecTime(HI_VOID);
HI_U64 PDT_UI_COMM_GetRemainDelayTime(HI_VOID);

HI_VOID PDT_UI_COMM_Second2String(HI_U64 second, HI_CHAR * buffer, HI_U32 size);


HI_S32 PDT_UI_COMM_GetDateTime(PDT_UI_DATETIME_S * pstDateTime);
HI_S32 PDT_UI_COMM_SetDateTime(PDT_UI_DATETIME_S * pstDateTime);
HI_S32 PDT_UI_COMM_GetDateString(PDT_UI_DATETIME_S * pstDateTime, HI_CHAR * pszBuffer, HI_U32 size);
HI_S32 PDT_UI_COMM_GetTimeString(PDT_UI_DATETIME_S * pstDateTime, HI_CHAR * pszBuffer, HI_U32 size);
HI_VOID PDT_UI_COMM_ShowAlarm(HI_U32 u32StringID);

HI_S32 PDT_UI_WINMNG_Init(HI_VOID);
HI_S32 PDT_UI_WINMNG_Deinit(HI_VOID);
HI_S32 PDT_UI_WINMNG_StartWindow(HI_HANDLE WinHdl,HI_BOOL bHideCurWin);
HI_S32 PDT_UI_WINMNG_FinishWindow(HI_HANDLE WinHdl);
HI_BOOL PDT_UI_WINMNG_WindowIsShow(HI_HANDLE WinHdl);


HI_S32 PDT_UI_RegisterTouchDevice();

HI_S32 PDT_UI_COMM_GetStringByID(const HI_U32 u32StrID, HI_CHAR* pszBuffer, HI_U32 u32Size);
HI_S32 PDT_UI_COMM_GetDirName(const HI_DTCF_DIR_E enDir, HI_CHAR* pszBuffer, HI_U32 u32Size);

HI_S32 PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_E enWorkMode, HI_S32 s32CamID, HI_PDT_PARAM_TYPE_E enType,
    HI_CHAR* pszBuffer, HI_U32 u32Size);
HI_S32 PDT_UI_COMM_ActiveNextWidget(HI_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HI_HANDLE CurWidgetHdl);
HI_BOOL PDT_UI_COMM_SdIsReady(HI_VOID);
HI_S32 PDT_UI_COMM_EventProc(HI_EVENT_S* pEvent);
HI_S32 PDT_UI_COMM_SendAsyncMsg(HI_MESSAGE_S* pstMsg, PDT_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc);
HI_S32 PDT_UI_COMM_SubscribeEvents(HI_VOID);
HI_S32 PDT_UI_COMM_POWEROFF(HI_VOID);

#ifdef CFG_TIME_STAMP_ON
HI_VOID PDT_UI_COMM_SwitchPageTime_begin(HI_VOID);
HI_VOID PDT_UI_COMM_SwitchPageTime_end(HI_VOID);
#endif

HI_S32 PDT_UI_HOME_Refresh(HI_VOID);
HI_S32 PDT_UI_HOME_EventProc(HI_EVENT_S * pstEvent);
HI_S32 PDT_UI_HOME_SET_VoCrop(PDT_UI_VO_VIEWCROP_E e_viewcrop);

HI_S32 PDT_UI_FILELIST_EventProc(HI_EVENT_S * pstEvent);
HI_VOID PDT_UI_FILELIST_SetDirectory(HI_DTCF_DIR_E enDir);
HI_VOID PDT_UI_PLAYBACK_VideoPlayerInit(HI_VOID);
HI_S32 PDT_UI_PLAYBACK_PlayFile(HI_VOID);
HI_S32 PDT_UI_FILELIST_Back(HI_VOID);

HI_S32 PDT_UI_SET_VoSwitch(HI_VOID);
HI_S32 PDT_UI_SET_Volume(HI_S32 s32Volume);

HI_S32 PDT_UI_SD_UPGRADE_EventProc(HI_EVENT_S * pstEvent);

HI_S32 PDT_UI_COMM_Translate(const HI_CHAR* pszSrcBuffer, HI_CHAR* pszDestBuffer, HI_U32 u32Size);
HI_S32 PDT_UI_COMM_GetStringByString(const HI_CHAR* pszSrcBuffer, HI_CHAR* pszDestBuffer, HI_U32 u32Size);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
