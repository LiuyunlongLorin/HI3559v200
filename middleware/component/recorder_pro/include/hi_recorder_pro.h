/**
* Copyright (C), 2017-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file          hi_recorder_pro.h
* @brief       record module header file
* @author    HiMobileCam middleware develop team
* @date       2017.02.25
*/

#ifndef __HI_RECORDER_PRO_H__
#define __HI_RECORDER_PRO_H__

#include "hi_mw_type.h"
#include "hi_error_def.h"
#include "hi_defs.h"
#include "hi_track_source.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/*************************** Error Definition ****************************/
typedef enum hiAPP_RECORD_ERR_CODE_E {
    /* general error code */
    APP_RECORD_ERR_REC_HANDLE_INVALID = 0x40,      /**<recorder handle invalid */
    APP_RECORD_ERR_REC_NO_AVAILABLE_HANDLE = 0x41, /**<no available recorder handle */
    APP_RECORD_ERR_MALLOC_FAIL = 0x42,             /**<malloc memory fail */

    APP_RECORD_ERR_THREAD_CREATE_FAIL = 0x43, /**<thread create failed */
    APP_RECORD_ERR_MUTEX_INIT_FAIL = 0x44,    /**<mutex init failed */
    APP_RECORD_ERR_COND_VAR_INIT_FAIL = 0x45, /**<cond init failed */
    APP_RECORD_ERR_INIT_TIMER_FAIL = 0x46,

    /* track source relate error */
    APP_RECORD_ERR_STREAM_START_FAIL = 0x60, /**<start stream fail */
    APP_RECORD_ERR_STREAM_STOP_FAIL = 0x61,  /**<stop stream fail */

    /* recorder opt related error code */
    APP_RECORD_ERR_STOPPING = 0x70,        /**<recorder repeated start */
    APP_RECORD_ERR_NOT_ALLOWED = 0x71,     /**<recorder permission not allowed */
    APP_RECORD_ERR_DO_MANUAL_SPLIT = 0x72, /**<recorder permission not allowed */

    /* MUX & file related error code */
    APP_RECORD_ERR_ALLOC_FILENAME_FAIL = 0x80, /**<alloc filename fail */

    /* buffer releated error code */
    APP_RECORD_ERR_BUFFER_CREATE_FAIL = 0x90,
    APP_RECORD_ERR_BUFFER_DESTROY_FAIL = 0x91,
    APP_RECORD_ERR_BUFFER_SET_MODE_FAIL = 0x92,
    APP_RECORD_ERR_BUFFER_WRITE_FRAME_FAIL = 0x93,
    APP_RECORD_ERR_BUFFER_READ_FRAME_FAIL = 0x94,

    APP_RECORD_BUTT = 0xFF
} HI_APP_RECORD_ERR_CODE_E;

/* general error code */
#define HI_ERR_RECORD_NULL_PTR      HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_ERR_NULL_PTR)
#define HI_ERR_RECORD_ILLEGAL_PARAM HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_ERR_ILLEGAL_PARAM)

#define HI_ERR_RECORD_HANDLE_INVALID      HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_REC_HANDLE_INVALID)
#define HI_ERR_RECORD_NO_AVAILABLE_HANDLE HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_REC_NO_AVAILABLE_HANDLE)
#define HI_ERR_RECORD_MALLOC_FAIL         HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_MALLOC_FAIL)
#define HI_ERR_RECORD_THREAD_CREATE_FAIL  HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_THREAD_CREATE_FAIL)
#define HI_ERR_RECORD_MUTEX_INIT_FAIL     HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_MUTEX_INIT_FAIL)
#define HI_ERR_RECORD_COND_VAR_INIT_FAIL  HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_COND_VAR_INIT_FAIL)
#define HI_ERR_RECORD_INIT_TIMER_FAIL     HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_INIT_TIMER_FAIL)

/* track source relate error */
#define HI_ERR_RECORD_STREAM_START_FAIL HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_STREAM_START_FAIL)
#define HI_ERR_RECORD_STREAM_STOP_FAIL  HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_STREAM_STOP_FAIL)

/* filemng & Storagemng related error code */
#define HI_ERR_RECORD_ALLOC_FILENAME_FAIL HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_ALLOC_FILENAME_FAIL)

#define HI_ERR_RECORD_DO_STOPPING     HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_STOPPING)
#define HI_ERR_RECORD_OPT_NOT_ALLOWED HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_NOT_ALLOWED)
#define HI_ERR_RECORD_DO_MANUAL_SPLIT HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_DO_MANUAL_SPLIT)

/* buff relate error code */
#define HI_ERR_RECORD_BUFFER_CREATE_FAIL      HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_BUFFER_CREATE_FAIL)
#define HI_ERR_RECORD_BUFFER_DESTROY_FAIL     HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_BUFFER_DESTROY_FAIL)
#define HI_ERR_RECORD_BUFFER_SET_MODE_FAIL    HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_BUFFER_SET_MODE_FAIL)
#define HI_ERR_RECORD_BUFFER_WRITE_FRAME_FAIL HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_BUFFER_WRITE_FRAME_FAIL)
#define HI_ERR_RECORD_BUFFER_READ_FRAME_FAIL  HI_APP_DEF_ERR(HI_APPID_REC, APP_ERR_LEVEL_ERROR, APP_RECORD_ERR_BUFFER_READ_FRAME_FAIL)

/*************************** Macro Definition ****************************/
/** \addtogroup     REC */
/** @{ */ /** <!-- [REC] */

#define HI_REC_STREAM_MAX_CNT (4)

#define HI_REC_TRACK_MAX_CNT (3)

#define HI_REC_FILE_NAME_LEN (256)

#define HI_REC_FRAME_MAX_SLICE (8)

/***splite define**/
/**record split type enum */
typedef enum hiREC_SPLIT_TYPE_E {
    HI_REC_SPLIT_TYPE_NONE = 0, /**<means split is disabled */
    HI_REC_SPLIT_TYPE_TIME,     /**<record split when time reaches */
    HI_REC_SPLIT_TYPE_BUTT
} HI_REC_SPLIT_TYPE_E;

/**record split attribute param */
typedef struct hiREC_SPLIT_ATTR_S {
    HI_REC_SPLIT_TYPE_E enSplitType; /**<split type */
    HI_U32 u32SplitTimeLenSec;       /**<split time, unit in second(s) */
} HI_REC_SPLIT_ATTR_S;

/**record manual split type enum */
typedef enum hiREC_MANUAL_SPLIT_TYPE_E {
    HI_REC_POST_MANUAL_SPLIT = 0, /**<post maunal split type */
    HI_REC_PRE_MANUAL_SPLIT,      /**<pre manual split type */
    HI_REC_NORMAL_MANUAL_SPLIT,   /**<normal manual split type */
    HI_REC_MANUAL_SPLIT_BUTT
} HI_REC_MANUAL_SPLIT_TYPE_E;

/**post manual split attribute */
typedef struct hiREC_POST_MANUAL_SPLIT_ATTR_S {
    HI_U32 u32AfterSec; /**<manual split file will end after u32AfterSec */
} HI_REC_POST_MANUAL_SPLIT_ATTR_S;

/**pre manual split attribute */
typedef struct hiREC_PRE_MANUAL_SPLIT_ATTR_S {
    HI_U32 u32DurationSec; /**<file duration of manual split file */
} HI_REC_PRE_MANUAL_SPLIT_ATTR_S;

/**normal manual split attribute */
typedef struct hiREC_NORMAL_MANUAL_SPLIT_ATTR_S {
    HI_U32 u32Rsv; /**<reserve */
} HI_REC_NORMAL_MANUAL_SPLIT_ATTR_S;

/**pre manual split attribute */
typedef struct hiREC_MANUAL_SPLIT_ATTR_S {
    HI_REC_MANUAL_SPLIT_TYPE_E enManualType; /**<maual split type */
    union {
        HI_REC_POST_MANUAL_SPLIT_ATTR_S stPostSplitAttr;     /**<post manual split attr */
        HI_REC_PRE_MANUAL_SPLIT_ATTR_S stPreSplitAttr;       /**<pre manual split attr */
        HI_REC_NORMAL_MANUAL_SPLIT_ATTR_S stNormalSplitAttr; /**<normal manual split attr */
    };
} HI_REC_MANUAL_SPLIT_ATTR_S;

/**record type enum */
typedef enum hiREC_TYPE_E {
    HI_REC_TYPE_NORMAL = 0, /**<normal record */
    HI_REC_TYPE_LAPSE,      /**<time lapse record, record a frame by an fixed time interval */
    HI_REC_TYPE_BUTT
} HI_REC_TYPE_E;

/**normal record attribute param */
typedef struct hiREC_NORMAL_ATTR_S {
    HI_U32 u32Rsv; /**<reserve */
} HI_REC_NORMAL_ATTR_S;

/**lapse record attribute param */
typedef struct hiREC_LAPSE_ATTR_S {
    HI_U32 u32IntervalMs; /**<lapse record time interval, unit in millisecord(ms) */
} HI_REC_LAPSE_ATTR_S;

/**record event enum */
typedef enum hiREC_EVENT_E {
    HI_REC_EVENT_START = 0, /**<record is started */
    HI_REC_EVENT_STOP,      /**<record is stopped */

    HI_REC_EVENT_NEW_FILE_BEGIN,            /**<new file start */
    HI_REC_EVENT_NEW_FILE_END,              /**<new file finish */
    HI_REC_EVENT_NEW_MANUAL_SPLIT_FILE_END, /**<new manual splite file finish */

    HI_REC_EVENT_ERR_CREATE_FILE_FAIL,        /**<create file failed */
    HI_REC_EVENT_ERR_WRITE_FILE_FAIL,         /**<write file failed */
    HI_REC_EVENT_ERR_CLOSE_FILE_FAIL,         /**<close file failed */
    HI_REC_EVENT_ERR_READ_DATA_ERROR,         /**<rec internal operation fail, must stop rec */
    HI_REC_EVENT_ERR_INTERNAL_OPERATION_FAIL, /**<rec internal operation fail, must stop rec */
    HI_REC_EVENT_BUTT
} HI_REC_EVENT_E;

/**<state event info */
typedef struct hiREC_STATE_EVENT_INFO_S {
    HI_S32 s32Rsv;
} HI_REC_STATE_EVENT_INFO_S;

/**<file event info */
typedef struct hiREC_FILE_EVENT_INFO_S {
    HI_CHAR aszFileName[HI_REC_FILE_NAME_LEN];
    HI_MW_PTR pMuxer;
} HI_REC_FILE_EVENT_INFO_S;

/**<error event info */
typedef struct hiREC_ERROR_EVENT_INFO_S {
    HI_CHAR aszFileName[HI_REC_FILE_NAME_LEN];
    HI_S32 s32ErrorCode;
} HI_REC_ERROR_EVENT_INFO_S;

/**event information */
typedef struct hiREC_EVENT_INFO_S {
    HI_REC_EVENT_E enEventCode;
    union {
        HI_REC_STATE_EVENT_INFO_S stStateInfo; /**<state event info */
        HI_REC_FILE_EVENT_INFO_S stFileInfo;   /**<file event info */
        HI_REC_ERROR_EVENT_INFO_S stErrorInfo; /**<error event info */
    } unEventInfo;
} HI_REC_EVENT_INFO_S;

/**rec frame data struct */
typedef struct hiREC_FRAME_DATA_S {
    HI_U64 u64TimeStamp;
    HI_U8 *apu8SliceAddr[HI_REC_FRAME_MAX_SLICE];
    HI_U32 au32SliceLen[HI_REC_FRAME_MAX_SLICE];
    HI_BOOL bKeyFrameFlag;
    HI_U32 u32SliceCnt;
} HI_REC_FRAME_DATA_S;

/** record event callback function */
typedef HI_S32 (*HI_REC_EVENT_CALLBACK_FN)(HI_MW_PTR pRecorder, const HI_REC_EVENT_INFO_S *pstEventInfo);

/** record create file function */
typedef HI_S32 (*HI_REC_REQUEST_FILE_NAMES_FN)(HI_MW_PTR pRecorder, HI_U32 u32FileCnt,
               HI_CHAR (*paszFilename)[HI_REC_FILE_NAME_LEN]);

/** create muxer callback function */
typedef HI_S32 (*HI_REC_CREATE_MUXER_FN)(HI_CHAR *pszFileName, HI_VOID *pMuxerCfg, HI_MW_PTR *ppMuxer);

/** destroy muxer callback function */
typedef HI_S32 (*HI_REC_DESTROY_MUXER_FN)(HI_MW_PTR pMuxer);

/** create track callback function */
typedef HI_S32 (*HI_REC_CREATE_TRACK_FN)(HI_MW_PTR pMuxer, HI_Track_Source_Handle pstTrackSrcHandle,
               HI_MW_PTR *pTrack);

/** write frame to track callback function */
typedef HI_S32 (*HI_REC_WRITE_FRAME_FN)(HI_MW_PTR pMuxer, HI_MW_PTR hTrackHandle,
               HI_REC_FRAME_DATA_S *pstFrameData);

/** rec stream buffer alloc function */
typedef HI_S32 (*HI_REC_BUF_ALLOC_FN)(HI_VOID *pBufArg, HI_U32 u32BufSize, HI_VOID **ppVmAddr);

/**rec stream buffer free function */
typedef HI_S32 (*HI_REC_BUF_FREE_FN)(HI_VOID *pBufArg, HI_U32 u32BufSize, HI_VOID *pVmAddr);

typedef struct hiREC_MUXER_OPERATE_FN_S {
    HI_REC_CREATE_MUXER_FN pfnCreateMuxer;   /**<create file muxer  */
    HI_REC_DESTROY_MUXER_FN pfnDestroyMuxer; /**<destroy file muxer */
    HI_REC_CREATE_TRACK_FN pfnCreateTrack;   /**< create track */
    HI_REC_WRITE_FRAME_FN pfnWriteFrame;     /**<write frame */
    HI_VOID *pMuxerCfg;                      /**<private muxer configure */
} HI_REC_MUXER_OPERATE_FN_S;

/**rec callback set */
typedef struct hiREC_CALLBACK_S {
    HI_REC_REQUEST_FILE_NAMES_FN pfnRequestFileNames; /**<callback  request file names */
} HI_REC_CALLBACK_S;

/**funcs of rec buffer  abstract */
typedef struct hiREC_BUF_ABSTRACTFUNC_S {
    HI_REC_BUF_ALLOC_FN pfnBufAlloc; /**< get the memory addr of the record buffer */
    HI_REC_BUF_FREE_FN pfnBufFree;   /**< release memory space of the record buffer */
    HI_VOID *pBufArg;                /**< buffer private argument pass by buffer alloc and free */
} HI_REC_BUF_ABSTRACTFUNC_S;

/**record stream attribute */
typedef struct hiREC_STREAM_ATTR_S {
    HI_U32 u32TrackCnt;                                            /**<track cnt */
    HI_Track_Source_Handle aHTrackSrcHandle[HI_REC_TRACK_MAX_CNT]; /**<array of track source cnt */
    HI_REC_MUXER_OPERATE_FN_S stMuxerOps;                          /**<muxer operates  */
    HI_REC_BUF_ABSTRACTFUNC_S stBufOps;                            /**<rec buffer operates */
} HI_REC_STREAM_ATTR_S;

/**record attribute param */
typedef struct hiREC_ATTR_S {
    HI_REC_TYPE_E enRecType; /**<record type */
    union {
        HI_REC_NORMAL_ATTR_S stNormalRecAttr; /**<normal record attribute */
        HI_REC_LAPSE_ATTR_S stLapseRecAttr;   /**<lapse record attribute */
    } unRecAttr;

    HI_REC_SPLIT_ATTR_S stSplitAttr; /**< record split attribute */

    HI_U32 u32StreamCnt;                                       /**< stream cnt */
    HI_REC_STREAM_ATTR_S astStreamAttr[HI_REC_STREAM_MAX_CNT]; /**< array of stream attr */
    HI_REC_CALLBACK_S stRecCallbacks;                          /**< rec callbak */
    HI_U32 u32BufferTimeMSec;                                  /**< data buffer size in Msec */
    HI_U32 u32PreRecTimeSec;                                   /**< pre record time */
} HI_REC_ATTR_S;

/*************************** Interface Definition ****************************/

/**
 * @brief create a new recorder
 * @param[in]pstRecAttr : the attribute of recorder
 * @param[out]ppRecorder : pointer of recorder
 * @return 0 success
 * @return others failure
 */
HI_S32 HI_REC_Create(HI_REC_ATTR_S *pstRecAttr, HI_MW_PTR *ppRecorder);

/**
 * @brief destory a recorder.
 * @param[in]pRecorder : pointer of recorder
 * @return 0 success
 * @return others failure
 */
HI_S32 HI_REC_Destroy(HI_MW_PTR pRecorder);

/**
 * @brief start recorder
 * @param[in]pRecorder, pointer of recorder
 * @return 0 success
 * @return -1 failure
 */
HI_S32 HI_REC_Start(HI_MW_PTR pRecorder);

/**
 * @brief stop recorder
 * @param[in]pRecorder : pointer of recorder
 * @param[in]bQuickMode : quick stop mode flag.
 * @return 0 success
 * @return others failure
 */
HI_S32 HI_REC_Stop(HI_MW_PTR pRecorder, HI_BOOL bQuickMode);

/**
 * @brief manual splite file.
 * @param[in]pRecorder : pointer of recorder
 * @param[in]pstSplitAttr : manual split attr.
 * @return 0 success
 * @return others failure
 */
HI_S32 HI_REC_ManualSplit(HI_MW_PTR pRecorder, HI_REC_MANUAL_SPLIT_ATTR_S *pstSplitAttr);

/**
 * @brief write frame data to rec.
 * @param[in]pRecorder : pointer of recorder
 * @param[in]pTrackSrcHandle : handle of track source
 * @param[in]pstFrameData : pointer to frame data.
 * @return 0 success
 * @return others failure
 */
HI_S32 HI_REC_WriteData(HI_MW_PTR pRecorder, HI_Track_Source_Handle pTrackSrcHandle,
                        HI_REC_FRAME_DATA_S *pstFrameData);

/**
 * @brief register recorder envent callback
 * @param[in]pRecorder : pointer of recorder
 * @param[in]pfnEventCallback : callback function
 * @return 0 success
 * @return others failure
 */
HI_S32 HI_REC_RegisterEventCallback(HI_MW_PTR pRecorder, HI_REC_EVENT_CALLBACK_FN pfnEventCallback);

/** @} */ /** <!-- ==== REC End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

