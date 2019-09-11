/**
* @file    hi_recordmng.h
* @brief   record manager module struct and interface declaration
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
* @version   1.0

*/

#ifndef __HI_RECORDMNG_H__
#define __HI_RECORDMNG_H__

/** head file of middleware module */
#include "hi_recorder_pro.h"

/** head file of reference module */
#include "hi_appcomm.h"
#include "hi_media_comm_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /** End of #ifdef __cplusplus */

/**\addtogroup     RECMNG */
/**@{ */  /**<!-- [RECMNG] */

/** maximum record task count */
#define HI_RECMNG_MAX_TASK_CNT (2)

/** minimum file in specified record task */
#define HI_RECMNG_TASK_MIN_FILE_CNT (1)
/** maximum file in specified record task */
#define HI_RECMNG_TASK_MAX_FILE_CNT (4)

/** minimum video stream in specified record file */
#define HI_RECMNG_FILE_MIN_VIDEO_TRACK_CNT (1)
/** maximum video stream in specified record file */
#define HI_RECMNG_FILE_MAX_VIDEO_TRACK_CNT (2)

/** maximum audio stream in specified record file */
#define HI_RECMNG_FILE_MAX_AUDIO_TRACK_CNT (1)

/** maximum private data stream in specified record file */
#define HI_RECMNG_FILE_MAX_PRIV_DATA_TRACK_CNT (1)

/** miniimum loop number */
#define HI_RECMNG_MIN_LOOP_NUM (1)
/** maximum loop number */
#define HI_RECMNG_MAX_LOOP_NUM (10)

/** maximum length of private data*/
#define HI_RECMNG_MAX_PRIV_DATA_LEN (128)

/** error code define */
#define HI_RECMNG_EINVAL              HI_APPCOMM_ERR_ID(HI_APP_MOD_RECMNG, HI_EINVAL)                  /**<illegal parameter */
#define HI_RECMNG_ENOINIT             HI_APPCOMM_ERR_ID(HI_APP_MOD_RECMNG, HI_ENOINIT)                 /**<module not inited */
#define HI_RECMNG_EINITIALIZED        HI_APPCOMM_ERR_ID(HI_APP_MOD_RECMNG, HI_EINITIALIZED)            /**<already initialized */
#define HI_RECMNG_ETASK_NOT_CREATED   HI_APPCOMM_ERR_ID(HI_APP_MOD_RECMNG, HI_ERRNO_CUSTOM_BOTTOM + 1) /**<task not created */
#define HI_RECMNG_ETASK_NOT_STARTED   HI_APPCOMM_ERR_ID(HI_APP_MOD_RECMNG, HI_ERRNO_CUSTOM_BOTTOM + 2) /**<task not started */
#define HI_RECMNG_ESPLIT_NOT_FINISHED HI_APPCOMM_ERR_ID(HI_APP_MOD_RECMNG, HI_ERRNO_CUSTOM_BOTTOM + 3) /**<split not finished */
#define HI_RECMNG_EBUSY               HI_APPCOMM_ERR_ID(HI_APP_MOD_RECMNG, HI_EBUSY)                   /**<task is busy */
#define HI_RECMNG_EMAXTASK            HI_APPCOMM_ERR_ID(HI_APP_MOD_RECMNG, HI_EFULL)                   /**<beyond maximum task */
#define HI_RECMNG_ETIMEOUT            HI_APPCOMM_ERR_ID(HI_APP_MOD_RECMNG, HI_ERRNO_CUSTOM_BOTTOM + 4) /**<timeout */
#define HI_RECMNG_EINTER              HI_APPCOMM_ERR_ID(HI_APP_MOD_RECMNG, HI_EINTER)                  /**<Internal error */

typedef struct hiRECMNG_PRIV_DATA_S
{
    HI_U64 u64TimeStamp;  /**<private data timestamp */
    HI_U8* pu8DataBuffer; /**<private data buffer */
    HI_U32 u32DataLength; /**<private date len */
} HI_RECMNG_PRIV_DATA_S;

/** record muxer type enum */
typedef enum hiRECMNG_MUXER_TYPE_E
{
    HI_RECMNG_MUXER_MP4  = 0,
    HI_RECMNG_MUXER_MOV  = 1,
    HI_RECMNG_MUXER_TS   = 2,
    HI_RECMNG_MUXER_BUTT = 3
} HI_RECMNG_MUXER_TYPE_E;

typedef struct hiRECMNG_PRIV_DATA_INFO_S
{
    HI_U32 u32FrameRate; /**<frame rate per second */
    HI_U32 u32BytesPerSec;
    HI_BOOL bStrictSync;
} HI_RECMNG_PRIV_DATA_INFO_S;

/** event ID define */
typedef enum hiEVENT_RECMNG_E
{
    HI_EVENT_RECMNG_FILE_BEGIN = HI_APPCOMM_EVENT_ID(HI_APP_MOD_RECMNG, 0), /**<file started */
    HI_EVENT_RECMNG_FILE_END, /**<file stopped */
    HI_EVENT_RECMNG_MANUAL_SPLIT_FILE_END, /**<manual split file finished */
    HI_EVENT_RECMNG_ERROR, /**<create/write/close file fail, read data error, internal operation fail */
    HI_EVENT_RECMNG_MUXER_CNT_CHANGE, /**< arg1:task handle, arg2:task muxer count, if all task's muxer count is 0, arg1:-1, arg2:0 */
    HI_EVENT_RECMNG_TASK_START, /**<task started */
    HI_EVENT_RECMNG_TASK_STOP, /**<task stopped */
    HI_EVENT_RECMNG_TASK_AUTO_STOP, /**<task stopped after manual split file end */
    HI_EVENT_RECMNG_BUTT
} HI_EVENT_RECMNG_E;

/** record callback: get video information */
typedef HI_S32 (*HI_RECMNG_GET_VIDEO_INFO_CALLBACK_FN_PTR)(HI_HANDLE VencHdl, HI_MEDIA_VIDEOINFO_S *pstVideoInfo);

/** record callback: get audio information */
typedef HI_S32 (*HI_RECMNG_GET_AUDIO_INFO_CALLBACK_FN_PTR)(HI_HANDLE AencHdl, HI_MEDIA_AUDIOINFO_S *pstAudioInfo);

/** record callback: get private data config */
typedef HI_S32 (*HI_RECMNG_GET_PRIV_DATA_CFG_CALLBACK_FN_PTR)(HI_HANDLE PrivDataHdl, HI_RECMNG_PRIV_DATA_INFO_S *pstPrivDataInfo);

/** record callback: start venc */
typedef HI_S32 (*HI_RECMNG_VENC_START_CALLBACK_FN_PTR)(HI_HANDLE VencHdl, HI_S32 s32FrameCnt);

/** record callback: stop venc */
typedef HI_S32 (*HI_RECMNG_VENC_STOP_CALLBACK_FN_PTR)(HI_HANDLE VencHdl);

/** record callback: start aenc */
typedef HI_S32 (*HI_RECMNG_AENC_START_CALLBACK_FN_PTR)(HI_HANDLE AencHdl);

/** record callback: stop aenc */
typedef HI_S32 (*HI_RECMNG_AENC_STOP_CALLBACK_FN_PTR)(HI_HANDLE AencHdl);

/** record callback: private data */
typedef HI_S32 (*HI_RECMNG_PRIV_DATA_START_CALLBACK_FN_PTR)(HI_HANDLE PrivDataHdl, HI_VOID* pvPrivDataInfo);

/** record callback: private data */
typedef HI_S32 (*HI_RECMNG_PRIV_DATA_STOP_CALLBACK_FN_PTR)(HI_HANDLE PrivDataHdl, HI_VOID* pvPrivDataInfo);

/** define media operating function struct of record manager */
typedef struct hiRECMNG_MEDIA_OPERATE_S
{
    HI_RECMNG_GET_VIDEO_INFO_CALLBACK_FN_PTR pfnGetVideoInfo; /**<get video information */
    HI_RECMNG_GET_AUDIO_INFO_CALLBACK_FN_PTR pfnGetAudioInfo; /**<get audio information */
    HI_RECMNG_GET_PRIV_DATA_CFG_CALLBACK_FN_PTR pfnGetPrivDataCfg; /**<get private data config */
    HI_RECMNG_VENC_START_CALLBACK_FN_PTR pfnVencStart; /**<start venc */
    HI_RECMNG_VENC_STOP_CALLBACK_FN_PTR pfnVencStop; /**<stop venc */
    HI_RECMNG_AENC_START_CALLBACK_FN_PTR pfnAencStart; /**<start aenc */
    HI_RECMNG_AENC_STOP_CALLBACK_FN_PTR pfnAencStop; /**<stop aenc */
    HI_RECMNG_PRIV_DATA_START_CALLBACK_FN_PTR pfnPrivDataStart; /**<start private data */
    HI_RECMNG_PRIV_DATA_STOP_CALLBACK_FN_PTR pfnPrivDataStop; /**<stop private data */
} HI_RECMNG_MEDIA_OPERATE_S;


/** record buffer type enum */
typedef enum hiRECMNG_BUF_TYPE_E
{
    HI_RECMNG_BUF_TYPE_MMZ = 0,
    HI_RECMNG_BUF_TYPE_OS = 1,
    HI_RECMNG_BUF_TYPE_BUTT
} HI_RECMNG_BUF_TYPE_E;

/** mmz buffer type configure */
typedef struct hiRECMNG_MMZ_BUF_CFG_S
{
    HI_CHAR szBufName[HI_APPCOMM_COMM_STR_LEN];
} HI_RECMNG_MMZ_BUF_CFG_S;

/** os buffer type configure */
typedef struct hiRECMNG_OS_BUF_CFG_S
{
    /**<reserved */
} HI_RECMNG_OS_BUF_CFG_S;

/** buffer configure */
typedef struct hiRECMNG_BUF_CFG_S
{
    HI_RECMNG_BUF_TYPE_E enBufType;
    union tagBUF_CFG_U
    {
        HI_RECMNG_MMZ_BUF_CFG_S stMmzBufCfg; /**<mmz buffer configure */
        HI_RECMNG_OS_BUF_CFG_S stOsBufCfg; /**<os buffer configure */
    } unBufCfg;
} HI_RECMNG_BUF_CFG_S;

/** file data source */
typedef struct hiRECMNG_FILE_DATA_SOURCE_S
{
    HI_U32 u32VencCnt; /**<count of video data source handle */
    HI_U32 u32AencCnt; /**<count of audio data source handle */
    HI_U32 u32PrivDataCnt; /**<count of private data source handle */
    HI_HANDLE aVencHdl[HI_RECMNG_FILE_MAX_VIDEO_TRACK_CNT]; /**<video data source handle, HI_INVALID_HANDLE: not support */
    HI_HANDLE aAencHdl[HI_RECMNG_FILE_MAX_AUDIO_TRACK_CNT]; /**<audio data source handle, HI_INVALID_HANDLE: not support */
    HI_HANDLE aPrivDataHdl[HI_RECMNG_FILE_MAX_PRIV_DATA_TRACK_CNT]; /**<private data source handle, HI_INVALID_HANDLE: not support */
    HI_HANDLE ThmHdl; /**<thumbnail data source handle, HI_INVALID_HANDLE: not support */
} HI_RECMNG_FILE_DATA_SOURCE_S;

/** mp4 muxer configure */
typedef struct hiRECMNG_MP4MUXER_CFG_S
{
    HI_U32 u32RepairUnit; /**<repair data unit, [10M,500M], unit: bytes, 0: not support repair */
} HI_RECMNG_MP4MUXER_CFG_S;

/** mov muxer configure */
typedef struct hiRECMNG_MOVMUXER_CFG_S
{
    HI_U32 u32RepairUnit; /**<repair data unit, [10M,500M], unit: bytes, 0: not support repair */
} HI_RECMNG_MOVMUXER_CFG_S;

/** ts muxer configure */
typedef struct hiRECMNG_TSMUXER_CFG_S
{
    /**<reserved */
} HI_RECMNG_TSMUXER_CFG_S;

/** file muxer configure */
typedef struct hiRECMNG_MUXER_CFG_S
{
    HI_RECMNG_MUXER_TYPE_E enMuxerType; /**<muxer type */
    union tagMUXER_CFG_U
    {
        HI_RECMNG_MP4MUXER_CFG_S stMp4Cfg; /**<mp4 muxer configure */
        HI_RECMNG_MOVMUXER_CFG_S stMovCfg; /**<mov muxer configure */
        HI_RECMNG_TSMUXER_CFG_S stTsCfg; /**<ts muxer configure */
    } unMuxerCfg;
    HI_U32 u32PreAllocUnit; /**<pre allocate size in bytes, [0,100M], 0 for not use pre allocate function, suggest 20M, unit :byte */
    HI_U32 u32VBufSize; /**<set the vbuf size for fwrite, (0,5M], unit :byte */
    HI_BOOL bConstantFps; /**< if not true, the muxer fps is decided by the input stream framerate. if true, the muxer fps is decides by set*/
} HI_RECMNG_MUXER_CFG_S;

/** file attribute */
typedef struct hiRECMNG_FILE_ATTR_S
{
    HI_RECMNG_BUF_CFG_S astBufCfg[HI_RECMNG_FILE_MAX_VIDEO_TRACK_CNT]; /**<buffer configure */
    HI_RECMNG_FILE_DATA_SOURCE_S stDataSource; /**<file data source */
    HI_S32 s32PlayFps; /**<play framerate fps: -1 use venc fps; >0 use assigned fps */
    HI_RECMNG_MUXER_CFG_S stMuxerCfg; /**<file muxer configure */
} HI_RECMNG_FILE_ATTR_S;


/** define filename struct */
typedef struct hiRECMNG_FILENAME_S
{
    HI_U32 u32RecFileCnt; /**<count of record file */
    HI_U32 u32ThmFileCnt; /**<count of thumbnail file */
    HI_CHAR aszRecFilename[HI_RECMNG_TASK_MAX_FILE_CNT][HI_APPCOMM_MAX_FILENAME_LEN]; /**<record filename */
    HI_CHAR aszThmFilename[HI_RECMNG_TASK_MAX_FILE_CNT][HI_APPCOMM_MAX_FILENAME_LEN]; /**<thumbnail filename */
} HI_RECMNG_FILENAME_S;

/** record callback: get filenames */
typedef HI_S32 (*HI_RECMNG_GET_FILENAMES_CALLBACK_FN_PTR)(HI_RECMNG_FILENAME_S *pstFilename, HI_VOID *pPrivData);

/** get filenames callback struct of record manage */
typedef struct hiRECMNG_GET_FILENAME_S
{
    HI_RECMNG_GET_FILENAMES_CALLBACK_FN_PTR pfnGetFilenames; /**<get filenames callback */
    HI_VOID *pvPrivData; /**<private data of get filename callback */
} HI_RECMNG_GET_FILENAME_S;


/** record thumbnail storage type enum */
typedef enum hiRECMNG_THM_TYPE_E
{
    HI_RECMNG_THM_TYPE_INDEPENDENCE = 0, /**<independence file */
    HI_RECMNG_THM_TYPE_EMBEDDED = 1, /**<embedded in stream file */
    HI_RECMNG_THM_TYPE_NONE = 2, /**<none thumbnail */
    HI_RECMNG_THM_TYPE_BUTT
} HI_RECMNG_THM_TYPE_E;

/** thumbnail data source */
typedef struct hiRECMNG_THM_ATTR_S
{
    HI_U32 u32ThmCnt; /**<count of thumbnail data source handle */
    HI_HANDLE aThmHdl[HI_RECMNG_TASK_MAX_FILE_CNT]; /**<thumbnail data source handle, HI_INVALID_HANDLE: not support */
} HI_RECMNG_THM_ATTR_S;

/** record callback: delete file */
typedef HI_S32 (*HI_RECMNG_DEL_FILE_CALLBACK_FN_PTR)(const HI_CHAR *pazFilename);

/** loop strategy attribute */
typedef struct hiRECMNG_LOOP_ATTR_S
{
    HI_BOOL bLoop; /**<enable or disable loop strayegy */
    HI_U32 u32LoopNum; /**<loop record file number, should not beyond HI_RECMNG_MAX_LOOP_NUM, meaningless if loop strayegy is disabled */
    HI_U32 u32FileIdx; /**<file index for loop strategy, range: [0, HI_RECMNG_TASK_MAX_FILE_CNT) */
    HI_RECMNG_DEL_FILE_CALLBACK_FN_PTR pfnDelFile; /**<callback function of delete file */
} HI_RECMNG_LOOP_ATTR_S;


/** record task's attribute */
typedef struct hiRECMNG_ATTR_S
{
    HI_REC_TYPE_E enRecType; /**<record type */
    union tagRECORD_ATTR_U
    {
        HI_REC_NORMAL_ATTR_S stNormalAttr; /** struct of normal record attribute */
        HI_REC_LAPSE_ATTR_S stLapseAttr; /** struct of lapse record attribute */
    } unRecordAttr;

    HI_REC_SPLIT_ATTR_S stSplitAttr; /**<split attribute, include split type and split interval */

    HI_U32 u32FileCnt; /**<file count in record task, should not beyond HI_RECMNG_TASK_MAX_FILE_CNT */
    HI_RECMNG_FILE_ATTR_S astFileAttr[HI_RECMNG_TASK_MAX_FILE_CNT]; /**<file attribute, include data source, muxer configure and so on */
    HI_RECMNG_GET_FILENAME_S stGetFilename; /**<record get filename callback and private data */

    HI_RECMNG_THM_TYPE_E enThmType; /**<record thumbnail storage type */
    HI_RECMNG_THM_ATTR_S stThmAttr; /**<thumbnail attribute, include thumbnail data source */

    HI_RECMNG_LOOP_ATTR_S stLoopAttr; /**<loop record attribute, including enable, loop num, delete file callback and so on*/

    HI_U32 u32BufferTimeMSec; /**<record buffer size in mili second */
    HI_U32 u32PreRecTimeSec; /**<pre-record time in second */
} HI_RECMNG_ATTR_S;


/**
 * @brief     register event
 * @return    0: success, non-zero: error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/4
 */
HI_S32 HI_RECMNG_RegisterEvent(HI_VOID);

/**
* @brief     record manage module initialization
* @param[in] pstMediaOps: media operate
* @return    0: success, non-zero: error code.
* @exception None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
*/
HI_S32 HI_RECMNG_Init(const HI_RECMNG_MEDIA_OPERATE_S *pstMediaOps);

/**
* @brief     record manage module deinitialization
* @return    0: success, non-zero: error code.
* @exception None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
*/
HI_S32 HI_RECMNG_Deinit(HI_VOID);

/**
* @brief      create record task, including multi-stream.
* @param[in]  pstRecAttr: record task attribute, e.g. record type, split, stream, thm.
* @param[out] pHdl: record task handle
* @return     0: success, non-zero: error code.
* @exception  None
* @author     HiMobileCam Reference Develop Team
* @date       2017/12/13
*/
HI_S32 HI_RECMNG_CreateTask(const HI_RECMNG_ATTR_S *pstRecAttr, HI_HANDLE *pHdl);

/**
* @brief     destroy record task.
* @param[in] Hdl: record task handle, created by HI_RECMNG_CreateTask
* @return    0: success, non-zero: error code.
* @exception None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
*/
HI_S32 HI_RECMNG_DestroyTask(HI_HANDLE Hdl);

/**
* @brief      start record task.
* @param[int] Hdl: record task handle, created by HI_RECMNG_CreateTask
* @param[in]  s32Timeout_ms: time out value in millisecond, -1: not support timeout.
* @return     0: success, non-zero: error code.
* @exception  None
* @author     HiMobileCam Reference Develop Team
* @date       2017/12/13
*/
HI_S32 HI_RECMNG_StartTask(HI_HANDLE Hdl, HI_S32 s32Timeout_ms);

/**
* @brief     stop record task .
* @param[in] Hdl: record task handle, created by HI_RECMNG_CreateTask
* @param[in] bQuickMode: quick stop mode flag.
* @param[in] s32Timeout_ms: time out value in millisecond, -1: not support timeout.
* @return    0: success, non-zero: error code.
* @exception None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
*/
HI_S32 HI_RECMNG_StopTask(HI_HANDLE Hdl, HI_BOOL bQuickMode, HI_S32 s32Timeout_ms);

/**
* @brief     stop record task asynchronously.
* @param[in] Hdl: record task handle, created by HI_RECMNG_CreateTask
* @param[in] bQuickMode: quick stop mode flag.
* @return    0: success, non-zero: error code.
* @exception None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
*/
HI_S32 HI_RECMNG_AsyncStopTask(HI_HANDLE Hdl, HI_BOOL bQuickMode);

/**
* @brief     manual split record file.
* @param[in] Hdl: record task handle, created by HI_RECMNG_CreateTask
* @param[in] pstSplitAttr: pre manual split attribute.
* @param[in] bStopAfterManualSplitEnd: whether stop task after manual split
* @return    0: success, non-zero: error code.
* @exception None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
*/
HI_S32 HI_RECMNG_ManualSplit(HI_HANDLE Hdl, HI_REC_MANUAL_SPLIT_ATTR_S *pstSplitAttr, HI_BOOL bStopAfterManualSplitEnd);

/**
* @brief     write private data.
* @param[in] PrivDataHdl: private data handle
* @param[in] pstPrivData: private data.
* @param[in] pvPrivDataInfo: private data info.
* @return    0: success, non-zero: error code.
* @exception None
* @author    HiMobileCam Reference Develop Team
* @date      2019/3/20
*/
HI_S32 HI_RECMNG_WritePrivateData(HI_HANDLE PrivDataHdl, HI_RECMNG_PRIV_DATA_S *pstPrivData, HI_VOID* pvPrivDataInfo);

/**@}*/  /**<!-- ==== RECMNG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_RECORDMNG_H__ */

