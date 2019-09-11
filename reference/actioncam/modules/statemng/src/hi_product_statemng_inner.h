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
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "hi_type.h"
#include "hi_hfsm.h"
#include "hi_eventhub.h"
#include "hi_mapi_comm_define.h"
#include "hi_recordmng.h"
#include "hi_storagemng.h"
#include "hi_filemng_comm.h"
#include "hi_product_param.h"
#include "hi_filemng_dcf.h"
#include "hi_upgrade.h"

#include "hi_appcomm_log.h"
#include "hi_appcomm_util.h"
#include "hi_timestamp.h"
#include "hi_appcomm_msg_client.h"
#include "hi_hal_common.h"
#include "hi_usb.h"
#include "hi_product_usbctrl.h"
#include "hi_hal_screen.h"
#include "hi_product_media.h"
#include "hi_liveserver.h"
#include "hi_product_scene_msg_client.h"

#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_aenc.h"

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
#define PDT_STATEMNG_STATE_REC_LOOP              "Rec_Loop"
#define PDT_STATEMNG_STATE_REC_LAPSE             "Rec_Lapse"
#define PDT_STATEMNG_STATE_REC_SLOW              "Rec_Slow"
#define PDT_STATEMNG_STATE_REC_SNAP              "Rec_Snap"
#define PDT_STATEMNG_STATE_PHOTO_SINGLE          "Photo_Single"
#define PDT_STATEMNG_STATE_PHOTO_DELAY           "Photo_Delay"
#define PDT_STATEMNG_STATE_MUTIPHOTO_BURST       "MutiPhoto_Burst"
#define PDT_STATEMNG_STATE_MUTIPHOTO_LAPSE       "MutiPhoto_Lapse"
#define PDT_STATEMNG_STATE_PLAYBACK              "PlayBack"
#define PDT_STATEMNG_STATE_UVC                   "UVC"
#define PDT_STATEMNG_STATE_USB_STORAGE           "UsbStorage"
#define PDT_STATEMNG_STATE_SUSPEND               "Suspend"
#define PDT_STATEMNG_STATE_HDMI_PREVIEW          "HDMIPreview"
#define PDT_STATEMNG_STATE_HDMI_PLAYBACK         "HDMIPlayback"
#define PDT_STATEMNG_STATE_UPGRADE               "Upgrade"

/** recmng function timeout */
#define PDT_STATEMNG_RECMNG_OPERATION_TIMEOUT_MS   (3000)

/** recmng play fps(for lapse) */
#define PDT_STATEMNG_RECMNG_LAPSE_PLAY_FPS         (30)

/** no timeout */
#define PDT_STATEMNG_RECMNG_NO_TIMEOUT             (-1)

/** venc handle */
#define PDT_STATEMNG_MAIN_STREAM_VENC_HDL          (0)
#define PDT_STATEMNG_SUB_STREAM_VENC_HDL           (1)
#define PDT_STATEMNG_PHOTO_VENC_HDL                (2)
#define PDT_STATEMNG_THM_VENC_HANDLE               (3)

/** stream name */
#define PDT_STATEMNG_MAIN_STREAM_NAME              ("11")
#define PDT_STATEMNG_SUB_STREAM_NAME               ("12")

/** liveserver max connect number */
#define PDT_STATEMNG_LIVESVR_MAX_CONNECT_CNT       (2)

/** default spot metry width and height percent data, from UI */
#define PDT_STATEMNG_SPOT_METERING_WIDTH_PERCENT   (15)
#define PDT_STATEMNG_SPOT_METERING_HEIGHT_PERCENT  (20)

/** array size */
#define PDT_STATEMNG_ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

#ifdef CONFIG_RAWCAP_ON
#ifdef CFG_POST_PROCESS
#define PDT_STATEMNG_RAWCAP_MAX_DEPTH              (4)
#else
#define PDT_STATEMNG_RAWCAP_MAX_DEPTH              (1)
#endif

#define PDT_STATEMNG_RAWCAP_REC_DEPTH              (1)
#define PDT_STATEMNG_RAWCAP_INTERVAL_TIME          (60000)
#endif

/** state machine max queue size */
#define PDT_STATEMNG_MAX_MSG_QUEUE_SIZE            (64)

#define PDT_STATEMNG_INVALID_HANDLE                (-1)

/** NULL pointer check */
#define PDT_STATEMNG_CHECK_NULL_PTR(ptr,string)\
do{\
    if(NULL == ptr)\
     {\
        MLOGE(RED"%s(null)\n"NONE,string);\
        return HI_PDT_STATEMNG_ENULLPTR;\
     }\
  }while(0)

/** message proc function parameter check */
#define PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(ContextMutex,argv,pStateID,Message,InProgress)\
do{\
    if(NULL == argv || NULL == pStateID)\
    {\
        MLOGE(RED"param err\n"NONE);\
        MUTEX_LOCK(ContextMutex);\
        PDT_STATEMNG_PublishResultEvent(Message, HI_FALSE);\
        InProgress = HI_FALSE;\
        MUTEX_UNLOCK(ContextMutex);\
        return HI_PROCESS_MSG_RESULTE_OK;\
    }\
  }while(0)

/** workmode check */
#define PDT_STATEMNG_CHECK_MSG_WORKMODE(ContextMutex,CurrentWorkMode,MessageWorkMode,Message,InProgress)\
do{\
    if(CurrentWorkMode != MessageWorkMode)\
    {\
        MLOGE(RED"workmode err\n"NONE);\
        MUTEX_LOCK(ContextMutex);\
        PDT_STATEMNG_PublishResultEvent(Message, HI_FALSE);\
        InProgress = HI_FALSE;\
        MUTEX_UNLOCK(ContextMutex);\
        return HI_PROCESS_MSG_RESULTE_OK;\
    }\
  }while(0)

/** function ret value check */
#define PDT_STATEMNG_CHECK_RET(RetValue,ErrString)\
do{\
    if(HI_SUCCESS != RetValue)\
    {\
        MLOGE(RED"%s failed(0x%08X)\n"NONE, ErrString, RetValue);\
        return HI_PDT_STATEMNG_EINTER;\
    }\
    else\
    {\
        HI_TIME_STAMP;\
    }\
  }while(0)

/** update statemng status */
#define PDT_STATEMNG_UPDATESTATUS(ContextMutex,ContextbInProgress,pstMsg,bResult,bInProgress)\
do{\
    MUTEX_LOCK(ContextMutex);\
    PDT_STATEMNG_PublishResultEvent(pstMsg, bResult);\
    ContextbInProgress = bInProgress;\
    MUTEX_UNLOCK(ContextMutex);\
  }while(0)

/** state attribution */
typedef struct tagPDT_STATEMNG_STATE_ATTR_S
{
    HI_STATE_S stState;
}PDT_STATEMNG_STATE_ATTR_S;

/** recmng context */
typedef struct tagPDT_STATEMNG_RECMNG_CONTEXT_S
{
    HI_PDT_MEDIAMODE_E enEnterMediaMode;
    HI_U8 u8FileMngObjCfgIdx;
    HI_BOOL bFileMngNewGrp;
    HI_HANDLE RecMngTaskHdl;
    HI_RECMNG_ATTR_S stRecMngAttr;
}PDT_STATEMNG_RECMNG_CONTEXT_S;

/** photomng context */
typedef struct tagPDT_STATEMNG_PHOTOMNG_CONTEXT_S
{
    HI_PDT_MEDIAMODE_E enEnterMediaMode;
    HI_U8 u8FileMngObjCfgIdx;
    HI_U8 u8JpgFileTypeIdx;
    HI_U8 u8DngFileTypeIdx;
    HI_U8 u8JpgDngFileTypeIdx;
    HI_BOOL bFileMngNewGrp;
    HI_HANDLE PhotoMngTaskHdl;
    HI_PHOTOMNG_TASK_CFG_S stPhotoMngTaskConfig;
    HI_PHOTOMNG_PHOTO_ATTR_S stPhotoAttr;
}PDT_STATEMNG_PHOTOMNG_CONTEXT_S;

/** statemng context */
typedef struct tagPDT_STATEMNG_CONTEXT_S
{
    HI_BOOL bInited;
    pthread_mutex_t Mutex;/**<Mutex for bInProgress*/
    HI_MW_PTR pvHfsmHdl;
    PDT_STATEMNG_STATE_ATTR_S stBase;
    HI_BOOL bPoweronAction;
    HI_PDT_WORKMODE_E enCurrentWorkMode;
    HI_BOOL bInProgress;
    HI_BOOL bRunning;
    HI_PDT_STATEMNG_CONFIG_S stStatemngCfg;

    HI_PDT_MEDIA_CFG_S stMediaCfg;
    HI_PDT_SCENE_MODE_S stSceneModeCfg;
    PDT_STATEMNG_RECMNG_CONTEXT_S stRecMngCtx;
    PDT_STATEMNG_PHOTOMNG_CONTEXT_S stPhotoMngCtx;
    HI_PDT_VIDEO_PROTUNE_ATTR_S stVideoProTune;
    HI_PDT_PHOTO_PROTUNE_ATTR_S stPhotoProTune;

    HI_PDT_PHOTO_SCENE_E        enScene;/**<Scene type for single and delay */
    HI_PDT_PHOTO_PROCALG_S      stProcAlg;/**<ProcAlg for single and delay */
    HI_PDT_SCENE_METRY_SPOT_PARAM_S stSpotMetryParam;/**<Spot metry */
    HI_STORAGEMNG_CFG_S stStorageMngCfg;

    HI_BOOL bVOShutdown;
    HI_S32 s32RecPair;
    HI_EXIT_MODE_E enExitMode;
#ifdef CONFIG_RAWCAP_ON
    HI_BOOL bRawCapOn;
    HI_HANDLE RawCapTaskHdl;
    HI_S32 u32RawCapFrameCnt;
    HI_BOOL bRawCapTaskCreate;
    HI_RAWCAP_CFG_S stRawCapCfg;
    HI_RAWCAP_TRIGGER_MODE_S stTriggerMode;
    HI_CHAR szPrefix[HI_APPCOMM_MAX_FILENAME_LEN];
#endif

    HI_BOOL bSuspend;
    HI_BOOL bPreRecNoSD;
    HI_BOOL bForceReBuid;
}PDT_STATEMNG_CONTEXT;

/**
 * @brief        get the pipehdl of video type
 * @param[out]   pVcapPipeHdl
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_GetVideoPipeHdl(HI_HANDLE* pVcapPipeHdl);

/**
 * @brief        get the pipehdl of video type
 * @param[out]   pVcapPipeHdl
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_GetSnapPipeHdl(HI_HANDLE* pVcapPipeHdl);

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
HI_S32 PDT_STATEMNG_WorkMode2State(HI_PDT_WORKMODE_E enPoweronWorkmode,
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
                                   HI_PDT_WORKMODE_E *penPoweronWorkmode);

/**
 * @brief        publish message process result event
 * @param[in]    pstMsg
 * @param[in]    bResult
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_PublishResultEvent(HI_MESSAGE_S *pstMsg, HI_BOOL bResult);

/**
 * @brief        publish event
 * @param[in]    EventID
 * @param[in]    arg1
 * @param[in]    arg2
 * @param[in]    bResult
 * @param[in]    u64CreateTime
 * @param[in]    szPayload
 * @param[in]    s32PayloadLen
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_PublishEvent(HI_EVENT_ID EventID, HI_S32 arg1, HI_S32 arg2, HI_BOOL bResult, HI_U64 u64CreateTime, HI_CHAR *szPayload, HI_S32 s32PayloadLen);

/**
 * @brief        generate Media config
 * @param[out]   pstOutMediaModeCfg
 * @param[in]    pstInMediaCfg
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_GenerateMediaCfg(HI_PDT_MEDIA_CFG_S *pstInMediaCfg,
                                     HI_PDT_MEDIA_CFG_S *pstOutMediaModeCfg);

/**
 * @brief        reset Media
 * @param[in]    pstMediaCfg
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_ResetMedia(const HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_PDT_SCENE_MODE_S* pstSceneCfg);

/**
 * @brief        upgrade search new Package
 * @param[in]    None
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_UpgradeSrchNewPkt(HI_VOID);

/**
 * @brief        filemng proccesses by mounted
 * @param[in]    None
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_FileMngProc(HI_VOID);

/**
 * @brief        Processes that do not upgrade
 * @param[in]    None
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_DoNotUpgrade(HI_MESSAGE_S *pstMsg);

/**
 * @brief        delete file callback function, for loop record
 * @param[in]    pszFilename
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_DelFile(const HI_CHAR *pszFilename);

/**
 * @brief        generate RecMng attribute
 * @param[out]   pstRecMngAttr
 * @param[in]    enEnterWorkMode
 * @param[in]    pstEnterWorkModeCfg
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_GenerateRecMngAttr(HI_PDT_WORKMODE_E enEnterWorkMode,
                                       const HI_PDT_WORKMODE_CFG_S *pstEnterWorkModeCfg,
                                       HI_RECMNG_ATTR_S *pstRecMngAttr);

/**
 * @brief        generate PhotoMng attribute
 * @param[out]   pstPhotoAttr
 * @param[in]    enEnterWorkMode
 * @param[in]    pstEnterWorkModeCfg
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_GeneratePhotoAttr(HI_PDT_WORKMODE_E enEnterWorkMode,
                                      const HI_PDT_WORKMODE_CFG_S *pstEnterWorkModeCfg,
                                      HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr);

/**
 * @brief        generate PhotoMng task config
 * @param[out]   pstPhotoMngTaskConfig
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_VOID PDT_STATEMNG_GeneratePhotoMngTaskCfg(HI_PHOTOMNG_TASK_CFG_S* pstPhotoMngTaskConfig);

/**
 * @brief        check workmode related setting item value
 * @param[in]    pstMsg
 * @param[in]    s32SettingItemValue
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_CheckWorkModeSettingItemValue(HI_MESSAGE_S * pstMsg,
                                                  HI_S32 s32SettingItemValue);

/**
 * @brief        save poweron workmode
 * @return       None
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SavePoweronWorkMode(HI_PDT_WORKMODE_E enSettingWorkmode);

/**
 * @brief        set audio
 * @param[in]    pstMsg
 * @param[in]    pstMediaCfg
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SetAudio(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
 * @brief        set flip
 * @param[in]    pstMsg
 * @param[in]    pstMediaCfg
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SetFlip(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
 * @brief        set ODS
 * @param[in]    pstMsg
 * @param[in]    pstMediaCfg
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SetOSD(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
 * @brief        set DIS
 * @param[in]    pstMsg
 * @param[in]    pstMediaCfg
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SetDIS(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
 * @brief        set LDC
 * @param[in]    pstMsg
 * @param[in]    pstMediaCfg
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SetLDC(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg);

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
 * @brief        init Photo states, add all Photo states to HFSM, init PhotoMng module
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_PhotoStatesInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit Photo states
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_PhotoStatesDeinit(HI_VOID);

/**
 * @brief        init RecSnap state
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_RecSnapStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit RecSnap State
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_RecSnapStateDeinit(HI_VOID);

/**
 * @brief        init HDMIPreview state
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_HDMIPreviewStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit HDMIPreview State
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_HDMIPreviewStateDeinit(HI_VOID);

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
 * @brief        init HDMIPlayback state
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_HDMIPlaybackStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit HDMIPlayback State
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_HDMIPlaybackStateDeinit(HI_VOID);

/**
 * @brief        init Upgrade state
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_UpgradeStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit Upgrade State
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_UpgradeStateDeinit(HI_VOID);

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
 * @brief        deinit USBStorage State
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_USBStorageStateDeinit(HI_VOID);

/**
 * @brief        init Suspend state
 * @param[in]    pstBase
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SuspendStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase);

/**
 * @brief        deinit Suspend State
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SuspendStateDeinit(HI_VOID);

/**
 * @brief        set ExpTime attibute
 * @param[in]    pstMsg
 * @param[in]    pstMediaCfg
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SetExpTime(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
 * @brief        set EV attibute
 * @param[in]    pstMsg
 * @param[in]    pstMediaCfg
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SetEV(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
 * @brief        set ISO attibute
 * @param[in]    pstMsg
 * @param[in]    pstMediaCfg
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SetISO(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
 * @brief        set WB attibute
 * @param[in]    pstMsg
 * @param[in]    pstMediaCfg
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SetWB(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
 * @brief        set Metry attibute
 * @param[in]    pstMsg
 * @param[in]    pstMediaCfg
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_SetMetry(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg);

#ifdef CONFIG_RAWCAP_ON
/**
 * @brief        get RawCap task source
 * @param[in]    enWorkmode
 * @param[out]   pstDataSource
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_VOID PDT_STATEMNG_GetRawCapTaskSrc(HI_PDT_WORKMODE_E enWorkmode,
    HI_RAWCAP_DATA_SOURCE_S *pstDataSource);
/**
 * @brief        get RawCap task file name
 * @param[in]    pvPrivateData
 * @param[out]   pstFilename
 * @return       0 success, non-zero error code
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2017/12/20
 */
HI_S32 PDT_STATEMNG_GetRawCapFileNames(HI_RAWCAP_FILENAME_S *pstFilename, HI_VOID* pvPrivateData);
#endif

/**
 * @brief        get camera exif info
 * @param[out]   pstExifInfo: exif info struct
 * @return 0     success,non-zero error code.
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/6/1
 */
HI_VOID PDT_STATEMNG_GetCamExifInfo(HI_MAPI_SNAP_EXIF_INFO_S* pstExifInfo);

/**
 * @brief        set system configure for different workmode when sd mounted
 * @return 0     success,non-zero error code.
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/6/1
 */
HI_VOID PDT_STATEMNG_SetSysConfig(HI_VOID);

/**
 * @brief        stop powon action
 * @return 0     success,non-zero error code.
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/6/1
 */
HI_S32 PDT_STATEMNG_StopEncoder(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_PDT_WORKMODE_E enWorkMode);

/**
 * @brief        init filemng
 * @return 0     success,non-zero error code.
 * @exception    None
 * @author       HiMobileCam Reference Develop Team
 * @date         2018/6/1
 */
HI_S32 PDT_STATEMNG_InitFileMng(HI_VOID);

/** @}*/  /** <!-- ==== STATEMNG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_STATEMNG_INNER__ */
