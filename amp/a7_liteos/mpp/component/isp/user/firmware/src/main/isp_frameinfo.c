/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_frameinfo.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       :
  Description   :
  History       :
  1.Date        :
    Author      :
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <sys/ioctl.h>
#include "mkp_isp.h"
#include "mpi_sys.h"
#include "isp_frameinfo.h"
#include "isp_main.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern HI_S32 g_as32IspFd[ISP_MAX_PIPE_NUM];

HI_S32 ISP_TransInfoInit(VI_PIPE ViPipe, ISP_TRANS_INFO_BUF_S *pstTransInfo)
{
    HI_S32 s32Ret;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstTransInfo);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_TRANS_BUF_INIT, pstTransInfo);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "isp[%d] init trans info bufs failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_TransInfoExit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_CHECK_PIPE(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_TRANS_BUF_EXIT);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "isp[%d] exit trans info buf failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_FrameInfoInit(VI_PIPE ViPipe)
{
    HI_U64 u64PhyAddr;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    u64PhyAddr = pstIspCtx->stIspTransInfo.stFrameInfo.u64PhyAddr;

    pstIspCtx->stFrameInfoCtrl.pstIspFrame = HI_MPI_SYS_Mmap(u64PhyAddr,
                                                             sizeof(ISP_FRAME_INFO_S) * ISP_MAX_FRAMEINFO_BUF_NUM);

    if (HI_NULL == pstIspCtx->stFrameInfoCtrl.pstIspFrame) {
        ISP_TRACE(HI_DBG_ERR, "isp[%d] mmap frame info buf failed!\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_FrameInfoExit(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (HI_NULL != pstIspCtx->stFrameInfoCtrl.pstIspFrame) {
        HI_MPI_SYS_Munmap(pstIspCtx->stFrameInfoCtrl.pstIspFrame, sizeof(ISP_FRAME_INFO_S) * ISP_MAX_FRAMEINFO_BUF_NUM);
        pstIspCtx->stFrameInfoCtrl.pstIspFrame = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_AttachInfoInit(VI_PIPE ViPipe)
{
    HI_U64 u64PhyAddr;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    u64PhyAddr = pstIspCtx->stIspTransInfo.stAttaInfo.u64PhyAddr;

    pstIspCtx->stAttachInfoCtrl.pstAttachInfo = HI_MPI_SYS_Mmap(u64PhyAddr, sizeof(ISP_ATTACH_INFO_S));

    if (HI_NULL == pstIspCtx->stAttachInfoCtrl.pstAttachInfo) {
        ISP_TRACE(HI_DBG_ERR, "isp[%d] mmap attach info buf failed!\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_AttachInfoExit(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (HI_NULL != pstIspCtx->stAttachInfoCtrl.pstAttachInfo) {
        HI_MPI_SYS_Munmap(pstIspCtx->stAttachInfoCtrl.pstAttachInfo, sizeof(ISP_ATTACH_INFO_S));
        pstIspCtx->stAttachInfoCtrl.pstAttachInfo = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_ColorGamutInfoInit(VI_PIPE ViPipe)
{
    HI_U64 u64PhyAddr;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    u64PhyAddr = pstIspCtx->stIspTransInfo.stColorGammutInfo.u64PhyAddr;

    pstIspCtx->stGamutInfoCtrl.pstColorGamutInfo = HI_MPI_SYS_Mmap(u64PhyAddr, sizeof(ISP_COLORGAMMUT_INFO_S));

    if (HI_NULL == pstIspCtx->stGamutInfoCtrl.pstColorGamutInfo) {
        ISP_TRACE(HI_DBG_ERR, "isp[%d] mmap color gamut info buf failed!\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    pstIspCtx->stGamutInfoCtrl.pstColorGamutInfo->enColorGamut = COLOR_GAMUT_BT709;

    return HI_SUCCESS;
}

HI_S32 ISP_ColorGamutInfoExit(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (HI_NULL != pstIspCtx->stGamutInfoCtrl.pstColorGamutInfo) {
        HI_MPI_SYS_Munmap(pstIspCtx->stGamutInfoCtrl.pstColorGamutInfo, sizeof(ISP_COLORGAMMUT_INFO_S));
        pstIspCtx->stGamutInfoCtrl.pstColorGamutInfo = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_ProInfoInit(VI_PIPE ViPipe, ISP_PRO_INFO_BUF_S *pstProInfo)
{
    HI_S32 s32Ret;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstProInfo);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_PRO_BUF_INIT, pstProInfo);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "isp[%d] init pro info bufs failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_ProInfoExit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_CHECK_PIPE(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_PRO_BUF_EXIT);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "isp[%d] exit pro info buf failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_ProNrParamInit(VI_PIPE ViPipe)
{
    HI_U64 u64PhyAddr;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    u64PhyAddr = pstIspCtx->stIspProInfo.stProNrParamInfo.u64PhyAddr;

    pstIspCtx->stProNrParamCtrl.pstProNrParam = HI_MPI_SYS_Mmap(u64PhyAddr, sizeof(ISP_PRO_NR_PARAM_S));

    if (HI_NULL == pstIspCtx->stProNrParamCtrl.pstProNrParam) {
        ISP_TRACE(HI_DBG_ERR, "isp[%d] mmap pro nr paramt buf failed!\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_ProNrParamExit(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (HI_NULL != pstIspCtx->stProNrParamCtrl.pstProNrParam) {
        HI_MPI_SYS_Munmap(pstIspCtx->stProNrParamCtrl.pstProNrParam, sizeof(ISP_PRO_NR_PARAM_S));
        pstIspCtx->stProNrParamCtrl.pstProNrParam = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_ProShpParamInit(VI_PIPE ViPipe)
{
    HI_U64 u64PhyAddr;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    u64PhyAddr = pstIspCtx->stIspProInfo.stProShpParamInfo.u64PhyAddr;

    pstIspCtx->stProShpParamCtrl.pstProShpParam = HI_MPI_SYS_Mmap(u64PhyAddr, sizeof(ISP_PRO_SHP_PARAM_S));

    if (HI_NULL == pstIspCtx->stProShpParamCtrl.pstProShpParam) {
        ISP_TRACE(HI_DBG_ERR, "isp[%d] mmap pro shp paramt buf failed!\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_ProShpParamExit(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (HI_NULL != pstIspCtx->stProShpParamCtrl.pstProShpParam) {
        HI_MPI_SYS_Munmap(pstIspCtx->stProShpParamCtrl.pstProShpParam, sizeof(ISP_PRO_SHP_PARAM_S));
        pstIspCtx->stProShpParamCtrl.pstProShpParam = HI_NULL;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
