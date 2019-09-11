/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_regcfg.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/05/07
  Description   :
  History       :
  1.Date        : 2017/01/12
    Author      :
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "mkp_isp.h"
#include "isp_regcfg.h"
#include "isp_config.h"
#include "isp_lut_config.h"
#include "isp_ext_config.h"
#include "isp_main.h"
#include "mpi_sys.h"
#include "hi_math.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

ISP_BE_BUF_S     g_astBeBufCtx[ISP_MAX_PIPE_NUM]   = {{0}};
ISP_REGCFG_S    *g_pastRegCfgCtx[ISP_MAX_PIPE_NUM] = {HI_NULL};
ISP_BE_LUT_BUF_S g_astBeLutBufCtx[ISP_MAX_PIPE_NUM] = {0};
const HI_U32     g_au32IsoLut[ISP_AUTO_ISO_STRENGTH_NUM] = {100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800};

#define BE_REG_GET_CTX(dev, pstCtx)     pstCtx = &g_astBeBufCtx[dev]
#define BE_LUT_BUF_GET_CTX(dev, pstCtx) pstCtx = &g_astBeLutBufCtx[dev]

extern HI_S32 g_as32IspFd[ISP_MAX_PIPE_NUM];

HI_S32 ISP_ModParamGet(ISP_MOD_PARAM_S *pstModParam)
{
    ISP_CHECK_POINTER(pstModParam);

    pstModParam->u32QuickStart = HI_FALSE;

    return HI_SUCCESS;
}

HI_S32 ISP_ClutBufInit(VI_PIPE ViPipe)
{
    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_CLUT_BUF_INIT))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] clut buffer init failed\n", ViPipe);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_ClutBufExit(VI_PIPE ViPipe)
{
    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_CLUT_BUF_EXIT))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] exit clut bufs failed\n", ViPipe);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SpecAwbBufInit(VI_PIPE ViPipe)
{
    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_SPECAWB_BUF_INIT))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] specawb buffer init failed\n", ViPipe);
        return HI_ERR_ISP_MEM_NOT_INIT;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SpecAwbBufExit(VI_PIPE ViPipe)
{
    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_SPECAWB_BUF_EXIT))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] exit specawb bufs failed\n", ViPipe);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_UpdateBeLutSttBufCtx(VI_PIPE ViPipe, HI_U64 u64PhyAddr)
{
    HI_U8   i;
    HI_U64  u64Size;
    HI_VOID *pVirtAddr = HI_NULL;
    ISP_CTX_S        *pstIspCtx = HI_NULL;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
        IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        return HI_FAILURE;
    }

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);

    pVirtAddr  = HI_MPI_SYS_Mmap(u64PhyAddr, u64Size * 2 * ISP_MAX_BE_NUM);

    if (HI_NULL == pVirtAddr)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d get be lut stt bufs address failed!\n", ViPipe);
        return HI_FAILURE;
    }

    for (i = 0; i < ISP_MAX_BE_NUM; i++)
    {
        pstBeLutBuf->astLutSttBuf[i].u64PhyAddr = u64PhyAddr + 2 * i * u64Size;
        pstBeLutBuf->astLutSttBuf[i].pVirAddr   = (HI_VOID *)((HI_U8 *)pVirtAddr + 2 * i * u64Size);;
        pstBeLutBuf->astLutSttBuf[i].u64Size    = u64Size;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_BeLutBufAddrInit(VI_PIPE ViPipe)
{
    HI_S32    s32Ret;
    HI_U64    u64PhyAddr;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
        IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        return HI_SUCCESS;
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_BE_LUT_STT_BUF_GET, &u64PhyAddr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d get be lut2stt bufs address failed%x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    hi_ext_system_be_lut_stt_buffer_high_addr_write(ViPipe, (u64PhyAddr >> 32));
    hi_ext_system_be_lut_stt_buffer_low_addr_write(ViPipe, (u64PhyAddr & 0xFFFFFFFF));

    s32Ret = ISP_UpdateBeLutSttBufCtx(ViPipe, u64PhyAddr);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_BeLutBufAddrExit(VI_PIPE ViPipe)
{
    HI_U8  i;
    ISP_CTX_S        *pstIspCtx = HI_NULL;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
        IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        return HI_SUCCESS;
    }

    if (HI_NULL != pstBeLutBuf->astLutSttBuf[0].pVirAddr)
    {
        HI_MPI_SYS_Munmap(pstBeLutBuf->astLutSttBuf[0].pVirAddr, sizeof(S_ISP_LUT_WSTT_TYPE) * 2 * ISP_MAX_BE_NUM);
        for (i = 0; i < ISP_MAX_BE_NUM; i++)
        {
            pstBeLutBuf->astLutSttBuf[i].pVirAddr = HI_NULL;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SttBufInit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_STT_BUF_INIT))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] stt buffer init failed\n", ViPipe);
        return HI_FAILURE;
    }

    s32Ret = ISP_BeLutBufAddrInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] be lut2stt buffer address init failed\n", ViPipe);
        ISP_SttBufExit(ViPipe);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SttBufExit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;

    s32Ret = ISP_BeLutBufAddrExit(ViPipe);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] be lut stt buffer exit failed\n", ViPipe);
        return HI_FAILURE;
    }

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_STT_BUF_EXIT))
    {
        ISP_TRACE(HI_DBG_ERR, "exit stt bufs failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SttAddrInit(VI_PIPE ViPipe)
{
    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_STT_ADDR_INIT))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] stt address init failed\n", ViPipe);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_CfgBeBufInit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_BE_BUF_S *pstBeBuf = HI_NULL;
    HI_U32 u32BeBufSize;

    ISP_CHECK_OFFLINE_MODE(ViPipe);
    BE_REG_GET_CTX(ViPipe, pstBeBuf);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_BE_CFG_BUF_INIT, &pstBeBuf->u64BePhyAddr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d init be config bufs failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    pstBeBuf->pBeVirtAddr = HI_MPI_SYS_MmapCache(pstBeBuf->u64BePhyAddr, sizeof(ISP_BE_WO_REG_CFG_S) * MAX_ISP_BE_BUF_NUM);

    if (HI_NULL == pstBeBuf->pBeVirtAddr)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d init be config bufs failed!\n", ViPipe);
        s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_BE_CFG_BUF_EXIT);

        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe:%d exit be config bufs failed %x!\n", ViPipe, s32Ret);
            return s32Ret;
        }

        return HI_ERR_ISP_NOMEM;
    }

    pstBeBuf->stBeWoCfgBuf.u64PhyAddr = pstBeBuf->u64BePhyAddr;

    /* Get be buffer start address & size */
    u32BeBufSize = sizeof(ISP_BE_WO_REG_CFG_S) * MAX_ISP_BE_BUF_NUM;
    hi_ext_system_be_buffer_address_high_write(ViPipe, (pstBeBuf->u64BePhyAddr >> 32));
    hi_ext_system_be_buffer_address_low_write(ViPipe, (pstBeBuf->u64BePhyAddr & 0xFFFFFFFF));
    hi_ext_system_be_buffer_size_write(ViPipe, u32BeBufSize);

    return HI_SUCCESS;
}

HI_S32 ISP_UpdateBeBufAddr(VI_PIPE ViPipe, HI_VOID *pVirtAddr)
{
    HI_U16 i;
    HI_U64 u64BufSize = 0;
    ISP_RUNNING_MODE_E enIspRuningMode;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    enIspRuningMode = pstIspCtx->stBlockAttr.enIspRunningMode;
    u64BufSize      = sizeof(ISP_BE_WO_REG_CFG_S) / ISP_STRIPING_MAX_NUM;

    switch (enIspRuningMode)
    {
        case ISP_MODE_RUNNING_STRIPING :
            for (i = 0; i < ISP_STRIPING_MAX_NUM; i++)
            {
                pstIspCtx->pIspBeVirtAddr[i]  = (HI_VOID *)((HI_U8 *)pVirtAddr + i * u64BufSize);
                pstIspCtx->pViProcVirtAddr[i] = (HI_VOID *)((HI_U8 *)pstIspCtx->pIspBeVirtAddr[i] + VIPROC_OFFLINE_OFFSET);
            }

            break;

        case ISP_MODE_RUNNING_OFFLINE :
            for (i = 0; i < ISP_STRIPING_MAX_NUM; i++)
            {
                if (0 == i)
                {
                    pstIspCtx->pIspBeVirtAddr[i]  = pVirtAddr;
                    pstIspCtx->pViProcVirtAddr[i] = (HI_VOID *)((HI_U8 *)pVirtAddr + VIPROC_OFFLINE_OFFSET);
                }
                else
                {
                    pstIspCtx->pIspBeVirtAddr[i]  = HI_NULL;
                    pstIspCtx->pViProcVirtAddr[i] = HI_NULL;
                }
            }

            break;

        default :
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_CfgBeBufMmap(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_U64 u64BePhyAddr;
    ISP_BE_BUF_S *pstBeBuf = HI_NULL;

    BE_REG_GET_CTX(ViPipe, pstBeBuf);

    u64BePhyAddr = pstBeBuf->stBeWoCfgBuf.u64PhyAddr;
    hi_ext_system_be_free_buffer_high_addr_write(ViPipe, (u64BePhyAddr >> 32));
    hi_ext_system_be_free_buffer_low_addr_write(ViPipe, (u64BePhyAddr & 0xFFFFFFFF));

    if (HI_NULL != pstBeBuf->pBeVirtAddr)
    {
        pstBeBuf->stBeWoCfgBuf.pVirAddr = (HI_VOID *)((HI_U8 *)pstBeBuf->pBeVirtAddr + \
                                          (pstBeBuf->stBeWoCfgBuf.u64PhyAddr - pstBeBuf->u64BePhyAddr));
    }
    else
    {
        pstBeBuf->stBeWoCfgBuf.pVirAddr = HI_NULL;
    }

    if (HI_NULL == pstBeBuf->stBeWoCfgBuf.pVirAddr)
    {
        return HI_FAILURE;
    }

    s32Ret = ISP_UpdateBeBufAddr(ViPipe, pstBeBuf->stBeWoCfgBuf.pVirAddr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d isp update BE bufs failed %x!\n", ViPipe, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_GetBeBufFirst(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_BE_BUF_S *pstBeBuf = HI_NULL;

    ISP_CHECK_OFFLINE_MODE(ViPipe);
    BE_REG_GET_CTX(ViPipe, pstBeBuf);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_BE_BUF_FIRST, &pstBeBuf->stBeWoCfgBuf.u64PhyAddr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d Get be free bufs failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    s32Ret = ISP_CfgBeBufMmap(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d ISP_CfgBeBufMmap failed %x!\n", ViPipe, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_GetBeFreeBuf(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_BE_BUF_S *pstBeBuf = HI_NULL;

    ISP_CHECK_OFFLINE_MODE(ViPipe);
    BE_REG_GET_CTX(ViPipe, pstBeBuf);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_BE_FREE_BUF_GET, &pstBeBuf->stBeWoCfgBuf);

    if (HI_SUCCESS != s32Ret)
    {
        //ISP_TRACE(HI_DBG_ERR, "Pipe:%d ISP_GetBeFreeBuf failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    s32Ret = ISP_CfgBeBufMmap(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d ISP_CfgBeBufMmap failed %x!\n", ViPipe, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_GetBeLastBuf(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_BE_BUF_S *pstBeBuf  = HI_NULL;

    ISP_CHECK_OFFLINE_MODE(ViPipe);
    BE_REG_GET_CTX(ViPipe, pstBeBuf);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_BE_LAST_BUF_GET, &pstBeBuf->stBeWoCfgBuf.u64PhyAddr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d Get be busy bufs failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    s32Ret = ISP_CfgBeBufMmap(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d ISP_CfgBeBufMmap failed %x!\n", ViPipe, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_CfgBeBufExit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_BE_BUF_S *pstBeBuf = HI_NULL;

    ISP_CHECK_OFFLINE_MODE(ViPipe);
    BE_REG_GET_CTX(ViPipe, pstBeBuf);

    if (HI_NULL != pstBeBuf->pBeVirtAddr)
    {
        HI_MPI_SYS_Munmap(pstBeBuf->pBeVirtAddr, sizeof(ISP_BE_WO_REG_CFG_S) * MAX_ISP_BE_BUF_NUM);
        pstBeBuf->pBeVirtAddr = HI_NULL;
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_BE_CFG_BUF_EXIT);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d exit be config bufs failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_CfgBeBufCtl(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_BE_BUF_S *pstBeBuf = HI_NULL;

    ISP_CHECK_OFFLINE_MODE(ViPipe);
    BE_REG_GET_CTX(ViPipe, pstBeBuf);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_BE_CFG_BUF_CTL, &pstBeBuf->stBeWoCfgBuf);
    if (s32Ret)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SetCfgBeBufState(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;

    ISP_CHECK_OFFLINE_MODE(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_BE_CFG_BUF_RUNNING);
    if (s32Ret)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

/* init isp be cfgs all buffer */
HI_S32 ISP_AllCfgsBeBufInit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;

    ISP_CHECK_OFFLINE_MODE(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_BE_All_BUF_INIT);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init be all bufs Failed with ec %#x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S8 ISP_GetBlockIdByPipe(VI_PIPE ViPipe)
{
    HI_S8 s8BlockId = 0;

    switch (ViPipe)
    {
        case ISP_BE0_PIPE_ID :
            s8BlockId = 0;
            break;

        case ISP_BE1_PIPE_ID :
            s8BlockId = 1;
            break;

        default:
            return HI_FAILURE;
    }

    return s8BlockId;
}

HI_S32 ISP_BeVregAddrInit(VI_PIPE ViPipe)
{
    HI_U8  k = 0;
    HI_S8  s8BlkDev = 0;
    HI_U8  u8BlockId = 0;
    HI_U64 u64BufSize = 0;
    ISP_RUNNING_MODE_E enIspRuningMode;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_BE_BUF_S *pstBeBuf = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    BE_REG_GET_CTX(ViPipe, pstBeBuf);

    enIspRuningMode = pstIspCtx->stBlockAttr.enIspRunningMode;
    u64BufSize      = sizeof(ISP_BE_WO_REG_CFG_S) / ISP_STRIPING_MAX_NUM;

    switch ( enIspRuningMode )
    {
        case ISP_MODE_RUNNING_ONLINE :
            s8BlkDev = ISP_GetBlockIdByPipe(ViPipe);

            if (-1 == s8BlkDev)
            {
                ISP_TRACE(HI_DBG_ERR, "ISP[%d] init Online Mode Pipe Err!\n", ViPipe);
                return HI_FAILURE;
            }

            u8BlockId = (HI_U8)s8BlkDev;

            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                if (0 == k)
                {
                    pstIspCtx->pIspBeVirtAddr[k]  = VReg_GetVirtAddrBase(ISP_BE_REG_BASE(u8BlockId));
                    pstIspCtx->pViProcVirtAddr[k] = VReg_GetVirtAddrBase(ISP_VIPROC_REG_BASE(u8BlockId));
                }
                else
                {
                    pstIspCtx->pIspBeVirtAddr[k]  = HI_NULL;
                    pstIspCtx->pViProcVirtAddr[k] = HI_NULL;
                }
            }

            break;

        case ISP_MODE_RUNNING_OFFLINE :
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                if (0 == k)
                {
                    pstIspCtx->pIspBeVirtAddr[k]  = pstBeBuf->stBeWoCfgBuf.pVirAddr;
                    pstIspCtx->pViProcVirtAddr[k] = (HI_VOID *)((HI_U8 *)pstBeBuf->stBeWoCfgBuf.pVirAddr + VIPROC_OFFLINE_OFFSET);
                }
                else
                {
                    pstIspCtx->pIspBeVirtAddr[k]  = HI_NULL;
                    pstIspCtx->pViProcVirtAddr[k] = HI_NULL;
                }
            }

            break;

        case ISP_MODE_RUNNING_SIDEBYSIDE :
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                if (k < ISP_MAX_BE_NUM)
                {
                    pstIspCtx->pIspBeVirtAddr[k]  = VReg_GetVirtAddrBase(ISP_BE_REG_BASE(k));
                    pstIspCtx->pViProcVirtAddr[k] = VReg_GetVirtAddrBase(ISP_VIPROC_REG_BASE(k));
                }
                else
                {
                    pstIspCtx->pIspBeVirtAddr[k]  = HI_NULL;
                    pstIspCtx->pViProcVirtAddr[k] = HI_NULL;
                }
            }

            break;

        case ISP_MODE_RUNNING_STRIPING :
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                pstIspCtx->pIspBeVirtAddr[k]  = (HI_VOID *)((HI_U8 *)pstBeBuf->stBeWoCfgBuf.pVirAddr + k * u64BufSize);
                pstIspCtx->pViProcVirtAddr[k] = (HI_VOID *)((HI_U8 *)pstIspCtx->pIspBeVirtAddr[k] + VIPROC_OFFLINE_OFFSET);
            }

            break;

        default:
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] init Running Mode Err!\n", ViPipe);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID *ISP_VRegCfgBufAddr(VI_PIPE ViPipe, BLK_DEV BlkDev)
{
    HI_U64 u64Size = 0;
    HI_U64 u64PhyAddrHigh;
    HI_U64 u64PhyAddrTemp;
    ISP_BE_BUF_S *pstBeBuf = HI_NULL;

    BE_REG_GET_CTX(ViPipe, pstBeBuf);

    u64Size = sizeof(ISP_BE_WO_REG_CFG_S) / ISP_STRIPING_MAX_NUM;

    if (HI_NULL != pstBeBuf->stBeWoCfgBuf.pVirAddr)
    {
        return ((HI_U8 *)pstBeBuf->stBeWoCfgBuf.pVirAddr + BlkDev * u64Size);
    }

    u64PhyAddrHigh = (HI_U64)hi_ext_system_be_free_buffer_high_addr_read(ViPipe);
    u64PhyAddrTemp = (HI_U64)hi_ext_system_be_free_buffer_low_addr_read(ViPipe);
    u64PhyAddrTemp |= (u64PhyAddrHigh << 32);

    pstBeBuf->stBeWoCfgBuf.u64PhyAddr = u64PhyAddrTemp;
    pstBeBuf->stBeWoCfgBuf.pVirAddr = HI_MPI_SYS_MmapCache(pstBeBuf->stBeWoCfgBuf.u64PhyAddr, sizeof(ISP_BE_WO_REG_CFG_S));

    return ((HI_U8 *)pstBeBuf->stBeWoCfgBuf.pVirAddr + BlkDev * u64Size);
}

HI_S32 ISP_GetBeVregCfgAddr(VI_PIPE ViPipe, HI_VOID *pVirtAddr[])
{
    HI_U8  k = 0;
    HI_S8  s8BlkDev = 0;
    HI_U8  u8BlockId = 0;
    HI_S32 s32Ret;
    ISP_WORKING_MODE_S stIspWorkMode;

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode);

    if (s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "get isp work mode failed!\n");
        return s32Ret;
    }

    switch ( stIspWorkMode.enIspRunningMode )
    {
        case ISP_MODE_RUNNING_ONLINE :
            s8BlkDev = ISP_GetBlockIdByPipe(ViPipe);

            if (-1 == s8BlkDev)
            {
                ISP_TRACE(HI_DBG_ERR, "ISP[%d] Online Mode Pipe Err!\n", ViPipe);
                return HI_FAILURE;
            }

            u8BlockId = (HI_U8)s8BlkDev;

            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                if (0 == k)
                {
                    pVirtAddr[k] = VReg_GetVirtAddrBase(ISP_BE_REG_BASE(u8BlockId));
                }
                else
                {
                    pVirtAddr[k] = HI_NULL;
                }
            }

            break;

        case ISP_MODE_RUNNING_OFFLINE :
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                if (0 == k)
                {
                    pVirtAddr[k] = ISP_VRegCfgBufAddr(ViPipe, (BLK_DEV)k);
                }
                else
                {
                    pVirtAddr[k] = HI_NULL;
                }
            }

            break;

        case ISP_MODE_RUNNING_SIDEBYSIDE :
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                if (k < ISP_MAX_BE_NUM)
                {
                    pVirtAddr[k] = VReg_GetVirtAddrBase(ISP_BE_REG_BASE(k));
                }
                else
                {
                    pVirtAddr[k] = HI_NULL;
                }
            }

            break;

        case ISP_MODE_RUNNING_STRIPING :
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                pVirtAddr[k] = ISP_VRegCfgBufAddr(ViPipe, (BLK_DEV)k);
            }

            break;

        default:
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] GetBe Running Mode Err!\n", ViPipe);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_GetViProcCfgAddr(VI_PIPE ViPipe, HI_VOID *pVirtAddr[])
{
    HI_U8  k = 0;
    HI_S8  s8BlkDev = 0;
    HI_U8  u8BlockId = 0;
    HI_S32 s32Ret;
    HI_VOID *pBeVirtAddr;
    ISP_WORKING_MODE_S stIspWorkMode;

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode);

    if (s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "get isp work mode failed!\n");
        return s32Ret;
    }

    switch ( stIspWorkMode.enIspRunningMode )
    {
        case ISP_MODE_RUNNING_ONLINE :
            s8BlkDev = ISP_GetBlockIdByPipe(ViPipe);

            if (-1 == s8BlkDev)
            {
                ISP_TRACE(HI_DBG_ERR, "ISP[%d] Online Mode Pipe Err!\n", ViPipe);
                return HI_FAILURE;
            }

            u8BlockId = (HI_U8)s8BlkDev;

            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                if (0 == k)
                {
                    pVirtAddr[k] = VReg_GetVirtAddrBase(ISP_VIPROC_REG_BASE(u8BlockId));
                }
                else
                {
                    pVirtAddr[k] = HI_NULL;
                }
            }

            break;

        case ISP_MODE_RUNNING_OFFLINE :
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                if (0 == k)
                {
                    pBeVirtAddr = ISP_VRegCfgBufAddr(ViPipe, (BLK_DEV)k);
                    pVirtAddr[k] = (HI_VOID * )((HI_U8 *)pBeVirtAddr + VIPROC_OFFLINE_OFFSET);
                }
                else
                {
                    pVirtAddr[k] = HI_NULL;
                }
            }

            break;

        case ISP_MODE_RUNNING_SIDEBYSIDE :
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                if (k < ISP_MAX_BE_NUM)
                {
                    pVirtAddr[k] = VReg_GetVirtAddrBase(ISP_VIPROC_REG_BASE(k));
                }
                else
                {
                    pVirtAddr[k] = HI_NULL;
                }
            }

            break;

        case ISP_MODE_RUNNING_STRIPING :
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                pBeVirtAddr = ISP_VRegCfgBufAddr(ViPipe, (BLK_DEV)k);
                pVirtAddr[k] = (HI_VOID * )((HI_U8 *)pBeVirtAddr + VIPROC_OFFLINE_OFFSET);
            }

            break;

        default:
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] GetBe Running Mode Err!\n", ViPipe);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID *ISP_GetFeVirAddr(VI_PIPE ViPipe)
{
    ISP_CHECK_FE_PIPE(ViPipe);

    return VReg_GetVirtAddrBase(ISP_FE_REG_BASE(ViPipe));
}

HI_VOID *ISP_GetBeLut2SttVirAddr(VI_PIPE ViPipe, BLK_DEV BlkDev, HI_U8 U8BufId)
{
    HI_S32  s32Ret;
    HI_U64  u64Size;
    HI_U64  u64PhyAddrHigh, u64PhyAddrTemp;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;
    ISP_CTX_S        *pstIspCtx   = HI_NULL;

    ISP_CHECK_FE_PIPE(ViPipe);
    ISP_CHECK_BE_DEV(BlkDev);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
        IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        return HI_NULL;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);

    if (HI_NULL != pstBeLutBuf->astLutSttBuf[BlkDev].pVirAddr)
    {
        return (HI_VOID *)((HI_U8 *)pstBeLutBuf->astLutSttBuf[BlkDev].pVirAddr + u64Size * U8BufId);
    }

    u64PhyAddrHigh  = (HI_U64)hi_ext_system_be_lut_stt_buffer_high_addr_read(ViPipe);
    u64PhyAddrTemp  = (HI_U64)hi_ext_system_be_lut_stt_buffer_low_addr_read(ViPipe);
    u64PhyAddrTemp |= (u64PhyAddrHigh << 32);

    s32Ret = ISP_UpdateBeLutSttBufCtx(ViPipe, u64PhyAddrTemp);

    if (HI_SUCCESS != s32Ret)
    {
        return HI_NULL;
    }

    return (HI_VOID *)((HI_U8 *)pstBeLutBuf->astLutSttBuf[BlkDev].pVirAddr + u64Size * U8BufId);
}

HI_VOID *ISP_GetBeVirAddr(VI_PIPE ViPipe, BLK_DEV BlkDev)
{
    HI_U32 s32Ret;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_VOID *pVirtAddr[ISP_STRIPING_MAX_NUM] = {HI_NULL};

    ISP_CHECK_FE_PIPE(ViPipe);
    ISP_CHECK_BE_DEV(BlkDev);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (pstIspCtx->pIspBeVirtAddr[BlkDev])
    {
        return pstIspCtx->pIspBeVirtAddr[BlkDev];
    }

    s32Ret = ISP_GetBeVregCfgAddr(ViPipe, pVirtAddr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get Be CfgAddr Failed!\n", ViPipe);
        return HI_NULL;
    }

    ISP_CHECK_NULLPTR(pVirtAddr[BlkDev]);

    return pVirtAddr[BlkDev];
}

HI_VOID *ISP_GetViProcVirAddr(VI_PIPE ViPipe, BLK_DEV BlkDev)
{
    HI_U32 s32Ret;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_VOID *pVirtAddr[ISP_STRIPING_MAX_NUM] = {HI_NULL};

    ISP_CHECK_FE_PIPE(ViPipe);
    ISP_CHECK_BE_DEV(BlkDev);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (pstIspCtx->pViProcVirtAddr[BlkDev])
    {
        return pstIspCtx->pViProcVirtAddr[BlkDev];
    }

    s32Ret = ISP_GetViProcCfgAddr(ViPipe, pVirtAddr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get viproc CfgAddr Failed!\n", ViPipe);
        return HI_NULL;
    }

    ISP_CHECK_NULLPTR(pVirtAddr[BlkDev]);

    return pVirtAddr[BlkDev];
}

static HI_S32 isp_clut_lut_wstt_addr_write(VI_PIPE ViPipe, HI_U8 i, HI_U8 u8BufId, S_VIPROC_REGS_TYPE *pstViProcReg)
{
    HI_U64 u64PhyAddr, u64Size;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64PhyAddr = pstBeLutBuf->astLutSttBuf[i].u64PhyAddr;

    if (0 == u64PhyAddr)
    {
        return HI_FAILURE;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);
    isp_clut_lut_addr_low_write(pstViProcReg, GetLowAddr(u64PhyAddr  + ISP_CLUT_LUT_WSTT_OFFSET + u8BufId * u64Size));
    isp_clut_lut_width_word_write(pstViProcReg, HI_ISP_CLUT_LUT_WIDTH_WORD_DEFAULT);

    return HI_SUCCESS;
}

static HI_S32 isp_sharpen_lut_wstt_addr_write(VI_PIPE ViPipe, HI_U8 i, HI_U8 u8BufId, S_VIPROC_REGS_TYPE *pstViProcReg)
{
    HI_U64 u64PhyAddr, u64Size;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64PhyAddr = pstBeLutBuf->astLutSttBuf[i].u64PhyAddr;

    if (0 == u64PhyAddr)
    {
        return HI_FAILURE;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);
    isp_sharpen_lut_addr_low_write(pstViProcReg, GetLowAddr(u64PhyAddr  + ISP_SHARPEN_LUT_WSTT_OFFSET + u8BufId * u64Size));

    return HI_SUCCESS;
}

static HI_S32 isp_dehaze_lut_wstt_addr_write(VI_PIPE ViPipe, HI_U8 i, HI_U8 u8BufId, S_VIPROC_REGS_TYPE *pstViProcReg)
{
    HI_U64 u64PhyAddr, u64Size;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64PhyAddr = pstBeLutBuf->astLutSttBuf[i].u64PhyAddr;

    if (0 == u64PhyAddr)
    {
        return HI_FAILURE;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);
    isp_dehaze_lut_addr_low_write(pstViProcReg, GetLowAddr(u64PhyAddr  + ISP_DEHAZE_LUT_WSTT_OFFSET + u8BufId * u64Size));

    return HI_SUCCESS;
}

static HI_S32 isp_gamma_lut_wstt_addr_write(VI_PIPE ViPipe, HI_U8 i, HI_U8 u8BufId, S_VIPROC_REGS_TYPE *pstViProcReg)
{
    HI_U64 u64PhyAddr, u64Size;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64PhyAddr = pstBeLutBuf->astLutSttBuf[i].u64PhyAddr;

    if (0 == u64PhyAddr)
    {
        return HI_FAILURE;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);
    isp_gamma_lut_addr_low_write(pstViProcReg, GetLowAddr(u64PhyAddr  + ISP_GAMMA_LUT_WSTT_OFFSET + u8BufId * u64Size));

    return HI_SUCCESS;
}

static HI_S32 isp_bnr_rlsc_lut_wstt_addr_write(VI_PIPE ViPipe, HI_U8 i, HI_U8 u8BufId, S_VIPROC_REGS_TYPE *pstViProcReg)
{
    HI_U64 u64PhyAddr, u64Size;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64PhyAddr = pstBeLutBuf->astLutSttBuf[i].u64PhyAddr;

    if (0 == u64PhyAddr)
    {
        return HI_FAILURE;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);
    isp_bnr_lut_addr_low_write(pstViProcReg, GetLowAddr(u64PhyAddr  + ISP_BNR_RLSC_LUT_WSTT_OFFSET + u8BufId * u64Size));

    return HI_SUCCESS;
}

static HI_S32 isp_dpc_lut_wstt_addr_write(VI_PIPE ViPipe, HI_U8 i, HI_U8 u8BufId, S_VIPROC_REGS_TYPE *pstViProcReg)
{
    HI_U64 u64PhyAddr, u64Size;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64PhyAddr = pstBeLutBuf->astLutSttBuf[i].u64PhyAddr;

    if (0 == u64PhyAddr)
    {
        return HI_FAILURE;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);
    isp_dcg_lut_addr_low_write(pstViProcReg, GetLowAddr(u64PhyAddr  + ISP_DPC_LUT_WSTT_OFFSET + u8BufId * u64Size));

    return HI_SUCCESS;
}

static HI_S32 isp_lsc_lut_wstt_addr_write(VI_PIPE ViPipe, HI_U8 i, HI_U8 u8BufId, S_VIPROC_REGS_TYPE *pstViProcReg)
{
    HI_U64 u64PhyAddr, u64Size;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64PhyAddr = pstBeLutBuf->astLutSttBuf[i].u64PhyAddr;

    if (0 == u64PhyAddr)
    {
        return HI_FAILURE;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);
    isp_lsc_lut_addr_low_write(pstViProcReg, GetLowAddr(u64PhyAddr  + ISP_LSC_LUT_WSTT_OFFSET + u8BufId * u64Size));

    return HI_SUCCESS;
}

#ifdef CONFIG_HI_ISP_CA_SUPPORT
static HI_S32 isp_ca_lut_wstt_addr_write(VI_PIPE ViPipe, HI_U8 i, HI_U8 u8BufId, S_VIPROC_REGS_TYPE *pstViProcReg)
{
    HI_U64 u64PhyAddr, u64Size;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64PhyAddr = pstBeLutBuf->astLutSttBuf[i].u64PhyAddr;

    if (0 == u64PhyAddr)
    {
        return HI_FAILURE;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);
    isp_ca_lut_addr_low_write(pstViProcReg, GetLowAddr(u64PhyAddr  + ISP_CA_LUT_WSTT_OFFSET + u8BufId * u64Size));

    return HI_SUCCESS;
}
#endif
static HI_S32 isp_ldci_lut_wstt_addr_write(VI_PIPE ViPipe, HI_U8 i, HI_U8 u8BufId, S_VIPROC_REGS_TYPE *pstViProcReg)
{
    HI_U64 u64PhyAddr, u64Size;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64PhyAddr = pstBeLutBuf->astLutSttBuf[i].u64PhyAddr;

    if (0 == u64PhyAddr)
    {
        return HI_FAILURE;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);
    isp_ldci_lut_addr_low_write(pstViProcReg, GetLowAddr(u64PhyAddr  + ISP_LDCI_LUT_WSTT_OFFSET + u8BufId * u64Size));

    return HI_SUCCESS;
}

#ifdef CONFIG_HI_ISP_PREGAMMA_SUPPORT
static HI_S32 isp_pregamma_lut_wstt_addr_write(VI_PIPE ViPipe, HI_U8 i, HI_U8 u8BufId, S_VIPROC_REGS_TYPE *pstViProcReg)
{
    HI_U64 u64PhyAddr, u64Size;
    ISP_BE_LUT_BUF_S *pstBeLutBuf = HI_NULL;

    BE_LUT_BUF_GET_CTX(ViPipe, pstBeLutBuf);

    u64PhyAddr = pstBeLutBuf->astLutSttBuf[i].u64PhyAddr;

    if (0 == u64PhyAddr)
    {
        return HI_FAILURE;
    }

    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);
    isp_pregamma_lut_addr_low_write(pstViProcReg, GetLowAddr(u64PhyAddr  + ISP_PREGAMMA_LUT_WSTT_OFFSET + u8BufId * u64Size));

    return HI_SUCCESS;
}
#endif
static HI_S32 ISP_FeDgRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    HI_U16  i;
    VI_PIPE ViPipeBind;
    ISP_FE_DG_DYNA_CFG_S *pstDynaRegCfg = HI_NULL;
    ISP_CTX_S         *pstIspCtx     = HI_NULL;
    S_ISPFE_REGS_TYPE *pstFeReg      = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    pstDynaRegCfg = &pstRegCfgInfo->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg;

    if (pstIspCtx->stWdrAttr.bMastPipe)
    {
        for (i = 0; i < pstIspCtx->stWdrAttr.stDevBindPipe.u32Num; i++)
        {
            ViPipeBind = pstIspCtx->stWdrAttr.stDevBindPipe.PipeId[i];
            ISP_CHECK_PIPE(ViPipeBind);
            pstFeReg = (S_ISPFE_REGS_TYPE *)ISP_GetFeVirAddr(ViPipeBind);
            ISP_CHECK_POINTER(pstFeReg);

            if (pstRegCfgInfo->unKey.bit1FeDgCfg)
            {
                isp_fe_dg2_en_write(pstFeReg, pstRegCfgInfo->stAlgRegCfg[0].stFeDgRegCfg.bDgEn);

                if (pstDynaRegCfg->bResh)
                {
                    isp_dg2_gain_write(pstFeReg, pstDynaRegCfg->au16GainR[i], pstDynaRegCfg->au16GainGR[i], pstDynaRegCfg->au16GainGB[i], pstDynaRegCfg->au16GainB[i]);
                    isp_dg2_clip_value_write(pstFeReg, pstDynaRegCfg->u32ClipValue);
                }
            }
        }

        pstDynaRegCfg->bResh = HI_FALSE;
        pstRegCfgInfo->unKey.bit1FeDgCfg = 0;
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_FeBlcRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    HI_U16  i;
    VI_PIPE ViPipeBind;
    ISP_FE_BLC_CFG_S  *pstFeBlcCfg = HI_NULL;
    ISP_CTX_S         *pstIspCtx   = HI_NULL;
    S_ISPFE_REGS_TYPE *pstFeReg    = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    pstFeBlcCfg = &pstRegCfgInfo->stAlgRegCfg[0].stFeBlcCfg;

    if (pstIspCtx->stWdrAttr.bMastPipe)
    {
        for (i = 0; i < pstIspCtx->stWdrAttr.stDevBindPipe.u32Num; i++)
        {
            ViPipeBind = pstIspCtx->stWdrAttr.stDevBindPipe.PipeId[i];
            ISP_CHECK_PIPE(ViPipeBind);
            pstFeReg = (S_ISPFE_REGS_TYPE *)ISP_GetFeVirAddr(ViPipeBind);
            ISP_CHECK_POINTER(pstFeReg);

            if (pstRegCfgInfo->unKey.bit1FeBlcCfg)
            {
                if (pstFeBlcCfg->bReshStatic)
                {
                    isp_dg2_en_write(pstFeReg, pstFeBlcCfg->stFeDgBlc.stStaticRegCfg.bBlcIn, pstFeBlcCfg->stFeDgBlc.stStaticRegCfg.bBlcOut);
                    isp_wb1_en_write(pstFeReg, pstFeBlcCfg->stFeWbBlc.stStaticRegCfg.bBlcIn, pstFeBlcCfg->stFeWbBlc.stStaticRegCfg.bBlcOut);
                    isp_ae1_blc_en_write(pstFeReg, pstFeBlcCfg->stFeAeBlc.stStaticRegCfg.bBlcIn);
                    isp_fe_blc1_en_write(pstFeReg, pstFeBlcCfg->stFeBlc.stStaticRegCfg.bBlcIn);
                }

                if (pstFeBlcCfg->bReshDyna)
                {
                    isp_dg2_offset_write(pstFeReg, pstFeBlcCfg->stFeDgBlc.stUsrRegCfg.au16Blc);
                    isp_wb1_offset_write(pstFeReg, pstFeBlcCfg->stFeWbBlc.stUsrRegCfg.au16Blc);
                    isp_ae1_offset_write(pstFeReg, pstFeBlcCfg->stFeAeBlc.stUsrRegCfg.au16Blc);
                    isp_blc1_offset_write(pstFeReg, pstFeBlcCfg->stFeBlc.stUsrRegCfg.au16Blc);
                }
            }
        }

        pstFeBlcCfg->bReshStatic = HI_FALSE;
        pstFeBlcCfg->bReshDyna   = HI_FALSE;
        pstRegCfgInfo->unKey.bit1FeBlcCfg = 0;
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_FeAeRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    HI_BOOL bLutUpdate = HI_FALSE;
    HI_U16  i, j, k;
    HI_U32  u32TableWeightTmp = 0;
    HI_U32  u32CombinWeight = 0;
    HI_U32  u32CombinWeightNum = 0;
    VI_PIPE ViPipeBind;
    ISP_AE_STATIC_CFG_S *pstStaticRegFeCfg = HI_NULL;
    ISP_AE_DYNA_CFG_S   *pstDynaRegFeCfg   = HI_NULL;
    S_ISPFE_REGS_TYPE   *pstFeReg          = HI_NULL;
    ISP_CTX_S *pstIspCtx   = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (pstIspCtx->stWdrAttr.bMastPipe && pstRegCfgInfo->unKey.bit1AeCfg1)
    {
        for (i = 0; i < pstIspCtx->stWdrAttr.stDevBindPipe.u32Num; i++)
        {
            ViPipeBind = pstIspCtx->stWdrAttr.stDevBindPipe.PipeId[i];
            ISP_CHECK_PIPE(ViPipeBind);

            pstFeReg = (S_ISPFE_REGS_TYPE *)ISP_GetFeVirAddr(ViPipeBind);
            ISP_CHECK_POINTER(pstFeReg);
            //ae fe static
            pstStaticRegFeCfg = &pstRegCfgInfo->stAlgRegCfg[0].stAeRegCfg.stStaticRegCfg;
            pstDynaRegFeCfg   = &pstRegCfgInfo->stAlgRegCfg[0].stAeRegCfg.stDynaRegCfg;

            isp_fe_ae1_en_write(pstFeReg, pstStaticRegFeCfg->u8FEEnable);
            isp_ae1_crop_pos_write(pstFeReg, pstStaticRegFeCfg->u16FECropPosX, pstStaticRegFeCfg->u16FECropPosY);
            isp_ae1_crop_outsize_write(pstFeReg, pstStaticRegFeCfg->u16FECropOutWidth - 1, pstStaticRegFeCfg->u16FECropOutHeight - 1);

            //ae fe dynamic
            isp_ae1_zone_write(pstFeReg, pstDynaRegFeCfg->u8FEWeightTableWidth, pstDynaRegFeCfg->u8FEWeightTableHeight);
            isp_ae1_skip_crg_write(pstFeReg, pstDynaRegFeCfg->u8FEHistSkipX, pstDynaRegFeCfg->u8FEHistOffsetX, pstDynaRegFeCfg->u8FEHistSkipY, pstDynaRegFeCfg->u8FEHistOffsetY);
            isp_ae1_bitmove_write(pstFeReg, pstDynaRegFeCfg->u8FEBitMove, pstDynaRegFeCfg->u8FEHistGammaMode, pstDynaRegFeCfg->u8FEAverGammaMode);
            isp_ae1_gamma_limit_write(pstFeReg, pstDynaRegFeCfg->u8FEGammaLimit);
            isp_ae1_fourplanemode_write(pstFeReg, pstDynaRegFeCfg->u8FEFourPlaneMode);

            u32CombinWeight = 0;
            u32CombinWeightNum = 0;

            isp_ae1_wei_waddr_write(pstFeReg, 0);

            for (j = 0; j < AE_ZONE_ROW; j++)
            {
                for (k = 0; k < AE_ZONE_COLUMN; k++)
                {
                    u32TableWeightTmp = (HI_U32)pstDynaRegFeCfg->au8FEWeightTable[j][k];
                    u32CombinWeight |= (u32TableWeightTmp << (8 * u32CombinWeightNum));
                    u32CombinWeightNum++;

                    if (u32CombinWeightNum == HI_ISP_AE_WEI_COMBIN_COUNT)
                    {
                        isp_ae1_wei_wdata_write(pstFeReg, u32CombinWeight);
                        u32CombinWeightNum = 0;
                        u32CombinWeight = 0;
                    }
                }
            }

            if ((u32CombinWeightNum != HI_ISP_AE_WEI_COMBIN_COUNT) && (u32CombinWeightNum != 0))
            {
                isp_ae1_wei_wdata_write(pstFeReg, u32CombinWeight);
            }

            bLutUpdate = pstDynaRegFeCfg->u8FEWightTableUpdate;
            //isp_ae1_lut_update_write(ViPipeBind, pstDynaRegFeCfg->u8FEWightTableUpdate);
        }
    }

    pstRegCfgInfo->stAlgRegCfg[0].stFeLutUpdateCfg.bAe1LutUpdate = bLutUpdate;
    return HI_SUCCESS;
}

static HI_S32 ISP_AeRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL bLutUpdate     = HI_FALSE;
    HI_BOOL bIsOfflineMode = HI_FALSE;
    HI_U16  j, k, m;
    HI_U32  u32CombinWeight = 0;
    HI_U32  u32CombinWeightNum = 0;
    ISP_AE_STATIC_CFG_S *pstStaticRegBeCfg = HI_NULL;
    ISP_AE_DYNA_CFG_S   *pstDynaRegBeCfg   = HI_NULL;
    ISP_MG_STATIC_CFG_S *pstMgStaticRegCfg = HI_NULL;
    ISP_MG_DYNA_CFG_S   *pstMgDynaRegCfg   = HI_NULL;
    S_ISPBE_REGS_TYPE    *pstBeReg         = HI_NULL;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_U32 au32CombineWgt[64] = {0};

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1AeCfg1)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);

        //ae be static
        pstStaticRegBeCfg = &pstRegCfgInfo->stAlgRegCfg[i].stAeRegCfg.stStaticRegCfg;
        pstDynaRegBeCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stAeRegCfg.stDynaRegCfg;

        //isp_ae_en_write(ViPipe, i, pstStaticRegBeCfg->u8BEEnable);
        isp_ae_crop_pos_x_write(pstBeReg, pstStaticRegBeCfg->u16BECropPosX);
        isp_ae_crop_pos_y_write(pstBeReg, pstStaticRegBeCfg->u16BECropPosY);
        isp_ae_crop_out_width_write(pstBeReg, pstStaticRegBeCfg->u16BECropOutWidth - 1);
        isp_ae_crop_out_height_write(pstBeReg, pstStaticRegBeCfg->u16BECropOutHeight - 1);
        //ae be dynamic
        isp_ae_sel_write(pstBeReg, pstDynaRegBeCfg->u8BEAESel);
        isp_ae_hnum_write(pstBeReg, pstDynaRegBeCfg->u8BEWeightTableWidth);
        isp_ae_vnum_write(pstBeReg, pstDynaRegBeCfg->u8BEWeightTableHeight);
        isp_ae_skip_x_write(pstBeReg, pstDynaRegBeCfg->u8BEHistSkipX);
        isp_ae_offset_x_write(pstBeReg, pstDynaRegBeCfg->u8BEHistOffsetX);
        isp_ae_skip_y_write(pstBeReg, pstDynaRegBeCfg->u8BEHistSkipY);
        isp_ae_offset_y_write(pstBeReg, pstDynaRegBeCfg->u8BEHistOffsetY);
        isp_ae_bitmove_write(pstBeReg, pstDynaRegBeCfg->u8BEBitMove);
        isp_ae_hist_gamma_mode_write(pstBeReg, pstDynaRegBeCfg->u8BEHistGammaMode);
        isp_ae_aver_gamma_mode_write(pstBeReg, pstDynaRegBeCfg->u8BEAverGammaMode);
        isp_ae_gamma_limit_write(pstBeReg, pstDynaRegBeCfg->u8BEGammaLimit);
        isp_ae_four_plane_mode_write(pstBeReg, pstDynaRegBeCfg->u8BEFourPlaneMode);

        m = 0;
        u32CombinWeight = 0;
        u32CombinWeightNum = 0;

        for (j = 0; j < pstDynaRegBeCfg->u8BEWeightTableHeight; j++)
        {
            for (k = 0; k < pstDynaRegBeCfg->u8BEWeightTableWidth; k++)
            {
                u32CombinWeight |= ((HI_U32)pstDynaRegBeCfg->au8BEWeightTable[j][k] << (8 * u32CombinWeightNum));
                u32CombinWeightNum++;

                if (u32CombinWeightNum == HI_ISP_AE_WEI_COMBIN_COUNT)
                {
                    //isp_ae_wei_wdata_write(pstBeReg, u32CombinWeight);
                    au32CombineWgt[m++] = u32CombinWeight;
                    u32CombinWeightNum = 0;
                    u32CombinWeight = 0;
                }
            }
        }

        if (u32CombinWeightNum != HI_ISP_AE_WEI_COMBIN_COUNT
            && u32CombinWeightNum != 0)
        {
            //isp_ae_wei_wdata_write(pstBeReg, u32CombinWeight);
            au32CombineWgt[m++] = u32CombinWeight;
        }

        if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
            IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
        {
            isp_ae_wei_waddr_write(pstBeReg, 0);

            for (m = 0; m < 64; m++)
            {
                isp_ae_wei_wdata_write(pstBeReg, au32CombineWgt[m]);
            }
        }
        else
        {
            isp_ae_weight_write(pstBeReg, au32CombineWgt);
        }

        bLutUpdate = pstDynaRegBeCfg->u8BEWightTableUpdate;

        /*mg static*/
        pstMgStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stMgRegCfg.stStaticRegCfg;
        pstMgDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stMgRegCfg.stDynaRegCfg;

        //isp_la_en_write(ViPipe, i, pstMgStaticRegCfg->u8Enable);
        isp_la_crop_pos_x_write(pstBeReg, pstMgStaticRegCfg->u16CropPosX);
        isp_la_crop_pos_y_write(pstBeReg, pstMgStaticRegCfg->u16CropPosY);
        isp_la_crop_out_width_write(pstBeReg, pstMgStaticRegCfg->u16CropOutWidth - 1);
        isp_la_crop_out_height_write(pstBeReg, pstMgStaticRegCfg->u16CropOutHeight - 1);

        /*mg dynamic*/
        isp_la_hnum_write(pstBeReg, pstMgDynaRegCfg->u8ZoneWidth);
        isp_la_vnum_write(pstBeReg, pstMgDynaRegCfg->u8ZoneHeight);
        isp_la_bitmove_write(pstBeReg, pstMgDynaRegCfg->u8BitMove);
        isp_la_gamma_en_write(pstBeReg, pstMgDynaRegCfg->u8GammaMode);
        isp_la_gamma_limit_write(pstBeReg, pstMgDynaRegCfg->u8GammaLimit);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg.bAeLutUpdate = bLutUpdate | bIsOfflineMode;

    return HI_SUCCESS;
}

static HI_S32 ISP_AfRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
#ifdef CONFIG_HI_ISP_AF_SUPPORT
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bUsrResh  = HI_FALSE;
    HI_BOOL  bIdxResh  = HI_FALSE;
    HI_U8    u8BlkNum  = pstRegCfgInfo->u8CfgNum;
    ISP_AF_REG_CFG_S  *pstAfRegBeCfg = HI_NULL;
    S_ISPBE_REGS_TYPE *pstBeReg      = HI_NULL;
    ISP_CTX_S         *pstIspCtx     = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    pstAfRegBeCfg  = &pstRegCfgInfo->stAlgRegCfg[i].stBEAfRegCfg;
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
    ISP_CHECK_POINTER(pstBeReg);

    bIdxResh = (isp_af_update_index_read(pstBeReg) != pstAfRegBeCfg->u32UpdateIndex);
    bUsrResh = (bIsOfflineMode) ? (pstRegCfgInfo->unKey.bit1AfBeCfg & bIdxResh) : (pstRegCfgInfo->unKey.bit1AfBeCfg);

    if (bUsrResh)
    {
        isp_af_update_index_write(pstBeReg, pstAfRegBeCfg->u32UpdateIndex);
        isp_af_lpf_en_write(pstBeReg, pstAfRegBeCfg->bLpfEnable);
        isp_af_fir0_lpf_en_write(pstBeReg, pstAfRegBeCfg->bFir0LpfEnable);
        isp_af_fir1_lpf_en_write(pstBeReg, pstAfRegBeCfg->bFir1LpfEnable);
        isp_af_iir0_ds_en_write(pstBeReg, pstAfRegBeCfg->bIir0DsEnable);
        isp_af_iir1_ds_en_write(pstBeReg, pstAfRegBeCfg->bIir1DsEnable);
        isp_af_iir_dilate0_write(pstBeReg, pstAfRegBeCfg->u8IirDilate0);
        isp_af_iir_dilate1_write(pstBeReg, pstAfRegBeCfg->u8IirDilate1);
        isp_af_iirplg_0_write(pstBeReg, pstAfRegBeCfg->u8IirPlgGroup0);
        isp_af_iirpls_0_write(pstBeReg, pstAfRegBeCfg->u8IirPlsGroup0);
        isp_af_iirplg_1_write(pstBeReg, pstAfRegBeCfg->u8IirPlgGroup1);
        isp_af_iirpls_1_write(pstBeReg, pstAfRegBeCfg->u8IirPlsGroup1);

        //isp_af_en_write(pstBeReg, pstAfRegBeCfg->bAfEnable);
        isp_af_iir0_en0_write(pstBeReg, pstAfRegBeCfg->bIir0Enable0);
        isp_af_iir0_en1_write(pstBeReg, pstAfRegBeCfg->bIir0Enable1);
        isp_af_iir0_en2_write(pstBeReg, pstAfRegBeCfg->bIir0Enable2);
        isp_af_iir1_en0_write(pstBeReg, pstAfRegBeCfg->bIir1Enable0);
        isp_af_iir1_en1_write(pstBeReg, pstAfRegBeCfg->bIir1Enable1);
        isp_af_iir1_en2_write(pstBeReg, pstAfRegBeCfg->bIir1Enable2);
        isp_af_peak_mode_write(pstBeReg, pstAfRegBeCfg->enPeakMode);
        isp_af_squ_mode_write(pstBeReg, pstAfRegBeCfg->enSquMode);
        isp_af_hnum_write(pstBeReg, pstAfRegBeCfg->u16WindowHnum);
        isp_af_vnum_write(pstBeReg, pstAfRegBeCfg->u16WindowVnum);

        isp_af_iirg0_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain0Group0);
        isp_af_iirg0_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain0Group1);

        isp_af_iirg1_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain1Group0);
        isp_af_iirg1_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain1Group1);

        isp_af_iirg2_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain2Group0);
        isp_af_iirg2_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain2Group1);

        isp_af_iirg3_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain3Group0);
        isp_af_iirg3_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain3Group1);

        isp_af_iirg4_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain4Group0);
        isp_af_iirg4_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain4Group1);

        isp_af_iirg5_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain5Group0);
        isp_af_iirg5_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain5Group1);

        isp_af_iirg6_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain6Group0);
        isp_af_iirg6_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16IirGain6Group1);

        isp_af_iirshift0_0_write(pstBeReg, pstAfRegBeCfg->u16Iir0ShiftGroup0);
        isp_af_iirshift0_1_write(pstBeReg, pstAfRegBeCfg->u16Iir1ShiftGroup0);
        isp_af_iirshift0_2_write(pstBeReg, pstAfRegBeCfg->u16Iir2ShiftGroup0);
        isp_af_iirshift0_3_write(pstBeReg, pstAfRegBeCfg->u16Iir3ShiftGroup0);
        isp_af_iirshift1_0_write(pstBeReg, pstAfRegBeCfg->u16Iir0ShiftGroup1);
        isp_af_iirshift1_1_write(pstBeReg, pstAfRegBeCfg->u16Iir1ShiftGroup1);
        isp_af_iirshift1_2_write(pstBeReg, pstAfRegBeCfg->u16Iir2ShiftGroup1);
        isp_af_iirshift1_3_write(pstBeReg, pstAfRegBeCfg->u16Iir3ShiftGroup1);

        isp_af_firh0_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16FirHGain0Group0);
        isp_af_firh0_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16FirHGain0Group1);

        isp_af_firh1_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16FirHGain1Group0);
        isp_af_firh1_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16FirHGain1Group1);

        isp_af_firh2_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16FirHGain2Group0);
        isp_af_firh2_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16FirHGain2Group1);

        isp_af_firh3_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16FirHGain3Group0);
        isp_af_firh3_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16FirHGain3Group1);

        isp_af_firh4_0_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16FirHGain4Group0);
        isp_af_firh4_1_write(pstBeReg, (HI_U32)pstAfRegBeCfg->s16FirHGain4Group1);

        /* AF BE crop */
        isp_af_crop_en_write(pstBeReg, pstAfRegBeCfg->bCropEnable);
        if (pstAfRegBeCfg->bCropEnable)
        {
            isp_af_pos_x_write(pstBeReg, pstAfRegBeCfg->u16CropPosX);
            isp_af_pos_y_write(pstBeReg, pstAfRegBeCfg->u16CropPosY);
            isp_af_crop_hsize_write(pstBeReg, pstAfRegBeCfg->u16CropHsize - 1);
            isp_af_crop_vsize_write(pstBeReg, pstAfRegBeCfg->u16CropVsize - 1);
        }

        /* AF BE raw cfg */
        isp_af_sel_write(pstBeReg, pstAfRegBeCfg->u8AfPosSel);
        isp_af_raw_mode_write(pstBeReg, pstAfRegBeCfg->bRawMode);
        isp_af_gain_lmt_write(pstBeReg, pstAfRegBeCfg->u8GainLimit);
        isp_af_gamma_write(pstBeReg, pstAfRegBeCfg->u8Gamma);
        isp_af_bayer_mode_write(pstBeReg, pstAfRegBeCfg->u8BayerMode);
        isp_af_offset_en_write(pstBeReg, pstAfRegBeCfg->bOffsetEnable);
        isp_af_offset_gr_write(pstBeReg, pstAfRegBeCfg->u16OffsetGr);
        isp_af_offset_gb_write(pstBeReg, pstAfRegBeCfg->u16OffsetGb);

        /* AF BE pre median filter */
        isp_af_mean_en_write(pstBeReg, pstAfRegBeCfg->bMeanEnable);
        isp_af_mean_thres_write(pstBeReg, 0xffff - pstAfRegBeCfg->u16MeanThres);

        /* level depend gain */
        isp_af_iir0_ldg_en_write (pstBeReg, pstAfRegBeCfg->bIir0LdgEnable);
        isp_af_iir_thre0_l_write (pstBeReg, pstAfRegBeCfg->u16IirThre0Low    );
        isp_af_iir_thre0_h_write (pstBeReg, pstAfRegBeCfg->u16IirThre0High   );
        isp_af_iir_slope0_l_write(pstBeReg, pstAfRegBeCfg->u16IirSlope0Low   );
        isp_af_iir_slope0_h_write(pstBeReg, pstAfRegBeCfg->u16IirSlope0High  );
        isp_af_iir_gain0_l_write (pstBeReg, pstAfRegBeCfg->u16IirGain0Low    );
        isp_af_iir_gain0_h_write (pstBeReg, pstAfRegBeCfg->u16IirGain0High   );

        isp_af_iir1_ldg_en_write (pstBeReg, pstAfRegBeCfg->bIir1LdgEnable);
        isp_af_iir_thre1_l_write (pstBeReg, pstAfRegBeCfg->u16IirThre1Low    );
        isp_af_iir_thre1_h_write (pstBeReg, pstAfRegBeCfg->u16IirThre1High   );
        isp_af_iir_slope1_l_write(pstBeReg, pstAfRegBeCfg->u16IirSlope1Low   );
        isp_af_iir_slope1_h_write(pstBeReg, pstAfRegBeCfg->u16IirSlope1High  );
        isp_af_iir_gain1_l_write (pstBeReg, pstAfRegBeCfg->u16IirGain1Low  );
        isp_af_iir_gain1_h_write (pstBeReg, pstAfRegBeCfg->u16IirGain1High );

        isp_af_fir0_ldg_en_write (pstBeReg, pstAfRegBeCfg->bFir0LdgEnable);
        isp_af_fir_thre0_l_write (pstBeReg, pstAfRegBeCfg->u16FirThre0Low    );
        isp_af_fir_thre0_h_write (pstBeReg, pstAfRegBeCfg->u16FirThre0High   );
        isp_af_fir_slope0_l_write(pstBeReg, pstAfRegBeCfg->u16FirSlope0Low    );
        isp_af_fir_slope0_h_write(pstBeReg, pstAfRegBeCfg->u16FirSlope0High   );
        isp_af_fir_gain0_l_write (pstBeReg, pstAfRegBeCfg->u16FirGain0Low     );
        isp_af_fir_gain0_h_write (pstBeReg, pstAfRegBeCfg->u16FirGain0High    );

        isp_af_fir1_ldg_en_write (pstBeReg, pstAfRegBeCfg->bFir1LdgEnable);
        isp_af_fir_thre1_l_write (pstBeReg, pstAfRegBeCfg->u16FirThre1Low    );
        isp_af_fir_thre1_h_write (pstBeReg, pstAfRegBeCfg->u16FirThre1High   );
        isp_af_fir_slope1_l_write(pstBeReg, pstAfRegBeCfg->u16FirSlope1Low   );
        isp_af_fir_slope1_h_write(pstBeReg, pstAfRegBeCfg->u16FirSlope1High  );
        isp_af_fir_gain1_l_write (pstBeReg, pstAfRegBeCfg->u16FirGain1Low  );
        isp_af_fir_gain1_h_write (pstBeReg, pstAfRegBeCfg->u16FirGain1High );

        /* AF BE coring */
        isp_af_iir_thre0_c_write (pstBeReg, pstAfRegBeCfg->u16IirThre0Coring   );
        isp_af_iir_slope0_c_write(pstBeReg, pstAfRegBeCfg->u16IirSlope0Coring  );
        isp_af_iir_peak0_c_write (pstBeReg, pstAfRegBeCfg->u16IirPeak0Coring  );

        isp_af_iir_thre1_c_write (pstBeReg, pstAfRegBeCfg->u16IirThre1Coring   );
        isp_af_iir_slope1_c_write(pstBeReg, pstAfRegBeCfg->u16IirSlope1Coring  );
        isp_af_iir_peak1_c_write (pstBeReg, pstAfRegBeCfg->u16IirPeak1Coring  );

        isp_af_fir_thre0_c_write (pstBeReg, pstAfRegBeCfg->u16FirThre0Coring   );
        isp_af_fir_slope0_c_write(pstBeReg, pstAfRegBeCfg->u16FirSlope0Coring  );
        isp_af_fir_peak0_c_write (pstBeReg, pstAfRegBeCfg->u16FirPeak0Coring  );

        isp_af_fir_thre1_c_write (pstBeReg, pstAfRegBeCfg->u16FirThre1Coring   );
        isp_af_fir_slope1_c_write(pstBeReg, pstAfRegBeCfg->u16FirSlope1Coring  );
        isp_af_fir_peak1_c_write (pstBeReg, pstAfRegBeCfg->u16FirPeak1Coring  );

        /* high luma counter */
        isp_af_hilight_write(pstBeReg, pstAfRegBeCfg->u8HilighThre );

        /* AF output shift */
        isp_af_acc_shift0_h_write(pstBeReg, pstAfRegBeCfg->u16AccShift0H );
        isp_af_acc_shift1_h_write(pstBeReg, pstAfRegBeCfg->u16AccShift1H );
        isp_af_acc_shift0_v_write(pstBeReg, pstAfRegBeCfg->u16AccShift0V );
        isp_af_acc_shift1_v_write(pstBeReg, pstAfRegBeCfg->u16AccShift1V );
        isp_af_acc_shift_y_write(pstBeReg, pstAfRegBeCfg->u16AccShiftY );
        isp_af_cnt_shift_y_write(pstBeReg, pstAfRegBeCfg->u16ShiftCountY);
        isp_af_cnt_shift0_v_write(pstBeReg, ISP_AF_CNT_SHIFT0_V_DEFAULT);
        isp_af_cnt_shift0_h_write(pstBeReg, 0x0);
        isp_af_cnt_shift1_h_write(pstBeReg, 0x0);
        isp_af_cnt_shift1_v_write(pstBeReg, 0x0);

        pstRegCfgInfo->unKey.bit1AfBeCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }
#endif

    return HI_SUCCESS;
}

static HI_S32 ISP_FeAwbRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    HI_U32 i;
    VI_PIPE ViPipeBind;
    ISP_AWB_REG_DYN_CFG_S  *pstAwbRegDynCfg = HI_NULL;
    ISP_AWB_REG_STA_CFG_S  *pstAwbRegStaCfg = HI_NULL;
    S_ISPFE_REGS_TYPE      *pstFeReg        = HI_NULL;
    ISP_CTX_S              *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (pstIspCtx->stWdrAttr.bMastPipe)
    {
        for (i = 0; i < pstIspCtx->stWdrAttr.stDevBindPipe.u32Num; i++)
        {
            ViPipeBind = pstIspCtx->stWdrAttr.stDevBindPipe.PipeId[i];
            ISP_CHECK_PIPE(ViPipeBind);
            pstFeReg = (S_ISPFE_REGS_TYPE *)ISP_GetFeVirAddr(ViPipeBind);
            ISP_CHECK_POINTER(pstFeReg);

            if (HI_TRUE == pstIspCtx->stLinkage.bSnapState)
            {
                isp_wb1_gain_write(pstFeReg, pstIspCtx->stSnapIspInfo.stIspCfgInfo.au32WhiteBalanceGain);
            }

            if (pstRegCfgInfo->unKey.bit1AwbDynCfg)
            {
                pstAwbRegDynCfg = &pstRegCfgInfo->stAlgRegCfg[0].stAwbRegCfg.stAwbRegDynCfg;
                isp_wb1_gain_write(pstFeReg, pstAwbRegDynCfg->au32FEWhiteBalanceGain);
                isp_fe_wb1_en_write(pstFeReg, pstAwbRegDynCfg->u8FEWbWorkEn);
            }

            pstAwbRegStaCfg = &pstRegCfgInfo->stAlgRegCfg[0].stAwbRegCfg.stAwbRegStaCfg;

            if (pstAwbRegStaCfg->bFEAwbStaCfg)
            {
                pstAwbRegStaCfg = &pstRegCfgInfo->stAlgRegCfg[0].stAwbRegCfg.stAwbRegStaCfg;
                isp_wb1_clip_value_write(pstFeReg, pstAwbRegStaCfg->u32FEClipValue);
            }
        }
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_AwbCCSet(S_ISPBE_REGS_TYPE *pstBeReg, HI_U16 *pu16BeCC)
{
    isp_cc_coef00_write(pstBeReg, CCM_CONVERT(CCM_CONVERT_PRE(pu16BeCC[0])));
    isp_cc_coef01_write(pstBeReg, CCM_CONVERT(CCM_CONVERT_PRE(pu16BeCC[1])));
    isp_cc_coef02_write(pstBeReg, CCM_CONVERT(CCM_CONVERT_PRE(pu16BeCC[2])));
    isp_cc_coef10_write(pstBeReg, CCM_CONVERT(CCM_CONVERT_PRE(pu16BeCC[3])));
    isp_cc_coef11_write(pstBeReg, CCM_CONVERT(CCM_CONVERT_PRE(pu16BeCC[4])));
    isp_cc_coef12_write(pstBeReg, CCM_CONVERT(CCM_CONVERT_PRE(pu16BeCC[5])));
    isp_cc_coef20_write(pstBeReg, CCM_CONVERT(CCM_CONVERT_PRE(pu16BeCC[6])));
    isp_cc_coef21_write(pstBeReg, CCM_CONVERT(CCM_CONVERT_PRE(pu16BeCC[7])));
    isp_cc_coef22_write(pstBeReg, CCM_CONVERT(CCM_CONVERT_PRE(pu16BeCC[8])));

    return HI_SUCCESS;
}

static HI_S32 ISP_AwbGainSet(S_ISPBE_REGS_TYPE *pstBeReg, HI_U32 *pu32BeWBGain)
{
    isp_wb_rgain_write(pstBeReg, pu32BeWBGain[0]);
    isp_wb_grgain_write(pstBeReg, pu32BeWBGain[1]);
    isp_wb_gbgain_write(pstBeReg, pu32BeWBGain[2]);
    isp_wb_bgain_write(pstBeReg, pu32BeWBGain[3]);

    return HI_SUCCESS;
}

#if 0
static HI_S32 ISP_StitchAwbSyncCfg(VI_PIPE ViPipe, ISP_AWB_REG_DYN_CFG_S *pstAwbRegDynCfg, HI_U8 i)
{
    HI_S32 k;
    VI_PIPE ViPipeS;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_REGCFG_S *pstRegCfgS = HI_NULL;
    ISP_AWB_REG_DYN_CFG_S *pstAwbRegDynCfgS = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (HI_TRUE == pstIspCtx->stIspParaRec.bInit)
    {
        if (HI_TRUE == pstIspCtx->stStitchAttr.bMainPipe)
        {
            for (k = 0; k < pstIspCtx->stStitchAttr.u8StitchPipeNum; k++)
            {
                ViPipeS = pstIspCtx->stStitchAttr.as8StitchBindId[k];
                ISP_CHECK_PIPE(ViPipeS);
                ISP_REGCFG_GET_CTX(ViPipeS, pstRegCfgS);
                pstAwbRegDynCfgS = &pstRegCfgS->stRegCfg.stAlgRegCfg[i].stAwbRegCfg.stAwbRegDynCfg;

                ISP_AwbCCSet(ViPipeS, pstAwbRegDynCfgS->au16BEColorMatrix, i);
                ISP_AwbGainSet(ViPipeS, pstAwbRegDynCfgS->au32BEWhiteBalanceGain, i);
            }
        }
    }
    else
    {
        ISP_AwbCCSet(ViPipe, pstAwbRegDynCfg->au16BEColorMatrix, i);
        ISP_AwbGainSet(ViPipe, pstAwbRegDynCfg->au32BEWhiteBalanceGain, i);
    }

    return HI_SUCCESS;
}
#endif
static HI_S32 ISP_AwbRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    ISP_AWB_REG_DYN_CFG_S  *pstAwbRegDynCfg = HI_NULL;
    ISP_AWB_REG_STA_CFG_S  *pstAwbRegStaCfg = HI_NULL;
    ISP_AWB_REG_USR_CFG_S  *pstAwbRegUsrCfg = HI_NULL;
    S_ISPBE_REGS_TYPE      *pstBeReg        = HI_NULL;
    S_ISPBE_REGS_TYPE      *pstBeRegEx      = HI_NULL;
    HI_BOOL  bIsOfflineMode;
    ISP_CTX_S *pstIspCtx   = HI_NULL;
    HI_BOOL bIdxResh, bUsrResh;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
    ISP_CHECK_POINTER(pstBeReg);

    pstAwbRegDynCfg = &pstRegCfgInfo->stAlgRegCfg[i].stAwbRegCfg.stAwbRegDynCfg;
    pstAwbRegStaCfg = &pstRegCfgInfo->stAlgRegCfg[i].stAwbRegCfg.stAwbRegStaCfg;
    pstAwbRegUsrCfg = &pstRegCfgInfo->stAlgRegCfg[i].stAwbRegCfg.stAwbRegUsrCfg;

    if (HI_TRUE == pstIspCtx->stLinkage.bSnapState)
    {
        if (HI_TRUE == pstIspCtx->stLinkage.bLoadCCM)
        {
            ISP_AwbCCSet(pstBeReg, pstIspCtx->stSnapIspInfo.stIspCfgInfo.au16CapCCM);
        }
        else
        {
            ISP_AwbCCSet(pstBeReg, pstAwbRegDynCfg->au16BEColorMatrix);
        }

        ISP_AwbGainSet(pstBeReg, pstIspCtx->stSnapIspInfo.stIspCfgInfo.au32WhiteBalanceGain);
    }

    if (pstRegCfgInfo->unKey.bit1AwbDynCfg)
    {
        if (HI_TRUE != pstIspCtx->stLinkage.bSnapState)
        {
            if (ISP_SNAP_PICTURE != pstIspCtx->stLinkage.enSnapPipeMode)
            {
                ISP_AwbCCSet(pstBeReg, pstAwbRegDynCfg->au16BEColorMatrix);
                ISP_AwbGainSet(pstBeReg, pstAwbRegDynCfg->au32BEWhiteBalanceGain);
            }
        }

        if ((IS_ONLINE_MODE(pstIspCtx->stLinkage.enPictureRunningMode) || \
             IS_SIDEBYSIDE_MODE(pstIspCtx->stLinkage.enPictureRunningMode)) && \
            (ISP_SNAP_PREVIEW == pstIspCtx->stLinkage.enSnapPipeMode))
        {
            ISP_CHECK_PIPE(pstIspCtx->stLinkage.s32PicturePipeId);
            pstBeRegEx = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(pstIspCtx->stLinkage.s32PicturePipeId, i);
            ISP_CHECK_POINTER(pstBeRegEx);
            ISP_AwbCCSet(pstBeRegEx, pstAwbRegDynCfg->au16BEColorMatrix);
            ISP_AwbGainSet(pstBeRegEx, pstAwbRegDynCfg->au32BEWhiteBalanceGain);
        }

        isp_awb_threshold_max_write(pstBeReg, pstAwbRegDynCfg->u16BEMeteringWhiteLevelAwb);
        isp_awb_threshold_min_write(pstBeReg, pstAwbRegDynCfg->u16BEMeteringBlackLevelAwb);
        isp_awb_cr_ref_max_write(pstBeReg, pstAwbRegDynCfg->u16BEMeteringCrRefMaxAwb);
        isp_awb_cr_ref_min_write(pstBeReg, pstAwbRegDynCfg->u16BEMeteringCrRefMinAwb);
        isp_awb_cb_ref_max_write(pstBeReg, pstAwbRegDynCfg->u16BEMeteringCbRefMaxAwb);
        isp_awb_cb_ref_min_write(pstBeReg, pstAwbRegDynCfg->u16BEMeteringCbRefMinAwb);

        //isp_wb_en_write(pstBeReg, pstAwbRegDynCfg->u8BEWbWorkEn);
        //isp_cc_en_write(pstBeReg, pstAwbRegDynCfg->u8BECcEn);

        //isp_cc_colortone_en_write(pstBeReg, pstAwbRegDynCfg->u16BECcColortoneEn);
        isp_cc_r_gain_write(pstBeReg, pstAwbRegDynCfg->u16BECcRGain);
        isp_cc_g_gain_write(pstBeReg, pstAwbRegDynCfg->u16BECcGGain);
        isp_cc_b_gain_write(pstBeReg, pstAwbRegDynCfg->u16BECcBGain);

        isp_cc_prot_en_write(pstBeReg, HI_ISP_CCM_PROT_EN_DEFAULT);
        isp_cc_recover_en_write(pstBeReg, HI_ISP_CCM_RECOVER_EN_DEFAULT);

        isp_awb_crop_pos_x_write(pstBeReg, pstAwbRegDynCfg->u32BECropPosX);
        isp_awb_crop_pos_y_write(pstBeReg, pstAwbRegDynCfg->u32BECropPosY);
        isp_awb_crop_out_width_write(pstBeReg, pstAwbRegDynCfg->u32BECropOutWidth - 1);
        isp_awb_crop_out_height_write(pstBeReg, pstAwbRegDynCfg->u32BECropOutHeight - 1);
    }

    if (pstAwbRegStaCfg->bBEAwbStaCfg)
    {
        isp_awb_bitmove_write(pstBeReg, pstAwbRegStaCfg->u8BEAwbBitmove);
        //isp_awb_en_write(pstBeReg, pstAwbRegStaCfg->u8BEAwbWorkEn);

        isp_awb_stat_raddr_write(pstBeReg, pstAwbRegStaCfg->u32BEAwbStatRaddr);

        isp_cc_prot_en_write(pstBeReg, HI_ISP_CCM_PROT_EN_DEFAULT);
        isp_cc_data_cc_thd0_write(pstBeReg, HI_ISP_CCM_CC_THD0_DEFAULT);
        isp_cc_data_cc_thd1_write(pstBeReg, HI_ISP_CCM_CC_THD1_DEFAULT);
        isp_cc_cc_prot_ratio_write(pstBeReg, HI_ISP_CCM_CC_PROT_RATIO_DEFAULT);
        isp_cc_data_rr_thd0_write(pstBeReg, HI_ISP_CCM_RR_THD0_DEFAULT);
        isp_cc_data_rr_thd1_write(pstBeReg, HI_ISP_CCM_RR_THD1_DEFAULT);
        isp_cc_data_gg_thd0_write(pstBeReg, HI_ISP_CCM_GG_THD0_DEFAULT);
        isp_cc_data_gg_thd1_write(pstBeReg, HI_ISP_CCM_GG_THD1_DEFAULT);
        isp_cc_data_bb_thd0_write(pstBeReg, HI_ISP_CCM_BB_THD0_DEFAULT);
        isp_cc_data_bb_thd1_write(pstBeReg, HI_ISP_CCM_BB_THD1_DEFAULT);
        isp_cc_max_rgb_thd_write(pstBeReg, HI_ISP_CCM_MAX_RGB_DEFAULT);
        isp_cc_rgb_prot_ratio_write(pstBeReg, HI_ISP_CCM_RGB_PROT_RATIO_DEFAULT);
        isp_cc_recover_en_write(pstBeReg, HI_ISP_CCM_RECOVER_EN_DEFAULT);
        isp_cc_lumin_ratio_write(pstBeReg, HI_ISP_CCM_LUM_RATIO_DEFAULT);
        isp_cc_hue_ratio_write(pstBeReg, HI_ISP_CCM_HUE_RATIO_DEFAULT);

        isp_cc_in_dc0_write(pstBeReg, pstAwbRegStaCfg->u32BECcInDc0);
        isp_cc_in_dc1_write(pstBeReg, pstAwbRegStaCfg->u32BECcInDc1);
        isp_cc_in_dc2_write(pstBeReg, pstAwbRegStaCfg->u32BECcInDc2);
        isp_cc_out_dc0_write(pstBeReg, pstAwbRegStaCfg->u32BECcOutDc0);
        isp_cc_out_dc1_write(pstBeReg, pstAwbRegStaCfg->u32BECcOutDc1);
        isp_cc_out_dc2_write(pstBeReg, pstAwbRegStaCfg->u32BECcOutDc2);
        isp_wb_clip_value_write(pstBeReg, pstAwbRegStaCfg->u32BEWbClipValue);
        isp_awb_offset_comp_write(pstBeReg, pstAwbRegStaCfg->u16BEAwbOffsetComp);

        pstAwbRegStaCfg->bBEAwbStaCfg = 0;
    }

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    bIdxResh = (isp_awb_update_index_read(pstBeReg) != pstAwbRegUsrCfg->u32UpdateIndex);
    bUsrResh = (bIsOfflineMode) ? (pstAwbRegUsrCfg->bResh & bIdxResh) : (pstAwbRegUsrCfg->bResh);

    if (bUsrResh)
    {
        isp_awb_update_index_write(pstBeReg, pstAwbRegUsrCfg->u32UpdateIndex);

        isp_awb_sel_write(pstBeReg, pstAwbRegUsrCfg->enBEAWBSwitch);
        isp_awb_hnum_write(pstBeReg, pstAwbRegUsrCfg->u16BEZoneCol);
        isp_awb_vnum_write(pstBeReg, pstAwbRegUsrCfg->u16BEZoneRow);
        pstAwbRegUsrCfg->bResh = bIsOfflineMode;  //if online mode, bResh=0; if offline mode, bResh=1; but only index != will resh
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_SharpenDynaRegConfig(S_ISPBE_REGS_TYPE *pstBeReg, ISP_SHARPEN_MPI_DYNA_REG_CFG_S *pstMpiDynaRegCfg, ISP_SHARPEN_DEFAULT_DYNA_REG_CFG_S *pstDefDynaRegCfg)
{
    isp_sharpen_bendetailctrl_write    (pstBeReg, pstMpiDynaRegCfg->bEnDetailCtrl);
    isp_sharpen_osht_dtl_wgt_write     (pstBeReg, pstMpiDynaRegCfg->u8detailOshtAmt);
    isp_sharpen_usht_dtl_wgt_write     (pstBeReg, pstMpiDynaRegCfg->u8detailUshtAmt);
    isp_sharpen_detl_oshtmul_write     (pstBeReg, pstMpiDynaRegCfg->s16detailOshtMul);
    isp_sharpen_detl_ushtmul_write     (pstBeReg, pstMpiDynaRegCfg->s16detailUshtMul);
    isp_sharpen_oshtamt_write          (pstBeReg, pstMpiDynaRegCfg->u8oshtAmt);
    isp_sharpen_ushtamt_write          (pstBeReg, pstMpiDynaRegCfg->u8ushtAmt);
    isp_sharpen_benshtctrlbyvar_write  (pstBeReg, pstMpiDynaRegCfg->u8bEnShtCtrlByVar);
    isp_sharpen_shtbldrt_write         (pstBeReg, pstMpiDynaRegCfg->u8shtBldRt);
    isp_sharpen_oshtvarthd1_write      (pstBeReg, pstMpiDynaRegCfg->u8oshtVarThd1);
    isp_sharpen_ushtvarthd1_write      (pstBeReg, pstMpiDynaRegCfg->u8ushtVarThd1);
    isp_sharpen_dirdiffsft_write       (pstBeReg, pstMpiDynaRegCfg->u8dirDiffSft);
    isp_sharpen_benlumactrl_write      (pstBeReg, pstMpiDynaRegCfg->u8bEnLumaCtrl);

    isp_sharpen_lumawgt0_write (pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[0]);
    isp_sharpen_lumawgt1_write (pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[1]);
    isp_sharpen_lumawgt2_write (pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[2]);
    isp_sharpen_lumawgt3_write (pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[3]);
    isp_sharpen_lumawgt4_write (pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[4]);
    isp_sharpen_lumawgt5_write (pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[5]);
    isp_sharpen_lumawgt6_write (pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[6]);
    isp_sharpen_lumawgt7_write (pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[7]);
    isp_sharpen_lumawgt8_write (pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[8]);
    isp_sharpen_lumawgt9_write (pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[9]);
    isp_sharpen_lumawgt10_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[10]);
    isp_sharpen_lumawgt11_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[11]);
    isp_sharpen_lumawgt12_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[12]);
    isp_sharpen_lumawgt13_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[13]);
    isp_sharpen_lumawgt14_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[14]);
    isp_sharpen_lumawgt15_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[15]);
    isp_sharpen_lumawgt16_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[16]);
    isp_sharpen_lumawgt17_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[17]);
    isp_sharpen_lumawgt18_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[18]);
    isp_sharpen_lumawgt19_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[19]);
    isp_sharpen_lumawgt20_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[20]);
    isp_sharpen_lumawgt21_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[21]);
    isp_sharpen_lumawgt22_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[22]);
    isp_sharpen_lumawgt23_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[23]);
    isp_sharpen_lumawgt24_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[24]);
    isp_sharpen_lumawgt25_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[25]);
    isp_sharpen_lumawgt26_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[26]);
    isp_sharpen_lumawgt27_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[27]);
    isp_sharpen_lumawgt28_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[28]);
    isp_sharpen_lumawgt29_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[29]);
    isp_sharpen_lumawgt30_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[30]);
    isp_sharpen_lumawgt31_write(pstBeReg, pstMpiDynaRegCfg->au8LumaWgt[31]);

    isp_sharpen_oshtvarmul_write      (pstBeReg, pstMpiDynaRegCfg->u16oshtVarMul);
    isp_sharpen_ushtvarmul_write      (pstBeReg, pstMpiDynaRegCfg->u16ushtVarMul);
    isp_sharpen_benskinctrl_write     (pstBeReg, pstMpiDynaRegCfg->u8bEnSkinCtrl);
    isp_sharpen_skinedgewgt0_write    (pstBeReg, pstMpiDynaRegCfg->u8skinEdgeWgt[0]);
    isp_sharpen_skinedgewgt1_write    (pstBeReg, pstMpiDynaRegCfg->u8skinEdgeWgt[1]);
    isp_sharpen_skinedgemul_write     (pstBeReg, pstMpiDynaRegCfg->s16skinEdgeMul);
    isp_sharpen_benchrctrl_write      (pstBeReg, pstMpiDynaRegCfg->u8bEnChrCtrl);
    isp_sharpen_chrrmul_write         (pstBeReg, pstMpiDynaRegCfg->s16chrRMul);
    isp_sharpen_chrbmul_write         (pstBeReg, pstMpiDynaRegCfg->s16chrBMul);
    isp_sharpen_chrrgain_write        (pstBeReg, pstMpiDynaRegCfg->u8chrRGain);
    isp_sharpen_chrbgain_write        (pstBeReg, pstMpiDynaRegCfg->u8chrBGain);
    isp_sharpen_chrggain_write        (pstBeReg, pstMpiDynaRegCfg->u16chrGGain);
    isp_sharpen_chrgmfgain_write      (pstBeReg, pstMpiDynaRegCfg->u16chrGmfGain);
    isp_sharpen_chrgmfmul_write       (pstBeReg, pstMpiDynaRegCfg->s16chrGmfMul);
    isp_sharpen_chrgmul_write         (pstBeReg, pstMpiDynaRegCfg->s16chrGMul);
    isp_sharpen_osht_dtl_thd0_write   (pstBeReg, pstMpiDynaRegCfg->u8detailOshtThr[0]);
    isp_sharpen_osht_dtl_thd1_write   (pstBeReg, pstMpiDynaRegCfg->u8detailOshtThr[1]);
    isp_sharpen_usht_dtl_thd0_write   (pstBeReg, pstMpiDynaRegCfg->u8detailUshtThr[0]);
    isp_sharpen_usht_dtl_thd1_write   (pstBeReg, pstMpiDynaRegCfg->u8detailUshtThr[1]);
    isp_sharpen_skinmaxu_write        (pstBeReg, pstMpiDynaRegCfg->u8skinMaxU);
    isp_sharpen_skinminu_write        (pstBeReg, pstMpiDynaRegCfg->u8skinMinU);
    isp_sharpen_skinmaxv_write        (pstBeReg, pstMpiDynaRegCfg->u8skinMaxV);
    isp_sharpen_skinminv_write        (pstBeReg, pstMpiDynaRegCfg->u8skinMinV);
    isp_sharpen_omaxgain_write        (pstBeReg, pstMpiDynaRegCfg->u16oMaxGain);
    isp_sharpen_umaxgain_write        (pstBeReg, pstMpiDynaRegCfg->u16uMaxGain);

    isp_sharpen_mfthdsftd_write       (pstBeReg, pstDefDynaRegCfg->u8mfThdSftD);
    isp_sharpen_mfthdselud_write      (pstBeReg, pstDefDynaRegCfg->u8mfThdSelUD);
    isp_sharpen_mfthdsftud_write      (pstBeReg, pstDefDynaRegCfg->u8mfThdSftUD);
    isp_sharpen_hfthdsftd_write       (pstBeReg, pstDefDynaRegCfg->u8hfThdSftD);
    isp_sharpen_hfthdselud_write      (pstBeReg, pstDefDynaRegCfg->u8hfThdSelUD);
    isp_sharpen_hfthdsftud_write      (pstBeReg, pstDefDynaRegCfg->u8hfThdSftUD);
    isp_sharpen_dirvarsft_write       (pstBeReg, pstDefDynaRegCfg->u8dirVarSft);
    isp_sharpen_range_thd0_write      (pstBeReg, pstDefDynaRegCfg->u8rangeThd0);
    isp_sharpen_range_wgt0_write      (pstBeReg, pstDefDynaRegCfg->u8rangeDirWgt0);
    isp_sharpen_range_mul_write       (pstBeReg, pstDefDynaRegCfg->s16rangeDirMul);
    isp_sharpen_hfepsvar_thr0_write   (pstBeReg, pstDefDynaRegCfg->u8hfEpsVarThr0);
    isp_sharpen_hfepsvar_thr1_write   (pstBeReg, pstDefDynaRegCfg->u8hfEpsVarThr1);
    isp_sharpen_hfepsval1_write       (pstBeReg, pstDefDynaRegCfg->u8hfEpsVal1);
    isp_sharpen_hfepsvar_mul_write    (pstBeReg, pstDefDynaRegCfg->s16hfEpsMul);
    isp_sharpen_oshtvarwgt0_write     (pstBeReg, pstDefDynaRegCfg->u8oshtVarWgt0);
    isp_sharpen_ushtvarwgt0_write     (pstBeReg, pstDefDynaRegCfg->u8ushtVarWgt0);
    isp_sharpen_oshtvardiffthd0_write (pstBeReg, pstDefDynaRegCfg->u8oshtVarDiffThd[0]);
    isp_sharpen_ushtvardiffthd0_write (pstBeReg, pstDefDynaRegCfg->u8ushtVarDiffThd[0]);
    isp_sharpen_selpixwgt_write       (pstBeReg, pstDefDynaRegCfg->u8selPixWgt);
    isp_sharpen_oshtvardiffthd1_write (pstBeReg, pstDefDynaRegCfg->u8oshtVarDiffThd[1]);
    isp_sharpen_oshtvardiffwgt1_write (pstBeReg, pstDefDynaRegCfg->u8oshtVarDiffWgt1);
    isp_sharpen_ushtvardiffthd1_write (pstBeReg, pstDefDynaRegCfg->u8ushtVarDiffThd[1]);
    isp_sharpen_ushtvardiffwgt1_write (pstBeReg, pstDefDynaRegCfg->u8ushtVarDiffWgt1);
    isp_sharpen_oshtvardiffmul_write  (pstBeReg, pstDefDynaRegCfg->s16oshtVarDiffMul);
    isp_sharpen_ushtvardiffmul_write  (pstBeReg, pstDefDynaRegCfg->s16ushtVarDiffMul);
    isp_sharpen_rmfscale_write        (pstBeReg, pstDefDynaRegCfg->u8RmfGainScale);
    isp_sharpen_bmfscale_write        (pstBeReg, pstDefDynaRegCfg->u8BmfGainScale);

    return HI_SUCCESS;
}

static HI_S32 ISP_SharpenRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bUsrResh     = HI_FALSE;
    HI_BOOL  bIdxResh     = HI_FALSE;
    HI_BOOL  bLutUpdate   = HI_FALSE;
    HI_BOOL  bCurRegNewEn = HI_FALSE;
    HI_U8    u8BlkNum     = pstRegCfgInfo->u8CfgNum;
    HI_U8    u8BufId;

    //ISP_SHARPEN_DEFAULT_DYNA_REG_CFG_S *pstSharpenDefaultDynaRegCfg = HI_NULL;
    ISP_SHARPEN_MPI_DYNA_REG_CFG_S     *pstSharpenMpiDynaRegCfg     = HI_NULL;
    ISP_SHARPEN_STATIC_REG_CFG_S       *pstSharpenStaticRegCfg      = HI_NULL;
    S_ISP_LUT_WSTT_TYPE                *pstBeLutSttReg              = HI_NULL;
    S_VIPROC_REGS_TYPE                 *pstViProcReg                = HI_NULL;
    S_ISPBE_REGS_TYPE                  *pstBeReg                    = HI_NULL;
    ISP_CTX_S                          *pstIspCtx                   = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1SharpenCfg)
    {
        //isp_sharpen_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stSharpenRegCfg.bEnable);

        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstViProcReg);

        pstSharpenStaticRegCfg      = &pstRegCfgInfo->stAlgRegCfg[i].stSharpenRegCfg.stStaticRegCfg;
        //pstSharpenDefaultDynaRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stDefaultDynaRegCfg;
        pstSharpenMpiDynaRegCfg     = &pstRegCfgInfo->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg;

        if (pstSharpenStaticRegCfg->bStaticResh)
        {
            isp_sharpen_lut_width_word_write(pstViProcReg, HI_ISP_SHARPEN_LUT_WIDTH_WORD_DEFAULT);
            isp_sharpen_mfthdseld_write      (pstBeReg, pstSharpenStaticRegCfg->u8mfThdSelD);
            isp_sharpen_hfthdseld_write      (pstBeReg, pstSharpenStaticRegCfg->u8hfThdSelD);
            isp_sharpen_dirvarscale_write    (pstBeReg, pstSharpenStaticRegCfg->u8dirVarScale);
            isp_sharpen_dirrly0_write        (pstBeReg, pstSharpenStaticRegCfg->u8dirRly[0]);
            isp_sharpen_dirrly1_write        (pstBeReg, pstSharpenStaticRegCfg->u8dirRly[1]);
            isp_sharpen_range_thd1_write     (pstBeReg, pstSharpenStaticRegCfg->u8rangeThd1);
            isp_sharpen_range_wgt1_write     (pstBeReg, pstSharpenStaticRegCfg->u8rangeDirWgt1);
            isp_sharpen_benhfeps_write       (pstBeReg, pstSharpenStaticRegCfg->u8bEnHfEps);
            isp_sharpen_hfepsval0_write      (pstBeReg, pstSharpenStaticRegCfg->u8hfEpsVal0);
            isp_sharpen_omaxchg_write        (pstBeReg, pstSharpenStaticRegCfg->u16oMaxChg);
            isp_sharpen_umaxchg_write        (pstBeReg, pstSharpenStaticRegCfg->u16uMaxChg);
            isp_sharpen_shtvarsft_write      (pstBeReg, pstSharpenStaticRegCfg->u8shtVarSft);
            isp_sharpen_oshtvarthd0_write    (pstBeReg, pstSharpenStaticRegCfg->u8oshtVarThd0);
            isp_sharpen_ushtvarthd0_write    (pstBeReg, pstSharpenStaticRegCfg->u8ushtVarThd0);
            isp_sharpen_oshtvardiffwgt0_write(pstBeReg, pstSharpenStaticRegCfg->u8oshtVarDiffWgt0);
            isp_sharpen_ushtvardiffwgt0_write(pstBeReg, pstSharpenStaticRegCfg->u8ushtVarDiffWgt0);
            isp_sharpen_oshtvarwgt1_write    (pstBeReg, pstSharpenStaticRegCfg->u8oshtVarWgt1);
            isp_sharpen_ushtvarwgt1_write    (pstBeReg, pstSharpenStaticRegCfg->u8ushtVarWgt1);
            isp_sharpen_lmtmf0_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtMF[0]);
            isp_sharpen_lmthf0_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtHF[0]);
            isp_sharpen_lmtmf1_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtMF[1]);
            isp_sharpen_lmthf1_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtHF[1]);
            isp_sharpen_lmtmf2_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtMF[2]);
            isp_sharpen_lmthf2_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtHF[2]);
            isp_sharpen_lmtmf3_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtMF[3]);
            isp_sharpen_lmthf3_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtHF[3]);
            isp_sharpen_lmtmf4_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtMF[4]);
            isp_sharpen_lmthf4_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtHF[4]);
            isp_sharpen_lmtmf5_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtMF[5]);
            isp_sharpen_lmthf5_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtHF[5]);
            isp_sharpen_lmtmf6_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtMF[6]);
            isp_sharpen_lmthf6_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtHF[6]);
            isp_sharpen_lmtmf7_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtMF[7]);
            isp_sharpen_lmthf7_write         (pstBeReg, pstSharpenStaticRegCfg->u8lmtHF[7]);

            isp_sharpen_skinsrcsel_write     (pstBeReg, pstSharpenStaticRegCfg->u8skinSrcSel);

            isp_sharpen_skinedgesft_write    (pstBeReg, pstSharpenStaticRegCfg->u8skinEdgeSft);

            isp_sharpen_skincntthd0_write    (pstBeReg, pstSharpenStaticRegCfg->u8skinCntThd[0]);
            isp_sharpen_skinedgethd0_write   (pstBeReg, pstSharpenStaticRegCfg->u8skinEdgeThd[0]);
            isp_sharpen_skinaccumthd0_write  (pstBeReg, pstSharpenStaticRegCfg->u16skinAccumThd[0]);
            isp_sharpen_skinaccumwgt0_write  (pstBeReg, pstSharpenStaticRegCfg->u8skinAccumWgt[0]);
            isp_sharpen_skincntthd1_write    (pstBeReg, pstSharpenStaticRegCfg->u8skinCntThd[1]);
            isp_sharpen_skinedgethd1_write   (pstBeReg, pstSharpenStaticRegCfg->u8skinEdgeThd[1]);
            isp_sharpen_skinaccumthd1_write  (pstBeReg, pstSharpenStaticRegCfg->u16skinAccumThd[1]);
            isp_sharpen_skinaccumwgt1_write  (pstBeReg, pstSharpenStaticRegCfg->u8skinAccumWgt[1]);

            isp_sharpen_chrrvarshift_write (pstBeReg, pstSharpenStaticRegCfg->u8chrRVarSft);
            isp_sharpen_chrrminu_write     (pstBeReg, pstSharpenStaticRegCfg->u8RcBmin);
            isp_sharpen_chrrmaxu_write     (pstBeReg, pstSharpenStaticRegCfg->u8RcBmax);
            isp_sharpen_chrrminv_write     (pstBeReg, pstSharpenStaticRegCfg->u8RcRmin);
            isp_sharpen_chrrmaxv_write     (pstBeReg, pstSharpenStaticRegCfg->u8RcRmax);
            isp_sharpen_chrrori0_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrROriCb);
            isp_sharpen_chrrori1_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrROriCr);
            isp_sharpen_chrrsft0_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrRSft[0]);
            isp_sharpen_chrrsft1_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrRSft[1]);
            isp_sharpen_chrrsft2_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrRSft[2]);
            isp_sharpen_chrrsft3_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrRSft[3]);
            isp_sharpen_chrrthd0_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrRThd[0]);
            isp_sharpen_chrrthd1_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrRThd[1]);

            isp_sharpen_chrgminu_write     (pstBeReg, pstSharpenStaticRegCfg->u8GcBmin);
            isp_sharpen_chrgmaxu_write     (pstBeReg, pstSharpenStaticRegCfg->u8GcBmax);
            isp_sharpen_chrgminv_write     (pstBeReg, pstSharpenStaticRegCfg->u8GcRmin);
            isp_sharpen_chrgmaxv_write     (pstBeReg, pstSharpenStaticRegCfg->u8GcRmax);
            isp_sharpen_chrgori0_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrGOriCb);
            isp_sharpen_chrgori1_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrGOriCr);
            isp_sharpen_chrgsft0_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrGSft[0]);
            isp_sharpen_chrgsft1_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrGSft[1]);
            isp_sharpen_chrgsft2_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrGSft[2]);
            isp_sharpen_chrgsft3_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrGSft[3]);
            isp_sharpen_chrgthd0_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrGThd[0]);
            isp_sharpen_chrgthd1_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrGThd[1]);

            isp_sharpen_chrbvarshift_write (pstBeReg, pstSharpenStaticRegCfg->u8chrBVarSft);
            isp_sharpen_chrbminu_write     (pstBeReg, pstSharpenStaticRegCfg->u8BcBmin);
            isp_sharpen_chrbmaxu_write     (pstBeReg, pstSharpenStaticRegCfg->u8BcBmax);
            isp_sharpen_chrbminv_write     (pstBeReg, pstSharpenStaticRegCfg->u8BcRmin);
            isp_sharpen_chrbmaxv_write     (pstBeReg, pstSharpenStaticRegCfg->u8BcRmax);
            isp_sharpen_chrbori0_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrBOriCb);
            isp_sharpen_chrbori1_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrBOriCr);
            isp_sharpen_chrbsft0_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrBSft[0]);
            isp_sharpen_chrbsft1_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrBSft[1]);
            isp_sharpen_chrbsft2_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrBSft[2]);
            isp_sharpen_chrbsft3_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrBSft[3]);
            isp_sharpen_chrbthd0_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrBThd[0]);
            isp_sharpen_chrbthd1_write     (pstBeReg, pstSharpenStaticRegCfg->u8chrBThd[1]);

            isp_sharpen_dirrt0_write       (pstBeReg, pstSharpenStaticRegCfg->u8dirRt[0]);
            isp_sharpen_dirrt1_write       (pstBeReg, pstSharpenStaticRegCfg->u8dirRt[1]);
            isp_sharpen_shtnoisemax_write  (pstBeReg, pstSharpenStaticRegCfg->u8shtNoiseMax);
            isp_sharpen_shtnoisemin_write  (pstBeReg, pstSharpenStaticRegCfg->u8shtNoiseMin);
            isp_sharpen_max_var_clip_write (pstBeReg, pstSharpenStaticRegCfg->u8MaxVarClipMin);

            isp_sharpen_lumasrcsel_write   (pstBeReg, pstSharpenStaticRegCfg->u8lumaSrcSel);
            isp_sharpen_skincntmul_write   (pstBeReg, pstSharpenStaticRegCfg->u8skinCntMul);
            isp_sharpen_skinaccummul_write (pstBeReg, pstSharpenStaticRegCfg->s16skinAccumMul);

            //filter
            isp_sharpen_udlpfcoef0_write   (pstBeReg, pstSharpenStaticRegCfg->s8lpfCoefUD[0]);
            isp_sharpen_udlpfcoef1_write   (pstBeReg, pstSharpenStaticRegCfg->s8lpfCoefUD[1]);
            isp_sharpen_udlpfcoef2_write   (pstBeReg, pstSharpenStaticRegCfg->s8lpfCoefUD[2]);

            isp_sharpen_dlpfcoef0_write    (pstBeReg, pstSharpenStaticRegCfg->s8lpfCoefD[0]);
            isp_sharpen_dlpfcoef1_write    (pstBeReg, pstSharpenStaticRegCfg->s8lpfCoefD[1]);
            isp_sharpen_dlpfcoef2_write    (pstBeReg, pstSharpenStaticRegCfg->s8lpfCoefD[2]);

            isp_sharpen_udhsfcoef0_write   (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefUD[0]);
            isp_sharpen_udhsfcoef1_write   (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefUD[1]);
            isp_sharpen_udhsfcoef2_write   (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefUD[2]);

            isp_sharpen_dhsfcoef0_0_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[0]);
            isp_sharpen_dhsfcoef0_1_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[1]);
            isp_sharpen_dhsfcoef0_2_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[2]);
            isp_sharpen_dhsfcoef0_3_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[3]);
            isp_sharpen_dhsfcoef0_4_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[4]);
            isp_sharpen_dhsfcoef0_5_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[5]);
            isp_sharpen_dhsfcoef0_6_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[6]);
            isp_sharpen_dhsfcoef0_7_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[7]);
            isp_sharpen_dhsfcoef0_8_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[8]);
            isp_sharpen_dhsfcoef0_9_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[9]);
            isp_sharpen_dhsfcoef0_10_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[10]);
            isp_sharpen_dhsfcoef0_11_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[11]);
            isp_sharpen_dhsfcoef0_12_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD0[12]);

            isp_sharpen_dhsfcoef1_0_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[0]);
            isp_sharpen_dhsfcoef1_1_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[1]);
            isp_sharpen_dhsfcoef1_2_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[2]);
            isp_sharpen_dhsfcoef1_3_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[3]);
            isp_sharpen_dhsfcoef1_4_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[4]);
            isp_sharpen_dhsfcoef1_5_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[5]);
            isp_sharpen_dhsfcoef1_6_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[6]);
            isp_sharpen_dhsfcoef1_7_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[7]);
            isp_sharpen_dhsfcoef1_8_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[8]);
            isp_sharpen_dhsfcoef1_9_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[9]);
            isp_sharpen_dhsfcoef1_10_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[10]);
            isp_sharpen_dhsfcoef1_11_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[11]);
            isp_sharpen_dhsfcoef1_12_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD1[12]);

            isp_sharpen_dhsfcoef2_0_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[0]);
            isp_sharpen_dhsfcoef2_1_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[1]);
            isp_sharpen_dhsfcoef2_2_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[2]);
            isp_sharpen_dhsfcoef2_3_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[3]);
            isp_sharpen_dhsfcoef2_4_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[4]);
            isp_sharpen_dhsfcoef2_5_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[5]);
            isp_sharpen_dhsfcoef2_6_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[6]);
            isp_sharpen_dhsfcoef2_7_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[7]);
            isp_sharpen_dhsfcoef2_8_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[8]);
            isp_sharpen_dhsfcoef2_9_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[9]);
            isp_sharpen_dhsfcoef2_10_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[10]);
            isp_sharpen_dhsfcoef2_11_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[11]);
            isp_sharpen_dhsfcoef2_12_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD2[12]);

            isp_sharpen_dhsfcoef3_0_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[0]);
            isp_sharpen_dhsfcoef3_1_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[1]);
            isp_sharpen_dhsfcoef3_2_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[2]);
            isp_sharpen_dhsfcoef3_3_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[3]);
            isp_sharpen_dhsfcoef3_4_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[4]);
            isp_sharpen_dhsfcoef3_5_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[5]);
            isp_sharpen_dhsfcoef3_6_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[6]);
            isp_sharpen_dhsfcoef3_7_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[7]);
            isp_sharpen_dhsfcoef3_8_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[8]);
            isp_sharpen_dhsfcoef3_9_write  (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[9]);
            isp_sharpen_dhsfcoef3_10_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[10]);
            isp_sharpen_dhsfcoef3_11_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[11]);
            isp_sharpen_dhsfcoef3_12_write (pstBeReg, pstSharpenStaticRegCfg->s8hsfCoefD3[12]);

            isp_sharpen_udlpfsft_write     (pstBeReg, pstSharpenStaticRegCfg->u8lpfSftUD);
            isp_sharpen_dlpfsft_write      (pstBeReg, pstSharpenStaticRegCfg->u8lpfSftD);
            isp_sharpen_udhsfsft_write     (pstBeReg, pstSharpenStaticRegCfg->u8hsfSftUD);
            isp_sharpen_dhsfsft_write      (pstBeReg, pstSharpenStaticRegCfg->u8hsfSftD);

            isp_sharpen_ben8dir_sel_write  (pstBeReg, pstSharpenStaticRegCfg->bEnShp8Dir);

            //shoot ctrl
            isp_sharpen_hfgain_sft_write   (pstBeReg, pstSharpenStaticRegCfg->u8hfGainSft);
            isp_sharpen_mfgain_sft_write   (pstBeReg, pstSharpenStaticRegCfg->u8mfGainSft);
            isp_sharpen_benshtvar_sel_write(pstBeReg, pstSharpenStaticRegCfg->u8shtVarSel);
            isp_sharpen_shtvar5x5_sft_write(pstBeReg, pstSharpenStaticRegCfg->u8shtVar5x5Sft);
            isp_sharpen_detailthd_sel_write(pstBeReg, pstSharpenStaticRegCfg->u8detailThdSel);
            isp_sharpen_dtl_thdsft_write   (pstBeReg, pstSharpenStaticRegCfg->u8detailThdSft);

            pstSharpenStaticRegCfg->bStaticResh = HI_FALSE;
        }

        bIdxResh = (isp_sharpen_update_index_read(pstBeReg) != pstSharpenMpiDynaRegCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstSharpenMpiDynaRegCfg->bResh & bIdxResh) : (pstSharpenMpiDynaRegCfg->bResh);

        if (bUsrResh)
        {
            isp_sharpen_update_index_write(pstBeReg, pstSharpenMpiDynaRegCfg->u32UpdateIndex);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                if (HI_TRUE != pstSharpenMpiDynaRegCfg->bSwitchMode)
                {
                    /*online Lut2stt regconfig*/
                    u8BufId = pstSharpenMpiDynaRegCfg->u8BufId;
                    pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, u8BufId);
                    ISP_CHECK_NULLPTR(pstBeLutSttReg);

                    isp_sharpen_lut_wstt_write(pstBeLutSttReg, pstSharpenMpiDynaRegCfg->u16mfGainD, \
                                               pstSharpenMpiDynaRegCfg->u16mfGainUD, pstSharpenMpiDynaRegCfg->u16hfGainD, \
                                               pstSharpenMpiDynaRegCfg->u16hfGainUD);

                    isp_sharpen_lut_wstt_addr_write(ViPipe, i, u8BufId, pstViProcReg);
                    isp_sharpen_stt2lut_en_write(pstBeReg, HI_TRUE);
                    isp_sharpen_stt2lut_regnew_write(pstBeReg, HI_TRUE);

                    pstSharpenMpiDynaRegCfg->u8BufId = 1 - u8BufId;

                    bCurRegNewEn = HI_TRUE;
                }
            }
            else
            {
                isp_sharpen_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstSharpenMpiDynaRegCfg->u16mfGainD, \
                                           pstSharpenMpiDynaRegCfg->u16mfGainUD, pstSharpenMpiDynaRegCfg->u16hfGainD, \
                                           pstSharpenMpiDynaRegCfg->u16hfGainUD);

                isp_sharpen_stt2lut_en_write(pstBeReg, HI_TRUE);
                isp_sharpen_stt2lut_regnew_write(pstBeReg, HI_TRUE);
            }

            pstSharpenMpiDynaRegCfg->bResh       = bIsOfflineMode;
            pstSharpenMpiDynaRegCfg->bSwitchMode = HI_FALSE;
        }

        if (HI_TRUE == pstRegCfgInfo->stAlgRegCfg[i].stSharpenRegCfg.bLut2SttEn)
        {
            bLutUpdate = pstSharpenMpiDynaRegCfg->bSwitchMode ? (HI_FALSE) : (pstSharpenMpiDynaRegCfg->bPreRegNewEn);
        }

        pstSharpenMpiDynaRegCfg->bPreRegNewEn = bCurRegNewEn;
        pstRegCfgInfo->unKey.bit1SharpenCfg   = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg.bSharpenLutUpdate = bLutUpdate | bIsOfflineMode;

    return HI_SUCCESS;
}


static HI_S32 ISP_EdgeMarkRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
#ifdef CONFIG_HI_ISP_EDGEMARK_SUPPORT
    HI_BOOL  bIsOfflineMode;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    ISP_CTX_S         *pstIspCtx  = HI_NULL;
    S_ISPBE_REGS_TYPE *pstBeReg   = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1EdgeMarkCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        //isp_sharpen_benmarkedge_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stEdgeMarkRegCfg.bEnable);
        isp_sharpen_mark_thdsft_write (pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stEdgeMarkRegCfg.u8markEdgeSft);
        isp_sharpen_mark_udata_write (pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stEdgeMarkRegCfg.u8uMarkValue);
        isp_sharpen_mark_vdata_write (pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stEdgeMarkRegCfg.u8vMarkValue);
        isp_sharpen_mark_thd_write    (pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stEdgeMarkRegCfg.u8MarkEdgeThd);
    }

    pstRegCfgInfo->unKey.bit1EdgeMarkCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
#endif

    return HI_SUCCESS;
}


static HI_S32 ISP_HlcRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
#ifdef CONFIG_HI_ISP_HLC_SUPPORT
    HI_BOOL  bIsOfflineMode;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    ISP_CTX_S         *pstIspCtx  = HI_NULL;
    S_ISPBE_REGS_TYPE *pstBeReg   = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1HlcCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        //isp_hlc_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stHlcRegCfg.bEnable);
        isp_hlc_y_max_load_value_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stHlcRegCfg.u8yMaxLoadValue);
        isp_hlc_y_max_value_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stHlcRegCfg.u8yMaxValue);
        isp_hlc_y_min_load_value_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stHlcRegCfg.u8yMinLoadValue);
        isp_hlc_y_min_value_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stHlcRegCfg.u8yMinValue);
        isp_hlc_c_max_load_value_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stHlcRegCfg.u8cMaxLoadValue);
        isp_hlc_c_max_value_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stHlcRegCfg.u8cMaxValue);
        isp_hlc_c_min_load_value_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stHlcRegCfg.u8cMinLoadValue);
        isp_hlc_c_min_value_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stHlcRegCfg.u8cMinValue);
    }

    pstRegCfgInfo->unKey.bit1HlcCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
#endif

    return HI_SUCCESS;
}


static HI_S32 ISP_DemRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL bIsOfflineMode;
    HI_BOOL bGfLutUpdate  = HI_FALSE;
    HI_U8   u8BlkNum      = pstRegCfgInfo->u8CfgNum;
    HI_U16  j;

    ISP_DEMOSAIC_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_DEMOSAIC_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    S_ISPBE_REGS_TYPE         *pstBeReg        = HI_NULL;
    ISP_CTX_S                 *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)\
                      || IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1DemCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        //isp_dmnr_vhdm_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stDemRegCfg.bVhdmEnable);
        //isp_dmnr_nddm_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stDemRegCfg.bNddmEnable);

        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stDemRegCfg.stStaticRegCfg;
        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stDemRegCfg.stDynaRegCfg;

        if (pstStaticRegCfg->bResh)   /*static*/
        {
            isp_demosaic_desat_enable_write(pstBeReg, pstStaticRegCfg->bDeSatEnable);
            isp_demosaic_g_intp_control_write(pstBeReg, pstStaticRegCfg->bGIntpCtrlEnable);
            isp_demosaic_ahd_en_write(pstBeReg, pstStaticRegCfg->bAHDEnable);
            isp_demosaic_de_fake_en_write(pstBeReg, pstStaticRegCfg->bDeFakeEnable);
            isp_demosaic_lpf_f3_write(pstBeReg, pstStaticRegCfg->u8Lpff3);
            isp_demosaic_desat_thresh1_write(pstBeReg, pstStaticRegCfg->u16DeSatThresh1);
            isp_demosaic_desat_thresh2_write(pstBeReg, pstStaticRegCfg->u16DeSatThresh2);
            isp_demosaic_desat_hig_write(pstBeReg, pstStaticRegCfg->u16DeSatHig);
            isp_demosaic_desat_protect_sl_write(pstBeReg, pstStaticRegCfg->u16DeSatProtSl);
            isp_demosaic_bld_limit1_write(pstBeReg, pstStaticRegCfg->u8hvBlendLimit1);
            isp_demosaic_bld_limit2_write(pstBeReg, pstStaticRegCfg->u8hvBlendLimit2);
            isp_demosaic_ahd_par1_write(pstBeReg, pstStaticRegCfg->u16AhdPart1);
            isp_demosaic_ahd_par2_write(pstBeReg, pstStaticRegCfg->u16AhdPart2);
            isp_demosaic_cx_var_max_rate_write(pstBeReg, pstStaticRegCfg->u8CxVarMaxRate);
            isp_demosaic_cx_var_min_rate_write(pstBeReg, pstStaticRegCfg->u8CxVarMinRate);
            isp_demosaic_g_clip_sft_bit_write(pstBeReg, pstStaticRegCfg->u8GClipBitSft);
            isp_demosaic_hv_ratio_write(pstBeReg, pstStaticRegCfg->u8hvColorRatio);
            isp_demosaic_hv_sel_write(pstBeReg, pstStaticRegCfg->u8hvSelection);
            isp_demosaic_cbcr_avg_thld_write(pstBeReg, pstStaticRegCfg->u16CbCrAvgThr);
            isp_nddm_dith_mask_write(pstBeReg, pstStaticRegCfg->u8DitherMask);
            isp_nddm_dith_ratio_write(pstBeReg, pstStaticRegCfg->u8DitherRatio);
            isp_nddm_gf_th_low_write(pstBeReg, pstStaticRegCfg->u16GFThLow);
            isp_nddm_gf_th_high_write(pstBeReg, pstStaticRegCfg->u16GFThHig);
            isp_demosaic_hlc_thresh_write(pstBeReg, 0x100);

            pstRegCfgInfo->stAlgRegCfg[i].stDemRegCfg.stStaticRegCfg.bResh = HI_FALSE;
        }

        if (pstDynaRegCfg->bResh)   /*dynamic*/
        {
            isp_demosaic_cc_hf_max_ratio_write(pstBeReg, pstDynaRegCfg->u8CcHFMaxRatio);
            isp_demosaic_cc_hf_min_ratio_write(pstBeReg, pstDynaRegCfg->u8CcHFMinRatio);
            isp_demosaic_lpf_f0_write(pstBeReg, pstDynaRegCfg->u8Lpff0);
            isp_demosaic_lpf_f1_write(pstBeReg, pstDynaRegCfg->u8Lpff1);
            isp_demosaic_lpf_f2_write(pstBeReg, pstDynaRegCfg->u8Lpff2);
            isp_demosaic_desat_low_write(pstBeReg, pstDynaRegCfg->u16DeSatLow);
            isp_demosaic_desat_protect_th_write(pstBeReg, pstDynaRegCfg->u16DeSatProtTh);
            isp_nddm_dith_max_write(pstBeReg, pstDynaRegCfg->u8DitherMax);
            isp_nddm_fcr_gf_gain_write(pstBeReg, pstDynaRegCfg->u8FcrGFGain);
            isp_nddm_awb_gf_gn_low_write(pstBeReg, pstDynaRegCfg->u8AwbGFGainLow);
            isp_nddm_awb_gf_gn_high_write(pstBeReg, pstDynaRegCfg->u8AwbGFGainHig);
            isp_nddm_awb_gf_gn_max_write(pstBeReg, pstDynaRegCfg->u8AwbGFGainMax);
            isp_nddm_ehc_gray_write(pstBeReg, pstDynaRegCfg->u8EhcGray);
            isp_demosaic_hf_intp_blur_th1_write(pstBeReg, pstDynaRegCfg->u16HfIntpBlurThLow);
            isp_demosaic_hf_intp_blur_th2_write(pstBeReg, pstDynaRegCfg->u16HfIntpBlurThHig);
            isp_nddm_fcr_det_low_write(pstBeReg, pstDynaRegCfg->u16FcrDetLow);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                isp_nddm_gf_lut_waddr_write(pstBeReg, 0);

                for (j = 0; j < HI_ISP_DEMOSAIC_LUT_LENGTH; j++)
                {
                    isp_nddm_gf_lut_wdata_write(pstBeReg, pstDynaRegCfg->au16GFBlurLut[j]);
                }
            }
            else
            {
                isp_nddm_gflut_write(pstBeReg, pstDynaRegCfg->au16GFBlurLut);
            }

            //isp_nddm_gf_lut_update_write(pstBeReg, pstDynaRegCfg->bUpdateGF);
            bGfLutUpdate         = pstDynaRegCfg->bUpdateGF;
            pstDynaRegCfg->bResh = bIsOfflineMode;

        }

        pstRegCfgInfo->unKey.bit1DemCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg.bNddmGfLutUpdate  = bGfLutUpdate | bIsOfflineMode;

    return HI_SUCCESS;
}

static HI_S32 ISP_FpnRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
#ifdef CONFIG_HI_VI_FPN_SUPPORT
    ISP_FPN_DYNA_CFG_S *pstDynaRegCfg;

    if (pstRegCfgInfo->unKey.bit1FpnCfg)
    {
        pstDynaRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stFpnRegCfg.stDynaRegCfg;

        isp_fpn_overflowthr_write(ViPipe, i, pstDynaRegCfg->u32IspFpnOverflowThr);
        isp_fpn_strength0_write(ViPipe, i, pstDynaRegCfg->u32IspFpnStrength[0]);

        pstRegCfgInfo->unKey.bit1FpnCfg = 0;
    }
#endif
    return HI_SUCCESS;
}

static HI_S32 ISP_LdciRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL bIsOfflineMode;
    HI_BOOL bLdciDrcLutUpdate  = HI_FALSE;
    HI_BOOL bLdciCalcLutUpdate = HI_FALSE;
    HI_U8   u8BufId;
    HI_U8   u8BlkNum           = pstRegCfgInfo->u8CfgNum;
    HI_U16  j, k;
    ISP_LDCI_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_LDCI_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    S_ISP_LUT_WSTT_TYPE   *pstBeLutSttReg  = HI_NULL;
    S_VIPROC_REGS_TYPE    *pstViProcReg    = HI_NULL;
    S_ISPBE_REGS_TYPE     *pstBeReg        = HI_NULL;
    ISP_CTX_S             *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1LdciCfg) {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);

        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stLdciRegCfg.stStaticRegCfg;
        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stLdciRegCfg.stDynaRegCfg;

        if (pstStaticRegCfg->bStaticResh) {
            /*static*/
            isp_ldci_stat_evratio_write(pstBeReg, 0x1000);
            isp_ldci_lut_width_word_write(pstViProcReg, HI_ISP_LDCI_LUT_WIDTH_WORD_DEFAULT);
            isp_ldci_luma_sel_write(pstBeReg, pstStaticRegCfg->u32CalcLumaSel);
            isp_ldci_bende_write(pstBeReg, pstStaticRegCfg->bDeEnable);
            isp_ldci_deref_write(pstBeReg, pstStaticRegCfg->bCalcDeRef);
            isp_ldci_deh_lpfsft_write(pstBeReg, pstStaticRegCfg->u32DehLpfSft);
            isp_ldci_coefh0_write(pstBeReg, pstStaticRegCfg->as16DehLpfCoefH[0]);
            isp_ldci_coefh1_write(pstBeReg, pstStaticRegCfg->as16DehLpfCoefH[1]);
            isp_ldci_coefh2_write(pstBeReg, pstStaticRegCfg->as16DehLpfCoefH[2]);
            isp_ldci_coefv0_write(pstBeReg, pstStaticRegCfg->as16DehLpfCoefV[0]);
            isp_ldci_coefv1_write(pstBeReg, pstStaticRegCfg->as16DehLpfCoefV[1]);
            isp_ldci_calc_drcen_write(pstBeReg, pstStaticRegCfg->bCalcDrcEnable);
            isp_ldci_pflmt_en_write(pstBeReg, pstStaticRegCfg->bCalcPfLmtEnable);
            isp_ldci_pfori0_write(pstBeReg, pstStaticRegCfg->u32CalcPfOri[0]);
            isp_ldci_pfori1_write(pstBeReg, pstStaticRegCfg->u32CalcPfOri[1]);
            isp_ldci_pfsft0_write(pstBeReg, pstStaticRegCfg->as16CalcPfSft[0]);
            isp_ldci_pfsft1_write(pstBeReg, pstStaticRegCfg->as16CalcPfSft[1]);
            isp_ldci_pfsft2_write(pstBeReg, pstStaticRegCfg->as16CalcPfSft[2]);
            isp_ldci_pfsft3_write(pstBeReg, pstStaticRegCfg->as16CalcPfSft[3]);
            isp_ldci_pfthd0_write(pstBeReg, pstStaticRegCfg->u32CalcPfThd[0]);
            isp_ldci_pfthd1_write(pstBeReg, pstStaticRegCfg->u32CalcPfThd[1]);
            isp_ldci_pfrly0_write(pstBeReg, pstStaticRegCfg->u32CalcPfRly[0]);
            isp_ldci_pfrly1_write(pstBeReg, pstStaticRegCfg->u32CalcPfRly[1]);
            isp_ldci_pfmul_write(pstBeReg, pstStaticRegCfg->u32CalcPfmul);
            isp_ldci_lpfsft_write(pstBeReg, pstStaticRegCfg->u32LpfSft);
            isp_ldci_stat_drcen_write(pstBeReg, pstStaticRegCfg->bStatDrcEnable);
            isp_ldci_chrposdamp_write(pstBeReg, pstStaticRegCfg->u32ChrPosDamp);
            isp_ldci_chrnegdamp_write(pstBeReg, pstStaticRegCfg->u32ChrNegDamp);
            isp_ldci_glb_hewgt_write(pstBeReg, pstStaticRegCfg->u32GlobalHeWgt);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) ||
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)) {
                isp_ldci_poply_waddr_write(pstBeReg, 0);
                isp_ldci_plyq01_waddr_write(pstBeReg, 0);
                isp_ldci_plyq23_waddr_write(pstBeReg, 0);


                for (j = 0; j < 65; j++) {
                    isp_ldci_poply_wdata_write(pstBeReg, pstStaticRegCfg->as16PolyP1[j], pstStaticRegCfg->as16StatPolyP2[j], pstStaticRegCfg->as16StatPolyP3[j]);
                    isp_ldci_plyq01_wdata_write(pstBeReg, pstStaticRegCfg->as16PolyQ0[j], pstStaticRegCfg->as16PolyQ1[j]);
                    isp_ldci_plyq23_wdata_write(pstBeReg, pstStaticRegCfg->as16PolyQ2[j], pstStaticRegCfg->as16PolyQ3[j]);
                }

                for (k = 0; k < 2; k++)  /*online Lut2stt regconfig*/
                {
                    pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, k);
                    ISP_CHECK_POINTER(pstBeLutSttReg);
                    isp_ldci_drc_cgain_lut_wstt_write(pstBeLutSttReg, pstStaticRegCfg->as16CalcDynRngCmpLut, pstStaticRegCfg->as16StatDynRngCmpLut, pstDynaRegCfg->u32ColorGainLut);
                    isp_ldci_usm_lut_wstt_write(pstBeLutSttReg, pstDynaRegCfg->u32UsmPosLut, pstDynaRegCfg->u32UsmNegLut);
                }
            } else {
                isp_ldci_poply1_wlut_write(pstBeReg, pstStaticRegCfg->as16PolyP1);
                isp_ldci_poply2_wlut_write(pstBeReg, pstStaticRegCfg->as16StatPolyP2);
                isp_ldci_poply3_wlut_write(pstBeReg, pstStaticRegCfg->as16StatPolyP3);
                isp_ldci_plyq0_wlut_write(pstBeReg, pstStaticRegCfg->as16PolyQ0);
                isp_ldci_plyq1_wlut_write(pstBeReg, pstStaticRegCfg->as16PolyQ1);
                isp_ldci_plyq2_wlut_write(pstBeReg, pstStaticRegCfg->as16PolyQ2);
                isp_ldci_plyq3_wlut_write(pstBeReg, pstStaticRegCfg->as16PolyQ3);

                isp_ldci_drc_cgain_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstStaticRegCfg->as16CalcDynRngCmpLut, pstStaticRegCfg->as16StatDynRngCmpLut, pstDynaRegCfg->u32ColorGainLut);
                isp_ldci_usm_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstDynaRegCfg->u32UsmPosLut, pstDynaRegCfg->u32UsmNegLut);
            }

            bLdciDrcLutUpdate            = HI_TRUE;
            //isp_ldci_drc_lut_update_write(pstBeReg, HI_TRUE);
            pstStaticRegCfg->bStaticResh = HI_FALSE;
        }

        /*dynamic*/
        isp_ldci_en_write(pstViProcReg, pstDynaRegCfg->bEnable);
        isp_ldci_wrstat_en_write(pstBeReg, pstDynaRegCfg->bWrstatEn);
        isp_ldci_rdstat_en_write(pstBeReg, pstDynaRegCfg->bRdstatEn);
        isp_ldci_calc_en_write(pstBeReg, pstDynaRegCfg->bCalcEnable);
        //isp_ldci_blc_ctrl_write(pstBeReg, pstDynaRegCfg->u32CalcBlcCtrl);
        isp_ldci_lpfcoef0_write(pstBeReg, pstDynaRegCfg->u32LpfCoef[0]);
        isp_ldci_lpfcoef1_write(pstBeReg, pstDynaRegCfg->u32LpfCoef[1]);
        isp_ldci_lpfcoef2_write(pstBeReg, pstDynaRegCfg->u32LpfCoef[2]);
        isp_ldci_lpfcoef3_write(pstBeReg, pstDynaRegCfg->u32LpfCoef[3]);
        isp_ldci_lpfcoef4_write(pstBeReg, pstDynaRegCfg->u32LpfCoef[4]);
        isp_ldci_lpfcoef5_write(pstBeReg, pstDynaRegCfg->u32LpfCoef[5]);
        isp_ldci_lpfcoef6_write(pstBeReg, pstDynaRegCfg->u32LpfCoef[6]);
        isp_ldci_lpfcoef7_write(pstBeReg, pstDynaRegCfg->u32LpfCoef[7]);
        isp_ldci_lpfcoef8_write(pstBeReg, pstDynaRegCfg->u32LpfCoef[8]);
        isp_ldci_calc_map_offsetx_write(pstBeReg, pstDynaRegCfg->u32CalcMapOffsetX);
        isp_ldci_smlmapstride_write(pstBeReg, pstDynaRegCfg->u32CalcSmlMapStride);
        isp_ldci_smlmapheight_write(pstBeReg, pstDynaRegCfg->u32CalcSmlMapHeight);
        isp_ldci_total_zone_write(pstBeReg, pstDynaRegCfg->u32CalcTotalZone);
        isp_ldci_scalex_write(pstBeReg, pstDynaRegCfg->u32CalcScaleX);
        isp_ldci_scaley_write(pstBeReg, pstDynaRegCfg->u32CalcScaleY);
        isp_ldci_stat_smlmapwidth_write(pstBeReg, pstDynaRegCfg->u32StatSmlMapWidth);
        isp_ldci_stat_smlmapheight_write(pstBeReg, pstDynaRegCfg->u32StatSmlMapHeight);
        isp_ldci_stat_total_zone_write(pstBeReg, pstDynaRegCfg->u32StatTotalZone);
        isp_ldci_blk_smlmapwidth0_write(pstBeReg, pstDynaRegCfg->u32BlkSmlMapWidth[0]);
        isp_ldci_blk_smlmapwidth1_write(pstBeReg, pstDynaRegCfg->u32BlkSmlMapWidth[1]);
        isp_ldci_blk_smlmapwidth2_write(pstBeReg, pstDynaRegCfg->u32BlkSmlMapWidth[2]);
        isp_ldci_hstart_write(pstBeReg, pstDynaRegCfg->u32StatHStart);
        isp_ldci_hend_write(pstBeReg, pstDynaRegCfg->u32StatHEnd);
        isp_ldci_vstart_write(pstBeReg, pstDynaRegCfg->u32StatVStart);
        isp_ldci_vend_write(pstBeReg, pstDynaRegCfg->u32StatVEnd);

        if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) ||
            IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)) {
            /* online Lut2stt regconfig */
            u8BufId = pstDynaRegCfg->u8BufId;

            pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, u8BufId);
            ISP_CHECK_POINTER(pstBeLutSttReg);

            isp_ldci_he_delut_lut_wstt_write(pstBeLutSttReg, pstDynaRegCfg->u32DeLut, pstDynaRegCfg->u32HePosLut, pstDynaRegCfg->u32HeNegLut);
            isp_ldci_lut_wstt_addr_write(ViPipe, i, u8BufId, pstViProcReg);

            pstDynaRegCfg->u8BufId = 1 - u8BufId;
        } else {
            isp_ldci_he_delut_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstDynaRegCfg->u32DeLut, pstDynaRegCfg->u32HePosLut, pstDynaRegCfg->u32HeNegLut);
        }

        isp_ldci_stt2lut_en_write(pstBeReg, HI_TRUE);
        isp_ldci_stt2lut_regnew_write(pstBeReg, HI_TRUE);

        bLdciCalcLutUpdate = pstDynaRegCfg->bCalcLutUpdate;

        pstRegCfgInfo->unKey.bit1LdciCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg.bLdciCalcLutUpdate = bLdciCalcLutUpdate | bIsOfflineMode;
    pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg.bLdciDrcLutUpdate  = bLdciDrcLutUpdate | bIsOfflineMode;

    return HI_SUCCESS;
}

static HI_S32 ISP_LcacRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bIdxResh   = HI_FALSE;
    HI_BOOL  bUsrResh   = HI_FALSE;
    HI_BOOL  bLutUpdate = HI_FALSE;
    HI_U8    j;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    ISP_LOCAL_CAC_USR_CFG_S    *pstUsrRegCfg    = HI_NULL;
    ISP_LOCAL_CAC_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    ISP_LOCAL_CAC_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    S_ISPBE_REGS_TYPE          *pstBeReg        = HI_NULL;
    ISP_CTX_S                  *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1LocalCacCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);

        //isp_demosaic_local_cac_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stLCacRegCfg.bLocalCacEn);

        /*static*/
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stLCacRegCfg.stStaticRegCfg;

        if (pstStaticRegCfg->bStaticResh)
        {
            isp_nddm_cac_blend_en_write(pstBeReg, pstStaticRegCfg->bNddmCacBlendEn);
            isp_nddm_cac_blend_rate_write(pstBeReg, pstStaticRegCfg->u16NddmCacBlendRate);
            isp_demosaic_r_counter_thr_write(pstBeReg, pstStaticRegCfg->u8RCounterThr);
            isp_demosaic_g_counter_thr_write(pstBeReg, pstStaticRegCfg->u8GCounterThr);
            isp_demosaic_b_counter_thr_write(pstBeReg, pstStaticRegCfg->u8BCounterThr);
            isp_demosaic_satu_thr_write(pstBeReg, pstStaticRegCfg->u16SatuThr);
            isp_demosaic_fake_cr_var_thr_high_write(pstBeReg, pstStaticRegCfg->u16FakeCrVarThrHigh);
            isp_demosaic_fake_cr_var_thr_low_write(pstBeReg, pstStaticRegCfg->u16FakeCrVarThrLow);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                isp_demosaic_depurplut_waddr_write(pstBeReg, 0);

                for (j = 0; j < LCAC_STRENGTH_NUM; j++)
                {
                    isp_demosaic_depurplut_wdata_write(pstBeReg, pstStaticRegCfg->au8DePurpleStr[j]);
                }

                bLutUpdate = HI_TRUE;
            }
            else
            {
                isp_demosaic_depurp_lut_write(pstBeReg, pstStaticRegCfg->au8DePurpleStr);
            }

            pstStaticRegCfg->bStaticResh = HI_FALSE;
        }

        /* Usr */
        pstUsrRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stLCacRegCfg.stUsrRegCfg;
        bIdxResh = (isp_lcac_update_index_read(pstBeReg) != pstUsrRegCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstUsrRegCfg->bResh & bIdxResh) : (pstUsrRegCfg->bResh);

        if (bUsrResh)
        {
            isp_lcac_update_index_write(pstBeReg, pstUsrRegCfg->u32UpdateIndex);

            isp_demosaic_purple_var_thr_write(pstBeReg, pstUsrRegCfg->u16VarThr);
            isp_demosaic_cb_thr_write(pstBeReg, pstUsrRegCfg->u16CbThr);
            isp_demosaic_cac_luma_high_cnt_thr_write(pstBeReg, pstUsrRegCfg->u8LumaHighCntThr);
            isp_demosaic_cac_cb_cnt_low_thr_write(pstBeReg, pstUsrRegCfg->u8CbCntLowThr);
            isp_demosaic_cac_cb_cnt_high_thr_write(pstBeReg, pstUsrRegCfg->u8CbCntHighThr);
            isp_demosaci_cac_bld_avg_cur_write(pstBeReg, pstUsrRegCfg->u8BldAvgCur);
            isp_demosaic_cbcr_ratio_high_limit_write(pstBeReg, pstUsrRegCfg->s16CbCrRatioLmtHigh);
            //isp_demosaic_cbcr_ratio_low_limit_write(pstBeReg, pstUsrRegCfg->s16CbCrRatioLmtLow);
            isp_demosaic_defcolor_cr_write(pstBeReg, pstUsrRegCfg->u8DeFColorCr);
            isp_demosaic_defcolor_cb_write(pstBeReg, pstUsrRegCfg->u8DeFColorCb);

            pstUsrRegCfg->bResh = bIsOfflineMode;
        }

        /*dynamic*/
        pstDynaRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stLCacRegCfg.stDynaRegCfg;

        if (pstDynaRegCfg->bResh)
        {
            isp_demosaic_r_luma_thr_write(pstBeReg, pstDynaRegCfg->u16RLumaThr);
            isp_demosaic_g_luma_thr_write(pstBeReg, pstDynaRegCfg->u16GLumaThr);
            isp_demosaic_b_luma_thr_write(pstBeReg, pstDynaRegCfg->u16BLumaThr);
            isp_demosaic_luma_thr_write  (pstBeReg, pstDynaRegCfg->u16LumaThr);
            isp_demosaic_cbcr_ratio_low_limit_write(pstBeReg, pstDynaRegCfg->s16CbCrRatioLmtLow);
            isp_demosaic_depurplectr_cr_write(pstBeReg, pstDynaRegCfg->u8DePurpleCtrCr);
            isp_demosaic_depurplectr_cb_write(pstBeReg, pstDynaRegCfg->u8DePurpleCtrCb);
            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1LocalCacCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg.bLCacLutUpdate = bLutUpdate | bIsOfflineMode;

    return HI_SUCCESS;
}

static HI_S32 ISP_FcrRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_U8 u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_BOOL  bIsOfflineMode;
    ISP_ANTIFALSECOLOR_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    ISP_ANTIFALSECOLOR_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    S_ISPBE_REGS_TYPE               *pstBeReg        = HI_NULL;
    ISP_CTX_S                       *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1FcrCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);

        //isp_demosaic_fcr_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stAntiFalseColorRegCfg.bFcrEnable);
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stAntiFalseColorRegCfg.stStaticRegCfg;
        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stAntiFalseColorRegCfg.stDynaRegCfg;

        /*static*/
        if (pstStaticRegCfg->bResh)
        {
            isp_demosaic_fcr_limit1_write(pstBeReg, pstStaticRegCfg->u16FcrLimit1);
            isp_demosaic_fcr_limit2_write(pstBeReg, pstStaticRegCfg->u16FcrLimit2);
            isp_demosaic_fcr_thr_write(pstBeReg, pstStaticRegCfg->u16FcrThr);
            pstStaticRegCfg->bResh = HI_FALSE;
        }

        /*dynamic*/
        if (pstDynaRegCfg->bResh)
        {
            isp_demosaic_fcr_gain_write(pstBeReg, pstDynaRegCfg->u8FcrGain);
            isp_demosaic_fcr_ratio_write(pstBeReg, pstDynaRegCfg->u8FcrRatio);
            isp_demosaic_fcr_gray_ratio_write(pstBeReg, pstDynaRegCfg->u8FcrGrayRatio);
            isp_demosaic_fcr_cmax_sel_write(pstBeReg, pstDynaRegCfg->u8FcrCmaxSel);
            isp_demosaic_fcr_detg_sel_write(pstBeReg, pstDynaRegCfg->u8FcrDetgSel);
            isp_demosaic_fcr_thresh1_write(pstBeReg, pstDynaRegCfg->u16FcrHfThreshLow);
            isp_demosaic_fcr_thresh2_write(pstBeReg, pstDynaRegCfg->u16FcrHfThreshHig);
            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1FcrCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_GcacRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
#ifdef CONFIG_HI_ISP_GCAC_SUPPORT
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bUsrResh  = HI_FALSE;
    HI_BOOL  bIdxResh  = HI_FALSE;
    HI_U8 u8BlkNum = pstRegCfgInfo->u8CfgNum;
    S_ISPBE_REGS_TYPE           *pstBeReg        = HI_NULL;
    ISP_GLOBAL_CAC_USR_CFG_S    *pstUsrRegCfg    = HI_NULL;
    ISP_GLOBAL_CAC_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_CTX_S                   *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)\
                      || IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1GlobalCacCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);

        //isp_gcac_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stGCacRegCfg.bGlobalCacEn);

        /*static*/
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stGCacRegCfg.stStaticRegCfg;

        if (pstStaticRegCfg->bStaticResh)
        {
            isp_gcac_ver_filt_en_write(pstBeReg, pstStaticRegCfg->bGCacVerFilEn);
            isp_demosaic_gcac_blend_select_write(pstBeReg, pstStaticRegCfg->u8GCacBlendSel);
            isp_gcac_chrv_mode_write(pstBeReg, pstStaticRegCfg->u8GcacChrVerMode);
            isp_gcac_clip_mode_v_write(pstBeReg, pstStaticRegCfg->u8GcacClipModeVer);
            isp_gcac_clip_mode_h_write(pstBeReg, pstStaticRegCfg->u8GcacClipModeHor);
            pstStaticRegCfg->bStaticResh = HI_FALSE;
        }

        /*usr*/
        pstUsrRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stGCacRegCfg.stUsrRegCfg;
        bIdxResh = (isp_gcac_update_index_read(pstBeReg) != pstUsrRegCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstUsrRegCfg->bResh & bIdxResh) : (pstUsrRegCfg->bResh);

        if (bUsrResh)
        {
            isp_gcac_update_index_write(pstBeReg, pstUsrRegCfg->u32UpdateIndex);
            isp_gcac_red_a_write(pstBeReg, pstUsrRegCfg->s16ParamRedA);
            isp_gcac_red_b_write(pstBeReg, pstUsrRegCfg->s16ParamRedB);
            isp_gcac_red_c_write(pstBeReg, pstUsrRegCfg->s16ParamRedC);
            isp_gcac_blue_a_write(pstBeReg, pstUsrRegCfg->s16ParamBlueA);
            isp_gcac_blue_b_write(pstBeReg, pstUsrRegCfg->s16ParamBlueB);
            isp_gcac_blue_c_write(pstBeReg, pstUsrRegCfg->s16ParamBlueC);
            isp_gcac_ns_y_write(pstBeReg, pstUsrRegCfg->u8VerNormShift);
            isp_gcac_nf_y_write(pstBeReg, pstUsrRegCfg->u8VerNormFactor);
            isp_gcac_ns_x_write(pstBeReg, pstUsrRegCfg->u8HorNormShift);
            isp_gcac_nf_x_write(pstBeReg, pstUsrRegCfg->u8HorNormFactor);
            isp_gcac_cnt_start_h_write(pstBeReg, pstUsrRegCfg->u16CenterCoorHor);
            isp_gcac_cnt_start_v_write(pstBeReg, pstUsrRegCfg->u16CenterCoorVer);
            isp_demosaic_varthr_for_blend_write(pstBeReg, pstUsrRegCfg->u16CorVarThr);
            isp_gcac_cor_start_h_write(pstBeReg, pstUsrRegCfg->u16StartCoorHor);
            isp_gcac_cor_start_v_write(pstBeReg, pstUsrRegCfg->u16StartCoorVer);
            pstUsrRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1GlobalCacCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }
#endif
    return HI_SUCCESS;
}


static HI_S32 ISP_DpcRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bUsrResh   = HI_FALSE;
    HI_BOOL  bIdxResh   = HI_FALSE;
    HI_BOOL  bStt2LutRegnew = HI_FALSE;
    HI_BOOL  bIsOfflineMode;
    HI_U8    u8BufId;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    ISP_DPC_STATIC_CFG_S    *pstStaticRegCfg = HI_NULL;
    ISP_DPC_DYNA_CFG_S      *pstDynaRegCfg   = HI_NULL;
    ISP_DPC_USR_CFG_S       *pstUsrRegCfg    = HI_NULL;
    S_ISP_LUT_WSTT_TYPE     *pstBeLutSttReg  = HI_NULL;
    S_ISPBE_REGS_TYPE       *pstBeReg        = HI_NULL;
    S_VIPROC_REGS_TYPE      *pstViProcReg    = HI_NULL;
    ISP_CTX_S               *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1DpCfg)
    {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);

        //isp_dpc_en_write(pstViProcReg, pstRegCfgInfo->stAlgRegCfg[i].stDpRegCfg.abDpcEn[0]);
        //isp_dpc_dpc_en1_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stDpRegCfg.abDpcEn[1]);

        /*static*/
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stDpRegCfg.stStaticRegCfg;

        if (pstStaticRegCfg->bStaticResh)
        {
            isp_dpc_output_mode_write(pstBeReg, pstStaticRegCfg->u8DpccOutputMode);
            isp_dpc_bpt_ctrl_write(pstBeReg, pstStaticRegCfg->u32DpccBptCtrl);
            isp_dcg_lut_width_word_write(pstViProcReg, HI_ISP_DPC_LUT_WIDTH_WORD_DEFAULT);
            pstStaticRegCfg->bStaticResh = HI_FALSE;
        }

        /*usr*/
        pstUsrRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stDpRegCfg.stUsrRegCfg;

        if (pstUsrRegCfg->stUsrDynaCorRegCfg.bResh)
        {
            isp_dpc_ex_soft_thr_max_write(pstBeReg, pstUsrRegCfg->stUsrDynaCorRegCfg.s8DpccSupTwinkleThrMax);
            isp_dpc_ex_soft_thr_min_write(pstBeReg, pstUsrRegCfg->stUsrDynaCorRegCfg.s8DpccSupTwinkleThrMin);
            isp_dpc_ex_hard_thr_en_write(pstBeReg, pstUsrRegCfg->stUsrDynaCorRegCfg.bDpccHardThrEn);
            isp_dpc_ex_rake_ratio_write(pstBeReg, pstUsrRegCfg->stUsrDynaCorRegCfg.u16DpccRakeRatio);
            pstUsrRegCfg->stUsrDynaCorRegCfg.bResh = bIsOfflineMode;
        }

        bIdxResh = (isp_dpc_update_index_read(pstBeReg) != pstUsrRegCfg->stUsrStaCorRegCfg.u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstUsrRegCfg->stUsrStaCorRegCfg.bResh & bIdxResh) : (pstUsrRegCfg->stUsrStaCorRegCfg.bResh);

        if (bUsrResh)
        {
            isp_dpc_update_index_write(pstBeReg, pstUsrRegCfg->stUsrStaCorRegCfg.u32UpdateIndex);
            isp_dpc_bpt_number_write(pstBeReg, pstUsrRegCfg->stUsrStaCorRegCfg.u16DpccBptNumber);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)\
                || IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                /*online Lut2stt regconfig*/
                u8BufId = pstUsrRegCfg->stUsrStaCorRegCfg.u8BufId;

                pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, u8BufId);
                ISP_CHECK_POINTER(pstBeLutSttReg);

                isp_dpc_lut_wstt_write(pstBeLutSttReg, pstUsrRegCfg->stUsrStaCorRegCfg.au32DpccBpTable);

                isp_dpc_lut_wstt_addr_write(ViPipe, i, u8BufId, pstViProcReg);

                isp_dpc_stt2lut_en_write(pstBeReg, HI_TRUE);
                //isp_dpc_stt2lut_regnew_write(pstBeReg, HI_TRUE);

                pstUsrRegCfg->stUsrStaCorRegCfg.u8BufId = 1 - u8BufId;

                bStt2LutRegnew = HI_TRUE;
            }
            else
            {
                isp_dpc_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstUsrRegCfg->stUsrStaCorRegCfg.au32DpccBpTable);
                isp_dpc_stt2lut_en_write(pstBeReg, HI_TRUE);
                isp_dpc_stt2lut_regnew_write(pstBeReg, HI_TRUE);
            }

            pstUsrRegCfg->stUsrStaCorRegCfg.bResh = bIsOfflineMode;
        }

        /*dynamic*/
        pstDynaRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stDpRegCfg.stDynaRegCfg;

        if (pstDynaRegCfg->bResh)
        {
            isp_dpc_stat_en_write(pstViProcReg, pstDynaRegCfg->bDpcStatEn);
            isp_dpc_blend_write(pstBeReg, pstDynaRegCfg->u32DpccAlpha);
            isp_dpc_mode_write(pstBeReg, pstDynaRegCfg->u16DpccMode);
            isp_dpc_set_use_write(pstBeReg, pstDynaRegCfg->u8DpccSetUse);
            isp_dpc_methods_set_1_write(pstBeReg, pstDynaRegCfg->u16DpccMethodsSet1);
            isp_dpc_methods_set_2_write(pstBeReg, pstDynaRegCfg->u16DpccMethodsSet2);
            isp_dpc_methods_set_3_write(pstBeReg, pstDynaRegCfg->u16DpccMethodsSet3);
            isp_dpc_line_thresh_1_write(pstBeReg, pstDynaRegCfg->au16DpccLineThr[0]);
            isp_dpc_line_mad_fac_1_write(pstBeReg, pstDynaRegCfg->au16DpccLineMadFac[0]);
            isp_dpc_pg_fac_1_write(pstBeReg, pstDynaRegCfg->au16DpccPgFac[0]);
            isp_dpc_rnd_thresh_1_write(pstBeReg, pstDynaRegCfg->au16DpccRndThr[0]);
            isp_dpc_rg_fac_1_write(pstBeReg, pstDynaRegCfg->au16DpccRgFac[0]);
            isp_dpc_line_thresh_2_write(pstBeReg, pstDynaRegCfg->au16DpccLineThr[1]);
            isp_dpc_line_mad_fac_2_write(pstBeReg, pstDynaRegCfg->au16DpccLineMadFac[1]);
            isp_dpc_pg_fac_2_write(pstBeReg, pstDynaRegCfg->au16DpccPgFac[1]);
            isp_dpc_rnd_thresh_2_write(pstBeReg, pstDynaRegCfg->au16DpccRndThr[1]);
            isp_dpc_rg_fac_2_write(pstBeReg, pstDynaRegCfg->au16DpccRgFac[1]);
            isp_dpc_line_thresh_3_write(pstBeReg, pstDynaRegCfg->au16DpccLineThr[2]);
            isp_dpc_line_mad_fac_3_write(pstBeReg, pstDynaRegCfg->au16DpccLineMadFac[2]);
            isp_dpc_pg_fac_3_write(pstBeReg, pstDynaRegCfg->au16DpccPgFac[2]);
            isp_dpc_rnd_thresh_3_write(pstBeReg, pstDynaRegCfg->au16DpccRndThr[2]);
            isp_dpc_rg_fac_3_write(pstBeReg, pstDynaRegCfg->au16DpccRgFac[2]);
            isp_dpc_ro_limits_write(pstBeReg, pstDynaRegCfg->u16DpccRoLimits);
            isp_dpc_rnd_offs_write(pstBeReg, pstDynaRegCfg->u16DpccRndOffs);
            isp_dpc_bpt_thresh_write(pstBeReg, pstDynaRegCfg->u32DpccBadThresh);

            isp_dpc_line_std_thr_1_write(pstBeReg, pstDynaRegCfg->au8DpccLineStdThr[0]);
            isp_dpc_line_std_thr_2_write(pstBeReg, pstDynaRegCfg->au8DpccLineStdThr[1]);
            isp_dpc_line_std_thr_3_write(pstBeReg, pstDynaRegCfg->au8DpccLineStdThr[2]);
            isp_dpc_line_std_thr_4_write(pstBeReg, pstDynaRegCfg->au8DpccLineStdThr[3]);
            isp_dpc_line_std_thr_5_write(pstBeReg, pstDynaRegCfg->au8DpccLineStdThr[4]);


            isp_dpc_line_diff_thr_1_write(pstBeReg, pstDynaRegCfg->au8DpccLineDiffThr[0]);
            isp_dpc_line_diff_thr_2_write(pstBeReg, pstDynaRegCfg->au8DpccLineDiffThr[1]);
            isp_dpc_line_diff_thr_3_write(pstBeReg, pstDynaRegCfg->au8DpccLineDiffThr[2]);
            isp_dpc_line_diff_thr_4_write(pstBeReg, pstDynaRegCfg->au8DpccLineDiffThr[3]);
            isp_dpc_line_diff_thr_5_write(pstBeReg, pstDynaRegCfg->au8DpccLineDiffThr[4]);

            isp_dpc_line_aver_fac_1_write(pstBeReg, pstDynaRegCfg->au8DpccLineAverFac[0]);
            isp_dpc_line_aver_fac_2_write(pstBeReg, pstDynaRegCfg->au8DpccLineAverFac[1]);
            isp_dpc_line_aver_fac_3_write(pstBeReg, pstDynaRegCfg->au8DpccLineAverFac[2]);
            isp_dpc_line_aver_fac_4_write(pstBeReg, pstDynaRegCfg->au8DpccLineAverFac[3]);
            isp_dpc_line_aver_fac_5_write(pstBeReg, pstDynaRegCfg->au8DpccLineAverFac[4]);

            isp_dpc_line_kerdiff_fac_write(pstBeReg, pstDynaRegCfg->u8DpccLineKerdiffFac);
            isp_dpc_blend_mode_write(pstBeReg, pstDynaRegCfg->u8DpccBlendMode);
            isp_dpc_bit_depth_sel_write(pstBeReg, pstDynaRegCfg->u8DpccBitDepthSel);

            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1DpCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stStt2LutRegnewCfg.bDpcStt2LutRegnew = bStt2LutRegnew;

    return HI_SUCCESS;
}

static HI_S32 ISP_GeRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
#ifdef CONFIG_HI_ISP_CR_SUPPORT
    HI_U16 j;
    HI_U8 u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_BOOL  bIsOfflineMode;
    ISP_GE_STATIC_CFG_S     *pstStaticRegCfg = HI_NULL;
    ISP_GE_DYNA_CFG_S       *pstDynaRegCfg   = HI_NULL;
    ISP_GE_USR_CFG_S        *pstUsrRegCfg    = HI_NULL;
    S_ISPBE_REGS_TYPE       *pstBeReg        = HI_NULL;
    ISP_CTX_S               *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)\
                      || IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1GeCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);

        //isp_ge_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stGeRegCfg.abGeEn[0]);
        //isp_ge_ge1_en_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stGeRegCfg.abGeEn[1]);

        /*static*/
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stGeRegCfg.stStaticRegCfg;

        if (pstStaticRegCfg->bStaticResh)
        {
            isp_ge_vnum_write(pstBeReg, pstStaticRegCfg->u8GeNumV);
            isp_ge_gr_en_write(pstBeReg, pstStaticRegCfg->bGeGrEn);
            isp_ge_gb_en_write(pstBeReg, pstStaticRegCfg->bGeGbEn);
            isp_ge_gr_gb_en_write(pstBeReg, pstStaticRegCfg->bGeGrGbEn);
            isp_ge_bit_depth_sel_write(pstBeReg, HI_ISP_GE_BIT_DEPTH_DEFAULT);
            pstStaticRegCfg->bStaticResh = HI_FALSE;
        }

        /*usr*/
        pstUsrRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stGeRegCfg.stUsrRegCfg;

        if (pstUsrRegCfg->bResh)
        {
            for (j = 0; j < pstRegCfgInfo->stAlgRegCfg[i].stGeRegCfg.u8ChnNum; j++)
            {
                isp_ge_ct_th2_write(pstBeReg, j, pstUsrRegCfg->au16GeCtTh2[j]);
                isp_ge_ct_slope1_write(pstBeReg, j, pstUsrRegCfg->au8GeCtSlope1[j]);
                isp_ge_ct_slope2_write(pstBeReg, j, pstUsrRegCfg->au8GeCtSlope2[j]);
            }

            isp_ge_hnum_write(pstBeReg, pstUsrRegCfg->u8GeNumH);
            isp_ge_crop_pos_x_write(pstBeReg, pstUsrRegCfg->u16GeCropPosX);
            isp_ge_crop_pos_y_write(pstBeReg, pstUsrRegCfg->u16GeCropPosY);
            isp_ge_crop_out_width_write(pstBeReg, pstUsrRegCfg->u16GeCropOutWidth - 1);
            isp_ge_crop_out_height_write(pstBeReg, pstUsrRegCfg->u16GeCropOutHeight - 1);
            pstUsrRegCfg->bResh = bIsOfflineMode;
        }

        /*dynamic*/
        pstDynaRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stGeRegCfg.stDynaRegCfg;

        if (pstDynaRegCfg->bResh)
        {
            for (j = 0; j < pstRegCfgInfo->stAlgRegCfg[i].stGeRegCfg.u8ChnNum; j++)
            {
                isp_ge_ct_th1_write(pstBeReg, j, pstDynaRegCfg->au16GeCtTh1[j]);
                isp_ge_ct_th3_write(pstBeReg, j, pstDynaRegCfg->au16GeCtTh3[j]);
            }

            isp_ge_strength_write(pstBeReg, pstDynaRegCfg->u16GeStrength);
            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1GeCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }
#endif

    return HI_SUCCESS;
}

static HI_S32 ISP_LscRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bUsrResh   = HI_FALSE;
    HI_BOOL  bIdxResh   = HI_FALSE;
    HI_BOOL  bLutUpdate = HI_FALSE;
    HI_BOOL  bStt2LutRegnew = HI_FALSE;
    HI_U8    u8BufId;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_U16   j;
    ISP_LSC_USR_CFG_S       *pstUsrRegCfg    = HI_NULL;
    ISP_LSC_STATIC_CFG_S    *pstStaticRegCfg = HI_NULL;
    S_ISP_LUT_WSTT_TYPE     *pstBeLutSttReg  = HI_NULL;
    S_VIPROC_REGS_TYPE      *pstViProcReg    = HI_NULL;
    S_ISPBE_REGS_TYPE       *pstBeReg        = HI_NULL;
    ISP_CTX_S               *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1LscCfg)
    {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);
        /*static*/
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stLscRegCfg.stStaticRegCfg;
        pstUsrRegCfg    = &pstRegCfgInfo->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg;

        //isp_lsc_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stLscRegCfg.bLscEn);
        if (pstStaticRegCfg->bStaticResh)
        {
            isp_lsc_numh_write(pstBeReg, pstStaticRegCfg->u8WinNumH);
            isp_lsc_numv_write(pstBeReg, pstStaticRegCfg->u8WinNumV);
            isp_lsc_lut_width_word_write(pstViProcReg, HI_ISP_LSC_LUT_WIDTH_WORD_DEFAULT);
            pstStaticRegCfg->bStaticResh = HI_FALSE;
        }

        /*usr*/
        if (pstUsrRegCfg->bCoefUpdate)
        {
            isp_lsc_mesh_str_write(pstBeReg, pstUsrRegCfg->u16MeshStr);
            isp_lsc_mesh_weight_write(pstBeReg, pstUsrRegCfg->u16Weight);
            pstUsrRegCfg->bCoefUpdate = bIsOfflineMode;
        }

        bIdxResh = (isp_lsc_update_index_read(pstBeReg) != pstUsrRegCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstUsrRegCfg->bLutUpdate & bIdxResh) : (pstUsrRegCfg->bLutUpdate);

        if (bUsrResh)
        {
            isp_lsc_update_index_write(pstBeReg, pstUsrRegCfg->u32UpdateIndex);
            //isp_lsc_mesh_scale_write(pstBeReg, pstUsrRegCfg->u8MeshScale);
            isp_lsc_width_offset_write(pstBeReg, pstUsrRegCfg->u16WidthOffset);

            for (j = 0; j < (HI_ISP_LSC_GRID_ROW - 1) / 2; j++)
            {
                isp_lsc_winY_info_write(pstBeReg, j, pstUsrRegCfg->au16DeltaY[j], pstUsrRegCfg->au16InvY[j]);
            }

            for (j = 0; j < (HI_ISP_LSC_GRID_COL - 1); j++)
            {
                isp_lsc_winX_info_write(pstBeReg, j, pstUsrRegCfg->au16DeltaX[j], pstUsrRegCfg->au16InvX[j]);
            }

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                if (HI_TRUE == pstRegCfgInfo->stAlgRegCfg[i].stLscRegCfg.bLut2SttEn) /*online Lut2stt regconfig*/
                {
                    u8BufId = pstUsrRegCfg->u8BufId;

                    pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, u8BufId);
                    ISP_CHECK_NULLPTR(pstBeLutSttReg);

                    isp_lsc_lut_wstt_write(pstBeLutSttReg, pstUsrRegCfg->au32RGain, pstUsrRegCfg->au32GrGain, pstUsrRegCfg->au32GbGain, pstUsrRegCfg->au32BGain);
                    isp_lsc_lut_wstt_addr_write(ViPipe, i, u8BufId, pstViProcReg);
                    isp_lsc_stt2lut_en_write(pstBeReg, HI_TRUE);
                    //isp_lsc_stt2lut_regnew_write(pstBeReg, HI_TRUE);

                    pstUsrRegCfg->u8BufId = 1 - u8BufId;
                    bStt2LutRegnew = HI_TRUE;
                }
            }
            else
            {
                isp_lsc_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstUsrRegCfg->au32RGain, pstUsrRegCfg->au32GrGain, pstUsrRegCfg->au32GbGain, pstUsrRegCfg->au32BGain);
                isp_lsc_stt2lut_en_write(pstBeReg, HI_TRUE);
                isp_lsc_stt2lut_regnew_write(pstBeReg, HI_TRUE);
            }

            bLutUpdate = HI_TRUE;

            pstUsrRegCfg->bLutUpdate = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1LscCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg.bLscLutUpdate       = bLutUpdate;
    pstRegCfgInfo->stAlgRegCfg[i].stStt2LutRegnewCfg.bLscStt2LutRegnew = bStt2LutRegnew;

    if (HI_TRUE == pstRegCfgInfo->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg.bSwitchLut2SttRegNew)
    {
        if (pstRegCfgInfo->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg.u8SwitchRegNewCnt < 3)
        {
            pstRegCfgInfo->stAlgRegCfg[i].stStt2LutRegnewCfg.bLscStt2LutRegnew = HI_TRUE;
            pstRegCfgInfo->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg.u8SwitchRegNewCnt++;
        }
        else
        {
            pstRegCfgInfo->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg.bSwitchLut2SttRegNew = HI_FALSE;
            pstRegCfgInfo->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg.u8SwitchRegNewCnt    = 0;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_GammaRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bUsrResh   = HI_FALSE;
    HI_BOOL  bIdxResh   = HI_FALSE;
    HI_BOOL  bStt2LutRegnew = HI_FALSE;
    HI_U8    u8BufId;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    S_ISPBE_REGS_TYPE      *pstBeReg        = HI_NULL;
    S_VIPROC_REGS_TYPE     *pstViProcReg    = HI_NULL;
    S_ISP_LUT_WSTT_TYPE    *pstBeLutSttReg  = HI_NULL;
    ISP_GAMMA_USR_CFG_S    *pstUsrRegCfg    = HI_NULL;
    ISP_CTX_S              *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    pstUsrRegCfg    = &pstRegCfgInfo->stAlgRegCfg[i].stGammaCfg.stUsrRegCfg;

    if (pstRegCfgInfo->unKey.bit1GammaCfg)
    {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);
        //isp_gamma_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stGammaCfg.bGammaEn);

        bIdxResh = (isp_gamma_update_index_read(pstBeReg) != pstUsrRegCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstUsrRegCfg->bGammaLutUpdateEn & bIdxResh) : (pstUsrRegCfg->bGammaLutUpdateEn);

        if (bUsrResh)
        {
            isp_gamma_update_index_write(pstBeReg, pstUsrRegCfg->u32UpdateIndex);
            isp_gamma_lut_width_word_write(pstViProcReg, HI_ISP_GAMMA_LUT_WIDTH_WORD_DEFAULT);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                if (HI_TRUE != pstUsrRegCfg->bSwitchMode)
                {
                    u8BufId = pstUsrRegCfg->u8BufId;

                    pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, u8BufId);
                    ISP_CHECK_POINTER(pstBeLutSttReg);

                    isp_gamma_lut_wstt_write(pstBeLutSttReg, pstUsrRegCfg->au16GammaLUT);
                    isp_gamma_lut_wstt_addr_write(ViPipe, i, u8BufId, pstViProcReg);
                    isp_gamma_stt2lut_en_write(pstBeReg, HI_TRUE);
                    //isp_gamma_stt2lut_regnew_write(pstBeReg, HI_TRUE);
                    pstUsrRegCfg->u8BufId = 1 - u8BufId;
                    bStt2LutRegnew = HI_TRUE;
                }
            }
            else
            {
                isp_gamma_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstUsrRegCfg->au16GammaLUT);
                isp_gamma_stt2lut_en_write(pstBeReg, HI_TRUE);
                isp_gamma_stt2lut_regnew_write(pstBeReg, HI_TRUE);
            }

            isp_gamma_inseg_write(pstBeReg, pstUsrRegCfg->au16GammaInSeg);/*inseg*/
            isp_gamma_pos_write(pstBeReg,   pstUsrRegCfg->au16GammaPos);  /* pos*/
            isp_gamma_step_write(pstBeReg,  pstUsrRegCfg->au8GammaStep);  /* step*/

            pstUsrRegCfg->bGammaLutUpdateEn = bIsOfflineMode;
            pstUsrRegCfg->bSwitchMode       = HI_FALSE;
        }

        pstRegCfgInfo->unKey.bit1GammaCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stStt2LutRegnewCfg.bGammaStt2LutRegnew = bStt2LutRegnew;

    return HI_SUCCESS;
}
static HI_S32 ISP_CscRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_U8 u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_BOOL  bIsOfflineMode;
    ISP_CSC_DYNA_CFG_S    *pstDynaRegCfg   = HI_NULL;
    S_ISPBE_REGS_TYPE     *pstBeReg        = HI_NULL;
    ISP_CTX_S             *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if ( pstRegCfgInfo->unKey.bit1CscCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);

        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stCscCfg.stDynaRegCfg;

        /*General*/
        //isp_csc_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stCscCfg.bEnable);

        /*Dynamic*/
        if (pstDynaRegCfg->bResh)
        {
            isp_csc_coef_write(pstBeReg, pstDynaRegCfg->as16CscCoef);
            isp_csc_in_dc_write(pstBeReg, pstDynaRegCfg->as16CscInDC); //10bit, left shift 2 bits
            isp_csc_out_dc_write(pstBeReg, pstDynaRegCfg->as16CscOutDC); //10bit, left shift 2 bits
            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1CscCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_CaRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
#ifdef CONFIG_HI_ISP_CA_SUPPORT
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bUsrResh   = HI_FALSE;
    HI_BOOL  bIdxResh   = HI_FALSE;
    HI_BOOL  bStt2LutRegnew = HI_FALSE;
    HI_U8    u8BufId;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    ISP_CA_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_CA_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    ISP_CA_USR_CFG_S    *pstUsrRegCfg    = HI_NULL;
    S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg  = HI_NULL;
    S_VIPROC_REGS_TYPE  *pstViProcReg    = HI_NULL;
    S_ISPBE_REGS_TYPE   *pstBeReg        = HI_NULL;
    ISP_CTX_S           *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1CaCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);
        //isp_ca_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stCaRegCfg.bCaEn);

        /*usr*/
        pstUsrRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stCaRegCfg.stUsrRegCfg;
        bIdxResh = (isp_ca_update_index_read(pstBeReg) != pstUsrRegCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstUsrRegCfg->bResh & bIdxResh) : (pstUsrRegCfg->bResh);

        if (bUsrResh)
        {
            isp_ca_update_index_write(pstBeReg, pstUsrRegCfg->u32UpdateIndex);
            /*isp_ca_cp_en_write(pstBeReg, pstUsrRegCfg->bCaCpEn);*/
            isp_ca_lumath_high_write(pstBeReg, pstUsrRegCfg->u16CaLumaThrHigh);
            isp_ca_lumaratio_high_write(pstBeReg, pstUsrRegCfg->u16CaLumaRatioHigh);

            if (pstUsrRegCfg->bCaLutUpdateEn)
            {
                if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                    IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
                {
                    u8BufId = pstUsrRegCfg->u8BufId;

                    pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, u8BufId);
                    ISP_CHECK_POINTER(pstBeLutSttReg);

                    isp_ca_lut_wstt_write(pstBeLutSttReg, pstUsrRegCfg->au16YRatioLUT);
                    isp_ca_lut_wstt_addr_write(ViPipe, i, u8BufId, pstViProcReg);
                    isp_ca_stt2lut_en_write(pstBeReg, HI_TRUE);
                    //isp_ca_stt2lut_regnew_write(pstBeReg, HI_TRUE);

                    pstUsrRegCfg->u8BufId = 1 - u8BufId;

                    bStt2LutRegnew = HI_TRUE;
                }
                else
                {
                    isp_ca_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstUsrRegCfg->au16YRatioLUT);
                    isp_ca_stt2lut_en_write(pstBeReg, HI_TRUE);
                    isp_ca_stt2lut_regnew_write(pstBeReg, HI_TRUE);
                }
            }
            pstUsrRegCfg->bResh = bIsOfflineMode;
        }

        /*dynamic*/
        pstDynaRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stCaRegCfg.stDynaRegCfg;

        if (pstDynaRegCfg->bResh)
        {
            isp_ca_isoratio_write(pstBeReg, pstDynaRegCfg->u16CaISORatio);
            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        /*static*/
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stCaRegCfg.stStaticRegCfg;

        if (pstStaticRegCfg->bStaticResh)
        {
            isp_ca_llhcproc_en_write(pstBeReg, pstStaticRegCfg->bCaLlhcProcEn);
            isp_ca_skinproc_en_write(pstBeReg, pstStaticRegCfg->bCaSkinProcEn);
            isp_ca_satadj_en_write(pstBeReg, pstStaticRegCfg->bCaSatuAdjEn);
            isp_ca_lumath_low_write(pstBeReg, pstStaticRegCfg->u16CaLumaThrLow);
            isp_ca_darkchromath_low_write(pstBeReg, pstStaticRegCfg->u16CaDarkChromaThrLow);
            isp_ca_darkchromath_high_write(pstBeReg, pstStaticRegCfg->u16CaDarkChromaThrHigh);
            isp_ca_sdarkchromath_low_write(pstBeReg, pstStaticRegCfg->u16CaSDarkChromaThrLow);
            isp_ca_sdarkchromath_high_write(pstBeReg, pstStaticRegCfg->u16CaSDarkChromaThrHigh);
            isp_ca_lumaratio_low_write(pstBeReg, pstStaticRegCfg->u16CaLumaRatioLow);
            isp_ca_yuv2rgb_coef_write(pstBeReg, pstStaticRegCfg->as16CaYuv2RgbCoef);
            isp_ca_yuv2rgb_indc_write(pstBeReg, pstStaticRegCfg->as16CaYuv2RgbInDc);
            isp_ca_yuv2rgb_outdc_write(pstBeReg, pstStaticRegCfg->as16CaYuv2RgbOutDc);
            isp_ca_skinlluma_umin_write(pstBeReg, pstStaticRegCfg->u16CaSkinLowLumaMinU);
            isp_ca_skinlluma_umax_write(pstBeReg, pstStaticRegCfg->u16CaSkinLowLumaMaxU);
            isp_ca_skinlluma_uymin_write(pstBeReg, pstStaticRegCfg->u16CaSkinLowLumaMinUy);
            isp_ca_skinlluma_uymax_write(pstBeReg, pstStaticRegCfg->u16CaSkinLowLumaMaxUy);
            isp_ca_skinhluma_umin_write(pstBeReg, pstStaticRegCfg->u16CaSkinHighLumaMinU);
            isp_ca_skinhluma_umax_write(pstBeReg, pstStaticRegCfg->u16CaSkinHighLumaMaxU);
            isp_ca_skinhluma_uymin_write(pstBeReg, pstStaticRegCfg->u16CaSkinHighLumaMinUy);
            isp_ca_skinhluma_uymax_write(pstBeReg, pstStaticRegCfg->u16CaSkinHighLumaMaxUy);
            isp_ca_skinlluma_vmin_write(pstBeReg, pstStaticRegCfg->u16CaSkinLowLumaMinV);
            isp_ca_skinlluma_vmax_write(pstBeReg, pstStaticRegCfg->u16CaSkinLowLumaMaxV);
            isp_ca_skinlluma_vymin_write(pstBeReg, pstStaticRegCfg->u16CaSkinLowLumaMinVy);
            isp_ca_skinlluma_vymax_write(pstBeReg, pstStaticRegCfg->u16CaSkinLowLumaMaxVy);
            isp_ca_skinhluma_vmin_write(pstBeReg, pstStaticRegCfg->u16CaSkinHighLumaMinV);
            isp_ca_skinhluma_vmax_write(pstBeReg, pstStaticRegCfg->u16CaSkinHighLumaMaxV);
            isp_ca_skinhluma_vymin_write(pstBeReg, pstStaticRegCfg->u16CaSkinHighLumaMinVy);
            isp_ca_skinhluma_vymax_write(pstBeReg, pstStaticRegCfg->u16CaSkinHighLumaMaxVy);
            isp_ca_skin_uvdiff_write(pstBeReg, pstStaticRegCfg->s16CaSkinUvDiff);
            isp_ca_skinratioth_low_write(pstBeReg, pstStaticRegCfg->u16CaSkinRatioThrLow);
            isp_ca_skinratioth_mid_write(pstBeReg, pstStaticRegCfg->u16CaSkinRatioThrMid);
            isp_ca_skinratioth_high_write(pstBeReg, pstStaticRegCfg->u16CaSkinRatioThrHigh);

            isp_ca_lut_width_word_write(pstViProcReg, HI_ISP_CA_LUT_WIDTH_WORD_DEFAULT);

            pstStaticRegCfg->bStaticResh = HI_FALSE;
        }

        pstRegCfgInfo->unKey.bit1CaCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stStt2LutRegnewCfg.bCaStt2LutRegnew = bStt2LutRegnew;
#endif

    return HI_SUCCESS;
}

static HI_S32 ISP_McdsRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_U8 u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_BOOL  bIsOfflineMode;
    ISP_MCDS_DYNA_REG_CFG_S    *pstDynaRegCfg   = HI_NULL;
    ISP_MCDS_STATIC_REG_CFG_S  *pstStaticRegCfg = HI_NULL;
    S_VIPROC_REGS_TYPE         *pstViProcReg    = HI_NULL;
    S_ISPBE_REGS_TYPE          *pstBeReg        = HI_NULL;
    ISP_CTX_S                  *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1McdsCfg)
    {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);

        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stMcdsRegCfg.stDynaRegCfg;
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stMcdsRegCfg.stStaticRegCfg;

        if (pstStaticRegCfg->bStaticResh)
        {
            isp_hcds_en_write(pstViProcReg, pstStaticRegCfg->bHcdsEn);
            isp_mcds_coefh_write(pstBeReg, pstStaticRegCfg->as16HCoef);
            isp_de_vcds_coefv_write(pstBeReg, pstStaticRegCfg->as8VCoef);
            isp_de_vcds_limit_write(pstBeReg, pstStaticRegCfg->u16CoringLimit);
            isp_mcds_midf_bldr_write(pstBeReg, pstStaticRegCfg->u8MidfBldr);
            pstStaticRegCfg->bStaticResh = 0;
        }

        if (pstDynaRegCfg->bDynaResh)
        {
            //isp_vcds_en_write(pstViProcReg, pstDynaRegCfg->bVcdsEn);
            isp_mcds_midf_en_write(pstBeReg, pstDynaRegCfg->bMidfEn);
            pstDynaRegCfg->bDynaResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1McdsCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_WdrRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bUsrResh   = HI_FALSE;
    HI_BOOL  bIdxResh   = HI_FALSE;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_U16   u16Offset0;
    HI_U32   u32BlcComp0;
    ISP_FSWDR_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_FSWDR_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    ISP_FSWDR_USR_CFG_S    *pstUsrRegCfg    = HI_NULL;
    S_VIPROC_REGS_TYPE     *pstViProcReg    = HI_NULL;
    S_ISPBE_REGS_TYPE      *pstBeReg        = HI_NULL;
    ISP_CTX_S              *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) ||
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1FsWdrCfg) {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);

        isp_wdr_en_write(pstViProcReg, pstRegCfgInfo->stAlgRegCfg[i].stWdrRegCfg.bWDREn);
        isp_bnr_wdr_enable_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stWdrRegCfg.bWDREn);

        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stWdrRegCfg.stStaticRegCfg;
        pstUsrRegCfg    = &pstRegCfgInfo->stAlgRegCfg[i].stWdrRegCfg.stUsrRegCfg;
        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stWdrRegCfg.stDynaRegCfg;

        /* static */
        if (pstStaticRegCfg->bFirstFrame == HI_TRUE) {
            isp_wdr_expovalue0_write(pstBeReg, pstStaticRegCfg->au16ExpoValue[0]);
            isp_wdr_expovalue1_write(pstBeReg, pstStaticRegCfg->au16ExpoValue[1]);
            isp_wdr_maxratio_write(pstBeReg, pstStaticRegCfg->u32MaxRatio);
            isp_wdr_exporratio0_write(pstBeReg, pstStaticRegCfg->au16ExpoRRatio[0]);
            isp_wdr_fusionmode_write(pstBeReg, pstUsrRegCfg->bFusionMode);
            isp_wdr_mdt_en_write(pstBeReg, pstDynaRegCfg->bWDRMdtEn);
            isp_wdr_short_thr_write(pstBeReg, pstDynaRegCfg->u16ShortThr);
            isp_wdr_long_thr_write(pstBeReg, pstDynaRegCfg->u16LongThr);
            u16Offset0  = pstRegCfgInfo->stAlgRegCfg[i].stWdrRegCfg.stSyncRegCfg.u16Offset0;
            u32BlcComp0 = (pstStaticRegCfg->au16ExpoValue[0] - pstStaticRegCfg->au16ExpoValue[1]) * u16Offset0;
            pstStaticRegCfg->au32BlcComp[0] = u32BlcComp0;
            isp_wdr_blc_comp0_write(pstBeReg, u32BlcComp0);
            pstStaticRegCfg->bFirstFrame = HI_FALSE;
        }

        if (pstStaticRegCfg->bResh) {
            isp_wdr_grayscale_mode_write(pstBeReg, pstStaticRegCfg->bGrayScaleMode);
            isp_wdr_bsaveblc_write(pstBeReg, pstStaticRegCfg->bSaveBLC);
            isp_wdr_mask_similar_thr_write(pstBeReg, pstStaticRegCfg->u8MaskSimilarThr);
            isp_wdr_mask_similar_cnt_write(pstBeReg, pstStaticRegCfg->u8MaskSimilarCnt);
            isp_wdr_dftwgt_fl_write(pstBeReg, pstStaticRegCfg->u16dftWgtFL);
            isp_wdr_bldrlhfidx_write(pstBeReg, pstStaticRegCfg->u8bldrLHFIdx);
            isp_wdr_saturate_thr_write(pstBeReg, pstStaticRegCfg->u16SaturateThr);
            isp_wdr_fusion_saturate_thd_write(pstBeReg, pstStaticRegCfg->u16FusionSaturateThd);
            isp_wdr_forcelong_smooth_en_write(pstBeReg, pstStaticRegCfg->bForceLongSmoothEn);
            pstStaticRegCfg->bResh = HI_FALSE;
        }

        /* usr */
        bIdxResh = (isp_wdr_update_index_read(pstBeReg) != pstUsrRegCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstUsrRegCfg->bResh & bIdxResh) : (pstUsrRegCfg->bResh);

        if (bUsrResh)
        {
            isp_wdr_update_index_write(pstBeReg, pstUsrRegCfg->u32UpdateIndex);
            isp_wdr_shortexpo_chk_write(pstBeReg, pstUsrRegCfg->bShortExpoChk);
            isp_wdr_mdtlbld_write(pstBeReg, pstUsrRegCfg->u8MdtLBld);
            isp_wdr_mdt_full_thr_write(pstBeReg, pstUsrRegCfg->u8MdtFullThr);
            isp_wdr_mdt_still_thr_write(pstBeReg, pstUsrRegCfg->u8MdtStillThr);
            isp_wdr_pixel_avg_max_diff_write(pstBeReg, pstUsrRegCfg->u16PixelAvgMaxDiff);

            pstUsrRegCfg->bResh = bIsOfflineMode;
        }

        /* dynamic */
        if (pstDynaRegCfg->bResh) {
            isp_wdr_sqrt_again_g_write(pstBeReg, pstDynaRegCfg->u8SqrtAgainG);
            isp_wdr_sqrt_dgain_g_write(pstBeReg, pstDynaRegCfg->u8SqrtDgainG);
            isp_wdr_mdt_nosfloor_write(pstBeReg, pstDynaRegCfg->u8MdtNosFloor);
            isp_wdr_mdthr_low_gain_write(pstBeReg, pstDynaRegCfg->u8MdThrLowGain);
            isp_wdr_mdthr_hig_gain_write(pstBeReg, pstDynaRegCfg->u8MdThrHigGain);

            isp_wdr_f0_still_thr_write(pstBeReg, pstDynaRegCfg->au16StillThr[0]);
            isp_wdr_erosion_en_write(pstBeReg, pstDynaRegCfg->bErosionEn);
            isp_wdr_fusion_f0_thr_r_write(pstBeReg, pstDynaRegCfg->au16FusionThrR[0]);
            isp_wdr_fusion_f1_thr_r_write(pstBeReg, pstDynaRegCfg->au16FusionThrR[1]);
            isp_wdr_fusion_f0_thr_g_write(pstBeReg, pstDynaRegCfg->au16FusionThrG[0]);
            isp_wdr_fusion_f1_thr_g_write(pstBeReg, pstDynaRegCfg->au16FusionThrG[1]);
            isp_wdr_fusion_f0_thr_b_write(pstBeReg, pstDynaRegCfg->au16FusionThrB[0]);
            isp_wdr_fusion_f1_thr_b_write(pstBeReg, pstDynaRegCfg->au16FusionThrB[1]);

            isp_bcom_en_write(pstViProcReg, pstDynaRegCfg->bBcomEn);
            isp_bdec_en_write(pstViProcReg, pstDynaRegCfg->bBdecEn);
            isp_bcom_alpha_write(pstBeReg, pstDynaRegCfg->u8bcom_alpha);
            isp_bdec_alpha_write(pstBeReg, pstDynaRegCfg->u8bdec_alpha);

            isp_wdr_forcelong_en_write(pstBeReg, pstDynaRegCfg->bForceLong);
            isp_wdr_forcelong_low_thd_write(pstBeReg, pstDynaRegCfg->u16ForceLongLowThr);
            isp_wdr_forcelong_high_thd_write(pstBeReg, pstDynaRegCfg->u16ForceLongHigThr);

            isp_wdr_shortchk_thd_write(pstBeReg, pstDynaRegCfg->u16ShortCheckThd);
            isp_wdr_fusion_data_mode_write(pstBeReg, pstDynaRegCfg->bFusionDataMode);

            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1FsWdrCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }
    return HI_SUCCESS;
}
static HI_S32 ISP_DrcRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bUsrResh     = HI_FALSE;
    HI_BOOL  bIdxResh     = HI_FALSE;
    HI_BOOL  bTmLutUpdate = HI_FALSE;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_U16   j;
    ISP_DRC_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_DRC_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    ISP_DRC_USR_CFG_S    *pstUsrRegCfg    = HI_NULL;
    S_VIPROC_REGS_TYPE   *pstViProcReg    = HI_NULL;
    S_ISPBE_REGS_TYPE    *pstBeReg        = HI_NULL;
    ISP_CTX_S            *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if ( pstRegCfgInfo->unKey.bit1DrcCfg)
    {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);

        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);

        isp_drc_en_write(pstViProcReg, pstRegCfgInfo->stAlgRegCfg[i].stDrcRegCfg.bDrcEn);

        if (HI_TRUE == pstRegCfgInfo->stAlgRegCfg[i].stDrcRegCfg.bDrcEn)
        {
            isp_drc_dither_en_write(pstBeReg, HI_FALSE);
        }
        else
        {
            isp_drc_dither_en_write(pstBeReg, !(DYNAMIC_RANGE_XDR == pstIspCtx->stHdrAttr.enDynamicRange));
        }

        /*Static*/
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stDrcRegCfg.stStaticRegCfg;

        if ( pstStaticRegCfg->bStaticResh )
        {
            isp_drc_outbits_write(pstBeReg, pstStaticRegCfg->u8BitWidthOut);

            isp_drc_wrstat_en_write(pstBeReg, pstStaticRegCfg->bWrstatEn);
            isp_drc_rdstat_en_write(pstBeReg, pstStaticRegCfg->bRdstatEn);
            //isp_drc_vbiflt_en_write(pstBeReg, pstStaticRegCfg->bVbifltEn);

            isp_drc_bin_write(pstBeReg, pstStaticRegCfg->u8BinNumZ);

            /* Static Registers*/
            isp_drc_local_edge_lmt_write(pstBeReg, pstStaticRegCfg->u8LocalEdgeLmt);

            isp_drc_r_wgt_write(pstBeReg, pstStaticRegCfg->u8RWgt);
            isp_drc_g_wgt_write(pstBeReg, pstStaticRegCfg->u8GWgt);
            isp_drc_b_wgt_write(pstBeReg, pstStaticRegCfg->u8BWgt);

            isp_drc_cc_lin_pow_write(pstBeReg, pstStaticRegCfg->u8ColorControlMode);
            isp_drc_cc_lut_ctrl_write(pstBeReg, pstStaticRegCfg->u8ColorControlLUTCtrl);
            isp_drc_cc_global_corr_write(pstBeReg, pstStaticRegCfg->u16GlobalColorCorr);

            isp_drc_wgt_box_tri_sel_write(pstBeReg, pstStaticRegCfg->bWgtBoxTriSel);
            isp_drc_color_corr_en_write(pstBeReg, pstStaticRegCfg->bColorCorrEnable);
            isp_drc_detail_boost_en_write(pstBeReg, pstStaticRegCfg->bDetailBoostEnable);
            isp_drc_pdw_sum_en_write(pstBeReg, pstStaticRegCfg->bPdwSumEnable);
            isp_drc_mono_chroma_en_write(pstBeReg, pstStaticRegCfg->bMonochromeMode);

            //Purple Fringe Detection & Reduction
            isp_drc_rg_ctr_write(pstBeReg, pstStaticRegCfg->u8PFRRGCtr);
            isp_drc_rg_wid_write(pstBeReg, pstStaticRegCfg->u8PFRRGWid);
            isp_drc_rg_slo_write(pstBeReg, pstStaticRegCfg->u8PFRRGSlo);

            isp_drc_bg_thr_write(pstBeReg, pstStaticRegCfg->u8PFRBGThr);
            isp_drc_bg_slo_write(pstBeReg, pstStaticRegCfg->u8PFRBGSlo);

            isp_drc_idxbase0_write(pstBeReg, pstStaticRegCfg->au8SegIdxBase[0]);
            isp_drc_idxbase1_write(pstBeReg, pstStaticRegCfg->au8SegIdxBase[1]);
            isp_drc_idxbase2_write(pstBeReg, pstStaticRegCfg->au8SegIdxBase[2]);
            isp_drc_idxbase3_write(pstBeReg, pstStaticRegCfg->au8SegIdxBase[3]);
            isp_drc_idxbase4_write(pstBeReg, pstStaticRegCfg->au8SegIdxBase[4]);
            isp_drc_idxbase5_write(pstBeReg, pstStaticRegCfg->au8SegIdxBase[5]);
            isp_drc_idxbase6_write(pstBeReg, pstStaticRegCfg->au8SegIdxBase[6]);
            isp_drc_idxbase7_write(pstBeReg, pstStaticRegCfg->au8SegIdxBase[7]);

            isp_drc_maxval0_write(pstBeReg, pstStaticRegCfg->au8SegMaxVal[0]);
            isp_drc_maxval1_write(pstBeReg, pstStaticRegCfg->au8SegMaxVal[1]);
            isp_drc_maxval2_write(pstBeReg, pstStaticRegCfg->au8SegMaxVal[2]);
            isp_drc_maxval3_write(pstBeReg, pstStaticRegCfg->au8SegMaxVal[3]);
            isp_drc_maxval4_write(pstBeReg, pstStaticRegCfg->au8SegMaxVal[4]);
            isp_drc_maxval5_write(pstBeReg, pstStaticRegCfg->au8SegMaxVal[5]);
            isp_drc_maxval6_write(pstBeReg, pstStaticRegCfg->au8SegMaxVal[6]);
            isp_drc_maxval7_write(pstBeReg, pstStaticRegCfg->au8SegMaxVal[7]);

            if (pstStaticRegCfg->bFirstFrame)
            {
                isp_drc_shp_log_write(pstBeReg, pstStaticRegCfg->u8ShpLog);
                isp_drc_shp_exp_write(pstBeReg, pstStaticRegCfg->u8ShpExp);
                isp_drc_div_denom_log_write(pstBeReg, pstStaticRegCfg->u32DivDenomLog);
                isp_drc_denom_exp_write(pstBeReg, pstStaticRegCfg->u32DenomExp);
                isp_drc_prev_luma_0_write(pstBeReg, pstStaticRegCfg->au32PrevLuma[0]);
                isp_drc_prev_luma_1_write(pstBeReg, pstStaticRegCfg->au32PrevLuma[1]);
                isp_drc_prev_luma_2_write(pstBeReg, pstStaticRegCfg->au32PrevLuma[2]);
                isp_drc_prev_luma_3_write(pstBeReg, pstStaticRegCfg->au32PrevLuma[3]);
                isp_drc_prev_luma_4_write(pstBeReg, pstStaticRegCfg->au32PrevLuma[4]);
                isp_drc_prev_luma_5_write(pstBeReg, pstStaticRegCfg->au32PrevLuma[5]);
                isp_drc_prev_luma_6_write(pstBeReg, pstStaticRegCfg->au32PrevLuma[6]);
                isp_drc_prev_luma_7_write(pstBeReg, pstStaticRegCfg->au32PrevLuma[7]);

                pstStaticRegCfg->bFirstFrame = HI_FALSE;
            }

            pstStaticRegCfg->bStaticResh = HI_FALSE;
        }

        /*User*/
        pstUsrRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stDrcRegCfg.stUsrRegCfg;
        bIdxResh = (isp_drc_update_index_read(pstBeReg) != pstUsrRegCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstUsrRegCfg->bUsrResh & bIdxResh) : (pstUsrRegCfg->bUsrResh);

        if (bUsrResh)
        {
            isp_drc_update_index_write(pstBeReg, pstUsrRegCfg->u32UpdateIndex);

            isp_drc_sft1_y_write(pstBeReg, pstUsrRegCfg->u8YSFT1);
            isp_drc_val1_y_write(pstBeReg, pstUsrRegCfg->u8YVAL1);
            isp_drc_sft2_y_write(pstBeReg, pstUsrRegCfg->u8YSFT2);
            isp_drc_val2_y_write(pstBeReg, pstUsrRegCfg->u8YVAL2);

            isp_drc_sft1_c_write(pstBeReg, pstUsrRegCfg->u8CSFT1);
            isp_drc_val1_c_write(pstBeReg, pstUsrRegCfg->u8CVAL1);
            isp_drc_sft2_c_write(pstBeReg, pstUsrRegCfg->u8CSFT2);
            isp_drc_val2_c_write(pstBeReg, pstUsrRegCfg->u8CVAL2);

            isp_drc_sft_write(pstBeReg, pstUsrRegCfg->u8SFT);
            isp_drc_val_write(pstBeReg, pstUsrRegCfg->u8VAL);

            isp_drc_var_spa_coarse_write(pstBeReg, pstUsrRegCfg->u8VarSpaCoarse);
            isp_drc_var_spa_medium_write(pstBeReg, pstUsrRegCfg->u8VarSpaMedium);
            isp_drc_var_spa_fine_write(pstBeReg, pstUsrRegCfg->u8VarSpaFine);

            isp_drc_var_rng_coarse_write(pstBeReg, pstUsrRegCfg->u8VarRngCoarse);
            isp_drc_var_rng_medium_write(pstBeReg, pstUsrRegCfg->u8VarRngMedium);
            isp_drc_var_rng_fine_write(pstBeReg, pstUsrRegCfg->u8VarRngFine);

            isp_drc_grad_rev_shift_write(pstBeReg, pstUsrRegCfg->u8GradShift);
            isp_drc_grad_rev_slope_write(pstBeReg, pstUsrRegCfg->u8GradSlope);
            isp_drc_grad_rev_max_write(pstBeReg, pstUsrRegCfg->u8GradMax);
            isp_drc_grad_rev_thres_write(pstBeReg, pstUsrRegCfg->u8GradThr);

            isp_drc_high_slo_write(pstBeReg, pstUsrRegCfg->u8PFRHighSlo);
            isp_drc_low_slo_write(pstBeReg, pstUsrRegCfg->u8PFRLowSlo );
            isp_drc_low_thr_write(pstBeReg, pstUsrRegCfg->u8PFRLowThr );

            isp_drc_gain_clip_knee_write(pstBeReg, pstUsrRegCfg->u8GainClipKnee);
            isp_drc_gain_clip_step_write(pstBeReg, pstUsrRegCfg->u8GainClipStep);

            isp_drc_mixing_coring_write(pstBeReg, pstUsrRegCfg->u8MixingCoring);
            isp_drc_dark_min_write(pstBeReg, pstUsrRegCfg->u8MixingDarkMin);
            isp_drc_dark_max_write(pstBeReg, pstUsrRegCfg->u8MixingDarkMax);
            isp_drc_dark_thr_write(pstBeReg, pstUsrRegCfg->u8MixingDarkThr);
            isp_drc_dark_slo_write(pstBeReg, pstUsrRegCfg->s8MixingDarkSlo);

            isp_drc_bright_min_write(pstBeReg, pstUsrRegCfg->u8MixingBrightMin);
            isp_drc_bright_max_write(pstBeReg, pstUsrRegCfg->u8MixingBrightMax);
            isp_drc_bright_thr_write(pstBeReg, pstUsrRegCfg->u8MixingBrightThr);
            isp_drc_bright_slo_write(pstBeReg, pstUsrRegCfg->s8MixingBrightSlo);

            isp_drc_detail_coring_write(pstBeReg, pstUsrRegCfg->u8DetailCoring);
            isp_drc_dark_step_write(pstBeReg, pstUsrRegCfg->u8DetailDarkStep);
            isp_drc_bright_step_write(pstBeReg, pstUsrRegCfg->u8DetailBrightStep);

            isp_drc_detail_dark_min_write(pstBeReg, pstUsrRegCfg->u8DetailDarkMin);
            isp_drc_detail_dark_max_write(pstBeReg, pstUsrRegCfg->u8DetailDarkMax);
            isp_drc_detail_dark_thr_write(pstBeReg, pstUsrRegCfg->u8DetailDarkThr);
            isp_drc_detail_dark_slo_write(pstBeReg, pstUsrRegCfg->s8DetailDarkSlo);

            isp_drc_detail_bright_min_write(pstBeReg, pstUsrRegCfg->u8DetailBrightMin);
            isp_drc_detail_bright_max_write(pstBeReg, pstUsrRegCfg->u8DetailBrightMax);
            isp_drc_detail_bright_thr_write(pstBeReg, pstUsrRegCfg->u8DetailBrightThr);
            isp_drc_detail_bright_slo_write(pstBeReg, pstUsrRegCfg->s8DetailBrightSlo);

            //dark & bright curve write
            isp_drc_detail_dark_curve0_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[0]);
            isp_drc_detail_dark_curve1_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[1]);
            isp_drc_detail_dark_curve2_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[2]);
            isp_drc_detail_dark_curve3_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[3]);
            isp_drc_detail_dark_curve4_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[4]);
            isp_drc_detail_dark_curve5_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[5]);
            isp_drc_detail_dark_curve6_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[6]);
            isp_drc_detail_dark_curve7_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[7]);
            isp_drc_detail_dark_curve8_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[8]);
            isp_drc_detail_dark_curve9_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[9]);
            isp_drc_detail_dark_curve10_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[10]);
            isp_drc_detail_dark_curve11_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[11]);
            isp_drc_detail_dark_curve12_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[12]);
            isp_drc_detail_dark_curve13_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[13]);
            isp_drc_detail_dark_curve14_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[14]);
            isp_drc_detail_dark_curve15_write(pstBeReg, pstUsrRegCfg->u8DetailDarkCurve[15]);

            isp_drc_detail_bright_curve0_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[0]);
            isp_drc_detail_bright_curve1_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[1]);
            isp_drc_detail_bright_curve2_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[2]);
            isp_drc_detail_bright_curve3_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[3]);
            isp_drc_detail_bright_curve4_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[4]);
            isp_drc_detail_bright_curve5_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[5]);
            isp_drc_detail_bright_curve6_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[6]);
            isp_drc_detail_bright_curve7_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[7]);
            isp_drc_detail_bright_curve8_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[8]);
            isp_drc_detail_bright_curve9_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[9]);
            isp_drc_detail_bright_curve10_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[10]);
            isp_drc_detail_bright_curve11_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[11]);
            isp_drc_detail_bright_curve12_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[12]);
            isp_drc_detail_bright_curve13_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[13]);
            isp_drc_detail_bright_curve14_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[14]);
            isp_drc_detail_bright_curve15_write(pstBeReg, pstUsrRegCfg->u8DetailBrightCurve[15]);

            isp_drc_cubic_sel_write(pstBeReg, pstUsrRegCfg->bCubicCurveSel);

            /* Spatial/range filtering coefficients */
            isp_drc_flt_spa_fine_write(pstBeReg, pstUsrRegCfg->u8FltSpaFine);
            isp_drc_flt_spa_medium_write(pstBeReg, pstUsrRegCfg->u8FltSpaMedium);
            isp_drc_flt_spa_coarse_write(pstBeReg, pstUsrRegCfg->u8FltSpaCoarse);
            isp_drc_flt_rng_fine_write(pstBeReg, pstUsrRegCfg->u8FltRngFine);
            isp_drc_flt_rng_medium_write(pstBeReg, pstUsrRegCfg->u8FltRngMedium);
            isp_drc_flt_rng_coarse_write(pstBeReg, pstUsrRegCfg->u8FltRngCoarse);

            /* Adaptive range filtering parameters */
            isp_drc_fr_ada_max_write(pstBeReg, pstUsrRegCfg->u8FltRngAdaMax);
            isp_drc_dis_offset_coef_write(pstBeReg, pstUsrRegCfg->u8DisOffsetCoef);
            isp_drc_thr_coef_low_write(pstBeReg, pstUsrRegCfg->u8DisThrCoefLow);
            isp_drc_thr_coef_high_write(pstBeReg, pstUsrRegCfg->u8DisThrCoefHigh);

            /* Adaptive spatial filtering parameters */
            isp_drc_fs_ada_max_write(pstBeReg, pstUsrRegCfg->u8FltSpaAdaMax);
            isp_drc_diff_thr_low_write(pstBeReg, pstUsrRegCfg->u8DiffThrLow);
            isp_drc_diff_thr_high_write(pstBeReg, pstUsrRegCfg->u8DiffThrHigh);

            /* Detail suppression parameters */
            isp_drc_suppress_bright_max_write(pstBeReg, pstUsrRegCfg->u8SuppressBrightMax);
            isp_drc_suppress_bright_min_write(pstBeReg, pstUsrRegCfg->u8SuppressBrightMin);
            isp_drc_suppress_bright_thr_write(pstBeReg, pstUsrRegCfg->u8SuppressBrightThr);
            isp_drc_suppress_bright_slo_write(pstBeReg, pstUsrRegCfg->u8SuppressBrightSlo);
            isp_drc_suppress_dark_max_write(pstBeReg, pstUsrRegCfg->u8SuppressDarkMax);
            isp_drc_suppress_dark_min_write(pstBeReg, pstUsrRegCfg->u8SuppressDarkMin);
            isp_drc_suppress_dark_thr_write(pstBeReg, pstUsrRegCfg->u8SuppressDarkThr);
            isp_drc_suppress_dark_slo_write(pstBeReg, pstUsrRegCfg->u8SuppressDarkSlo);

            isp_drc_detail_sub_factor_write(pstBeReg, pstUsrRegCfg->s8DetailSubFactor);

            isp_drc_bin_mix_factor_coarse_0_write(pstBeReg, pstUsrRegCfg->au8BinMixCoarse[0]);
            isp_drc_bin_mix_factor_coarse_1_write(pstBeReg, pstUsrRegCfg->au8BinMixCoarse[1]);
            isp_drc_bin_mix_factor_coarse_2_write(pstBeReg, pstUsrRegCfg->au8BinMixCoarse[2]);
            isp_drc_bin_mix_factor_coarse_3_write(pstBeReg, pstUsrRegCfg->au8BinMixCoarse[3]);
            isp_drc_bin_mix_factor_coarse_4_write(pstBeReg, pstUsrRegCfg->au8BinMixCoarse[4]);
            isp_drc_bin_mix_factor_coarse_5_write(pstBeReg, pstUsrRegCfg->au8BinMixCoarse[5]);
            isp_drc_bin_mix_factor_coarse_6_write(pstBeReg, pstUsrRegCfg->au8BinMixCoarse[6]);
            isp_drc_bin_mix_factor_coarse_7_write(pstBeReg, pstUsrRegCfg->au8BinMixCoarse[7]);

            isp_drc_bin_mix_factor_medium_0_write(pstBeReg, pstUsrRegCfg->au8BinMixMedium[0]);
            isp_drc_bin_mix_factor_medium_1_write(pstBeReg, pstUsrRegCfg->au8BinMixMedium[1]);
            isp_drc_bin_mix_factor_medium_2_write(pstBeReg, pstUsrRegCfg->au8BinMixMedium[2]);
            isp_drc_bin_mix_factor_medium_3_write(pstBeReg, pstUsrRegCfg->au8BinMixMedium[3]);
            isp_drc_bin_mix_factor_medium_4_write(pstBeReg, pstUsrRegCfg->au8BinMixMedium[4]);
            isp_drc_bin_mix_factor_medium_5_write(pstBeReg, pstUsrRegCfg->au8BinMixMedium[5]);
            isp_drc_bin_mix_factor_medium_6_write(pstBeReg, pstUsrRegCfg->au8BinMixMedium[6]);
            isp_drc_bin_mix_factor_medium_7_write(pstBeReg, pstUsrRegCfg->au8BinMixMedium[7]);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                isp_drc_cclut_waddr_write(pstBeReg, 0);

                for ( j = 0 ; j < HI_ISP_DRC_CC_NODE_NUM; j++ )
                {
                    isp_drc_cclut_wdata_write(pstBeReg, pstUsrRegCfg->u16CCLUT[j]);
                }
            }
            else
            {
                isp_drc_cclut_write(pstBeReg, pstUsrRegCfg->u16CCLUT);
            }

            pstUsrRegCfg->bUsrResh = bIsOfflineMode;
        }

        /*Dynamic*/
        pstDynaRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stDrcRegCfg.stDynaRegCfg;

        if (pstDynaRegCfg->bDynaResh)
        {
            isp_drc_high_thr_write(pstBeReg, pstDynaRegCfg->u8PFRHighThr);

            if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                isp_drc_wrstat_en_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stDrcRegCfg.bDrcEn);
                isp_drc_rdstat_en_write(pstBeReg, pstRegCfgInfo->stAlgRegCfg[i].stDrcRegCfg.bDrcEn);
            }

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                if (pstDynaRegCfg->bLutUpdate)
                {
                    isp_drc_tmlut0_waddr_write(pstBeReg, 0);
                    for (j = 0; j < HI_ISP_DRC_TM_NODE_NUM; j++)
                    {
                        isp_drc_tmlut0_wdata_write(pstBeReg, ((pstDynaRegCfg->au16ToneMappingValue0[j]) << 14) | (pstDynaRegCfg->au16ToneMappingDiff0[j]));
                    }
                    //isp_drc_lut_update_write(pstBeReg, 1);
                    bTmLutUpdate = HI_TRUE;
                }
            }
            else
            {
                isp_drc_tmlut0_value_write(pstBeReg, pstDynaRegCfg->au16ToneMappingValue0);
                isp_drc_tmlut0_diff_write(pstBeReg, pstDynaRegCfg->au16ToneMappingDiff0);
                //isp_drc_lut_update_write(pstBeReg, 1);
            }

            isp_drc_lut_mix_ctrl_write(pstBeReg, pstDynaRegCfg->u16LutMixCtrl);
            isp_drc_strength_write(pstBeReg, pstDynaRegCfg->u16Strength);

            if (pstDynaRegCfg->bImgSizeChanged)
            {
                isp_drc_vbiflt_en_write(pstBeReg, pstDynaRegCfg->bVbifltEn);
                isp_drc_big_x_init_write(pstBeReg, pstDynaRegCfg->u8BigXInit);
                isp_drc_idx_x_init_write(pstBeReg, pstDynaRegCfg->u8IdxXInit);
                isp_drc_cnt_x_init_write(pstBeReg, pstDynaRegCfg->u16CntXInit);
                isp_drc_acc_x_init_write(pstBeReg, pstDynaRegCfg->u16AccXInit);
                isp_drc_blk_wgt_init_write(pstBeReg, pstDynaRegCfg->u16WgtXInit); // drc v4.0
                isp_drc_total_width_write(pstBeReg, pstDynaRegCfg->u16TotalWidth - 1);
                isp_drc_stat_width_write(pstBeReg, pstDynaRegCfg->u16StatWidth - 1);

                isp_drc_hnum_write(pstBeReg, pstDynaRegCfg->u8BlockHNum);
                isp_drc_vnum_write(pstBeReg, pstDynaRegCfg->u8BlockVNum);

                isp_drc_zone_hsize_write(pstBeReg, pstDynaRegCfg->u16BlockHSize - 1);
                isp_drc_zone_vsize_write(pstBeReg, pstDynaRegCfg->u16BlockVSize - 1);
                isp_drc_chk_x_write(pstBeReg, pstDynaRegCfg->u8BlockChkX);
                isp_drc_chk_y_write(pstBeReg, pstDynaRegCfg->u8BlockChkY);

                isp_drc_div_x0_write(pstBeReg, pstDynaRegCfg->u16DivX0);
                isp_drc_div_x1_write(pstBeReg, pstDynaRegCfg->u16DivX1);
                isp_drc_div_y0_write(pstBeReg, pstDynaRegCfg->u16DivY0);
                isp_drc_div_y1_write(pstBeReg, pstDynaRegCfg->u16DivY1);

                isp_drc_bin_scale_write(pstBeReg, pstDynaRegCfg->u8BinScale);

                pstDynaRegCfg->bImgSizeChanged = bIsOfflineMode;
            }

            pstDynaRegCfg->bDynaResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1DrcCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg.bDrcTmLutUpdate = bTmLutUpdate | bIsOfflineMode;

    return HI_SUCCESS;
}

static HI_S32 ISP_DehazeRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bIsOfflineMode;
    HI_U8    u8BufId;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_U16   u16BlkNum;
    ISP_DEHAZE_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_DEHAZE_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    S_ISP_LUT_WSTT_TYPE     *pstBeLutSttReg  = HI_NULL;
    S_VIPROC_REGS_TYPE      *pstViProcReg    = HI_NULL;
    S_ISPBE_REGS_TYPE       *pstBeReg        = HI_NULL;
    ISP_CTX_S               *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)\
                      || IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if ( pstRegCfgInfo->unKey.bit1DehazeCfg)
    {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stDehazeRegCfg.stStaticRegCfg;
        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stDehazeRegCfg.stDynaRegCfg;

        u16BlkNum = ((pstStaticRegCfg->u8Dchnum + 1) * (pstStaticRegCfg->u8Dcvnum + 1) + 1) / 2;
        //isp_dehaze_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stDehazeRegCfg.bDehazeEn);
        /*Static Registers*/

        if (pstStaticRegCfg->bResh)
        {
            isp_dehaze_max_mode_write(pstBeReg, pstStaticRegCfg->u8MaxMode);
            isp_dehaze_thld_write(pstBeReg, pstStaticRegCfg->u16DehazeThld);
            isp_dehaze_blthld_write(pstBeReg, pstStaticRegCfg->u16DehazeBlthld);
            isp_dehaze_neg_mode_write(pstBeReg, pstStaticRegCfg->u8DehazeNegMode);
            isp_dehaze_block_sum_write(pstBeReg, pstStaticRegCfg->u16BlockSum);
            isp_dehaze_dc_numh_write(pstBeReg, pstStaticRegCfg->u8Dchnum);
            isp_dehaze_dc_numv_write(pstBeReg, pstStaticRegCfg->u8Dcvnum);
            isp_dehaze_lut_width_word_write(pstViProcReg, HI_ISP_DEHAZE_LUT_WIDTH_WORD_DEFAULT);
            pstStaticRegCfg->bResh = HI_FALSE;
        }

        /*Dynamic Regs*/
        if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
            IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
        {
            /*online Lut2stt regconfig*/
            u8BufId = pstDynaRegCfg->u8BufId;

            pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, u8BufId);
            ISP_CHECK_POINTER(pstBeLutSttReg);

            isp_dehaze_lut_wstt_write(pstBeLutSttReg, u16BlkNum, pstDynaRegCfg->prestat, pstDynaRegCfg->u8Lut);
            isp_dehaze_lut_wstt_addr_write(ViPipe, i, u8BufId, pstViProcReg);
            isp_dehaze_stt2lut_en_write(pstBeReg, HI_TRUE);
            isp_dehaze_stt2lut_regnew_write(pstBeReg, HI_TRUE);
            pstDynaRegCfg->u8BufId = 1 - u8BufId;
        }
        else
        {

            isp_dehaze_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, u16BlkNum, pstDynaRegCfg->prestat, pstDynaRegCfg->u8Lut);
            isp_dehaze_stt2lut_en_write(pstBeReg, HI_TRUE);
            isp_dehaze_stt2lut_regnew_write(pstBeReg, HI_TRUE);
        }

        //isp_dehaze_pre_lut_update_write(pstBeReg, pstDynaRegCfg->u32Update);

        //isp_dehaze_gstrth_write(pstBeReg, pstDynaRegCfg->u8Strength);
        isp_dehaze_block_sizeh_write(pstBeReg, pstDynaRegCfg->u16Blockhsize);
        isp_dehaze_block_sizev_write(pstBeReg, pstDynaRegCfg->u16Blockvsize);
        isp_dehaze_phase_x_write(pstBeReg, pstDynaRegCfg->u32phasex);
        isp_dehaze_phase_y_write(pstBeReg, pstDynaRegCfg->u32phasey);

        isp_dehaze_smlmapoffset_write(pstBeReg, pstDynaRegCfg->u32smlMapOffset);
        isp_dehaze_statstartx_write(pstBeReg, pstDynaRegCfg->u32StatStartX);
        isp_dehaze_statendx_write(pstBeReg, pstDynaRegCfg->u32StatEndX);

        isp_dehaze_stat_numv_write(pstBeReg, pstDynaRegCfg->u8StatnumV);
        isp_dehaze_stat_numh_write(pstBeReg, pstDynaRegCfg->u8StatnumH);

        pstRegCfgInfo->unKey.bit1DehazeCfg  = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_BayerNrRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_U16   j;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bLutUpdate = HI_FALSE;
    ISP_BAYERNR_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_BAYERNR_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    ISP_BAYERNR_USR_CFG_S    *pstUsrRegCfg    = HI_NULL;
    S_ISPBE_REGS_TYPE        *pstBeReg        = HI_NULL;
    ISP_CTX_S                *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)\
                      || IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1BayernrCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        //isp_bnr_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stBnrRegCfg.bBnrEnable);

        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stBnrRegCfg.stStaticRegCfg;
        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stBnrRegCfg.stDynaRegCfg;
        pstUsrRegCfg    = &pstRegCfgInfo->stAlgRegCfg[i].stBnrRegCfg.stUsrRegCfg;

        if (pstStaticRegCfg->bResh)         /*satic*/
        {
            isp_bnr_jnlm_sel_write(pstBeReg, pstStaticRegCfg->u8JnlmSel);
            isp_bnr_sad_winsize_sel_write(pstBeReg, pstStaticRegCfg->u8SADWinSizeSel);
            isp_bnr_jnlm_maxwtcoef_write(pstBeReg, pstStaticRegCfg->u16JnlmMaxWtCoef);
            pstStaticRegCfg->bResh = HI_FALSE;
        }

        if (pstDynaRegCfg->bResh)
        {
            isp_bnr_medc_enable_write(pstBeReg, pstDynaRegCfg->bMedcEnable);
            //isp_bnr_wdr_enable_write(pstBeReg, pstDynaRegCfg->bWdrModeEn);
            isp_bnr_wdr_enfusion_write(pstBeReg, pstDynaRegCfg->bWdrFusionEn);
            isp_bnr_encenterweight_write(pstBeReg, pstDynaRegCfg->bCenterWgtEn);
            isp_bnr_wdr_mapgain_write(pstBeReg, pstDynaRegCfg->u8WdrMapGain);
            isp_bnr_wdr_mapfltmod_write(pstBeReg, pstDynaRegCfg->u8WdrMapFltMode);
            isp_bnr_ratio_r_write(pstBeReg, pstDynaRegCfg->au8BnrCRatio[0]);
            isp_bnr_ratio_gr_write(pstBeReg, pstDynaRegCfg->au8BnrCRatio[1]);
            isp_bnr_ratio_gb_write(pstBeReg, pstDynaRegCfg->au8BnrCRatio[2]);
            isp_bnr_ratio_b_write(pstBeReg, pstDynaRegCfg->au8BnrCRatio[3]);
            isp_bnr_amed_mode_r_write(pstBeReg, pstDynaRegCfg->au8AmedMode[0]);
            isp_bnr_amed_mode_gr_write(pstBeReg, pstDynaRegCfg->au8AmedMode[1]);
            isp_bnr_amed_mode_gb_write(pstBeReg, pstDynaRegCfg->au8AmedMode[2]);
            isp_bnr_amed_mode_b_write(pstBeReg, pstDynaRegCfg->au8AmedMode[3]);
            isp_bnr_amed_lev_r_write(pstBeReg, pstDynaRegCfg->au8AmedLevel[0]);
            isp_bnr_amed_lev_gr_write(pstBeReg, pstDynaRegCfg->au8AmedLevel[1]);
            isp_bnr_amed_lev_gb_write(pstBeReg, pstDynaRegCfg->au8AmedLevel[2]);
            isp_bnr_amed_lev_b_write(pstBeReg, pstDynaRegCfg->au8AmedLevel[3]);
            isp_bnr_jnlm_symcoef_write(pstBeReg, pstDynaRegCfg->u8JnlmSymCoef);
            isp_bnr_jnlm_gain_write(pstBeReg, pstDynaRegCfg->u8JnlmGain);
            isp_bnr_jnlm_coringhig_write(pstBeReg, pstDynaRegCfg->u16JnlmCoringHig);
            isp_bnr_shotratio_write(pstBeReg, pstDynaRegCfg->u16ShotRatio);
            isp_bnr_jnlm_edgedetth_write(pstBeReg, pstDynaRegCfg->u32EdgeDetThr);
            isp_bnr_rlmt_rgain_write(pstBeReg, pstDynaRegCfg->u16RLmtRgain);
            isp_bnr_rlmt_bgain_write(pstBeReg, pstDynaRegCfg->u16RLmtBgain);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                isp_bnr_lmt_odd_waddr_write(pstBeReg, 0);
                isp_bnr_lmt_s_odd_waddr_write(pstBeReg, 0);

                for (j = 1; j < HI_ISP_BAYERNR_LMTLUTNUM; j += 2)
                {
                    isp_bnr_lmt_odd_wdata_write(pstBeReg, pstDynaRegCfg->au8JnlmLimitLut[j]);
                    isp_bnr_lmt_s_odd_wdata_write(pstBeReg, pstDynaRegCfg->au8JnlmLimitSLut[j]);
                }

                isp_bnr_lmt_even_waddr_write(pstBeReg, 0);
                isp_bnr_lmt_s_even_waddr_write(pstBeReg, 0);

                for (j = 0; j < HI_ISP_BAYERNR_LMTLUTNUM; j += 2)
                {
                    isp_bnr_lmt_even_wdata_write(pstBeReg, pstDynaRegCfg->au8JnlmLimitLut[j]);
                    isp_bnr_lmt_s_even_wdata_write(pstBeReg, pstDynaRegCfg->au8JnlmLimitSLut[j]);
                }

                isp_bnr_cor_odd_waddr_write(pstBeReg, 0);
                isp_bnr_gcor_odd_waddr_write(pstBeReg, 0);

                for (j = 1; j < HI_ISP_BAYERNR_LUT_LENGTH; j += 2)
                {
                    isp_bnr_cor_odd_wdata_write(pstBeReg, pstDynaRegCfg->au16JnlmCoringLowLut[j]);
                    isp_bnr_gcor_odd_wdata_write(pstBeReg, pstDynaRegCfg->au8GCoringGainLut[j]);
                }

                isp_bnr_cor_even_waddr_write(pstBeReg, 0);
                isp_bnr_gcor_even_waddr_write(pstBeReg, 0);

                for (j = 0; j < HI_ISP_BAYERNR_LUT_LENGTH; j += 2)
                {
                    isp_bnr_cor_even_wdata_write(pstBeReg, pstDynaRegCfg->au16JnlmCoringLowLut[j]);
                    isp_bnr_gcor_even_wdata_write(pstBeReg, pstDynaRegCfg->au8GCoringGainLut[j]);
                }

                bLutUpdate = HI_TRUE;
            }
            else
            {
                isp_bnr_lmt_even_write(pstBeReg, pstDynaRegCfg->au8JnlmLimitLut);
                isp_bnr_lmt_odd_write(pstBeReg, pstDynaRegCfg->au8JnlmLimitLut);
                isp_bnr_cor_even_write(pstBeReg, pstDynaRegCfg->au16JnlmCoringLowLut);
                isp_bnr_cor_odd_write(pstBeReg, pstDynaRegCfg->au16JnlmCoringLowLut);
                isp_bnr_gcor_even_write(pstBeReg, pstDynaRegCfg->au8GCoringGainLut);
                isp_bnr_gcor_odd_write(pstBeReg, pstDynaRegCfg->au8GCoringGainLut);
                isp_bnr_lmt_s_odd_write(pstBeReg, pstDynaRegCfg->au8JnlmLimitSLut);
                isp_bnr_lmt_s_even_write(pstBeReg, pstDynaRegCfg->au8JnlmLimitSLut);
            }

            isp_bnr_jnlmgain_r0_write(pstBeReg, pstDynaRegCfg->au32JnlmLimitMultGain[BAYER_RGGB]);
            isp_bnr_jnlmgain_gr0_write(pstBeReg, pstDynaRegCfg->au32JnlmLimitMultGain[BAYER_GRBG]);
            isp_bnr_jnlmgain_gb0_write(pstBeReg, pstDynaRegCfg->au32JnlmLimitMultGain[BAYER_GBRG]);
            isp_bnr_jnlmgain_b0_write(pstBeReg, pstDynaRegCfg->au32JnlmLimitMultGain[BAYER_BGGR]);
            isp_bnr_jnlmgain_r_s_write(pstBeReg, pstDynaRegCfg->au32JnlmLimitMultGainS[BAYER_RGGB]);
            isp_bnr_jnlmgain_gr_s_write(pstBeReg, pstDynaRegCfg->au32JnlmLimitMultGainS[BAYER_GRBG]);
            isp_bnr_jnlmgain_gb_s_write(pstBeReg, pstDynaRegCfg->au32JnlmLimitMultGainS[BAYER_GBRG]);
            isp_bnr_jnlmgain_b_s_write(pstBeReg, pstDynaRegCfg->au32JnlmLimitMultGainS[BAYER_BGGR]);

            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        if (pstUsrRegCfg->bResh)
        {
            //isp_bnr_rlsc_en_write(pstBeReg, pstUsrRegCfg->bBnrLscEn);
            isp_bnr_mono_sensor_write(pstBeReg, pstUsrRegCfg->bBnrMonoSensorEn);
            isp_bnr_rlsc_maxgain_write(pstBeReg, pstUsrRegCfg->u8BnrLscMaxGain);
            isp_bnr_rlsc_cmp_str_write(pstBeReg, pstUsrRegCfg->u16BnrLscCmpStrength);

            pstUsrRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1BayernrCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg.bBnrLutUpdate = bLutUpdate | bIsOfflineMode;

    return HI_SUCCESS;
}

static HI_S32 ISP_BNRLscRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bUsrResh   = HI_FALSE;
    HI_BOOL  bIdxResh   = HI_FALSE;
    HI_BOOL  bStt2LutRegnew = HI_FALSE;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_U8    u8BufId;
    ISP_RLSC_USR_CFG_S      *pstUsrRegCfg    = HI_NULL;
    ISP_RLSC_STATIC_CFG_S   *pstStaticRegCfg = HI_NULL;
    S_ISP_LUT_WSTT_TYPE     *pstBeLutSttReg  = HI_NULL;
    S_VIPROC_REGS_TYPE      *pstViProcReg    = HI_NULL;
    S_ISPBE_REGS_TYPE       *pstBeReg        = HI_NULL;
    ISP_CTX_S               *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1RLscCfg)
    {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);
        /*static*/
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stRLscRegCfg.stStaticRegCfg;

        if (pstStaticRegCfg->bStaticResh)
        {
            isp_bnr_rlsc_nodenum_write(pstBeReg, pstStaticRegCfg->u16NodeNum);
            isp_bnr_lut_width_word_write(pstViProcReg, HI_ISP_BNR_RLSC_LUT_WIDTH_WORD_DEFAULT);
            pstStaticRegCfg->bStaticResh = HI_FALSE;
        }

        /*usr*/
        pstUsrRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg;

        isp_bnr_rlsc_rx_write(pstBeReg, pstUsrRegCfg->u16CenterRX);
        isp_bnr_rlsc_ry_write(pstBeReg, pstUsrRegCfg->u16CenterRY);
        isp_bnr_rlsc_grx_write(pstBeReg, pstUsrRegCfg->u16CenterGrX);
        isp_bnr_rlsc_gry_write(pstBeReg, pstUsrRegCfg->u16CenterGrY);
        isp_bnr_rlsc_gbx_write(pstBeReg, pstUsrRegCfg->u16CenterGbX);
        isp_bnr_rlsc_gby_write(pstBeReg, pstUsrRegCfg->u16CenterGbY);
        isp_bnr_rlsc_bx_write(pstBeReg, pstUsrRegCfg->u16CenterBX);
        isp_bnr_rlsc_by_write(pstBeReg, pstUsrRegCfg->u16CenterBY);

        isp_bnr_rlsc_offsetcenterr_write(pstBeReg, pstUsrRegCfg->u16OffCenterR);
        isp_bnr_rlsc_offsetcentergr_write(pstBeReg, pstUsrRegCfg->u16OffCenterGr);
        isp_bnr_rlsc_offsetcentergb_write(pstBeReg, pstUsrRegCfg->u16OffCenterGb);
        isp_bnr_rlsc_offsetcenterb_write(pstBeReg, pstUsrRegCfg->u16OffCenterB);

        if (pstUsrRegCfg->bCoefUpdate)
        {
            //isp_bnr_rlsc_en_write(pstBeReg, pstUsrRegCfg->bRLscFuncEn);
            isp_bnr_rlsc_gainstr_write(pstBeReg, pstUsrRegCfg->u16GainStr);

            pstUsrRegCfg->bCoefUpdate = bIsOfflineMode;
        }

        bIdxResh = (isp_rlsc_update_index_read(pstBeReg) != pstUsrRegCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstUsrRegCfg->bLutUpdate & bIdxResh) : (pstUsrRegCfg->bLutUpdate);

        if (bUsrResh)
        {
            isp_rlsc_update_index_write(pstBeReg, pstUsrRegCfg->u32UpdateIndex);
            isp_bnr_rlsc_gainscale_write(pstBeReg, pstUsrRegCfg->u8GainScale);
            isp_bnr_rlsc_widthoffset_write(pstBeReg, pstUsrRegCfg->u16WidthOffset);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)\
                || IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                /*online Lut2stt regconfig*/
                u8BufId = pstUsrRegCfg->u8BufId;

                pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, u8BufId);
                ISP_CHECK_POINTER(pstBeLutSttReg);
                isp_bnr_lut_wstt_write(pstBeLutSttReg, pstUsrRegCfg->au16GainLut[0], pstUsrRegCfg->au16GainLut[1], pstUsrRegCfg->au16GainLut[2], pstUsrRegCfg->au16GainLut[3]);

                isp_bnr_rlsc_lut_wstt_addr_write(ViPipe, i, u8BufId, pstViProcReg);

                isp_bnr_stt2lut_en_write(pstBeReg, HI_TRUE);
                //isp_bnr_stt2lut_regnew_write(pstBeReg, HI_TRUE);

                pstUsrRegCfg->u8BufId = 1 - u8BufId;

                bStt2LutRegnew = HI_TRUE;
            }
            else
            {
                isp_bnr_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstUsrRegCfg->au16GainLut[0], pstUsrRegCfg->au16GainLut[1], pstUsrRegCfg->au16GainLut[2], pstUsrRegCfg->au16GainLut[3]);
                isp_bnr_stt2lut_en_write(pstBeReg, HI_TRUE);
                isp_bnr_stt2lut_regnew_write(pstBeReg, HI_TRUE);
            }

            pstUsrRegCfg->bLutUpdate = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1RLscCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stStt2LutRegnewCfg.bBnrLscStt2LutRegnew = bStt2LutRegnew;

    if (HI_TRUE == pstRegCfgInfo->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg.bSwitchLut2SttRegNew)
    {
        if (pstRegCfgInfo->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg.u8SwitchRegNewCnt < 3)
        {
            pstRegCfgInfo->stAlgRegCfg[i].stStt2LutRegnewCfg.bBnrLscStt2LutRegnew = HI_TRUE;
            pstRegCfgInfo->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg.u8SwitchRegNewCnt++;
        }
        else
        {
            pstRegCfgInfo->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg.bSwitchLut2SttRegNew = HI_FALSE;
            pstRegCfgInfo->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg.u8SwitchRegNewCnt    = 0;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_DetailRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_U16   j;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bLutUpdate = HI_FALSE;
    ISP_DETAIL_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_DETAIL_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    ISP_DETAIL_USR_CFG_S    *pstUsrRegCfg    = HI_NULL;
    S_ISPBE_REGS_TYPE       *pstBeReg        = HI_NULL;
    ISP_CTX_S               *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1DetailCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        //isp_de_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stDeRegCfg.bDeEnable);
        //isp_bnr_de_enable_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stDeRegCfg.bDeEnable);

        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stDeRegCfg.stStaticRegCfg;
        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stDeRegCfg.stDynaRegCfg;
        pstUsrRegCfg    = &pstRegCfgInfo->stAlgRegCfg[i].stDeRegCfg.stUsrRegCfg;

        if (pstStaticRegCfg->bResh)
        {
            isp_bnr_de_posclip_write(pstBeReg, pstStaticRegCfg->u8DePosClip );
            isp_bnr_de_negclip_write(pstBeReg, pstStaticRegCfg->u8DeNegClip);
            isp_de_clip_ratio_write(pstBeReg, pstStaticRegCfg->u16ClipRatio  );
            isp_de_luma_scale_x0_write(pstBeReg, pstStaticRegCfg->u16LumaScaleX0 );
            isp_de_luma_scale_x1_write(pstBeReg, pstStaticRegCfg->u16LumaScaleX1 );
            isp_de_luma_scale_y0_write(pstBeReg, pstStaticRegCfg->u16LumaScaleY0 );
            isp_de_luma_scale_y1_write(pstBeReg, pstStaticRegCfg->u16LumaScaleY1 );
            isp_de_luma_scale_k_write(pstBeReg, pstStaticRegCfg->u16LumaScaleK );
            isp_de_satugain_x0_write(pstBeReg, pstStaticRegCfg->u16SatuGainX0);
            isp_de_satugain_x1_write(pstBeReg, pstStaticRegCfg->u16SatuGainX1);
            isp_de_satugain_y0_write(pstBeReg, pstStaticRegCfg->u16SatuGainY0);
            isp_de_satugain_y1_write(pstBeReg, pstStaticRegCfg->u16SatuGainY1);
            isp_de_satugain_k_write(pstBeReg, pstStaticRegCfg->u16SatuGainK);
            isp_de_edge_scale_x0_write(pstBeReg, pstStaticRegCfg->u16EdgeScaleX0 );
            isp_de_edge_scale_x1_write(pstBeReg, pstStaticRegCfg->u16EdgeScaleX1 );
            isp_de_edge_scale_y0_write(pstBeReg, pstStaticRegCfg->u16EdgeScaleY0 );
            isp_de_edge_scale_y1_write(pstBeReg, pstStaticRegCfg->u16EdgeScaleY1 );
            isp_de_edge_scale_k_write(pstBeReg, pstStaticRegCfg->u16EdgeScaleK );
            isp_de_global_ratio_write(pstBeReg, pstStaticRegCfg->u16GlobalRatio);
            isp_de_sgm_mode_write(pstBeReg, pstStaticRegCfg->bSgmMode);
            isp_de_edge_mode_write(pstBeReg, pstStaticRegCfg->bEdgeMode);
            isp_de_lumagain_mode_write(pstBeReg, pstStaticRegCfg->bLumaGainMode);
            isp_de_lumabit_mode_write(pstBeReg, pstStaticRegCfg->u8LumaBitMode);

            pstStaticRegCfg->bResh = HI_FALSE;
        }

        if (pstDynaRegCfg->bResh)
        {
            isp_de_sgm_pos4_gain_write(pstBeReg, pstDynaRegCfg->u8SgmPos4Gain );
            isp_de_sgm_pos3_gain_write(pstBeReg, pstDynaRegCfg->u8SgmPos3Gain );
            isp_de_sgm_pos2_gain_write(pstBeReg, pstDynaRegCfg->u8SgmPos2Gain );
            isp_de_sgm_neg4_gain_write(pstBeReg, pstDynaRegCfg->u8SgmNeg4Gain );
            isp_de_sgm_neg3_gain_write(pstBeReg, pstDynaRegCfg->u8SgmNeg3Gain );
            isp_de_sgm_neg2_gain_write(pstBeReg, pstDynaRegCfg->u8SgmNeg2Gain );
            isp_de_gain_lf_write(pstBeReg, pstDynaRegCfg->u16GainLF      );
            isp_de_gain_hf_pos_write(pstBeReg, pstDynaRegCfg->u16GainHFPos   );
            isp_de_gain_hf_neg_write(pstBeReg, pstDynaRegCfg->u16GainHFNeg   );

            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        if (pstUsrRegCfg->bResh)
        {
            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                isp_de_luma_gain_waddr_write(pstBeReg, 0);
                for (j = 0; j < HI_ISP_DE_LUMA_GAIN_LUT_N; j += 1)
                {
                    isp_de_luma_gain_wdata_write(pstBeReg, pstUsrRegCfg->au16LumaGainLut[j]);
                }

                bLutUpdate = HI_TRUE;
            }
            else
            {
                isp_de_luma_gain_write(pstBeReg, pstUsrRegCfg->au16LumaGainLut);
            }
            pstUsrRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1DetailCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg.bDeLutUpdate = bLutUpdate | bIsOfflineMode;

    return HI_SUCCESS;
}

static HI_S32 ISP_DgRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_U8  u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_BOOL  bIsOfflineMode;
    ISP_DG_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_DG_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    S_VIPROC_REGS_TYPE  *pstViProcReg    = HI_NULL;
    S_ISPBE_REGS_TYPE   *pstBeReg        = HI_NULL;
    ISP_CTX_S           *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1DgCfg)
    {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);
        isp_dg_en_write(pstViProcReg, pstRegCfgInfo->stAlgRegCfg[i].stDgRegCfg.bDgEn);

        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stDgRegCfg.stStaticRegCfg;
        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stDgRegCfg.stDynaRegCfg;

        if (pstStaticRegCfg->bResh)
        {
            isp_dg_rgain_write(pstBeReg,  pstDynaRegCfg->u16GainR);
            isp_dg_grgain_write(pstBeReg, pstDynaRegCfg->u16GainGR);
            isp_dg_gbgain_write(pstBeReg, pstDynaRegCfg->u16GainGB);
            isp_dg_bgain_write(pstBeReg,  pstDynaRegCfg->u16GainB);

            pstStaticRegCfg->bResh = HI_FALSE;
        }

        if (pstDynaRegCfg->bResh)
        {
            isp_dg_clip_value_write(pstBeReg, pstDynaRegCfg->u32ClipValue);
            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1DgCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_4DgRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_BOOL  bIsOfflineMode;
    ISP_4DG_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_4DG_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    ISP_CTX_S            *pstIspCtx       = HI_NULL;
    S_ISPBE_REGS_TYPE    *pstBeReg        = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1WDRDgCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        //isp_4dg_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].st4DgRegCfg.bEnable);

        //static
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].st4DgRegCfg.stStaticRegCfg;

        if (pstStaticRegCfg->bResh)
        {
            isp_4dg0_rgain_write(pstBeReg, pstStaticRegCfg->u16GainR0);
            isp_4dg0_grgain_write(pstBeReg, pstStaticRegCfg->u16GainGR0);
            isp_4dg0_gbgain_write(pstBeReg, pstStaticRegCfg->u16GainGB0);
            isp_4dg0_bgain_write(pstBeReg, pstStaticRegCfg->u16GainB0);
            isp_4dg1_rgain_write(pstBeReg, pstStaticRegCfg->u16GainR1);
            isp_4dg1_grgain_write(pstBeReg, pstStaticRegCfg->u16GainGR1);
            isp_4dg1_gbgain_write(pstBeReg, pstStaticRegCfg->u16GainGB1);
            isp_4dg1_bgain_write(pstBeReg, pstStaticRegCfg->u16GainB1);
            pstStaticRegCfg->bResh = HI_FALSE;
        }

        //dynamic
        pstDynaRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].st4DgRegCfg.stDynaRegCfg;

        if (pstDynaRegCfg->bResh)
        {
            isp_4dg0_clip_value_write(pstBeReg, pstDynaRegCfg->u32ClipValue0);
            isp_4dg1_clip_value_write(pstBeReg, pstDynaRegCfg->u32ClipValue1);

            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1WDRDgCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_FlickRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL bIsOfflineMode;
    HI_U8   u8BlkNum = pstRegCfgInfo->u8CfgNum;
    S_VIPROC_REGS_TYPE       *pstViProcReg    = HI_NULL;
    ISP_FLICKER_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    ISP_CTX_S                *pstIspCtx       = HI_NULL;
    S_ISPBE_REGS_TYPE        *pstBeReg        = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1FlickerCfg)
    {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);

        isp_flicker_en_write(pstViProcReg, pstRegCfgInfo->stAlgRegCfg[i].stFlickerRegCfg.bEnable);

        pstDynaRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stFlickerRegCfg.stDynaRegCfg;

        if (pstDynaRegCfg->bResh)
        {
            isp_flick_overth_write(pstBeReg, pstDynaRegCfg->u16OverThr);
            isp_flick_gravg_pre_write(pstBeReg, pstDynaRegCfg->s16GrAvgPre);
            isp_flick_gbavg_pre_write(pstBeReg, pstDynaRegCfg->s16GbAvgPre);

            pstDynaRegCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1FlickerCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_PreGammaRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
#ifdef CONFIG_HI_ISP_PREGAMMA_SUPPORT
    HI_BOOL bIsOfflineMode;
    HI_BOOL bUsrResh       = HI_FALSE;
    HI_BOOL bIdxResh       = HI_FALSE;
    HI_BOOL bStt2LutRegnew = HI_FALSE;
    HI_U8   u8BlkNum       = pstRegCfgInfo->u8CfgNum;
    HI_U8   u8BufId;
    ISP_PREGAMMA_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;
    ISP_PREGAMMA_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    S_ISP_LUT_WSTT_TYPE       *pstBeLutSttReg  = HI_NULL;
    S_VIPROC_REGS_TYPE        *pstViProcReg    = HI_NULL;
    S_ISPBE_REGS_TYPE         *pstBeReg        = HI_NULL;
    ISP_CTX_S                 *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1PreGammaCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);

        //isp_pregamma_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stPreGammaCfg.bPreGammaEn);

        pstDynaRegCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stPreGammaCfg.stDynaRegCfg;
        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stPreGammaCfg.stStaticRegCfg;

        //Enable PreGamma
        if (pstStaticRegCfg->bStaticResh)
        {
            isp_pregamma_idxbase_write(pstBeReg, pstStaticRegCfg->au8SegIdxBase);
            isp_pregamma_maxval_write(pstBeReg, pstStaticRegCfg->au8SegMaxVal);
            isp_pregamma_lut_width_word_write(pstViProcReg, HI_ISP_PREGAMMA_LUT_WIDTH_WORD_DEFAULT);
            pstStaticRegCfg->bStaticResh = HI_FALSE;
        }

        bIdxResh = (isp_pregamma_update_index_read(pstBeReg) != pstDynaRegCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstDynaRegCfg->bPreGammaLutUpdateEn & bIdxResh) : (pstDynaRegCfg->bPreGammaLutUpdateEn);

        //LUT update
        if (bUsrResh)
        {
            isp_pregamma_update_index_write(pstBeReg, pstDynaRegCfg->u32UpdateIndex);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                if (HI_TRUE != pstDynaRegCfg->bSwitchMode)
                {
                    /*online Lut2stt regconfig*/
                    u8BufId = pstDynaRegCfg->u8BufId;

                    pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, u8BufId);
                    ISP_CHECK_POINTER(pstBeLutSttReg);

                    isp_pregamma_lut_wstt_write(pstBeLutSttReg, pstDynaRegCfg->u32PreGammaLUT);
                    isp_pregamma_lut_wstt_addr_write(ViPipe, i, u8BufId, pstViProcReg);
                    isp_pregamma_stt2lut_en_write(pstBeReg, HI_TRUE);
                    //isp_pregamma_stt2lut_regnew_write(pstBeReg, HI_TRUE);
                    pstDynaRegCfg->u8BufId = 1 - u8BufId;
                    bStt2LutRegnew = HI_TRUE;
                }
            }
            else
            {
                isp_pregamma_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstDynaRegCfg->u32PreGammaLUT);
                isp_pregamma_stt2lut_en_write(pstBeReg, HI_TRUE);
                isp_pregamma_stt2lut_regnew_write(pstBeReg, HI_TRUE);
            }

            pstDynaRegCfg->bPreGammaLutUpdateEn = bIsOfflineMode;
            pstDynaRegCfg->bSwitchMode          = HI_FALSE;
        }

        pstRegCfgInfo->unKey.bit1PreGammaCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stStt2LutRegnewCfg.bPreGammaStt2LutRegnew = bStt2LutRegnew;

#endif
    return HI_SUCCESS;
}

static HI_S32 ISP_BeBlcRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL bIsOfflineMode;
    HI_U8   u8BlkNum = pstRegCfgInfo->u8CfgNum;
    ISP_BE_BLC_CFG_S  *pstBeBlcCfg = HI_NULL;
    ISP_CTX_S         *pstIspCtx   = HI_NULL;
    S_ISPBE_REGS_TYPE *pstBeReg    = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1BeBlcCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);

        pstBeBlcCfg = &pstRegCfgInfo->stAlgRegCfg[i].stBeBlcCfg;

        if (pstBeBlcCfg->bReshStatic)
        {
            /*4Dg*/
            isp_4dg_en_in_write(pstBeReg, pstBeBlcCfg->st4DgBlc[0].stStaticRegCfg.bBlcIn);
            isp_4dg_en_out_write(pstBeReg, pstBeBlcCfg->st4DgBlc[0].stStaticRegCfg.bBlcOut);

            /*WDR*/
            isp_wdr_bsaveblc_write(pstBeReg, pstBeBlcCfg->stWdrBlc[0].stStaticRegCfg.bBlcOut);

            /*lsc*/
            isp_lsc_blc_in_en_write(pstBeReg, pstBeBlcCfg->stLscBlc.stStaticRegCfg.bBlcIn);
            isp_lsc_blc_out_en_write(pstBeReg, pstBeBlcCfg->stLscBlc.stStaticRegCfg.bBlcOut);
            /*Dg*/
            isp_dg_en_in_write(pstBeReg, pstBeBlcCfg->stDgBlc.stStaticRegCfg.bBlcIn);
            isp_dg_en_out_write(pstBeReg, pstBeBlcCfg->stDgBlc.stStaticRegCfg.bBlcOut);
            /*AE*/
            isp_ae_blc_en_write(pstBeReg, pstBeBlcCfg->stAeBlc.stStaticRegCfg.bBlcIn);
            /*MG*/
            isp_la_blc_en_write(pstBeReg, pstBeBlcCfg->stMgBlc.stStaticRegCfg.bBlcIn);
            /*WB*/
            isp_wb_en_in_write(pstBeReg, pstBeBlcCfg->stWbBlc.stStaticRegCfg.bBlcIn);
            isp_wb_en_out_write(pstBeReg, pstBeBlcCfg->stWbBlc.stStaticRegCfg.bBlcOut);

            pstBeBlcCfg->bReshStatic = bIsOfflineMode;
        }

        if (pstBeBlcCfg->bReshDyna)
        {
            /*4Dg*/
            isp_4dg0_ofsr_write(pstBeReg, pstBeBlcCfg->st4DgBlc[0].stUsrRegCfg.au16Blc[0]);
            isp_4dg0_ofsgr_write(pstBeReg, pstBeBlcCfg->st4DgBlc[0].stUsrRegCfg.au16Blc[1]);
            isp_4dg0_ofsgb_write(pstBeReg, pstBeBlcCfg->st4DgBlc[0].stUsrRegCfg.au16Blc[2]);
            isp_4dg0_ofsb_write(pstBeReg, pstBeBlcCfg->st4DgBlc[0].stUsrRegCfg.au16Blc[3]);

            isp_4dg1_ofsr_write(pstBeReg, pstBeBlcCfg->st4DgBlc[1].stUsrRegCfg.au16Blc[0]);
            isp_4dg1_ofsgr_write(pstBeReg, pstBeBlcCfg->st4DgBlc[1].stUsrRegCfg.au16Blc[1]);
            isp_4dg1_ofsgb_write(pstBeReg, pstBeBlcCfg->st4DgBlc[1].stUsrRegCfg.au16Blc[2]);
            isp_4dg1_ofsb_write(pstBeReg, pstBeBlcCfg->st4DgBlc[1].stUsrRegCfg.au16Blc[3]);

            /*WDR*/
            isp_wdr_outblc_write(pstBeReg, pstBeBlcCfg->stWdrBlc[0].stUsrRegCfg.u16OutBlc);
            isp_wdr_f0_inblc_r_write(pstBeReg, pstBeBlcCfg->stWdrBlc[0].stUsrRegCfg.au16Blc[0]);
            isp_wdr_f0_inblc_gr_write(pstBeReg, pstBeBlcCfg->stWdrBlc[0].stUsrRegCfg.au16Blc[1]);
            isp_wdr_f0_inblc_gb_write(pstBeReg, pstBeBlcCfg->stWdrBlc[0].stUsrRegCfg.au16Blc[2]);
            isp_wdr_f0_inblc_b_write(pstBeReg, pstBeBlcCfg->stWdrBlc[0].stUsrRegCfg.au16Blc[3]);

            isp_wdr_f1_inblc_r_write(pstBeReg, pstBeBlcCfg->stWdrBlc[1].stUsrRegCfg.au16Blc[0]);
            isp_wdr_f1_inblc_gr_write(pstBeReg, pstBeBlcCfg->stWdrBlc[1].stUsrRegCfg.au16Blc[1]);
            isp_wdr_f1_inblc_gb_write(pstBeReg, pstBeBlcCfg->stWdrBlc[1].stUsrRegCfg.au16Blc[2]);
            isp_wdr_f1_inblc_b_write(pstBeReg, pstBeBlcCfg->stWdrBlc[1].stUsrRegCfg.au16Blc[3]);

            /*flicker*/
            isp_flick_f0_inblc_r_write(pstBeReg, pstBeBlcCfg->stFlickerBlc[0].stUsrRegCfg.au16Blc[0]);
            isp_flick_f0_inblc_gr_write(pstBeReg, pstBeBlcCfg->stFlickerBlc[0].stUsrRegCfg.au16Blc[1]);
            isp_flick_f0_inblc_gb_write(pstBeReg, pstBeBlcCfg->stFlickerBlc[0].stUsrRegCfg.au16Blc[2]);
            isp_flick_f0_inblc_b_write(pstBeReg, pstBeBlcCfg->stFlickerBlc[0].stUsrRegCfg.au16Blc[3]);

            isp_flick_f1_inblc_r_write(pstBeReg, pstBeBlcCfg->stFlickerBlc[1].stUsrRegCfg.au16Blc[0]);
            isp_flick_f1_inblc_gr_write(pstBeReg, pstBeBlcCfg->stFlickerBlc[1].stUsrRegCfg.au16Blc[1]);
            isp_flick_f1_inblc_gb_write(pstBeReg, pstBeBlcCfg->stFlickerBlc[1].stUsrRegCfg.au16Blc[2]);
            isp_flick_f1_inblc_b_write(pstBeReg, pstBeBlcCfg->stFlickerBlc[1].stUsrRegCfg.au16Blc[3]);

            /*expander*/
            isp_expander_offset_r_write(pstBeReg, pstBeBlcCfg->stExpanderBlc.stUsrRegCfg.au16Blc[0]);
            isp_expander_offset_gr_write(pstBeReg, pstBeBlcCfg->stExpanderBlc.stUsrRegCfg.au16Blc[1]);
            isp_expander_offset_gb_write(pstBeReg, pstBeBlcCfg->stExpanderBlc.stUsrRegCfg.au16Blc[2]);
            isp_expander_offset_b_write(pstBeReg, pstBeBlcCfg->stExpanderBlc.stUsrRegCfg.au16Blc[3]);

            /*bnr*/
            isp_bnr_rlmt_blc_write(pstBeReg, (pstBeBlcCfg->stBnrBlc.stUsrRegCfg.au16Blc[0] >> 2));

            /*lsc*/
            isp_lsc_blc_r_write(pstBeReg, pstBeBlcCfg->stLscBlc.stUsrRegCfg.au16Blc[0]);
            isp_lsc_blc_gr_write(pstBeReg, pstBeBlcCfg->stLscBlc.stUsrRegCfg.au16Blc[1]);
            isp_lsc_blc_gb_write(pstBeReg, pstBeBlcCfg->stLscBlc.stUsrRegCfg.au16Blc[2]);
            isp_lsc_blc_b_write(pstBeReg, pstBeBlcCfg->stLscBlc.stUsrRegCfg.au16Blc[3]);

            /*Dg*/
            isp_dg_ofsr_write(pstBeReg, pstBeBlcCfg->stDgBlc.stUsrRegCfg.au16Blc[0]);
            isp_dg_ofsgr_write(pstBeReg, pstBeBlcCfg->stDgBlc.stUsrRegCfg.au16Blc[1]);
            isp_dg_ofsgb_write(pstBeReg, pstBeBlcCfg->stDgBlc.stUsrRegCfg.au16Blc[2]);
            isp_dg_ofsb_write(pstBeReg, pstBeBlcCfg->stDgBlc.stUsrRegCfg.au16Blc[3]);

            /*AE*/
            isp_ae_offset_r_write(pstBeReg, pstBeBlcCfg->stAeBlc.stUsrRegCfg.au16Blc[0]);
            isp_ae_offset_gr_write(pstBeReg, pstBeBlcCfg->stAeBlc.stUsrRegCfg.au16Blc[1]);
            isp_ae_offset_gb_write(pstBeReg, pstBeBlcCfg->stAeBlc.stUsrRegCfg.au16Blc[2]);
            isp_ae_offset_b_write(pstBeReg, pstBeBlcCfg->stAeBlc.stUsrRegCfg.au16Blc[3]);
            /*MG*/
            isp_la_offset_r_write(pstBeReg, pstBeBlcCfg->stMgBlc.stUsrRegCfg.au16Blc[0]);
            isp_la_offset_gr_write(pstBeReg, pstBeBlcCfg->stMgBlc.stUsrRegCfg.au16Blc[1]);
            isp_la_offset_gb_write(pstBeReg, pstBeBlcCfg->stMgBlc.stUsrRegCfg.au16Blc[2]);
            isp_la_offset_b_write(pstBeReg, pstBeBlcCfg->stMgBlc.stUsrRegCfg.au16Blc[3]);
            /*WB*/
            isp_wb_ofsr_write (pstBeReg, pstBeBlcCfg->stWbBlc.stUsrRegCfg.au16Blc[0]);
            isp_wb_ofsgr_write(pstBeReg, pstBeBlcCfg->stWbBlc.stUsrRegCfg.au16Blc[1]);
            isp_wb_ofsgb_write(pstBeReg, pstBeBlcCfg->stWbBlc.stUsrRegCfg.au16Blc[2]);
            isp_wb_ofsb_write (pstBeReg, pstBeBlcCfg->stWbBlc.stUsrRegCfg.au16Blc[3]);

            //DE
            isp_bnr_de_blcvalue_write(pstBeReg, pstBeBlcCfg->stDeBlc.stUsrRegCfg.au16Blc[0]);

            pstBeBlcCfg->bReshDyna = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1BeBlcCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_ClutRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bIsOfflineMode;
    HI_BOOL  bUsrResh  = HI_FALSE;
    HI_BOOL  bIdxResh  = HI_FALSE;
    HI_BOOL  bStt2LutRegnew = HI_FALSE;
    HI_U8    u8BufId;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    ISP_CLUT_USR_COEF_CFG_S     *pstClutUsrCoefCfg   = HI_NULL;
    S_ISP_LUT_WSTT_TYPE         *pstBeLutSttReg      = HI_NULL;
    S_VIPROC_REGS_TYPE          *pstViProcReg        = HI_NULL;
    S_ISPBE_REGS_TYPE           *pstBeReg            = HI_NULL;
    ISP_CTX_S                   *pstIspCtx           = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if ((pstRegCfgInfo->unKey.bit1ClutCfg))
    {
        pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        ISP_CHECK_POINTER(pstViProcReg);

        //isp_clut_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stClutCfg.bEnable);
        isp_clut_sel_write(pstBeReg, HI_ISP_CLUT_SEL_WRITE);
        pstClutUsrCoefCfg   = &pstRegCfgInfo->stAlgRegCfg[i].stClutCfg.stUsrRegCfg.stClutUsrCoefCfg;

        bIdxResh = (isp_clut_update_index_read(pstBeReg) != pstClutUsrCoefCfg->u32UpdateIndex);
        bUsrResh = (bIsOfflineMode) ? (pstClutUsrCoefCfg->bResh & bIdxResh) : (pstClutUsrCoefCfg->bResh);

        if (bUsrResh)
        {
            isp_clut_update_index_write(pstBeReg, pstClutUsrCoefCfg->u32UpdateIndex);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)\
                || IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                /*online Lut2stt regconfig*/
                u8BufId = pstClutUsrCoefCfg->u8BufId;

                pstBeLutSttReg = (S_ISP_LUT_WSTT_TYPE *)ISP_GetBeLut2SttVirAddr(ViPipe, i, u8BufId);
                ISP_CHECK_POINTER(pstBeLutSttReg);

                isp_clut_lut_wstt_write(pstBeLutSttReg, pstClutUsrCoefCfg->au32LuStt);

                isp_clut_lut_wstt_addr_write(ViPipe, i, u8BufId, pstViProcReg);
                isp_clut_stt2lut_en_write(pstBeReg, HI_TRUE);
                //isp_clut_stt2lut_regnew_write(pstBeReg, HI_TRUE);

                pstClutUsrCoefCfg->u8BufId = 1 - u8BufId;
                bStt2LutRegnew = HI_TRUE;
            }
            else
            {
                isp_clut_lut_wstt_write(&pstBeReg->stIspBeLut.stBeLut2Stt, pstClutUsrCoefCfg->au32LuStt);
                isp_clut_lut_width_word_write(pstViProcReg, HI_ISP_CLUT_LUT_WIDTH_WORD_DEFAULT);
                isp_clut_stt2lut_en_write(pstBeReg, HI_TRUE);
                isp_clut_stt2lut_regnew_write(pstBeReg, HI_TRUE);
            }

            //isp_clut_lut_update_write(ViPipe, i, HI_TRUE);
            pstClutUsrCoefCfg->bResh = bIsOfflineMode;
        }

        pstRegCfgInfo->unKey.bit1ClutCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    pstRegCfgInfo->stAlgRegCfg[i].stStt2LutRegnewCfg.bClutStt2LutRegnew = bStt2LutRegnew;

    return HI_SUCCESS;
}

static HI_S32 ISP_ExpanderRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_BOOL  bIsOfflineMode;
    HI_U8    u8BlkNum = pstRegCfgInfo->u8CfgNum;
    HI_U16   j;
    ISP_EXPANDER_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    S_ISPBE_REGS_TYPE         *pstBeReg        = HI_NULL;
    ISP_CTX_S                 *pstIspCtx       = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bIsOfflineMode = (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                      IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode));

    if (pstRegCfgInfo->unKey.bit1ExpanderCfg)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstBeReg);
        //isp_expander_en_write(ViPipe, i, pstRegCfgInfo->stAlgRegCfg[i].stExpanderCfg.bEnable);

        pstStaticRegCfg = &pstRegCfgInfo->stAlgRegCfg[i].stExpanderCfg.stStaticCfg;

        if (pstStaticRegCfg->bResh)
        {
            isp_expander_bitw_out_write(pstBeReg, pstStaticRegCfg->u8BitDepthOut);
            isp_expander_bitw_in_write(pstBeReg, pstStaticRegCfg->u8BitDepthIn);

            if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
                IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
            {
                isp_expander_lut_waddr_write(pstBeReg, 0);

                for ( j = 0 ; j < EXPANDER_NODE_NUM; j++ )
                {
                    isp_expander_lut_wdata_write(pstBeReg, pstStaticRegCfg->au32Lut[j]);
                }
            }
            else
            {
                isp_expander_lut_write(pstBeReg, pstStaticRegCfg->au32Lut);
            }

            pstStaticRegCfg->bResh = HI_FALSE;
        }

        pstRegCfgInfo->unKey.bit1ExpanderCfg = bIsOfflineMode ? 1 : ((u8BlkNum <= (i + 1)) ? 0 : 1);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_FeUpdateRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    HI_U32  i;
    VI_PIPE ViPipeBind;
    ISP_CTX_S         *pstIspCtx = HI_NULL;
    S_ISPFE_REGS_TYPE *pstFeReg  = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    for (i = 0; i < pstIspCtx->stWdrAttr.stDevBindPipe.u32Num; i++)
    {
        ViPipeBind = pstIspCtx->stWdrAttr.stDevBindPipe.PipeId[i];
        ISP_CHECK_PIPE(ViPipeBind);
        pstFeReg = (S_ISPFE_REGS_TYPE *)ISP_GetFeVirAddr(ViPipeBind);
        ISP_CHECK_POINTER(pstFeReg);

        isp_fe_update_mode_write(pstFeReg, HI_FALSE);
        isp_fe_update_write(pstFeReg, HI_TRUE);

        if (pstRegCfgInfo->stAlgRegCfg[0].stFeLutUpdateCfg.bAe1LutUpdate)
        {
            isp_ae1_lut_update_write(pstFeReg, pstRegCfgInfo->stAlgRegCfg[0].stFeLutUpdateCfg.bAe1LutUpdate);
        }
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_FeSystemRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    HI_BOOL bIspCropEn = HI_FALSE;
    HI_U8   u8RggbCfg;
    HI_U32  i;
    HI_S32  s32X, s32Y;
    HI_U32  u32Width, u32Height;
    HI_U32  u32PipeW, u32PipeH;
    VI_PIPE ViPipeBind, ViPipeId;
    ISP_CTX_S         *pstIspCtx = HI_NULL;
    S_ISPFE_REGS_TYPE *pstFeReg  = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    ViPipeId  = pstIspCtx->stWdrAttr.stDevBindPipe.PipeId[0];
    u8RggbCfg = hi_ext_system_rggb_cfg_read(ViPipeId);

    s32X      = pstIspCtx->stSysRect.s32X;
    s32Y      = pstIspCtx->stSysRect.s32Y;
    u32Width  = pstIspCtx->stSysRect.u32Width;
    u32Height = pstIspCtx->stSysRect.u32Height;
    u32PipeW  = pstIspCtx->stPipeSize.u32Width;
    u32PipeH  = pstIspCtx->stPipeSize.u32Height;

    /* ISP crop low-power process */
    if ((0 == s32X) &&
        (0 == s32Y) &&
        (u32Width  == u32PipeW) &&
        (u32Height == u32PipeH))
    {
        bIspCropEn = HI_FALSE;
    }
    else
    {
        bIspCropEn = HI_TRUE;
    }

    for (i = 0; i < pstIspCtx->stWdrAttr.stDevBindPipe.u32Num; i++)
    {
        ViPipeBind = pstIspCtx->stWdrAttr.stDevBindPipe.PipeId[i];
        ISP_CHECK_PIPE(ViPipeBind);
        pstFeReg = (S_ISPFE_REGS_TYPE *)ISP_GetFeVirAddr(ViPipeBind);
        ISP_CHECK_POINTER(pstFeReg);

        /*ISP FE/BE Set Offline Mode*/
        /* isp regs uptate mode:   0: update; 1:frame */
        //isp_fe_update_mode_write(ViPipeBind, HI_TRUE);
        isp_fe_crop_en_write(pstFeReg, bIspCropEn);
        isp_crop_pos_write(pstFeReg, s32X, s32Y);
        isp_crop_size_out_write(pstFeReg, u32Width - 1, u32Height - 1);
        isp_fe_rggb_cfg_write(pstFeReg, u8RggbCfg);
        isp_fe_fix_timing_write(pstFeReg, HI_ISP_FE_FIX_TIMING_STAT);
        isp_fe_size_write(pstFeReg, u32PipeW  - 1, u32PipeH - 1);
        isp_fe_blk_size_write(pstFeReg, u32PipeW  - 1, u32PipeH - 1);
        isp_fe_blk_f_hblank_write(pstFeReg, 0);
        isp_fe_sync_mode_write(pstFeReg, 0, 0);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_RegDefault(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    S_ISPBE_REGS_TYPE  *pstBeReg     = HI_NULL;
    S_VIPROC_REGS_TYPE *pstViProcReg = HI_NULL;

    pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
    pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
    ISP_CHECK_POINTER(pstBeReg);
    ISP_CHECK_POINTER(pstViProcReg);

    isp_clip_y_min_write(pstBeReg, ISP_CLIP_Y_MIN_DEFAULT);
    isp_clip_y_max_write(pstBeReg, ISP_CLIP_Y_MAX_DEFAULT);
    isp_clip_c_min_write(pstBeReg, ISP_CLIP_C_MIN_DEFAULT);
    isp_clip_c_max_write(pstBeReg, ISP_CLIP_C_MAX_DEFAULT);
    isp_csc_sum_en_write(pstBeReg, ISP_CSC_SUM_EN_DEFAULT);
    isp_yuv444_sum_en_write(pstBeReg, ISP_YUV444_SUM_EN_DEFAULT);
    isp_yuv422_sum_en_write(pstBeReg, ISP_YUV422_SUM_EN_DEFAULT);
    isp_wdr_sum_en_write(pstBeReg, ISP_WDR_SUM_EN_DEFAULT);
    isp_demosaic_sum_en_write(pstBeReg, ISP_DEMOSAIC_SUM_EN_DEFAULT);

    isp_blk_f_hblank_write(pstBeReg, HI_ISP_BLK_F_HBLANK_DEFAULT);
    isp_blk_f_vblank_write(pstBeReg, HI_ISP_BLK_F_VBLANK_DEFAULT);
    isp_blk_b_hblank_write(pstBeReg, HI_ISP_BLK_B_HBLANK_DEFAULT);
    isp_blk_b_vblank_write(pstBeReg, HI_ISP_BLK_B_VBLANK_DEFAULT);
    /*SDR*/
    isp_sqrt_en_write(pstViProcReg, HI_FALSE);//sqrt in raw
    isp_sq_en_write(pstViProcReg, HI_FALSE);//sq in rgb
    isp_sqrt1_en_write(pstViProcReg, HI_FALSE);//sqrt in rgb

    isp_blc_en_write(pstViProcReg, HI_FALSE);
    isp_split_en_write(pstViProcReg, HI_FALSE);

    return HI_SUCCESS;
}

static HI_S32 ISP_SystemRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    //HI_BOOL bMcdsVEn;
    HI_U32  u32RggbCfg;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    S_VIPROC_REGS_TYPE *pstViProcReg = HI_NULL;
    S_ISPBE_REGS_TYPE  *pstBeReg     = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    u32RggbCfg = hi_ext_system_rggb_cfg_read(ViPipe);
    //bMcdsVEn = pstRegCfgInfo->stAlgRegCfg[i].stMcdsRegCfg.stDynaRegCfg.bVcdsEn;

    pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
    pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
    ISP_CHECK_POINTER(pstBeReg);
    ISP_CHECK_POINTER(pstViProcReg);

    //isp_be_en_write(pstBeReg, HI_TRUE);

    isp_be_rggb_cfg_write(pstViProcReg, u32RggbCfg);
    //isp_format_write(pstBeReg, !bMcdsVEn); /* ISP Be YUV format */

    if ((IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)) ||
        (IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)))
    {
        isp_stt_en_write(pstBeReg, HI_TRUE);
    }
    else
    {
        isp_stt_en_write(pstBeReg, HI_FALSE);
    }

    if (1 == pstRegCfgInfo->stAlgRegCfg[i].stFpnRegCfg.stDynaRegCfg.u32IspFpnCalibCorr) //fpn calib mode
    {
        isp_sumy_en_write(pstViProcReg, HI_FALSE);
    }
    else
    {
        isp_sumy_en_write(pstViProcReg, HI_TRUE);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_DitherRegConfig (VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    S_ISPBE_REGS_TYPE *pstBeReg = HI_NULL;

    pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
    ISP_CHECK_POINTER(pstBeReg);
    /*after drc module*/
    isp_drc_dither_out_bits_write(pstBeReg, HI_ISP_DRC_DITHER_OUT_BITS_DEFAULT);
    isp_drc_dither_round_write(pstBeReg, HI_ISP_DRC_DITHER_ROUND_DEFAULT);
    isp_drc_dither_spatial_mode_write(pstBeReg, HI_ISP_DRC_DITHER_SPATIAL_MODE_DEFAULT);

    /*after gamma module*/
    isp_dmnr_dither_en_write(pstBeReg, HI_TRUE);
    isp_dmnr_dither_out_bits_write(pstBeReg, HI_ISP_DMNR_DITHER_OUT_BITS_DEFAULT);
    isp_dmnr_dither_round_write(pstBeReg, HI_ISP_DMNR_DITHER_ROUND_DEFAULT);
    isp_dmnr_dither_spatial_mode_write(pstBeReg, HI_ISP_DMNR_DITHER_SPATIAL_MODE_DEFAULT);

    /*after CA module*/
    isp_acm_dither_en_write(pstBeReg, HI_TRUE);
    isp_acm_dither_out_bits_write(pstBeReg, HI_ISP_ACM_DITHER_OUT_BITS_DEFAULT);
    isp_acm_dither_round_write(pstBeReg, HI_ISP_ACM_DITHER_ROUND_DEFAULT);
    isp_acm_dither_spatial_mode_write(pstBeReg, HI_ISP_ACM_DITHER_SPATIAL_MODE_DEFAULT);

    isp_sqrt1_dither_en_write(pstBeReg, HI_FALSE);

    return HI_SUCCESS;
}

static HI_S32 ISP_FeSttRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    HI_U16  u16NumH, u16NumV;
    HI_U32  k;
    HI_BOOL bStt = HI_TRUE;
    VI_PIPE ViPipeBind;
    ISP_CTX_S         *pstIspCtx = HI_NULL;
    S_ISPFE_REGS_TYPE *pstFeReg  = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    for (k = 0; k < pstIspCtx->stWdrAttr.stDevBindPipe.u32Num; k++)
    {
        ViPipeBind = pstIspCtx->stWdrAttr.stDevBindPipe.PipeId[k];
        ISP_CHECK_PIPE(ViPipeBind);
        pstFeReg = (S_ISPFE_REGS_TYPE *)ISP_GetFeVirAddr(ViPipeBind);
        ISP_CHECK_POINTER(pstFeReg);

        if (pstIspCtx->stSpecialOpt.bFeSttUpdate)
        {
            isp_ae1_stt_en_write(pstFeReg, bStt);
            isp_ae1_stt_bst_write(pstFeReg, 0xF);
        }

        /*ae*/
        u16NumH = pstRegCfgInfo->stAlgRegCfg[0].stAeRegCfg.stDynaRegCfg.u8FEWeightTableWidth;
        u16NumV = pstRegCfgInfo->stAlgRegCfg[0].stAeRegCfg.stDynaRegCfg.u8FEWeightTableHeight;
        isp_ae1_stt_size_write(pstFeReg, (u16NumH * u16NumV + 3) / 4);
    }

    pstIspCtx->stSpecialOpt.bFeSttUpdate = HI_FALSE;

    return HI_SUCCESS;
}

static HI_S32 ISP_OnlineSttRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_U16 u16Height;
    HI_U16 u16NumH, u16NumV, u16BinNum;
    HI_BOOL bStt = HI_TRUE;
    ISP_CTX_S         *pstIspCtx = HI_NULL;
    S_ISPBE_REGS_TYPE *pstBeReg  = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if ((IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)) ||
        (IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)))
    {
        return HI_SUCCESS;
    }

    pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
    ISP_CHECK_POINTER(pstBeReg);

    if (pstIspCtx->stSpecialOpt.abBeOnSttUpdate[i])
    {
        isp_ae_stt_en_write(pstBeReg, bStt);
        isp_ae_stt_bst_write(pstBeReg, 0xF);

        isp_la_stt_en_write(pstBeReg, bStt);
        isp_la_stt_bst_write(pstBeReg, 0xF);

        isp_awb_stt_en_write(pstBeReg, bStt);
        isp_awb_stt_bst_write(pstBeReg, 0xF);

        isp_af_stt_en_write(pstBeReg, bStt);
        isp_af_stt_bst_write(pstBeReg, 0xF);

        isp_dehaze_stt_en_write(pstBeReg, bStt);
        isp_dehaze_stt_bst_write(pstBeReg, 0xF);

        isp_ldci_lpfstt_en_write(pstBeReg, bStt);
        isp_ldci_lpfstt_bst_write(pstBeReg, 0xF);

        isp_flick_stt_en_write(pstBeReg, bStt);
        isp_flick_stt_bst_write(pstBeReg, 0xF);

        pstIspCtx->stSpecialOpt.abBeOnSttUpdate[i] = HI_FALSE;
    }

    /*AE*/
    u16NumH = pstRegCfgInfo->stAlgRegCfg[i].stAeRegCfg.stDynaRegCfg.u8BEWeightTableWidth;
    u16NumV = pstRegCfgInfo->stAlgRegCfg[i].stAeRegCfg.stDynaRegCfg.u8BEWeightTableHeight;
    isp_ae_stt_size_write(pstBeReg, (u16NumH * u16NumV + 3) / 4);

    /*MG*/
    u16NumH = pstRegCfgInfo->stAlgRegCfg[i].stMgRegCfg.stDynaRegCfg.u8ZoneWidth;
    u16NumV = pstRegCfgInfo->stAlgRegCfg[i].stMgRegCfg.stDynaRegCfg.u8ZoneHeight;
    isp_la_stt_size_write(pstBeReg, (u16NumH * u16NumV + 3 ) / 4);

    /*AWB*/
    u16NumH = pstRegCfgInfo->stAlgRegCfg[i].stAwbRegCfg.stAwbRegUsrCfg.u16BEZoneCol;
    u16NumV = pstRegCfgInfo->stAlgRegCfg[i].stAwbRegCfg.stAwbRegUsrCfg.u16BEZoneRow;
    u16BinNum = 1;//pstRegCfgInfo->stAlgRegCfg[i].stAwbRegCfg.stAwbRegUsrCfg.u16BEZoneBin;
    isp_awb_stt_size_write(pstBeReg, (u16NumH * u16NumV * u16BinNum * 2  + 3 ) / 4);

    /*AF*/
    u16NumH = pstRegCfgInfo->stAlgRegCfg[i].stBEAfRegCfg.u16WindowHnum;
    u16NumV = pstRegCfgInfo->stAlgRegCfg[i].stBEAfRegCfg.u16WindowVnum;
    isp_af_stt_size_write(pstBeReg, (u16NumH * u16NumV * 4 + 3 ) / 4);

    /*Dehaze*/
    u16NumH = pstRegCfgInfo->stAlgRegCfg[i].stDehazeRegCfg.stStaticRegCfg.u8Dchnum;
    u16NumV = pstRegCfgInfo->stAlgRegCfg[i].stDehazeRegCfg.stStaticRegCfg.u8Dcvnum;
    isp_dehaze_stt_size_write(pstBeReg, (((u16NumH + 1) * (u16NumV + 1)) + 3) / 4);

    /*Ldci*/
    u16NumH = pstRegCfgInfo->stAlgRegCfg[i].stLdciRegCfg.stDynaRegCfg.u32StatSmlMapWidth;
    u16NumV = pstRegCfgInfo->stAlgRegCfg[i].stLdciRegCfg.stDynaRegCfg.u32StatSmlMapHeight;
    isp_ldci_lpfstt_size_write(pstBeReg, (u16NumH * u16NumV + 3 ) / 4);

    /*Flicker*/
    u16Height = pstIspCtx->stBlockAttr.stFrameRect.u32Height;
    isp_flick_stt_size_write(pstBeReg, (u16Height / 16 + 3) / 4);

    return HI_SUCCESS;
}

static HI_S32 ISP_BeAlgLut2SttRegnewRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    ISP_BE_STT2LUT_REGNEW_REG_CFG_S  *pstBeStt2LutRegnewCfg = &pstRegCfgInfo->stAlgRegCfg[i].stStt2LutRegnewCfg;
    S_ISPBE_REGS_TYPE                *pstBeReg              = HI_NULL;
    ISP_CTX_S                        *pstIspCtx             = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
    ISP_CHECK_POINTER(pstBeReg);

    if ((pstIspCtx->u32FrameCnt < 3) || pstBeStt2LutRegnewCfg->bGammaStt2LutRegnew)
    {
        isp_gamma_stt2lut_regnew_write(pstBeReg, HI_TRUE);
    }

    if ((pstIspCtx->u32FrameCnt < 3) || pstBeStt2LutRegnewCfg->bLscStt2LutRegnew)
    {
        isp_lsc_stt2lut_regnew_write(pstBeReg, HI_TRUE);
    }

    if ((pstIspCtx->u32FrameCnt < 3) || pstBeStt2LutRegnewCfg->bBnrLscStt2LutRegnew)
    {
        isp_bnr_stt2lut_regnew_write(pstBeReg, HI_TRUE);
    }

    if ((pstIspCtx->u32FrameCnt < 3) || pstBeStt2LutRegnewCfg->bCaStt2LutRegnew)
    {
        isp_ca_stt2lut_regnew_write(pstBeReg, HI_TRUE);
    }

    if ((pstIspCtx->u32FrameCnt < 3) || pstBeStt2LutRegnewCfg->bClutStt2LutRegnew)
    {
        isp_clut_stt2lut_regnew_write(pstBeReg, HI_TRUE);
    }

    if ((pstIspCtx->u32FrameCnt < 3) || pstBeStt2LutRegnewCfg->bDpcStt2LutRegnew)
    {
        isp_dpc_stt2lut_regnew_write(pstBeReg, HI_TRUE);
    }

    if ((pstIspCtx->u32FrameCnt < 3) || pstBeStt2LutRegnewCfg->bPreGammaStt2LutRegnew)
    {
        isp_pregamma_stt2lut_regnew_write(pstBeReg, HI_TRUE);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_BeAlgLutUpdateRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    ISP_BE_LUT_UPDATE_REG_CFG_S  *pstBeLutUpdateCfg = &pstRegCfgInfo->stAlgRegCfg[i].stBeLutUpdateCfg;
    S_ISPBE_REGS_TYPE            *pstBeReg          = HI_NULL;

    pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
    ISP_CHECK_POINTER(pstBeReg);

    if (pstBeLutUpdateCfg->bAeLutUpdate)
    {
        isp_ae_lut_update_write(pstBeReg, pstBeLutUpdateCfg->bAeLutUpdate);
    }

    if (pstBeLutUpdateCfg->bSharpenLutUpdate)
    {
        isp_sharpen_lut_update_write(pstBeReg, pstBeLutUpdateCfg->bSharpenLutUpdate);
    }

    if (pstBeLutUpdateCfg->bDrcTmLutUpdate)
    {
        isp_drc_lut_update_write(pstBeReg, pstBeLutUpdateCfg->bDrcTmLutUpdate);
    }

    if (pstBeLutUpdateCfg->bLCacLutUpdate)
    {
        isp_demosaic_depurplut_update_write(pstBeReg, pstBeLutUpdateCfg->bLCacLutUpdate);
    }

    if (pstBeLutUpdateCfg->bNddmGfLutUpdate)
    {
        isp_nddm_gf_lut_update_write(pstBeReg, pstBeLutUpdateCfg->bNddmGfLutUpdate);
    }
    if (pstBeLutUpdateCfg->bLdciDrcLutUpdate)
    {
        isp_ldci_drc_lut_update_write(pstBeReg, pstBeLutUpdateCfg->bLdciDrcLutUpdate);
    }
    if (pstBeLutUpdateCfg->bLdciCalcLutUpdate)
    {
        isp_ldci_calc_lut_update_write(pstBeReg, pstBeLutUpdateCfg->bLdciCalcLutUpdate);
    }

    if (pstBeLutUpdateCfg->bBnrLutUpdate)
    {
        isp_bnr_lut_update_write(pstBeReg, pstBeLutUpdateCfg->bBnrLutUpdate);
    }
    if (pstBeLutUpdateCfg->bDeLutUpdate)
    {
        isp_de_lut_update_write(pstBeReg, pstBeLutUpdateCfg->bDeLutUpdate);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_BeCurRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    ISP_ALG_REG_CFG_S  *pstAlgRegCfg = &pstRegCfgInfo->stAlgRegCfg[i];
    S_ISPBE_REGS_TYPE  *pstBeReg     = HI_NULL;
    S_VIPROC_REGS_TYPE *pstViProcReg = HI_NULL;

    pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
    pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
    ISP_CHECK_POINTER(pstBeReg);
    ISP_CHECK_POINTER(pstViProcReg);

    isp_ae_en_write(pstViProcReg, pstAlgRegCfg->stAeRegCfg.stStaticRegCfg.u8BEEnable);
    isp_la_en_write(pstViProcReg, pstAlgRegCfg->stMgRegCfg.stStaticRegCfg.u8Enable);
    isp_wb_en_write(pstViProcReg, pstAlgRegCfg->stAwbRegCfg.stAwbRegDynCfg.u8BEWbWorkEn);
    isp_cc_en_write(pstViProcReg, pstAlgRegCfg->stAwbRegCfg.stAwbRegDynCfg.u8BECcEn);
    isp_cc_colortone_en_write(pstBeReg, pstAlgRegCfg->stAwbRegCfg.stAwbRegDynCfg.u16BECcColortoneEn);
    isp_awb_en_write(pstViProcReg, pstAlgRegCfg->stAwbRegCfg.stAwbRegStaCfg.u8BEAwbWorkEn);
    isp_af_en_write(pstViProcReg, pstAlgRegCfg->stBEAfRegCfg.bAfEnable);
    isp_sharpen_en_write(pstViProcReg, pstAlgRegCfg->stSharpenRegCfg.bEnable);
    isp_sharpen_benmarkedge_write(pstBeReg, pstAlgRegCfg->stEdgeMarkRegCfg.bEnable);
    isp_hlc_en_write(pstViProcReg, pstAlgRegCfg->stHlcRegCfg.bEnable);
    isp_dmnr_vhdm_en_write(pstViProcReg, pstAlgRegCfg->stDemRegCfg.bVhdmEnable);
    isp_dmnr_nddm_en_write(pstViProcReg, pstAlgRegCfg->stDemRegCfg.bNddmEnable);
    //isp_ldci_en_write(pstViProcReg, pstAlgRegCfg->stLdciRegCfg.stDynaRegCfg.bEnable);
    isp_demosaic_local_cac_en_write(pstBeReg, pstAlgRegCfg->stLCacRegCfg.bLocalCacEn);
    isp_gcac_en_write(pstBeReg, pstAlgRegCfg->stGCacRegCfg.bGlobalCacEn);
    isp_demosaic_fcr_en_write(pstBeReg, pstAlgRegCfg->stAntiFalseColorRegCfg.bFcrEnable);
    isp_dpc_en_write(pstViProcReg, pstAlgRegCfg->stDpRegCfg.abDpcEn[0]);
    isp_dpc_dpc_en1_write(pstBeReg, pstAlgRegCfg->stDpRegCfg.abDpcEn[1]);
    isp_ge_en_write(pstViProcReg, pstAlgRegCfg->stGeRegCfg.abGeEn[0]);
    isp_ge_ge1_en_write(pstBeReg, pstAlgRegCfg->stGeRegCfg.abGeEn[1]);
    isp_lsc_en_write(pstViProcReg, pstAlgRegCfg->stLscRegCfg.bLscEn);
    isp_gamma_en_write(pstViProcReg, pstAlgRegCfg->stGammaCfg.bGammaEn);
    isp_csc_en_write(pstViProcReg, pstAlgRegCfg->stCscCfg.bEnable);
    isp_ca_en_write(pstViProcReg, pstAlgRegCfg->stCaRegCfg.bCaEn);
    //isp_wdr_en_write(pstViProcReg, pstAlgRegCfg->stWdrRegCfg.bWDREn);
    //isp_drc_en_write(pstViProcReg, pstAlgRegCfg->stDrcRegCfg.bDrcEn);
    isp_dehaze_en_write(pstViProcReg, pstAlgRegCfg->stDehazeRegCfg.bDehazeEn);
    //isp_bnr_en_write(pstViProcReg, pstAlgRegCfg->stBnrRegCfg.bBnrEnable);
    if (pstAlgRegCfg->stLscRegCfg.bLscEn)
    {
        isp_bnr_rlsc_en_write(pstBeReg, pstAlgRegCfg->stBnrRegCfg.stUsrRegCfg.bBnrLscEn);
    }
    else
    {
        isp_bnr_rlsc_en_write(pstBeReg, 0);
    }
    //isp_dg_en_write(pstViProcReg, pstAlgRegCfg->stDgRegCfg.bDgEn);
    //isp_4dg_en_write(pstViProcReg, pstAlgRegCfg->st4DgRegCfg.bEnable);
    isp_pregamma_en_write(pstViProcReg, pstAlgRegCfg->stPreGammaCfg.bPreGammaEn);
    isp_clut_en_write(pstViProcReg, pstAlgRegCfg->stClutCfg.bEnable);
    isp_expander_en_write(pstViProcReg, pstAlgRegCfg->stExpanderCfg.bEnable);
    //isp_de_en_write(pstViProcReg, pstAlgRegCfg->stDeRegCfg.bDeEnable);
    //isp_bnr_de_enable_write(ViPipe, i, pstAlgRegCfg->stDeRegCfg.bDeEnable);
    //isp_nry0_en_write(pstViProcReg, pstAlgRegCfg->stDeRegCfg.bNry0Enable);

    isp_ldci_blc_ctrl_write(pstBeReg, pstAlgRegCfg->stLdciRegCfg.stDynaRegCfg.u32CalcBlcCtrl);

    if (pstAlgRegCfg->stLscRegCfg.stUsrRegCfg.bLutUpdate)
    {
        isp_lsc_mesh_scale_write(pstBeReg, pstAlgRegCfg->stLscRegCfg.stUsrRegCfg.u8MeshScale);
    }
    if (pstAlgRegCfg->stClutCfg.stUsrRegCfg.stClutUsrCtrlCfg.bResh)
    {
        isp_clut_gain0_write(pstBeReg, pstAlgRegCfg->stClutCfg.stUsrRegCfg.stClutUsrCtrlCfg.u32GainR);
        isp_clut_gain1_write(pstBeReg, pstAlgRegCfg->stClutCfg.stUsrRegCfg.stClutUsrCtrlCfg.u32GainG);
        isp_clut_gain2_write(pstBeReg, pstAlgRegCfg->stClutCfg.stUsrRegCfg.stClutUsrCtrlCfg.u32GainB);
    }

    isp_dehaze_air_r_write(pstBeReg, pstAlgRegCfg->stDehazeRegCfg.stDynaRegCfg.u16AirR);
    isp_dehaze_air_g_write(pstBeReg, pstAlgRegCfg->stDehazeRegCfg.stDynaRegCfg.u16AirG);
    isp_dehaze_air_b_write(pstBeReg, pstAlgRegCfg->stDehazeRegCfg.stDynaRegCfg.u16AirB);
    isp_dehaze_gstrth_write(pstBeReg,pstAlgRegCfg->stDehazeRegCfg.stDynaRegCfg.u8Strength);

    ISP_SharpenDynaRegConfig(pstBeReg, &pstAlgRegCfg->stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg, &pstAlgRegCfg->stSharpenRegCfg.stDynaRegCfg.stDefaultDynaRegCfg);

    return HI_SUCCESS;
}

static HI_S32 ISP_BeLastRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    ISP_LUT2STT_SYNC_REG_CFG_S *pstLut2SttSyncCfg = &pstRegCfgInfo->stAlgRegCfg[i].astLut2SttSyncCfg[0];
    S_ISPBE_REGS_TYPE          *pstBeReg          = HI_NULL;
    S_VIPROC_REGS_TYPE         *pstViProcReg      = HI_NULL;

    pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
    pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
    ISP_CHECK_POINTER(pstBeReg);
    ISP_CHECK_POINTER(pstViProcReg);

    isp_ae_en_write(pstViProcReg, pstLut2SttSyncCfg->bAeEn);
    isp_la_en_write(pstViProcReg, pstLut2SttSyncCfg->bLaEn);
    isp_wb_en_write(pstViProcReg, pstLut2SttSyncCfg->bWbEn);
    isp_cc_en_write(pstViProcReg, pstLut2SttSyncCfg->bCcmEn);
    isp_cc_colortone_en_write(pstBeReg, pstLut2SttSyncCfg->bCcmColorToneEn);
    isp_awb_en_write(pstViProcReg, pstLut2SttSyncCfg->bAwbEn);
    isp_af_en_write(pstViProcReg, pstLut2SttSyncCfg->bAfEn);
    isp_sharpen_en_write(pstViProcReg, pstLut2SttSyncCfg->bSharpenEn);
    isp_sharpen_benmarkedge_write(pstBeReg, pstLut2SttSyncCfg->bEdgeMarkEn);
    isp_hlc_en_write(pstViProcReg, pstLut2SttSyncCfg->bHlcEn);
    isp_dmnr_vhdm_en_write(pstViProcReg, pstLut2SttSyncCfg->bVhdmEn);
    isp_dmnr_nddm_en_write(pstViProcReg, pstLut2SttSyncCfg->bNddmEn);
    //isp_ldci_en_write(pstViProcReg, pstLut2SttSyncCfg->bLdciEn);
    isp_demosaic_local_cac_en_write(pstBeReg, pstLut2SttSyncCfg->bLcacEn);
    isp_gcac_en_write(pstBeReg, pstLut2SttSyncCfg->bGcacEn);
    isp_demosaic_fcr_en_write(pstBeReg, pstLut2SttSyncCfg->bFcrEn);
    isp_dpc_en_write(pstViProcReg, pstLut2SttSyncCfg->abDpcEn[0]);
    isp_dpc_dpc_en1_write(pstBeReg, pstLut2SttSyncCfg->abDpcEn[1]);
    isp_ge_en_write(pstViProcReg, pstLut2SttSyncCfg->abGeEn[0]);
    isp_ge_ge1_en_write(pstBeReg, pstLut2SttSyncCfg->abGeEn[1]);
    isp_lsc_en_write(pstViProcReg, pstLut2SttSyncCfg->bLscEn);
    isp_gamma_en_write(pstViProcReg, pstLut2SttSyncCfg->bGammaEn);
    isp_csc_en_write(pstViProcReg, pstLut2SttSyncCfg->bCscEn);
    isp_ca_en_write(pstViProcReg, pstLut2SttSyncCfg->bCaEn);
    //isp_wdr_en_write(pstViProcReg, pstLut2SttSyncCfg->bWdrEn);
    //isp_drc_en_write(pstViProcReg, pstLut2SttSyncCfg->bDrcEn);
    isp_dehaze_en_write(pstViProcReg, pstLut2SttSyncCfg->bDehazeEn);
    //isp_bnr_en_write(pstViProcReg, pstLut2SttSyncCfg->bBnrEn);
    //isp_bnr_rlsc_en_write(pstBeReg, pstLut2SttSyncCfg->bBnrLscEn);
    if (pstLut2SttSyncCfg->bLscEn)
    {
        isp_bnr_rlsc_en_write(pstBeReg, pstLut2SttSyncCfg->bBnrLscEn);
    }
    else
    {
        isp_bnr_rlsc_en_write(pstBeReg, 0);
    }

    //isp_dg_en_write(pstViProcReg, pstLut2SttSyncCfg->bDgEn);
    //isp_4dg_en_write(pstViProcReg, pstLut2SttSyncCfg->b4DgEn);
    isp_pregamma_en_write(pstViProcReg, pstLut2SttSyncCfg->bPregammaEn);
    isp_clut_en_write(pstViProcReg, pstLut2SttSyncCfg->bClutEn);
    isp_expander_en_write(pstViProcReg, pstLut2SttSyncCfg->bExpanderEn);
    //isp_de_en_write(pstViProcReg, pstLut2SttSyncCfg->bDeEn);
    //isp_bnr_de_enable_write(pstBeReg, pstLut2SttSyncCfg->bDeEn);
    //isp_nry0_en_write(pstViProcReg, pstLut2SttSyncCfg->bNry0En);

    isp_ldci_blc_ctrl_write(pstBeReg, pstLut2SttSyncCfg->stLdciSyncCfg.u32CalcBlcCtrl);

    if (pstLut2SttSyncCfg->stLscSyncCfg.bResh)
    {
        isp_lsc_mesh_scale_write(pstBeReg, pstLut2SttSyncCfg->stLscSyncCfg.u8MeshScale);
    }
    if (pstLut2SttSyncCfg->stClutSyncCfg.bResh)
    {
        isp_clut_gain0_write(pstBeReg, pstLut2SttSyncCfg->stClutSyncCfg.u32GainR);
        isp_clut_gain1_write(pstBeReg, pstLut2SttSyncCfg->stClutSyncCfg.u32GainG);
        isp_clut_gain2_write(pstBeReg, pstLut2SttSyncCfg->stClutSyncCfg.u32GainB);
    }

    if (HI_TRUE == pstRegCfgInfo->stAlgRegCfg[i].stDehazeRegCfg.bLut2SttEn)
    {
        isp_dehaze_air_r_write(pstBeReg, pstLut2SttSyncCfg->stDehazeSyncCfg.u16AirR);
        isp_dehaze_air_g_write(pstBeReg, pstLut2SttSyncCfg->stDehazeSyncCfg.u16AirG);
        isp_dehaze_air_b_write(pstBeReg, pstLut2SttSyncCfg->stDehazeSyncCfg.u16AirB);
        isp_dehaze_gstrth_write(pstBeReg,pstLut2SttSyncCfg->stDehazeSyncCfg.u8Strength);
    }

    ISP_SharpenDynaRegConfig(pstBeReg, &pstLut2SttSyncCfg->stSharpenSyncCfg.stMpiDynaRegCfg, &pstLut2SttSyncCfg->stSharpenSyncCfg.stDefaultDynaRegCfg);

    return HI_SUCCESS;
}


static HI_S32 ISP_BeAlgSyncRegConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    ISP_CTX_S *pstIspCtx   = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
        IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        ISP_BeCurRegConfig(ViPipe, pstRegCfgInfo, i);
    }
    else
    {
        ISP_BeLastRegConfig(ViPipe, pstRegCfgInfo, i);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_SaveBeSyncReg(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo, HI_U8 i)
{
    HI_U8 j;
    ISP_CTX_S *pstIspCtx   = HI_NULL;
    ISP_LUT2STT_SYNC_REG_CFG_S *pstLut2SttSyncCfg = &pstRegCfgInfo->stAlgRegCfg[i].astLut2SttSyncCfg[0];
    ISP_ALG_REG_CFG_S          *pstAlgRegCfg      = &pstRegCfgInfo->stAlgRegCfg[i];

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
        IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        return HI_SUCCESS;
    }

    pstLut2SttSyncCfg->bAeEn           = pstAlgRegCfg->stAeRegCfg.stStaticRegCfg.u8BEEnable;
    pstLut2SttSyncCfg->bLaEn           = pstAlgRegCfg->stMgRegCfg.stStaticRegCfg.u8Enable;
    pstLut2SttSyncCfg->bAwbEn          = pstAlgRegCfg->stAwbRegCfg.stAwbRegStaCfg.u8BEAwbWorkEn;
    pstLut2SttSyncCfg->bWbEn           = pstAlgRegCfg->stAwbRegCfg.stAwbRegDynCfg.u8BEWbWorkEn;
    pstLut2SttSyncCfg->bCcmEn          = pstAlgRegCfg->stAwbRegCfg.stAwbRegDynCfg.u8BECcEn;
    pstLut2SttSyncCfg->bCcmColorToneEn = pstAlgRegCfg->stAwbRegCfg.stAwbRegDynCfg.u16BECcColortoneEn;
    pstLut2SttSyncCfg->bAfEn           = pstAlgRegCfg->stBEAfRegCfg.bAfEnable;
    pstLut2SttSyncCfg->bSharpenEn      = pstAlgRegCfg->stSharpenRegCfg.bEnable;
    pstLut2SttSyncCfg->bEdgeMarkEn     = pstAlgRegCfg->stEdgeMarkRegCfg.bEnable;
    pstLut2SttSyncCfg->bHlcEn          = pstAlgRegCfg->stHlcRegCfg.bEnable;
    pstLut2SttSyncCfg->bVhdmEn         = pstAlgRegCfg->stDemRegCfg.bVhdmEnable;
    pstLut2SttSyncCfg->bNddmEn         = pstAlgRegCfg->stDemRegCfg.bNddmEnable;
    pstLut2SttSyncCfg->bLdciEn         = pstAlgRegCfg->stLdciRegCfg.stDynaRegCfg.bEnable;
    pstLut2SttSyncCfg->bLcacEn         = pstAlgRegCfg->stLCacRegCfg.bLocalCacEn;
    pstLut2SttSyncCfg->bGcacEn         = pstAlgRegCfg->stGCacRegCfg.bGlobalCacEn;
    pstLut2SttSyncCfg->bFcrEn          = pstAlgRegCfg->stAntiFalseColorRegCfg.bFcrEnable;
    pstLut2SttSyncCfg->bLscEn          = pstAlgRegCfg->stLscRegCfg.bLscEn;
    pstLut2SttSyncCfg->bGammaEn        = pstAlgRegCfg->stGammaCfg.bGammaEn;
    pstLut2SttSyncCfg->bCscEn          = pstAlgRegCfg->stCscCfg.bEnable;
    pstLut2SttSyncCfg->bCaEn           = pstAlgRegCfg->stCaRegCfg.bCaEn;
    pstLut2SttSyncCfg->bWdrEn          = pstAlgRegCfg->stWdrRegCfg.bWDREn;
    pstLut2SttSyncCfg->bDrcEn          = pstAlgRegCfg->stDrcRegCfg.bDrcEn;
    pstLut2SttSyncCfg->bDehazeEn       = pstAlgRegCfg->stDehazeRegCfg.bDehazeEn;
    //pstLut2SttSyncCfg->bBnrEn          = pstAlgRegCfg->stBnrRegCfg.bBnrEnable;
    pstLut2SttSyncCfg->bBnrLscEn       = pstAlgRegCfg->stBnrRegCfg.stUsrRegCfg.bBnrLscEn;
    pstLut2SttSyncCfg->bDgEn           = pstAlgRegCfg->stDgRegCfg.bDgEn;
    pstLut2SttSyncCfg->b4DgEn          = pstAlgRegCfg->st4DgRegCfg.bEnable;
    pstLut2SttSyncCfg->bPregammaEn     = pstAlgRegCfg->stPreGammaCfg.bPreGammaEn;
    pstLut2SttSyncCfg->bClutEn         = pstAlgRegCfg->stClutCfg.bEnable;
    //pstLut2SttSyncCfg->bWdrSplitEn     = pstAlgRegCfg->stSplitCfg.bEnable;
    pstLut2SttSyncCfg->bExpanderEn     = pstAlgRegCfg->stExpanderCfg.bEnable;
    //pstLut2SttSyncCfg->bDeEn           = pstAlgRegCfg->stDeRegCfg.bDeEnable;

    for (j = 0; j < 4; j++)
    {
        pstLut2SttSyncCfg->abDpcEn[j] = pstAlgRegCfg->stDpRegCfg.abDpcEn[j];
        pstLut2SttSyncCfg->abGeEn[j]  = pstAlgRegCfg->stGeRegCfg.abGeEn[j];
    }

    pstLut2SttSyncCfg->stLscSyncCfg.bResh       = pstAlgRegCfg->stBeLutUpdateCfg.bLscLutUpdate;
    pstLut2SttSyncCfg->stLscSyncCfg.u8MeshScale = pstAlgRegCfg->stLscRegCfg.stUsrRegCfg.u8MeshScale;
    pstLut2SttSyncCfg->stClutSyncCfg.bResh      = pstAlgRegCfg->stClutCfg.stUsrRegCfg.stClutUsrCtrlCfg.bResh;
    pstLut2SttSyncCfg->stClutSyncCfg.u32GainR   = pstAlgRegCfg->stClutCfg.stUsrRegCfg.stClutUsrCtrlCfg.u32GainR;
    pstLut2SttSyncCfg->stClutSyncCfg.u32GainG   = pstAlgRegCfg->stClutCfg.stUsrRegCfg.stClutUsrCtrlCfg.u32GainG;
    pstLut2SttSyncCfg->stClutSyncCfg.u32GainB   = pstAlgRegCfg->stClutCfg.stUsrRegCfg.stClutUsrCtrlCfg.u32GainB;

    pstLut2SttSyncCfg->stDehazeSyncCfg.u16AirR  = pstAlgRegCfg->stDehazeRegCfg.stDynaRegCfg.u16AirR;
    pstLut2SttSyncCfg->stDehazeSyncCfg.u16AirG  = pstAlgRegCfg->stDehazeRegCfg.stDynaRegCfg.u16AirG;
    pstLut2SttSyncCfg->stDehazeSyncCfg.u16AirB  = pstAlgRegCfg->stDehazeRegCfg.stDynaRegCfg.u16AirB;
    pstLut2SttSyncCfg->stDehazeSyncCfg.u8Strength = pstAlgRegCfg->stDehazeRegCfg.stDynaRegCfg.u8Strength;

    pstLut2SttSyncCfg->stLdciSyncCfg.u32CalcBlcCtrl  = pstAlgRegCfg->stLdciRegCfg.stDynaRegCfg.u32CalcBlcCtrl;

    /* sharpen begain */
    memcpy(&pstLut2SttSyncCfg->stSharpenSyncCfg.stMpiDynaRegCfg, &pstAlgRegCfg->stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg,
           sizeof(ISP_SHARPEN_MPI_DYNA_REG_CFG_S));
    memcpy(&pstLut2SttSyncCfg->stSharpenSyncCfg.stDefaultDynaRegCfg, &pstAlgRegCfg->stSharpenRegCfg.stDynaRegCfg.stDefaultDynaRegCfg,
           sizeof(ISP_SHARPEN_DEFAULT_DYNA_REG_CFG_S));
    /* shrpen end */

    return HI_SUCCESS;
}

HI_S32 ISP_BeReshCfg(ISP_ALG_REG_CFG_S *pstAlgRegCfg)
{
    pstAlgRegCfg->stAwbRegCfg.stAwbRegStaCfg.bBEAwbStaCfg     = HI_TRUE;
    pstAlgRegCfg->stAwbRegCfg.stAwbRegUsrCfg.bResh            = HI_TRUE;

    pstAlgRegCfg->stDemRegCfg.stStaticRegCfg.bResh            = HI_TRUE;
    pstAlgRegCfg->stDemRegCfg.stDynaRegCfg.bResh              = HI_TRUE;

    pstAlgRegCfg->stLdciRegCfg.stStaticRegCfg.bStaticResh     = HI_TRUE;

    pstAlgRegCfg->stLCacRegCfg.stStaticRegCfg.bStaticResh     = HI_TRUE;
    pstAlgRegCfg->stLCacRegCfg.stUsrRegCfg.bResh              = HI_TRUE;
    pstAlgRegCfg->stLCacRegCfg.stDynaRegCfg.bResh             = HI_TRUE;
    pstAlgRegCfg->stGCacRegCfg.stStaticRegCfg.bStaticResh     = HI_TRUE;
    pstAlgRegCfg->stGCacRegCfg.stUsrRegCfg.bResh              = HI_TRUE;

    pstAlgRegCfg->stAntiFalseColorRegCfg.stStaticRegCfg.bResh = HI_TRUE;
    pstAlgRegCfg->stAntiFalseColorRegCfg.stDynaRegCfg.bResh   = HI_TRUE;

    pstAlgRegCfg->stDpRegCfg.stStaticRegCfg.bStaticResh           = HI_TRUE;
    pstAlgRegCfg->stDpRegCfg.stUsrRegCfg.stUsrDynaCorRegCfg.bResh = HI_TRUE;
    pstAlgRegCfg->stDpRegCfg.stUsrRegCfg.stUsrStaCorRegCfg.bResh  = HI_TRUE;
    pstAlgRegCfg->stDpRegCfg.stDynaRegCfg.bResh                   = HI_TRUE;

    pstAlgRegCfg->stGeRegCfg.stStaticRegCfg.bStaticResh       = HI_TRUE;
    pstAlgRegCfg->stGeRegCfg.stUsrRegCfg.bResh                = HI_TRUE;
    pstAlgRegCfg->stGeRegCfg.stDynaRegCfg.bResh               = HI_TRUE;

    pstAlgRegCfg->stLscRegCfg.stStaticRegCfg.bStaticResh      = HI_TRUE;
    pstAlgRegCfg->stLscRegCfg.stUsrRegCfg.bCoefUpdate         = HI_TRUE;
    pstAlgRegCfg->stLscRegCfg.stUsrRegCfg.bLutUpdate          = HI_TRUE;

    pstAlgRegCfg->stRLscRegCfg.stStaticRegCfg.bStaticResh     = HI_TRUE;
    pstAlgRegCfg->stRLscRegCfg.stUsrRegCfg.bCoefUpdate        = HI_TRUE;
    pstAlgRegCfg->stRLscRegCfg.stUsrRegCfg.bLutUpdate         = HI_TRUE;

    pstAlgRegCfg->stGammaCfg.stUsrRegCfg.bGammaLutUpdateEn    = HI_TRUE;
    pstAlgRegCfg->stCscCfg.stDynaRegCfg.bResh                 = HI_TRUE;

    pstAlgRegCfg->stCaRegCfg.stStaticRegCfg.bStaticResh       = HI_TRUE;
    pstAlgRegCfg->stCaRegCfg.stDynaRegCfg.bResh               = HI_TRUE;
    pstAlgRegCfg->stCaRegCfg.stUsrRegCfg.bResh                = HI_TRUE;
    pstAlgRegCfg->stCaRegCfg.stUsrRegCfg.bCaLutUpdateEn       = HI_TRUE;

    pstAlgRegCfg->stMcdsRegCfg.stStaticRegCfg.bStaticResh     = HI_TRUE;
    pstAlgRegCfg->stMcdsRegCfg.stDynaRegCfg.bDynaResh         = HI_TRUE;

    pstAlgRegCfg->stWdrRegCfg.stStaticRegCfg.bResh            = HI_TRUE;
    pstAlgRegCfg->stWdrRegCfg.stUsrRegCfg.bResh               = HI_TRUE;
    pstAlgRegCfg->stWdrRegCfg.stDynaRegCfg.bResh              = HI_TRUE;

    pstAlgRegCfg->stDrcRegCfg.stStaticRegCfg.bStaticResh      = HI_TRUE;
    pstAlgRegCfg->stDrcRegCfg.stUsrRegCfg.bUsrResh            = HI_TRUE;
    pstAlgRegCfg->stDrcRegCfg.stDynaRegCfg.bDynaResh          = HI_TRUE;

    pstAlgRegCfg->stDehazeRegCfg.stStaticRegCfg.bResh         = HI_TRUE;
    pstAlgRegCfg->stDehazeRegCfg.stDynaRegCfg.u32LutUpdate    = 1;

    pstAlgRegCfg->stBnrRegCfg.stStaticRegCfg.bResh            = HI_TRUE;
    pstAlgRegCfg->stBnrRegCfg.stDynaRegCfg.bResh              = HI_TRUE;
    pstAlgRegCfg->stBnrRegCfg.stUsrRegCfg.bResh               = HI_TRUE;

    pstAlgRegCfg->st4DgRegCfg.stStaticRegCfg.bResh            = HI_TRUE;
    pstAlgRegCfg->st4DgRegCfg.stDynaRegCfg.bResh              = HI_TRUE;
    pstAlgRegCfg->stDgRegCfg.stStaticRegCfg.bResh             = HI_TRUE;
    pstAlgRegCfg->stDgRegCfg.stDynaRegCfg.bResh               = HI_TRUE;

    pstAlgRegCfg->stPreGammaCfg.stStaticRegCfg.bStaticResh        = HI_TRUE;
    pstAlgRegCfg->stPreGammaCfg.stDynaRegCfg.bPreGammaLutUpdateEn = HI_TRUE;
    pstAlgRegCfg->stFlickerRegCfg.stDynaRegCfg.bResh              = HI_TRUE;

    pstAlgRegCfg->stBeBlcCfg.bReshStatic                      = HI_TRUE;
    pstAlgRegCfg->stBeBlcCfg.bReshDyna                        = HI_TRUE;

    pstAlgRegCfg->stClutCfg.stUsrRegCfg.stClutUsrCtrlCfg.bResh = HI_TRUE;
    pstAlgRegCfg->stClutCfg.stUsrRegCfg.stClutUsrCoefCfg.bResh = HI_TRUE;

    pstAlgRegCfg->stExpanderCfg.stStaticCfg.bResh                        = HI_TRUE;

    pstAlgRegCfg->stSharpenRegCfg.stStaticRegCfg.bStaticResh             = HI_TRUE;
    pstAlgRegCfg->stSharpenRegCfg.stDynaRegCfg.stDefaultDynaRegCfg.bResh = HI_TRUE;
    pstAlgRegCfg->stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.bResh     = HI_TRUE;

    pstAlgRegCfg->stDeRegCfg.stStaticRegCfg.bResh            = HI_TRUE;
    pstAlgRegCfg->stDeRegCfg.stDynaRegCfg.bResh              = HI_TRUE;
    pstAlgRegCfg->stDeRegCfg.stUsrRegCfg.bResh               = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 ISP_ResetFeSttEn(VI_PIPE ViPipe)
{
    HI_U8   i;
    VI_PIPE ViPipeBind;
    ISP_CTX_S         *pstIspCtx = HI_NULL;
    S_ISPFE_REGS_TYPE *pstFeReg  = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (pstIspCtx->stWdrAttr.bMastPipe)
    {
        for (i = 0; i < pstIspCtx->stWdrAttr.stDevBindPipe.u32Num; i++)
        {
            ViPipeBind = pstIspCtx->stWdrAttr.stDevBindPipe.PipeId[i];
            ISP_CHECK_PIPE(ViPipeBind);

            pstFeReg = (S_ISPFE_REGS_TYPE *)ISP_GetFeVirAddr(ViPipeBind);
            ISP_CHECK_POINTER(pstFeReg);

            isp_ae1_stt_en_write(pstFeReg, HI_FALSE);
            isp_fe_update_write(pstFeReg,  HI_TRUE);
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_AlgEnExit(VI_PIPE ViPipe)
{
    HI_U8   i;
    VI_PIPE ViPipeBind;
    ISP_CTX_S           *pstIspCtx    = HI_NULL;
    S_ISPFE_REGS_TYPE   *pstFeReg     = HI_NULL;
    S_ISPBE_REGS_TYPE   *pstBeReg     = HI_NULL;
    S_VIPROC_REGS_TYPE  *pstViProcReg = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    if (HI_FALSE == pstIspCtx->stIspParaRec.bInit)
    {
        return HI_SUCCESS;
    }

    /*FE*/
    if (pstIspCtx->stWdrAttr.bMastPipe)
    {
        for (i = 0; i < pstIspCtx->stWdrAttr.stDevBindPipe.u32Num; i++)
        {
            ViPipeBind = pstIspCtx->stWdrAttr.stDevBindPipe.PipeId[i];
            ISP_CHECK_PIPE(ViPipeBind);

            pstFeReg = (S_ISPFE_REGS_TYPE *)ISP_GetFeVirAddr(ViPipeBind);
            ISP_CHECK_POINTER(pstFeReg);

            isp_fe_ae1_en_write(pstFeReg,  HI_FALSE);
            isp_fe_wb1_en_write(pstFeReg,  HI_FALSE);
            isp_fe_dg2_en_write(pstFeReg,  HI_FALSE);
            isp_ae1_stt_en_write(pstFeReg, HI_FALSE);
            isp_fe_update_write(pstFeReg,  HI_TRUE);
        }
    }

    if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
        IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        for (i = 0; i < pstIspCtx->stBlockAttr.u8BlockNum; i++)
        {
            pstBeReg     = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);
            pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);

            ISP_CHECK_POINTER(pstBeReg);
            ISP_CHECK_POINTER(pstViProcReg);

            isp_ae_en_write(pstViProcReg, HI_FALSE);
            isp_la_en_write(pstViProcReg, HI_FALSE);
            isp_awb_en_write(pstViProcReg, HI_FALSE);
            isp_af_en_write(pstViProcReg, HI_FALSE);
            isp_dehaze_en_write(pstViProcReg, HI_FALSE);
            isp_ldci_en_write(pstViProcReg, HI_FALSE);
            isp_flicker_en_write(pstViProcReg, HI_FALSE);

            isp_sharpen_stt2lut_en_write(pstBeReg, HI_FALSE);
            isp_ldci_stt2lut_en_write(pstBeReg, HI_FALSE);
            isp_dehaze_stt2lut_en_write(pstBeReg, HI_FALSE);
            isp_gamma_stt2lut_en_write(pstBeReg, HI_FALSE);
            isp_lsc_stt2lut_en_write(pstBeReg, HI_FALSE);
            isp_bnr_stt2lut_en_write(pstBeReg, HI_FALSE);
            isp_ca_stt2lut_en_write(pstBeReg, HI_FALSE);
            isp_clut_stt2lut_en_write(pstBeReg, HI_FALSE);
            isp_dpc_stt2lut_en_write(pstBeReg, HI_FALSE);
            isp_pregamma_stt2lut_en_write(pstBeReg, HI_FALSE);

            isp_be_manual_reg_up_write(pstViProcReg, HI_TRUE);
        }
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_FeRegsConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    ISP_CTX_S *pstIspCtx   = HI_NULL;
    ISP_CHECK_VIR_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (pstIspCtx->stWdrAttr.bMastPipe)
    {
        /*FE alg cfgs setting to register*/
        ISP_FeAeRegConfig(ViPipe, pstRegCfgInfo);  /*Ae*/
        ISP_FeAwbRegConfig(ViPipe, pstRegCfgInfo); /*awb*/
        ISP_FeDgRegConfig(ViPipe, pstRegCfgInfo);  /*DG*/
        ISP_FeBlcRegConfig(ViPipe, pstRegCfgInfo);

        ISP_FeSystemRegConfig(ViPipe, pstRegCfgInfo);

        ISP_FeSttRegConfig(ViPipe, pstRegCfgInfo);
        ISP_FeUpdateRegConfig(ViPipe, pstRegCfgInfo);
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_BeSyncParamInit(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    HI_U8  j;
    HI_S32 s32Ret;
    ISP_BE_SYNC_PARA_S stBeSyncParam = {0};
    ISP_ALG_REG_CFG_S  *pstAlgRegCfg = &pstRegCfgInfo->stAlgRegCfg[0];
    ISP_CTX_S          *pstIspCtx    = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

     if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) ||
         IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)) {
        return HI_SUCCESS;
     }

    /* DG */
    stBeSyncParam.au32IspDgain[0] = pstAlgRegCfg->stDgRegCfg.stDynaRegCfg.u16GainR;
    stBeSyncParam.au32IspDgain[1] = pstAlgRegCfg->stDgRegCfg.stDynaRegCfg.u16GainGR;
    stBeSyncParam.au32IspDgain[2] = pstAlgRegCfg->stDgRegCfg.stDynaRegCfg.u16GainGB;
    stBeSyncParam.au32IspDgain[3] = pstAlgRegCfg->stDgRegCfg.stDynaRegCfg.u16GainB;

    /* LDCI */
    stBeSyncParam.u32LdciComp = 0x1000;

    /* DRC */
    stBeSyncParam.u8DrcShpLog       = pstAlgRegCfg->stDrcRegCfg.stStaticRegCfg.u8ShpLog;
    stBeSyncParam.u32DrcDivDenomLog = pstAlgRegCfg->stDrcRegCfg.stStaticRegCfg.u32DivDenomLog;
    stBeSyncParam.u32DrcDenomExp    = pstAlgRegCfg->stDrcRegCfg.stStaticRegCfg.u32DenomExp;

    for (j = 0; j < SYNC_DRC_PRELUMA_NUM; j++) {
        stBeSyncParam.au32DrcPrevLuma[j] = pstAlgRegCfg->stDrcRegCfg.stStaticRegCfg.au32PrevLuma[j];
    }

    /* WDR */
    stBeSyncParam.u32WdrExpRatio   = pstAlgRegCfg->stWdrRegCfg.stStaticRegCfg.au16ExpoRRatio[0];
    stBeSyncParam.u32FlickExpRatio = pstAlgRegCfg->stWdrRegCfg.stStaticRegCfg.au16ExpoValue[0];

    for (j = 0; j < SYNC_WDR_EXP_VAL_NUM; j++) {
        stBeSyncParam.au32WdrExpVal[j] = pstAlgRegCfg->stWdrRegCfg.stStaticRegCfg.au16ExpoValue[j];
        stBeSyncParam.au32WDRGain[j]   = 0x100;
    }

    stBeSyncParam.u32WdrBlcComp  = pstAlgRegCfg->stWdrRegCfg.stStaticRegCfg.au32BlcComp[0];
    stBeSyncParam.bWDRMdtEn      = pstAlgRegCfg->stWdrRegCfg.stDynaRegCfg.bWDRMdtEn;
    stBeSyncParam.bFusionMode    = pstAlgRegCfg->stWdrRegCfg.stUsrRegCfg.bFusionMode;
    stBeSyncParam.u16ShortThr    = pstAlgRegCfg->stWdrRegCfg.stDynaRegCfg.u16ShortThr;
    stBeSyncParam.u16LongThr     = pstAlgRegCfg->stWdrRegCfg.stDynaRegCfg.u16LongThr;
    stBeSyncParam.u32WdrMaxRatio = pstAlgRegCfg->stWdrRegCfg.stStaticRegCfg.u32MaxRatio;

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_BE_SYNC_PARAM_INIT, &stBeSyncParam);
    if (s32Ret != HI_SUCCESS) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Init BE Sync Param Failed with ec %#x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_BeRegsConfig(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    HI_U32  i;
    HI_S32 s32Ret = 0;
    S_VIPROC_REGS_TYPE *pstViProcReg = HI_NULL;
    ISP_CTX_S          *pstIspCtx    = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    for (i = 0; i < pstRegCfgInfo->u8CfgNum; i++)
    {
        ISP_SystemRegConfig(ViPipe, pstRegCfgInfo, i);   /*sys*/
        ISP_DitherRegConfig(ViPipe, pstRegCfgInfo, i);   /*dither*/
        ISP_OnlineSttRegConfig(ViPipe, pstRegCfgInfo, i);
        /*Be alg cfgs setting to register*/
        ISP_AeRegConfig(ViPipe, pstRegCfgInfo, i);       /*ae*/
        ISP_AwbRegConfig(ViPipe, pstRegCfgInfo, i);      /*awb*/
        ISP_AfRegConfig(ViPipe, pstRegCfgInfo, i);       /*AF*/
        ISP_SharpenRegConfig(ViPipe, pstRegCfgInfo, i);  /*sharpen*/
        ISP_EdgeMarkRegConfig(ViPipe, pstRegCfgInfo, i); /*sharpen*/
        ISP_HlcRegConfig(ViPipe, pstRegCfgInfo, i);      /*Hlc*/
        ISP_DemRegConfig(ViPipe, pstRegCfgInfo, i);      /*demosaic*/
        ISP_FpnRegConfig(ViPipe, pstRegCfgInfo, i);      /*FPN*/
        ISP_LdciRegConfig(ViPipe, pstRegCfgInfo, i);     /*ldci*/
        ISP_LcacRegConfig(ViPipe, pstRegCfgInfo, i);     /*Local cac*/
        ISP_GcacRegConfig(ViPipe, pstRegCfgInfo, i);     /*global cac*/
        ISP_FcrRegConfig(ViPipe, pstRegCfgInfo, i);      /*FCR*/
        ISP_DpcRegConfig(ViPipe, pstRegCfgInfo, i);      /*dpc*/
        ISP_GeRegConfig(ViPipe, pstRegCfgInfo, i);       /*ge*/
        ISP_LscRegConfig(ViPipe, pstRegCfgInfo, i);      /*BE LSC*/
        ISP_GammaRegConfig(ViPipe, pstRegCfgInfo, i);    /*gamma*/
        ISP_CscRegConfig(ViPipe, pstRegCfgInfo, i);      /*csc*/
        ISP_CaRegConfig(ViPipe, pstRegCfgInfo, i);       /*ca*/
        ISP_McdsRegConfig(ViPipe, pstRegCfgInfo, i);     /*mcds*/
        ISP_WdrRegConfig(ViPipe, pstRegCfgInfo, i);      /*wdr*/
        ISP_DrcRegConfig(ViPipe, pstRegCfgInfo, i);      /*drc*/
        ISP_DehazeRegConfig(ViPipe, pstRegCfgInfo, i);   /*Dehaze*/
        ISP_BayerNrRegConfig(ViPipe, pstRegCfgInfo, i);  /*BayerNR*/
        ISP_BNRLscRegConfig(ViPipe, pstRegCfgInfo, i);   /*BayerNR-LSC*/
        ISP_DgRegConfig(ViPipe, pstRegCfgInfo, i);       /*DG*/
        ISP_4DgRegConfig(ViPipe, pstRegCfgInfo, i);      /*4DG*/
        ISP_PreGammaRegConfig(ViPipe, pstRegCfgInfo, i); /*PreGamma*/
        ISP_FlickRegConfig(ViPipe, pstRegCfgInfo, i);    /*Flicker*/
        ISP_BeBlcRegConfig(ViPipe, pstRegCfgInfo, i);
        ISP_ClutRegConfig(ViPipe, pstRegCfgInfo, i);     /*CLUT*/
        ISP_DetailRegConfig(ViPipe, pstRegCfgInfo, i);   /*detail*/
        ISP_ExpanderRegConfig(ViPipe, pstRegCfgInfo, i); /*expander*/
    }

    for (i = 0; i < pstRegCfgInfo->u8CfgNum; i++)
    {
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstViProcReg);
        ISP_BeAlgSyncRegConfig(ViPipe, pstRegCfgInfo, i);
        ISP_BeAlgLut2SttRegnewRegConfig(ViPipe, pstRegCfgInfo, i);
        //isp_be_reg_up_write(pstViProcReg, HI_TRUE);
        ISP_BeAlgLutUpdateRegConfig(ViPipe, pstRegCfgInfo, i);
    }

    for (i = 0; i < pstRegCfgInfo->u8CfgNum; i++)
    {
        ISP_SaveBeSyncReg(ViPipe, pstRegCfgInfo, i);
    }

    if ((IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)) ||
        (IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)))
    {
        s32Ret = ISP_CfgBeBufCtl(ViPipe);

        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe:%d Be config bufs ctl failed %x!\n", ViPipe, s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_BeRegsConfigInit(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfgInfo)
{
    HI_U32  i;
    S_VIPROC_REGS_TYPE *pstViProcReg = HI_NULL;

    for (i = 0; i < pstRegCfgInfo->u8CfgNum; i++)
    {
        ISP_SaveBeSyncReg(ViPipe, pstRegCfgInfo, i);
    }

    for (i = 0; i < pstRegCfgInfo->u8CfgNum; i++)
    {
        ISP_RegDefault(ViPipe, pstRegCfgInfo, i);
        ISP_SystemRegConfig(ViPipe, pstRegCfgInfo, i);    /*sys*/
        ISP_DitherRegConfig(ViPipe, pstRegCfgInfo, i);    /*dither*/
        ISP_OnlineSttRegConfig(ViPipe, pstRegCfgInfo, i);
        /*Be alg cfgs setting to register*/
        ISP_AeRegConfig(ViPipe, pstRegCfgInfo, i);       /*ae*/
        ISP_AwbRegConfig(ViPipe, pstRegCfgInfo, i);      /*awb*/
        ISP_AfRegConfig(ViPipe, pstRegCfgInfo, i);       /*AF*/
        ISP_SharpenRegConfig(ViPipe, pstRegCfgInfo, i);  /*sharpen*/
        ISP_EdgeMarkRegConfig(ViPipe, pstRegCfgInfo, i); /*sharpen*/
        ISP_HlcRegConfig(ViPipe, pstRegCfgInfo, i);      /*Hlc*/
        ISP_DemRegConfig(ViPipe, pstRegCfgInfo, i);      /*demosaic*/
        ISP_FpnRegConfig(ViPipe, pstRegCfgInfo, i);      /*FPN*/
        ISP_LdciRegConfig(ViPipe, pstRegCfgInfo, i);     /*ldci*/
        ISP_LcacRegConfig(ViPipe, pstRegCfgInfo, i);     /*Local cac*/
        ISP_GcacRegConfig(ViPipe, pstRegCfgInfo, i);     /*global cac*/
        ISP_FcrRegConfig(ViPipe, pstRegCfgInfo, i);      /*FCR*/
        ISP_DpcRegConfig(ViPipe, pstRegCfgInfo, i);      /*dpc*/
        ISP_GeRegConfig(ViPipe, pstRegCfgInfo, i);       /*ge*/
        ISP_LscRegConfig(ViPipe, pstRegCfgInfo, i);      /*BE LSC*/
        ISP_GammaRegConfig(ViPipe, pstRegCfgInfo, i);    /*gamma*/
        ISP_CscRegConfig(ViPipe, pstRegCfgInfo, i);      /*csc*/
        ISP_CaRegConfig(ViPipe, pstRegCfgInfo, i);       /*ca*/
        ISP_McdsRegConfig(ViPipe, pstRegCfgInfo, i);     /*mcds*/
        ISP_WdrRegConfig(ViPipe, pstRegCfgInfo, i);      /*wdr*/
        ISP_DrcRegConfig(ViPipe, pstRegCfgInfo, i);      /*drc*/
        ISP_DehazeRegConfig(ViPipe, pstRegCfgInfo, i);   /*Dehaze*/
        ISP_BayerNrRegConfig(ViPipe, pstRegCfgInfo, i);  /*BayerNR*/
        ISP_BNRLscRegConfig(ViPipe, pstRegCfgInfo, i);   /*BayerNR-LSC*/
        ISP_DgRegConfig(ViPipe, pstRegCfgInfo, i);       /*DG*/
        ISP_4DgRegConfig(ViPipe, pstRegCfgInfo, i);      /*4DG*/
        ISP_PreGammaRegConfig(ViPipe, pstRegCfgInfo, i); /*PreGamma*/
        ISP_FlickRegConfig(ViPipe, pstRegCfgInfo, i);    /*Flicker*/
        ISP_BeBlcRegConfig(ViPipe, pstRegCfgInfo, i);
        ISP_ClutRegConfig(ViPipe, pstRegCfgInfo, i);     /*CLUT*/
        ISP_DetailRegConfig(ViPipe, pstRegCfgInfo, i);   /*detail*/
        ISP_ExpanderRegConfig(ViPipe, pstRegCfgInfo, i); /*expander*/
    }

    for (i = 0; i < pstRegCfgInfo->u8CfgNum; i++)
    {
        pstViProcReg = (S_VIPROC_REGS_TYPE *)ISP_GetViProcVirAddr(ViPipe, i);
        ISP_CHECK_POINTER(pstViProcReg);
        ISP_BeAlgSyncRegConfig(ViPipe, pstRegCfgInfo, i);
        ISP_BeAlgLut2SttRegnewRegConfig(ViPipe, pstRegCfgInfo, i);
        isp_be_reg_up_write(pstViProcReg, HI_TRUE);
        ISP_BeAlgLutUpdateRegConfig(ViPipe, pstRegCfgInfo, i);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_RegCfgInit(VI_PIPE ViPipe)
{
    ISP_REGCFG_S *pstRegCfgCtx = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pstRegCfgCtx);

    if (HI_NULL == pstRegCfgCtx)
    {
        pstRegCfgCtx = (ISP_REGCFG_S *)ISP_MALLOC(sizeof(ISP_REGCFG_S));
        if (HI_NULL == pstRegCfgCtx)
        {
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] RegCfgCtx malloc memory failed!\n", ViPipe);
            return HI_ERR_ISP_NOMEM;
        }
    }

    memset(pstRegCfgCtx, 0, sizeof(ISP_REGCFG_S));

    ISP_REGCFG_SET_CTX(ViPipe, pstRegCfgCtx);

    return HI_SUCCESS;
}

HI_S32 ISP_RegCfgExit(VI_PIPE ViPipe)
{
    ISP_REGCFG_S *pstRegCfgCtx = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pstRegCfgCtx);
    ISP_FREE(pstRegCfgCtx);
    ISP_REGCFG_RESET_CTX(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_GetRegCfgCtx(VI_PIPE ViPipe, HI_VOID **ppCfg)
{
    ISP_CTX_S    *pstIspCtx = HI_NULL;
    ISP_REGCFG_S *pstRegCfg = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_REGCFG_GET_CTX(ViPipe, pstRegCfg);
    ISP_CHECK_POINTER(pstRegCfg);

    if (!pstRegCfg->bInit)
    {
        pstRegCfg->stRegCfg.unKey.u64Key = 0;

        pstRegCfg->bInit = HI_TRUE;
    }

    pstRegCfg->stRegCfg.u8CfgNum = pstIspCtx->stBlockAttr.u8BlockNum;

    *ppCfg = &pstRegCfg->stRegCfg;

    return HI_SUCCESS;
}

HI_S32 ISP_RegCfgInfoInit(VI_PIPE ViPipe)
{
    ISP_REGCFG_S *pstRegCfg = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pstRegCfg);
    ISP_CHECK_POINTER(pstRegCfg);

    ISP_FeRegsConfig(ViPipe, &pstRegCfg->stRegCfg);
    ISP_BeRegsConfigInit(ViPipe, &pstRegCfg->stRegCfg);
    ISP_BeSyncParamInit(ViPipe, &pstRegCfg->stRegCfg);
    return HI_SUCCESS;
}

HI_S32 ISP_RegCfgInfoSet(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_REGCFG_S *pstRegCfg = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pstRegCfg);
    ISP_CHECK_POINTER(pstRegCfg);

    ISP_FeRegsConfig(ViPipe, &pstRegCfg->stRegCfg);
    ISP_BeRegsConfig(ViPipe, &pstRegCfg->stRegCfg);

    if (pstRegCfg->stRegCfg.stKernelRegCfg.unKey.u32Key)
    {
        s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_REG_CFG_SET, &pstRegCfg->stRegCfg.stKernelRegCfg);

        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "Config ISP register Failed with ec %#x!\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_VOID ISP_SnsRegsInfoCheck(VI_PIPE ViPipe, ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    if ((pstSnsRegsInfo->enSnsType >= ISP_SNS_TYPE_BUTT))
    {
        ISP_TRACE(HI_DBG_ERR, "senor's regs info invalid, enSnsType %d\n", pstSnsRegsInfo->enSnsType);
        return;
    }

    if (pstSnsRegsInfo->u32RegNum > ISP_MAX_SNS_REGS)
    {
        ISP_TRACE(HI_DBG_ERR, "senor's regs info invalid, u32RegNum %d\n", pstSnsRegsInfo->u32RegNum);
        return;
    }

    return;
}

HI_S32 ISP_SyncCfgSet(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_S32 s32PipeSt = 0;
    HI_S32 s32PipeEd = 0;
    HI_S8 s8StitchMainPipe;
    ISP_CTX_S    *pstIspCtx  = HI_NULL;
    ISP_REGCFG_S *pstRegCfg  = HI_NULL;
    ISP_REGCFG_S *pstRegCfgS = HI_NULL;
    ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (HI_TRUE == pstIspCtx->stStitchAttr.bStitchEnable)
    {
        s8StitchMainPipe = pstIspCtx->stStitchAttr.as8StitchBindId[0];

        if (IS_STITCH_MAIN_PIPE(ViPipe, s8StitchMainPipe))
        {
            s32PipeSt = 0;
            s32PipeEd = pstIspCtx->stStitchAttr.u8StitchPipeNum - 1;
        }
        else
        {
            s32PipeSt = ViPipe;
            s32PipeEd = ViPipe - 1;
        }
    }
    else
    {
        s32PipeSt = ViPipe;
        s32PipeEd = ViPipe;
    }

    while (s32PipeSt <= s32PipeEd)
    {
        if (HI_TRUE == pstIspCtx->stStitchAttr.bStitchEnable)
        {
            ViPipe = pstIspCtx->stStitchAttr.as8StitchBindId[s32PipeSt];
        }
        else
        {
            ViPipe = s32PipeSt;
        }

        ISP_GET_CTX(ViPipe, pstIspCtx);
        ISP_REGCFG_GET_CTX(ViPipe, pstRegCfg);
        ISP_CHECK_POINTER(pstRegCfg);
        ISP_CHECK_OPEN(ViPipe);

        if (HI_SUCCESS != ISP_SensorUpdateSnsReg(ViPipe))
        {
            /* If Users need to config AE sync info themselves, they can set pfn_cmos_get_sns_reg_info to NULL in cmos.c */
            /* Then there will be NO AE sync configs in kernel of firmware */
            return HI_SUCCESS;
        }

        ISP_SensorGetSnsReg(ViPipe, &pstSnsRegsInfo);
        memcpy(&pstRegCfg->stSyncCfgNode.stSnsRegsInfo, pstSnsRegsInfo, sizeof(ISP_SNS_REGS_INFO_S));
        ISP_SnsRegsInfoCheck(ViPipe, &pstRegCfg->stSyncCfgNode.stSnsRegsInfo);
        memcpy(&pstRegCfg->stSyncCfgNode.stAERegCfg, &pstRegCfg->stRegCfg.stAlgRegCfg[0].stAeRegCfg2, sizeof(ISP_AE_REG_CFG_2_S));
        memcpy(&pstRegCfg->stSyncCfgNode.stAWBRegCfg.au32WDRWBGain[0], &pstRegCfg->stRegCfg.stAlgRegCfg[0].stAwbRegCfg.stAwbRegDynCfg.au32WDRWBGain[0], sizeof(HI_U32) * ISP_BAYER_CHN_NUM);
        memcpy(&pstRegCfg->stSyncCfgNode.stAWBRegCfg.au32BEWhiteBalanceGain[0], &pstRegCfg->stRegCfg.stAlgRegCfg[0].stAwbRegCfg.stAwbRegDynCfg.au32BEWhiteBalanceGain[0], sizeof(HI_U32) * ISP_BAYER_CHN_NUM);
        memcpy(&pstRegCfg->stSyncCfgNode.stAWBRegCfg.au16ColorMatrix[0], &pstRegCfg->stRegCfg.stAlgRegCfg[0].stAwbRegCfg.stAwbRegDynCfg.au16BEColorMatrix[0], sizeof(HI_U16)*CCM_MATRIX_SIZE);
        memcpy(&pstRegCfg->stSyncCfgNode.stDRCRegCfg, &pstRegCfg->stRegCfg.stAlgRegCfg[0].stDrcRegCfg.stSyncRegCfg,  sizeof(ISP_DRC_REG_CFG_2_S));
        memcpy(&pstRegCfg->stSyncCfgNode.stWDRRegCfg, &pstRegCfg->stRegCfg.stAlgRegCfg[0].stWdrRegCfg.stSyncRegCfg,  sizeof(ISP_FSWDR_SYNC_CFG_S));
        memcpy(&pstRegCfg->stSyncCfgNode.stDeRegCfg,  &pstRegCfg->stRegCfg.stAlgRegCfg[0].stDeRegCfg.stKernelRegCfg, sizeof(ISP_DE_REG_KERNEL_S));

        if (HI_TRUE == pstIspCtx->stStitchAttr.bStitchEnable)
        {
            s8StitchMainPipe = pstIspCtx->stStitchAttr.as8StitchBindId[0];
            ISP_REGCFG_GET_CTX(s8StitchMainPipe, pstRegCfgS);
            if (HI_NULL == pstRegCfgS)
            {
                return HI_FAILURE;
            }

            if (!IS_STITCH_MAIN_PIPE(ViPipe, s8StitchMainPipe))
            {
                memcpy(&pstRegCfg->stSyncCfgNode.stSnsRegsInfo, &pstRegCfgS->stSyncCfgNode.stSnsRegsInfo, sizeof(ISP_SNS_REGS_INFO_S));
                memcpy(&pstRegCfg->stSyncCfgNode.stSnsRegsInfo.unComBus, &pstSnsRegsInfo->unComBus, sizeof(ISP_SNS_COMMBUS_U));
                memcpy(&pstRegCfg->stSyncCfgNode.stSnsRegsInfo.stSlvSync.u32SlaveBindDev, &pstSnsRegsInfo->stSlvSync.u32SlaveBindDev, sizeof(HI_U32));
                memcpy(&pstRegCfg->stSyncCfgNode.stAERegCfg, &pstRegCfgS->stRegCfg.stAlgRegCfg[0].stAeRegCfg2, sizeof(ISP_AE_REG_CFG_2_S));
                memcpy(&pstRegCfg->stSyncCfgNode.stAWBRegCfg.au16ColorMatrix[0], &pstRegCfg->stRegCfg.stAlgRegCfg[0].stAwbRegCfg.stAwbRegDynCfg.au16BEColorMatrix[0], sizeof(HI_U16)*CCM_MATRIX_SIZE);
                memcpy(&pstRegCfg->stSyncCfgNode.stAWBRegCfg.au32BEWhiteBalanceGain[0], &pstRegCfg->stRegCfg.stAlgRegCfg[0].stAwbRegCfg.stAwbRegDynCfg.au32BEWhiteBalanceGain[0], sizeof(HI_U32) * ISP_BAYER_CHN_NUM);
                memcpy(&pstRegCfg->stSyncCfgNode.stAWBRegCfg.au32WDRWBGain[0], &pstRegCfg->stRegCfg.stAlgRegCfg[0].stAwbRegCfg.stAwbRegDynCfg.au32WDRWBGain[0], sizeof(HI_U32) * ISP_BAYER_CHN_NUM);
            }
        }

        pstRegCfg->stSyncCfgNode.bValid = HI_TRUE;

        s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_SYNC_CFG_SET, &pstRegCfg->stSyncCfgNode);

        if (HI_SUCCESS != s32Ret)
        {
            //ISP_TRACE(HI_DBG_ERR, "Config Sync register Failed with ec %#x!\n", s32Ret);
            return s32Ret;
        }

        pstSnsRegsInfo->bConfig = HI_TRUE;

        s32PipeSt++;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SnapRegCfgSet(VI_PIPE ViPipe, ISP_CONFIG_INFO_S *pstSnapInfo)
{
    HI_S32 s32Ret;

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_CONFIG_INFO_SET, pstSnapInfo);

    if (HI_SUCCESS != s32Ret)
    {
        //ISP_TRACE(HI_DBG_ERR, "Config Sync register Failed with ec %#x!\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SnapRegCfgGet(VI_PIPE ViPipe, ISP_SNAP_INFO_S *pstSnapInfo)
{
    HI_S32 s32Ret;

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_SNAP_INFO_GET, pstSnapInfo);

    if (HI_SUCCESS != s32Ret)
    {
        //ISP_TRACE(HI_DBG_ERR, "Config Sync register Failed with ec %#x!\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_BOOL ISP_ProTriggerGet(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_BOOL bEnable;

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_PRO_TRIGGER_GET, &bEnable);

    if (HI_SUCCESS != s32Ret)
    {
        //ISP_TRACE(HI_DBG_ERR, "Config Sync register Failed with ec %#x!\n", s32Ret);
        return HI_FALSE;
    }

    return bEnable;
}

HI_S32 ISP_RegCfgCtrl(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_CTX_S    *pstIspCtx = HI_NULL;
    ISP_REGCFG_S *pstRegCfg = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_REGCFG_GET_CTX(ViPipe, pstRegCfg);

    pstRegCfg->stRegCfg.unKey.u64Key  = 0xFFFFFFFFFFFFFFFF;

    for (i = pstIspCtx->stBlockAttr.u8PreBlockNum; i < pstIspCtx->stBlockAttr.u8BlockNum; i++)
    {
        memcpy(&pstRegCfg->stRegCfg.stAlgRegCfg[i], &pstRegCfg->stRegCfg.stAlgRegCfg[0], sizeof(ISP_ALG_REG_CFG_S));
    }

    if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
        IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        for (i = 0; i < pstIspCtx->stBlockAttr.u8BlockNum; i++)
        {
            //i = MIN2(i, ISP_MAX_BE_NUM - 1);
            pstIspCtx->stSpecialOpt.abBeOnSttUpdate[i] = HI_TRUE;
        }

        for (i = pstIspCtx->stBlockAttr.u8PreBlockNum; i < pstIspCtx->stBlockAttr.u8BlockNum; i++)
        {
            ISP_BeReshCfg(&pstRegCfg->stRegCfg.stAlgRegCfg[i]);
        }
    }

    pstRegCfg->stRegCfg.u8CfgNum = pstIspCtx->stBlockAttr.u8BlockNum;

    return HI_SUCCESS;
}

static HI_S32 ISP_LutApbRegconfig(VI_PIPE ViPipe, ISP_REGCFG_S *pstRegCfg)
{
    HI_U8  i;
    HI_U16 j;
    S_ISPBE_REGS_TYPE     *pstBeReg            = HI_NULL;
    ISP_LDCI_DYNA_CFG_S   *pstLdciDynaRegCfg   = HI_NULL;
    ISP_LDCI_STATIC_CFG_S *pstLdciStaticRegCfg = HI_NULL;

    for (i = 0; i < pstRegCfg->stRegCfg.u8CfgNum; i++)
    {
        pstBeReg = (S_ISPBE_REGS_TYPE *)ISP_GetBeVirAddr(ViPipe, i);

        ISP_CHECK_POINTER(pstBeReg);

        /*gamma*/
        isp_gamma_lut_waddr_write(pstBeReg, 0);

        for (j = 0; j < GAMMA_REG_NODE_NUM; j++) {
            isp_gamma_lut_wdata_write(pstBeReg, pstRegCfg->stRegCfg.stAlgRegCfg[i].stGammaCfg.stUsrRegCfg.au16GammaLUT[j]);
        }

        /*sharpen*/
        isp_sharpen_mfgaind_waddr_write(pstBeReg, 0);
        isp_sharpen_mfgainud_waddr_write(pstBeReg, 0);
        isp_sharpen_hfgaind_waddr_write(pstBeReg, 0);
        isp_sharpen_hfgainud_waddr_write(pstBeReg, 0);

        for (j = 0; j < SHRP_GAIN_LUT_SIZE; j++) {
            isp_sharpen_mfgaind_wdata_write(pstBeReg, pstRegCfg->stRegCfg.stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.u16mfGainD[j]);
            isp_sharpen_mfgainud_wdata_write(pstBeReg, pstRegCfg->stRegCfg.stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.u16mfGainUD[j]);
            isp_sharpen_hfgaind_wdata_write(pstBeReg, pstRegCfg->stRegCfg.stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.u16hfGainD[j]);
            isp_sharpen_hfgainud_wdata_write(pstBeReg, pstRegCfg->stRegCfg.stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.u16hfGainUD[j]);
        }

        isp_sharpen_lut_update_write(pstBeReg, HI_TRUE);

        /*PreGamma*/
        if (HI_TRUE == pstRegCfg->stRegCfg.stAlgRegCfg[i].stPreGammaCfg.stDynaRegCfg.bSwitchMode)
        {
            isp_pregamma_lut_waddr_write(pstBeReg, 0);

            for (j = 0; j < PREGAMMA_NODE_NUM; j++)
            {
                isp_pregamma_lut_wdata_write(pstBeReg, pstRegCfg->stRegCfg.stAlgRegCfg[i].stPreGammaCfg.stDynaRegCfg.u32PreGammaLUT[j]);
            }
        }

        /* LDCI */
        pstLdciDynaRegCfg   = &pstRegCfg->stRegCfg.stAlgRegCfg[i].stLdciRegCfg.stDynaRegCfg;
        pstLdciStaticRegCfg = &pstRegCfg->stRegCfg.stAlgRegCfg[i].stLdciRegCfg.stStaticRegCfg;
        isp_ldci_drc_waddr_write(pstBeReg, 0);
        isp_ldci_cgain_waddr_write(pstBeReg, 0);
        isp_ldci_de_usm_waddr_write(pstBeReg, 0);
        isp_ldci_he_waddr_write(pstBeReg, 0);

        for (j = 0;j < 65;j++) {
            isp_ldci_drc_wdata_write(pstBeReg, pstLdciStaticRegCfg->as16CalcDynRngCmpLut[j], pstLdciStaticRegCfg->as16StatDynRngCmpLut[j]);
        }

        for (j = 0;j < LDCI_COLOR_GAIN_LUT_SIZE;j++) {
            isp_ldci_cgain_wdata_write(pstBeReg, pstLdciDynaRegCfg->u32ColorGainLut[j]);
        }

        for (j = 0;j < LDCI_DE_USM_LUT_SIZE;j++) {
            isp_ldci_de_usm_wdata_write(pstBeReg, pstLdciDynaRegCfg->u32UsmPosLut[j], pstLdciDynaRegCfg->u32UsmNegLut[j], pstLdciDynaRegCfg->u32DeLut[j]);
        }

        for (j = 0;j < LDCI_HE_LUT_SIZE;j++) {
            isp_ldci_he_wdata_write(pstBeReg, pstLdciDynaRegCfg->u32HePosLut[j], pstLdciDynaRegCfg->u32HeNegLut[j]);
        }

        isp_ldci_drc_lut_update_write(pstBeReg, HI_TRUE);
        isp_ldci_calc_lut_update_write(pstBeReg, HI_TRUE);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SwitchRegSet(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_REGCFG_S *pstRegCfg = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_REGCFG_GET_CTX(ViPipe, pstRegCfg);
    ISP_CHECK_POINTER(pstRegCfg);

    if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
        IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        //ISP_RegCfgInfoSet(ViPipe);
        ISP_LutApbRegconfig(ViPipe, pstRegCfg);
        ISP_RegCfgInfoInit(ViPipe);

        if (pstRegCfg->stRegCfg.stKernelRegCfg.unKey.u32Key)
        {
            s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_REG_CFG_SET, &pstRegCfg->stRegCfg.stKernelRegCfg);

            if (HI_SUCCESS != s32Ret)
            {
                ISP_TRACE(HI_DBG_ERR, "Config ISP register Failed with ec %#x!\n", s32Ret);
                return s32Ret;
            }
        }

        pstIspCtx->stBlockAttr.u8PreBlockNum = pstIspCtx->stBlockAttr.u8BlockNum;

        return HI_SUCCESS;
    }

    /* record the register config infomation to fhy and kernel,and be valid in next frame. */
    ISP_FeRegsConfig(ViPipe, &pstRegCfg->stRegCfg);
    ISP_BeRegsConfigInit(ViPipe, &pstRegCfg->stRegCfg);

    pstIspCtx->stIspParaRec.bStitchSync = HI_TRUE;
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_SYNC_INIT_SET, &pstIspCtx->stIspParaRec.bStitchSync);
    if (HI_SUCCESS != s32Ret)
    {
        pstIspCtx->stIspParaRec.bStitchSync = HI_FALSE;
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set isp stitch sync failed!\n", ViPipe);
    }

    s32Ret = ISP_AllCfgsBeBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d init all be bufs failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    pstIspCtx->stBlockAttr.u8PreBlockNum = pstIspCtx->stBlockAttr.u8BlockNum;

    return HI_SUCCESS;
}

HI_S32 ISP_Lut2SttApbReg(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_REGCFG_S *pstRegCfg = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) ||
        IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)) {
        ISP_REGCFG_GET_CTX(ViPipe, pstRegCfg);
        ISP_CHECK_POINTER(pstRegCfg);

        ISP_LutApbRegconfig(ViPipe, pstRegCfg);
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
