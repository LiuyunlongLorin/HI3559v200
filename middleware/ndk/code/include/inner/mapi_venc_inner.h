/**
 * Copyright (C), 2017-2018, Hisilicon Tech. Co., Ltd.
 * All rights reserved.
 *
 * @file     mapi_venc_inner.h
 * @brief    inner struct definition for video encoded module.
 * @author   HiMobileCam NDK develop team
 * @date     2018/02/07
 */
#ifndef __MAPI_VENC_INNER_H__
#define __MAPI_VENC_INNER_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <pthread.h>

#include "hi_mapi_comm_define.h"
#include "hi_mapi_venc_define.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_log.h"

#define VENC_GET_FRAME_THREAD_USLEEP_TIME         (10 * 1000)
#define VENC_GET_FRAME_THREAD_SLEECT_TIMEOUT_USEC (10 * 1000)
#define VENC_PROCESS_STREAM_USLEEP_TIME           (10 * 1000)
#define VENC_FRAME_LOST_BPS_THRESHOLD             (8 * 8 * 1024 * 1024)
#define SEC2NSEC_SCALE                            (1 * 1000 * 1000 * 1000)
#define VENC_MAX_U32_VALUE                        (0xffffffff)

#define MAPI_VENC_DATAFIFONODE_CNT 100

/* Fast-start version, maximum frame rate at high frame rate */
#define MAX_STREAM_CNT 800

#define VENC_SCENEMODE_TO_MPI(scenemode) ((scenemode - HI_MAPI_VENC_SCENE_MODE_NORMAL) + SCENE_0)

#define ALIGN_NEXT(v, a)   ((((v) + ((a)-1)) & (~((a)-1))))
#define ALIGN_LENGTH(l, a) ALIGN_NEXT(l, a)

#define CHECK_MAPI_VENC_NULL_PTR(ptr)                                \
    do {                                                             \
        if (NULL == ptr) {                                           \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC NULL pointer\n"); \
            return HI_MAPI_VENC_ENULL_PTR;                           \
        }                                                            \
    } while (0)

#define CHECK_MAPI_VENC_CHECK_PREINIT(state)                                           \
    do {                                                                               \
        if (!state) {                                                                  \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "media not init yet,venc no pre init\n"); \
            return HI_MAPI_VENC_ENOT_INITED;                                           \
        }                                                                              \
    } while (0)

#define CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(state)                                    \
    do {                                                                               \
        if (!state) {                                                                  \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "media not init yet,venc no pre init\n"); \
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);                                    \
            return HI_MAPI_VENC_ENOT_INITED;                                           \
        }                                                                              \
    } while (0)

#define CHECK_MAPI_VENC_CHECK_INIT(state)                                   \
    do {                                                                    \
        if (!state) {                                                       \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC has not been inited\n"); \
            return HI_MAPI_VENC_ENOT_INITED;                                \
        }                                                                   \
    } while (0)

#define CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(state)                            \
    do {                                                                    \
        if (!state) {                                                       \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC has not been inited\n"); \
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);                         \
            return HI_MAPI_VENC_ENOT_INITED;                                \
        }                                                                   \
    } while (0)

#define CHECK_MAPI_VCAPPIPE_HANDLE(handle)                                                                     \
    do {                                                                                                       \
        if (HI_MAPI_VCAP_MAX_PIPE_NUM <= handle) {                                                             \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VcapPipe handle(%d) is invalid, only support [0, %d]\n", handle, \
                           HI_MAPI_VCAP_MAX_PIPE_NUM - 1);                                                     \
            return HI_MAPI_VENC_EHANDLE_ILLEGAL;                                                               \
        }                                                                                                      \
    } while (0)

#define CHECK_MAPI_PIPECHN_HANDLE(handle)                                                                     \
    do {                                                                                                      \
        if (HI_MAPI_PIPE_MAX_CHN_NUM <= handle) {                                                             \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "PipeChn handle(%d) is invalid, only support [0, %d]\n", handle, \
                           HI_MAPI_PIPE_MAX_CHN_NUM - 1);                                                     \
            return HI_MAPI_VENC_EHANDLE_ILLEGAL;                                                              \
        }                                                                                                     \
    } while (0)

#define CHECK_MAPI_VPSS_HANDLE(handle)                                                                     \
    do {                                                                                                   \
        if (HI_MAPI_VPSS_MAX_NUM <= handle) {                                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Vpss handle(%d) is invalid, only support [0, %d]\n", handle, \
                           HI_MAPI_VPSS_MAX_NUM - 1);                                                      \
            return HI_MAPI_VENC_EHANDLE_ILLEGAL;                                                           \
        }                                                                                                  \
    } while (0)

#define CHECK_MAPI_VPORT_HANDLE(handle)                                                                     \
    do {                                                                                                    \
        if (HI_MAPI_VPORT_MAX_NUM <= handle) {                                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VPort handle(%d) is invalid, only support [0, %d]\n", handle, \
                           HI_MAPI_VPORT_MAX_NUM - 1);                                                      \
            return HI_MAPI_VENC_EHANDLE_ILLEGAL;                                                            \
        }                                                                                                   \
    } while (0)

#define CHECK_MAPI_STITCH_HANDLE(handle)                                                                     \
    do {                                                                                                     \
        if (HI_MAPI_STITCH_MAX_NUM <= handle) {                                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Stitch handle(%d) is invalid, only support [0, %d]\n", handle, \
                           HI_MAPI_STITCH_MAX_NUM - 1);                                                      \
            return HI_MAPI_VENC_EHANDLE_ILLEGAL;                                                             \
        }                                                                                                    \
    } while (0)

#define CHECK_MAPI_STITCHPORT_HANDLE(handle)                                                                     \
    do {                                                                                                         \
        if (HI_MAPI_STITCH_PORT_MAX_NUM <= handle) {                                                             \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "StitchPort handle(%d) is invalid, only support [0, %d]\n", handle, \
                           HI_MAPI_STITCH_PORT_MAX_NUM - 1);                                                     \
            return HI_MAPI_VENC_EHANDLE_ILLEGAL;                                                                 \
        }                                                                                                        \
    } while (0)

#define CHECK_MAPI_VENC_HANDLE_RANGE(handle)                                \
    do {                                                                    \
        if (HI_MAPI_VENC_MAX_CHN_NUM <= handle) {                           \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC handle out of range\n"); \
            return HI_MAPI_VENC_EHANDLE_ILLEGAL;                            \
        }                                                                   \
    } while (0)

#define CHECK_MAPI_VENC_FRAMECNT_RANGE(framecnt)                              \
    do {                                                                      \
        if (HI_MAPI_VENC_LIMITLESS_FRAME_COUNT > framecnt || 0 == framecnt) { \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "frame count out of range\n");   \
            return HI_MAPI_VENC_EILLEGAL_PARAM;                               \
        }                                                                     \
    } while (0)

#define CHECK_MAPI_VENC_OSD_HANDLE_RANGE(handle)                                \
    do {                                                                        \
        if (HI_MAPI_VENC_OSD_MAX_NUM <= handle) {                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC OSD handle out of range\n"); \
            return HI_MAPI_VENC_EHANDLE_ILLEGAL;                                \
        }                                                                       \
    } while (0)

#define CHECK_MAPI_VENC_MAX_VAL(paraname, atc, max)                                                         \
    do {                                                                                                    \
        if (atc > max) {                                                                                    \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "%s:%d can not larger than max val %d\n", paraname, atc, max); \
            return HI_MAPI_VENC_EILLEGAL_PARAM;                                                             \
        }                                                                                                   \
    } while (0)

#define CHECK_MAPI_VENC_MIN_VAL(paraname, atc, min)                                                       \
    do {                                                                                                  \
        if (atc < min) {                                                                                  \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "%s:%d can not less than min val %d\n", paraname, atc, min); \
            return HI_MAPI_VENC_EILLEGAL_PARAM;                                                           \
        }                                                                                                 \
    } while (0)

#define CHECK_MEDIA_VENC_ALIGN(value, align)                                              \
    do {                                                                                  \
        if (0 != (value % align)) {                                                       \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "%d doesn't align with %d\n", value, align); \
            return HI_MAPI_VENC_EILLEGAL_PARAM;                                           \
        }                                                                                 \
    } while (0)

/* venc osd status information struct */
typedef struct tagMAPI_VENC_OSD_STATUS_S {
    HI_BOOL bOsdAttrSetted; /**< attribute is setted. */
    HI_BOOL bOsdStart;      /**< attribute is start. */
} VENC_OSD_STATUS_S;

/* venc hisilink stream struct */
typedef struct tagMAPI_VENC_HISILINK_STREAM_S {
    HI_HANDLE handle;
    HI_VOID *pPriv;
    HI_MAPI_VENC_DATA_S stVencData;
} MAPI_VENC_HISILINK_STREAM_S;

/* venc chn information struct */
typedef struct tagMAPI_VENC_CHN_S {
    pthread_mutex_t m_VencChn_lock;
    HI_BOOL bStarted; /**< Whether the stream has been started */
    HI_BOOL bInited;  /**< whether the stream initialized */
    HI_BOOL bFirstFrame;
    HI_HANDLE VencHandle;
    HI_S32 s32VcapPipe; /**< bound vcap pipe */
    HI_S32 s32PipeChn;  /**< bound vcap pipe channel */
    HI_S32 s32VprocGrp; /**< bound vproc group */
    HI_S32 s32VprocChn; /**< bound vproc channel */
    HI_BOOL bStitched;  /**< vpss or avs */
    HI_S32 s32VencFd;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    /* < Registered callback function ,each channel supports Registered callback function max number is:
               VENC_CHN_REGISTER_CALLBACK_MAX_COUNT */
    HI_MAPI_VENC_CALLBACK_S astVencRegisterCallbacks[HI_MAPI_VENC_CALLBACK_MAX_NUM];
    HI_S32 s32StartFrameCount;
    VENC_OSD_STATUS_S abOsdStatus[HI_MAPI_VENC_OSD_MAX_NUM];
    HI_MAPI_OSD_ATTR_S astOsdAttr[HI_MAPI_VENC_OSD_MAX_NUM];
} MAPI_VENC_CHN_S;

/* data information about datafifo */
typedef struct tagMAPI_VENC_LINKDATA_INFO_S {
    HI_U32 u32LinkFrmNum;
    HI_U32 u32LastFrmSeq;
} MAPI_VENC_LINKDATA_INFO_S;

/* hisilink data process callback function pointer */
typedef HI_S32 (*PFN_VENC_HISILINK_DataProc)(HI_HANDLE VencHdl,
    MAPI_VENC_HISILINK_STREAM_S *pVStreamLinkData);

/* venc datafifo information struct */
typedef struct tagMAPI_VENC_DATAFIFO_S {
    HI_U64 DataFifoHdl;
    HI_U64 u64DataFifoSharePhyAddr;
    HI_U64 au64StreamBufPhyAddr[MAX_TILE_NUM];
    HI_U64 au64StreamBufLen[MAX_TILE_NUM];
    HI_VOID *pDataVirtAddr;
    HI_BOOL bUsed; /* HI_TRUE: clent and server datafifo has been install link */
    HI_BOOL bFirstFrame;
    HI_BOOL bStart;
    pthread_mutex_t m_DatafifoChn_lock;
    MAPI_VENC_LINKDATA_INFO_S stVLinkDataInfo;
    PFN_VENC_HISILINK_DataProc pFnVencHisiLinkDataProc;
    MAPI_VENC_HISILINK_STREAM_S stUnhandleStream; /**< the last unhandle venc frame */
} MAPI_VENC_DATAFIFO_S;

/* thread information which use datafifo */
typedef struct tagMAPI_VENC_PROCVDATA_THREAD_S {
    pthread_t tProcVDataThread;
    pthread_mutex_t m_ProcVDataThd_lock;
    pthread_cond_t c_ProcVDataThd_cond;
    HI_BOOL bProcVDataThdStartFlg;
} MAPI_VENC_PROCVDATA_THREAD_S;

/* context for venc */
typedef struct tagMAPI_VENC_CONTEXT_S {
    HI_BOOL bVencInited;
    HI_U32 u32DatafifeNodeNum;
    MAPI_VENC_CHN_S astVencChn[HI_MAPI_VENC_MAX_CHN_NUM];
    MAPI_VENC_DATAFIFO_S astVDatafifo[HI_MAPI_VENC_MAX_CHN_NUM];
    MAPI_VENC_PROCVDATA_THREAD_S astProcVDataThread[HI_MAPI_VENC_MAX_CHN_NUM];
} MAPI_VENC_CONTEXT_S;

HI_VOID MAPI_VENC_ClearDatafifoData(HI_HANDLE VencHdl);
HI_S32 MAPI_VENC_OpenDatafifoWriter(HI_HANDLE VencHdl, HI_U32 u32DatafifeNodeNum,
                                    MAPI_VENC_DATAFIFO_S *pstVEncDatafifo);
HI_S32 MAPI_VENC_CloseDatafifoWriter(HI_HANDLE VencHdl);
HI_VOID MAPI_VENC_OpenDatafifoStatus(HI_HANDLE VencHdl);
HI_VOID MAPI_VENC_GetDataInfo(HI_HANDLE VencHdl, MAPI_VENC_LINKDATA_INFO_S *pstLinkDataInfo);
HI_BOOL MAPI_VENC_GetLiteBootStatus(HI_HANDLE VencHdl);
HI_S32 MAPI_VENC_Client_Init(HI_VOID);
HI_S32 MAPI_VENC_Client_Deinit(HI_VOID);
HI_VOID MAPI_VENC_Client_SetDatafifoNodeNum(HI_U32 u32DatafifeNodeNum);
HI_S32 MAPI_VENC_Init(const HI_MAPI_VENC_PARAM_MOD_S *pstVencModPara);
HI_S32 MAPI_VENC_Deinit(HI_VOID);

#endif /* __MAPI_VENC_INNER_H__ */
