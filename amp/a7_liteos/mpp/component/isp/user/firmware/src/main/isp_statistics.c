/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_statistics.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/05/06
  Description   :
  History       :
  1.Date        : 2013/05/06
    Author      :
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "mkp_isp.h"
#include "mpi_sys.h"
#include "isp_statistics.h"
#include "isp_ext_config.h"
#include "isp_main.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct hiISP_STA_S {
    HI_U64  u64StatPhyaddr;
    HI_VOID *pStatVirtAddr;

    HI_BOOL bRead;
    ISP_STAT_INFO_S stStatInfo;
} ISP_STA_S;

ISP_STA_S g_astStatCtx[ISP_MAX_PIPE_NUM] = { { 0 } };
#define STATISTICS_GET_CTX(dev, pstCtx) pstCtx = &g_astStatCtx[dev]

extern HI_S32 g_as32IspFd[ISP_MAX_PIPE_NUM];

HI_S32 ISP_StatisticsInit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_STA_S *pstStat = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    STATISTICS_GET_CTX(ViPipe, pstStat);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_STAT_BUF_INIT, &pstStat->u64StatPhyaddr);

    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init statistics bufs failed!\n", ViPipe);
        return s32Ret;
    }

    pstStat->pStatVirtAddr = HI_MPI_SYS_MmapCache(pstStat->u64StatPhyaddr,
                                                  sizeof(ISP_STAT_S) * MAX_ISP_STAT_BUF_NUM);

    if (HI_NULL == pstStat->pStatVirtAddr) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] mmap statistics bufs failed!\n", ViPipe);

        s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_STAT_BUF_EXIT);

        if (HI_SUCCESS != s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] exit statistics bufs failed!\n", ViPipe);
            return s32Ret;
        }

        return HI_ERR_ISP_NOMEM;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_StatisticsExit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_STA_S *pstStat = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    STATISTICS_GET_CTX(ViPipe, pstStat);

    if (HI_NULL != pstStat->pStatVirtAddr) {
        HI_MPI_SYS_Munmap(pstStat->pStatVirtAddr,
                          sizeof(ISP_STAT_S) * MAX_ISP_STAT_BUF_NUM);
        pstStat->pStatVirtAddr = HI_NULL;
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_STAT_BUF_EXIT);

    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] exit statistics bufs failed!\n", ViPipe);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_StatisticsGetBuf(VI_PIPE ViPipe, HI_VOID **ppStat)
{
    HI_S32 s32Ret;
    ISP_STA_S *pstStat = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(ppStat);

    STATISTICS_GET_CTX(ViPipe, pstStat);

    if (!pstStat->bRead) {
        s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_STAT_BUF_GET, &pstStat->stStatInfo);
        if (s32Ret) {
            return s32Ret;
        }

        if (HI_NULL != pstStat->pStatVirtAddr) {
            pstStat->stStatInfo.pVirtAddr = (HI_VOID *)((HI_U8 *)pstStat->pStatVirtAddr +
                                                        (pstStat->stStatInfo.u64PhyAddr - pstStat->u64StatPhyaddr));
        } else {
            pstStat->stStatInfo.pVirtAddr = HI_NULL;
        }

        pstStat->bRead = HI_TRUE;
    }

    if (HI_NULL == pstStat->stStatInfo.pVirtAddr) {
        return HI_FAILURE;
    }

    *ppStat = pstStat->stStatInfo.pVirtAddr;

    return HI_SUCCESS;
}

HI_S32 ISP_StatisticsPutBuf(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_U32 u32KeyLowbit, u32KeyHighbit;
    ISP_STA_S *pstStat = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    STATISTICS_GET_CTX(ViPipe, pstStat);

    u32KeyLowbit  = hi_ext_system_statistics_ctrl_lowbit_read(ViPipe);
    u32KeyHighbit = hi_ext_system_statistics_ctrl_highbit_read(ViPipe);
    pstStat->stStatInfo.unKey.u64Key = ((HI_U64)u32KeyHighbit << 32) + u32KeyLowbit;

    if (pstStat->bRead) {
        s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_STAT_BUF_PUT, &pstStat->stStatInfo);
        if (s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "Release ISP[%d] stat buf Failed with ec %#x!\n", ViPipe, s32Ret);
        }
        pstStat->stStatInfo.pVirtAddr = HI_NULL;
        pstStat->bRead = HI_FALSE;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
