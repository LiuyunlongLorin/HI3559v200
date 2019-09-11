/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vproc.c
 * @brief   server vproc module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_comm_vb.h"
#include "hi_comm_vpss.h"
#include "hi_comm_vi.h"
#include "hi_comm_region.h"
#include "hi_comm_snap.h"
#include "hi_comm_venc.h"
#include "mpi_vpss.h"
#include "mpi_vi.h"
#include "mpi_region.h"
#include "mpi_venc.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_snap.h"

#include "string.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>

#include "hi_mapi_comm_define.h"
#include "hi_mapi_vproc_define.h"
#include "mapi_vproc_inner.h"
#include "hi_mapi_log.h"
#include "mapi_comm_inner.h"
#ifdef SUPPORT_PHOTO_POST_PROCESS
#include "hi_comm_photo.h"
#include "mpi_photo.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifdef SUPPORT_PHOTO_POST_PROCESS
static HI_U64 g_u64PhotoPubPhyAddr = 0;
static HI_VOID *g_pPhotoPubVirAddr = HI_NULL;
#endif

static VPROC_VPSS_S g_astVpssCtx[HI_MAPI_VPSS_MAX_NUM];
static pthread_mutex_t g_vprocLock;

#ifdef SUPPORT_STITCH
static VPROC_STITCH_S g_astStitchCtx[HI_MAPI_STITCH_MAX_NUM];

static HI_VOID MAPI_VPROC_StitchReset(HI_VOID)
{
    HI_U32 i, j, k;

    MUTEX_LOCK(g_vprocLock);

    for (i = 0; i < HI_MAPI_STITCH_MAX_NUM; i++) {
        g_astStitchCtx[i].u32PipeNum = 0;
        g_astStitchCtx[i].bStitchStart = HI_FALSE;

        for (j = 0; j < HI_MAPI_STITCH_PORT_MAX_NUM; j++) {
            g_astStitchCtx[i].astStitchPortAttr[j].bStitchPortStart = HI_FALSE;
            g_astStitchCtx[i].astStitchPortAttr[j].bDumpYuvEnable = HI_FALSE;
            g_astStitchCtx[i].astStitchPortAttr[j].stCallbackFun.pfunVProcYUVProc = HI_NULL;
            g_astStitchCtx[i].astStitchPortAttr[j].stCallbackFun.pPrivateData = HI_NULL;
            g_astStitchCtx[i].astStitchPortAttr[j].bStitchDumpYUVStartd = HI_FALSE;
            g_astStitchCtx[i].astStitchPortAttr[j].pthreadDumpYUV = -1;

            for (k = 0; k < HI_MAPI_VPROC_OSD_MAX_NUM; k++) {
                memset(&g_astStitchCtx[i].astStitchPortAttr[j].astPortOsdAttr[k], 0, sizeof(PORT_OSD_ATTR_S));
            }
        }

        for (j = 0; j < HI_MAPI_STITCH_PIPE_MAX_NUM; j++) {
            g_astStitchCtx[i].astStitchSrc[j].VpssHdl = HI_INVALID_HANDLE;
            g_astStitchCtx[i].astStitchSrc[j].VPortHdl = HI_INVALID_HANDLE;
        }
    }

    MUTEX_UNLOCK(g_vprocLock);
    return;
}

#endif

#ifdef SUPPORT_PHOTO_POST_PROCESS
static HI_VOID VPROC_PhotoProcess_Reset(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 i = 0;

    /* Dump Data backup */
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.pstVbBlk = NULL;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.enPhotoType = VPROC_PHOTO_TYPE_BUTT;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.enAlgType = PHOTO_ALG_TYPE_BUTT;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.VencHdl = HI_INVALID_HANDLE;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.s32FrameIndex = 0;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.s32FrameCnt = 0;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64DestFrmAddr = 0;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64BNRSrcAddr = 0;

    for (i = 0; i < PHOTO_MFNR_FRAME_NUM; i++) {
        g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64YUVSrcAddr[i] = 0;
    }
}
#endif

static HI_VOID MAPI_VPROC_VpssReset(HI_VOID)
{
    HI_U32 i, j, k;

    MUTEX_LOCK(g_vprocLock);

    for (i = 0; i < HI_MAPI_VPSS_MAX_NUM; i++) {
        g_astVpssCtx[i].bVpssStart = HI_FALSE;
        g_astVpssCtx[i].s32BindVcapPipeHdl = HI_INVALID_HANDLE;
        g_astVpssCtx[i].s32BindPipeChnHdl = HI_INVALID_HANDLE;

        for (j = 0; j < HI_MAPI_VPORT_MAX_NUM; j++) {
            g_astVpssCtx[i].astVpssPortAttr[j].bVPortStart = HI_FALSE;
            g_astVpssCtx[i].astVpssPortAttr[j].bDumpYuvEnable = HI_FALSE;
            g_astVpssCtx[i].astVpssPortAttr[j].stCallbackFun.pfunVProcYUVProc = HI_NULL;
            g_astVpssCtx[i].astVpssPortAttr[j].stCallbackFun.pPrivateData = HI_NULL;
            g_astVpssCtx[i].astVpssPortAttr[j].pthreadDumpYUV = -1;
            g_astVpssCtx[i].astVpssPortAttr[j].bVpssDumpYUVStarted = HI_FALSE;
            g_astVpssCtx[i].astVpssPortAttr[j].BindVencHdl = HI_INVALID_HANDLE;
            g_astVpssCtx[i].astVpssPortAttr[j].enPhotoType = -1;

            for (k = 0; k < HI_MAPI_VPROC_OSD_MAX_NUM; k++) {
                memset(&g_astVpssCtx[i].astVpssPortAttr[j].astPortOsdAttr[k], 0, sizeof(PORT_OSD_ATTR_S));
            }
#ifdef SUPPORT_PHOTO_POST_PROCESS
            VPROC_PhotoProcess_Reset(i, j);
#endif
        }
    }

    MUTEX_UNLOCK(g_vprocLock);
    return;
}

/***ndk inner*****/
HI_S32 MAPI_VPROC_Init(HI_VOID)
{
    MUTEX_INIT_LOCK(g_vprocLock);

    MAPI_VPROC_VpssReset();
#ifdef SUPPORT_STITCH
    MAPI_VPROC_StitchReset();
#endif

    return HI_SUCCESS;
}

HI_S32 MAPI_VPROC_Deinit(HI_VOID)
{
    MAPI_VPROC_VpssReset();
#ifdef SUPPORT_STITCH
    MAPI_VPROC_StitchReset();
#endif

    MUTEX_DESTROY(g_vprocLock);
    return HI_SUCCESS;
}

HI_S32 VPROC_GetChnFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                         HI_MAPI_FRAME_DATA_S *pstYUVFrameData)
{
    VIDEO_FRAME_INFO_S stVideoFrame;
    HI_S32 s32Ret = HI_SUCCESS;

    if (bStitch) {
#ifdef SUPPORT_STITCH
        s32Ret = HI_MPI_AVS_GetChnFrame(GrpHdl, PortHdl, &stVideoFrame, 2000);
        VPROC_CHECK_RET("HI_MPI_AVS_GetChnFrame failed", s32Ret);
#else
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
    } else {
        s32Ret = HI_MPI_VPSS_GetChnFrame(GrpHdl, PortHdl, &stVideoFrame, 2000);
        VPROC_CHECK_RET("HI_MPI_VPSS_GetChnFrame failed", s32Ret);
    }

    pstYUVFrameData->enFrameDataType = HI_FRAME_DATA_TYPE_YUV;
    pstYUVFrameData->u32Width = stVideoFrame.stVFrame.u32Width;
    pstYUVFrameData->u32Height = stVideoFrame.stVFrame.u32Height;
    pstYUVFrameData->enPixelFormat = stVideoFrame.stVFrame.enPixelFormat;
    pstYUVFrameData->enCompressMode = stVideoFrame.stVFrame.enCompressMode;
    pstYUVFrameData->u64PhyAddr[0] = stVideoFrame.stVFrame.u64PhyAddr[0];
    pstYUVFrameData->u64PhyAddr[1] = stVideoFrame.stVFrame.u64PhyAddr[1];
    pstYUVFrameData->u64PhyAddr[2] = stVideoFrame.stVFrame.u64PhyAddr[2];
    pstYUVFrameData->u64VirAddr[0] = stVideoFrame.stVFrame.u64VirAddr[0];
    pstYUVFrameData->u64VirAddr[1] = stVideoFrame.stVFrame.u64VirAddr[1];
    pstYUVFrameData->u64VirAddr[2] = stVideoFrame.stVFrame.u64VirAddr[2];
    pstYUVFrameData->u32Stride[0] = stVideoFrame.stVFrame.u32Stride[0];
    pstYUVFrameData->u32Stride[1] = stVideoFrame.stVFrame.u32Stride[1];
    pstYUVFrameData->u32Stride[2] = stVideoFrame.stVFrame.u32Stride[2];
    pstYUVFrameData->u64pts = stVideoFrame.stVFrame.u64PTS;
    pstYUVFrameData->u32PoolId = stVideoFrame.u32PoolId;
    memcpy(&pstYUVFrameData->stVideoSupplement, &stVideoFrame.stVFrame.stSupplement, sizeof(VIDEO_SUPPLEMENT_S));

    return HI_SUCCESS;
}

HI_S32 VPROC_ReleaseChnFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                             HI_MAPI_FRAME_DATA_S *pstYUVFrameData)
{
    HI_S32 s32Ret;
    VIDEO_FRAME_INFO_S stVideoFrame;

    stVideoFrame.stVFrame.u32Width = pstYUVFrameData->u32Width;
    stVideoFrame.stVFrame.u32Height = pstYUVFrameData->u32Height;
    stVideoFrame.stVFrame.enPixelFormat = pstYUVFrameData->enPixelFormat;
    stVideoFrame.stVFrame.enCompressMode = pstYUVFrameData->enCompressMode;
    stVideoFrame.stVFrame.u64PhyAddr[0] = pstYUVFrameData->u64PhyAddr[0];
    stVideoFrame.stVFrame.u64PhyAddr[1] = pstYUVFrameData->u64PhyAddr[1];
    stVideoFrame.stVFrame.u64PhyAddr[2] = pstYUVFrameData->u64PhyAddr[2];
    stVideoFrame.stVFrame.u64VirAddr[0] = pstYUVFrameData->u64VirAddr[0];
    stVideoFrame.stVFrame.u64VirAddr[1] = pstYUVFrameData->u64VirAddr[1];
    stVideoFrame.stVFrame.u64VirAddr[2] = pstYUVFrameData->u64VirAddr[2];
    stVideoFrame.stVFrame.u32Stride[0] = pstYUVFrameData->u32Stride[0];
    stVideoFrame.stVFrame.u32Stride[1] = pstYUVFrameData->u32Stride[1];
    stVideoFrame.stVFrame.u32Stride[2] = pstYUVFrameData->u32Stride[2];
    stVideoFrame.stVFrame.u64PTS = pstYUVFrameData->u64pts;
    stVideoFrame.u32PoolId = pstYUVFrameData->u32PoolId;
    memcpy(&stVideoFrame.stVFrame.stSupplement, &pstYUVFrameData->stVideoSupplement, sizeof(VIDEO_SUPPLEMENT_S));

    if (bStitch) {
#ifdef SUPPORT_STITCH
        s32Ret = HI_MPI_AVS_ReleaseChnFrame(GrpHdl, PortHdl, &stVideoFrame);
        VPROC_CHECK_RET("HI_MPI_AVS_ReleaseChnFrame failed", s32Ret);
#else
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
    } else {
        s32Ret = HI_MPI_VPSS_ReleaseChnFrame(GrpHdl, PortHdl, &stVideoFrame);
        VPROC_CHECK_RET("HI_MPI_VPSS_ReleaseChnFrame failed", s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetPortFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                                  HI_MAPI_FRAME_DATA_S *pstYUVFrameData)
{
    HI_S32 s32Ret;
    VPROC_CHECK_NULL_PTR(pstYUVFrameData);

    if (bStitch) {
#ifdef SUPPORT_STITCH
        STITCH_CHECK_HANDLE(GrpHdl);
        STITCHPORT_CHECK_HANDLE(PortHdl);
#else
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
    } else {
        VPSS_CHECK_HANDLE(GrpHdl);
        VPORT_CHECK_HANDLE(PortHdl);
    }

    s32Ret = VPROC_GetChnFrame(GrpHdl, PortHdl, bStitch, pstYUVFrameData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Call VPROC_GetChnFrame fail.\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_ReleasePortFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                                      HI_MAPI_FRAME_DATA_S *pstYUVFrameData)
{
    HI_S32 s32Ret;
    VPROC_CHECK_NULL_PTR(pstYUVFrameData);

    if (bStitch) {
#ifdef SUPPORT_STITCH
        STITCH_CHECK_HANDLE(GrpHdl);
        STITCHPORT_CHECK_HANDLE(PortHdl);
#else
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
    } else {
        VPSS_CHECK_HANDLE(GrpHdl);
        VPORT_CHECK_HANDLE(PortHdl);
    }

    s32Ret = VPROC_ReleaseChnFrame(GrpHdl, PortHdl, bStitch, pstYUVFrameData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Call VPROC_ReleaseChnFrame fail.\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_InitVpss(HI_HANDLE VpssHdl, HI_MAPI_VPSS_ATTR_S *pstVpssAttr)
{
    VPSS_GRP_ATTR_S stGrpAttr;
    HI_S32 s32Ret;
    VPSS_CHECK_HANDLE(VpssHdl);
    VPROC_CHECK_NULL_PTR(pstVpssAttr);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].bVpssStart == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Vpss group %d has been initialized.\n", VpssHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    stGrpAttr.u32MaxW = pstVpssAttr->u32MaxW;
    stGrpAttr.u32MaxH = pstVpssAttr->u32MaxH;
    stGrpAttr.enPixelFormat = pstVpssAttr->enPixelFormat;
    stGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stGrpAttr.stFrameRate.s32SrcFrameRate = pstVpssAttr->stFrameRate.s32SrcFrameRate;
    stGrpAttr.stFrameRate.s32DstFrameRate = pstVpssAttr->stFrameRate.s32DstFrameRate;
    stGrpAttr.bNrEn = pstVpssAttr->bNrEn;
    memcpy(&stGrpAttr.stNrAttr, &pstVpssAttr->stNrAttr, sizeof(VPSS_NR_ATTR_S));

    s32Ret = HI_MPI_VPSS_CreateGrp(VpssHdl, &stGrpAttr);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_VPSS_CreateGrp failed", s32Ret, g_vprocLock);

    s32Ret = HI_MPI_VPSS_StartGrp(VpssHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_VPSS_StartGrp failed", s32Ret, g_vprocLock);

    g_astVpssCtx[VpssHdl].bVpssStart = HI_TRUE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_DeinitVpss(HI_HANDLE VpssHdl)
{
    HI_S32 s32Ret;
    VPSS_CHECK_HANDLE(VpssHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].bVpssStart == HI_FALSE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Vpss group %d has been Deinit.\n", VpssHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_VPSS_StopGrp(VpssHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_VPSS_StopGrp failed", s32Ret, g_vprocLock);

    s32Ret = HI_MPI_VPSS_DestroyGrp(VpssHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_VPSS_DestroyGrp failed", s32Ret, g_vprocLock);

    g_astVpssCtx[VpssHdl].bVpssStart = HI_FALSE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_BindVcap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE VpssHdl)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    VPSS_GRP_ATTR_S stGrpAttr;
    VI_CHN_ATTR_S stViChnAttr;
    VCAPPIPE_CHECK_HANDLE(VcapPipeHdl);
    VCAPCHN_CHECK_HANDLE(PipeChnHdl);
    VPSS_CHECK_HANDLE(VpssHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].bVpssStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d uninitialized.\n", VpssHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    if ((HI_HANDLE)g_astVpssCtx[VpssHdl].s32BindVcapPipeHdl == VcapPipeHdl
        && (HI_HANDLE) g_astVpssCtx[VpssHdl].s32BindPipeChnHdl == PipeChnHdl) {
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    stSrcChn.enModId = HI_ID_VI;
    stSrcChn.s32DevId = VcapPipeHdl;
    stSrcChn.s32ChnId = PipeChnHdl;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssHdl;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_SYS_Bind failed", s32Ret, g_vprocLock);

    g_astVpssCtx[VpssHdl].s32BindVcapPipeHdl = VcapPipeHdl;
    g_astVpssCtx[VpssHdl].s32BindPipeChnHdl = PipeChnHdl;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_UnBindVCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE VpssHdl)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    VCAPPIPE_CHECK_HANDLE(VcapPipeHdl);
    VCAPCHN_CHECK_HANDLE(PipeChnHdl);
    VPSS_CHECK_HANDLE(VpssHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].s32BindVcapPipeHdl == HI_INVALID_HANDLE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d has been unbind.\n", VpssHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    stSrcChn.enModId = HI_ID_VI;
    stSrcChn.s32DevId = VcapPipeHdl;
    stSrcChn.s32ChnId = PipeChnHdl;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssHdl;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_SYS_UnBind failed", s32Ret, g_vprocLock);

    g_astVpssCtx[VpssHdl].s32BindVcapPipeHdl = HI_INVALID_HANDLE;
    g_astVpssCtx[VpssHdl].s32BindVcapPipeHdl = HI_INVALID_HANDLE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetPortAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_VPORT_ATTR_S *pstVPortAttr)
{
    HI_S32 s32Ret;
    VPSS_CHN_ATTR_S stChnAttr;
    VPSS_GRP_ATTR_S stGrpAttr;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstVPortAttr);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].bVpssStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d Uninitialized.\n", VpssHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bSupportBufferShare = pstVPortAttr->bSupportBufferShare;

    MUTEX_UNLOCK(g_vprocLock);

    // chn attr set
    memset(&stChnAttr, 0, sizeof(VPSS_CHN_ATTR_S));
    s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);

    stChnAttr.u32Width = pstVPortAttr->u32Width;
    stChnAttr.u32Height = pstVPortAttr->u32Height;
    stChnAttr.enPixelFormat = pstVPortAttr->enPixelFormat;
    stChnAttr.enVideoFormat = pstVPortAttr->enVideoFormat;
    stChnAttr.enCompressMode = pstVPortAttr->enCompressMode;
    stChnAttr.enChnMode = VPSS_CHN_MODE_USER;
    stChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stChnAttr.stFrameRate.s32SrcFrameRate = pstVPortAttr->stFrameRate.s32SrcFrameRate;
    stChnAttr.stFrameRate.s32DstFrameRate = pstVPortAttr->stFrameRate.s32DstFrameRate;

    memcpy(&stChnAttr.stAspectRatio, &pstVPortAttr->stAspectRatio, sizeof(ASPECT_RATIO_S));
    s32Ret = HI_MPI_VPSS_SetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET("HI_MPI_VPSS_SetChnAttr failed", s32Ret);
#ifdef SUPPORT_PHOTO_POST_PROCESS
    s32Ret = HI_MPI_VPSS_SetChnAlign(VpssHdl, VPortHdl, PHOTO_ALIGN_LEN);
    VPROC_CHECK_RET("HI_MPI_VPSS_SetChnAlign failed", s32Ret);
#endif

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetPortAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_VPORT_ATTR_S *pstVPortAttr)
{
    HI_S32 s32Ret;
    VPSS_CHN_ATTR_S stChnAttr;
    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstVPortAttr);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].bVpssStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d Uninitialized.\n", VpssHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    pstVPortAttr->bSupportBufferShare = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bSupportBufferShare;
    MUTEX_UNLOCK(g_vprocLock);

    s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);

    pstVPortAttr->u32Width = stChnAttr.u32Width;
    pstVPortAttr->u32Height = stChnAttr.u32Height;
    pstVPortAttr->enVideoFormat = stChnAttr.enVideoFormat;
    pstVPortAttr->enPixelFormat = stChnAttr.enPixelFormat;
    pstVPortAttr->enCompressMode = stChnAttr.enCompressMode;
    pstVPortAttr->stFrameRate.s32SrcFrameRate = stChnAttr.stFrameRate.s32SrcFrameRate;
    pstVPortAttr->stFrameRate.s32DstFrameRate = stChnAttr.stFrameRate.s32DstFrameRate;

    memcpy(&pstVPortAttr->stAspectRatio, &stChnAttr.stAspectRatio, sizeof(ASPECT_RATIO_S));

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StartPort(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].bVpssStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d Uninitialized,Can not start VPort.\n", VpssHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVPortStart == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d VPort %d has been start.\n", VpssHdl, VPortHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

#if ((defined __HI3559V200__) || (defined __HI3518EV300__))
    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bSupportBufferShare == HI_TRUE) {
        s32Ret = HI_MPI_VPSS_EnableBufferShare(VpssHdl, VPortHdl);
        VPROC_CHECK_RET_UNLOCK("HI_MPI_VPSS_EnableBufferShare failed", s32Ret, g_vprocLock);
    }
#endif

    s32Ret = HI_MPI_VPSS_EnableChn(VpssHdl, VPortHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_VPSS_EnableChn failed", s32Ret, g_vprocLock);

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVPortStart = HI_TRUE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StopPort(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].bVpssStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d Uninitialized.\n", VpssHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVPortStart == HI_FALSE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d VPort %d has been stop.\n", VpssHdl, VPortHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

#if ((defined __HI3559V200__) || (defined __HI3518EV300__))
    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bSupportBufferShare == HI_TRUE) {
        s32Ret = HI_MPI_VPSS_DisableBufferShare(VpssHdl, VPortHdl);
        VPROC_CHECK_RET_UNLOCK("HI_MPI_VPSS_DisableBufferShare failed", s32Ret, g_vprocLock);
    }
#endif

    s32Ret = HI_MPI_VPSS_DisableChn(VpssHdl, VPortHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_VPSS_DisableChn failed", s32Ret, g_vprocLock);

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVPortStart = HI_FALSE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetPortAttrEx(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_VPROC_CMD_E enCMD,
                                   HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pAttr);
    MAPI_UNUSED(u32Len);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].bVpssStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d Uninitialized.\n", VpssHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    MUTEX_UNLOCK(g_vprocLock);

    switch (enCMD) {
        case HI_VPROC_CMD_PortMirror: {
            VPSS_CHN_ATTR_S stChnAttr;
            HI_MAPI_VPROC_MIRROR_ATTR_S *pMirrorAttr = (HI_MAPI_VPROC_MIRROR_ATTR_S *)pAttr;

            s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
            VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);

            stChnAttr.bMirror = pMirrorAttr->bEnable;
            s32Ret = HI_MPI_VPSS_SetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
            VPROC_CHECK_RET("HI_MPI_VPSS_SetChnAttr failed", s32Ret);
        }
        break;

        case HI_VPROC_CMD_PortFlip: {
            VPSS_CHN_ATTR_S stChnAttr;
            HI_MAPI_VPROC_FLIP_ATTR_S *pFlipAttr = (HI_MAPI_VPROC_FLIP_ATTR_S *)pAttr;

            s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
            VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);

            stChnAttr.bFlip = pFlipAttr->bEnable;
            s32Ret = HI_MPI_VPSS_SetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
            VPROC_CHECK_RET("HI_MPI_VPSS_SetChnAttr failed", s32Ret);
        }
        break;

        case HI_VPROC_CMD_PortRotate: {
            ROTATION_E *penRotate_mapi = (ROTATION_E *)pAttr;
            ROTATION_E enRotate = *penRotate_mapi;
            s32Ret = HI_MPI_VPSS_SetChnRotation(VpssHdl, VPortHdl, enRotate);
            VPROC_CHECK_RET("HI_MPI_VPSS_SetChnRotation failed", s32Ret);
        }
        break;

        case HI_VPROC_CMD_PortBufWrap: {
            /* the function be support in the future 56a and 59a version */
#if defined(__HI3518EV300__) || defined(__HI3559V200__)

            VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap = (VPSS_CHN_BUF_WRAP_S *)pAttr;
            VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap;

            if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVPortStart != HI_TRUE) {
                memcpy(&stVpssChnBufWrap, pstVpssChnBufWrap, sizeof(VPSS_CHN_BUF_WRAP_S));

                s32Ret = HI_MPI_VPSS_SetChnBufWrapAttr(VpssHdl, VPortHdl, &stVpssChnBufWrap);

                VPROC_CHECK_RET("HI_MPI_VPSS_SetChnBufWrapAttr failed", s32Ret);
            } else {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VPortHdl is started.\n");
                return HI_MAPI_VPROC_ENOT_SUPPORT;
            }
#else
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MPI not support SetChnBufWrapAttr.\n");
            return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
        }
        break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Do not support cmd %d.\n", enCMD);
            return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetPortAttrEx(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_VPROC_CMD_E enCMD,
                                   HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MAPI_UNUSED(u32Len);

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pAttr);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].bVpssStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d Uninitialized.\n", VpssHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    MUTEX_UNLOCK(g_vprocLock);

    switch (enCMD) {
        case HI_VPROC_CMD_PortMirror: {
            VPSS_CHN_ATTR_S stChnAttr;
            HI_MAPI_VPROC_MIRROR_ATTR_S *pMirrorAttr = (HI_MAPI_VPROC_MIRROR_ATTR_S *)pAttr;
            s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
            VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);

            pMirrorAttr->bEnable = stChnAttr.bMirror;
        }
        break;

        case HI_VPROC_CMD_PortFlip: {
            VPSS_CHN_ATTR_S stChnAttr;
            HI_MAPI_VPROC_FLIP_ATTR_S *pFlipAttr = (HI_MAPI_VPROC_FLIP_ATTR_S *)pAttr;
            s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
            VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);

            pFlipAttr->bEnable = stChnAttr.bFlip;
        }
        break;

        case HI_VPROC_CMD_PortRotate: {
            ROTATION_E enRotate;
            s32Ret = HI_MPI_VPSS_GetChnRotation(VpssHdl, VPortHdl, &enRotate);
            VPROC_CHECK_RET("HI_MPI_VPSS_GetChnRotation failed", s32Ret);

            ROTATION_E *penRotate_mapi = (ROTATION_E *)pAttr;
            *penRotate_mapi = enRotate;
        }
        break;

        case HI_VPROC_CMD_PortBufWrap: {
            /* the function be support in the future 56a and 59a version */
#if defined(__HI3518EV300__) || defined(__HI3559V200__)

            VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap = (VPSS_CHN_BUF_WRAP_S *)pAttr;
            ;
            VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap;

            s32Ret = HI_MPI_VPSS_GetChnBufWrapAttr(VpssHdl, VPortHdl, &stVpssChnBufWrap);

            VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);
            memcpy((VPSS_CHN_BUF_WRAP_S *)pAttr, &stVpssChnBufWrap, sizeof(VPSS_CHN_BUF_WRAP_S));

#else
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MPI not support GetChnBufWrapAttr.\n");
            return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
        }
        break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Do not support cmd %d.\n", enCMD);
            return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    return HI_SUCCESS;
}

#ifdef SUPPORT_STITCH
static HI_S32 VPROC_CheckStitchAttr(HI_MAPI_STITCH_ATTR_S *pstStitchAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i;
    HI_HANDLE VpssHdl;
    HI_HANDLE VPortHdl;
    VPSS_CHN_ATTR_S stChnAttr;

    for (i = 0; i < pstStitchAttr->u32PipeNum; i++) {
        VpssHdl = pstStitchAttr->astStitchSrc[i].VpssHdl;
        VPortHdl = pstStitchAttr->astStitchSrc[i].VPortHdl;
        VPSS_CHECK_HANDLE(VpssHdl);
        VPORT_CHECK_HANDLE(VPortHdl);
        s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
        VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);

        if (pstStitchAttr->enMode == AVS_MODE_BLEND) {
            if (VIDEO_FORMAT_TILE_16x8 != stChnAttr.enVideoFormat) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "In blend mode , the input video format must be 16BPP.\n");
                return HI_MAPI_VPROC_ENOT_SUPPORT;
            }

        } else if ((pstStitchAttr->enMode == AVS_MODE_NOBLEND_VER) ||
                   (pstStitchAttr->enMode == AVS_MODE_NOBLEND_HOR) || (pstStitchAttr->enMode == AVS_MODE_NOBLEND_QR)) {
            if (stChnAttr.enVideoFormat != VIDEO_FORMAT_LINEAR) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "In no blend mode , the input video format must be linear.\n");
                return HI_MAPI_VPROC_ENOT_SUPPORT;
            }
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Do not support mode %d.\n", pstStitchAttr->enMode);
            return HI_MAPI_VPROC_ENOT_SUPPORT;
        }
    }

    return HI_SUCCESS;
}

/* Stitch */
HI_S32 HI_MAPI_VPROC_CreateStitch(HI_HANDLE StitchHdl, HI_MAPI_STITCH_ATTR_S *pstStitchAttr)
{
    HI_S32 s32Ret;
    HI_U32 i;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    AVS_GRP_ATTR_S stAVSGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    STITCH_CHECK_HANDLE(StitchHdl);
    VPROC_CHECK_NULL_PTR(pstStitchAttr);

    MUTEX_LOCK(g_vprocLock);

    if (g_astStitchCtx[StitchHdl].bStitchStart == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d has been created.\n", StitchHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    s32Ret = VPROC_CheckStitchAttr(pstStitchAttr);
    VPROC_CHECK_RET_UNLOCK("Check StitchAttr failed", s32Ret, g_vprocLock);

    stAVSGrpAttr.enMode = pstStitchAttr->enMode;
    stAVSGrpAttr.bSyncPipe = pstStitchAttr->bSyncPipe;
    stAVSGrpAttr.u32PipeNum = pstStitchAttr->u32PipeNum;
    stAVSGrpAttr.stFrameRate.s32SrcFrameRate = pstStitchAttr->stFrameRate.s32SrcFrameRate;
    stAVSGrpAttr.stFrameRate.s32DstFrameRate = pstStitchAttr->stFrameRate.s32DstFrameRate;
    memcpy(&stAVSGrpAttr.stLUT, &pstStitchAttr->stLUT, sizeof(AVS_LUT_S));
    memcpy(&stAVSGrpAttr.stGainAttr, &pstStitchAttr->stGainAttr, sizeof(AVS_GAIN_ATTR_S));
    memcpy(&stAVSGrpAttr.stOutAttr, &pstStitchAttr->stOutAttr, sizeof(AVS_OUTPUT_ATTR_S));

    s32Ret = HI_MPI_AVS_CreateGrp(StitchHdl, &stAVSGrpAttr);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_AVS_CreateGrp failed", s32Ret, g_vprocLock);

    s32Ret = HI_MPI_AVS_StartGrp(StitchHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_AVS_StartGrp failed", s32Ret, g_vprocLock);

    g_astStitchCtx[StitchHdl].bStitchStart = HI_TRUE;
    g_astStitchCtx[StitchHdl].u32PipeNum = pstStitchAttr->u32PipeNum;

    for (i = 0; i < pstStitchAttr->u32PipeNum; i++) {
        stSrcChn.enModId = HI_ID_VPSS;
        stSrcChn.s32DevId = pstStitchAttr->astStitchSrc[i].VpssHdl;
        stSrcChn.s32ChnId = pstStitchAttr->astStitchSrc[i].VPortHdl;

        stDestChn.enModId = HI_ID_AVS;
        stDestChn.s32DevId = StitchHdl;
        stDestChn.s32ChnId = i;

        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        VPROC_CHECK_RET_UNLOCK("HI_MPI_SYS_Bind failed", s32Ret, g_vprocLock);

        g_astStitchCtx[StitchHdl].astStitchSrc[i].VpssHdl = pstStitchAttr->astStitchSrc[i].VpssHdl;
        g_astStitchCtx[StitchHdl].astStitchSrc[i].VPortHdl = pstStitchAttr->astStitchSrc[i].VPortHdl;
    }

    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_DestroyStitch(HI_HANDLE StitchHdl)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    STITCH_CHECK_HANDLE(StitchHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astStitchCtx[StitchHdl].bStitchStart == HI_FALSE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d has been Destroy.\n", StitchHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    for (i = 0; i < g_astStitchCtx[StitchHdl].u32PipeNum; i++) {
        stSrcChn.enModId = HI_ID_VPSS;
        stSrcChn.s32DevId = g_astStitchCtx[StitchHdl].astStitchSrc[i].VpssHdl;
        stSrcChn.s32ChnId = g_astStitchCtx[StitchHdl].astStitchSrc[i].VPortHdl;

        stDestChn.enModId = HI_ID_AVS;
        stDestChn.s32DevId = StitchHdl;
        stDestChn.s32ChnId = i;

        s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        VPROC_CHECK_RET_UNLOCK("HI_MPI_SYS_UnBind failed", s32Ret, g_vprocLock);

        g_astStitchCtx[StitchHdl].astStitchSrc[i].VpssHdl = -1;
        g_astStitchCtx[StitchHdl].astStitchSrc[i].VPortHdl = -1;
    }

    s32Ret = HI_MPI_AVS_StopGrp(StitchHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_AVS_StopGrp failed", s32Ret, g_vprocLock);

    s32Ret = HI_MPI_AVS_DestroyGrp(StitchHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_AVS_DestroyGrp failed", s32Ret, g_vprocLock);

    g_astStitchCtx[StitchHdl].bStitchStart = HI_FALSE;
    g_astStitchCtx[StitchHdl].u32PipeNum = 0;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_SetStitchAttr(HI_HANDLE StitchHdl, HI_MAPI_STITCH_ATTR_S *pstStitchAttr)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    AVS_GRP_ATTR_S stAVSGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    STITCH_CHECK_HANDLE(StitchHdl);
    VPROC_CHECK_NULL_PTR(pstStitchAttr);

    MUTEX_LOCK(g_vprocLock);

    if (g_astStitchCtx[StitchHdl].bStitchStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d not created.\n", StitchHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    MUTEX_UNLOCK(g_vprocLock);

    s32Ret = HI_MPI_AVS_GetGrpAttr(StitchHdl, &stAVSGrpAttr);
    VPROC_CHECK_RET("HI_MPI_AVS_GetGrpAttr failed", s32Ret);

    stAVSGrpAttr.bSyncPipe = pstStitchAttr->bSyncPipe;
    stAVSGrpAttr.stFrameRate.s32SrcFrameRate = pstStitchAttr->stFrameRate.s32SrcFrameRate;
    stAVSGrpAttr.stFrameRate.s32DstFrameRate = pstStitchAttr->stFrameRate.s32DstFrameRate;
    memcpy(&stAVSGrpAttr.stLUT, &pstStitchAttr->stLUT, sizeof(AVS_LUT_S));
    memcpy(&stAVSGrpAttr.stGainAttr, &pstStitchAttr->stGainAttr, sizeof(AVS_GAIN_ATTR_S));
    memcpy(&stAVSGrpAttr.stOutAttr, &pstStitchAttr->stOutAttr, sizeof(AVS_OUTPUT_ATTR_S));

    s32Ret = HI_MPI_AVS_SetGrpAttr(StitchHdl, &stAVSGrpAttr);
    VPROC_CHECK_RET("HI_MPI_AVS_SetGrpAttr failed", s32Ret);

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_GetStitchAttr(HI_HANDLE StitchHdl, HI_MAPI_STITCH_ATTR_S *pstStitchAttr)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    AVS_GRP_ATTR_S stAVSGrpAttr;
    STITCH_CHECK_HANDLE(StitchHdl);
    VPROC_CHECK_NULL_PTR(pstStitchAttr);

    MUTEX_LOCK(g_vprocLock);

    if (g_astStitchCtx[StitchHdl].bStitchStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d not created.\n", StitchHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    s32Ret = HI_MPI_AVS_GetGrpAttr(StitchHdl, &stAVSGrpAttr);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_AVS_GetGrpAttr failed", s32Ret, g_vprocLock);

    pstStitchAttr->enMode = stAVSGrpAttr.enMode;
    pstStitchAttr->u32PipeNum = stAVSGrpAttr.u32PipeNum;
    pstStitchAttr->bSyncPipe = stAVSGrpAttr.bSyncPipe;
    pstStitchAttr->stFrameRate.s32SrcFrameRate = stAVSGrpAttr.stFrameRate.s32SrcFrameRate;
    pstStitchAttr->stFrameRate.s32DstFrameRate = stAVSGrpAttr.stFrameRate.s32DstFrameRate;

    memcpy(&pstStitchAttr->stLUT, &stAVSGrpAttr.stLUT, sizeof(AVS_LUT_S));
    memcpy(&pstStitchAttr->stGainAttr, &stAVSGrpAttr.stGainAttr, sizeof(AVS_GAIN_ATTR_S));
    memcpy(&pstStitchAttr->stOutAttr, &stAVSGrpAttr.stOutAttr, sizeof(AVS_OUTPUT_ATTR_S));
    memcpy(pstStitchAttr->astStitchSrc, g_astStitchCtx[StitchHdl].astStitchSrc,
           sizeof(HI_VPROC_STITCH_SRC_S) * HI_MAPI_STITCH_PIPE_MAX_NUM);
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

/* StitchPort */
HI_S32 HI_MAPI_VPROC_SetStitchPortAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                       HI_MAPI_STITCH_PORT_ATTR_S *pstStitchPortAttr)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    AVS_CHN_ATTR_S stAVSChnAttr;
    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_CHECK_NULL_PTR(pstStitchPortAttr);

    MUTEX_LOCK(g_vprocLock);

    if (g_astStitchCtx[StitchHdl].bStitchStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d not created.\n", StitchHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    MUTEX_UNLOCK(g_vprocLock);

    stAVSChnAttr.u32Width = pstStitchPortAttr->u32Width;
    stAVSChnAttr.u32Height = pstStitchPortAttr->u32Height;
    stAVSChnAttr.enCompressMode = pstStitchPortAttr->enCompressMode;
    stAVSChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stAVSChnAttr.u32Depth = 0;
    s32Ret = HI_MPI_AVS_SetChnAttr(StitchHdl, StitchPortHdl, &stAVSChnAttr);
    VPROC_CHECK_RET("HI_MPI_AVS_SetChnAttr failed", s32Ret);

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_GetStitchPortAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                       HI_MAPI_STITCH_PORT_ATTR_S *pstStitchPortAttr)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    AVS_CHN_ATTR_S stAVSChnAttr;
    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_CHECK_NULL_PTR(pstStitchPortAttr);

    MUTEX_LOCK(g_vprocLock);

    if (g_astStitchCtx[StitchHdl].bStitchStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d not created.\n", StitchHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    MUTEX_UNLOCK(g_vprocLock);

    s32Ret = HI_MPI_AVS_GetChnAttr(StitchHdl, StitchPortHdl, &stAVSChnAttr);
    VPROC_CHECK_RET("HI_MPI_AVS_GetChnAttr failed", s32Ret);

    pstStitchPortAttr->u32Width = stAVSChnAttr.u32Width;
    pstStitchPortAttr->u32Height = stAVSChnAttr.u32Height;
    pstStitchPortAttr->enCompressMode = stAVSChnAttr.enCompressMode;

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_StartStitchPort(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astStitchCtx[StitchHdl].bStitchStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d not created.\n", StitchHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bStitchPortStart == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d StitchPort %d has been start.\n", StitchHdl, StitchPortHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_AVS_EnableChn(StitchHdl, StitchPortHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_AVS_EnableChn failed", s32Ret, g_vprocLock);

    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bStitchPortStart = HI_TRUE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_StopStitchPort(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astStitchCtx[StitchHdl].bStitchStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d not created.\n", StitchHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bStitchPortStart == HI_FALSE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d StitchPort %d has been stop.\n", StitchHdl, StitchPortHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_AVS_DisableChn(StitchHdl, StitchPortHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_AVS_DisableChn failed", s32Ret, g_vprocLock);

    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bStitchPortStart = HI_FALSE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

#endif

#ifdef SUPPORT_PHOTO_POST_PROCESS /* photo */
static HI_S32 VPROC_InitPhotoParm(HI_MAPI_PHOTO_TYPE_E enPhotoType, VPSS_CHN_ATTR_S *pstChnAttr,
                                  PHOTO_ALG_TYPE_E *penAlgType, HI_S32 *ps32FrameNum, HI_U32 *pu32PhotoMemSize)
{
    switch (enPhotoType) {
        case VPROC_PHOTO_TYPE_HDR:
            *ps32FrameNum = PHOTO_HDR_FRAME_NUM;
            *penAlgType = PHOTO_ALG_TYPE_HDR;
            *pu32PhotoMemSize = HDR_GetPublicMemSize(pstChnAttr->u32Width, pstChnAttr->u32Height);
            break;

        case VPROC_PHOTO_TYPE_MFNR:
            *ps32FrameNum = PHOTO_MFNR_FRAME_NUM;
            *penAlgType = PHOTO_ALG_TYPE_MFNR;
            *pu32PhotoMemSize = MFNR_GetPublicMemSize(pstChnAttr->u32Width, pstChnAttr->u32Height);
            break;

        case VPROC_PHOTO_TYPE_SFNR:
            *ps32FrameNum = 1;
            *penAlgType = PHOTO_ALG_TYPE_SFNR;
            *pu32PhotoMemSize = SFNR_GetPublicMemSize(pstChnAttr->u32Width, pstChnAttr->u32Height);
            break;

        case VPROC_PHOTO_TYPE_DE:
            *ps32FrameNum = 1;
            *penAlgType = PHOTO_ALG_TYPE_DE;
            *pu32PhotoMemSize = DE_GetPublicMemSize(pstChnAttr->u32Width, pstChnAttr->u32Height);
            break;

        case VPROC_PHOTO_TYPE_MFNR_DE:
            *ps32FrameNum = PHOTO_MFNR_FRAME_NUM;
            *penAlgType = PHOTO_ALG_TYPE_MFNR;
            *pu32PhotoMemSize = MFNR_GetPublicMemSize(pstChnAttr->u32Width, pstChnAttr->u32Height);
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Do not support Photo type %d.\n", enPhotoType);
            return HI_MAPI_VPROC_EILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static HI_S32 VPROC_GetJpegChn(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE *pVEncHdl)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_BIND_DEST_S stDesChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssHdl;
    stSrcChn.s32ChnId = VPortHdl;
    s32Ret = HI_MPI_SYS_GetBindbySrc(&stSrcChn, &stDesChn);
    VPROC_CHECK_RET("HI_MPI_SYS_GetBindbySrc failed", s32Ret);

    if (stDesChn.u32Num > 1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "multi target bind to vpss chn.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    } else if (stDesChn.u32Num < 1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "vproc and venc are not binded yet.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    if (stDesChn.astMppChn[0].enModId != HI_ID_VENC) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "vpss chn binder %d is not venc.\n", stDesChn.astMppChn[0].enModId);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    *pVEncHdl = stDesChn.astMppChn[0].s32ChnId;

    return HI_SUCCESS;
}

static HI_S32 VPROC_CutOff_Venc(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssHdl;
    stSrcChn.s32ChnId = VPortHdl;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VencHdl;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    VPROC_CHECK_RET("HI_MPI_SYS_UnBind failed", s32Ret);

    return HI_SUCCESS;
}

static HI_S32 VPROC_Reconnect_Venc(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    VPSS_CHN_ATTR_S stChnAttr;

    s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);

    stChnAttr.u32Depth = 0;

    s32Ret = HI_MPI_VPSS_SetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET("HI_MPI_VPSS_SetChnAttr failed", s32Ret);

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssHdl;
    stSrcChn.s32ChnId = VPortHdl;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VencHdl;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    VPROC_CHECK_RET("HI_MPI_SYS_Bind failed", s32Ret);

    return HI_SUCCESS;
}

static inline HI_S32 VPROC_CalcFrameSize(VIDEO_FRAME_INFO_S *pstFrameInfo, HI_U32 *pu32Size,
                                         HI_U32 *pu32LumaSize, HI_U32 *pu32ChrmSize)
{
    HI_U32 u32Size, u32LumaSize, u32ChrmSize;
    VIDEO_FRAME_S *pstFrame = &pstFrameInfo->stVFrame;

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == pstFrameInfo->stVFrame.enPixelFormat) {
        u32Size = (pstFrame->u32Stride[0] * pstFrame->u32Height) * 3 / 2;
        u32LumaSize = (pstFrame->u32Stride[0] * pstFrame->u32Height);
        u32ChrmSize = u32LumaSize / 4;
    } else {
        printf("not support pixelformat: %d\n", pstFrameInfo->stVFrame.enPixelFormat);
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    if (pstFrameInfo->stVFrame.enCompressMode != COMPRESS_MODE_NONE) {
        printf("not support CompressMode: %d\n", pstFrameInfo->stVFrame.enCompressMode);
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    if (pu32Size != NULL) {
        *pu32Size = u32Size;
    }

    if (pu32LumaSize != NULL) {
        *pu32LumaSize = u32LumaSize;
    }

    if (pu32ChrmSize != NULL) {
        *pu32ChrmSize = u32ChrmSize;
    }

    return HI_SUCCESS;
}

static HI_S32 VPROC_GetVB(VB_POOL hPool, VIDEO_FRAME_INFO_S *pstFrameInfo, VB_BLK *pstVbBlk)
{
    VB_BLK VbBlk;
    HI_U32 u32PhyAddr, u32Stride;
    HI_U32 u32Size, u32LumaSize, u32ChrmSize;
    HI_S32 s32Ret;

    s32Ret = VPROC_CalcFrameSize(pstFrameInfo, &u32Size, &u32LumaSize, &u32ChrmSize);
    VPROC_CHECK_RET("CalcFrameSize failed", s32Ret);

    VbBlk = HI_MPI_VB_GetBlock(hPool, u32Size, HI_NULL);

    if (VbBlk == VB_INVALID_HANDLE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_VB_GetBlock failed! size:%d\n", u32Size);
        return HI_MAPI_VPROC_ENOBUF;
    }

    *pstVbBlk = VbBlk;

    u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(VbBlk);

    if (u32PhyAddr == 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_VB_Handle2PhysAddr failed!\n");
        HI_MPI_VB_ReleaseBlock(VbBlk);
        return HI_MAPI_VPROC_ENOMEM;
    }

    pstFrameInfo->u32PoolId = HI_MPI_VB_Handle2PoolId(VbBlk);

    if (pstFrameInfo->u32PoolId == VB_INVALID_POOLID) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "u32PoolId err!\n");
        HI_MPI_VB_ReleaseBlock(VbBlk);
        return HI_MAPI_VPROC_ENOMEM;
    }

    pstFrameInfo->stVFrame.u64PhyAddr[0] = u32PhyAddr;
    pstFrameInfo->stVFrame.u64PhyAddr[1] = pstFrameInfo->stVFrame.u64PhyAddr[0] + u32LumaSize;
    pstFrameInfo->stVFrame.u64PhyAddr[2] = pstFrameInfo->stVFrame.u64PhyAddr[1] + u32ChrmSize;

    pstFrameInfo->stVFrame.u64VirAddr[0] = 0;
    pstFrameInfo->stVFrame.u64VirAddr[1] = 0;
    pstFrameInfo->stVFrame.u64VirAddr[2] = 0;

    return HI_SUCCESS;
}

static HI_S32 VPROC_GetBNRRaw(HI_HANDLE VpssHdl, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    HI_S32 s32Ret;
    MPP_CHN_S stDestChn;
    MPP_CHN_S stSrcChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssHdl;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn);
    VPROC_CHECK_RET("HI_MPI_SYS_GetBindbyDest failed", s32Ret);

    s32Ret = HI_MPI_VI_GetPipeBNRRaw(stSrcChn.s32DevId, pstFrameInfo, 5000);
    VPROC_CHECK_RET("HI_MPI_VI_GetPipeBNRRaw failed", s32Ret);

    return HI_SUCCESS;
}

static HI_S32 VPROC_ReleaseBNRRaw(HI_HANDLE VpssHdl, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    HI_S32 s32Ret;
    BNR_DUMP_ATTR_S stBNRAttr;
    MPP_CHN_S stDestChn;
    MPP_CHN_S stSrcChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssHdl;
    stDestChn.s32ChnId = 0;
    s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn);
    VPROC_CHECK_RET("HI_MPI_SYS_GetBindbyDest failed", s32Ret);

    s32Ret = HI_MPI_VI_ReleasePipeBNRRaw(stSrcChn.s32DevId, pstFrameInfo);
    VPROC_CHECK_RET("HI_MPI_SNAP_ReleaseBNRRaw failed", s32Ret);

    return HI_SUCCESS;
}

static HI_S32 VPROC_GetFrameISO(VIDEO_FRAME_INFO_S *pstFrameInfo, HI_U32 *pu32ISO)
{
    HI_S32 s32Ret;
    HI_U64 u64IspInfoPhyAddr;
    ISP_FRAME_INFO_S *pstIspFrameInfo = HI_NULL;

    u64IspInfoPhyAddr = pstFrameInfo->stVFrame.stSupplement.u64IspInfoPhyAddr;

#ifndef __HuaweiLite__
    /* the branch is used in single linux version */
    pstIspFrameInfo = HI_MPI_SYS_Mmap(u64IspInfoPhyAddr, sizeof(ISP_FRAME_INFO_S));

    if (pstIspFrameInfo == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SYS_Mmap failed.\n");
        return HI_MAPI_VPROC_EILLEGAL_PARAM;
    }

    *pu32ISO = pstIspFrameInfo->u32ISO;
    s32Ret = HI_MPI_SYS_Munmap(pstIspFrameInfo, sizeof(ISP_FRAME_INFO_S));
    VPROC_CHECK_RET("HI_MPI_SYS_Munmap failed", s32Ret);
#else
    /* phy addr equals virt addr in Litos system */
    pstIspFrameInfo = (ISP_FRAME_INFO_S *)(HI_UL)u64IspInfoPhyAddr;
    *pu32ISO = pstIspFrameInfo->u32ISO;
#endif

    return HI_SUCCESS;
}

static HI_S32 VPROC_CopyMemByPhyAddr(HI_U64 u64DesPhyAddr, HI_U64 u64SrcPhyAddr, HI_S32 s32Size)
{
    HI_S32 s32Ret;

#ifndef __HuaweiLite__
    HI_VOID *pSrcVirAddr = HI_NULL;
    HI_VOID *pDesVirAddr = HI_NULL;

    /* the branch is used in single linux version */
    pSrcVirAddr = HI_MPI_SYS_Mmap(u64SrcPhyAddr, s32Size);

    if (pSrcVirAddr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SYS_Mmap failed.\n");
        return HI_MAPI_VPROC_EILLEGAL_PARAM;
    }

    pDesVirAddr = HI_MPI_SYS_Mmap(u64DesPhyAddr, s32Size);

    if (pDesVirAddr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SYS_Mmap failed.\n");
        return HI_MAPI_VPROC_EILLEGAL_PARAM;
    }

    memcpy(pDesVirAddr, pSrcVirAddr, s32Size);

    s32Ret = HI_MPI_SYS_Munmap(pSrcVirAddr, s32Size);
    VPROC_CHECK_RET("HI_MPI_SYS_Munmap failed", s32Ret);

    s32Ret = HI_MPI_SYS_Munmap(pDesVirAddr, s32Size);
    VPROC_CHECK_RET("HI_MPI_SYS_Munmap failed", s32Ret);

#else
    /* phy addr equals virt addr in Litos system */
    memcpy ((HI_VOID *)(HI_UL)u64DesPhyAddr, (HI_VOID *)(HI_UL)u64SrcPhyAddr, s32Size);
#endif
    return HI_SUCCESS;
}

static HI_S32 VPROC_CopySupplement(VIDEO_FRAME_S *pstDesVFrame, VIDEO_FRAME_S *pstSrcVFrame)
{
    HI_S32 s32Ret;
    VB_SUPPLEMENT_CONFIG_S stSupplementConf;
    HI_VOID *pSrcVirAddr = HI_NULL;
    HI_VOID *pDesVirAddr = HI_NULL;

    s32Ret = HI_MPI_VB_GetSupplementConfig(&stSupplementConf);
    VPROC_CHECK_RET("HI_MPI_VB_GetSupplementConfig failed", s32Ret);

    if (stSupplementConf.u32SupplementConfig & VB_SUPPLEMENT_JPEG_MASK) {
        VPROC_CopyMemByPhyAddr(pstDesVFrame->stSupplement.u64JpegDCFPhyAddr,
                               pstSrcVFrame->stSupplement.u64JpegDCFPhyAddr, sizeof(JPEG_DCF_S));
    }

    if (stSupplementConf.u32SupplementConfig & VB_SUPPLEMENT_ISPINFO_MASK) {
        VPROC_CopyMemByPhyAddr(pstDesVFrame->stSupplement.u64IspInfoPhyAddr,
                               pstSrcVFrame->stSupplement.u64IspInfoPhyAddr, sizeof(ISP_FRAME_INFO_S));
    }

    if (stSupplementConf.u32SupplementConfig & VB_SUPPLEMENT_DNG_MASK) {
        VPROC_CopyMemByPhyAddr(pstDesVFrame->stSupplement.u64FrameDNGPhyAddr,
                               pstSrcVFrame->stSupplement.u64FrameDNGPhyAddr, sizeof(DNG_IMAGE_DYNAMIC_INFO_S));
    }

    return HI_SUCCESS;
}

static HI_VOID VPROC_Photo_FrameInfoToData(const VIDEO_FRAME_INFO_S *pstFrameInfo,
                                           HI_MAPI_FRAME_DATA_S *pstFrameData, HI_BOOL bYUVFrame)
{
    HI_S32 s32Looper = 0;

    memcpy(&pstFrameData->stVideoSupplement, &pstFrameInfo->stVFrame.stSupplement, sizeof(VIDEO_SUPPLEMENT_S));
    pstFrameData->enCompressMode = pstFrameInfo->stVFrame.enCompressMode;

    if (bYUVFrame) {
        pstFrameData->enFrameDataType = HI_FRAME_DATA_TYPE_YUV;
    } else {
        pstFrameData->enFrameDataType = HI_FRAME_DATA_TYPE_RAW;
    }

    pstFrameData->enPixelFormat = pstFrameInfo->stVFrame.enPixelFormat;
    pstFrameData->u32Height = pstFrameInfo->stVFrame.u32Height;
    pstFrameData->u32Width = pstFrameInfo->stVFrame.u32Width;
    pstFrameData->u32PoolId = pstFrameInfo->u32PoolId;
    pstFrameData->u64pts = pstFrameInfo->stVFrame.u64PTS;

    for (s32Looper = 0; s32Looper < FRAME_DATA_ADDR_NUM; s32Looper++) {
        pstFrameData->u64PhyAddr[s32Looper] = pstFrameInfo->stVFrame.u64PhyAddr[s32Looper];
        pstFrameData->u32Stride[s32Looper] = pstFrameInfo->stVFrame.u32Stride[s32Looper];
        pstFrameData->u64VirAddr[s32Looper] = pstFrameInfo->stVFrame.u64VirAddr[s32Looper];
    }

    return;
}

static HI_S32 VPROC_PhotoProcess_Dump(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_BOOL bBNRRaw,
                                      VIDEO_FRAME_INFO_S *pstVideoFrame)
{
    HI_U32 u32Size = 0;
    HI_VOID *VirAddr = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_FRAME_DATA_S stFrameData;

    VPROC_CHECK_NULL_PTR(pstVideoFrame);

    memset(&stFrameData, 0x00, sizeof(HI_MAPI_FRAME_DATA_S));

    if (!bBNRRaw) {
        VPROC_Photo_FrameInfoToData(pstVideoFrame, &stFrameData, HI_TRUE);
        /* dump YUV */
        if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == stFrameData.enPixelFormat) {
            u32Size = (stFrameData.u32Stride[0] * stFrameData.u32Height) * 3 / 2;
        } else if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == stFrameData.enPixelFormat) {
            u32Size = (stFrameData.u32Stride[0] * stFrameData.u32Height) * 2;
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "PixelFormat error! current pixelFormat is %d\n",
                           stFrameData.enPixelFormat);
            return HI_FAILURE;
        }
        VirAddr = HI_MPI_SYS_Mmap(stFrameData.u64PhyAddr[0], u32Size);
        if (VirAddr == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Mmap YUV PhyAddr fail!\n");
            return HI_FAILURE;
        }
        stFrameData.u64VirAddr[0] = (HI_U64)(HI_UL)VirAddr;
        if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pfunVProcYUVProc != NULL) {
            g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pfunVProcYUVProc(VpssHdl, VPortHdl,
                &stFrameData, g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pPrivateData);
        }

        s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)(HI_UL)stFrameData.u64PhyAddr[0], u32Size);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Munmap YUV Addr fail s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
    } else {
        VPROC_Photo_FrameInfoToData(pstVideoFrame, &stFrameData, HI_FALSE);
        /* dump BNR raw */
        u32Size = (stFrameData.u32Stride[0] * stFrameData.u32Height);
        stFrameData.u64VirAddr[0] = (HI_U64)(HI_UL)HI_MPI_SYS_Mmap(stFrameData.u64PhyAddr[0], u32Size);
        if (stFrameData.u64VirAddr[0] == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Mmap BNR VirAddr fail!\n");
            return HI_FAILURE;
        }
        stFrameData.stVideoSupplement.pFrameDNGVirAddr = HI_MPI_SYS_Mmap(
            stFrameData.stVideoSupplement.u64FrameDNGPhyAddr, sizeof(DNG_IMAGE_DYNAMIC_INFO_S));
        if (stFrameData.stVideoSupplement.pFrameDNGVirAddr == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Mmap BNR DNG Addr fail!\n");
            return HI_FAILURE;
        }
        stFrameData.stVideoSupplement.pJpegDCFVirAddr = HI_MPI_SYS_Mmap(stFrameData.stVideoSupplement.u64JpegDCFPhyAddr,
                                                                        sizeof(JPEG_DCF_S));
        if (stFrameData.stVideoSupplement.pJpegDCFVirAddr == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Mmap BNR DCF Addr fail!\n");
            return HI_FAILURE;
        }

        if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stBNRRawCB.pfunVProcBNRProc != NULL) {
            g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stBNRRawCB.pfunVProcBNRProc(VpssHdl, VPortHdl,
                &stFrameData, g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stBNRRawCB.pPrivateData);
        }

        s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)(HI_UL)stFrameData.u64VirAddr[0], u32Size);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Munmap BNR VirAddr fail s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
        s32Ret = HI_MPI_SYS_Munmap(stFrameData.stVideoSupplement.pFrameDNGVirAddr, sizeof(DNG_IMAGE_DYNAMIC_INFO_S));
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Munmap BNR DNG Addr fail s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
        s32Ret = HI_MPI_SYS_Munmap(stFrameData.stVideoSupplement.pJpegDCFVirAddr, sizeof(JPEG_DCF_S));
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Munmap BNR DCF Addr fail s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

static HI_VOID VPROC_PhotoProcess_Free(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 i = 0;
    HI_S32 s32FrameCnt;
    VIDEO_FRAME_INFO_S *pstSrcVideoFrame[PHOTO_MFNR_FRAME_NUM];
    VIDEO_FRAME_INFO_S *pstDesVideoFrame = NULL;
    VIDEO_FRAME_INFO_S *pstBNRRaw = NULL;

    /* get the data from backup */
    MUTEX_LOCK(g_vprocLock);
    s32FrameCnt = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.s32FrameCnt;

    pstBNRRaw = (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64BNRSrcAddr;
    pstDesVideoFrame =
        (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64DestFrmAddr;

    for (i = 0; i < s32FrameCnt; i++) {
        pstSrcVideoFrame[i] =
            (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64YUVSrcAddr[i];
    }

    if (pstBNRRaw != NULL) {
        free(pstBNRRaw);
    }

    if (pstDesVideoFrame != NULL) {
        free(pstDesVideoFrame);
    }

    for (i = 0; i < s32FrameCnt; i++) {
        if (pstSrcVideoFrame[i] != NULL) {
            free(pstSrcVideoFrame[i]);
        }
    }

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.pstVbBlk != NULL) {
        free(g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.pstVbBlk);
        g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.pstVbBlk = NULL;
    }

    VPROC_PhotoProcess_Reset(VpssHdl, VPortHdl);
    MUTEX_UNLOCK(g_vprocLock);
}

HI_S32 VPROC_PhotoProcess_Release(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    HI_S32 s32FrameCnt;
    VB_BLK *pstVbBlk = NULL;
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
    VIDEO_FRAME_INFO_S *pstSrcVideoFrame[PHOTO_MFNR_FRAME_NUM];
    VIDEO_FRAME_INFO_S *pstDesVideoFrame = NULL;
    VIDEO_FRAME_INFO_S *pstBNRRaw = NULL;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    /* get the data from backup */
    MUTEX_LOCK(g_vprocLock);
    enPhotoType = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.enPhotoType;
    s32FrameCnt = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.s32FrameCnt;
    pstVbBlk = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.pstVbBlk;

    pstBNRRaw = (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64BNRSrcAddr;

    for (i = 0; i < s32FrameCnt; i++) {
        pstSrcVideoFrame[i] =
            (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64YUVSrcAddr[i];
    }
    MUTEX_UNLOCK(g_vprocLock);

    for (i = 0; i < s32FrameCnt; i++) {
        if (pstSrcVideoFrame[i] != NULL) {
            s32Ret = HI_MPI_VPSS_ReleaseChnFrame(VpssHdl, VPortHdl, pstSrcVideoFrame[i]);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_VPSS_ReleaseChnFrame failed. total %d, %dth error :%d\n",
                               s32FrameCnt, i, s32Ret);
            }
        }
    }

    if (pstVbBlk != NULL) {
        s32Ret = HI_MPI_VB_ReleaseBlock(*pstVbBlk);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_VB_ReleaseBlock failed. error :%d\n", s32Ret);
        }
    }


    if ((enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        if (pstBNRRaw != NULL) {
            s32Ret = VPROC_ReleaseBNRRaw(VpssHdl, pstBNRRaw);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Release BNR Raw failed. error :%d\n", s32Ret);
            }
        }
    }

    VPROC_PhotoProcess_Free(VpssHdl, VPortHdl);

    return s32Ret;
}

HI_S32 VPROC_PhotoProcess_GetBNR(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                 VPROC_PHOTO_DUMP_DATA_S *pstDumpData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32FrameNum = 0;
    HI_HANDLE VencHdl;
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
    PHOTO_ALG_TYPE_E enAlgType;
    VIDEO_FRAME_INFO_S *pstBNRRaw = NULL;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpData);

    MUTEX_LOCK(g_vprocLock);
    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVPortStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d VPort %d have not started.\n", VpssHdl, VPortHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    if (g_pPhotoPubVirAddr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "The PHOTO is not initialized.\n");
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    enPhotoType = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].enPhotoType;
    VencHdl = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].BindVencHdl;
    MUTEX_UNLOCK(g_vprocLock);

    switch (enPhotoType) {
        case VPROC_PHOTO_TYPE_HDR:
            s32FrameNum = PHOTO_HDR_FRAME_NUM;
            enAlgType = PHOTO_ALG_TYPE_HDR;
            break;

        case VPROC_PHOTO_TYPE_MFNR:
        case VPROC_PHOTO_TYPE_MFNR_DE:
            s32FrameNum = PHOTO_MFNR_FRAME_NUM;
            enAlgType = PHOTO_ALG_TYPE_MFNR;
            break;

        case VPROC_PHOTO_TYPE_SFNR:
            s32FrameNum = 1;
            enAlgType = PHOTO_ALG_TYPE_SFNR;
            break;

        case VPROC_PHOTO_TYPE_DE:
            s32FrameNum = 1;
            enAlgType = PHOTO_ALG_TYPE_DE;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Do not support Photo type %d.\n", enPhotoType);
            return HI_MAPI_VPROC_EILLEGAL_PARAM;
    }

    pstDumpData->s32FrameCnt = s32FrameNum;
    pstDumpData->enPhotoType = enPhotoType;

    MUTEX_LOCK(g_vprocLock);
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.s32FrameCnt = s32FrameNum;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.enPhotoType = enPhotoType;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.enAlgType = enAlgType;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.VencHdl = VencHdl;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.s32FrameIndex = 0;
    MUTEX_UNLOCK(g_vprocLock);

    if ((enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        pstBNRRaw = (VIDEO_FRAME_INFO_S *)malloc(sizeof(VIDEO_FRAME_INFO_S));
        if (pstBNRRaw == NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Malloc BNR raw mem fail.\n");
            return HI_MAPI_VPROC_ENOMEM;
        }
        memset(pstBNRRaw, 0x00, sizeof(VIDEO_FRAME_INFO_S));

        s32Ret = VPROC_GetBNRRaw(VpssHdl, pstBNRRaw);
        VPROC_CHECK_RET_EXIT("Get BNR Raw failed", s32Ret, GOTO_LABLEL);

        VPROC_Photo_FrameInfoToData(pstBNRRaw, &pstDumpData->stBNRRaw, HI_FALSE);
        MUTEX_LOCK(g_vprocLock);
        g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64BNRSrcAddr = (HI_U64)(HI_UL)pstBNRRaw;
        MUTEX_UNLOCK(g_vprocLock);
    }

    return HI_SUCCESS;

GOTO_LABLEL:
    if (pstBNRRaw != NULL) {
        free(pstBNRRaw);
    }

    VPROC_PhotoProcess_Release(VpssHdl, VPortHdl);

    return s32Ret;
}

HI_S32 VPROC_PhotoProcess_GetYUV(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                 VPROC_PHOTO_DUMP_DATA_S *pstDumpData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32FrameIndex;
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
    VIDEO_FRAME_INFO_S *pstSrcVideoFrame = NULL;
    VIDEO_FRAME_INFO_S *pstBNRRaw = NULL;
    PHOTO_ALG_ATTR_S stPhotoAttr;

    VPSS_CHECK_HANDLE_EXIT(s32Ret, VpssHdl, GOTO_LABLEL);
    VPORT_CHECK_HANDLE_EXIT(s32Ret, VPortHdl, GOTO_LABLEL);
    VPROC_CHECK_NULL_PTR_EXIT(s32Ret, pstDumpData, GOTO_LABLEL);

    MUTEX_LOCK(g_vprocLock);
    s32FrameIndex = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.s32FrameIndex;
    enPhotoType = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.enPhotoType;
    pstBNRRaw = (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64BNRSrcAddr;
    MUTEX_UNLOCK(g_vprocLock);

    if ((enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        VPROC_CHECK_NULL_PTR_EXIT(s32Ret, pstBNRRaw, GOTO_LABLEL);
    }

    pstSrcVideoFrame = (VIDEO_FRAME_INFO_S *)malloc(sizeof(VIDEO_FRAME_INFO_S));
    VPROC_CHECK_NULL_PTR_EXIT(s32Ret, pstSrcVideoFrame, GOTO_LABLEL);
    memset(pstSrcVideoFrame, 0x00, sizeof(VIDEO_FRAME_INFO_S));

    s32Ret = HI_MPI_VPSS_GetChnFrame(VpssHdl, VPortHdl, pstSrcVideoFrame, 5000);
    VPROC_CHECK_RET_EXIT("HI_MPI_VPSS_GetChnFrame failed", s32Ret, GOTO_LABLEL);
    VPROC_Photo_FrameInfoToData(pstSrcVideoFrame, &pstDumpData->stYUVData, HI_TRUE);

    MUTEX_LOCK(g_vprocLock);
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64YUVSrcAddr[s32FrameIndex] =
        (HI_U64)( HI_UL)pstSrcVideoFrame;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;

GOTO_LABLEL:
    if (pstSrcVideoFrame != NULL) {
        free(pstSrcVideoFrame);
    }

    VPROC_PhotoProcess_Release(VpssHdl, VPortHdl);

    return s32Ret;
}

HI_S32 VPROC_PhotoProcess_AlgProcess(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ISO = 0;
    VB_BLK *pstVbBlk = NULL;
    HI_BOOL bAllocDesBuf = HI_FALSE;
    HI_S32 s32FrameIndex;
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
    PHOTO_ALG_TYPE_E enAlgType;
    VIDEO_FRAME_INFO_S *pstDesVideoFrame = NULL;
    VIDEO_FRAME_INFO_S *pstSrcVideoFrame = NULL;
    VIDEO_FRAME_INFO_S *pstBNRRaw = NULL;
    PHOTO_ALG_ATTR_S stPhotoAttr;

    VPSS_CHECK_HANDLE_EXIT(s32Ret, VpssHdl, GOTO_LABLEL);
    VPORT_CHECK_HANDLE_EXIT(s32Ret, VPortHdl, GOTO_LABLEL);

    MUTEX_LOCK(g_vprocLock);
    s32FrameIndex = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.s32FrameIndex;
    enPhotoType = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.enPhotoType;
    enAlgType = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.enAlgType;
    pstSrcVideoFrame = (VIDEO_FRAME_INFO_S *)(HI_UL)
        g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64YUVSrcAddr[s32FrameIndex];
    pstBNRRaw = (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64BNRSrcAddr;

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.pstVbBlk == NULL) {
        bAllocDesBuf = HI_FALSE;
    } else {
        bAllocDesBuf = HI_TRUE;
    }
    MUTEX_UNLOCK(g_vprocLock);

    if ((enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        VPROC_CHECK_NULL_PTR_EXIT(s32Ret, pstBNRRaw, GOTO_LABLEL);
    }
    VPROC_CHECK_NULL_PTR_EXIT(s32Ret, pstSrcVideoFrame, GOTO_LABLEL);

    if (!bAllocDesBuf) {
        pstDesVideoFrame = (VIDEO_FRAME_INFO_S *)malloc(sizeof(VIDEO_FRAME_INFO_S));
        VPROC_CHECK_NULL_PTR_EXIT(s32Ret, pstSrcVideoFrame, GOTO_LABLEL);
        memset(pstDesVideoFrame, 0x00, sizeof(VIDEO_FRAME_INFO_S));

        /* update destframe info */
        memcpy(pstDesVideoFrame, pstSrcVideoFrame, sizeof(VIDEO_FRAME_INFO_S));

        pstVbBlk = (VB_BLK *)malloc(sizeof(VB_BLK));
        VPROC_CHECK_NULL_PTR_EXIT(s32Ret, pstVbBlk, GOTO_LABLEL);
        memset(pstVbBlk, 0x00, sizeof(VB_BLK));

        s32Ret = VPROC_GetVB(VB_INVALID_POOLID, pstDesVideoFrame, pstVbBlk);
        VPROC_CHECK_RET_EXIT("Photo process Dump data get vb fail.", s32Ret, GOTO_LABLEL);

        /* update vproc dump info */
        MUTEX_LOCK(g_vprocLock);
        g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.pstVbBlk = pstVbBlk;
        g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64DestFrmAddr = (HI_U64)(HI_UL)pstDesVideoFrame;
        MUTEX_UNLOCK(g_vprocLock);
    } else {
        MUTEX_LOCK(g_vprocLock);
        pstDesVideoFrame =
            (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64DestFrmAddr;
        MUTEX_UNLOCK(g_vprocLock);
    }

    s32Ret = VPROC_GetFrameISO(pstSrcVideoFrame, &u32ISO);
    VPROC_CHECK_RET_EXIT("Photo process Dump data get frame iso fail.", s32Ret, GOTO_LABLEL0);

    memset(&stPhotoAttr, 0, sizeof(PHOTO_ALG_ATTR_S));

    switch (enPhotoType) {
        case VPROC_PHOTO_TYPE_HDR:
            stPhotoAttr.stHDRAttr.stSrcFrm = *pstSrcVideoFrame;
            stPhotoAttr.stHDRAttr.stDesFrm = *pstDesVideoFrame;
            stPhotoAttr.stHDRAttr.u32FrmIndex = s32FrameIndex;
            stPhotoAttr.stHDRAttr.u32ISO = u32ISO;
            stPhotoAttr.stHDRAttr.u32FaceNum = 0;
            break;

        case VPROC_PHOTO_TYPE_MFNR:
            stPhotoAttr.stMFNRAttr.stSrcFrm = *pstSrcVideoFrame;
            stPhotoAttr.stMFNRAttr.stDesFrm = *pstDesVideoFrame;
            stPhotoAttr.stMFNRAttr.u32FrmIndex = s32FrameIndex;
            stPhotoAttr.stMFNRAttr.u32ISO = u32ISO;
            break;

        case VPROC_PHOTO_TYPE_MFNR_DE:
            stPhotoAttr.stMFNRAttr.stSrcFrm = *pstSrcVideoFrame;
            stPhotoAttr.stMFNRAttr.stDesFrm = *pstDesVideoFrame;
            stPhotoAttr.stMFNRAttr.stRawFrm = *pstBNRRaw;
            stPhotoAttr.stMFNRAttr.u32FrmIndex = s32FrameIndex;
            stPhotoAttr.stMFNRAttr.u32ISO = u32ISO;
            break;

        case VPROC_PHOTO_TYPE_SFNR:
            stPhotoAttr.stSFNRAttr.stFrm = *pstSrcVideoFrame;
            stPhotoAttr.stSFNRAttr.u32ISO = u32ISO;
            break;

        case VPROC_PHOTO_TYPE_DE:
            stPhotoAttr.stDEAttr.stFrm = *pstSrcVideoFrame;
            stPhotoAttr.stDEAttr.stRawFrm = *pstBNRRaw;
            stPhotoAttr.stDEAttr.u32ISO = u32ISO;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Do not support Photo type %d.\n", enPhotoType);
            s32Ret = HI_MAPI_VPROC_EILLEGAL_PARAM;
            goto GOTO_LABLEL;
    }

    s32Ret = HI_MPI_PHOTO_AlgProcess(enAlgType, &stPhotoAttr);
    VPROC_CHECK_RET_EXIT("Photo process Dump data AlgProcess fail.", s32Ret, GOTO_LABLEL0);

    MUTEX_LOCK(g_vprocLock);
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.s32FrameIndex++;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;

GOTO_LABLEL0:
    if (HI_MPI_VB_ReleaseBlock(*pstVbBlk) != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_VB_ReleaseBlock failed.\n");
    }

GOTO_LABLEL:
    if (pstDesVideoFrame != NULL) {
        free(pstDesVideoFrame);

        MUTEX_LOCK(g_vprocLock);
        g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64DestFrmAddr = 0;
        MUTEX_UNLOCK(g_vprocLock);
    }

    if (pstVbBlk != NULL) {
        free(pstVbBlk);

        MUTEX_LOCK(g_vprocLock);
        g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.pstVbBlk = NULL;
        MUTEX_UNLOCK(g_vprocLock);
    }

    VPROC_PhotoProcess_Release(VpssHdl, VPortHdl);

    return s32Ret;
}

HI_S32 VPROC_PhotoProcess_Send(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    HI_S32 s32FrameCnt;
    VB_BLK *pstVbBlk = NULL;
    HI_HANDLE VencHdl;
    PHOTO_ALG_TYPE_E enAlgType;
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
    VIDEO_FRAME_INFO_S *pstSrcVideoFrame[PHOTO_MFNR_FRAME_NUM];
    VIDEO_FRAME_INFO_S *pstDesVideoFrame = NULL;
    VIDEO_FRAME_INFO_S *pstBNRRaw = NULL;
    VENC_JPEG_ENCODE_MODE_E enJpegEncodeMode;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    /* get the data from backup */
    MUTEX_LOCK(g_vprocLock);
    enPhotoType = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.enPhotoType;
    enAlgType = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.enAlgType;
    s32FrameCnt = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.s32FrameCnt;
    VencHdl = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.VencHdl;
    pstVbBlk = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.pstVbBlk;

    pstBNRRaw = (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64BNRSrcAddr;
    pstDesVideoFrame =
        (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64DestFrmAddr;

    for (i = 0; i < s32FrameCnt; i++) {
        pstSrcVideoFrame[i] =
            (VIDEO_FRAME_INFO_S *)(HI_UL)g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stPhotoDump.u64YUVSrcAddr[i];
    }
    MUTEX_UNLOCK(g_vprocLock);

    /* judge whether the frame is NULL */
    if ((enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        VPROC_CHECK_NULL_PTR_EXIT(s32Ret, pstBNRRaw, GOTO_LABLEL);
    }

    VPROC_CHECK_NULL_PTR_EXIT(s32Ret, pstDesVideoFrame, GOTO_LABLEL);

    for (i = 0; i < s32FrameCnt; i++) {
        VPROC_CHECK_NULL_PTR_EXIT(s32Ret, pstSrcVideoFrame[i], GOTO_LABLEL);
    }

    if ((enAlgType == PHOTO_ALG_TYPE_HDR) || (enAlgType == PHOTO_ALG_TYPE_MFNR)) {
        s32Ret = VPROC_CopySupplement(&pstDesVideoFrame->stVFrame, &pstSrcVideoFrame[s32FrameCnt / 2]->stVFrame);
        VPROC_CHECK_RET_EXIT("Copy Supplement failed", s32Ret, GOTO_LABLEL);
    }

    s32Ret = HI_MPI_VENC_GetJpegEncodeMode(VencHdl, &enJpegEncodeMode);
    VPROC_CHECK_RET_EXIT("HI_MPI_VENC_GetJpegEncodeMode failed", s32Ret, GOTO_LABLEL);

    s32Ret = HI_MPI_VENC_SetJpegEncodeMode(VencHdl, JPEG_ENCODE_ALL);
    VPROC_CHECK_RET_EXIT("HI_MPI_VENC_SetJpegEncodeMode failed", s32Ret, GOTO_LABLEL);

    if ((enPhotoType == VPROC_PHOTO_TYPE_SFNR) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        s32Ret = HI_MPI_VENC_SendFrame(VencHdl, pstSrcVideoFrame[0], 1000);
    } else {
        s32Ret = HI_MPI_VENC_SendFrame(VencHdl, pstDesVideoFrame, 1000);
    }

    VPROC_CHECK_RET_EXIT("HI_MPI_VENC_SendFrame failed", s32Ret, GOTO_LABLEL);

    s32Ret = HI_MPI_VENC_SetJpegEncodeMode(VencHdl, enJpegEncodeMode);
    VPROC_CHECK_RET_EXIT("HI_MPI_VENC_SetJpegEncodeMode failed", s32Ret, GOTO_LABLEL);

GOTO_LABLEL:
    for (i = 0; i < s32FrameCnt; i++) {
        if (HI_MPI_VPSS_ReleaseChnFrame(VpssHdl, VPortHdl, pstSrcVideoFrame[i]) != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_VPSS_ReleaseChnFrame failed.\n");
        }
    }

    if (HI_MPI_VB_ReleaseBlock(*pstVbBlk) != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_VB_ReleaseBlock failed.\n");
    }

    if ((enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        if (VPROC_ReleaseBNRRaw(VpssHdl, pstBNRRaw) != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Release BNR Raw failed.\n");
        }
    }

    VPROC_PhotoProcess_Free(VpssHdl, VPortHdl);

    return s32Ret;
}

HI_S32 HI_MAPI_VPROC_InitPhoto(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_PHOTO_TYPE_E enPhotoType)
{
    HI_S32 s32Ret;
    HI_S32 s32FrameNum = 0;
    HI_U32 u32PhotoMemSize = 0;
    HI_CHAR cBufName[32] = "PhotoPubMem";
    PHOTO_ALG_INIT_S stPhotoInit;
    PHOTO_ALG_TYPE_E enAlgType;
    VPSS_CHN_ATTR_S stChnAttr;
    HI_HANDLE VencHdl = HI_INVALID_HANDLE;
    BNR_DUMP_ATTR_S stBNRAttr;
    PHOTO_ALG_COEF_S stAlgCoef;
    MPP_CHN_S stDestChn;
    MPP_CHN_S stSrcChn;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_pPhotoPubVirAddr != HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "The PHOTO has been initialized.\n");
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_ENOT_PERM;
    }

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssHdl;
    stDestChn.s32ChnId = 0;
    s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_SYS_GetBindbyDest failed", s32Ret, g_vprocLock);

    if (stSrcChn.enModId != HI_ID_VI) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "vproc's src is not vcap.\n");
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_VPSS_GetChnAttr failed", s32Ret, g_vprocLock);

    s32Ret = VPROC_InitPhotoParm(enPhotoType, &stChnAttr, &enAlgType, &s32FrameNum, &u32PhotoMemSize);
    VPROC_CHECK_RET_UNLOCK("Init PhotoParm failed", s32Ret, g_vprocLock);

    if ((enPhotoType == VPROC_PHOTO_TYPE_DE) || (enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE)) {
        stBNRAttr.bEnable = HI_TRUE;
        stBNRAttr.u32Depth = s32FrameNum;

        s32Ret = HI_MPI_VI_SetPipeBNRRawDumpAttr(stSrcChn.s32DevId, &stBNRAttr);
        VPROC_CHECK_RET_UNLOCK("HI_MPI_VI_SetPipeBNRRawDumpAttr failed", s32Ret, g_vprocLock);
    }

    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&g_u64PhotoPubPhyAddr, &g_pPhotoPubVirAddr, cBufName, HI_NULL,
                                        u32PhotoMemSize);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_SYS_MmzAlloc_Cached failed", s32Ret, g_vprocLock);

    stPhotoInit.u64PublicMemPhyAddr = g_u64PhotoPubPhyAddr;
    stPhotoInit.u64PublicMemVirAddr = (HI_U64)(HI_UL)g_pPhotoPubVirAddr;
    stPhotoInit.u32PublicMemSize = u32PhotoMemSize;
    stPhotoInit.bPrintDebugInfo = HI_FALSE;
    s32Ret = HI_MPI_PHOTO_AlgInit(enAlgType, &stPhotoInit);
    VPROC_CHECK_RET_EXIT("HI_MPI_PHOTO_AlgInit failed", s32Ret, EXIT1);

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].enPhotoType = enPhotoType;

    if (enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) {
        s32Ret = HI_MPI_PHOTO_GetAlgCoef(PHOTO_ALG_TYPE_MFNR, &stAlgCoef);
        VPROC_CHECK_RET_EXIT("HI_MPI_PHOTO_GetAlgCoef failed", s32Ret, EXIT2);

        stAlgCoef.stPhotoMfnrCoef.bDEEnable = HI_TRUE;

        s32Ret = HI_MPI_PHOTO_SetAlgCoef(PHOTO_ALG_TYPE_MFNR, &stAlgCoef);
        VPROC_CHECK_RET_EXIT("HI_MPI_PHOTO_SetAlgCoef failed", s32Ret, EXIT2);
    } else if (enPhotoType == VPROC_PHOTO_TYPE_MFNR) {
        s32Ret = HI_MPI_PHOTO_GetAlgCoef(PHOTO_ALG_TYPE_MFNR, &stAlgCoef);
        VPROC_CHECK_RET_EXIT("HI_MPI_PHOTO_GetAlgCoef failed", s32Ret, EXIT2);

        stAlgCoef.stPhotoMfnrCoef.bDEEnable = HI_FALSE;

        s32Ret = HI_MPI_PHOTO_SetAlgCoef(PHOTO_ALG_TYPE_MFNR, &stAlgCoef);
        VPROC_CHECK_RET_EXIT("HI_MPI_PHOTO_SetAlgCoef failed", s32Ret, EXIT2);
    }

    stChnAttr.u32Depth = s32FrameNum;

    s32Ret = HI_MPI_VPSS_SetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET_EXIT("HI_MPI_VPSS_SetChnAttr failed", s32Ret, EXIT2);

    s32Ret = VPROC_GetJpegChn(VpssHdl, VPortHdl, &VencHdl);
    VPROC_CHECK_RET_EXIT("Get JpegChn failed", s32Ret, EXIT3);
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].BindVencHdl = VencHdl;

    s32Ret = VPROC_CutOff_Venc(VpssHdl, VPortHdl, VencHdl);
    VPROC_CHECK_RET_EXIT("CutOff Venc failed", s32Ret, EXIT3);
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;

EXIT3:

    stChnAttr.u32Depth = 0;

    if (HI_MPI_VPSS_SetChnAttr(VpssHdl, VPortHdl, &stChnAttr) != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_VPSS_SetChnAttr failed.\n");
    }

EXIT2:

    if (HI_MPI_PHOTO_AlgDeinit(enAlgType) != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_PHOTO_AlgDeinit failed.\n");
    }

EXIT1:

    if (HI_MPI_SYS_MmzFree(g_u64PhotoPubPhyAddr, g_pPhotoPubVirAddr) != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SYS_MmzFree failed.\n");
    }

    g_u64PhotoPubPhyAddr = 0;
    g_pPhotoPubVirAddr = HI_NULL;
    MUTEX_UNLOCK(g_vprocLock);

    return s32Ret;
}

HI_S32 HI_MAPI_VPROC_DeinitPhoto(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret;
    HI_HANDLE VencHdl;
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
    PHOTO_ALG_TYPE_E enAlgType = PHOTO_ALG_TYPE_BUTT;
    BNR_DUMP_ATTR_S stBNRAttr;
    MPP_CHN_S stDestChn;
    MPP_CHN_S stSrcChn;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_pPhotoPubVirAddr == HI_NULL) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "The PHOTO has been Deinit.\n");
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    enPhotoType = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].enPhotoType;
    VencHdl = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].BindVencHdl;

    s32Ret = VPROC_Reconnect_Venc(VpssHdl, VPortHdl, VencHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Reconnect Venc %d failed.\n", VencHdl);
    }

    if ((enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        stDestChn.enModId = HI_ID_VPSS;
        stDestChn.s32DevId = VpssHdl;
        stDestChn.s32ChnId = 0;
        s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SYS_GetBindbyDest failed.\n");
        }

        stBNRAttr.bEnable = HI_FALSE;
        stBNRAttr.u32Depth = 0;

        s32Ret = HI_MPI_VI_SetPipeBNRRawDumpAttr(stSrcChn.s32DevId, &stBNRAttr);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SNAP_SetBNRRawDumpAttr failed!\n");
        }
    }

    switch (enPhotoType) {
        case VPROC_PHOTO_TYPE_HDR:
            enAlgType = PHOTO_ALG_TYPE_HDR;
            break;

        case VPROC_PHOTO_TYPE_MFNR:
        case VPROC_PHOTO_TYPE_MFNR_DE:
            enAlgType = PHOTO_ALG_TYPE_MFNR;
            break;

        case VPROC_PHOTO_TYPE_SFNR:
            enAlgType = PHOTO_ALG_TYPE_SFNR;
            break;

        case VPROC_PHOTO_TYPE_DE:
            enAlgType = PHOTO_ALG_TYPE_DE;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Do not support Photo type %d.\n", enPhotoType);
    }

    if (HI_MPI_PHOTO_AlgDeinit(enAlgType) != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_PHOTO_AlgDeinit failed.\n");
    }

    if (HI_MPI_SYS_MmzFree(g_u64PhotoPubPhyAddr, g_pPhotoPubVirAddr) != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SYS_MmzFree failed.\n");
    }

    g_u64PhotoPubPhyAddr = 0;
    g_pPhotoPubVirAddr = HI_NULL;
    MUTEX_UNLOCK(g_vprocLock);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_PhotoProcess(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    HI_S32 s32FrameNum = 0;
    HI_U32 u32ISO;
    HI_HANDLE VencHdl;
    HI_BOOL bAllocDesBuf = HI_FALSE;
    VB_BLK VbBlk;
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
    VIDEO_FRAME_INFO_S stDesVideoFrame;
    VIDEO_FRAME_INFO_S stSrcVideoFrame[PHOTO_MFNR_FRAME_NUM];
    PHOTO_ALG_ATTR_S stPhotoAttr;
    PHOTO_ALG_TYPE_E enAlgType;
    VIDEO_FRAME_INFO_S stRawInfo;
    VENC_JPEG_ENCODE_MODE_E enJpegEncodeMode;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVPortStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss %d VPort %d have not started.\n", VpssHdl, VPortHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    if (g_pPhotoPubVirAddr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "The PHOTO is not initialized.\n");
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    enPhotoType = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].enPhotoType;
    VencHdl = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].BindVencHdl;
    MUTEX_UNLOCK(g_vprocLock);

    switch (enPhotoType) {
        case VPROC_PHOTO_TYPE_HDR:
            s32FrameNum = PHOTO_HDR_FRAME_NUM;
            enAlgType = PHOTO_ALG_TYPE_HDR;
            break;

        case VPROC_PHOTO_TYPE_MFNR:
        case VPROC_PHOTO_TYPE_MFNR_DE:
            s32FrameNum = PHOTO_MFNR_FRAME_NUM;
            enAlgType = PHOTO_ALG_TYPE_MFNR;
            break;

        case VPROC_PHOTO_TYPE_SFNR:
            s32FrameNum = 1;
            enAlgType = PHOTO_ALG_TYPE_SFNR;
            break;

        case VPROC_PHOTO_TYPE_DE:
            s32FrameNum = 1;
            enAlgType = PHOTO_ALG_TYPE_DE;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Do not support Photo type %d.\n", enPhotoType);
            return HI_MAPI_VPROC_EILLEGAL_PARAM;
    }

    if ((enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        s32Ret = VPROC_GetBNRRaw(VpssHdl, &stRawInfo);
        VPROC_CHECK_RET("Get BNR Raw failed", s32Ret);

        /* BNR */
        if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stBNRRawCB.pfunVProcBNRProc != NULL) {
            s32Ret = VPROC_PhotoProcess_Dump(VpssHdl, VPortHdl, HI_TRUE, &stRawInfo);
            VPROC_CHECK_RET_EXIT("VPROC_PhotoDumpBNRRaw failed", s32Ret, EXIT2);
        }
    }

    for (i = 0; i < s32FrameNum; i++) {
        s32Ret = HI_MPI_VPSS_GetChnFrame(VpssHdl, VPortHdl, &stSrcVideoFrame[i], 5000);
        VPROC_CHECK_RET_EXIT("HI_MPI_VPSS_GetChnFrame failed", s32Ret, EXIT1);

        /* yuv */
        if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pfunVProcYUVProc != NULL) {
            s32Ret = VPROC_PhotoProcess_Dump(VpssHdl, VPortHdl, HI_FALSE, &stSrcVideoFrame[i]);
            VPROC_CHECK_RET_EXIT("VPROC_PhotoDumpYUV failed", s32Ret, EXIT2);
        }

        if (!bAllocDesBuf) {
            stDesVideoFrame = stSrcVideoFrame[i];

            s32Ret = VPROC_GetVB(VB_INVALID_POOLID, &stDesVideoFrame, &VbBlk);
            VPROC_CHECK_RET_EXIT("Get VB failed", s32Ret, EXIT1);

            bAllocDesBuf = HI_TRUE;
        }

        s32Ret = VPROC_GetFrameISO(&stSrcVideoFrame[i], &u32ISO);
        VPROC_CHECK_RET_EXIT("Get Frame ISO failed", s32Ret, EXIT2);

        memset(&stPhotoAttr, 0, sizeof(PHOTO_ALG_ATTR_S));

        switch (enPhotoType) {
            case VPROC_PHOTO_TYPE_HDR:
                stPhotoAttr.stHDRAttr.stSrcFrm = stSrcVideoFrame[i];
                stPhotoAttr.stHDRAttr.stDesFrm = stDesVideoFrame;
                stPhotoAttr.stHDRAttr.u32FrmIndex = i;
                stPhotoAttr.stHDRAttr.u32ISO = u32ISO;
                stPhotoAttr.stHDRAttr.u32FaceNum = 0;
                break;

            case VPROC_PHOTO_TYPE_MFNR:
                stPhotoAttr.stMFNRAttr.stSrcFrm = stSrcVideoFrame[i];
                stPhotoAttr.stMFNRAttr.stDesFrm = stDesVideoFrame;
                stPhotoAttr.stMFNRAttr.u32FrmIndex = i;
                stPhotoAttr.stMFNRAttr.u32ISO = u32ISO;
                break;

            case VPROC_PHOTO_TYPE_MFNR_DE:
                stPhotoAttr.stMFNRAttr.stSrcFrm = stSrcVideoFrame[i];
                stPhotoAttr.stMFNRAttr.stDesFrm = stDesVideoFrame;
                stPhotoAttr.stMFNRAttr.stRawFrm = stRawInfo;
                stPhotoAttr.stMFNRAttr.u32FrmIndex = i;
                stPhotoAttr.stMFNRAttr.u32ISO = u32ISO;
                break;

            case VPROC_PHOTO_TYPE_SFNR:
                stPhotoAttr.stSFNRAttr.stFrm = stSrcVideoFrame[i];
                stPhotoAttr.stSFNRAttr.u32ISO = u32ISO;
                break;

            case VPROC_PHOTO_TYPE_DE:
                stPhotoAttr.stDEAttr.stFrm = stSrcVideoFrame[i];
                stPhotoAttr.stDEAttr.stRawFrm = stRawInfo;
                stPhotoAttr.stDEAttr.u32ISO = u32ISO;
                break;

            default:
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Do not support Photo type %d.\n", enPhotoType);
                goto EXIT2;
        }

        s32Ret = HI_MPI_PHOTO_AlgProcess(enAlgType, &stPhotoAttr);
        VPROC_CHECK_RET_EXIT("HI_MPI_PHOTO_AlgProcess failed", s32Ret, EXIT2);
    }

    if ((enAlgType == PHOTO_ALG_TYPE_HDR) || (enAlgType == PHOTO_ALG_TYPE_MFNR)) {
        s32Ret = VPROC_CopySupplement(&stDesVideoFrame.stVFrame, &stSrcVideoFrame[s32FrameNum / 2].stVFrame);
        VPROC_CHECK_RET_EXIT("Copy Supplement failed", s32Ret, EXIT2);
    }

    s32Ret = HI_MPI_VENC_GetJpegEncodeMode(VencHdl, &enJpegEncodeMode);
    VPROC_CHECK_RET_EXIT("HI_MPI_VENC_GetJpegEncodeMode failed", s32Ret, EXIT2);

    s32Ret = HI_MPI_VENC_SetJpegEncodeMode(VencHdl, JPEG_ENCODE_ALL);
    VPROC_CHECK_RET_EXIT("HI_MPI_VENC_SetJpegEncodeMode failed", s32Ret, EXIT2);

    if ((enPhotoType == VPROC_PHOTO_TYPE_SFNR) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        s32Ret = HI_MPI_VENC_SendFrame(VencHdl, &stSrcVideoFrame[0], 1000);
    } else {
        s32Ret = HI_MPI_VENC_SendFrame(VencHdl, &stDesVideoFrame, 1000);
    }

    VPROC_CHECK_RET_EXIT("HI_MPI_VENC_SendFrame failed", s32Ret, EXIT2);

    s32Ret = HI_MPI_VENC_SetJpegEncodeMode(VencHdl, enJpegEncodeMode);
    VPROC_CHECK_RET_EXIT("HI_MPI_VENC_SetJpegEncodeMode failed", s32Ret, EXIT2);

EXIT2:

    for (i = 0; i < s32FrameNum; i++) {
        if (HI_MPI_VPSS_ReleaseChnFrame(VpssHdl, VPortHdl, &stSrcVideoFrame[i]) != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_VPSS_ReleaseChnFrame failed.\n");
        }
    }

    if (HI_MPI_VB_ReleaseBlock(VbBlk) != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_VB_ReleaseBlock failed.\n");
    }

EXIT1:

    if ((enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) || (enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        if (VPROC_ReleaseBNRRaw(VpssHdl, &stRawInfo) != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Release BNR Raw failed.\n");
        }
    }

    return s32Ret;
}

HI_S32 HI_MAPI_VPROC_GetCurrentPhotoType(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                         HI_MAPI_PHOTO_TYPE_E *penPhotoType)
{
    VPROC_CHECK_NULL_PTR(penPhotoType);

    MUTEX_LOCK(g_vprocLock);

    if (g_pPhotoPubVirAddr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "The PHOTO is not initialized.\n");
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    *penPhotoType = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].enPhotoType;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetPhotoAlgCoef(PHOTO_ALG_TYPE_E enAlgType, const PHOTO_ALG_COEF_S *pstAlgCoef)
{
    HI_S32 s32Ret;
    VPROC_CHECK_NULL_PTR(pstAlgCoef);

    MUTEX_LOCK(g_vprocLock);

    if (g_pPhotoPubVirAddr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "The PHOTO is not initialized.\n");
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    MUTEX_UNLOCK(g_vprocLock);

    s32Ret = HI_MPI_PHOTO_SetAlgCoef(enAlgType, pstAlgCoef);
    VPROC_CHECK_RET("HI_MPI_PHOTO_SetAlgCoef failed", s32Ret);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetPhotoAlgCoef(PHOTO_ALG_TYPE_E enAlgType, PHOTO_ALG_COEF_S *pstAlgCoef)
{
    HI_S32 s32Ret;
    VPROC_CHECK_NULL_PTR(pstAlgCoef);

    MUTEX_LOCK(g_vprocLock);

    if (g_pPhotoPubVirAddr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "The PHOTO is not initialized.\n");
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    MUTEX_UNLOCK(g_vprocLock);

    s32Ret = HI_MPI_PHOTO_GetAlgCoef(enAlgType, pstAlgCoef);
    VPROC_CHECK_RET("HI_MPI_PHOTO_GetAlgCoef failed", s32Ret);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_RegPhotoDumpCB(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                    HI_PHOTO_DUMP_CALLBACK_FUNC_S *pstDumpCB)
{
    VPSS_CHECK_HANDLE(VpssHdl);
    VPSS_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpCB);

    if ((pstDumpCB->stBNRRawCB.pfunVProcBNRProc == NULL) && ( pstDumpCB->stYUVDataCB.pfunVProcYUVProc ==NULL)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Reg PhotoDump CB ptr both is NULL!\n");
        return HI_MAPI_VPROC_EILLEGAL_PARAM;
    }

    if (pstDumpCB->stBNRRawCB.pfunVProcBNRProc != NULL) {
        memcpy(&g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stBNRRawCB, &pstDumpCB->stBNRRawCB,
               sizeof(HI_DUMP_BNRRAW_CALLBACK_FUNC_S));
    }

    if (pstDumpCB->stYUVDataCB.pfunVProcYUVProc != NULL) {
        memcpy(&g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun, &pstDumpCB->stYUVDataCB,
            sizeof(HI_DUMP_YUV_CALLBACK_FUNC_S));
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_UnRegPhotoDumpCB(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    VPSS_CHECK_HANDLE(VpssHdl);
    VPSS_CHECK_HANDLE(VPortHdl);

    memset(&g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun, 0x00, sizeof(HI_DUMP_YUV_CALLBACK_FUNC_S));
    memset(&g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stBNRRawCB, 0x00, sizeof(HI_DUMP_BNRRAW_CALLBACK_FUNC_S));

    return HI_SUCCESS;
}
#endif
static HI_S32 VpssDumpYUV(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_FRAME_DATA_S *pstFrameData)
{
    HI_S32 s32Ret;
    HI_VOID *VirAddr = NULL;
    HI_U32 u32Size = 0;

    s32Ret = VPROC_GetChnFrame(VpssHdl, VPortHdl, HI_FALSE, pstFrameData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "get vpss chn frame timeout.\n");
        return HI_FAILURE;
    }

    if (pstFrameData->enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        u32Size = (pstFrameData->u32Stride[0] * pstFrameData->u32Height) * 3 / 2;
    } else if (pstFrameData->enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_422) {
        u32Size = (pstFrameData->u32Stride[0] * pstFrameData->u32Height) * 2;
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "enPixelFormat error!\n");
    }

    VirAddr = HI_MPI_SYS_Mmap(pstFrameData->u64PhyAddr[0], u32Size);

    if (VirAddr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SYS_Mmap fail!\n");
    }

    pstFrameData->u64VirAddr[0] = (HI_U64)(HI_UL)VirAddr;

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pfunVProcYUVProc(VpssHdl, VPortHdl,
        pstFrameData, g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pPrivateData);

    s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)(HI_UL)pstFrameData->u64VirAddr[0], u32Size);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SYS_Munmap fail s32Ret:%x\n", s32Ret);
    }

    s32Ret = VPROC_ReleaseChnFrame(VpssHdl, VPortHdl, HI_FALSE, pstFrameData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Release vpss chn frame fail s32Ret:%x\n", s32Ret);
    }

    return HI_SUCCESS;
}

HI_VOID *VpssDumpYUVthread(void *pArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, s32Count;
    HI_HANDLE VpssHdl;
    HI_HANDLE VPortHdl;
    VProcDumpYuvThreadArg_S *pstArg;
    HI_MAPI_FRAME_DATA_S stYUVFrameData;

    prctl(PR_SET_NAME, (unsigned long)"Hi_pTDumpYUV", 0, 0, 0);
    pstArg = (VProcDumpYuvThreadArg_S *)pArg;
    VpssHdl = pstArg->GrpHdl;
    VPortHdl = pstArg->PortHdl;
    s32Count = pstArg->s32Count;

    if (s32Count >= 0) {
        for (i = 0; i < s32Count; i++) {
            s32Ret = VpssDumpYUV(VpssHdl, VPortHdl, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss Dump YUV %d count fail s32Ret:%x\n", i, s32Ret);
                continue;
            }
        }
    } else if (-1 == s32Count) {
        while (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVpssDumpYUVStarted) {
            /* count is -1 */
            s32Ret = VpssDumpYUV(VpssHdl, VPortHdl, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss Dump YUV fail s32Ret:%x\n", s32Ret);
                continue;
            }
        }
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss Dump YUV input count is unsupport.\n");
        return NULL;
    }

    return NULL;
}

#ifdef SUPPORT_STITCH
static HI_S32 StitchDumpYUV(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_MAPI_FRAME_DATA_S *pstYUVFrameData)
{
    HI_S32 s32Ret;
    HI_VOID *VirAddr = NULL;
    HI_U32 u32Size = 0;

    s32Ret = VPROC_GetChnFrame(StitchHdl, StitchPortHdl, HI_TRUE, pstYUVFrameData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "get Stitch chn frame timeout.\n");
        return HI_FAILURE;
    }

    if (pstYUVFrameData->enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        u32Size = (pstYUVFrameData->u32Stride[0] * pstYUVFrameData->u32Height) * 3 / 2;
    } else if (pstYUVFrameData->enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_422) {
        u32Size = (pstYUVFrameData->u32Stride[0] * pstYUVFrameData->u32Height) * 2;
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "enPixelFormat error!\n");
    }

    VirAddr = HI_MPI_SYS_Mmap(pstYUVFrameData->u64PhyAddr[0], u32Size);

    if (VirAddr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SYS_Mmap fail!\n");
    }

    pstYUVFrameData->u64VirAddr[0] = (HI_U64)(HI_UL)VirAddr;

    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].stCallbackFun.pfunVProcYUVProc(StitchHdl, StitchPortHdl,
        pstYUVFrameData, g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].stCallbackFun.pPrivateData);

    s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)(HI_UL)pstYUVFrameData->u64VirAddr[0], u32Size);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "HI_MPI_SYS_Munmap fail s32Ret:%x\n", s32Ret);
    }

    s32Ret = VPROC_ReleaseChnFrame(StitchHdl, StitchPortHdl, HI_TRUE, pstYUVFrameData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Release Stitch chn frame error.\n");
    }

    return HI_SUCCESS;
}

HI_VOID *StitchDumpYUVthread(void *pArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, s32Count;
    HI_HANDLE StitchHdl;
    HI_HANDLE StitchPortHdl;
    VProcDumpYuvThreadArg_S *pstArg;
    HI_MAPI_FRAME_DATA_S stYUVFrameData;
    HI_VOID *VirAddr;
    HI_U32 u32Size = 0;

    prctl(PR_SET_NAME, (unsigned long)"Hi_pTDumpYUV", 0, 0, 0);
    pstArg = (VProcDumpYuvThreadArg_S *)pArg;
    StitchHdl = pstArg->GrpHdl;
    StitchPortHdl = pstArg->PortHdl;
    s32Count = pstArg->s32Count;

    if (s32Count >= 0) {
        for (i = 0; i < s32Count; i++) {
            s32Ret = StitchDumpYUV(StitchHdl, StitchPortHdl, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "StitchHdl %d StitchPortHdl %d Dump Yuv %d count fail.\n", StitchHdl,
                               StitchPortHdl, i);
                continue;
            }
        }
    } else if (-1 == s32Count) {
        while (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bStitchDumpYUVStartd) {
            /* count is -1 */
            s32Ret = StitchDumpYUV(StitchHdl, StitchPortHdl, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "StitchHdl %d StitchPortHdl %d Dump Yuv fail.\n", StitchHdl,
                               StitchPortHdl);
                continue;
            }
        }
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch Dump YUV input count is unsupport.\n");
        return NULL;
    }

    return NULL;
}
#endif

/* DumpYUV */
HI_S32 HI_MAPI_VPROC_SetVpssDumpYUVAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                        HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr)
{
    HI_S32 s32Ret;
    VPSS_CHN_ATTR_S stChnAttr;
    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpYUVAttr);

    s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);

    stChnAttr.u32Depth = pstDumpYUVAttr->u32Depth;
    s32Ret = HI_MPI_VPSS_SetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET("HI_MPI_VPSS_SetChnAttr failed", s32Ret);

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bDumpYuvEnable = pstDumpYUVAttr->bEnable;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetVpssDumpYUVAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                        HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr)
{
    HI_S32 s32Ret;
    VPSS_CHN_ATTR_S stChnAttr;
    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpYUVAttr);

    s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET("HI_MPI_VPSS_GetChnAttr failed", s32Ret);

    pstDumpYUVAttr->u32Depth = stChnAttr.u32Depth;
    pstDumpYUVAttr->bEnable = g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bDumpYuvEnable;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StartVpssDumpYUV(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_S32 s32Count,
                                      HI_DUMP_YUV_CALLBACK_FUNC_S *pstCallbackFun)
{
    HI_S32 s32Ret;
    VPSS_CHN_ATTR_S stChnAttr;
    static VProcDumpYuvThreadArg_S stVpssDumpArg[HI_MAPI_VPSS_MAX_NUM][HI_MAPI_VPORT_MAX_NUM];

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstCallbackFun);
    VPROC_CHECK_NULL_PTR(pstCallbackFun->pfunVProcYUVProc);

    if (s32Count == 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss DumpYUV input is ereor.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVPortStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "vpss %d vport %d have not started.\n", VpssHdl, VPortHdl);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bDumpYuvEnable != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss dumpYUV has not been enabled.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    memset(&stChnAttr, 0x00, sizeof(VPSS_CHN_ATTR_S));
    s32Ret = HI_MPI_VPSS_GetChnAttr(VpssHdl, VPortHdl, &stChnAttr);
    VPROC_CHECK_RET("dump yuv call HI_MPI_VPSS_GetChnAttr failed", s32Ret);

    if (stChnAttr.u32Depth == 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss can't support dump YUV when the depth is zero.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pfunVProcYUVProc != HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss DumpYUV is busy,please wait.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pfunVProcYUVProc = pstCallbackFun->pfunVProcYUVProc;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pPrivateData = pstCallbackFun->pPrivateData;

    stVpssDumpArg[VpssHdl][VPortHdl].GrpHdl = VpssHdl;
    stVpssDumpArg[VpssHdl][VPortHdl].PortHdl = VPortHdl;
    stVpssDumpArg[VpssHdl][VPortHdl].s32Count = s32Count;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVpssDumpYUVStarted = HI_TRUE;

    s32Ret = pthread_create(&g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].pthreadDumpYUV, HI_NULL,
                            VpssDumpYUVthread, (void *)&stVpssDumpArg[VpssHdl][VPortHdl]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Create Vpss dump yuv thread error.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StopVpssDumpYUV(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bDumpYuvEnable != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss dumpYUV has not been enabled.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].bVpssDumpYUVStarted = HI_FALSE;

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].pthreadDumpYUV != -1) {
        s32Ret = pthread_join(g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].pthreadDumpYUV, HI_NULL);
        VPROC_CHECK_RET("pthread_join failed", s32Ret);
    }

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].pthreadDumpYUV = -1;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pfunVProcYUVProc = HI_NULL;
    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].stCallbackFun.pPrivateData = HI_NULL;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetStitchDumpYUVAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                          HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    AVS_CHN_ATTR_S stAVSChnAttr;
    VPSS_CHECK_HANDLE(StitchHdl);
    VPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpYUVAttr);

    s32Ret = HI_MPI_AVS_GetChnAttr(StitchHdl, StitchPortHdl, &stAVSChnAttr);
    VPROC_CHECK_RET("HI_MPI_AVS_GetChnAttr failed", s32Ret);

    stAVSChnAttr.u32Depth = pstDumpYUVAttr->u32Depth;
    s32Ret = HI_MPI_AVS_SetChnAttr(StitchHdl, StitchPortHdl, &stAVSChnAttr);
    VPROC_CHECK_RET("HI_MPI_AVS_SetChnAttr failed", s32Ret);

    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bDumpYuvEnable = pstDumpYUVAttr->bEnable;

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_GetStitchDumpYUVAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                          HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    AVS_CHN_ATTR_S stAVSChnAttr;
    VPSS_CHECK_HANDLE(StitchHdl);
    VPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpYUVAttr);

    s32Ret = HI_MPI_AVS_GetChnAttr(StitchHdl, StitchPortHdl, &stAVSChnAttr);
    VPROC_CHECK_RET("HI_MPI_AVS_GetChnAttr failed", s32Ret);

    pstDumpYUVAttr->u32Depth = stAVSChnAttr.u32Depth;
    pstDumpYUVAttr->bEnable = g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bDumpYuvEnable;

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_StartStitchDumpYUV(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_S32 s32Count,
                                        HI_DUMP_YUV_CALLBACK_FUNC_S *pstCallbackFun)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    AVS_CHN_ATTR_S stAVSChnAttr;
    static VProcDumpYuvThreadArg_S stStitchDumpArg[HI_MAPI_STITCH_MAX_NUM][HI_MAPI_STITCH_PORT_MAX_NUM];
    VPSS_CHECK_HANDLE(StitchHdl);
    VPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_CHECK_NULL_PTR(pstCallbackFun);
    VPROC_CHECK_NULL_PTR(pstCallbackFun->pfunVProcYUVProc);

    if (s32Count == 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch DumpYUV input is ereor.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bStitchPortStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d StitchPort %d have not started.\n", StitchHdl, StitchHdl);
        return HI_MAPI_VPROC_ENOTREADY;
    }

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bDumpYuvEnable != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch dumpYUV has not been enabled.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    memset(&stAVSChnAttr, 0x00, sizeof(AVS_CHN_ATTR_S));
    s32Ret = HI_MPI_AVS_GetChnAttr(StitchHdl, StitchPortHdl, &stAVSChnAttr);
    VPROC_CHECK_RET("dump stitch yuv call HI_MPI_AVS_GetChnAttr failed", s32Ret);

    if (stAVSChnAttr.u32Depth == 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss can't support dump stitch YUV when the depth is zero.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].stCallbackFun.pfunVProcYUVProc != HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch DumpYUV is busy,please wait.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].stCallbackFun.pfunVProcYUVProc =
        pstCallbackFun->pfunVProcYUVProc;
    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].stCallbackFun.pPrivateData =
        pstCallbackFun->pPrivateData;

    stStitchDumpArg[StitchHdl][StitchPortHdl].GrpHdl = StitchHdl;
    stStitchDumpArg[StitchHdl][StitchPortHdl].PortHdl = StitchPortHdl;
    stStitchDumpArg[StitchHdl][StitchPortHdl].s32Count = s32Count;
    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bStitchDumpYUVStartd = HI_TRUE;

    s32Ret = pthread_create(&g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].pthreadDumpYUV, HI_NULL,
                            StitchDumpYUVthread, (void *)&stStitchDumpArg[StitchHdl][StitchPortHdl]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Create Stitch dump yuv thread error.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_StopStitchDumpYUV(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CHECK_HANDLE(StitchHdl);
    VPORT_CHECK_HANDLE(StitchPortHdl);

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bDumpYuvEnable != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch dumpYUV has not been enabled.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }
    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bStitchDumpYUVStartd = HI_FALSE;

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].pthreadDumpYUV != -1) {
        s32Ret = pthread_join(g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].pthreadDumpYUV, HI_NULL);
        VPROC_CHECK_RET("pthread_join failed", s32Ret);
    }

    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].pthreadDumpYUV = -1;
    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].stCallbackFun.pfunVProcYUVProc = HI_NULL;
    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].stCallbackFun.pPrivateData = HI_NULL;

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

/* OSD */
static HI_S32 VPROC_OSD_SetAttr(HI_HANDLE RgnHdl, const MPP_CHN_S *pstChn, const HI_MAPI_OSD_ATTR_S *pstOsdAttr)
{
    HI_S32 s32Ret;
    RGN_CHN_ATTR_S stRgnChnAttr;

    s32Ret = HI_MPI_RGN_GetDisplayAttr(RgnHdl, pstChn, &stRgnChnAttr);
    VPROC_CHECK_RET("HI_MPI_RGN_GetDisplayAttr failed", s32Ret);

    stRgnChnAttr.bShow = pstOsdAttr->stOsdDisplayAttr.bShow;

    if (stRgnChnAttr.enType == OVERLAYEX_RGN) {
        stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = pstOsdAttr->stOsdDisplayAttr.s32RegionX;
        stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = pstOsdAttr->stOsdDisplayAttr.s32RegionY;
        stRgnChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha = pstOsdAttr->stOsdDisplayAttr.u32BgAlpha;
        stRgnChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha = pstOsdAttr->stOsdDisplayAttr.u32FgAlpha;
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "disp region type %d is not osd\n", stRgnChnAttr.enType);
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    s32Ret = HI_MPI_RGN_SetDisplayAttr(RgnHdl, pstChn, &stRgnChnAttr);
    VPROC_CHECK_RET("HI_MPI_RGN_SetDisplayAttr failed", s32Ret);

    s32Ret = HI_MPI_RGN_SetBitMap(RgnHdl, &pstOsdAttr->stBitmapAttr);
    VPROC_CHECK_RET("HI_MPI_RGN_SetBitMap failed", s32Ret);

    return HI_SUCCESS;
}

static HI_S32 VPROC_OSD_Start(HI_HANDLE RgnHdl, HI_HANDLE OSDHdl, const MPP_CHN_S *pstChn,
                              const HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    RGN_CHN_ATTR_S stRgnChnAttr;
    RGN_ATTR_S stRgnAttr;

    stRgnAttr.enType = OVERLAYEX_RGN;
    stRgnAttr.unAttr.stOverlayEx.u32CanvasNum = pstOSDAttr->u32CanvasNum;
    stRgnAttr.unAttr.stOverlayEx.enPixelFmt = pstOSDAttr->stBitmapAttr.enPixelFormat;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Width = pstOSDAttr->stBitmapAttr.u32Width;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Height = pstOSDAttr->stBitmapAttr.u32Height;
    stRgnAttr.unAttr.stOverlayEx.u32BgColor = pstOSDAttr->stOsdDisplayAttr.u32Color;

    stRgnChnAttr.enType = OVERLAYEX_RGN;
    stRgnChnAttr.bShow = pstOSDAttr->stOsdDisplayAttr.bShow;
    stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = pstOSDAttr->stOsdDisplayAttr.s32RegionX;
    stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = pstOSDAttr->stOsdDisplayAttr.s32RegionY;
    stRgnChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha = pstOSDAttr->stOsdDisplayAttr.u32BgAlpha;
    stRgnChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha = pstOSDAttr->stOsdDisplayAttr.u32FgAlpha;
    stRgnChnAttr.unChnAttr.stOverlayExChn.u32Layer = OSDHdl;

    s32Ret = HI_MPI_RGN_Create(RgnHdl, &stRgnAttr);
    VPROC_CHECK_RET("HI_MPI_RGN_Create failed", s32Ret);

    s32Ret = HI_MPI_RGN_SetBitMap(RgnHdl, &pstOSDAttr->stBitmapAttr);
    VPROC_CHECK_RET("HI_MPI_RGN_SetBitMap failed", s32Ret);

    s32Ret = HI_MPI_RGN_AttachToChn(RgnHdl, pstChn, &stRgnChnAttr);
    VPROC_CHECK_RET("HI_MPI_RGN_AttachToChn failed", s32Ret);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetVpssOSDAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl,
                                    HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 RgnHdl;
    MPP_CHN_S stChn;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);
    VPROC_CHECK_NULL_PTR(pstOSDAttr);

    if ((HI_MAPI_RGN_BUF_MIN_NUM > pstOSDAttr->u32CanvasNum)
        || (pstOSDAttr->u32CanvasNum > HI_MAPI_RGN_BUF_MAX_NUM)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "vpss osd attr CanvasNum illegal is %d\n", pstOSDAttr->u32CanvasNum);
        return HI_MAPI_VPROC_EILLEGAL_PARAM;
    }

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].astPortOsdAttr[OSDHdl].bOsdStarted) {
        stChn.enModId = HI_ID_VPSS;
        stChn.s32DevId = VpssHdl;
        stChn.s32ChnId = VPortHdl;
        RgnHdl = MAPI_GetRgnHdl(VpssHdl, VPortHdl, OSDHdl, HI_ID_VPSS);

        s32Ret = VPROC_OSD_SetAttr(RgnHdl, &stChn, pstOSDAttr);
        VPROC_CHECK_RET_UNLOCK("VPROC_OSD_SetAttr failed", s32Ret, g_vprocLock);
    }

    memcpy(&g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].astPortOsdAttr[OSDHdl].stOsdAttr, pstOSDAttr,
           sizeof(HI_MAPI_OSD_ATTR_S));
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetVpssOSDAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl,
                                    HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);
    VPROC_CHECK_NULL_PTR(pstOSDAttr);

    MUTEX_LOCK(g_vprocLock);
    memcpy(pstOSDAttr, &g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].astPortOsdAttr[OSDHdl].stOsdAttr,
           sizeof(HI_MAPI_OSD_ATTR_S));
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StartVpssOSD(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    HI_U32 RgnHdl;
    MPP_CHN_S stChn;
    HI_MAPI_OSD_ATTR_S *pstOSDAttr;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].astPortOsdAttr[OSDHdl].bOsdStarted) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "vpss %d vport %d osd %d already been started\n", VpssHdl, VPortHdl, OSDHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    stChn.enModId = HI_ID_VPSS;
    stChn.s32DevId = VpssHdl;
    stChn.s32ChnId = VPortHdl;

    s32Ret = MAPI_AddRgnHdl(VpssHdl, VPortHdl, OSDHdl, HI_ID_VPSS);
    VPROC_CHECK_RET_UNLOCK("MAPI_AddRgnHdl failed", s32Ret, g_vprocLock);

    RgnHdl = MAPI_GetRgnHdl(VpssHdl, VPortHdl, OSDHdl, HI_ID_VPSS);
    pstOSDAttr = &g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].astPortOsdAttr[OSDHdl].stOsdAttr;

    s32Ret = VPROC_OSD_Start(RgnHdl, OSDHdl, &stChn, pstOSDAttr);
    VPROC_CHECK_RET_UNLOCK("VPROC_OSD_Start failed", s32Ret, g_vprocLock);

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].astPortOsdAttr[OSDHdl].bOsdStarted = HI_TRUE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StopVpssOSD(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    HI_U32 RgnHdl;
    MPP_CHN_S stChn;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);

    MUTEX_LOCK(g_vprocLock);

    if (!g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].astPortOsdAttr[OSDHdl].bOsdStarted) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "vpss %d vport %d osd %d already been stoped\n", VpssHdl, VPortHdl, OSDHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    stChn.enModId = HI_ID_VPSS;
    stChn.s32DevId = VpssHdl;
    stChn.s32ChnId = VPortHdl;
    RgnHdl = MAPI_GetRgnHdl(VpssHdl, VPortHdl, OSDHdl, HI_ID_VPSS);

    s32Ret = MAPI_DelRgnHdl(VpssHdl, VPortHdl, OSDHdl, HI_ID_VPSS);
    VPROC_CHECK_RET_UNLOCK("MAPI_DelRgnHdl failed", s32Ret, g_vprocLock);

    s32Ret = HI_MPI_RGN_DetachFromChn(RgnHdl, &stChn);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_RGN_DetachFromChn failed", s32Ret, g_vprocLock);

    s32Ret = HI_MPI_RGN_Destroy(RgnHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_RGN_Destroy failed", s32Ret, g_vprocLock);

    g_astVpssCtx[VpssHdl].astVpssPortAttr[VPortHdl].astPortOsdAttr[OSDHdl].bOsdStarted = HI_FALSE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetStitchOSDAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl,
                                      HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 RgnHdl;
    MPP_CHN_S stChn;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);
    VPROC_CHECK_NULL_PTR(pstOSDAttr);

    if ((HI_MAPI_RGN_BUF_MIN_NUM > pstOSDAttr->u32CanvasNum)
        || (pstOSDAttr->u32CanvasNum > HI_MAPI_RGN_BUF_MAX_NUM)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "vpss osd stitch attr CanvasNum illegal is %d\n", pstOSDAttr->u32CanvasNum);
        return HI_MAPI_VPROC_EILLEGAL_PARAM;
    }

    MUTEX_LOCK(g_vprocLock);

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].astPortOsdAttr[OSDHdl].bOsdStarted) {
        stChn.enModId = HI_ID_AVS;
        stChn.s32DevId = StitchHdl;
        stChn.s32ChnId = StitchPortHdl;
        RgnHdl = MAPI_GetRgnHdl(StitchHdl, StitchPortHdl, OSDHdl, HI_ID_AVS);

        s32Ret = VPROC_OSD_SetAttr(RgnHdl, &stChn, pstOSDAttr);
        VPROC_CHECK_RET_UNLOCK("VPROC_OSD_SetAttr failed", s32Ret, g_vprocLock);
    }

    memcpy(&g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].astPortOsdAttr[OSDHdl].stOsdAttr, pstOSDAttr,
           sizeof(HI_MAPI_OSD_ATTR_S));
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_GetStitchOSDAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl,
                                      HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
#ifdef SUPPORT_STITCH

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);
    VPROC_CHECK_NULL_PTR(pstOSDAttr);

    MUTEX_LOCK(g_vprocLock);
    memcpy(pstOSDAttr, &g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].astPortOsdAttr[OSDHdl].stOsdAttr,
           sizeof(HI_MAPI_OSD_ATTR_S));
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_StartStitchOSD(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    HI_U32 RgnHdl;
    MPP_CHN_S stChn;
    HI_MAPI_OSD_ATTR_S *pstOSDAttr;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);

    MUTEX_LOCK(g_vprocLock);

    if (g_astStitchCtx[StitchHdl].bStitchStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d not created.\n", StitchHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bStitchPortStart == HI_FALSE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d StitchPort %d not start.\n", StitchHdl, StitchPortHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].astPortOsdAttr[OSDHdl].bOsdStarted) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "stitch %d stitchport %d osd %d already been started\n", StitchHdl,
                        StitchPortHdl, OSDHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    stChn.enModId = HI_ID_AVS;
    stChn.s32DevId = StitchHdl;
    stChn.s32ChnId = StitchPortHdl;

    s32Ret = MAPI_AddRgnHdl(StitchHdl, StitchPortHdl, OSDHdl, HI_ID_AVS);
    VPROC_CHECK_RET_UNLOCK("MAPI_AddRgnHdl failed", s32Ret, g_vprocLock);

    RgnHdl = MAPI_GetRgnHdl(StitchHdl, StitchPortHdl, OSDHdl, HI_ID_AVS);
    pstOSDAttr = &g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].astPortOsdAttr[OSDHdl].stOsdAttr;

    s32Ret = VPROC_OSD_Start(RgnHdl, OSDHdl, &stChn, pstOSDAttr);
    VPROC_CHECK_RET_UNLOCK("VPROC_OSD_Start failed", s32Ret, g_vprocLock);

    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].astPortOsdAttr[OSDHdl].bOsdStarted = HI_TRUE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_StopStitchOSD(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    HI_U32 RgnHdl;
    MPP_CHN_S stChn;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);

    MUTEX_LOCK(g_vprocLock);
    if (g_astStitchCtx[StitchHdl].bStitchStart != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d not created.\n", StitchHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    if (g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].bStitchPortStart == HI_FALSE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "Stitch group %d StitchPort %d not start.\n", StitchHdl, StitchPortHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_MAPI_VPROC_EUNEXIST;
    }

    if (!g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].astPortOsdAttr[OSDHdl].bOsdStarted) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VPROC, "stitch %d stitchport %d osd %d already been stoped\n", StitchHdl,
                        StitchPortHdl, OSDHdl);
        MUTEX_UNLOCK(g_vprocLock);
        return HI_SUCCESS;
    }

    stChn.enModId = HI_ID_AVS;
    stChn.s32DevId = StitchHdl;
    stChn.s32ChnId = StitchPortHdl;
    RgnHdl = MAPI_GetRgnHdl(StitchHdl, StitchPortHdl, OSDHdl, HI_ID_AVS);

    s32Ret = MAPI_DelRgnHdl(StitchHdl, StitchPortHdl, OSDHdl, HI_ID_AVS);
    VPROC_CHECK_RET_UNLOCK("MAPI_DelRgnHdl failed", s32Ret, g_vprocLock);

    s32Ret = HI_MPI_RGN_DetachFromChn(RgnHdl, &stChn);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_RGN_DetachFromChn failed", s32Ret, g_vprocLock);

    s32Ret = HI_MPI_RGN_Destroy(RgnHdl);
    VPROC_CHECK_RET_UNLOCK("HI_MPI_RGN_Destroy failed", s32Ret, g_vprocLock);

    g_astStitchCtx[StitchHdl].astStitchPortAttr[StitchPortHdl].astPortOsdAttr[OSDHdl].bOsdStarted = HI_FALSE;
    MUTEX_UNLOCK(g_vprocLock);

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
