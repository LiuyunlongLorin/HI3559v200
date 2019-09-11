/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_aenc_inner.h
 * @brief   aenc module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MAPI_AENC_INNER_H__
#define __MAPI_AENC_INNER_H__

#include <pthread.h>
#include "hi_mapi_aenc.h"
#include "hi_mapi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AENC */
/** @{ */ /** <!-- [AENC] */

#define AENCODE_GET_FRAME_THREAD_USLEEP_TIME         (10 * 1000)
#define AENCODE_GET_FRAME_THREAD_SLEECT_TIMEOUT_USEC (10 * 1000)
#define AENC_PROCESS_STREAM_USLEEP_TIME              (10 * 1000)

#define CHECK_MAPI_AENC_NULL_PTR(ptr)                                \
    do {                                                             \
        if (HI_NULL == ptr) {                                        \
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC NULL pointer\n"); \
            return HI_MAPI_AENC_ENULLPTR;                            \
        }                                                            \
    } while (0)

#define CHECK_MAPI_AENC_CHECK_INIT(state)                                                                        \
    do {                                                                                                         \
        if (!state) {                                                                                            \
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n"); \
            return HI_MAPI_AENC_ENOTINITED;                                                                      \
        }                                                                                                        \
    } while (0)

#define CHECK_MAPI_AENC_ACAPHANDLE_RANGE(handle)                                                                      \
    do {                                                                                                              \
        if (handle >= HI_MAPI_ACAP_DEV_MAX_NUM) {                                                                     \
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP handle[%u] out of range[%u]\n", handle, HI_MAPI_ACAP_DEV_MAX_NUM); \
            return HI_MAPI_AENC_EINVALIDHDL;                                                                          \
        }                                                                                                             \
    } while (0)

#define CHECK_MAPI_AENC_ACAPCHNHANDLE_RANGE(handle)                                            \
    do {                                                                                       \
        if (handle >= HI_MAPI_ACAP_CHN_MAX_NUM) {                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP Chn handle[%u] out of range[%u]\n", handle, \
                           HI_MAPI_ACAP_CHN_MAX_NUM);                                          \
            return HI_MAPI_AENC_EINVALIDHDL;                                                   \
        }                                                                                      \
    } while (0)

#define CHECK_MAPI_AENC_HANDLE_RANGE(handle)                                                                          \
    do {                                                                                                              \
        if (handle >= HI_MAPI_AENC_CHN_MAX_NUM) {                                                                     \
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC handle[%u] out of range[%u]\n", handle, HI_MAPI_AENC_CHN_MAX_NUM); \
            return HI_MAPI_AENC_EINVALIDHDL;                                                                          \
        }                                                                                                             \
    } while (0)

/* Structure */
/** aenc chn state */
typedef enum tagMAPI_AENC_CHN_STATE_E {
    MAPI_AENC_CHN_UNINITED = 0,
    MAPI_AENC_CHN_STOPED, /**< inited but not start */
    MAPI_AENC_CHN_STARTED,
    MAPI_AENC_CHN_BUTT
} MAPI_AENC_CHN_STATE_E;

/** aenc datafifo state */
typedef enum tagMAPI_AENC_DATAFIFO_STATE_E {
    MAPI_AENC_DATAFIFO_CLOSED = 0,
    MAPI_AENC_DATAFIFO_OPENED,
    MAPI_AENC_DATAFIFO_BUTT
} MAPI_AENC_DATAFIFO_STATE_E;

/** aenc hisilink stream struct */
typedef struct tagMAPI_AENC_HISILINK_STREAM_S {
    HI_HANDLE handle;
    AUDIO_STREAM_S stAencData;
    HI_VOID *pPriv;
} MAPI_AENC_HISILINK_STREAM_S;

/* data info about datafifo */
typedef struct tagMAPI_AENC_LINKDATA_INFO_S {
    HI_U32 u32LinkFrmNum; /**< have data write to datafifo for server , have data read from datafifo for client */
    HI_U32 u32LastFrmSeq; /**< last frame sequence which write to datafifo */
} MAPI_AENC_LINKDATA_INFO_S;

/** hisilink data process callback function pointer */
typedef HI_S32 (*HI_MAPI_AENC_LINKDATAPROC_CALLBACK_FN_PTR)(HI_HANDLE AencHdl,
    MAPI_AENC_HISILINK_STREAM_S *pLinkAStreamData);

/** aenc chn info struct */
typedef struct tagMAPI_AENC_CHN_S {
    HI_HANDLE AencChnHdl;                 /**< chn handle */
    HI_S32 s32AencChnFd;                  /**< get encode frame fd by chn */
    HI_MAPI_AENC_ATTR_S stAencChnAttr;    /**< aenc chn attr */
    MAPI_AENC_CHN_STATE_E enAencChnState; /**< aenc chn current state */
    pthread_mutex_t m_AencChn_lock;
    HI_S32 s32AiDev; /**< bind ai dev */
    HI_S32 s32AiChn; /**< bind ai chn */
    HI_MAPI_AENC_CALLBACK_S astAencRegCallBacks[HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM];
} MAPI_AENC_CHN_S;

/** aenc datafifo info struct */
typedef struct tagMAPI_AENC_DATAFIFO_S {
    HI_U64 DatafifoHdl;
    HI_U64 u64DataFifoSharePhyAddr;              /**< datafifo phy address */
    HI_U64 u64StreamBufPhyAddr;                  /**< datafifo data phy address */
    HI_U32 u32StreamBufLen;                      /**< datafifo data len */
    HI_VOID *pDataVirtAddr;                      /**< datafifo data virtual addr */
    MAPI_AENC_LINKDATA_INFO_S stALinkDataInfo;   /**< data info about datafifo */
    MAPI_AENC_DATAFIFO_STATE_E enADatafifoState; /**< datafifo current state */
    HI_MAPI_AENC_LINKDATAPROC_CALLBACK_FN_PTR pFnALinkDataProcCB;
    MAPI_AENC_HISILINK_STREAM_S stUnhandleStream; /**< the last unhandle aenc frame */
} MAPI_AENC_DATAFIFO_S;

/** thread info which use datafifo */
typedef struct tagMAPI_AENC_PROCADATA_THREAD_S {
    pthread_t tProcADataThread;
    pthread_mutex_t m_ProcADataThd_lock; /**< lock for condition */
    pthread_cond_t c_ProcADataThd_cond;  /**< condition for thread sleep */
    HI_BOOL bProcADataThdStartFlg;       /**< control thread start or stop */
} MAPI_AENC_PROCADATA_THREAD_S;

/** context for aenc */
typedef struct tagMAPI_AENC_CONTEXT_S {
    HI_BOOL bAencInited; /**< module inited flag */
    MAPI_AENC_CHN_S astAencChn[HI_MAPI_AENC_CHN_MAX_NUM];
    MAPI_AENC_DATAFIFO_S astADatafifo[HI_MAPI_AENC_CHN_MAX_NUM];
    MAPI_AENC_PROCADATA_THREAD_S astProcADataThread[HI_MAPI_AENC_CHN_MAX_NUM];
} MAPI_AENC_CONTEXT_S;

/** context for aenc */
typedef struct tagMAPI_AENC_CHN_CONTEXT_S {
    HI_BOOL bAencInited; /**< module inited flag */
    MAPI_AENC_CHN_S astAencChn[HI_MAPI_AENC_CHN_MAX_NUM];
    MAPI_AENC_PROCADATA_THREAD_S astProcADataThread[HI_MAPI_AENC_CHN_MAX_NUM];
} MAPI_AENC_CHN_CONTEXT_S;

HI_S32 MAPI_AENC_OpenDatafifoWriter(HI_HANDLE AencHdl, MAPI_AENC_DATAFIFO_S *pstAencDatafifo);
HI_S32 MAPI_AENC_CloseDatafifoWriter(HI_HANDLE AencHdl);
HI_U32 MAPI_AENC_GetLinkDataInfo(HI_HANDLE AencHdl, MAPI_AENC_LINKDATA_INFO_S *pstLinkDataInfo);
HI_S32 MAPI_AENC_Client_Init(HI_VOID);
HI_S32 MAPI_AENC_Client_Deinit(HI_VOID);
HI_S32 MAPI_AENC_Init(HI_VOID);
HI_S32 MAPI_AENC_Deinit(HI_VOID);
HI_VOID MAPI_AENC_ReleaseUnhandleStream(HI_HANDLE AencHdl);

/** @} */ /** <!-- ==== AENC End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_AENC_INNER_H__ */
