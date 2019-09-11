/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_proc.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/08/06
  Description   :
  History       :
  1.Date        : 2013/08/06
    Author      :
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "mpi_sys.h"
#include "mkp_isp.h"
#include "isp_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct hiISP_PROC_S {
    HI_U32 u32IntCount;
    HI_U32 u32ProcParam;
    ISP_PROC_MEM_S stProcMem;
} ISP_PROC_S;

ISP_PROC_S g_astProcCtx[ISP_MAX_PIPE_NUM] = { { 0 } };
#define PROC_GET_CTX(dev, pstCtx) pstCtx = &g_astProcCtx[dev]

HI_S32 ISP_UpdateProcParam(VI_PIPE ViPipe);

HI_S32 ISP_ProcInit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_PROC_S *pstProc = HI_NULL;

    PROC_GET_CTX(ViPipe, pstProc);

    ISP_CHECK_OPEN(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_PROC_PARAM_GET, &pstProc->u32ProcParam);
    if (s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get proc param %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    if (0 == pstProc->u32ProcParam) {
        return HI_SUCCESS;
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_PROC_INIT, &pstProc->stProcMem);
    if (s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init proc ec %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    pstProc->stProcMem.pProcVirtAddr = HI_MPI_SYS_Mmap(pstProc->stProcMem.u64ProcPhyAddr,
                                                       pstProc->stProcMem.u32ProcSize);
    if (HI_NULL == pstProc->stProcMem.pProcVirtAddr) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] mmap proc mem failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOMEM;
        goto freeproc;
    }
    pstProc->u32IntCount = 0;

    return HI_SUCCESS;

freeproc:
    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_PROC_EXIT)) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] exit proc failed!\n", ViPipe);
        return HI_FAILURE;
    }
    return s32Ret;
}

HI_S32 ISP_ProcWrite(const ISP_ALG_NODE_S *astAlgs, VI_PIPE ViPipe)
{
    HI_S32 s32Ret, i;
    ISP_PROC_S *pstProc = HI_NULL;
    ISP_CTRL_PROC_WRITE_S stProcCtrl;

    PROC_GET_CTX(ViPipe, pstProc);

    ISP_CHECK_OPEN(ViPipe);

    ISP_UpdateProcParam(ViPipe);

    if (0 == pstProc->u32ProcParam) {
        return HI_SUCCESS;
    }

    if (HI_NULL == pstProc->stProcMem.pProcVirtAddr) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] the proc hasn't init!\n", ViPipe);
        return HI_FAILURE;
    }

    /* write proc info 1s a time */
    pstProc->u32IntCount++;
    if (pstProc->u32IntCount < pstProc->u32ProcParam) {
        return HI_SUCCESS;
    }
    pstProc->u32IntCount = 0;

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_PROC_WRITE_ING);
    if (s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] write proc failed, ec %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    stProcCtrl.pcProcBuff = (HI_CHAR *)pstProc->stProcMem.pProcVirtAddr;
    stProcCtrl.u32BuffLen = pstProc->stProcMem.u32ProcSize - 1;
    stProcCtrl.u32WriteLen = 0;

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (astAlgs[i].bUsed) {
            if (HI_NULL != astAlgs[i].stAlgFunc.pfn_alg_ctrl) {
                astAlgs[i].stAlgFunc.pfn_alg_ctrl(ViPipe, ISP_PROC_WRITE, &stProcCtrl);
            }

            if (stProcCtrl.u32WriteLen > stProcCtrl.u32BuffLen) {
                ISP_TRACE(HI_DBG_ERR, "ISP[%d] Warning!! Proc buff overflow!\n", ViPipe);
                stProcCtrl.u32WriteLen = stProcCtrl.u32BuffLen;
                break;
            }

            if (0 != stProcCtrl.u32WriteLen) {
                if ('\0' != stProcCtrl.pcProcBuff[stProcCtrl.u32WriteLen - 1]) {
                    ISP_TRACE(HI_DBG_ERR, "ISP[%d] Warning!! alg %d's proc doesn't finished with endl!\n", ViPipe, astAlgs[i].enAlgType);
                }
                stProcCtrl.pcProcBuff[stProcCtrl.u32WriteLen - 1] = '\n';
            }

            /* update the proc ctrl */
            stProcCtrl.pcProcBuff = &stProcCtrl.pcProcBuff[stProcCtrl.u32WriteLen];
            stProcCtrl.u32BuffLen = stProcCtrl.u32BuffLen - stProcCtrl.u32WriteLen;
            stProcCtrl.u32WriteLen = 0;
            if (0 == stProcCtrl.u32BuffLen) {
                break;
            }
        }
    }

    stProcCtrl.pcProcBuff[stProcCtrl.u32WriteLen] = '\0';
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_PROC_WRITE_OK);
    if (s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] write proc failed, ec %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_ProcExit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_VOID *pVirtAddr;
    ISP_PROC_S *pstProc = HI_NULL;

    PROC_GET_CTX(ViPipe, pstProc);

    ISP_CHECK_OPEN(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_PROC_PARAM_GET, &pstProc->u32ProcParam);
    if (s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get proc param %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    if (0 == pstProc->u32ProcParam) {
        return HI_SUCCESS;
    }

    if (HI_NULL != pstProc->stProcMem.pProcVirtAddr) {
        pVirtAddr = pstProc->stProcMem.pProcVirtAddr;
        pstProc->stProcMem.pProcVirtAddr = HI_NULL;
        HI_MPI_SYS_Munmap(pVirtAddr, pstProc->stProcMem.u32ProcSize);
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_PROC_EXIT);
    if (s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] exit proc ec %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_UpdateProcParam(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_PROC_S *pstProc = HI_NULL;

    PROC_GET_CTX(ViPipe, pstProc);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_PROC_PARAM_GET, &pstProc->u32ProcParam);
    if (s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get proc param %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
