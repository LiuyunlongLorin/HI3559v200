/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_debug.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/16
  Description   :
  History       :
  1.Date        : 2013/01/16
    Author      :
    Modification: Created file

******************************************************************************/

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <stdio.h>
#include "isp_main.h"
#include "hi_isp_debug.h"
#include "isp_debug.h"
#include "isp_ext_config.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "mpi_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 ISP_DbgSet(VI_PIPE ViPipe, const ISP_DEBUG_INFO_S *pstDbgInfo)
{
    HI_U32 u32Size = 0;

    if (hi_ext_system_sys_debug_enable_read(ViPipe) && pstDbgInfo->bDebugEn) {
        ISP_TRACE(HI_DBG_ERR, "Hisi isp has enabled debug info!\n");
        return HI_FAILURE;
    }

    if (pstDbgInfo->bDebugEn) {
        if (0 == pstDbgInfo->u64PhyAddr) {
            ISP_TRACE(HI_DBG_ERR, "Hisi isp lib's debug phyaddr is 0!\n");
            return HI_FAILURE;
        }

        if (0 == pstDbgInfo->u32Depth) {
            ISP_TRACE(HI_DBG_ERR, "Hisi ae lib's debug depth is 0!\n");
            return HI_FAILURE;
        }
        u32Size = sizeof(ISP_DBG_ATTR_S) +
                  sizeof(ISP_DBG_ATTR_S) * pstDbgInfo->u32Depth;
    }

    /* don't clear phyaddr and size when disable dbg info. */
    if (pstDbgInfo->bDebugEn) {
        hi_ext_system_sys_debug_enable_write(ViPipe, pstDbgInfo->bDebugEn);
        hi_ext_system_sys_debug_high_addr_write(ViPipe, ((pstDbgInfo->u64PhyAddr >> 32) & 0xFFFFFFFF));
        hi_ext_system_sys_debug_low_addr_write(ViPipe, (pstDbgInfo->u64PhyAddr & 0xFFFFFFFF));
        hi_ext_system_sys_debug_depth_write(ViPipe, pstDbgInfo->u32Depth);
        hi_ext_system_sys_debug_size_write(ViPipe, u32Size);
    } else {
        hi_ext_system_sys_debug_enable_write(ViPipe, pstDbgInfo->bDebugEn);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DbgGet(VI_PIPE ViPipe, ISP_DEBUG_INFO_S *pstDbgInfo)
{
    HI_U64 u64PhyAddrHigh;
    HI_U64 u64PhyAddrTemp;

    u64PhyAddrHigh  = (HI_U64)hi_ext_system_sys_debug_high_addr_read(ViPipe);
    u64PhyAddrTemp  = (HI_U64)hi_ext_system_sys_debug_low_addr_read(ViPipe);
    u64PhyAddrTemp |= (u64PhyAddrHigh << 32);

    pstDbgInfo->u64PhyAddr = u64PhyAddrTemp;
    pstDbgInfo->bDebugEn   = hi_ext_system_sys_debug_enable_read(ViPipe);
    pstDbgInfo->u32Depth   = hi_ext_system_sys_debug_depth_read(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_DbgRunBgn(ISP_DBG_CTRL_S *pstDbg, HI_U32 u32FrmCnt)
{
    // ISP_DBG_ATTR_S   *pstDbgAttr = HI_NULL;
    ISP_DBG_STATUS_S *pstDbgStatus = HI_NULL;

    if (!pstDbg->bDebugEn) {
        if (HI_NULL != pstDbg->pstDbgAttr) {
            HI_MPI_SYS_Munmap(pstDbg->pstDbgAttr, pstDbg->u32Size);
            pstDbg->pstDbgAttr = HI_NULL;
            pstDbg->pstDbgStatus = HI_NULL;
        }
        return HI_SUCCESS;
    }

    if ((pstDbg->bDebugEn) && (HI_NULL == pstDbg->pstDbgAttr)) {
        pstDbg->pstDbgAttr = (ISP_DBG_ATTR_S *)HI_MPI_SYS_Mmap(pstDbg->u64PhyAddr, pstDbg->u32Size);
        if (HI_NULL == pstDbg->pstDbgAttr) {
            ISP_TRACE(HI_DBG_ERR, "isp map debug buf failed!\n");
            return HI_FAILURE;
        }
        pstDbg->pstDbgStatus = (ISP_DBG_STATUS_S *)(pstDbg->pstDbgAttr + 1);

        // pstDbgAttr = pstDbg->pstDbgAttr;
        /* ------------- record attr ------------------ */
    }

    pstDbgStatus = pstDbg->pstDbgStatus + (u32FrmCnt % DIV_0_TO_1(pstDbg->u32Depth));

    pstDbgStatus->u32FrmNumBgn = u32FrmCnt;

    return HI_SUCCESS;
}

HI_S32 ISP_DbgRunEnd(ISP_DBG_CTRL_S *pstDbg, HI_U32 u32FrmCnt)
{
    ISP_DBG_STATUS_S *pstDbgStatus = HI_NULL;

    if ((!pstDbg->bDebugEn) || (HI_NULL == pstDbg->pstDbgStatus)) {
        return HI_SUCCESS;
    }

    pstDbgStatus = pstDbg->pstDbgStatus + (u32FrmCnt % DIV_0_TO_1(pstDbg->u32Depth));

    /* ------------- record status ------------------ */

    pstDbgStatus->u32FrmNumEnd = u32FrmCnt;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
