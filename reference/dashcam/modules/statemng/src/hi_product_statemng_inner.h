/**
 * @file      hi_product_statemng_inner.h
 * @brief     Describes the data structure definition,the micro definition,
 *            and the function definition inner statemng module.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 * @version   1.0
 */
#ifndef __HI_PRODUCT_STATEMNG_INNER__
#define __HI_PRODUCT_STATEMNG_INNER__

#include <stdio.h>
#include <pthread.h>
#include <memory.h>
#include <semaphore.h>
#include <signal.h>
#include "hi_type.h"
#include "hi_hfsm.h"
#include "hi_eventhub.h"
#include "hi_mapi_comm_define.h"
#include "hi_recordmng.h"
#include "hi_storagemng.h"
#include "hi_filemng_comm.h"
#include "hi_product_param.h"
#include "hi_filemng_dtcf.h"
//#include "hi_scene_master.h"
#include "hi_appcomm_log.h"
#include "hi_appcomm_util.h"
#include "hi_timestamp.h"
#include "hi_hal_common.h"
#include "hi_usb.h"
#include "hi_upgrade.h"
#include "hi_product_usbctrl.h"
#include "hi_hal_screen.h"
#include "hi_product_media.h"
#ifdef CONFIG_MOTIONDETECT_ON
#include "hi_videodetect.h"
#endif
#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"
#ifdef CONFIG_RAWCAP_ON
#include "hi_rawcapture.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     STATEMNG */
/** @{ */  /** <!-- [STATEMNG] */

/** redefine module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "StateMng"

#define PDT_STATEMNG_MODULE_NAME                 "StateMng"

/** state name */
#define PDT_STATEMNG_STATE_BASE                  "Base"
#define PDT_STATEMNG_STATE_REC_NORMAL            "Rec_Normal"
#define PDT_STATEMNG_STATE_PHOTO                 "Photo"
#define PDT_STATEMNG_STATE_PLAYBACK              "PlayBack"
#define PDT_STATEMNG_STATE_UVC                   "UVC"
#define PDT_STATEMNG_STATE_USB_STORAGE           "UsbStorage"
#define PDT_STATEMNG_STATE_PARKING_REC           "Parking_Rec"
#define PDT_STATEMNG_STATE_UPGRADE               "Upgrade"

/** recmng function timeout */
#define PDT_STATEMNG_RECMNG_OPERATION_TIMEOUT_MS (3000)

/** Cam Id */
#define PDT_STATEMNG_FRONT_CAMID      (0)
#define PDT_STATEMNG_BACK_CAMID       (1)

/** no timeout */
#define PDT_STATEMNG_RECMNG_NO_TIMEOUT           (-1)

/** recmng play fps(for lapse) */
#define PDT_STATEMNG_RECMNG_LAPSE_PLAY_FPS         (30)
/** array size */
#define PDT_STATEMNG_ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

/** liveserver max connect number */
#define PDT_STATEMNG_LIVESVR_MAX_CONNECT_CNT       (1)

#ifdef CONFIG_RAWCAP_ON
#define PDT_STATEMNG_RAWCAP_DEPTH                  (1)
#endif

/** NULL pointer check */
#define PDT_STATEMNG_CHECK_POINTER(ptr,errcode,string)\
do{\
    if(NULL == ptr)\
     {\
        MLOGE(RED"%s NULL pointer\n\n"NONE,string);\
        return errcode;\
     }\
  }while(0)

/** Expression Check */
#define PDT_STATEMNG_CHECK_EXPR(expr, errcode) \
    do { \
        if (!(expr)){   \
            MLOGE("expr[%s] false\n", #expr); \
            return errcode;  \
        }   \
    }while(0)

/** message proc function parameter check */
#define PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(argv,pStateID,Message,InProgress)\
do{\
    if(NULL == argv || NULL == pStateID || NULL == Message)\
    {\
        MLOGE(RED"parameter argv or pStateID or Message NULL\n\n"NONE);\
        MUTEX_LOCK(PDT_STATEMNG_GetCtx()->Mutex);\
        InProgress = HI_FALSE;\
        MUTEX_UNLOCK(PDT_STATEMNG_GetCtx()->Mutex);\
        PDT_STATEMNG_PublishEvent(Message, HI_FAILURE);\
        return HI_PROCESS_MSG_RESULTE_OK;\
    }\
  }while(0)

/** workmode check */
#define PDT_STATEMNG_CHECK_MSG_WORKMODE(CurrentWorkMode,MessageWorkMode,Message,InProgress)\
do{\
    if(CurrentWorkMode != MessageWorkMode)\
    {\
        MLOGE(RED"workmode in message error, CurrentWorkMode(%d), MessageWorkMode(%d)\n\n"NONE,\
              CurrentWorkMode, MessageWorkMode);\
        MUTEX_LOCK(PDT_STATEMNG_GetCtx()->Mutex);\
        InProgress = HI_FALSE;\
        MUTEX_UNLOCK(PDT_STATEMNG_GetCtx()->Mutex);\
        PDT_STATEMNG_PublishEvent(Message, HI_FAILURE);\
        return HI_PROCESS_MSG_RESULTE_OK;\
    }\
  }while(0)

/** function ret value check */
#define PDT_STATEMNG_CHECK_RET(ret,errcode,errstring)\
do{\
    if(HI_SUCCESS != ret)\
    {\
        MLOGE(RED"%s failed, s32Ret(0x%08X)\n\n"NONE, errstring, ret);\
        return errcode;\
    }\
    else\
    {\
        HI_TIME_STAMP;\
    }\
  }while(0)

/** function ret value check */
#define PDT_STATEMNG_CHECK_RET_AND_UPDATESTATUS(pstMsg,ret,errcode,errstring)\
do{\
    if(HI_SUCCESS != ret)\
    {\
        MLOGE(RED"%s failed, s32Ret(0x%08X)\n\n"NONE, errstring, ret);\
        MUTEX_LOCK(PDT_STATEMNG_GetCtx()->Mutex);\
        PDT_STATEMNG_GetCtx()->bInProgress = HI_FALSE;\
        MUTEX_UNLOCK(PDT_STATEMNG_GetCtx()->Mutex);\
        PDT_STATEMNG_PublishEvent(pstMsg, ret);\
        return errcode;\
    }\
  }while(0)

/** just check return value,but not return */
#define PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(ret,errstring)\
do{\
    if(HI_SUCCESS != ret)\
    {\
        MLOGE(RED"%s failed, s32Ret(0x%08X)\n\n"NONE, errstring, ret);\
    }\
  }while(0)

/**check ret value, unlock mutex when error */
#define PDT_STATEMNG_CHECK_RET_WITH_UNLOCK(retvalue,errcode,errstring)\
do{\
    if(HI_SUCCESS != retvalue)\
    {\
        MLOGE(RED"%s failed, s32Ret(0x%08X)\n\n"NONE, errstring, retvalue);\
        MUTEX_UNLOCK(PDT_STATEMNG_GetCtx()->Mutex);\
        return errcode;\
    }\
  }while(0)

/** update statemng status */
#define PDT_STATEMNG_UPDATESTATUS(pstMsg,Result,bProgress)\
do{\
    MUTEX_LOCK(PDT_STATEMNG_GetCtx()->Mutex);\
    PDT_STATEMNG_GetCtx()->bInProgress = bProgress;\
    MUTEX_UNLOCK(PDT_STATEMNG_GetCtx()->Mutex);\
    PDT_STATEMNG_PublishEvent(pstMsg, Result);\
  }while(0)

/** state attribution */
typedef struct tagPDT_STATEMNG_STATE_ATTR_S
{
    HI_STATE_S stState;
    HI_BOOL bRunning;
}PDT_STATEMNG_STATE_ATTR_S;

/** record task context */
typedef struct tagPDT_STATEMNG_RECTASK_CONTEXT_S
{
    HI_S32 s32CamID;
    HI_BOOL bEnable;
    HI_HANDLE RecMngTaskHdl;
    HI_BOOL bFileEndRecv;
    HI_BOOL bAudioStarted;
    HI_RECMNG_ATTR_S stRecMngAttr;
}PDT_STATEMNG_RECTASK_CONTEXT_S;

/** record context */
typedef struct tagPDT_STATEMNG_REC_CONTEXT_S
{
    HI_BOOL bEMRRecord; /**<EMR record status */
    HI_BOOL bRecStarted; /**<record status */
    PDT_STATEMNG_RECTASK_CONTEXT_S  astRecTaskCtx[HI_PDT_RECTASK_MAX_CNT];
}PDT_STATEMNG_REC_CONTEXT_S;

#ifdef CONFIG_RAWCAP_ON
/** rawcap task context */
typedef struct tagPDT_STATEMNG_RAWCAPTASK_CONTEXT_S
{
    HI_S32 s32CamID;
    HI_BOOL bEnable;
    HI_BOOL bRawCapTaskCreate;
    HI_HANDLE RawCapTaskHdl;
    HI_HANDLE VcapDevHdl;
    HI_CHAR szPrefix[HI_APPCOMM_MAX_FILENAME_LEN];
    HI_RAWCAP_CFG_S stRawCapCfg;
}PDT_STATEMNG_RAWCAPTASK_CONTEXT_S;

/** rawcap context */
typedef struct tagPDT_STATEMNG_RAWCAP_CONTEXT_S
{
    HI_BOOL bDebug;
    PDT_STATEMNG_RAWCAPTASK_CONTEXT_S  astRawCapTaskCtx[HI_PDT_RAWCAPTASK_MAX_CNT];
}PDT_STATEMNG_RAWCAP_CONTEXT_S;
#endif

/** photo task context */
typedef struct tagPDT_STATEMNG_PHOTOASK_CONTEXT_S
{
    HI_S32    s32CamID;
    HI_HANDLE PhotoMngTaskHdl;
    HI_BOOL   bEnable;
    HI_BOOL   bISPBypss;
    HI_PHOTOMNG_PHOTO_ATTR_S stPhotoAttr;
}PDT_STATEMNG_PHOTOTASK_CONTEXT_S;

/** photo context */
typedef struct tagPDT_STATEMNG_PHOTO_CONTEXT_S
{
    PDT_STATEMNG_PHOTOTASK_CONTEXT_S stPhotoTaskCtx[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
}PDT_STATEMNG_PHOTO_CONTEXT_S;

typedef struct tagPDT_STATEMNG_PARKING_REC_CONTEXT_S
{
    HI_HANDLE TimeTaskHdl;
    HI_S32 s32SumEmrTime;
    HI_S32 s32EmrCurTime;
}PDT_STATEMNG_PARKING_REC_CONTEXT_S;

typedef struct tagPDT_STATEMNG_POWER_CHECK_CONTEXT_S
{
    HI_HANDLE TimeTaskHdl;
 }PDT_STATEMNG_POWER_CHECK_CONTEXT_S;


#ifdef CONFIG_MOTIONDETECT_ON
typedef struct tagPDT_STATEMNG_VIDEODETECTTASK_CONTEXT_S
{
    HI_BOOL bVideoDetectTaskCreate;
    HI_HANDLE VideoDetectTaskHdl;
} PDT_STATEMNG_VIDEODETECTTASK_CONTEXT_S;

typedef struct tagPDT_STATEMNG_VIDEODETECT_CONTEXT_S
{
    PDT_STATEMNG_VIDEODETECTTASK_CONTEXT_S  astVideoDetectTaskCtx[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
} PDT_STATEMNG_VIDEODETECT_CONTEXT_S;
#endif

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
typedef struct tagPDT_STATEMNG_TEMPCYCLE_CONTEXT_S
{
    HI_BOOL bHighTemp;
} PDT_STATEMNG_TEMPCYCLE_CONTEXT_S;
#endif


/** statemng context */
typedef struct tagPDT_STATEMNG_CONTEXT_S
{
    HI_BOOL bInited;
    pthread_mutex_t Mutex;/**<Mutex for bInProgress*/
    HI_MW_PTR pvHfsmHdl;
    PDT_STATEMNG_STATE_ATTR_S stBase;
    HI_PDT_WORKMODE_E enCurrentWorkMode;
    HI_BOOL bInProgress;
    HI_BOOL bRunning;
    HI_BOOL bSDAvailable;
    HI_PDT_STATEMNG_CONFIG_S stStatemngCfg;

    HI_PDT_PARAM_CAM_CONTEXT_S astCamCtx[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    PDT_STATEMNG_REC_CONTEXT_S stRecCtx;
    PDT_STATEMNG_PHOTO_CONTEXT_S stPhotoCtx;
    HI_PDT_FILEMNG_CFG_S stFileMngCfg;
    HI_STORAGEMNG_CFG_S stStorageMngCfg;
    HI_S32 s32LiveCamID;
    HI_S32 s32PreviewCamID;
    HI_STORAGE_STATE_E enPowerOnState;
    HI_BOOL bScreenDormantStatus;
    PDT_STATEMNG_POWER_CHECK_CONTEXT_S stPowerCheckCtx;
    PDT_STATEMNG_PARKING_REC_CONTEXT_S stParkingRecCtx;
    #ifdef CONFIG_RAWCAP_ON
    PDT_STATEMNG_RAWCAP_CONTEXT_S stRawCapCtx;
    #endif
#ifdef CONFIG_MOTIONDETECT_ON
    PDT_STATEMNG_VIDEODETECT_CONTEXT_S stVideoDetectCtx;
#endif

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
    PDT_STATEMNG_TEMPCYCLE_CONTEXT_S stTempCycleCtx;
#endif

    HI_BOOL bForceReBuid;
}PDT_STATEMNG_CONTEXT;

/**
 * @brief        change workmode to state
 * @param[in]    enPoweronWorkmode
 * @param[in]    s32NameLength
 * @param[out]   pszStateName
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_WorkMode2State(HI_PDT_WORKMODE_E enWorkmode,
                                   HI_CHAR *pszStateName, HI_S32 s32NameLength);

/**
 * @brief        change state to workmode
 * @param[in]    pszStateName
 * @param[out]   penPoweronWorkmode
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_State2WorkMode(const HI_CHAR *pszStateName,
                                   HI_PDT_WORKMODE_E *penWorkmode);

/**
* @brief    publish message process result event
* @param[in] pstMsg: message pointer
* @param[in] s32Result: result
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/16
*/
HI_S32 PDT_STATEMNG_PublishEvent(HI_MESSAGE_S *pstMsg, HI_S32 s32Result);

/**
 * @brief        reset Media
 * @param[in]    pstMediaCfg
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_ResetMedia(const HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
 * @brief        search new upgrade Package
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/08/30
 */
HI_S32 PDT_STATEMNG_UpgradeSrchNewPkt(HI_VOID);

/**
* @brief    generate record context
* @param[in] enEnterWorkMode: enter workmode
* @param[in] pstEnterWorkModeCfg: enter workmode config
* @param[out] pstRecCtx: record context
* @param[out] pstMediaCfg: media config updated by record context
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/17
*/
HI_S32 PDT_STATEMNG_GenerateRecCtx(HI_PDT_WORKMODE_E enEnterWorkMode,
                                   const HI_PDT_WORKMODE_CFG_S* pstEnterWorkModeCfg,
                                   PDT_STATEMNG_REC_CONTEXT_S* pstRecCtx,HI_PDT_MEDIA_CFG_S* pstMediaCfg);

/**
* @brief    generate photo context
* @param[in] enEnterWorkMode: enter workmode
* @param[in] pstEnterWorkModeCfg: enter workmode config
* @param[out] pstPhotoCtx: photo context
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/17
*/
HI_S32 PDT_STATEMNG_GeneratePhotoCtx(HI_U32 u32CamID,
                                       HI_PDT_WORKMODE_E enEnterWorkMode,
                                       const HI_PDT_WORKMODE_CFG_S *pstEnterWorkModeCfg,
                                       PDT_STATEMNG_PHOTOTASK_CONTEXT_S* pstPhotoTaskCtx);

/**
* @brief    change rec type
* @param[in] enRecType: enter rec ype
* @param[out]   None
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/12/13
*/
HI_S32 PDT_STATEMNG_ChangeRecType(HI_REC_TYPE_E enRecType);

/**
* @brief    check setting item value
* @param[in] pstMsg: message pointer
* @param[in] s32PreviewCamID: preview cam id
* @param[in] s32SettingItemValue: setting value
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/17
*/
HI_S32 PDT_STATEMNG_CheckCamSettingItemValue(HI_MESSAGE_S * pstMsg,HI_S32 s32PreviewCamID, HI_S32 s32SettingItemValue);

/**
* @brief    set audio
* @param[in] pstMsg: message pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/16
*/
HI_S32 PDT_STATEMNG_SetAudio(HI_MESSAGE_S *pstMsg);


/**
* @brief    :set flip
* @param[in] pstMsg: message pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/16
*/
HI_S32 PDT_STATEMNG_SetFlip(HI_MESSAGE_S *pstMsg);

/**
* @brief set mirror
* @param[in] pstMsg
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/17
*/
HI_S32 PDT_STATEMNG_SetMirror(HI_MESSAGE_S *pstMsg);


/**
* @brief    set osd
* @param[in] pstMsg: message pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/16
*/
HI_S32 PDT_STATEMNG_SetOSD(HI_MESSAGE_S *pstMsg);

/**
* @brief    set LDC
* @param[in] pstMsg: message pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/16
*/
HI_S32 PDT_STATEMNG_SetLDC(HI_MESSAGE_S *pstMsg);

/**
* @brief    set Rec SplitTime
* @param[in] pstMsg: message pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/16
*/
HI_S32 PDT_STATEMNG_SetRecSplitTime(HI_MESSAGE_S* pstMsg);

/**
* @brief    set WDR
* @param[in] pstMsg: message pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/16
*/
HI_S32 PDT_STATEMNG_SetWDR(HI_MESSAGE_S *pstMsg);

/**
* @brief    set videomode
* @param[in] pstMsg: message pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/16
*/
HI_S32 PDT_STATEMNG_SetVideoMode(HI_MESSAGE_S *pstMsg);

/**
* @brief    set crop
* @param[in] pstMsg: message pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/16
*/
HI_S32 PDT_STATEMNG_SetCrop(HI_MESSAGE_S *pstMsg);

/**
* @brief    set record
* @param[in] pstMsg: message pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/17
*/
HI_S32 PDT_STATEMNG_SetRecEnable(HI_MESSAGE_S *pstMsg);

/**
* @brief    set usb mode
* @param[in] HI_VOID
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/12/15
*/
HI_S32 PDT_STATEMNG_SetUsbMode(HI_VOID);

/**
* @brief    Set Power on Work Mode
* @param[in] work mode
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2019/02/28
*/
HI_S32 PDT_STATEMNG_SetPoweronWorkMode(HI_PDT_WORKMODE_E enSettingWorkmode);


/**
* @brief    Update Live Switch Osd State
* @param[in] Live ID
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2019/02/28
*/
HI_S32 PDT_STATEMNG_UpdateLiveSwitchOsdState(HI_S32 s32LiveCamID);

/**
* @brief    init statemng context
* @param[in] stStatemngCfg: statemng configure pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/26
*/
HI_S32 PDT_STATEMNG_ContextInit(const HI_PDT_STATEMNG_CONFIG_S* pstStatemngCfg);

/**
* @brief    get statemng context pointer
* @param[in] HI_VOID
* @return statemng context pointer
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/26
*/
PDT_STATEMNG_CONTEXT *PDT_STATEMNG_GetCtx(HI_VOID);

/**
* @brief    get media mode context by specified workmode
* @param[in] enWorkMode: workmode
* @param[out] pstCamCtx: Cam context array pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/5/23
*/
HI_S32 PDT_STATEMNG_GetMediaModeCfg(HI_PDT_WORKMODE_E enWorkMode,HI_PDT_PARAM_CAM_CONTEXT_S *pstCamCtx);

/**
* @brief    update video out configure
* @param[in] pstVideoOutCfg: video out configure structure pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/5/23
*/

HI_S32 PDT_STATEMNG_UpdateDispCfg(HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
* @brief    Get media configure
* @param[in] pstMediaCfg: media configure structure pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/5/23
*/
HI_S32 PDT_STATEMNG_GetMediaCfg(HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
 * @brief        init Base state
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_BaseStateInit(HI_VOID);

/**
 * @brief        deinit Base state
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_BaseStateDeinit(HI_VOID);

/**
 * @brief        init Rec states, add all Rec states to HFSM, init RecMng module
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_RecStatesInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit Rec states
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_RecStatesDeinit(HI_VOID);

/**
 * @brief        init parking rec state
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_ParkingRecStatesInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);
/**
 * @brief        deinit parking rec state
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_ParkingRecStatesDeinit(HI_VOID);

/**
* @brief    record manualsplit
* @param[in] bStopAfterManualSplitEnd:record stop after Manual Split End
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/27
*/
HI_S32 PDT_STATEMNG_RecManualSplit(HI_BOOL bStopAfterManualSplitEnd);

/**
 * @brief        start record
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_StartRec(HI_VOID);
/**
 * @brief        stop record
 * @param[in]    bSync:stoprecord style,
                        HI_TRUE:stop sync
                        HI_FALSE:stop async
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_StopRec(HI_BOOL bSync);

#ifdef CONFIG_MOTIONDETECT_ON
/**
 * @brief        start md
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/12/26
 */
HI_S32 PDT_STATEMNG_StartMd(HI_VOID);

/**
 * @brief        stop md
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/12/26
 */
HI_S32 PDT_STATEMNG_StopMd(HI_VOID);

#endif

/**
 * @brief        add rtsp streams
 * @param[in]    pstMediaCfg: media configureation
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_AddRtspStreams(const HI_PDT_MEDIA_CFG_S *pstMediaCfg);
/**
 * @brief        init Playback state
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_PlaybackStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit Playback State
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_PlaybackStateDeinit(HI_VOID);

/**
 * @brief        init UVC state
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_UVCStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit UVC State
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_UVCStateDeinit(HI_VOID);

/**
 * @brief        init USBStorage state
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_USBStorageStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        Reset part of mediacfg for Playback
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_VOID PDT_STATEMNG_MediaCfgResetForPlayback(HI_PDT_MEDIA_CFG_S* pstMediaCfg);
/**
 * @brief        deinit USBStorage State
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_USBStorageStateDeinit(HI_VOID);

/**
 * @brief        init Upgrade state
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/08/30
 */
HI_S32 PDT_STATEMNG_UpgradeStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit Upgrade State
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/08/30
 */
HI_S32 PDT_STATEMNG_UpgradeStateDeinit(HI_VOID);

/**
 * @brief        get extern power supply State
 * @param[in]    pbIsOn :extern power supply status
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/04/17
 */
HI_S32 PDT_STATEMNG_GetPowerSupplyStatue(HI_BOOL* pbIsOn);

/**
 * @brief        creat extern power supply check time task
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/04/17
 */
HI_S32 PDT_STATEMNG_CreatePowerSupplyCheckTask(HI_VOID);

/**
 * @brief        destroy extern power supply check time task
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/04/17
 */
HI_S32 PDT_STATEMNG_DestroyPowerSupplyCheckTask(HI_VOID);


/**
* @brief    get preview camera id
* @param[in] HI_VOID
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/26
*/
HI_S32 PDT_STATEMNG_GetPreViewCamID(HI_VOID);

/**
* @brief   init filemng
* @param[in] HI_VOID
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/26
*/
HI_S32 PDT_STATEMNG_InitFileMng(HI_VOID);

/**
* @brief   send shutdown message
* @param[in] HI_VOID
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2019/1/25
*/
HI_VOID PDT_STATEMNG_SendShutdownMsg(HI_VOID);

/**
* @brief   get photo file name
* @param[in] s32FileCnt:photo count
* @param[in] pvPrivateData:photo param
* @param[out] pstFileList:get photo file name
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2019/2/22
*/
HI_S32 PDT_STATEMNG_GetPhotoFileName(HI_S32 s32FileCnt, HI_PHOTOMNG_FILENAME_S* pstFileList, HI_VOID* pvPrivateData);

/**
* @brief   get photo file info
* @param[in] pvPrivateData:photo info
* @param[out] pstPhotoParam:get photo exif  info
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2019/2/22
*/
HI_S32 PDT_STATEMNG_GetPhotoParam(HI_PHOTOMNG_PHOTO_PARAM_S* pstPhotoParam, HI_VOID* pvPrivateData);
/**
 * @brief        init photo states, add all photo states to HFSM, init PhotoMng module
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2019/02/208
 */
HI_S32 PDT_STATEMNG_PhotoStatesInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit photo states
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2019/02/208
 */
HI_S32 PDT_STATEMNG_PhotoStatesDeinit(HI_VOID);

/**
 * @brief        Emergency task end route proc
 * @param[in]    entype recordmng event type
 * @param[in]    RecMngTaskHdl recordmng task id
 * @param[in]    bForceEnd force emergency task
 * @return       void
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2019/02/208
 */
HI_VOID PDT_STATEMNG_EmrTaskEndProc(HI_EVENT_RECMNG_E entype,HI_HANDLE RecMngTaskHdl,HI_BOOL bForceEnd);

/**
 * @brief        Storge mount event proc
 * @param[in]    enPoweronWorkmode :work mode
 * @return       void
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2019/02/208
 */
HI_VOID PDT_STATEMNG_MountedMsgProc(HI_PDT_WORKMODE_E enPoweronWorkmode);

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
/**
 * @brief        update Temperature Cycle state
 * @param[in]   bHighTemp :high temp or not
 * @return       void
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date         2019/02/208
 */
HI_S32 PDT_STATEMNG_UpdateTempCycleState(HI_BOOL bHighTemp);
#endif

/**
 * @brief        gps data start
 * @param[in]    Private Data handle
 * @param[out]   Private Data info
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2019/03/28
 */
HI_S32 PDT_STATEMNG_GPSDataStart(HI_HANDLE privDataHdl, HI_VOID* privDataInfo);

/**
 * @brief        gps data stop
 * @param[in]    Private Data handle
 * @param[out]   Private Data info
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2019/03/28
 */

HI_S32 PDT_STATEMNG_GPSDataStop(HI_HANDLE privDataHdl, HI_VOID* privDataInfo);

/**
 * @brief        get gps data cfg
 * @param[in]    Private Data handle
 * @param[out]   Private Data info
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2019/03/28
 */

HI_S32 PDT_STATEMNG_GetGPSDataCfg(HI_HANDLE PrivDataHdl, HI_RECMNG_PRIV_DATA_INFO_S* privateInfo);
/** @}*/  /** <!-- ==== STATEMNG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_STATEMNG_INNER__ */
