/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vproc_inner.h
 * @brief   server vcap module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MAPI_VPROC_INNER_H__
#define __MAPI_VPROC_INNER_H__

#include <pthread.h>
#include "hi_mapi_log.h"
#include "hi_mapi_vproc_define.h"

#ifdef SUPPORT_PHOTO_POST_PROCESS
#include "mpi_photo.h"
#include "hi_comm_photo.h"
#endif

#define DEFAULT_CANVASNUM   2
#define PHOTO_ALIGN_LEN     128

/* Accroding to the header file supplied by sdk: hi_comm_video.h struct VIDEO_FRAME_S member:
    u64PhyAddr/u64VirAddr/u32Stride */
#define FRAME_DATA_ADDR_NUM 3

#define VCAPPIPE_CHECK_HANDLE(Hdl)                                                                            \
    do {                                                                                                      \
        if (Hdl >= HI_MAPI_VCAP_MAX_PIPE_NUM) {                                                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VcapPipe handle(%d) is invalid, only suppwort [0, %d]\n", Hdl, \
                           HI_MAPI_VCAP_MAX_PIPE_NUM - 1);                                                    \
            return HI_MAPI_VPROC_EILLEGAL_PARAM;                                                              \
        }                                                                                                     \
    } while (0)

#define VCAPCHN_CHECK_HANDLE(Hdl)                                                                           \
    do {                                                                                                    \
        if (Hdl >= HI_MAPI_PIPE_MAX_CHN_NUM) {                                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "PipeChn handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_PIPE_MAX_CHN_NUM - 1);                                                   \
            return HI_MAPI_VPROC_EILLEGAL_PARAM;                                                            \
        }                                                                                                   \
    } while (0)

#define VPSS_CHECK_HANDLE(Hdl)                                                                           \
    do {                                                                                                 \
        if (Hdl >= HI_MAPI_VPSS_MAX_NUM) {                                                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_VPSS_MAX_NUM - 1);                                                    \
            return HI_MAPI_VPROC_EINVALID_DEVID;                                                         \
        }                                                                                                \
    } while (0)

#define VPORT_CHECK_HANDLE(Hdl)                                                                           \
    do {                                                                                                  \
        if (Hdl >= HI_MAPI_VPORT_MAX_NUM) {                                                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VPort handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_VPORT_MAX_NUM - 1);                                                    \
            return HI_MAPI_VPROC_EINVALID_CHNID;                                                          \
        }                                                                                                 \
    } while (0)

#define STITCH_CHECK_HANDLE(Hdl)                                                                           \
    do {                                                                                                   \
        if (Hdl >= HI_MAPI_STITCH_MAX_NUM) {                                                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_STITCH_MAX_NUM - 1);                                                    \
            return HI_MAPI_VPROC_EINVALID_DEVID;                                                           \
        }                                                                                                  \
    } while (0)

#define STITCHPORT_CHECK_HANDLE(Hdl)                                                                           \
    do {                                                                                                       \
        if (Hdl >= HI_MAPI_STITCH_PORT_MAX_NUM) {                                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "StitchPort handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_STITCH_PORT_MAX_NUM - 1);                                                   \
            return HI_MAPI_VPROC_EINVALID_CHNID;                                                               \
        }                                                                                                      \
    } while (0)

#define VPROC_OSD_CHECK_HANDLE(Hdl)                                                                           \
    do {                                                                                                      \
        if (Hdl >= HI_MAPI_VPROC_OSD_MAX_NUM) {                                                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VPROC OSD handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_VPROC_OSD_MAX_NUM - 1);                                                    \
            return HI_MAPI_VPROC_EILLEGAL_PARAM;                                                              \
        }                                                                                                     \
    } while (0)

#define VPROC_CHECK_NULL_PTR(pstPtr)                              \
    do {                                                          \
        if (NULL == (pstPtr)) {                                   \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Null pointer!\n"); \
            return HI_MAPI_VPROC_ENULL_PTR;                       \
        }                                                         \
    } while (0)

#define VPROC_CHECK_RET(str, ret)                                          \
    do {                                                                   \
        if (ret != HI_SUCCESS) {                                           \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "%s,ret:[%#x]\n", str, ret); \
            return ret;                                                    \
        }                                                                  \
    } while (0)

#define VPROC_CHECK_RET_UNLOCK(str, ret, mLock)                            \
    do {                                                                   \
        if (ret != HI_SUCCESS) {                                           \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "%s,ret:[%#x]\n", str, ret); \
            MUTEX_UNLOCK(mLock);                                           \
            return ret;                                                    \
        }                                                                  \
    } while (0)

#define VPROC_CHECK_RET_EXIT(str, ret, GOTO_LABLEL)                        \
    do {                                                                   \
        if (ret != HI_SUCCESS) {                                           \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "%s,ret:[%#x]\n", str, ret); \
            goto GOTO_LABLEL;                                              \
        }                                                                  \
    } while (0)

#define VPROC_CHECK_NULL_PTR_EXIT(ret, pstPtr, GOTO_LABLEL)       \
    do {                                                          \
        if ((pstPtr) == NULL) {                                   \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Null pointer!\n"); \
            ret = HI_MAPI_VPROC_ENULL_PTR;                        \
            goto GOTO_LABLEL;                                     \
        }                                                         \
    } while (0)

#define VPSS_CHECK_HANDLE_EXIT(ret, Hdl, GOTO_LABLEL)                                                    \
    do {                                                                                                 \
        if (Hdl >= HI_MAPI_VPSS_MAX_NUM) {                                                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_VPSS_MAX_NUM - 1);                                                    \
            ret = HI_MAPI_VPROC_EINVALID_DEVID;                                                          \
            goto GOTO_LABLEL;                                                                            \
        }                                                                                                \
    } while (0)

#define VPORT_CHECK_HANDLE_EXIT(ret, Hdl, GOTO_LABLEL)                                                    \
    do {                                                                                                  \
        if (Hdl >= HI_MAPI_VPORT_MAX_NUM) {                                                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VPort handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_VPORT_MAX_NUM - 1);                                                    \
            ret = HI_MAPI_VPROC_EINVALID_CHNID;                                                           \
            goto GOTO_LABLEL;                                                                             \
        }                                                                                                 \
    } while (0)

#ifdef SUPPORT_PHOTO_POST_PROCESS

typedef struct tagPORT_PHOTO_DUMP_INFO_S {
    VB_BLK *pstVbBlk;     /* the address VB info */
    HI_S32 s32FrameCnt;   /* the count of YUV frame */
    HI_S32 s32FrameIndex; /* the index number of YUV frame */
    HI_HANDLE VencHdl;
    PHOTO_ALG_TYPE_E enAlgType;
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
    HI_U64 u64DestFrmAddr;                      /* the dest frame address */
    HI_U64 u64BNRSrcAddr;                       /* Vproc photo get bnr frame address */
    HI_U64 u64YUVSrcAddr[PHOTO_MFNR_FRAME_NUM]; /* Vproc photo get YUV frame address */
} PORT_PHOTO_DUMP_INFO_S;

typedef struct tagVPROC_PHOTO_DUMP_INFO_S {
    HI_BOOL bPhotoDump;
    HI_DUMP_BNRRAW_CALLBACK_FUNC_S stBNRRawCB;
    HI_DUMP_YUV_CALLBACK_FUNC_S stYUVDataCB;
    pthread_mutex_t PhotoDump_Lock;
} VPROC_PHOTO_DUMP_INFO_S;

typedef struct tagVPROC_PHOTO_DUMP_DATA_S {
    HI_S32 s32FrameCnt;
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
    HI_MAPI_FRAME_DATA_S stBNRRaw;
    HI_MAPI_FRAME_DATA_S stYUVData;
} VPROC_PHOTO_DUMP_DATA_S;
#endif

typedef struct tagVProcDumpYuvThreadArg_S {
    HI_HANDLE GrpHdl;
    HI_HANDLE PortHdl;
    HI_S32 s32Count;
} VProcDumpYuvThreadArg_S;

typedef struct tagVPROC_OSD_ATTR_S {
    HI_BOOL bOsdStarted;
    HI_MAPI_OSD_ATTR_S stOsdAttr;
} PORT_OSD_ATTR_S;

typedef struct tagVPSS_PORT_S {
    HI_BOOL bVPortStart;
    HI_BOOL bDumpYuvEnable;
    HI_BOOL bSupportBufferShare;
    HI_DUMP_YUV_CALLBACK_FUNC_S stCallbackFun;
    pthread_t pthreadDumpYUV;
    HI_BOOL bVpssDumpYUVStarted;
    PORT_OSD_ATTR_S astPortOsdAttr[HI_MAPI_VPROC_OSD_MAX_NUM];
    HI_HANDLE BindVencHdl;
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
    HI_DUMP_BNRRAW_CALLBACK_FUNC_S stBNRRawCB;
#ifdef SUPPORT_PHOTO_POST_PROCESS
    PORT_PHOTO_DUMP_INFO_S stPhotoDump;
#endif
} VPSS_PORT_ATTR_S;

typedef struct tagVPROC_VPSS_S {
    HI_BOOL bVpssStart;
    HI_S32 s32BindVcapPipeHdl;
    HI_S32 s32BindPipeChnHdl;
    VPSS_PORT_ATTR_S astVpssPortAttr[HI_MAPI_VPORT_MAX_NUM];
} VPROC_VPSS_S;

typedef struct tagPORT_DUMP_YUV_INFO_S {
    HI_BOOL bDumpYuvEnable;
    HI_DUMP_YUV_CALLBACK_FUNC_S stCallbackFun;
    HI_BOOL bDumpYuvStartd;
    pthread_t pthreadDumpYUV;
} PORT_DUMP_YUV_INFO_S;

#ifdef SUPPORT_STITCH
typedef struct tagSTITCH_PORT_S {
    HI_BOOL bStitchPortStart;
    HI_BOOL bDumpYuvEnable;
    HI_DUMP_YUV_CALLBACK_FUNC_S stCallbackFun;
    pthread_t pthreadDumpYUV;
    HI_BOOL bStitchDumpYUVStartd;
    PORT_OSD_ATTR_S astPortOsdAttr[HI_MAPI_VPROC_OSD_MAX_NUM];
} STITCH_PORT_ATTR_S;

typedef struct tagVPROC_STITCH_S {
    HI_U32 u32PipeNum;
    HI_BOOL bStitchStart;
    HI_VPROC_STITCH_SRC_S astStitchSrc[HI_MAPI_STITCH_PIPE_MAX_NUM];
    STITCH_PORT_ATTR_S astStitchPortAttr[HI_MAPI_STITCH_PORT_MAX_NUM];
} VPROC_STITCH_S;

#endif

HI_S32 VPROC_GetChnFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                         HI_MAPI_FRAME_DATA_S *pstYUVFrameData);
HI_S32 VPROC_ReleaseChnFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                             HI_MAPI_FRAME_DATA_S *pstYUVFrameData);
HI_S32 MAPI_VPROC_Client_Init(HI_VOID);
HI_S32 MAPI_VPROC_Client_Deinit(HI_VOID);
HI_S32 MAPI_VPROC_Init(HI_VOID);
HI_S32 MAPI_VPROC_Deinit(HI_VOID);

#endif
