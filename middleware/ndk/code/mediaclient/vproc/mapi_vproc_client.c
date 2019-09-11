/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vproc_client.c
 * @brief   NDK vproc client functions
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hi_type.h"
#include "hi_math.h"
#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#ifdef SUPPORT_PHOTO_POST_PROCESS
#include "hi_comm_photo.h"
#endif
#include "mpi_sys.h"

#include "hi_mapi_sys.h"
#include "msg_vproc.h"
#include "msg_define.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_vproc_define.h"
#include "mapi_vproc_inner.h"
#include "hi_mapi_log.h"
#include "msg_wrapper.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static PORT_DUMP_YUV_INFO_S g_astVpssDumpYuvInfo[HI_MAPI_VPSS_MAX_NUM][HI_MAPI_VPORT_MAX_NUM];

#ifdef SUPPORT_PHOTO_POST_PROCESS
/* redefine a param to diff from vpss dump YUV */
static VPROC_PHOTO_DUMP_INFO_S g_astPhotoDumpInfo[HI_MAPI_VPSS_MAX_NUM][HI_MAPI_VPORT_MAX_NUM];
#endif

#ifdef SUPPORT_STITCH
static PORT_DUMP_YUV_INFO_S g_astStitchDumpYUVInfo[HI_MAPI_STITCH_MAX_NUM][HI_MAPI_STITCH_PORT_MAX_NUM];

HI_VOID MAPI_VPROC_Client_StitchReset(HI_VOID)
{
    HI_U32 i, j;

    for (i = 0; i < HI_MAPI_STITCH_MAX_NUM; i++) {
        for (j = 0; j < HI_MAPI_STITCH_PORT_MAX_NUM; j++) {
            g_astStitchDumpYUVInfo[i][j].bDumpYuvEnable = HI_FALSE;
            g_astStitchDumpYUVInfo[i][j].pthreadDumpYUV = -1;
            g_astStitchDumpYUVInfo[i][j].stCallbackFun.pfunVProcYUVProc = HI_NULL;
            g_astStitchDumpYUVInfo[i][j].bDumpYuvStartd = HI_FALSE;
            g_astStitchDumpYUVInfo[i][j].stCallbackFun.pPrivateData = HI_NULL;
        }
    }

    return;
}
#endif

HI_VOID MAPI_VPROC_Client_VpssReset(HI_VOID)
{
    HI_U32 i, j;
    for (i = 0; i < HI_MAPI_VPSS_MAX_NUM; i++) {
        for (j = 0; j < HI_MAPI_VPORT_MAX_NUM; j++) {
            g_astVpssDumpYuvInfo[i][j].bDumpYuvEnable = HI_FALSE;
            g_astVpssDumpYuvInfo[i][j].pthreadDumpYUV = -1;
            g_astVpssDumpYuvInfo[i][j].stCallbackFun.pfunVProcYUVProc = HI_NULL;
            g_astVpssDumpYuvInfo[i][j].bDumpYuvStartd = HI_FALSE;
            g_astVpssDumpYuvInfo[i][j].stCallbackFun.pPrivateData = HI_NULL;
#ifdef SUPPORT_PHOTO_POST_PROCESS
            g_astPhotoDumpInfo[i][j].bPhotoDump = HI_FALSE;
            g_astPhotoDumpInfo[i][j].stBNRRawCB.pfunVProcBNRProc = HI_NULL;
            g_astPhotoDumpInfo[i][j].stBNRRawCB.pPrivateData = HI_NULL;
            g_astPhotoDumpInfo[i][j].stYUVDataCB.pfunVProcYUVProc = HI_NULL;
            g_astPhotoDumpInfo[i][j].stYUVDataCB.pPrivateData = HI_NULL;
#endif
        }
    }

    return;
}
HI_S32 MAPI_VPROC_Client_Init(HI_VOID)
{
    HI_U32 i, j;

#ifdef SUPPORT_PHOTO_POST_PROCESS
    for (i = 0; i < HI_MAPI_VPSS_MAX_NUM; i++) {
        for (j = 0; j < HI_MAPI_VPORT_MAX_NUM; j++) {
            MUTEX_INIT_LOCK(g_astPhotoDumpInfo[i][j].PhotoDump_Lock);
        }
    }
#endif

    MAPI_VPROC_Client_VpssReset();
#ifdef SUPPORT_STITCH
    MAPI_VPROC_Client_StitchReset();
#endif

    return HI_SUCCESS;
}

HI_S32 MAPI_VPROC_Client_Deinit(HI_VOID)
{
    HI_U32 i, j;

    MAPI_VPROC_Client_VpssReset();
#ifdef SUPPORT_STITCH
    MAPI_VPROC_Client_StitchReset();
#endif

#ifdef SUPPORT_PHOTO_POST_PROCESS
    for (i = 0; i < HI_MAPI_VPSS_MAX_NUM; i++) {
        for (j = 0; j < HI_MAPI_VPORT_MAX_NUM; j++) {
            MUTEX_DESTROY(g_astPhotoDumpInfo[i][j].PhotoDump_Lock);
        }
    }
#endif

    return HI_SUCCESS;
}

HI_S32 VPROC_GetChnFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                         HI_MAPI_FRAME_DATA_S *pstYUVFrameData)
{
    HI_S32 s32Ret;
    HI_U32 u32Size = 0;
    HI_U32 u32ModFd;
    HI_VOID *VirAddr;
    MAPI_PRIV_DATA_S stPrivData;

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, GrpHdl, PortHdl);

    if (bStitch) {
        stPrivData.as32PrivData[0] = HI_ID_AVS;
    } else {
        stPrivData.as32PrivData[0] = HI_ID_VPSS;
    }

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_CHN_FRAME, pstYUVFrameData, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == pstYUVFrameData->enPixelFormat) {
        u32Size = (pstYUVFrameData->u32Stride[0] * pstYUVFrameData->u32Height) * 3 / 2;

        VirAddr = HI_MPI_SYS_Mmap(pstYUVFrameData->u64PhyAddr[0], u32Size);
        if (VirAddr == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_VPROC_Mmap fail!\n");
            return HI_FAILURE;
        }

        pstYUVFrameData->u64VirAddr[0] = (HI_UL)VirAddr;
    } else if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == pstYUVFrameData->enPixelFormat) {
        u32Size = (pstYUVFrameData->u32Stride[0] * pstYUVFrameData->u32Height) * 2;

        VirAddr = HI_MPI_SYS_Mmap(pstYUVFrameData->u64PhyAddr[0], u32Size);
        if (VirAddr == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_VPROC_Mmap fail!\n");
            return HI_FAILURE;
        }

        pstYUVFrameData->u64VirAddr[0] = (HI_UL)VirAddr;
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "enPixelFormat is unsupport!!\n");
    }

    return HI_SUCCESS;
}

#ifdef SUPPORT_PHOTO_POST_PROCESS
static HI_S32 VPROC_PhotoProcess_DumpYUV(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                         VPROC_PHOTO_DUMP_DATA_S stPhotoDump)
{
    HI_U32 u32Size = 0;
    HI_VOID *VirAddr;
    HI_S32 s32Ret;

    /* dump YUV */
    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == stPhotoDump.stYUVData.enPixelFormat) {
        u32Size = (stPhotoDump.stYUVData.u32Stride[0] * stPhotoDump.stYUVData.u32Height) * 3 / 2;
    } else if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == stPhotoDump.stYUVData.enPixelFormat) {
        u32Size = (stPhotoDump.stYUVData.u32Stride[0] * stPhotoDump.stYUVData.u32Height) * 2;
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "PixelFormat error! current pixelFormat is %d\n",
                       stPhotoDump.stYUVData.enPixelFormat);
        return HI_FAILURE;
    }

    VirAddr = HI_MPI_SYS_Mmap(stPhotoDump.stYUVData.u64PhyAddr[0], u32Size);
    if (VirAddr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Mmap YUV PhyAddr fail!\n");
        return HI_FAILURE;
    }

    stPhotoDump.stYUVData.u64VirAddr[0] = (HI_U64)(HI_UL)VirAddr;
    if (g_astPhotoDumpInfo[VpssHdl][VPortHdl].stYUVDataCB.pfunVProcYUVProc != HI_NULL) {
        g_astPhotoDumpInfo[VpssHdl][VPortHdl].stYUVDataCB.pfunVProcYUVProc(VpssHdl, VPortHdl,
            &stPhotoDump.stYUVData, g_astPhotoDumpInfo[VpssHdl][VPortHdl].stYUVDataCB.pPrivateData);
    }

    s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)(HI_UL)stPhotoDump.stYUVData.u64PhyAddr[0], u32Size);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Munmap YUV Addr fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 VPROC_PhotoProcess_DumpBNR(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                         VPROC_PHOTO_DUMP_DATA_S stPhotoDump)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Size = 0;

    if ((stPhotoDump.enPhotoType == VPROC_PHOTO_TYPE_MFNR_DE) || (stPhotoDump.enPhotoType == VPROC_PHOTO_TYPE_DE)) {
        /* dump BNR raw */
        u32Size = (stPhotoDump.stBNRRaw.u32Stride[0] * stPhotoDump.stBNRRaw.u32Height);

        stPhotoDump.stBNRRaw.u64VirAddr[0] = (HI_U64)(HI_UL)HI_MPI_SYS_Mmap(stPhotoDump.stBNRRaw.u64PhyAddr[0],
                                                                            u32Size);
        if (stPhotoDump.stBNRRaw.u64VirAddr[0] == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Mmap BNR VirAddr fail!\n");
            return HI_FAILURE;
        }

        stPhotoDump.stBNRRaw.stVideoSupplement.pFrameDNGVirAddr =
            HI_MPI_SYS_Mmap(stPhotoDump.stBNRRaw.stVideoSupplement.u64FrameDNGPhyAddr,
                            sizeof(DNG_IMAGE_DYNAMIC_INFO_S));

        if (stPhotoDump.stBNRRaw.stVideoSupplement.pFrameDNGVirAddr == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Mmap BNR DNG Addr fail!\n");
            return HI_FAILURE;
        }

        stPhotoDump.stBNRRaw.stVideoSupplement.pJpegDCFVirAddr =
            HI_MPI_SYS_Mmap(stPhotoDump.stBNRRaw.stVideoSupplement.u64JpegDCFPhyAddr,
                            sizeof(JPEG_DCF_S));

        if (stPhotoDump.stBNRRaw.stVideoSupplement.pJpegDCFVirAddr == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Mmap BNR DCF Addr fail!\n");
            return HI_FAILURE;
        }

        if (g_astPhotoDumpInfo[VpssHdl][VPortHdl].stBNRRawCB.pfunVProcBNRProc != NULL) {
            g_astPhotoDumpInfo[VpssHdl][VPortHdl].stBNRRawCB.pfunVProcBNRProc(VpssHdl, VPortHdl,
                &stPhotoDump.stBNRRaw, g_astPhotoDumpInfo[VpssHdl][VPortHdl].stBNRRawCB.pPrivateData);
        }

        s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)(HI_UL)stPhotoDump.stBNRRaw.u64VirAddr[0], u32Size);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Munmap BNR VirAddr fail s32Ret:%x\n", s32Ret);
            return s32Ret;
        }

        s32Ret = HI_MPI_SYS_Munmap(stPhotoDump.stBNRRaw.stVideoSupplement.pFrameDNGVirAddr,
                                   sizeof(DNG_IMAGE_DYNAMIC_INFO_S));
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Munmap BNR DNG Addr fail s32Ret:%x\n", s32Ret);
            return s32Ret;
        }

        s32Ret = HI_MPI_SYS_Munmap(stPhotoDump.stBNRRaw.stVideoSupplement.pJpegDCFVirAddr, sizeof(JPEG_DCF_S));
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Munmap BNR DCF Addr fail s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}
#endif

HI_S32 VPROC_ReleaseChnFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                             HI_MAPI_FRAME_DATA_S *pstYUVFrameData)
{
    HI_S32 s32Ret;
    HI_U32 u32Size = 0;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, GrpHdl, PortHdl);

    if (bStitch) {
        stPrivData.as32PrivData[0] = HI_ID_AVS;
    } else {
        stPrivData.as32PrivData[0] = HI_ID_VPSS;
    }

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_RELEASE_CHN_FRAME,
                           pstYUVFrameData, sizeof(HI_MAPI_FRAME_DATA_S), &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == pstYUVFrameData->enPixelFormat) {
        u32Size = (pstYUVFrameData->u32Stride[0] * pstYUVFrameData->u32Height) * 3 / 2;

        s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)(HI_UL)pstYUVFrameData->u64VirAddr[0], u32Size);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_VPROC_Munmap fail s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
    } else if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == pstYUVFrameData->enPixelFormat) {
        u32Size = (pstYUVFrameData->u32Stride[0] * pstYUVFrameData->u32Height) * 2;

        s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)(HI_UL)pstYUVFrameData->u64VirAddr[0], u32Size);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_VPROC_Munmap fail s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "enPixelFormat is unsupport!\n");
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetPortFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                                  HI_MAPI_FRAME_DATA_S *pstYUVFrameData)
{
    HI_S32 s32Ret;

    VPROC_CHECK_NULL_PTR(pstYUVFrameData);

    if (bStitch) {
        STITCH_CHECK_HANDLE(GrpHdl);
        STITCHPORT_CHECK_HANDLE(PortHdl);
    } else {
        VPSS_CHECK_HANDLE(GrpHdl);
        VPORT_CHECK_HANDLE(PortHdl);
    }

    s32Ret = VPROC_GetChnFrame(GrpHdl, PortHdl, bStitch, pstYUVFrameData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VPROC_GetChnFrame fail s32Ret:%x\n", s32Ret);
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
        STITCH_CHECK_HANDLE(GrpHdl);
        STITCHPORT_CHECK_HANDLE(PortHdl);
    } else {
        VPSS_CHECK_HANDLE(GrpHdl);
        VPORT_CHECK_HANDLE(PortHdl);
    }

    s32Ret = VPROC_ReleaseChnFrame(GrpHdl, PortHdl, bStitch, pstYUVFrameData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VPROC_ReleaseChnFrame fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_InitVpss(HI_HANDLE VpssHdl, HI_MAPI_VPSS_ATTR_S *pstVpssAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPROC_CHECK_NULL_PTR(pstVpssAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_INIT_VPSS, pstVpssAttr, sizeof(HI_MAPI_VPSS_ATTR_S), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_DeinitVpss(HI_HANDLE VpssHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    VPSS_CHECK_HANDLE(VpssHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_DEINIT_VPSS, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_BindVcap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE VpssHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VCAPPIPE_CHECK_HANDLE(VcapPipeHdl);
    VCAPCHN_CHECK_HANDLE(PipeChnHdl);
    VPSS_CHECK_HANDLE(VpssHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, 0);
    stPrivData.as32PrivData[0] = VcapPipeHdl;
    stPrivData.as32PrivData[1] = PipeChnHdl;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_BIND_VCAP, HI_NULL, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_UnBindVCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE VpssHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VCAPPIPE_CHECK_HANDLE(VcapPipeHdl);
    VCAPCHN_CHECK_HANDLE(PipeChnHdl);
    VPSS_CHECK_HANDLE(VpssHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, 0);
    stPrivData.as32PrivData[0] = VcapPipeHdl;
    stPrivData.as32PrivData[1] = PipeChnHdl;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_UNBIND_VCAP, HI_NULL, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetPortAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_VPORT_ATTR_S *pstVPortAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstVPortAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_SET_VPORT_ATTR, pstVPortAttr, sizeof(HI_MAPI_VPORT_ATTR_S),
                           HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetPortAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_VPORT_ATTR_S *pstVPortAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstVPortAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_VPORT_ATTR, pstVPortAttr, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StartPort(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_START_VPORT, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StopPort(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_STOP_VPORT, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetPortAttrEx(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                   HI_MAPI_VPROC_CMD_E enCMD, HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    stPrivData.as32PrivData[0] = enCMD;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_SET_VPORT_ATTREX, pAttr, u32Len, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetPortAttrEx(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                   HI_MAPI_VPROC_CMD_E enCMD, HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    stPrivData.as32PrivData[0] = enCMD;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_VPORT_ATTREX, pAttr, u32Len, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

#ifdef SUPPORT_STITCH
HI_S32 HI_MAPI_VPROC_CreateStitch(HI_HANDLE StitchHdl, HI_MAPI_STITCH_ATTR_S *pstStitchAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    STITCH_CHECK_HANDLE(StitchHdl);
    VPROC_CHECK_NULL_PTR(pstStitchAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_CREAT_STITCH, pstStitchAttr, sizeof(HI_MAPI_STITCH_ATTR_S),
                           HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_DestroyStitch(HI_HANDLE StitchHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    STITCH_CHECK_HANDLE(StitchHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_DESTROY_STITCH, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetStitchAttr(HI_HANDLE StitchHdl, HI_MAPI_STITCH_ATTR_S *pstStitchAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    STITCH_CHECK_HANDLE(StitchHdl);
    VPROC_CHECK_NULL_PTR(pstStitchAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_SET_STITCH_ATTR,
                           pstStitchAttr, sizeof(HI_MAPI_STITCH_ATTR_S), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetStitchAttr(HI_HANDLE StitchHdl, HI_MAPI_STITCH_ATTR_S *pstStitchAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    STITCH_CHECK_HANDLE(StitchHdl);
    VPROC_CHECK_NULL_PTR(pstStitchAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_STITCH_ATTR, pstStitchAttr, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetStitchPortAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                       HI_MAPI_STITCH_PORT_ATTR_S *pstStitchPortAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_CHECK_NULL_PTR(pstStitchPortAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, StitchPortHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_SET_STITCH_PORT_ATTR,
                           pstStitchPortAttr, sizeof(HI_MAPI_STITCH_PORT_ATTR_S), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetStitchPortAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                       HI_MAPI_STITCH_PORT_ATTR_S *pstStitchPortAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_CHECK_NULL_PTR(pstStitchPortAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, StitchPortHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_STITCH_PORT_ATTR, pstStitchPortAttr, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StartStitchPort(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, StitchPortHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_START_STITCH_PORT, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StopStitchPort(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, StitchPortHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_STOP_STITCH_PORT, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}
#endif

#ifdef SUPPORT_PHOTO_POST_PROCESS
HI_S32 HI_MAPI_VPROC_RegPhotoDumpCB(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                    HI_PHOTO_DUMP_CALLBACK_FUNC_S *pstDumpCB)
{
    VPSS_CHECK_HANDLE(VpssHdl);
    VPSS_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpCB);

    MUTEX_LOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
    if ((pstDumpCB->stBNRRawCB.pfunVProcBNRProc == NULL) && (pstDumpCB->stYUVDataCB.pfunVProcYUVProc == NULL)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC,
                       "Reg PhotoDump Callback input BNR raw and YUV CB function ptr is both NULL!!\n");

        g_astPhotoDumpInfo[VpssHdl][VPortHdl].bPhotoDump = HI_FALSE;
        MUTEX_UNLOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
        return HI_MAPI_VPROC_EILLEGAL_PARAM;
    }

    if (pstDumpCB->stBNRRawCB.pfunVProcBNRProc != NULL) {
        memcpy(&g_astPhotoDumpInfo[VpssHdl][VPortHdl].stBNRRawCB,
               &pstDumpCB->stBNRRawCB, sizeof(HI_DUMP_BNRRAW_CALLBACK_FUNC_S));
        g_astPhotoDumpInfo[VpssHdl][VPortHdl].bPhotoDump = HI_TRUE;
    }

    if (pstDumpCB->stYUVDataCB.pfunVProcYUVProc != NULL) {
        memcpy(&g_astPhotoDumpInfo[VpssHdl][VPortHdl].stYUVDataCB,
               &pstDumpCB->stYUVDataCB, sizeof(HI_DUMP_YUV_CALLBACK_FUNC_S));
        g_astPhotoDumpInfo[VpssHdl][VPortHdl].bPhotoDump = HI_TRUE;
    }

    MUTEX_UNLOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_UnRegPhotoDumpCB(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    VPSS_CHECK_HANDLE(VpssHdl);
    VPSS_CHECK_HANDLE(VPortHdl);

    MUTEX_LOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
    memset(&g_astPhotoDumpInfo[VpssHdl][VPortHdl].stBNRRawCB, 0x00, sizeof(HI_DUMP_BNRRAW_CALLBACK_FUNC_S));
    memset(&g_astPhotoDumpInfo[VpssHdl][VPortHdl].stYUVDataCB, 0x00, sizeof(HI_DUMP_YUV_CALLBACK_FUNC_S));

    g_astPhotoDumpInfo[VpssHdl][VPortHdl].bPhotoDump = HI_FALSE;
    MUTEX_UNLOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_InitPhoto(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_PHOTO_TYPE_E enPhotoType)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    stPrivData.as32PrivData[0] = enPhotoType;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_INIT_PHOTO, HI_NULL, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_DeinitPhoto(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_DEINIT_PHOTO, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_PhotoProcess(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

#ifdef SUPPORT_PHOTO_POST_PROCESS
    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);

    if (g_astPhotoDumpInfo[VpssHdl][VPortHdl].bPhotoDump == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Ready to enter photo process in normal Mode...\n");

        u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);

        s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_PHOTO_PROCESS, HI_NULL, 0, HI_NULL);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Ready to enter photo process in debug Mode...\n");

        /* in case photo process twice continously in little time */
        MUTEX_LOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
        VPROC_PHOTO_DUMP_DATA_S stPhotoDump;
        HI_S32 s32FrameCnt = 0;
        HI_S32 i = 0;

        memset(&stPhotoDump, 0x00, sizeof(VPROC_PHOTO_DUMP_DATA_S));

        /* get bnr raw in debug mode */
        s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_PHOTO_PROCESS_GETBNR, &stPhotoDump, 0, HI_NULL);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MSG_CMD_VPROC_PHOTO_PROCESS_GETBNR fail s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
            return s32Ret;
        }

        s32Ret = VPROC_PhotoProcess_DumpBNR(VpssHdl, VPortHdl, stPhotoDump);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VPROC_Photo_DumpYUV fail s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
            return s32Ret;
        }

        s32FrameCnt = stPhotoDump.s32FrameCnt;

        for (i = 0; i < s32FrameCnt; i++) {
            memset(&stPhotoDump.stYUVData, 0, sizeof(HI_MAPI_FRAME_DATA_S));

            /* get dump yuv in debug mode */
            s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_PHOTO_PROCESS_GETYUV, &stPhotoDump, 0, HI_NULL);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MSG_CMD_VPROC_PHOTO_PROCESS_GETYUV fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
                return s32Ret;
            }

            s32Ret = VPROC_PhotoProcess_DumpYUV(VpssHdl, VPortHdl, stPhotoDump);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VPROC_Photo_DumpYUV fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
                return s32Ret;
            }

            /* process frame in debug mode */
            s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_PHOTO_PROCESS_ALGPROCESS, HI_NULL, 0, HI_NULL);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MSG_CMD_VPROC_PHOTO_PROCESS_ALGPROCESS fail s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
                return s32Ret;
            }
        }

        /* send the frame to venc to encode */
        s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_PHOTO_PROCESS_SEND, HI_NULL, 0, HI_NULL);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MSG_CMD_VPROC_PHOTO_PROCESS_SEND fail s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
            return s32Ret;
        }

        MUTEX_UNLOCK(g_astPhotoDumpInfo[VpssHdl][VPortHdl].PhotoDump_Lock);
    }
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support photo dump bnr raw.\n");
#endif

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetCurrentPhotoType(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
    HI_MAPI_PHOTO_TYPE_E *penPhotoType)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(penPhotoType);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_CURRENT_PHOTO_TYPE, penPhotoType, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetPhotoAlgCoef(PHOTO_ALG_TYPE_E enAlgType, const PHOTO_ALG_COEF_S *pstAlgCoef)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    PHOTO_ALG_COEF_S stAlgCoef;
    MAPI_PRIV_DATA_S stPrivData;
    VPROC_CHECK_NULL_PTR(pstAlgCoef);

    memcpy(&stAlgCoef, pstAlgCoef, sizeof(PHOTO_ALG_COEF_S));

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, 0, 0);
    stPrivData.as32PrivData[0] = enAlgType;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_SET_PHOTO_ALGCOEF,
                           &stAlgCoef, sizeof(PHOTO_ALG_COEF_S), &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetPhotoAlgCoef(PHOTO_ALG_TYPE_E enAlgType, PHOTO_ALG_COEF_S *pstAlgCoef)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VPROC_CHECK_NULL_PTR(pstAlgCoef);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, 0, 0);
    stPrivData.as32PrivData[0] = enAlgType;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_PHOTO_ALGCOEF, pstAlgCoef, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}
#endif

HI_VOID *VpssDumpYUVthread(void *pArg)
{
    HI_S32 s32Ret;
    HI_S32 i, s32Count;
    HI_HANDLE VpssHdl;
    HI_HANDLE VPortHdl;
    VProcDumpYuvThreadArg_S *pstArg;
    HI_MAPI_FRAME_DATA_S stYUVFrameData = {0};

    prctl(PR_SET_NAME, (unsigned long)"Hi_pTDumpYUV", 0, 0, 0);
    pstArg = (VProcDumpYuvThreadArg_S *)pArg;
    VpssHdl = pstArg->GrpHdl;
    VPortHdl = pstArg->PortHdl;
    s32Count = pstArg->s32Count;

    if (s32Count >= 0) {
        for (i = 0; i < s32Count; i++) {
            s32Ret = VPROC_GetChnFrame(VpssHdl, VPortHdl, HI_FALSE, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "get vpss chn frame timeout.\n");
                continue;
            }

            g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].stCallbackFun.pfunVProcYUVProc(VpssHdl, VPortHdl,
                &stYUVFrameData, g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].stCallbackFun.pPrivateData);

            s32Ret = VPROC_ReleaseChnFrame(VpssHdl, VPortHdl, HI_FALSE, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Release vpss chn frame error.\n");
            }
        }
    } else if (-1 == s32Count) {
        while (g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].bDumpYuvStartd) {
            s32Ret = VPROC_GetChnFrame(VpssHdl, VPortHdl, HI_FALSE, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "get vpss chn frame timeout.\n");
                continue;
            }

            g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].stCallbackFun.pfunVProcYUVProc(VpssHdl, VPortHdl,
                &stYUVFrameData, g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].stCallbackFun.pPrivateData);

            s32Ret = VPROC_ReleaseChnFrame(VpssHdl, VPortHdl, HI_FALSE, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Release vpss chn frame error.\n");
            }
        }
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss Dump YUV input count is unsupport.\n");
        return NULL;
    }

    return NULL;
}

#ifdef SUPPORT_STITCH

HI_VOID *StitchDumpYUVthread(void *pArg)
{
    HI_S32 s32Ret;
    HI_S32 i, s32Count;
    HI_HANDLE StitchHdl;
    HI_HANDLE StitchPortHdl;
    VProcDumpYuvThreadArg_S *pstArg;
    HI_MAPI_FRAME_DATA_S stYUVFrameData = {0};

    prctl(PR_SET_NAME, (unsigned long)"Hi_pTDumpYUV", 0, 0, 0);
    pstArg = (VProcDumpYuvThreadArg_S *)pArg;
    StitchHdl = pstArg->GrpHdl;
    StitchPortHdl = pstArg->PortHdl;
    s32Count = pstArg->s32Count;

    if (s32Count >= 0) {
        for (i = 0; i < s32Count; i++) {
            s32Ret = VPROC_GetChnFrame(StitchHdl, StitchPortHdl, HI_TRUE, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "get vpss chn frame timeout.\n");
                continue;
            }

            g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].stCallbackFun.pfunVProcYUVProc(StitchHdl, StitchPortHdl,
                &stYUVFrameData, g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].stCallbackFun.pPrivateData);

            s32Ret = VPROC_ReleaseChnFrame(StitchHdl, StitchPortHdl, HI_TRUE, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Release vpss chn frame error.\n");
            }
        }
    } else if (-1 == s32Count) {
        while (g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].bDumpYuvStartd) {
            s32Ret = VPROC_GetChnFrame(StitchHdl, StitchPortHdl, HI_TRUE, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "get vpss chn frame timeout.\n");
                continue;
            }

            g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].stCallbackFun.pfunVProcYUVProc(StitchHdl, StitchPortHdl,
                &stYUVFrameData, g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].stCallbackFun.pPrivateData);

            s32Ret = VPROC_ReleaseChnFrame(StitchHdl, StitchPortHdl, HI_TRUE, &stYUVFrameData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Release vpss chn frame error.\n");
            }
        }
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch Dump YUV input count is unsupport.\n");
        return NULL;
    }

    return NULL;
}

#endif

HI_S32 HI_MAPI_VPROC_SetVpssDumpYUVAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                        HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpYUVAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_VPSS;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_SET_DUMP_YUV_ATTR, pstDumpYUVAttr,
                           sizeof(HI_MAPI_DUMP_YUV_ATTR_S), &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].bDumpYuvEnable = pstDumpYUVAttr->bEnable;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetVpssDumpYUVAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                        HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpYUVAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_VPSS;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_DUMP_YUV_ATTR, pstDumpYUVAttr, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StartVpssDumpYUV(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_S32 s32Count,
                                      HI_DUMP_YUV_CALLBACK_FUNC_S *pstCallbackFun)
{
    HI_S32 s32Ret;
    HI_MAPI_DUMP_YUV_ATTR_S stDumpYUVAttr;
    static VProcDumpYuvThreadArg_S stVpssDumpArg[HI_MAPI_VPSS_MAX_NUM][HI_MAPI_VPORT_MAX_NUM];

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_CHECK_NULL_PTR(pstCallbackFun);
    VPROC_CHECK_NULL_PTR(pstCallbackFun->pPrivateData);

    if (s32Count == 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss DumpYUV input is ereor.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    if (g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].bDumpYuvEnable != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VProc dump YUV has not been enabled.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    memset(&stDumpYUVAttr, 0x00, sizeof(HI_MAPI_DUMP_YUV_ATTR_S));
    s32Ret = HI_MAPI_VPROC_GetVpssDumpYUVAttr(VpssHdl, VPortHdl, &stDumpYUVAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Start Vpss dumpYUV call GetVpssDumpYUVAttr error.\n");
        return s32Ret;
    }

    if (stDumpYUVAttr.u32Depth == 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss can't support dump YUV when the depth is zero.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    if (g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].stCallbackFun.pfunVProcYUVProc != HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss DumpYUV is busy,please wait.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].stCallbackFun.pfunVProcYUVProc = pstCallbackFun->pfunVProcYUVProc;
    g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].stCallbackFun.pPrivateData = pstCallbackFun->pPrivateData;

    stVpssDumpArg[VpssHdl][VPortHdl].GrpHdl = VpssHdl;
    stVpssDumpArg[VpssHdl][VPortHdl].PortHdl = VPortHdl;
    stVpssDumpArg[VpssHdl][VPortHdl].s32Count = s32Count;
    g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].bDumpYuvStartd = HI_TRUE;

    s32Ret = pthread_create(&g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].pthreadDumpYUV, HI_NULL,
                            VpssDumpYUVthread, (void *)&stVpssDumpArg[VpssHdl][VPortHdl]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Create vproc dump yuv thread error.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StopVpssDumpYUV(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl)
{
    HI_S32 s32Ret;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);

    if (g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].bDumpYuvEnable != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VProc dump YUV has not been enabled.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].bDumpYuvStartd = HI_FALSE;
    if ((pthread_t)-1 != g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].pthreadDumpYUV) {
        pthread_join(g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].pthreadDumpYUV, HI_NULL);
    }

    g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].pthreadDumpYUV = -1;
    g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].stCallbackFun.pfunVProcYUVProc = HI_NULL;
    g_astVpssDumpYuvInfo[VpssHdl][VPortHdl].stCallbackFun.pPrivateData = HI_NULL;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetStitchDumpYUVAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                          HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr)
{
#ifdef SUPPORT_STITCH

    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpYUVAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, StitchPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_AVS;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_SET_DUMP_YUV_ATTR, pstDumpYUVAttr,
                           sizeof(HI_MAPI_DUMP_YUV_ATTR_S), &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].bDumpYuvEnable = pstDumpYUVAttr->bEnable;

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
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_CHECK_NULL_PTR(pstDumpYUVAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, StitchPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_AVS;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_DUMP_YUV_ATTR, pstDumpYUVAttr, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

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
    HI_MAPI_DUMP_YUV_ATTR_S stDumpYUVAttr;
    static VProcDumpYuvThreadArg_S stStitchDumpArg[HI_MAPI_STITCH_MAX_NUM][HI_MAPI_STITCH_PORT_MAX_NUM];

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_CHECK_NULL_PTR(pstCallbackFun);
    VPROC_CHECK_NULL_PTR(pstCallbackFun->pfunVProcYUVProc);

    if (s32Count == 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Stitch DumpYUV input is error.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    if (g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].bDumpYuvEnable != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VProc dump YUV has not been enabled.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }

    memset(&stDumpYUVAttr, 0x00, sizeof(HI_MAPI_DUMP_YUV_ATTR_S));
    s32Ret = HI_MAPI_VPROC_GetStitchDumpYUVAttr(StitchHdl, StitchPortHdl, &stDumpYUVAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Start Vpss stitch dumpYUV call GetStitchDumpYUVAttr error.\n");
        return s32Ret;
    }

    if (stDumpYUVAttr.u32Depth == 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss can't support dump stitch YUV when the depth is zero.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    if (g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].stCallbackFun.pfunVProcYUVProc != HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Vpss DumpYUV is busy,please wait.\n");
        return HI_MAPI_VPROC_ENOT_SUPPORT;
    }

    g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].stCallbackFun.pfunVProcYUVProc = pstCallbackFun->pfunVProcYUVProc;
    g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].stCallbackFun.pPrivateData = pstCallbackFun->pPrivateData;

    stStitchDumpArg[StitchHdl][StitchPortHdl].GrpHdl = StitchHdl;
    stStitchDumpArg[StitchHdl][StitchPortHdl].PortHdl = StitchPortHdl;
    stStitchDumpArg[StitchHdl][StitchPortHdl].s32Count = s32Count;
    g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].bDumpYuvStartd = HI_TRUE;

    s32Ret = pthread_create(&g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].pthreadDumpYUV, HI_NULL,
                            StitchDumpYUVthread, (void *)&stStitchDumpArg[StitchHdl][StitchPortHdl]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Create vproc dump yuv thread error.\n");
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

    HI_S32 s32Ret;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);

    if (g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].bDumpYuvEnable != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "VProc dump YUV has not been enabled.\n");
        return HI_MAPI_VPROC_ENOTREADY;
    }
    g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].bDumpYuvStartd = HI_FALSE;
    if (g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].pthreadDumpYUV != (pthread_t)-1) {
        pthread_join(g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].pthreadDumpYUV, HI_NULL);
    }

    g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].pthreadDumpYUV = -1;
    g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].stCallbackFun.pfunVProcYUVProc = NULL;
    g_astStitchDumpYUVInfo[StitchHdl][StitchPortHdl].stCallbackFun.pPrivateData = NULL;

    return HI_SUCCESS;

#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "Not support stitch.\n");
    return HI_MAPI_VPROC_ENOT_SUPPORT;
#endif
}

HI_S32 HI_MAPI_VPROC_SetVpssOSDAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl,
                                    HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);
    VPROC_CHECK_NULL_PTR(pstOSDAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_VPSS;
    stPrivData.as32PrivData[1] = OSDHdl;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_SET_OSD_ATTR, pstOSDAttr, sizeof(HI_MAPI_OSD_ATTR_S), &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetVpssOSDAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl,
                                    HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);
    VPROC_CHECK_NULL_PTR(pstOSDAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_VPSS;
    stPrivData.as32PrivData[1] = OSDHdl;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_OSD_ATTR, pstOSDAttr, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StartVpssOSD(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_VPSS;
    stPrivData.as32PrivData[1] = OSDHdl;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_START_OSD, HI_NULL, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StopVpssOSD(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    VPSS_CHECK_HANDLE(VpssHdl);
    VPORT_CHECK_HANDLE(VPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, VpssHdl, VPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_VPSS;
    stPrivData.as32PrivData[1] = OSDHdl;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_STOP_OSD, HI_NULL, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_SetStitchOSDAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl,
                                      HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);
    VPROC_CHECK_NULL_PTR(pstOSDAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, StitchPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_AVS;
    stPrivData.as32PrivData[1] = OSDHdl;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_SET_OSD_ATTR, pstOSDAttr, sizeof(HI_MAPI_OSD_ATTR_S), &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_GetStitchOSDAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl,
                                      HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);
    VPROC_CHECK_NULL_PTR(pstOSDAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, StitchPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_AVS;
    stPrivData.as32PrivData[1] = OSDHdl;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_GET_OSD_ATTR, pstOSDAttr, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StartStitchOSD(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, StitchPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_AVS;
    stPrivData.as32PrivData[1] = OSDHdl;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_START_OSD, HI_NULL, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VPROC_StopStitchOSD(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    STITCH_CHECK_HANDLE(StitchHdl);
    STITCHPORT_CHECK_HANDLE(StitchPortHdl);
    VPROC_OSD_CHECK_HANDLE(OSDHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_VPROC, StitchHdl, StitchPortHdl);
    stPrivData.as32PrivData[0] = HI_ID_AVS;
    stPrivData.as32PrivData[1] = OSDHdl;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VPROC_STOP_OSD, HI_NULL, 0, &stPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VPROC, "MAPI_SendSync fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
