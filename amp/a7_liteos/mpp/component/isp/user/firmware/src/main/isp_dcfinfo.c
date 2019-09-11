/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_dcfinfo.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2014/06/17
  Description   :
  History       :
  1.Date        : 2014/06/17
    Author      :
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <sys/ioctl.h>
#include "mkp_isp.h"
#include "mpi_sys.h"
#include "isp_dcfinfo.h"
#include "isp_ext_config.h"
#include "isp_main.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern HI_S32 g_as32IspFd[ISP_MAX_PIPE_NUM];

HI_S32 ISP_UpdateInfoInit(VI_PIPE ViPipe)
{
    HI_U64 u64PhyAddr;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    u64PhyAddr = pstIspCtx->stIspTransInfo.stUpdateInfo.u64PhyAddr;

    hi_ext_system_update_info_high_phyaddr_write(ViPipe, ((u64PhyAddr >> 32) & 0xFFFFFFFF));
    hi_ext_system_update_info_low_phyaddr_write(ViPipe, (u64PhyAddr & 0xFFFFFFFF));

    pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo = HI_MPI_SYS_Mmap(u64PhyAddr,
                                                                   (sizeof(ISP_DCF_UPDATE_INFO_S) * ISP_MAX_UPDATEINFO_BUF_NUM + sizeof(ISP_DCF_CONST_INFO_S)));

    if (HI_NULL == pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo) {
        ISP_TRACE(HI_DBG_ERR, "isp[%d] mmap update info buf failed!\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    pstIspCtx->stUpdateInfoCtrl.pstIspDCFConstInfo =
       (ISP_DCF_CONST_INFO_S *)(pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo + ISP_MAX_UPDATEINFO_BUF_NUM);

    return HI_SUCCESS;
}

HI_S32 ISP_UpdateInfoExit(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (HI_NULL != pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo) {
        HI_MPI_SYS_Munmap(pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo,
                          (sizeof(ISP_DCF_UPDATE_INFO_S) * ISP_MAX_UPDATEINFO_BUF_NUM + sizeof(ISP_DCF_CONST_INFO_S)));
        pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo = HI_NULL;
        pstIspCtx->stUpdateInfoCtrl.pstIspDCFConstInfo = HI_NULL;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
