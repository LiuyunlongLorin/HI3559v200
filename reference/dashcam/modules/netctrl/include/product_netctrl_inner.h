/**
* @file     product_netctrl_inner.h
* @brief    describe the data enum/structure definition,and declare the inner function
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
* @version   1.0

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "libhttpd.h"

#include "hi_product_netctrl.h"
#include "hi_product_param_define.h"
#include "hi_product_param.h"
#include "hi_product_statemng.h"

#include "hi_storagemng.h"
#include "hi_filemng_dtcf.h"
#include "hi_playback.h"
#include "hi_keymng.h"
#include "hi_timedtask.h"
#include "hi_appcomm_log.h"

#include "hisnet.h"
#include "hisnet_type_define.h"
#include "hisnet_argparser.h"

#include "hi_message.h"
#include "hi_eventhub.h"
#include "hi_demuxer.h"
#include "hi_system.h"

#include "hi_upgrade_define.h"
#ifdef CONFIG_MOTIONDETECT_ON
#include "hi_videodetect.h"
#endif

#ifndef __PRODUCT_NETCTRL_INNER_H__
#define __PRODUCT_NETCTRL_INNER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     NETCTRL */
/** @{ */  /** <!-- [NETCTRL] */

#define NETCTRL_ARRAY_SIZE(array)  (sizeof(array)/sizeof(array[0]))
#define NETCTRL_MAX_CABABILITYE_STR_LEN (128)
#ifndef __HuaweiLite__
#define NETCTRL_FILE_ROOT_PATH "/app/"
#else
#define NETCTRL_FILE_ROOT_PATH "/"
#endif
/** the webserver client max count */
#define NETCTRL_MAX_WEBSERVER_CLIENT_NUM (2)

#define CLIENT_CONNECT_TIMEOUT (2)

#define CLIENT_MSG_RECV_PORT  (9002)

#define EVENT_ID_MAX_LEN (256)
#define BACK_CAM_ID (1)
#define Front_CAM_ID (0)

/** item value description maximum length */
#define PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN  (32)
#define PDT_NETCTRL_STR_LEN  (32)

#define CLIENT_IP_LEN (16)

/** workmode description */
#define PDT_NETCTRL_WORKMODE_NORM_REC     "NORM_REC"
#define PDT_NETCTRL_WORKMODE_PHOTO        "PHOTO"
#define PDT_NETCTRL_WORKMODE_PLAYBACK     "PLAYBACK"
#define PDT_NETCTRL_WORKMODE_UVC          "UVC"
#define PDT_NETCTRL_WORKMODE_USB_STORAGE  "USB_STORAGE"
#define PDT_NETCTRL_WORKMODE_PARKING_REC  "PARKING_REC"
#define PDT_NETCTRL_WORKMODE_UPGRADE      "UPGRADE"

/** param type description */
/** WorkMode Dependent Param */
#define PDT_NETCTRL_PARAM_TYPE_MEDIAMODE            "MEDIAMODE"
#define PDT_NETCTRL_PARAM_TYPE_ENC_PAYLOAD_TYPE     "ENC_PAYLOAD_TYPE"
#define PDT_NETCTRL_PARAM_TYPE_FLIP                 "FLIP"
#define PDT_NETCTRL_PARAM_TYPE_MIRROR               "MIRROR"
#define PDT_NETCTRL_PARAM_TYPE_LDC                  "LDC"
#define PDT_NETCTRL_PARAM_TYPE_CROP                 "VO_CROP"
#define PDT_NETCTRL_PARAM_TYPE_BAKE_REC             "BACK_REC"
#define PDT_NETCTRL_PARAM_TYPE_OSD                  "OSD"
#define PDT_NETCTRL_PARAM_TYPE_AUDIO                "AUDIO"
#define PDT_NETCTRL_PARAM_TYPE_PREVIEW_CAMID        "PREVIEW_CAMID"
#define PDT_NETCTRL_UPGRADE                         "UPGRADE"
#define PDT_NETCTRL_PARAM_TYPE_WDR                  "WDR"
#define PDT_NETCTRL_PARAM_TYPE_VIDEOMODE            "VIDEOMODE"
#define PDT_NETCTRL_PARAM_TYPE_LANGUAGE             "LANGUAGE"

#define PDT_NETCTRL_PARAM_TYPE_REC_TYPE             "REC_TYPE"
#define PDT_NETCTRL_PARAM_TYPE_LAPSE_INTERVAL       "LAPSE_INTERVAL"
#define PDT_NETCTRL_PARAM_TYPE_SPLITTIME            "Rec_Split_Time"
#define PDT_NETCTRL_PARAM_TYPE_PREVIEWPIP           "PreviewPIP"
#define PDT_NETCTRL_PARAM_TYPE_MD_SENSITIVITY       "MD_SENSITIVITY"
#define PDT_NETCTRL_PARAM_TYPE_POWERON_WORKMODE     "POWERON_WORKMODE"


/** param type description */
/** WorkMode Independent Param */
#define PDT_NETCTRL_PARAM_TYPE_WIFI_AP              "WIFI_AP"
#define PDT_NETCTRL_PARAM_TYPE_DEV_INFO             "DEV_INFO"
#define PDT_NETCTRL_PARAM_TYPE_VOLUME               "VOLUME"
#define PDT_NETCTRL_PARAM_TYPE_SCREEN_BRIGHTNESS    "SCREEN_BRIGHTNESS"
#define PDT_NETCTRL_PARAM_TYPE_SCREEN_DORMANT_MODE  "SCREEN_DORMANT"
#define PDT_NETCTRL_PARAM_TYPE_KEYTONE              "KEYTONE"
#define PDT_NETCTRL_PARAM_TYPE_BOOTSOUND            "BOOTSOUND"
#define PDT_NETCTRL_PARAM_TYPE_GSENSOR_SENSITIVITY  "GSR_SENSITIVITY"
#define PDT_NETCTRL_PARAM_TYPE_USB_MODE             "USB_MODE"
#define PDT_NETCTRL_PARAM_TYPE_GSENSOR_PARKING      "GSR_PARKING"
#define PDT_NETCTRL_PARAM_TYPE_PARKING_COUNT        "PARKING_COLLISION_CNT"

#define PDT_NETCTRL_ON   "1"
#define PDT_NETCTRL_OFF  "0"
#define PDT_NETCTRL_H264 "H264"
#define PDT_NETCTRL_H265 "H265"

#define PDT_NETCTRL_SUCCESS "SUCCESS"
#define PDT_NETCTRL_FAILURE "FAILURE"


/** Key Type Description */
#define PDT_NETCTRL_KEY_MENU "KEY_MENU"
#define PDT_NETCTRL_KEY_BACK "KEY_BACK"
#define PDT_NETCTRL_KEY_OK   "KEY_OK"

/** Log Level Description */
#define PDT_NETCTRL_LOG_FATAL   "FATAL"
#define PDT_NETCTRL_LOG_ERROR   "ERROR"
#define PDT_NETCTRL_LOG_WARNING "WARNING"
#define PDT_NETCTRL_LOG_INFO    "INFO"
#define PDT_NETCTRL_LOG_DEBUG   "DEBUG"

/** Upgrade */
#define PDT_UPGRADE_RECEIVE_SEC (2)
#define PDT_UPGRADE_MAX_WAITCOUNT (15)
#define PDT_UPGRADE_MAX_FILENAME (256)
#define PDT_UPGRADE_UNITSIZE (256*1024)
#define NETCTRL_MAX_RECEIVE_COUNT (1)


typedef struct hi_PDT_UPGRADE_RECEIVE_INFO_S
{
    HI_BOOL bEnable;
    HI_U32 u32OffSet;
    HI_U32 u32UnitSize;
    HI_U32  u32PktLen;                              /**<package Size */
    HI_U32 u32RecvBufLen;
    FILE* pFileFP;
    HI_CHAR szPath[PDT_UPGRADE_MAX_FILENAME];
    HI_UPGRADE_DEV_INFO_S stDevInfo;
} HI_PDT_UPGRADE_RECEIVE_INFO_S;

/** remember the message that has sent and hasn't processed */
typedef struct tagPDT_NETCTRL_MESSAGE_CONTEXT
{
    HI_MESSAGE_S stMsg;     /**< the message that has been sent*/
    HI_BOOL bMsgProcessed;  /**< the flag of message has been processed or not*/
    pthread_mutex_t MsgMutex;  /**< lock*/
} PDT_NETCTRL_MESSAGE_CONTEXT;

/** the client info */
typedef struct tagPDT_NETCTRL_CLIENT_INFO_S
{
    HI_BOOL bConnected;   /**< the client has connected or not*/
    HI_CHAR szIPv4Addr[CLIENT_IP_LEN]; /**< the IPAddr of the client that has connected*/
} PDT_NETCTRL_CLIENT_INFO_S;

typedef struct tagPDT_NETCTRL_REMOTEFILE_WRAPPER_S
{
    HISNET_REMOTEFILENAME_S* pstFileList;
    HI_U32 u32Cnt;
} PDT_NETCTRL_REMOTEFILE_WRAPPER_S;


/**  item value description structure */
typedef struct tagPDT_NETCTRL_ITEM_VALUE_S
{
    HI_CHAR szDesc[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN]; /**<item value description in string */
    HI_S32  s32Value; /**<item value */
} PDT_NETCTRL_ITEM_VALUE_S;


/**
* @brief    Set Sys time ,
* @param[in] pstSysTime, eg. HISNET_TIME_ATTR_S
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/6/30
*/
HI_S32 PDT_NETCTRL_SetTime(const HISNET_TIME_ATTR_S* pstSysTime);


/**
* @brief    get event ID desc string ,
* @param[in] u32EventID, eg. HI_EVENT_STATEMNG_START
* @param[out] pszDesc : desc string
* @param[out] u32BufLen : desc buffer len
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/6/13
*/
HI_S32 PDT_NETCTRL_GetEventIDDescByValue(HI_U32 u32EventID, HI_CHAR* pszDesc, HI_U32 u32BufLen);


/**
* @brief    get specified file information, eg. filesize, resolution
* @param[in] pazFileName, suffix should be MP4/LRV
* @param[in] pstFileInfo : file information
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/18
*/
HI_S32 PDT_NETCTRL_GetVideoFileExhaustiveInfo(const HI_CHAR* pazFileName, HI_FORMAT_FILE_INFO_S* pstFileInfo);
HI_VOID PDT_NETCTRL_GetDlsym(HI_VOID);


/**
* @brief     get workmode value by workmode description
* @param[in] pName
* @param[in] ps32Value
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 PDT_NETCTRL_GetWorkModeValueByDesc(HI_CHAR* pDesc, HI_S32* ps32WorkMode);


/**
* @brief     get workmode description by paramtype value
* @param[in] pDesc
* @param[in] ps32Value
* @param[in] u32SiZe
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 PDT_NETCTRL_GetWorkModeDescByValue(HI_CHAR* pDesc, HI_S32 s32WorkMode, HI_S32 u32SiZe);


/**
* @brief     get paramtype value by paramtype description
* @param[in] pName
* @param[in] ps32Value
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 PDT_NETCTRL_GetParamTypeValueByDesc(HI_CHAR* pDesc, HI_S32* ps32WorkMode);


/**
* @brief     get paramtype description by paramtype value
* @param[in] pDesc
* @param[in] ps32Value
* @param[in] u32SiZe
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 PDT_NETCTRL_GetParamTypeDescByValue(HI_CHAR* pDesc, HI_S32 s32WorkMode, HI_S32 u32SiZe);


/**
* @brief     get param value description
* @param[in] enWorkMode
* @param[in] enType
* @param[in] pDesc
* @param[in] u32Size
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/26
*/
HI_S32 PDT_NETCTRL_GetParamValueDesc(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_TYPE_E enType,
                                     HI_CHAR* pDesc, HI_U32 u32Size);


/**
* @brief     get param value by description
* @param[in] enWorkMode
* @param[in] enType
* @param[in] pDesc
* @param[in] u32Size
* @param[in] ps32Value
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/26
*/
HI_S32 PDT_NETCTRL_GetParamValueByDesc(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_TYPE_E enType,
                                       HI_CHAR* pDesc, HI_U32 u32Size, HI_S32* ps32Value);


/**
* @brief     set common param
* @param[in] enType
* @param[in] pvParam
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/27
*/
HI_S32 PDT_NETCTRL_SetCommParam(HI_PDT_PARAM_TYPE_E enType, HI_VOID* pvParam);


/**
* @brief     get log level value by description
* @param[in] pDesc
* @param[in] ps32LogLevel
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/19
*/
HI_S32 PDT_NETCTRL_GetLogLevelValueByDesc(HI_CHAR* pDesc, HI_S32* ps32LogLevel);


/**
* @brief     get file directory value by description
* @param[in] pDesc
* @param[in] penfiledir
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/6/23
*/
HI_S32 PDT_NETCTRL_GetFileDirValueByDesc(HI_CHAR* pDesc, HI_DTCF_DIR_E* ps32filedir);

/**
* @brief     get Get Sd Prompt Info
* @param[in] sdPromptInfo
* @param[in] descriptionSize
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2019/4/17
*/
HI_S32 PDT_NETCTRL_GetSdPromptInfo(HI_CHAR *description, HI_S32 descriptionSize);

/** @}*/  /** <!-- ==== NETCTRL End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __PRODUCT_NETCTRL_INNER_H__*/


