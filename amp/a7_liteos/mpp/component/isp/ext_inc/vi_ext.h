/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : vi_ext.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       :
  Description   :
  History       :
  1.Date        : 2016/10/08
    Author      :
    Modification: Created file

******************************************************************************/
#ifndef __VI_EXT_H__
#define __VI_EXT_H__

#include "hi_type.h"
#include "hi_common.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/* Pipe work mode */
typedef enum hiVI_PIPE_MODE_E {
    VI_PIPE_MODE_OFFLINE = 0, /* Offline mode */
    VI_PIPE_MODE_ONLINE, /* Online mode */
    VI_PIPE_MODE_PARALLEL, /* Parallel mode */

    VI_PIPE_MODE_BUTT
} VI_PIPE_MODE_E;

typedef struct hiVI_FLASH_CALLBACK_S {
    HI_S32 (*pfnViGetFlashStatus)(HI_VOID);
} VI_FLASH_CALLBACK_S;

typedef struct hiVI_PIPE_HDR_ATTR_S {
    DYNAMIC_RANGE_E enDynamicRange;
    PIXEL_FORMAT_E enFormat;
    HI_BOOL bMirror;
} VI_PIPE_HDR_ATTR_S;

typedef struct hiVI_PIPE_WDR_ATTR_S {
    VI_DEV ViDev;
    WDR_MODE_E enWDRMode;
    VI_DEV_BIND_PIPE_S stDevBindPipe;
    HI_BOOL bMastPipe;
} VI_PIPE_WDR_ATTR_S;

/*
Defines the ISP stitch attribute.
*/
typedef struct hiVI_STITCH_ATTR_S {
    HI_BOOL bStitchEnable;
    HI_BOOL bMainPipe;
    HI_U8 u8StitchPipeNum;
    HI_S8 as8StitchBindId[VI_MAX_PIPE_NUM];
} VI_STITCH_ATTR_S;

typedef struct hiISP_STITCH_SYNC_BE_STATS_S {
    HI_VOID *pstSyncBeStt[VI_MAX_PIPE_NUM];
} ISP_STITCH_SYNC_BE_STATS_S;

typedef struct hiVI_SNAP_STATUS_S {
    HI_BOOL bDiscardFrame;
    HI_U32 u32FrameFlag;
} VI_SNAP_STATUS_S;

typedef struct hiVI_PIPE_SPLIT_ATTR_S {
    VI_VPSS_MODE_E enMode;
    HI_S32 s32SplitNum;
    HI_U32 u32OverLap;
    RECT_S astRect[VI_MAX_SPLIT_NODE_NUM];
    RECT_S astWchOutRect;
    PIXEL_FORMAT_E enPixFmt;
} VI_PIPE_SPLIT_ATTR_S;

typedef struct hiVI_PROC_MODULE_PARAM_S {
    HI_BOOL bViProcEn[VIPROC_IP_NUM];
} VI_PROC_MODULE_PARAM_S;

typedef struct hiVI_FUSION_GROUP_S {
    HI_BOOL bFusionEn;
    HI_U32 u32PipeNum;
    VI_PIPE PipeId[VI_MAX_PIPE_NUM];
} VI_FUSION_GROUP_S;

typedef enum hiSNAP_ISP_STATE_E {
    SNAP_STATE_NULL = 0,
    SNAP_STATE_CFG = 1,
    SNAP_STATE_BUTT
} SNAP_ISP_STATE_E;

typedef struct hiISP_SNAP_INFO_S {
    SNAP_ISP_STATE_E enSnapState;
    HI_U32 u32ProIndex;
    ISP_CONFIG_INFO_S stIspCfgInfo;
} ISP_SNAP_INFO_S;

typedef HI_S32 FN_VI_RegisterFlashCallBack(HI_S32 ViDev, VI_FLASH_CALLBACK_S *pstFlashCb);
typedef HI_S32 FN_VI_GetPipeWDRAttr(VI_PIPE ViPipe, VI_PIPE_WDR_ATTR_S *pstWDRAttr);
typedef HI_S32 FN_VI_GetPipeHDRAttr(VI_PIPE ViPipe, VI_PIPE_HDR_ATTR_S *pstHDRAttr);
typedef HI_S32 FN_VI_GetPipeBindDevId(VI_PIPE ViPipe, VI_DEV *ViDev);
typedef HI_S32 FN_VI_GetPipeBindDevId(VI_PIPE ViPipe, VI_DEV *ViDev);
typedef HI_S32 FN_VI_GetBeSttInfo(VI_PIPE ViPipe, HI_U8 u8Block, HI_VOID *pstBeStt);
typedef HI_S32 FN_VI_GetPipeStitchAttr(VI_PIPE ViPipe, VI_STITCH_ATTR_S *pstStitchAttr);
typedef HI_S32 FN_VI_GetStitchSyncBeSttInfo(VI_PIPE ViPipe, ISP_STITCH_SYNC_BE_STATS_S *pstStitchSyncStats);
typedef HI_S32 FN_VI_UpdateViVpssMode(VI_VPSS_MODE_S *pstViVpssMode);
typedef HI_S32 FN_VI_GetPipeSnapStatus(VI_PIPE ViPipe, VI_SNAP_STATUS_S *pstViSnapStatus);
typedef HI_S32 FN_VI_GetSplitAttr(VI_PIPE ViPipe, VI_PIPE_SPLIT_ATTR_S *pstSplitAttr);
typedef HI_S32 FN_VI_GetProcModuleParam(VI_PROC_MODULE_PARAM_S *pstProcModuleParam);
typedef HI_S32 FN_VI_DownSemaphore(VI_PIPE ViPipe);
typedef HI_S32 FN_VI_UpSemaphore(VI_PIPE ViPipe);
typedef HI_BOOL FN_VI_IsPipeExisted(VI_PIPE ViPipe);
typedef HI_S32 FN_VI_GetPipeBindDevSize(VI_PIPE ViPipe, SIZE_S *pstSize);
typedef HI_S32 FN_VI_GetPipeNRXStatus(VI_PIPE ViPipe, HI_BOOL *pbNrxEn);
typedef HI_S32 FN_VI_GetFusionGroupAttr(VI_PIPE ViPipe, VI_FUSION_GROUP_S *pstFusionGroupAttr);
typedef HI_S32 FN_VI_SetVpssFrameInterruptAttr(VI_PIPE ViPipe, const FRAME_INTERRUPT_ATTR_S *pstFrameIntAttr);
typedef HI_S32 FN_VI_GetViFrameInterruptAttr(VI_PIPE ViPipe, FRAME_INTERRUPT_ATTR_S *pstFrameIntAttr);
typedef HI_S32 FN_VI_IspClearInputQueue(VI_PIPE ViPipe);
typedef HI_S32 FN_VI_GetIspConfig(VI_PIPE ViPipe, ISP_SNAP_INFO_S *pstSnapIspInfo);

typedef struct hiVI_EXPORT_FUNC_S {
    FN_VI_GetStitchSyncBeSttInfo *pfnViGetStitchSyncBeSttInfo;
    FN_VI_GetPipeStitchAttr *pfnViGetPipeStitchAttr;
    FN_VI_GetPipeWDRAttr *pfnViGetPipeWDRAttr;
    FN_VI_GetPipeHDRAttr *pfnViGetPipeHDRAttr;
    FN_VI_GetPipeBindDevId *pfnViGetPipeBindDevId;
    FN_VI_GetBeSttInfo *pfnViGetBeSttInfo;
    FN_VI_RegisterFlashCallBack *pfnViRegisterFlashCallBack;
    FN_VI_UpdateViVpssMode *pfnVIUpdateViVpssMode;
    FN_VI_GetPipeSnapStatus *pfnViGetPipeSnapStatus;
    FN_VI_GetSplitAttr *pfnViGetSplitAttr;
    FN_VI_GetProcModuleParam *pfnViGetProcModuleParam;
    FN_VI_DownSemaphore *pfnViDownSemaphore;
    FN_VI_UpSemaphore *pfnViUpSemaphore;
    FN_VI_IsPipeExisted *pfnViIsPipeExisted;
    FN_VI_GetPipeBindDevSize *pfnViGetPipeBindDevSize;
    FN_VI_GetPipeNRXStatus *pfnViGetPipeNRXStatus;
    FN_VI_GetFusionGroupAttr *pfnViGetFusionGroupAttr;
    FN_VI_SetVpssFrameInterruptAttr *pfnViSetVpssFrameInterruptAttr;
    FN_VI_GetViFrameInterruptAttr *pfnViGetViFrameInterruptAttr;
    FN_VI_IspClearInputQueue *pfnViIspClearInputQueue;
    FN_VI_GetIspConfig *pfnViGetIspConfig;

} VI_EXPORT_FUNC_S;

#define CKFN_VI_IspClearInputQueue() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViIspClearInputQueue != HI_NULL)
#define CALL_VI_IspClearInputQueue(ViPipe) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViIspClearInputQueue(ViPipe)

#define CKFN_VI_GetViFrameInterruptAttr() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetViFrameInterruptAttr != HI_NULL)
#define CALL_VI_GetViFrameInterruptAttr(ViPipe, pstFrameIntAttr) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetViFrameInterruptAttr(ViPipe, pstFrameIntAttr)

#define CKFN_VI_SetVpssFrameInterruptAttr() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViSetVpssFrameInterruptAttr != HI_NULL)
#define CALL_VI_SetVpssFrameInterruptAttr(ViPipe, pstFrameIntAttr) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViSetVpssFrameInterruptAttr(ViPipe, pstFrameIntAttr)

#define CKFN_VI_UpdateViVpssMode() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnVIUpdateViVpssMode != HI_NULL)
#define CALL_VI_UpdateViVpssMode(pstViVpssMode) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnVIUpdateViVpssMode(pstViVpssMode)

#define CKFN_VI_GetPipeStitchAttr() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeStitchAttr != HI_NULL)
#define CALL_VI_GetPipeStitchAttr(ViPipe, pstStitchAttr) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeStitchAttr(ViPipe, pstStitchAttr)

#define CKFN_VI_GetStitchSyncBeSttInfo() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetStitchSyncBeSttInfo != HI_NULL)
#define CALL_VI_GetStitchSyncBeSttInfo(ViPipe, pstStitchSyncStats) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetStitchSyncBeSttInfo(ViPipe, pstStitchSyncStats)

#define CKFN_VI_GetPipeWDRAttr() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeWDRAttr != HI_NULL)
#define CALL_VI_GetPipeWDRAttr(ViPipe, pstWDRAttr) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeWDRAttr(ViPipe, pstWDRAttr)

#define CKFN_VI_GetPipeHDRAttr() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeHDRAttr != HI_NULL)
#define CALL_VI_GetPipeHDRAttr(ViPipe, pstHDRAttr) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeHDRAttr(ViPipe, pstHDRAttr)

#define CKFN_VI_GetPipeBindDevId() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeBindDevId != HI_NULL)
#define CALL_VI_GetPipeBindDevId(ViPipe, ViDev) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeBindDevId(ViPipe, ViDev)

#define CKFN_VI_GetBeSttInfo() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetBeSttInfo != HI_NULL)
#define CALL_VI_GetBeSttInfo(ViPipe, u8Block, pstBeStt) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetBeSttInfo(ViPipe, u8Block, pstBeStt)

#define CKFN_VI_RegisterFlashCallBack() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViRegisterFlashCallBack != HI_NULL)
#define CALL_VI_RegisterFlashCallBack(ViDev, pstFlashCb) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViRegisterFlashCallBack(ViDev, pstFlashCb)

#define CKFN_VI_GetSnapStatus() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeSnapStatus != HI_NULL)
#define CALL_VI_GetSnapStatus(ViPipe, pstViSnapStatus) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeSnapStatus(ViPipe, pstViSnapStatus)

#define CKFN_VI_GetSplitAttr() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetSplitAttr != HI_NULL)
#define CALL_VI_GetSplitAttr(ViPipe, pstSplitAttr) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetSplitAttr(ViPipe, pstSplitAttr)

#define CKFN_VI_GetProcModuleParam() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetProcModuleParam != HI_NULL)
#define CALL_VI_GetProcModuleParam(pstProcModuleParam) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetProcModuleParam(pstProcModuleParam)

#define CKFN_VI_DownSemaphore() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViDownSemaphore != HI_NULL)
#define CALL_VI_DownSemaphore(ViPipe) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViDownSemaphore(ViPipe)

#define CKFN_VI_UpSemaphore() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViUpSemaphore != HI_NULL)
#define CALL_VI_UpSemaphore(ViPipe) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViUpSemaphore(ViPipe)

#define CKFN_VI_IsPipeExisted() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViIsPipeExisted != HI_NULL)
#define CALL_VI_IsPipeExisted(ViPipe) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViIsPipeExisted(ViPipe)

#define CKFN_VI_GetPipeBindDevSize() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeBindDevSize != HI_NULL)
#define CALL_VI_GetPipeBindDevSize(ViPipe, pstSize) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeBindDevSize(ViPipe, pstSize)

#define CKFN_VI_GetPipeNRXStatus() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeNRXStatus != HI_NULL)
#define CALL_VI_GetPipeNRXStatus(ViPipe, pbNrxEn) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetPipeNRXStatus(ViPipe, pbNrxEn)

#define CKFN_VI_GetFusionGroupAttr() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetFusionGroupAttr != HI_NULL)
#define CALL_VI_GetFusionGroupAttr(ViPipe, pstFusionGroupAttr) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetFusionGroupAttr(ViPipe, pstFusionGroupAttr)

#define CKFN_VI_GetIspConfig() \
    (FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetIspConfig != HI_NULL)
#define CALL_VI_GetIspConfig(ViPipe, pstSnapIspInfo) \
    FUNC_ENTRY(VI_EXPORT_FUNC_S, HI_ID_VI)->pfnViGetIspConfig(ViPipe, pstSnapIspInfo)

typedef struct hi_VI_CAP_PIC_INFO_S {
    VIDEO_FRAME_INFO_S stVideoFrame;
    ISP_CONFIG_INFO_S stIspConfigInfo;
    ISP_DCF_UPDATE_INFO_S stDCFUpdateInfo;
    HI_U64 u64StartTime;
    HI_U64 u64EndTime;
} VI_CAP_PIC_INFO_S;

typedef struct HI_VI_BUF_INFO_S {
    VI_CAP_PIC_INFO_S stPicInfo;
    HI_BOOL bBufOk;
    HI_BOOL bCapOK;
    HI_BOOL bSended;
    HI_BOOL bLost;
} VI_BUF_INFO_S;

typedef enum hiVI_FRAME_INFO_E {
    FRAME_NORMAL = 0x0,
    FRAME_LOWDELAY,
    FRAME_LOWDELAY_FIN,
} VI_FRAME_INFO_E;

typedef enum hiVI_FRAME_SOURCE_TYPE_E {
    VI_FRAME_SOURCE_NORMAL = 0x0,
    VI_FRAME_SOURCE_USERPIC,
    VI_FRAME_SOURCE_BUTT,
} VI_FRAME_SOURCE_TYPE_E;

typedef struct hi_VI_PIC_INFO_S {
    VIDEO_FRAME_INFO_S stVideoFrame;
    VIDEO_FRAME_INFO_S stSingleYFrame;
    HI_U64 u64StartTime;
    HI_U64 u64EndTime;
    VI_FRAME_INFO_E enFrameInfo;
    VI_FRAME_SOURCE_TYPE_E enFrameSource;
} VI_PIC_INFO_S;

typedef struct hi_VI_SEND_INFO_S {
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_PIC_INFO_S *pstDestPicInfo;
} VI_SEND_INFO_S;

typedef enum hiVI_FPN_WORK_MODE_E {
    FPN_MODE_NONE = 0x0,
    FPN_MODE_CORRECTION,
    FPN_MODE_CALIBRATE,
    FPN_MODE_BUTT
} VI_FPN_WORK_MODE_E;

typedef struct hiVI_FPN_ATTR_S {
    VI_FPN_WORK_MODE_E enFpnWorkMode;
    union {
        ISP_FPN_CALIBRATE_ATTR_S stCalibrateAttr;
        ISP_FPN_ATTR_S stCorrectionAttr;
    };
} VI_FPN_ATTR_S;

HI_S32 MPI_VI_SetFPNAttr(VI_PIPE ViPipe, VI_FPN_ATTR_S *pstFPNAttr);
HI_S32 MPI_VI_GetFPNAttr(VI_PIPE ViPipe, VI_FPN_ATTR_S *pstFPNAttr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

