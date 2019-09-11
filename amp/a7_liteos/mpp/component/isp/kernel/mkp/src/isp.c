/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2010/08/17
  Description   :
  History       :
  1.Date        : 2010/08/17
    Author      :
    Modification: Created file

******************************************************************************/

#include "hi_osal.h"
#include "hi_common.h"
#include "hi_comm_isp.h"
#include "isp_drv_defines.h"
#include "mkp_isp.h"
#include "isp.h"
#include "isp_drv.h"
#include "isp_list.h"
#include "isp_drv_vreg.h"
#include "mm_ext.h"
#include "proc_ext.h"
#include "mod_ext.h"
#include "sys_ext.h"
#include "isp_ext.h"
#include "dev_ext.h"
#include "isp_reg_define.h"
#include "isp_stt_define.h"
#include "hi_i2c.h"
#include "hi_spi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


/****************************************************************************
 * MACRO DEFINITION                                                         *
 ****************************************************************************/

int  ISP_IrqRoute(VI_PIPE ViPipe);
int  ISP_DoIntBottomHalf(int irq, void *id);


/****************************************************************************
 * GLOBAL VARIABLES                                                         *
 ****************************************************************************/
ISP_DRV_CTX_S           g_astIspDrvCtx[ISP_MAX_PIPE_NUM] = {{0}};

unsigned int            isp_fe_irq = ISP_FE_IRQ_NR;

osal_spinlock_t         g_stIspLock[ISP_MAX_PIPE_NUM];
osal_spinlock_t         g_stIspSyncLock[ISP_MAX_PIPE_NUM];


static ISP_VERSION_S    gs_stIspLibInfo;


/* ISP ModParam info */
HI_U32                  g_PwmNumber[ISP_MAX_PIPE_NUM]  = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = 3};
HI_U32                  g_UpdatePos[ISP_MAX_PIPE_NUM]  = {0}; /* 0: frame start; 1: frame end */
HI_U32                  g_IntTimeout[ISP_MAX_PIPE_NUM] = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = 200}; /* The time(unit:ms) of interrupt timeout */
HI_U32                  g_StatIntvl[ISP_MAX_PIPE_NUM]  = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = 1};   /* update isp statistic information per stat-intval frame, purpose to reduce CPU load */
HI_U32                  g_ProcParam[ISP_MAX_PIPE_NUM]  = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = 30}; /* 0: close proc; n: write proc info's interval int num */
HI_U32                  g_PortIntDelay[ISP_MAX_PIPE_NUM] = {0}; /* Port intertupt delay value */
HI_BOOL                 g_IntBothalf = HI_FALSE;  /* 1 to enable interrupt processing at bottom half */
HI_BOOL                 g_QuickStart = HI_FALSE;  /* 1 to enable interrupt processing at bottom half */

HI_BOOL                 g_UseBothalf = HI_FALSE;  /* 1 to use interrupt processing at bottom half */
HI_U32                  g_IspExitTimeout = 2000;  /* The time(unit:ms) of exit be buffer timeout */
HI_U16                  g_au16ResMaxWidth[ISP_MAX_PIPE_NUM]  = {ISP_RES_WIDTH_MAX};
HI_U16                  g_au16ResMaxHeight[ISP_MAX_PIPE_NUM] = {ISP_RES_HEIGHT_MAX};

//#define TEST_TIME

#ifdef TEST_TIME
HI_U32  g_test_phyaddr;
HI_U16 *g_test_pviraddr;
struct osal_timeval time1;
struct osal_timeval time2;
#endif


/****************************************************************************
 * INTERNAL FUNCTION DECLARATION                                            *
 ****************************************************************************/

HI_S32 ISP_DRV_SetIntEnable(VI_PIPE ViPipe, HI_BOOL bEn)
{
    HI_S32 ViDev;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
    /* TODO:  pipe bind videv */
    ViDev = pstDrvCtx->stWDRAttr.ViDev;

    ISP_CHECK_VIR_PIPE(ViPipe);

    if (bEn)
    {
        if ((IS_FULL_WDR_MODE(pstDrvCtx->stWDRCfg.u8WDRMode))\
            || (IS_HALF_WDR_MODE(pstDrvCtx->stWDRCfg.u8WDRMode)))
        {
            IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_INT_MASK) |= VI_PT_INT_FSTART;
            IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_FSTART_DLY) = g_PortIntDelay[ViPipe];
        }
        IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_INT_MASK) |= VI_PT_INT_ERR;

        IO_RW_FE_ADDRESS(ViPipe, ISP_INT_FE) = 0xFF;
        IO_RW_FE_ADDRESS(ViPipe, ISP_INT_FE_MASK) |= ISP_INT_FE_FSTART;
        IO_RW_FE_ADDRESS(ViPipe, ISP_INT_FE_MASK) |= ISP_INT_FE_FEND;
    }
    else
    {
        if ((IS_FULL_WDR_MODE(pstDrvCtx->stWDRCfg.u8WDRMode))\
            || (IS_HALF_WDR_MODE(pstDrvCtx->stWDRCfg.u8WDRMode)))
        {
            IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_INT_MASK) &= (~VI_PT_INT_FSTART);
            IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_FSTART_DLY) = 0;
        }
        IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_INT) = 0xF;
        IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_INT_MASK) &= (~VI_PT_INT_ERR);

        IO_RW_FE_ADDRESS(ViPipe, ISP_INT_FE_MASK) &= (~ISP_INT_FE_FSTART);
        IO_RW_FE_ADDRESS(ViPipe, ISP_INT_FE_MASK) &= (~ISP_INT_FE_FEND);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_WaitConditionCallback(const HI_VOID *pParam)
{
    HI_BOOL bCondition;

    bCondition = *(HI_BOOL *)pParam;

    return (HI_TRUE == bCondition);
}

HI_S32 ISP_DRV_WaitExitCallback(const HI_VOID *pParam)
{
    HI_S32 bCondition;

    bCondition = *(HI_S32 *)pParam;

    return (0 == bCondition);
}


HI_S32 ISP_GetFrameEdge(VI_PIPE ViPipe, HI_U32 *pu32Status)
{
    unsigned long u32Flags;
    HI_S32 s32Ret;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pu32Status);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    s32Ret = osal_wait_event_timeout_interruptible(&pstDrvCtx->stIspWait, ISP_DRV_WaitConditionCallback, \
             &pstDrvCtx->bEdge, g_IntTimeout[ViPipe]);

    if (s32Ret <= 0)
    {
        *pu32Status = 0;
        ISP_TRACE(HI_DBG_WARN, "Get Interrupt timeout failed!\n");
        return HI_FAILURE;
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->bEdge = HI_FALSE;
    *pu32Status = pstDrvCtx->u32Status;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

static int ISP_GetVdStartTimeOut(VI_PIPE ViPipe, HI_U32 u32MilliSec, HI_U32 *pu32status)
{
    unsigned long u32Flags;
    HI_S32 s32Ret;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (osal_down_interruptible(&pstDrvCtx->stIspSem))
    {
        return -ERESTARTSYS;
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->bVdStart = HI_FALSE;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if (0 != u32MilliSec)
    {
        s32Ret = osal_wait_event_timeout_interruptible(&pstDrvCtx->stIspWaitVdStart, ISP_DRV_WaitConditionCallback, \
                 &pstDrvCtx->bVdStart, (u32MilliSec));

        if (s32Ret <= 0)
        {
            osal_up(&pstDrvCtx->stIspSem);
            return HI_FAILURE;
        }
    }
    else
    {
        s32Ret = osal_wait_event_interruptible(&pstDrvCtx->stIspWaitVdStart, ISP_DRV_WaitConditionCallback, \
                                               &pstDrvCtx->bVdStart);

        if (s32Ret)
        {
            osal_up(&pstDrvCtx->stIspSem);
            return HI_FAILURE;
        }

    }

    *pu32status = pstDrvCtx->u32Status;

    osal_up(&pstDrvCtx->stIspSem);

    return HI_SUCCESS;
}

static int ISP_GetVdEndTimeOut(VI_PIPE ViPipe, HI_U32 u32MilliSec, HI_U32 *pu32status)
{
    unsigned long u32Flags;
    HI_S32 s32Ret;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (osal_down_interruptible(&pstDrvCtx->stIspSemVd))
    {
        return -ERESTARTSYS;
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->bVdEnd = HI_FALSE;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if (0 != u32MilliSec)
    {
        s32Ret = osal_wait_event_timeout_interruptible(&pstDrvCtx->stIspWaitVdEnd, ISP_DRV_WaitConditionCallback, \
                 &pstDrvCtx->bVdEnd, (u32MilliSec));

        if (s32Ret <= 0)
        {
            osal_up(&pstDrvCtx->stIspSemVd);
            return HI_FAILURE;
        }
    }
    else
    {
        s32Ret = osal_wait_event_interruptible(&pstDrvCtx->stIspWaitVdEnd, ISP_DRV_WaitConditionCallback, \
                                               &pstDrvCtx->bVdEnd);

        if (s32Ret)
        {
            osal_up(&pstDrvCtx->stIspSemVd);
            return HI_FAILURE;
        }

    }

    *pu32status = pstDrvCtx->u32Status;

    osal_up(&pstDrvCtx->stIspSemVd);

    return HI_SUCCESS;
}

static int ISP_GetVdBeEndTimeOut(VI_PIPE ViPipe, HI_U32 u32MilliSec, HI_U32 *pu32status)
{
    unsigned long u32Flags;
    HI_S32 s32Ret;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (osal_down_interruptible(&pstDrvCtx->stIspSemBeVd))
    {
        return -ERESTARTSYS;
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->bVdBeEnd = HI_FALSE;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if (0 != u32MilliSec)
    {
        s32Ret = osal_wait_event_timeout_interruptible(&pstDrvCtx->stIspWaitVdBeEnd, ISP_DRV_WaitConditionCallback, \
                 &pstDrvCtx->bVdBeEnd, (u32MilliSec));

        if (s32Ret <= 0)
        {
            osal_up(&pstDrvCtx->stIspSemBeVd);
            return HI_FAILURE;
        }
    }
    else
    {
        s32Ret = osal_wait_event_interruptible(&pstDrvCtx->stIspWaitVdBeEnd, ISP_DRV_WaitConditionCallback, \
                                               &pstDrvCtx->bVdBeEnd);

        if (s32Ret)
        {
            osal_up(&pstDrvCtx->stIspSemBeVd);
            return HI_FAILURE;
        }

    }

    *pu32status = 1;

    osal_up(&pstDrvCtx->stIspSemBeVd);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_WorkModeInit(VI_PIPE ViPipe, ISP_BLOCK_ATTR_S  *pstBlkAttr)
{
    HI_S32  s32Ret = HI_SUCCESS;
    unsigned long u32Flags;
    ISP_RUNNING_MODE_E enIspRunningMode = ISP_MODE_RUNNING_OFFLINE;
    VI_PIPE_SPLIT_ATTR_S stPipeSplitAttr;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    FRAME_INTERRUPT_ATTR_S stFrameIntAttr = {0};

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstBlkAttr);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    pstDrvCtx->stWorkMode.u8PreBlockNum = pstDrvCtx->stWorkMode.u8BlockNum;

    osal_memset(&stPipeSplitAttr, 0, sizeof(VI_PIPE_SPLIT_ATTR_S));

    if (CKFN_VI_GetSplitAttr())
    {
        s32Ret = CALL_VI_GetSplitAttr(ViPipe, &stPipeSplitAttr);

        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe:%d CALL_VI_GetSplitAttr failed 0x%x!\n", ViPipe, s32Ret);
            return s32Ret;
        }
    }

    ISP_CHECK_BLOCK_NUM(stPipeSplitAttr.s32SplitNum);

    if (VI_PARALLEL_VPSS_OFFLINE == stPipeSplitAttr.enMode || VI_PARALLEL_VPSS_PARALLEL == stPipeSplitAttr.enMode)
    {
        enIspRunningMode = ISP_MODE_RUNNING_SIDEBYSIDE;
    }
    else if (VI_ONLINE_VPSS_OFFLINE == stPipeSplitAttr.enMode || VI_ONLINE_VPSS_ONLINE == stPipeSplitAttr.enMode)
    {
        enIspRunningMode = ISP_MODE_RUNNING_ONLINE;
    }
    else if (VI_OFFLINE_VPSS_OFFLINE == stPipeSplitAttr.enMode || VI_OFFLINE_VPSS_ONLINE == stPipeSplitAttr.enMode)
    {
        if (1 == stPipeSplitAttr.s32SplitNum)
        {
            enIspRunningMode = ISP_MODE_RUNNING_OFFLINE;
        }
        else
        {
            enIspRunningMode = ISP_MODE_RUNNING_STRIPING;
        }
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d Not support Mode: %d!\n", ViPipe, stPipeSplitAttr.enMode);
        return HI_FAILURE;
    }

    if ((PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stPipeSplitAttr.enPixFmt) || (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == stPipeSplitAttr.enPixFmt) ||
        (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == stPipeSplitAttr.enPixFmt) || (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == stPipeSplitAttr.enPixFmt) ||
        (PIXEL_FORMAT_YUV_SEMIPLANAR_444 == stPipeSplitAttr.enPixFmt) || (PIXEL_FORMAT_YVU_SEMIPLANAR_444 == stPipeSplitAttr.enPixFmt))
    {
        pstDrvCtx->bYuvMode = HI_TRUE;
        pstDrvCtx->stWorkMode.enIspDataInputMode = ISP_MODE_BT1120_YUV;
    }
    else
    {
        pstDrvCtx->bYuvMode = HI_FALSE;
        pstDrvCtx->stWorkMode.enIspDataInputMode = ISP_MODE_RAW;
    }
    if (CKFN_VI_GetViFrameInterruptAttr())
    {
        s32Ret = CALL_VI_GetViFrameInterruptAttr(ViPipe, &stFrameIntAttr);

        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe:%d Get Vi Int Attr err 0x%x!\n", ViPipe, s32Ret);
            return s32Ret;
        }
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->stWorkMode.u8BlockNum       = stPipeSplitAttr.s32SplitNum;
    pstDrvCtx->stWorkMode.enIspRunningMode = enIspRunningMode;
    pstDrvCtx->stWorkMode.u32OverLap       = stPipeSplitAttr.u32OverLap;

    pstBlkAttr->u8BlockNum       = stPipeSplitAttr.s32SplitNum;
    pstBlkAttr->u32OverLap       = stPipeSplitAttr.u32OverLap;
    pstBlkAttr->enIspRunningMode = enIspRunningMode;

    pstDrvCtx->stWorkMode.stFrameRect.u32Width  = stPipeSplitAttr.astWchOutRect.u32Width;
    pstDrvCtx->stWorkMode.stFrameRect.u32Height = stPipeSplitAttr.astWchOutRect.u32Height;

    osal_memcpy(&pstBlkAttr->stFrameRect, &pstDrvCtx->stWorkMode.stFrameRect, sizeof(SIZE_S));
    osal_memcpy(pstDrvCtx->stWorkMode.astBlockRect, stPipeSplitAttr.astRect, sizeof(RECT_S) * ISP_STRIPING_MAX_NUM);
    osal_memcpy(pstBlkAttr->astBlockRect, stPipeSplitAttr.astRect, sizeof(RECT_S) * ISP_STRIPING_MAX_NUM);
    pstDrvCtx->stFrameIntAttr = stFrameIntAttr;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_WorkModeExit(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_EXIT_STATE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    pstDrvCtx->stWorkMode.enIspRunningMode = ISP_MODE_RUNNING_OFFLINE;

    return HI_SUCCESS;
}

HI_U8 ISP_DRV_GetBlockNum(VI_PIPE ViPipe)
{
    return g_astIspDrvCtx[ViPipe].stWorkMode.u8BlockNum;
}

HI_S8 ISP_DRV_GetBlockId(VI_PIPE ViPipe, ISP_RUNNING_MODE_E enRunningMode)
{
    HI_S8 s8BlockId = 0;

    switch (enRunningMode)
    {
        default:
        case ISP_MODE_RUNNING_OFFLINE :
        case ISP_MODE_RUNNING_SIDEBYSIDE :
        case ISP_MODE_RUNNING_STRIPING :
            s8BlockId = 0;
            break;
        case ISP_MODE_RUNNING_ONLINE :
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
            break;
    }

    return s8BlockId;
}

HI_S32 ISP_DRV_GetBeRegsAttr(VI_PIPE ViPipe, S_ISPBE_REGS_TYPE *apstBeReg[], ISP_BE_REGS_ATTR_S *pstBlkAttr)
{
    HI_U8  k = 0;
    HI_S8  s8BlockId = 0;
    HI_U8  u8BlkDev = 0;
    HI_U8  u8BlkNum = 1;
    ISP_RUNNING_MODE_E enIspRunningMode;
    ISP_BE_WO_REG_CFG_S *pstIspBeRegCfg = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstBlkAttr);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
    enIspRunningMode = pstDrvCtx->stWorkMode.enIspRunningMode;
    u8BlkNum = ISP_DRV_GetBlockNum(ViPipe);
    u8BlkNum = DIV_0_TO_1(u8BlkNum);
    s8BlockId = ISP_DRV_GetBlockId(ViPipe, enIspRunningMode);
    if (-1 == s8BlockId)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Online Mode Pipe Err!\n", ViPipe);
        return HI_FAILURE;
    }

    u8BlkDev = (HI_U8)s8BlockId;
    pstBlkAttr->u8BlockDev = u8BlkDev;
    pstBlkAttr->u8BlockNum = u8BlkNum;

    switch (enIspRunningMode)
    {
        case ISP_MODE_RUNNING_ONLINE :
            ISP_DRV_BEREG_CTX(u8BlkDev, apstBeReg[u8BlkDev]);
            break;
        case ISP_MODE_RUNNING_OFFLINE :
            ISP_CHECK_BEBUF_INIT(ViPipe);
            if (HI_NULL == pstDrvCtx->pstUseNode)
            {
                ISP_TRACE(HI_DBG_WARN, "ISP[%d] UseNode is Null!\n", ViPipe);
                return HI_FAILURE;
            }
            pstIspBeRegCfg = (ISP_BE_WO_REG_CFG_S *)pstDrvCtx->pstUseNode->stBeCfgBuf.pVirAddr;
            apstBeReg[u8BlkDev] = &pstIspBeRegCfg->stBeRegCfg[u8BlkDev];
            break;
        case ISP_MODE_RUNNING_SIDEBYSIDE :
            for (k = 0; k < ISP_MAX_BE_NUM; k++)
            {
                ISP_DRV_BEREG_CTX(k, apstBeReg[k]);
            }
            break;
        case ISP_MODE_RUNNING_STRIPING :
            ISP_CHECK_BEBUF_INIT(ViPipe);
            if (HI_NULL == pstDrvCtx->pstUseNode)
            {
                ISP_TRACE(HI_DBG_WARN, "ISP[%d] UseNode is Null!\n", ViPipe);
                return HI_FAILURE;
            }
            pstIspBeRegCfg = (ISP_BE_WO_REG_CFG_S *)pstDrvCtx->pstUseNode->stBeCfgBuf.pVirAddr;
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                apstBeReg[k] = &pstIspBeRegCfg->stBeRegCfg[k];
            }
            break;
        default:
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] Running Mode Err!\n", ViPipe);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ChnSelectWrite(VI_PIPE ViPipe, HI_U32 u32ChannelSel)
{
    HI_U8   k;
    HI_U8   u8BlkDev = 0;
    HI_U8   u8BlkNum = 1;
    HI_U32  au32ChnSwitch[5] = {0};
    HI_U32  s32Ret = HI_SUCCESS;
    ISP_BE_REGS_ATTR_S stIspBeRegsAttr = {0};
    S_ISPBE_REGS_TYPE *apstBeReg[ISP_STRIPING_MAX_NUM] = {HI_NULL};
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_PIPE(ViPipe);

    s32Ret = ISP_DRV_GetBeRegsAttr(ViPipe, apstBeReg, &stIspBeRegsAttr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get BeRegs Attr Err!\n", ViPipe);
        return HI_FAILURE;
    }

    if (HI_TRUE == pstDrvCtx->bYuvMode)
    {
        au32ChnSwitch[4] = 1;
    }
    else
    {
        au32ChnSwitch[4] = 0;
    }

    u8BlkDev = stIspBeRegsAttr.u8BlockDev;
    u8BlkNum = stIspBeRegsAttr.u8BlockNum;

    switch (u32ChannelSel & 0x3)
    {
        case 0:
            au32ChnSwitch[0] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[0];
            au32ChnSwitch[1] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[1];
            au32ChnSwitch[2] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[2];
            au32ChnSwitch[3] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[3];
            break;

        case 1:
            au32ChnSwitch[1] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[0];
            au32ChnSwitch[0] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[1];
            au32ChnSwitch[2] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[2];
            au32ChnSwitch[3] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[3];
            break;

        case 2:
            au32ChnSwitch[2] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[0];
            au32ChnSwitch[1] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[1];
            au32ChnSwitch[0] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[2];
            au32ChnSwitch[3] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[3];
            break;

        case 3:
            au32ChnSwitch[3] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[0];
            au32ChnSwitch[2] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[1];
            au32ChnSwitch[1] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[2];
            au32ChnSwitch[0] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[3];
            break;
    }

    if (IS_FS_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
    {
        for (k = 0; k < u8BlkNum; k++)
        {
            ISP_DRV_SetInputSel(apstBeReg[k + u8BlkDev], &au32ChnSwitch[0]);
        }
    }

    return HI_SUCCESS;
}

/* ISP FE read sta*/
HI_S32 ISP_DRV_FE_StatisticsRead(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstStatInfo)
{
    HI_U8          u8ChnNumMax = 1;
    HI_U32         k;
    VI_PIPE        ViPipeBind = 0;
    ISP_STAT_KEY_U unStatkey;
    ISP_DRV_CTX_S     *pstDrvCtx = HI_NULL;
    ISP_STAT_S        *pstStat   = HI_NULL;
    S_ISPFE_REGS_TYPE *pstFeReg  = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_VIR_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstStatInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
    if (HI_FALSE == pstDrvCtx->stWDRAttr.bMastPipe)
    {
        return HI_SUCCESS;
    }

    pstStat = (ISP_STAT_S *)pstStatInfo->pVirtAddr;
    if (HI_NULL == pstStat)
    {
        return HI_FAILURE;
    }

    unStatkey.u64Key = pstStatInfo->unKey.bit32IsrAccess;

    u8ChnNumMax = MIN2(pstDrvCtx->stWDRAttr.stDevBindPipe.u32Num, ISP_WDR_CHN_MAX);

    for (k = 0; k < u8ChnNumMax; k++)
    {
        /*get side statistics*/
        ViPipeBind = pstDrvCtx->stWDRAttr.stDevBindPipe.PipeId[k];
        //osal_printk("ViPipeBind: %d\n", ViPipeBind);
        ISP_CHECK_PIPE(ViPipeBind);
        ISP_DRV_FEREG_CTX(ViPipeBind, pstFeReg);
        ISP_DRV_FE_APB_StatisticsRead(pstStat, pstFeReg, ViPipeBind, k, unStatkey);
        ISP_DRV_FE_STT_StatisticsRead(pstStat, pstFeReg, pstDrvCtx, ViPipeBind, k, unStatkey);
    }

    return HI_SUCCESS;
}

/* ISP BE read sta from FHY, online mode */
HI_S32 ISP_DRV_BeStatisticsRead(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstStatInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8  u8BlkDev, u8BlkNum;
    ISP_STAT_S *pstStat = HI_NULL;

    ISP_STAT_KEY_U unStatkey;
    ISP_BE_REGS_ATTR_S stIspBeRegsAttr = {0};
    S_ISPBE_REGS_TYPE *apstBeReg[ISP_STRIPING_MAX_NUM] = {HI_NULL};

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstStatInfo);

    s32Ret = ISP_DRV_GetBeRegsAttr(ViPipe, apstBeReg, &stIspBeRegsAttr);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    u8BlkDev = stIspBeRegsAttr.u8BlockDev;
    u8BlkNum = stIspBeRegsAttr.u8BlockNum;

    pstStat = (ISP_STAT_S *)pstStatInfo->pVirtAddr;

    if (HI_NULL == pstStat)
    {
        return HI_FAILURE;
    }

    unStatkey.u64Key = pstStatInfo->unKey.bit32IsrAccess;

    pstStat->bBEUpdate = HI_TRUE;

    ISP_DRV_BE_APB_StatisticsRead(pstStat, apstBeReg, u8BlkNum, u8BlkDev, unStatkey);
    ISP_DRV_BE_STT_StatisticsRead(ViPipe, pstStat, apstBeReg, u8BlkNum, u8BlkDev, unStatkey);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeOfflineStitchStatisticsRead(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstStatInfo)
{
    HI_S32 k = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8 u8BlkDev;

    ISP_STAT_KEY_U unStatkey;
    ISP_STAT_S *pstStat = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_STITCH_SYNC_BE_STATS_S stBeStitch;
    ISP_BE_REGS_ATTR_S  stIspBeRegsAttr = {0};
    S_ISPBE_REGS_TYPE   *apstBeReg[ISP_STRIPING_MAX_NUM] = {HI_NULL};

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstStatInfo);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
    unStatkey.u64Key = pstStatInfo->unKey.bit32IsrAccess;
    s32Ret = ISP_DRV_GetBeRegsAttr(ViPipe, apstBeReg, &stIspBeRegsAttr);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }
    u8BlkDev = stIspBeRegsAttr.u8BlockDev;

    if ((HI_TRUE != pstDrvCtx->stStitchAttr.bMainPipe) && (HI_TRUE != pstDrvCtx->stStitchAttr.bStitchEnable))
    {
        return  HI_SUCCESS;
    }

    pstStat = (ISP_STAT_S *)pstStatInfo->pVirtAddr;

    if (HI_NULL == pstStat)
    {
        return HI_FAILURE;
    }

    for (k = 0; k < pstDrvCtx->stStitchAttr.u8StitchPipeNum; k++)
    {

        stBeStitch.pstSyncBeStt[k] = (S_ISP_STITCH_STT_REGS_TYPE * )pstDrvCtx->stBeOffStitSttAttr.astBeStitchBuf[k].pVirAddr;

        if (HI_NULL == stBeStitch.pstSyncBeStt[k])
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] apstBeStt is NULL point\n", ViPipe);
            return HI_FAILURE;
        }
    }

    /* VI   CallBack Function */
    if (CKFN_VI_GetStitchSyncBeSttInfo())
    {
        s32Ret = CALL_VI_GetStitchSyncBeSttInfo(ViPipe,  &stBeStitch);

        if (HI_SUCCESS != s32Ret)
        {
            pstStat->bBEUpdate = HI_FALSE;
            ISP_TRACE(HI_DBG_WARN, "Pipe[%d] CALL_VI_GetStitchSyncBeSttInfo err 0x%x\n", ViPipe, s32Ret);
            return HI_FAILURE;
        }

        pstStat->bBEUpdate = HI_TRUE;
    }
    else
    {
        pstStat->bBEUpdate = HI_FALSE;
        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] CALL_VI_GetStitchSyncBeSttInfo is NULL\n", ViPipe);
        return HI_FAILURE;
    }

    if (HI_FALSE == pstStat->bBEUpdate)
    {
        pstDrvCtx->stDrvDbgInfo.u32IspBeStaLost++;
    }

    if (unStatkey.bit1BEAeStiGloStat)
    {
        s32Ret = ISP_DRV_BeOfflineAEStitchGlobalStatisticsRead(pstStat, pstDrvCtx);
    }

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] ISP_DRV_BeOfflineAEStitchGlobalStatisticsRead err 0x%x\n", ViPipe, s32Ret);
        return HI_FAILURE;
    }

    if (unStatkey.bit1BEAeStiLocStat)
    {
        s32Ret = ISP_DRV_BeOfflineAEStitchLocalStatisticsRead(pstStat, pstDrvCtx);
    }

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] ISP_DRV_BeOfflineAEStitchLocalStatisticsRead err 0x%x\n", ViPipe, s32Ret);
        return HI_FAILURE;
    }

    /* BE AWB statistics */
    s32Ret = ISP_DRV_BeOfflineAWBStitchStatisticsRead(pstStat, apstBeReg, pstDrvCtx, u8BlkDev, unStatkey);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] ISP_DRV_BeOfflineAWBStitchStatisticsRead err 0x%x\n", ViPipe, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/* ISP BE read sta from VI, offline mode */

HI_S32 ISP_DRV_BeOfflineStatisticsRead(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstStatInfo)
{
    HI_S32 k;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8  u8BlkNum, u8BlkDev;
    HI_S8  s8BlockId = 0;
    ISP_STAT_KEY_U unStatkey;
    ISP_STAT_S *pstStat = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    //ISP_BE_REGS_ATTR_S  stIspBeRegsAttr = {0};

    S_ISP_STT_REGS_TYPE *apstBeStt[ISP_STRIPING_MAX_NUM] = {[0 ... (ISP_STRIPING_MAX_NUM - 1)] = HI_NULL};
    ISP_RUNNING_MODE_E enIspRunningMode;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstStatInfo);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    u8BlkNum = ISP_DRV_GetBlockNum(ViPipe);
    u8BlkNum = DIV_0_TO_1(u8BlkNum);

    enIspRunningMode = pstDrvCtx->stWorkMode.enIspRunningMode;

    s8BlockId = ISP_DRV_GetBlockId(ViPipe, enIspRunningMode);

    if (-1 == s8BlockId)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Online Mode Pipe Err!\n", ViPipe);
        return HI_FAILURE;
    }

    u8BlkDev = (HI_U8)s8BlockId;

    pstStat = (ISP_STAT_S *)pstStatInfo->pVirtAddr;

    if (HI_NULL == pstStat)
    {
        return HI_FAILURE;
    }

    for (k = 0; k < u8BlkNum; k++)
    {
        apstBeStt[k] = (S_ISP_STT_REGS_TYPE *)pstDrvCtx->stBeOffSttAttr.astBeSttBuf[k].pVirAddr;
        if (HI_NULL == apstBeStt[k])
        {
            ISP_TRACE(HI_DBG_ERR, "apstBeStt is NULL point\n");
            return HI_FAILURE;
        }

        if (CKFN_VI_GetBeSttInfo())
        {
            s32Ret = CALL_VI_GetBeSttInfo(ViPipe, k, apstBeStt[k]);
            if (HI_SUCCESS != s32Ret)
            {
                pstStat->bBEUpdate = HI_FALSE;
                ISP_TRACE(HI_DBG_ERR, "Pipe[%d] CALL_VI_GetBeSttInfo err 0x%x\n", ViPipe, s32Ret);
                return HI_FAILURE;
            }

            pstStat->bBEUpdate = HI_TRUE;
        }
        else
        {
            pstStat->bBEUpdate = HI_FALSE;
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] VI_GetBeSttInfo is NULL\n", ViPipe);
            return HI_FAILURE;
        }
    }

    if (HI_FALSE == pstStat->bBEUpdate)
    {
        pstDrvCtx->stDrvDbgInfo.u32IspBeStaLost++;
    }

    unStatkey.u64Key = pstStatInfo->unKey.bit32IsrAccess;
    // BE comm statistics
    if (unStatkey.bit1CommStat)
    {

    }

    //BE AE statistics
    if (unStatkey.bit1BEAeGloStat)
    {
        ISP_DRV_BE_AE_Global_OfflineStatisticsRead(pstStat, apstBeStt, u8BlkNum);
    }

    if (unStatkey.bit1BEAeLocStat)
    {
        ISP_DRV_BE_AE_Local_OfflineStatisticsRead(pstStat, apstBeStt, u8BlkNum);
    }

    if (unStatkey.bit1MgStat)
    {
        ISP_DRV_BE_MG_OfflineStatisticsRead(pstStat, apstBeStt, u8BlkNum);
    }

    /* BE AWB statistics */

    ISP_DRV_BE_AWB_OfflineStatisticsRead(ViPipe, pstStat,  apstBeStt, u8BlkNum, u8BlkDev, unStatkey);


    /* BE AF statistics */
    if (unStatkey.bit1BEAfStat)
    {
        ISP_DRV_BE_AF_OfflineStatisticsRead(ViPipe, pstStat, apstBeStt, u8BlkNum);
    }

    if (unStatkey.bit1Dehaze)
    {
        ISP_DRV_BE_Dehaze_OfflineStatisticsRead(pstStat, apstBeStt, u8BlkNum);
    }

    if (unStatkey.bit1DpStat)
    {
        ISP_DRV_DPC_OfflineCalibInfoRead(pstStat, apstBeStt, u8BlkNum);
    }

    if (unStatkey.bit1Flicker)
    {
        ISP_DRV_Fliker_OfflineStatisticsRead(pstStat, apstBeStt, u8BlkNum);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StatisticsRead(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstStatInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstStatInfo);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    /*online snap, AE and AWB params set by the preview pipe.
      In order to get picture as fast as, dehaze don't used.*/
    if (IS_ONLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode) || \
        IS_SIDEBYSIDE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode))
    {
        if ((pstDrvCtx->stSnapAttr.s32PicturePipeId == ViPipe) &&
            (pstDrvCtx->stSnapAttr.s32PicturePipeId != pstDrvCtx->stSnapAttr.s32PreviewPipeId))
        {
            return HI_SUCCESS;
        }
    }

    if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable)
    {
        if (HI_TRUE == pstDrvCtx->stStitchAttr.bMainPipe)
        {
            ISP_DRV_FE_StitchStatisticsRead(ViPipe, pstStatInfo);
        }

        ISP_DRV_FE_StitchNonStatisticsRead(ViPipe, pstStatInfo);
    }
    else
    {
        s32Ret = ISP_DRV_FE_StatisticsRead(ViPipe, pstStatInfo);

        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "ISP_DRV_FE_StatisticsRead failed!\n");
            return HI_FAILURE;
        }
    }

    if (IS_ONLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode) || \
        IS_SIDEBYSIDE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode))
    {
        //BE statistics for online
        s32Ret = ISP_DRV_BeStatisticsRead(ViPipe, pstStatInfo);
        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "ISP_DRV_BeStatisticsRead failed!\n");
            return HI_FAILURE;
        }

        if ((HI_TRUE == pstDrvCtx->stStitchAttr.bMainPipe) && (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable))
        {
            ISP_DRV_BE_StitchStatisticsRead(ViPipe, pstStatInfo);
        }
    }
    else if (IS_OFFLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode) || \
             IS_STRIPING_MODE(pstDrvCtx->stWorkMode.enIspRunningMode))
    {
        //BE statistics for offline
        s32Ret = ISP_DRV_BeOfflineStatisticsRead(ViPipe, pstStatInfo);
        if (s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "ISP_DRV_BeOfflineStatisticsRead failed!\n");
            return HI_FAILURE;
        }

        if ((HI_TRUE == pstDrvCtx->stStitchAttr.bMainPipe) && (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable))
        {
            ISP_DRV_BeOfflineStitchStatisticsRead(ViPipe, pstStatInfo);
        }
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "enIspOnlineMode err 0x%x!\n", pstDrvCtx->stWorkMode.enIspRunningMode);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StatBufInit(VI_PIPE ViPipe, HI_U64 *pu64PhyAddr)
{
    HI_S32 s32Ret, i;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U64 u64PhyAddr, u64Size;
    HI_U8  *pu8VirAddr;
    HI_CHAR acName[MAX_MMZ_NAMELEN] = {0};
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pu64PhyAddr);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stStatisticsBuf.bInit);

    osal_snprintf(acName, sizeof(acName), "ISP[%d].Stat", ViPipe);

    u64Size = sizeof(ISP_STAT_S) * MAX_ISP_STAT_BUF_NUM;
    s32Ret = CMPI_MmzMallocCached(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, u64Size);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "alloc ISP statistics buf err\n");
        return HI_ERR_ISP_NOMEM;
    }

    osal_memset(pu8VirAddr, 0, u64Size);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->stStatisticsBuf.u64PhyAddr = u64PhyAddr;
    pstDrvCtx->stStatisticsBuf.pVirAddr = (HI_VOID *)pu8VirAddr;
    pstDrvCtx->stStatisticsBuf.u64Size  = u64Size;

    OSAL_INIT_LIST_HEAD(&pstDrvCtx->stStatisticsBuf.stFreeList);
    OSAL_INIT_LIST_HEAD(&pstDrvCtx->stStatisticsBuf.stBusyList);
    OSAL_INIT_LIST_HEAD(&pstDrvCtx->stStatisticsBuf.stUserList);

    for (i = 0; i < MAX_ISP_STAT_BUF_NUM; i++)
    {
        pstDrvCtx->stStatisticsBuf.astNode[i].stStatInfo.u64PhyAddr   =
            u64PhyAddr + i * sizeof(ISP_STAT_S);
        pstDrvCtx->stStatisticsBuf.astNode[i].stStatInfo.pVirtAddr    =
            (HI_VOID *)(pu8VirAddr + i * sizeof(ISP_STAT_S));

        pstDrvCtx->stStatisticsBuf.astNode[i].stStatInfo.unKey.u64Key =
            ((HI_U64)ISP_STATISTICS_KEY << 32) + ISP_STATISTICS_KEY;

        osal_list_add_tail(&pstDrvCtx->stStatisticsBuf.astNode[i].list,
                           &pstDrvCtx->stStatisticsBuf.stFreeList);
    }

    pstDrvCtx->stStatisticsBuf.bInit      = HI_TRUE;
    pstDrvCtx->stStatisticsBuf.u32BusyNum = 0;
    pstDrvCtx->stStatisticsBuf.u32UserNum = 0;
    pstDrvCtx->stStatisticsBuf.u32FreeNum = MAX_ISP_STAT_BUF_NUM;

    *pu64PhyAddr = pstDrvCtx->stStatisticsBuf.u64PhyAddr;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StatBufExit(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U64 u64PhyAddr;
    HI_U8  *pu8VirAddr;
    //HI_U64 u64Size;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_EXIT_STATE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stStatisticsBuf.bInit);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    u64PhyAddr = pstDrvCtx->stStatisticsBuf.u64PhyAddr;
    pu8VirAddr = (HI_U8 *)pstDrvCtx->stStatisticsBuf.pVirAddr;
    //u64Size    = pstDrvCtx->stStatisticsBuf.u64Size;

    pstDrvCtx->stStatisticsBuf.pVirAddr = HI_NULL;
    pstDrvCtx->stStatisticsBuf.astNode[0].stStatInfo.pVirtAddr = HI_NULL;
    pstDrvCtx->stStatisticsBuf.astNode[1].stStatInfo.pVirtAddr = HI_NULL;
    pstDrvCtx->stStatisticsBuf.u64PhyAddr = 0;
    pstDrvCtx->stStatisticsBuf.astNode[0].stStatInfo.u64PhyAddr = 0;
    pstDrvCtx->stStatisticsBuf.astNode[1].stStatInfo.u64PhyAddr = 0;
    pstDrvCtx->stStatisticsBuf.bInit    = HI_FALSE;

    OSAL_INIT_LIST_HEAD(&pstDrvCtx->stStatisticsBuf.stFreeList);
    OSAL_INIT_LIST_HEAD(&pstDrvCtx->stStatisticsBuf.stBusyList);
    OSAL_INIT_LIST_HEAD(&pstDrvCtx->stStatisticsBuf.stUserList);
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if (0 != u64PhyAddr)
    {
        //hil_mmb_invalid_cache_byaddr(pu8VirAddr, u64PhyAddr, u64Size);
        CMPI_MmzFree(u64PhyAddr, pu8VirAddr);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StatBufUserGet(VI_PIPE ViPipe, ISP_STAT_INFO_S **ppstStatInfo)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    struct osal_list_head *plist;
    ISP_STAT_NODE_S *pstNode = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_STABUF_INIT(ViPipe);
    ISP_CHECK_POINTER(ppstStatInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (osal_list_empty(&pstDrvCtx->stStatisticsBuf.stBusyList))
    {
        ISP_TRACE(HI_DBG_WARN, "busy list empty\n");
        *ppstStatInfo = HI_NULL;
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_FAILURE;
    }

    /* get busy */
    plist = pstDrvCtx->stStatisticsBuf.stBusyList.next;
    if (HI_NULL == plist)
    {
        ISP_TRACE(HI_DBG_WARN, "busy list empty\n");
        *ppstStatInfo = HI_NULL;
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_FAILURE;
    }
    osal_list_del(plist);
    pstDrvCtx->stStatisticsBuf.u32BusyNum--;

    /* return info */
    pstNode = osal_list_entry(plist, ISP_STAT_NODE_S, list);
    *ppstStatInfo = &pstNode->stStatInfo;

    /* put user */
    osal_list_add_tail(plist, &pstDrvCtx->stStatisticsBuf.stUserList);
    pstDrvCtx->stStatisticsBuf.u32UserNum++;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StatBufUserPut(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstStatInfo)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    struct osal_list_head *plist = HI_NULL;
    ISP_STAT_NODE_S *pstNode = HI_NULL;
    HI_BOOL bValid = HI_FALSE;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_STABUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstStatInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    osal_list_for_each(plist, &pstDrvCtx->stStatisticsBuf.stUserList)
    {
        pstNode = osal_list_entry(plist, ISP_STAT_NODE_S, list);
        //  HI_ASSERT(HI_NULL != pstNode);
        if (HI_NULL == pstNode)
        {
            ISP_TRACE(HI_DBG_ERR, "pstNode  null pointer\n");
            break;
        }

        if (pstNode->stStatInfo.u64PhyAddr == pstStatInfo->u64PhyAddr)
        {
            bValid = HI_TRUE;
            pstNode->stStatInfo.unKey.u64Key = pstStatInfo->unKey.u64Key;
            //pstDrvCtx->u32bit16IsrAccess = pstStatInfo->unKey.bit16IsrAccess;
            break;
        }
    }

    if (!bValid)
    {
        ISP_TRACE(HI_DBG_ERR, "invalid stat info, phy 0x%llx\n", pstStatInfo->u64PhyAddr);
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_FAILURE;
    }

    /* get user */
    //HI_ASSERT(plist != HI_NULL);
    if (HI_NULL == plist)
    {
        ISP_TRACE(HI_DBG_ERR, "user list empty\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_FAILURE;
    }
    osal_list_del(plist);
    pstDrvCtx->stStatisticsBuf.u32UserNum--;

    /* put free */
    osal_list_add_tail(plist, &pstDrvCtx->stStatisticsBuf.stFreeList);
    pstDrvCtx->stStatisticsBuf.u32FreeNum++;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StatBufBusyPut(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    struct osal_list_head *plist;
    ISP_STAT_NODE_S *pstNode = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_STABUF_INIT(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    /* There should be one frame of the newest statistics info in busy list. */
    while (!osal_list_empty(&pstDrvCtx->stStatisticsBuf.stBusyList))
    {
        plist = pstDrvCtx->stStatisticsBuf.stBusyList.next;
        //HI_ASSERT(plist != HI_NULL);
        if (HI_NULL == plist)
        {
            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
            return HI_FAILURE;
        }
        osal_list_del(plist);
        pstDrvCtx->stStatisticsBuf.u32BusyNum--;

        osal_list_add_tail(plist, &pstDrvCtx->stStatisticsBuf.stFreeList);
        pstDrvCtx->stStatisticsBuf.u32FreeNum++;
    }

    if (osal_list_empty(&pstDrvCtx->stStatisticsBuf.stFreeList))
    {
        ISP_TRACE(HI_DBG_WARN, "free list empty\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

        return HI_FAILURE;
    }

    /* get free */
    plist = pstDrvCtx->stStatisticsBuf.stFreeList.next;
    //HI_ASSERT(plist != HI_NULL);
    if (HI_NULL == plist)
    {
        ISP_TRACE(HI_DBG_WARN, "free list empty\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_FAILURE;
    }
    osal_list_del(plist);
    pstDrvCtx->stStatisticsBuf.u32FreeNum--;

    /* read statistics */
    pstNode = osal_list_entry(plist, ISP_STAT_NODE_S, list);

    pstDrvCtx->stStatisticsBuf.pstActStat = &pstNode->stStatInfo;
    //pstNode->stStatInfo.unKey.u32Key = 0xffff;
    ISP_DRV_StatisticsRead(ViPipe, &pstNode->stStatInfo);

    /* put busy */
    osal_list_add_tail(plist, &pstDrvCtx->stStatisticsBuf.stBusyList);
    pstDrvCtx->stStatisticsBuf.u32BusyNum++;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ClutBufInit(VI_PIPE ViPipe)
{
    HI_U64 u64PhyAddr, u64Size;
    HI_U8 *pu8VirAddr;
    ISP_DRV_CTX_S      *pstDrvCtx = HI_NULL;
    HI_CHAR acName[MAX_MMZ_NAMELEN] = {0};

    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stClutBufAttr.bInit);

    osal_snprintf(acName, sizeof(acName), "ISP[%d].Clut", ViPipe);

    u64Size =  HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32);

    if (HI_SUCCESS != CMPI_MmzMallocNocache(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, u64Size))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] MmzMalloc Clut buffer Failure!\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    osal_memset(pu8VirAddr, 0, u64Size);

    pstDrvCtx->stClutBufAttr.bInit                = HI_TRUE;
    pstDrvCtx->stClutBufAttr.stClutBuf.u64Size    = u64Size;
    pstDrvCtx->stClutBufAttr.stClutBuf.u64PhyAddr = u64PhyAddr;
    pstDrvCtx->stClutBufAttr.stClutBuf.pVirAddr   = (HI_VOID *)pu8VirAddr;

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ClutBufExit(VI_PIPE ViPipe)
{
    HI_U64   u64PhyAddr;
    HI_VOID *pVirAddr;
    ISP_DRV_CTX_S      *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_EXIT_STATE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stClutBufAttr.bInit);

    u64PhyAddr = pstDrvCtx->stClutBufAttr.stClutBuf.u64PhyAddr;
    pVirAddr   = pstDrvCtx->stClutBufAttr.stClutBuf.pVirAddr;

    pstDrvCtx->stClutBufAttr.bInit                = HI_FALSE;
    pstDrvCtx->stClutBufAttr.stClutBuf.u64Size    = 0;
    pstDrvCtx->stClutBufAttr.stClutBuf.u64PhyAddr = 0;
    pstDrvCtx->stClutBufAttr.stClutBuf.pVirAddr   = HI_NULL;

    if (0 != u64PhyAddr)
    {
        CMPI_MmzFree(u64PhyAddr, pVirAddr);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_SpecAwbBufInit(VI_PIPE ViPipe)
{
#ifdef CONFIG_HI_ISP_SPECAWB_SUPPORT
    HI_U64 u64PhyAddr, u64Size;
    HI_U8 *pu8VirAddr;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_CHAR acName[MAX_MMZ_NAMELEN] = {0};

    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stSpecAwbBufAttr.bInit);

    osal_snprintf(acName, sizeof(acName), "ISP[%d].SpecAwb", ViPipe);

    u64Size =  ISP_SPECAWB_BUF_SIZE;

    if (HI_SUCCESS != CMPI_MmzMallocNocache(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, u64Size))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] MmzMalloc SpecAwb buffer Failure!\n", ViPipe);
        return HI_FAILURE;
    }

    osal_memset(pu8VirAddr, 0, u64Size);

    pstDrvCtx->stSpecAwbBufAttr.bInit                   = HI_TRUE;
    pstDrvCtx->stSpecAwbBufAttr.stSpecAwbBuf.u64Size    = u64Size;
    pstDrvCtx->stSpecAwbBufAttr.stSpecAwbBuf.u64PhyAddr = u64PhyAddr;
    pstDrvCtx->stSpecAwbBufAttr.stSpecAwbBuf.pVirAddr   = (HI_VOID *)pu8VirAddr;
#endif
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_SpecAwbBufExit(VI_PIPE ViPipe)
{
#ifdef CONFIG_HI_ISP_SPECAWB_SUPPORT
    HI_U64   u64PhyAddr;
    HI_VOID *pVirAddr;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_EXIT_STATE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stSpecAwbBufAttr.bInit);

    u64PhyAddr = pstDrvCtx->stSpecAwbBufAttr.stSpecAwbBuf.u64PhyAddr;
    pVirAddr   = pstDrvCtx->stSpecAwbBufAttr.stSpecAwbBuf.pVirAddr;

    pstDrvCtx->stSpecAwbBufAttr.bInit                   = HI_FALSE;
    pstDrvCtx->stSpecAwbBufAttr.stSpecAwbBuf.u64Size    = 0;
    pstDrvCtx->stSpecAwbBufAttr.stSpecAwbBuf.u64PhyAddr = 0;
    pstDrvCtx->stSpecAwbBufAttr.stSpecAwbBuf.pVirAddr   = HI_NULL;

    if (0 != u64PhyAddr)
    {
        CMPI_MmzFree(u64PhyAddr, pVirAddr);
    }
#endif
    return HI_SUCCESS;
}


HI_S32 ISP_DRV_BeBufInit(VI_PIPE ViPipe, HI_U64 *pu64PhyAddr)
{
    HI_S32 s32Ret;
    HI_U32 i;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_BE_BUF_NODE_S *pstNode = HI_NULL;
    HI_U64 u64PhyAddr, u64Size;
    HI_U8  *pu8VirAddr;
    HI_CHAR acName[MAX_MMZ_NAMELEN] = {0};
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pu64PhyAddr);
    ISP_CHECK_ONLINE_MODE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stBeBufInfo.bInit);

    osal_snprintf(acName, sizeof(acName), "ISP[%d].BeCfg", ViPipe);

    u64Size = sizeof(ISP_BE_WO_REG_CFG_S);

    s32Ret = CMPI_MmzMallocCached(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, u64Size * MAX_ISP_BE_BUF_NUM);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] alloc ISP BeCfgBuf err!\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    osal_memset(pu8VirAddr, 0, u64Size * MAX_ISP_BE_BUF_NUM);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->stBeBufInfo.bInit                   = HI_TRUE;
    pstDrvCtx->stBeBufInfo.stBeBufHaddr.u64PhyAddr = u64PhyAddr;
    pstDrvCtx->stBeBufInfo.stBeBufHaddr.pVirAddr   = (HI_VOID *)pu8VirAddr;
    pstDrvCtx->stBeBufInfo.stBeBufHaddr.u64Size    = u64Size * MAX_ISP_BE_BUF_NUM;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    s32Ret = ISP_CreatBeBufQueue(&pstDrvCtx->stBeBufQueue, MAX_ISP_BE_BUF_NUM);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] CreatBeBufQueue fail!\n", ViPipe);

        goto fail0;
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    for (i = 0; i < MAX_ISP_BE_BUF_NUM; i++)
    {
        pstNode = ISP_QueueGetFreeBeBuf(&pstDrvCtx->stBeBufQueue);

        if (HI_NULL == pstNode)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] Get QueueGetFreeBeBuf fail!\r\n", ViPipe);
            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
            goto fail1;
        }

        pstNode->stBeCfgBuf.u64PhyAddr = u64PhyAddr + i * u64Size;
        pstNode->stBeCfgBuf.pVirAddr   = (HI_VOID *)(pu8VirAddr + i * u64Size);
        pstNode->stBeCfgBuf.u64Size    = u64Size;
        pstNode->s32HoldCnt            = 0;

        ISP_QueuePutFreeBeBuf(&pstDrvCtx->stBeBufQueue, pstNode);
    }

    pstDrvCtx->pstUseNode        = HI_NULL;
    pstDrvCtx->enIspRunningState = ISP_BE_BUF_STATE_INIT;
    pstDrvCtx->enIspExitState    = ISP_BE_BUF_READY;

    *pu64PhyAddr = pstDrvCtx->stBeBufInfo.stBeBufHaddr.u64PhyAddr;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;

fail1:
    ISP_DestroyBeBufQueue(&pstDrvCtx->stBeBufQueue);

fail0:
    pstDrvCtx->stBeBufInfo.bInit                   = HI_FALSE;
    pstDrvCtx->stBeBufInfo.stBeBufHaddr.u64PhyAddr = 0;
    pstDrvCtx->stBeBufInfo.stBeBufHaddr.pVirAddr   = HI_NULL;
    pstDrvCtx->stBeBufInfo.stBeBufHaddr.u64Size    = 0;

    if (0 != u64PhyAddr)
    {
        //hil_mmb_invalid_cache_byaddr(pu8VirAddr, u64PhyAddr, u64Size);
        CMPI_MmzFree(u64PhyAddr, (HI_VOID *)pu8VirAddr);
    }

    return HI_FAILURE;
}

HI_S32 ISP_DRV_BeBufExit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_U64 u64PhyAddr;//, u64Size;
    HI_VOID *pVirAddr;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_ONLINE_MODE(ViPipe);
    ISP_CHECK_EXIT_STATE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stBeBufInfo.bInit);

    u64PhyAddr = pstDrvCtx->stBeBufInfo.stBeBufHaddr.u64PhyAddr;
    pVirAddr   = pstDrvCtx->stBeBufInfo.stBeBufHaddr.pVirAddr;
    //u64Size    = pstDrvCtx->stBeBufInfo.stBeBufHaddr.u64Size;

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->enIspExitState = ISP_BE_BUF_WAITING;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if(CHECK_FUNC_ENTRY(HI_ID_VI) && CKFN_VI_UpdateViVpssMode())
    {
        /* Note: this function cannot be placed in the ISP lock, otherwise it will be deadlocked. */
        CALL_VI_IspClearInputQueue(ViPipe);
    }

    s32Ret = osal_wait_event_timeout_interruptible(&pstDrvCtx->stIspExitWait, ISP_DRV_WaitExitCallback, \
             &pstDrvCtx->stBeBufInfo.s32UseCnt, g_IspExitTimeout);

    if (s32Ret <= 0)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d isp exit wait failed:s32Ret:%d!\n", ViPipe, s32Ret);
        return HI_FAILURE;
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    ISP_DestroyBeBufQueue(&pstDrvCtx->stBeBufQueue);

    pstDrvCtx->stBeBufInfo.bInit                   = HI_FALSE;
    pstDrvCtx->stBeBufInfo.stBeBufHaddr.u64PhyAddr = 0;
    pstDrvCtx->stBeBufInfo.stBeBufHaddr.pVirAddr   = HI_NULL;
    pstDrvCtx->stBeBufInfo.stBeBufHaddr.u64Size    = 0;
    pstDrvCtx->enIspExitState                      = ISP_BE_BUF_EXIT;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if (0 != u64PhyAddr)
    {
        //hil_mmb_invalid_cache_byaddr(pVirAddr, u64PhyAddr, u64Size);
        CMPI_MmzFree(u64PhyAddr, pVirAddr);
    }

    return HI_SUCCESS;
}

osal_spinlock_t *ISP_DRV_GetSpinLock(VI_PIPE ViPipe)
{
    VI_PIPE MainPipeS;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_TRUE != pstDrvCtx->stStitchAttr.bStitchEnable)
    {
        return &g_stIspLock[ViPipe];
    }
    else
    {
        MainPipeS = pstDrvCtx->stStitchAttr.as8StitchBindId[0];
        return &g_stIspSyncLock[MainPipeS];
    }
}

HI_S32 ISP_DRV_GetReadyBeBuf(VI_PIPE ViPipe, ISP_BE_WO_CFG_BUF_S *pstBeCfgBuf)
{
    osal_spinlock_t *pIspSpinLock;
    unsigned long u32Flags;
    ISP_BE_BUF_NODE_S *pstNode = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstBeCfgBuf);
    ISP_CHECK_ONLINE_MODE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    //ISP_DRV_GetSpinLock(ViPipe, &IspSpinLock);
    pIspSpinLock = ISP_DRV_GetSpinLock(ViPipe);
    osal_spin_lock_irqsave(pIspSpinLock, &u32Flags);

    if ((ISP_BE_BUF_EXIT == pstDrvCtx->enIspExitState) ||
        (ISP_BE_BUF_WAITING == pstDrvCtx->enIspExitState))
    {
        ISP_TRACE(HI_DBG_ERR, "ViPipe[%d] ISP BE Buf not existed!!!\n", ViPipe);
        osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);
        return HI_FAILURE;
    }

    pstNode = ISP_QueueQueryBusyBeBuf(&pstDrvCtx->stBeBufQueue);
    if (HI_NULL == pstNode)
    {
        ISP_TRACE(HI_DBG_ERR, "ViPipe[%d] QueueQueryBusyBeBuf pstNode is null!\n", ViPipe);
        osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);
        return HI_FAILURE;
    }

    pstNode->s32HoldCnt++;
    pstDrvCtx->stBeBufInfo.s32UseCnt++;

    osal_memcpy(pstBeCfgBuf, &pstNode->stBeCfgBuf, sizeof(ISP_BE_WO_CFG_BUF_S));
    pstDrvCtx->enIspExitState = ISP_BE_BUF_READY;

    osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);

    return HI_SUCCESS;

}

HI_VOID ISP_DRV_PutBusyToFree(VI_PIPE ViPipe, ISP_BE_WO_CFG_BUF_S *pstBeCfgBuf)
{
    ISP_BE_BUF_NODE_S *pstNode;
    struct osal_list_head *pListTmp = HI_NULL;
    struct osal_list_head *pListNode = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_list_for_each_safe(pListNode, pListTmp, &pstDrvCtx->stBeBufQueue.stBusyList)
    {
        pstNode = osal_list_entry(pListNode, ISP_BE_BUF_NODE_S, list);

        if (pstNode->stBeCfgBuf.u64PhyAddr == pstBeCfgBuf->u64PhyAddr)
        {
            if (pstNode->s32HoldCnt > 0)
            {
                pstNode->s32HoldCnt--;
            }

            if (0 == pstNode->s32HoldCnt)
            {
                if (ISP_QueueGetBusyNum(&pstDrvCtx->stBeBufQueue) > 1)
                {
                    ISP_QueueDelBusyBeBuf(&pstDrvCtx->stBeBufQueue, pstNode);
                    ISP_QueuePutFreeBeBuf(&pstDrvCtx->stBeBufQueue, pstNode);
                }
            }
        }
    }

    return;
}

HI_S32 ISP_DRV_PutFreeBeBuf(VI_PIPE ViPipe, ISP_BE_WO_CFG_BUF_S *pstBeCfgBuf)
{
    osal_spinlock_t *pIspSpinLock = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstBeCfgBuf);
    ISP_CHECK_ONLINE_MODE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    //ISP_DRV_GetSpinLock(ViPipe, &IspSpinLock);
    pIspSpinLock = ISP_DRV_GetSpinLock(ViPipe);
    osal_spin_lock_irqsave(pIspSpinLock, &u32Flags);

    if (ISP_BE_BUF_EXIT == pstDrvCtx->enIspExitState)
    {
        ISP_TRACE(HI_DBG_ERR, "ViPipe[%d] ISP BE Buf not existed!!!\n", ViPipe);
        osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);
        return HI_FAILURE;
    }

    ISP_DRV_PutBusyToFree(ViPipe, pstBeCfgBuf);

    if (pstDrvCtx->stBeBufInfo.s32UseCnt > 0)
    {
        pstDrvCtx->stBeBufInfo.s32UseCnt--;
    }

    osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);

    osal_wakeup(&pstDrvCtx->stIspExitWait);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_HoldBusyBeBuf(VI_PIPE ViPipe, ISP_BE_WO_CFG_BUF_S *pstBeCfgBuf)
{
    osal_spinlock_t *pIspSpinLock = HI_NULL;
    ISP_BE_BUF_NODE_S *pstNode;
    struct osal_list_head *pListTmp = HI_NULL;
    struct osal_list_head *pListNode = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;
    unsigned long  u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstBeCfgBuf);
    ISP_CHECK_ONLINE_MODE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    //ISP_DRV_GetSpinLock(ViPipe, &IspSpinLock);
    pIspSpinLock = ISP_DRV_GetSpinLock(ViPipe);
    osal_spin_lock_irqsave(pIspSpinLock, &u32Flags);

    if ((ISP_BE_BUF_EXIT == pstDrvCtx->enIspExitState) ||
        (ISP_BE_BUF_WAITING == pstDrvCtx->enIspExitState))
    {
        ISP_TRACE(HI_DBG_ERR, "ViPipe[%d] ISP BE Buf not existed!!!\n", ViPipe);
        osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);
        return HI_FAILURE;
    }

    osal_list_for_each_safe(pListNode, pListTmp, &pstDrvCtx->stBeBufQueue.stBusyList)
    {
        pstNode = osal_list_entry(pListNode, ISP_BE_BUF_NODE_S, list);

        if (pstNode->stBeCfgBuf.u64PhyAddr == pstBeCfgBuf->u64PhyAddr)
        {
            pstNode->s32HoldCnt++;
            pstDrvCtx->stBeBufInfo.s32UseCnt++;
        }
    }

    pstDrvCtx->enIspExitState = ISP_BE_BUF_READY;

    osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetBeSyncPara(VI_PIPE ViPipe, ISP_BE_SYNC_PARA_S *pstBeSyncPara)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstBeSyncPara);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable )
    {
        s32Ret = ISP_DRV_GetStitchBeSyncPara(ViPipe, pstBeSyncPara);

        if (HI_SUCCESS != s32Ret)
        {
            return HI_FAILURE;
        }
    }
    else
    {
        osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
        osal_memcpy(pstBeSyncPara, &pstDrvCtx->stIspBeSyncPara, sizeof(ISP_BE_SYNC_PARA_S));
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeBufSwitchState(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    osal_spinlock_t *pIspSpinLock = HI_NULL;

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    //ISP_DRV_GetSpinLock(ViPipe, &IspSpinLock);
    pIspSpinLock = ISP_DRV_GetSpinLock(ViPipe);

    osal_spin_lock_irqsave(pIspSpinLock, &u32Flags);

    if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable)
    {
        pstDrvCtx->enIspRunningState = ISP_BE_BUF_STATE_SWITCH;
    }

    osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);

    return HI_SUCCESS;
}


HI_S32 ISP_DRV_SwitchMode(VI_PIPE ViPipe, ISP_DRV_CTX_S *pstDrvCtx)
{
    HI_U8   i, j, k;
    HI_U8   u8BlkDev = 0;
    HI_U8   u8BlkNum = 1;
    HI_S32  ViDev;
    HI_U32  s32Ret = HI_SUCCESS;
    HI_U32  au32ChnSwitch[5] = {0};
    ISP_SYNC_CFG_S *pstSyncCfg = HI_NULL;
    ISP_BE_REGS_ATTR_S stIspBeRegsAttr = {0};
    S_ISPBE_REGS_TYPE *apstBeReg[ISP_STRIPING_MAX_NUM] = {HI_NULL};
    ISP_SYNC_CFG_BUF_NODE_S *pstCfgNode = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstDrvCtx);
    ViDev = pstDrvCtx->stWDRAttr.ViDev;

    if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable)
    {
        pstDrvCtx->enIspRunningState = ISP_BE_BUF_STATE_SWITCH;
    }

    s32Ret = ISP_DRV_GetBeRegsAttr(ViPipe, apstBeReg, &stIspBeRegsAttr);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_SUCCESS;
    }

    u8BlkDev = stIspBeRegsAttr.u8BlockDev;
    u8BlkNum = stIspBeRegsAttr.u8BlockNum;

    pstSyncCfg = &pstDrvCtx->stSyncCfg;
    pstSyncCfg->u8WDRMode = pstDrvCtx->stWDRCfg.u8WDRMode;

    for (j = 0; j < 3; j++)
    {
        for (i = 0; i < CFG2VLD_DLY_LIMIT; i++)
        {
            pstSyncCfg->u32ExpRatio[j][i] = pstDrvCtx->stWDRCfg.au32ExpRatio[j];
        }
    }

    pstCfgNode = pstDrvCtx->stSyncCfg.apstNode[0];
    if (HI_NULL != pstCfgNode)
    {
        /* wdr fusion mode need update */
        osal_memcpy(&pstCfgNode->stWDRRegCfg, &pstDrvCtx->stWDRCfg.stWdrRegCfg, sizeof(ISP_FSWDR_SYNC_CFG_S));
        ISP_DRV_RegConfigWdr(ViPipe, apstBeReg, pstDrvCtx, &pstCfgNode->stWDRRegCfg, u8BlkNum, u8BlkDev, pstDrvCtx->stWDRCfg.au32ExpRatio);
        ISP_DRV_RegConfigDrc(ViPipe, apstBeReg, pstDrvCtx, &pstCfgNode->stDRCRegCfg, u8BlkNum, u8BlkDev, pstDrvCtx->stWDRCfg.au32ExpRatio);
    }

    /* init cfg when modes change */
    {
        osal_memset(&pstDrvCtx->stSyncCfg.stSyncCfgBuf, 0, sizeof(ISP_SYNC_CFG_BUF_S));
        osal_memset(&pstDrvCtx->stSyncCfg.apstNode, 0, sizeof(pstDrvCtx->stSyncCfg.apstNode));

        pstSyncCfg->u8VCNum = 0;
        pstSyncCfg->u8VCCfgNum = 0;
        pstSyncCfg->u8Cfg2VldDlyMAX = 1;

        /* get N (N to 1 WDR) */
        switch (pstSyncCfg->u8WDRMode)
        {
            default:
            case WDR_MODE_NONE:
            case WDR_MODE_BUILT_IN:
            case WDR_MODE_2To1_LINE:
            case WDR_MODE_3To1_LINE:
            case WDR_MODE_4To1_LINE:
                pstSyncCfg->u8VCNumMax = 0;
                break;

            case WDR_MODE_2To1_FRAME :
            case WDR_MODE_2To1_FRAME_FULL_RATE:
                pstSyncCfg->u8VCNumMax = 1;
                break;

            case WDR_MODE_3To1_FRAME :
            case WDR_MODE_3To1_FRAME_FULL_RATE:
                pstSyncCfg->u8VCNumMax = 2;
                break;

            case WDR_MODE_4To1_FRAME :
            case WDR_MODE_4To1_FRAME_FULL_RATE:
                pstSyncCfg->u8VCNumMax = 3;
                break;
        }

        /* Channel Switch config */
        if (IS_FULL_WDR_MODE(pstSyncCfg->u8WDRMode))
        {
            au32ChnSwitch[0] = 0;
            au32ChnSwitch[1] = 1;
            au32ChnSwitch[2] = 2;
            au32ChnSwitch[3] = 3;
        }
        else if (IS_HALF_WDR_MODE(pstSyncCfg->u8WDRMode))
        {
            au32ChnSwitch[0] = 1 % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
            au32ChnSwitch[1] = (au32ChnSwitch[0] + 1) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
            au32ChnSwitch[2] = (au32ChnSwitch[0] + 2) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
            au32ChnSwitch[3] = (au32ChnSwitch[0] + 3) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
        }
        else if (IS_LINE_WDR_MODE(pstSyncCfg->u8WDRMode))
        {
            if (IS_2to1_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
            {
                au32ChnSwitch[0] = 0;
                au32ChnSwitch[1] = 1;
                au32ChnSwitch[2] = 2;
                au32ChnSwitch[3] = 3;
            }
            else if (IS_3to1_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
            {
                au32ChnSwitch[0] = 0;
                au32ChnSwitch[1] = 1;
                au32ChnSwitch[2] = 2;
                au32ChnSwitch[3] = 3;
            }
            else if (IS_4to1_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
            {
                au32ChnSwitch[0] = 0;
                au32ChnSwitch[1] = 1;
                au32ChnSwitch[2] = 2;
                au32ChnSwitch[3] = 3;
            }
            else
            {
            }

        }
        else
        {
            au32ChnSwitch[0] = 0;
            au32ChnSwitch[1] = 1;
            au32ChnSwitch[2] = 2;
            au32ChnSwitch[3] = 3;
        }

        if (HI_TRUE == pstDrvCtx->bYuvMode)
        {
            au32ChnSwitch[4] = 1;
        }
        else
        {
            au32ChnSwitch[4] = 0;
        }

        for (k = 0; k < u8BlkNum; k++)
        {
            ISP_DRV_SetInputSel(apstBeReg[k + u8BlkDev], &au32ChnSwitch[0]);

            osal_memcpy(&pstDrvCtx->astChnSelAttr[k].au32WdrChnSel, au32ChnSwitch, sizeof(au32ChnSwitch));
        }

        /* pt_int_mask */
        if ((IS_FULL_WDR_MODE(pstSyncCfg->u8WDRMode)) || (IS_HALF_WDR_MODE(pstSyncCfg->u8WDRMode)))
        {
            IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_INT_MASK) |= VI_PT_INT_FSTART;
        }
        else
        {
            IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_INT_MASK) &= (~VI_PT_INT_FSTART);
        }
    }

    pstSyncCfg->u8PreWDRMode = pstSyncCfg->u8WDRMode;

    return HI_SUCCESS;

}

HI_S32 ISP_DRV_GetSyncControlnfo(VI_PIPE ViPipe, ISP_SYNC_CFG_S *pstSyncCfg)
{
    HI_S32 ViDev;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_SYNC_CFG_BUF_NODE_S *pstCurNode = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstSyncCfg);
    /* get VC number form logic */
    // pstSyncCfg->u8VCNum++;
    // pstSyncCfg->u8VCNum = pstSyncCfg->u8VCNum % DIV_0_TO_1(pstSyncCfg->u8VCNumMax + 1);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
    ViDev = pstDrvCtx->stWDRAttr.ViDev;

    pstSyncCfg->u8VCNum = (IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT0_ID) & 0x30) >> 4;

    if (0 == pstSyncCfg->u8VCNumMax)
    {
        pstSyncCfg->u8VCNum = 0;
    }

    if (pstSyncCfg->u8VCNum > pstSyncCfg->u8VCNumMax)
    {
        ISP_TRACE(HI_DBG_ERR, "err VC number(%d), can't be large than VC total(%d)!\n", pstSyncCfg->u8VCNum, pstSyncCfg->u8VCNumMax);
    }

    /* get Cfg2VldDlyMAX */
    if (!ISPSyncBufIsEmpty(&pstSyncCfg->stSyncCfgBuf))
    {
        pstCurNode = &pstSyncCfg->stSyncCfgBuf.stSyncCfgBufNode[pstSyncCfg->stSyncCfgBuf.u8BufRDFlag];

        if (pstCurNode != HI_NULL)
        {
            if (pstCurNode->bValid)
            {
                pstSyncCfg->u8Cfg2VldDlyMAX = pstCurNode->stSnsRegsInfo.u8Cfg2ValidDelayMax;
            }
        }
    }

    if ((pstSyncCfg->u8Cfg2VldDlyMAX > CFG2VLD_DLY_LIMIT) || (pstSyncCfg->u8Cfg2VldDlyMAX < 1))
    {
        ISP_TRACE(HI_DBG_WARN, "Delay of config to valid is:0x%x\n", pstSyncCfg->u8Cfg2VldDlyMAX);
        pstSyncCfg->u8Cfg2VldDlyMAX = 1;
    }

    /* calc VCCfgNum =  (Cfg2VldDlyMAX * (VCNumMax + 1) + VCNum - Cfg2VldDlyMAX) % (VCNumMax + 1) */
    pstSyncCfg->u8VCCfgNum = (pstSyncCfg->u8VCNum + pstSyncCfg->u8VCNumMax * pstSyncCfg->u8Cfg2VldDlyMAX) % DIV_0_TO_1(pstSyncCfg->u8VCNumMax + 1);

    return HI_SUCCESS;
}

static HI_S32 ISP_DRV_CalcExpRatio(ISP_SYNC_CFG_S *pstSyncCfg, ISP_SYNC_CFG_BUF_NODE_S *pstCurNode, ISP_SYNC_CFG_BUF_NODE_S *pstPreNode, HI_U64 *au64CurSnsGain, HI_U64 *au64PreSnsGain)
{
    HI_U32 i, j;
    HI_U64 u64Tmp     = 0;
    HI_U64 u64Ratio   = 0;
    HI_U64 au64Exp[4] = {0};

    if (IS_FULL_WDR_MODE(pstSyncCfg->u8WDRMode))
    {
        switch (pstSyncCfg->u8VCCfgNum)
        {
            case 0:
                au64Exp[0] = pstCurNode->stAERegCfg.u32IntTime[0] * au64CurSnsGain[0];
                au64Exp[1] = pstPreNode->stAERegCfg.u32IntTime[1] * au64PreSnsGain[0];
                au64Exp[2] = pstPreNode->stAERegCfg.u32IntTime[2] * au64PreSnsGain[0];
                au64Exp[3] = pstPreNode->stAERegCfg.u32IntTime[3] * au64PreSnsGain[0];
                break;

            case 1:
                au64Exp[0] = pstCurNode->stAERegCfg.u32IntTime[0] * au64CurSnsGain[0];
                au64Exp[1] = pstCurNode->stAERegCfg.u32IntTime[1] * au64CurSnsGain[0];
                au64Exp[2] = pstPreNode->stAERegCfg.u32IntTime[2] * au64PreSnsGain[0];
                au64Exp[3] = pstPreNode->stAERegCfg.u32IntTime[3] * au64PreSnsGain[0];
                break;

            case 2:
                au64Exp[0] = pstCurNode->stAERegCfg.u32IntTime[0] * au64CurSnsGain[0];
                au64Exp[1] = pstCurNode->stAERegCfg.u32IntTime[1] * au64CurSnsGain[0];
                au64Exp[2] = pstCurNode->stAERegCfg.u32IntTime[2] * au64CurSnsGain[0];
                au64Exp[3] = pstPreNode->stAERegCfg.u32IntTime[3] * au64PreSnsGain[0];
                break;

            case 3:
                au64Exp[0] = pstCurNode->stAERegCfg.u32IntTime[0] * au64CurSnsGain[0];
                au64Exp[1] = pstCurNode->stAERegCfg.u32IntTime[1] * au64CurSnsGain[0];
                au64Exp[2] = pstCurNode->stAERegCfg.u32IntTime[2] * au64CurSnsGain[0];
                au64Exp[3] = pstCurNode->stAERegCfg.u32IntTime[3] * au64CurSnsGain[0];
                break;
        }
    }
    else
    {
        au64Exp[0] = pstCurNode->stAERegCfg.u32IntTime[0] * au64CurSnsGain[0];
        au64Exp[1] = pstCurNode->stAERegCfg.u32IntTime[1] * au64CurSnsGain[1];
        au64Exp[2] = pstCurNode->stAERegCfg.u32IntTime[2] * au64CurSnsGain[0];
        au64Exp[3] = pstCurNode->stAERegCfg.u32IntTime[3] * au64CurSnsGain[0];
    }

    if ((IS_LINE_WDR_MODE(pstSyncCfg->u8WDRMode)) && (ISP_FSWDR_LONG_FRAME_MODE == pstCurNode->stAERegCfg.enFSWDRMode))
    {
        for (j = 0; j < 3; j++)
        {
            for (i = CFG2VLD_DLY_LIMIT - 1; i >= 1; i--)
            {
                pstSyncCfg->u32ExpRatio[j][i] = pstSyncCfg->u32ExpRatio[j][i - 1];
                pstSyncCfg->u32WDRGain[j][i] = pstSyncCfg->u32WDRGain[j][i - 1];
            }

            pstSyncCfg->u32ExpRatio[j][0] = 0x40;
            pstSyncCfg->u32WDRGain[j][0] = 0x100;
        }

        for (i = CFG2VLD_DLY_LIMIT - 1; i >= 1; i--)
        {
            pstSyncCfg->u8LFMode[i] = pstSyncCfg->u8LFMode[i - 1];
        }

        pstSyncCfg->u8LFMode[0] = 1;
        pstSyncCfg->u32WDRGain[3][0] = 0x100;
    }
    else if ((IS_LINE_WDR_MODE(pstSyncCfg->u8WDRMode)) && (ISP_FSWDR_AUTO_LONG_FRAME_MODE == pstCurNode->stAERegCfg.enFSWDRMode))
    {
        for (j = 0; j < 3; j++)
        {
            for (i = CFG2VLD_DLY_LIMIT - 1; i >= 1; i--)
            {
                pstSyncCfg->u32ExpRatio[j][i] = pstSyncCfg->u32ExpRatio[j][i - 1];
                pstSyncCfg->u32WDRGain[j][i] = pstSyncCfg->u32WDRGain[j][i - 1];
            }

            u64Ratio = au64Exp[j + 1];
            u64Tmp   = au64Exp[j];
            u64Tmp   = DIV_0_TO_1(u64Tmp);

            while (u64Ratio > (0x1LL << 25) || u64Tmp > (0x1LL << 25))
            {
                u64Tmp >>= 1;
                u64Ratio >>= 1;
            }

            u64Ratio = (u64Ratio * pstCurNode->stAERegCfg.au32WDRGain[j + 1]) << WDR_EXP_RATIO_SHIFT;
            u64Tmp = (u64Tmp * pstCurNode->stAERegCfg.au32WDRGain[j]) ;

            while (u64Tmp > (0x1LL << 31))
            {
                u64Tmp >>= 1;
                u64Ratio >>= 1;
            }

            u64Ratio = osal_div64_u64(u64Ratio, DIV_0_TO_1(u64Tmp));
            u64Ratio = u64Ratio < 0x45 ? 0x40 : u64Ratio;
            pstSyncCfg->u32ExpRatio[j][0] = (HI_U32)u64Ratio;
            pstSyncCfg->u32WDRGain[j][0] = pstCurNode->stAERegCfg.au32WDRGain[j];
        }

        for (i = CFG2VLD_DLY_LIMIT - 1; i >= 1; i--)
        {
            pstSyncCfg->u8LFMode[i] = pstSyncCfg->u8LFMode[i - 1];
            pstSyncCfg->u32WDRGain[3][i] = pstSyncCfg->u32WDRGain[3][i - 1];
        }

        pstSyncCfg->u8LFMode[0] = 2;
        pstSyncCfg->u32WDRGain[3][0] = 0x100;

    }
    else
    {
        for (j = 0; j < 3; j++)
        {
            for (i = CFG2VLD_DLY_LIMIT - 1; i >= 1; i--)
            {
                pstSyncCfg->u32ExpRatio[j][i] = pstSyncCfg->u32ExpRatio[j][i - 1];
                pstSyncCfg->u32WDRGain[j][i] = pstSyncCfg->u32WDRGain[j][i - 1];
            }

            u64Ratio = au64Exp[j + 1];
            u64Tmp = au64Exp[j];
            u64Tmp = DIV_0_TO_1(u64Tmp);

            while (u64Ratio > (0x1LL << 25) || u64Tmp > (0x1LL << 25))
            {
                u64Tmp >>= 1;
                u64Ratio >>= 1;
            }

            u64Ratio = ((u64Ratio * pstCurNode->stAERegCfg.au32WDRGain[j + 1])) << WDR_EXP_RATIO_SHIFT;
            u64Tmp = (u64Tmp * pstCurNode->stAERegCfg.au32WDRGain[j]);

            while (u64Tmp > (0x1LL << 31))
            {
                u64Tmp >>= 1;
                u64Ratio >>= 1;
            }

            u64Ratio = osal_div64_u64(u64Ratio, DIV_0_TO_1(u64Tmp));
            pstSyncCfg->u32ExpRatio[j][0] = (HI_U32)u64Ratio;
            pstSyncCfg->u32WDRGain[j][0]  = pstCurNode->stAERegCfg.au32WDRGain[j];
        }

        for (i = CFG2VLD_DLY_LIMIT - 1; i >= 1; i--)
        {
            pstSyncCfg->u8LFMode[i]      = pstSyncCfg->u8LFMode[i - 1];
            pstSyncCfg->u32WDRGain[3][i] = pstSyncCfg->u32WDRGain[3][i - 1];
        }

        pstSyncCfg->u8LFMode[0]      = 0;
        pstSyncCfg->u32WDRGain[3][0] = 0x100;
    }

    return HI_SUCCESS;
}

static HI_S32 ISP_DRV_CalcDrcComp(ISP_SYNC_CFG_S *pstSyncCfg, ISP_SYNC_CFG_BUF_NODE_S *pstCurNode, ISP_SYNC_CFG_BUF_NODE_S *pstPreNode)
{
    HI_U32 i;
    HI_U64 u64Tmp    = 0;
    HI_U64 u64CurExp = 0;
    HI_U64 u64PreExp = 0;

    if (IS_2to1_WDR_MODE(pstSyncCfg->u8WDRMode)) {
        u64CurExp = pstCurNode->stAERegCfg.u64ExposureSF;
        u64PreExp = pstPreNode->stAERegCfg.u64ExposureSF;
    } else {
        u64CurExp = pstCurNode->stAERegCfg.u64Exposure;
        u64PreExp = pstPreNode->stAERegCfg.u64Exposure;
    }
    while (u64CurExp > (0x1LL << 31) || u64PreExp > (0x1LL << 31))
    {
        u64CurExp >>= 1;
        u64PreExp >>= 1;
    }
    u64CurExp = u64CurExp * pstCurNode->stAERegCfg.au32WDRGain[0];
    u64PreExp = u64PreExp * pstPreNode->stAERegCfg.au32WDRGain[0];

    for (i = CFG2VLD_DLY_LIMIT - 1; i >= 1; i--)
    {
        pstSyncCfg->u32DRCComp[i] = pstSyncCfg->u32DRCComp[i - 1];
    }

    u64CurExp = u64CurExp << DRC_COMP_SHIFT;
    u64Tmp = DIV_0_TO_1(u64PreExp);

    while (u64Tmp > (0x1LL << 31))
    {
        u64Tmp >>= 1;
        u64CurExp >>= 1;
    }

    u64CurExp = osal_div64_u64(u64CurExp, u64Tmp);
    pstSyncCfg->u32DRCComp[0] = (HI_U32)u64CurExp;

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_CalcSyncCfg(ISP_SYNC_CFG_S *pstSyncCfg)
{
    HI_U32 i;// j;
    ISP_SYNC_CFG_BUF_NODE_S *pstCurNode = HI_NULL;
    ISP_SYNC_CFG_BUF_NODE_S *pstPreNode = HI_NULL;
    HI_U64 au64CurSnsGain[2] = {0}, au64PreSnsGain[2] = {0};
    HI_BOOL bErr = HI_FALSE;

    ISP_CHECK_POINTER(pstSyncCfg);

    /* update node when VCCfgNum is 0 */
    if (pstSyncCfg->u8VCCfgNum == 0)
    {
        for (i = CFG2VLD_DLY_LIMIT; i >= 1; i--)
        {
            pstSyncCfg->apstNode[i] = pstSyncCfg->apstNode[i - 1];
        }

        //ISPSyncBufRead(&pstSyncCfg->stSyncCfgBuf, &pstSyncCfg->apstNode[0]);

        /* avoid skip effective AE results */
        if (ISPSyncBufIsErr(&pstSyncCfg->stSyncCfgBuf))
        {
            bErr = HI_TRUE;
        }

        /* read the newest information */
        ISPSyncBufRead2(&pstSyncCfg->stSyncCfgBuf, &pstSyncCfg->apstNode[0]);
    }

    pstCurNode = pstSyncCfg->apstNode[0];

    if (HI_NULL == pstCurNode)
    {
        return HI_SUCCESS;
    }

    if (HI_FALSE == pstCurNode->bValid)
    {
        return HI_SUCCESS;
    }

    if (HI_TRUE == bErr)
    {
        if (ISP_SNS_I2C_TYPE == pstCurNode->stSnsRegsInfo.enSnsType)
        {
            for (i = 0; i < pstCurNode->stSnsRegsInfo.u32RegNum; i++)
            {
                pstCurNode->stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            }
        }
        else if (ISP_SNS_SSP_TYPE == pstCurNode->stSnsRegsInfo.enSnsType)
        {
            for (i = 0; i < pstCurNode->stSnsRegsInfo.u32RegNum; i++)
            {
                pstCurNode->stSnsRegsInfo.astSspData[i].bUpdate = HI_TRUE;
            }
        }
        else
        {
            /* do nothing */
        }

        pstCurNode->stSnsRegsInfo.stSlvSync.bUpdate = HI_TRUE;
    }

    if (HI_NULL == pstSyncCfg->apstNode[1])
    {
        pstPreNode = pstSyncCfg->apstNode[0];
    }
    else
    {
        if (HI_FALSE == pstSyncCfg->apstNode[1]->bValid)
        {
            pstPreNode = pstSyncCfg->apstNode[0];
        }
        else
        {
            pstPreNode = pstSyncCfg->apstNode[1];
        }
    }

    if(IS_2to1_WDR_MODE(pstSyncCfg->u8WDRMode))
    {
        if (pstSyncCfg->u8VCCfgNum == 0)
        {
            au64PreSnsGain[0] = pstPreNode->stAERegCfg.u64ExposureSF;
            au64PreSnsGain[0] = osal_div_u64(au64PreSnsGain[0], DIV_0_TO_1(pstPreNode->stAERegCfg.u32IntTime[0]));
            au64PreSnsGain[0] = au64PreSnsGain[0] << 8;
            au64PreSnsGain[0] = osal_div_u64(au64PreSnsGain[0], DIV_0_TO_1((HI_U64)pstPreNode->stAERegCfg.u32IspDgain * pstPreNode->stAERegCfg.au32WDRGain[0] >> 8));

            au64CurSnsGain[0] = pstCurNode->stAERegCfg.u64ExposureSF;
            au64CurSnsGain[0] = osal_div_u64(au64CurSnsGain[0], DIV_0_TO_1(pstCurNode->stAERegCfg.u32IntTime[0]));
            au64CurSnsGain[0] = au64CurSnsGain[0] << 8;
            au64CurSnsGain[0] = osal_div_u64(au64CurSnsGain[0], DIV_0_TO_1((HI_U64)pstCurNode->stAERegCfg.u32IspDgain * pstCurNode->stAERegCfg.au32WDRGain[0] >> 8));

            pstSyncCfg->u64PreSnsGainSF = au64PreSnsGain[0];
            pstSyncCfg->u64CurSnsGainSF = au64CurSnsGain[0];

			au64PreSnsGain[1] = pstPreNode->stAERegCfg.u64Exposure;
            au64PreSnsGain[1] = osal_div_u64(au64PreSnsGain[1], DIV_0_TO_1(pstPreNode->stAERegCfg.u32IntTime[1]));
            au64PreSnsGain[1] = au64PreSnsGain[1] << 8;
            au64PreSnsGain[1] = osal_div_u64(au64PreSnsGain[1], DIV_0_TO_1((HI_U64)pstPreNode->stAERegCfg.u32IspDgain * pstPreNode->stAERegCfg.au32WDRGain[1] >> 8));

            au64CurSnsGain[1] = pstCurNode->stAERegCfg.u64Exposure;
            au64CurSnsGain[1] = osal_div_u64(au64CurSnsGain[1], DIV_0_TO_1(pstCurNode->stAERegCfg.u32IntTime[1]));
            au64CurSnsGain[1] = au64CurSnsGain[1] << 8;
            au64CurSnsGain[1] = osal_div_u64(au64CurSnsGain[1], DIV_0_TO_1((HI_U64)pstCurNode->stAERegCfg.u32IspDgain * pstCurNode->stAERegCfg.au32WDRGain[1] >> 8));

            pstSyncCfg->u64PreSnsGain = au64PreSnsGain[1];
            pstSyncCfg->u64CurSnsGain = au64CurSnsGain[1];
        }

        au64PreSnsGain[0] = pstSyncCfg->u64PreSnsGainSF;
        au64CurSnsGain[0] = pstSyncCfg->u64CurSnsGainSF;
		au64PreSnsGain[1] = pstSyncCfg->u64PreSnsGain;
        au64CurSnsGain[1] = pstSyncCfg->u64CurSnsGain;
	}
	else
	{
        if (pstSyncCfg->u8VCCfgNum == 0)
        {
            au64PreSnsGain[0] = pstPreNode->stAERegCfg.u64Exposure;
            au64PreSnsGain[0] = osal_div_u64(au64PreSnsGain[0], DIV_0_TO_1(pstPreNode->stAERegCfg.u32IntTime[0]));
            au64PreSnsGain[0] = au64PreSnsGain[0] << 8;
            au64PreSnsGain[0] = osal_div_u64(au64PreSnsGain[0], DIV_0_TO_1(pstPreNode->stAERegCfg.u32IspDgain));

            au64CurSnsGain[0] = pstCurNode->stAERegCfg.u64Exposure;
            au64CurSnsGain[0] = osal_div_u64(au64CurSnsGain[0], DIV_0_TO_1(pstCurNode->stAERegCfg.u32IntTime[0]));
            au64CurSnsGain[0] = au64CurSnsGain[0] << 8;
            au64CurSnsGain[0] = osal_div_u64(au64CurSnsGain[0], DIV_0_TO_1(pstCurNode->stAERegCfg.u32IspDgain));

            pstSyncCfg->u64PreSnsGain = au64PreSnsGain[0];
            pstSyncCfg->u64CurSnsGain = au64CurSnsGain[0];
        }

        au64PreSnsGain[0] = pstSyncCfg->u64PreSnsGain;
        au64CurSnsGain[0] = pstSyncCfg->u64CurSnsGain;
	}

    /* calculate exposure ratio */

    ISP_DRV_CalcExpRatio(pstSyncCfg, pstCurNode, pstPreNode, au64CurSnsGain, au64PreSnsGain);

    /* calculate AlgProc */
    if (IS_LINE_WDR_MODE(pstSyncCfg->u8WDRMode))
    {
        for (i = CFG2VLD_DLY_LIMIT - 1; i >= 1; i--)
        {
            pstSyncCfg->u8AlgProc[i] = pstSyncCfg->u8AlgProc[i - 1];
        }
        pstSyncCfg->u8AlgProc[0] = pstCurNode->stWDRRegCfg.bWDRMdtEn;
    }

    /* calculate DRC compensation */
    if (pstSyncCfg->u8VCCfgNum == 0)
    {
        ISP_DRV_CalcDrcComp(pstSyncCfg, pstCurNode, pstPreNode);
    }

    return HI_SUCCESS;
}

HI_U8 ISP_DRV_GetBeSyncIndex(ISP_DRV_CTX_S *pstDrvCtx)
{
    HI_U8 index;

    if (IS_ONLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode) || IS_SIDEBYSIDE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode))
    {
        index = (pstDrvCtx->stSyncCfg.u8Cfg2VldDlyMAX - 1) / DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
    }
    else
    {
        if (FRAME_INTERRUPT_START == pstDrvCtx->stFrameIntAttr.enIntType)
        {
            if (IS_HALF_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
            {
                index = pstDrvCtx->stSyncCfg.u8Cfg2VldDlyMAX;
            }
            else
            {
                index = pstDrvCtx->stSyncCfg.u8Cfg2VldDlyMAX + 1;
            }
        }
        else
        {
            if (IS_HALF_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
            {
                index = pstDrvCtx->stSyncCfg.u8Cfg2VldDlyMAX - 1;
            }
            else
            {
                index = pstDrvCtx->stSyncCfg.u8Cfg2VldDlyMAX;
            }
        }
    }
    return index;
}

HI_S32 ISP_DRV_RegConfigIsp(VI_PIPE ViPipe, ISP_DRV_CTX_S *pstDrvCtx)
{

    HI_S32  i;
    HI_U8   u8CfgNodeIdx, u8CfgNodeVC;
    HI_U32  u32IspDgain;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_U32  u32Ratio[3] = {0x40};
    HI_U8   u8BlkDev = 0;
    HI_U8   u8BlkNum = 1;
    HI_U8   u8SnapBlkDev = 0;
    HI_U8   u8SnapBlkNum = 1;
    HI_U32  au32WDRGain[4]   = {0x100, 0x100, 0x100, 0x100};
    HI_U32  au32WDRWBGain[4] = {0x100, 0x100, 0x100, 0x100};
    HI_U32  au32WBViGain[4]  = {0x100, 0x100, 0x100, 0x100};
    HI_U16  au16CCM[9] =
    {
        0x01A7,  0x809D,  0x800A,
        0x804F,  0x018E,  0x803F,
        0x000B,  0x8098,  0x018D
    };
    ISP_AE_REG_CFG_2_S *pstAERegCfg = HI_NULL;
    ISP_AWB_REG_CFG_2_S *pstAWBRegCfg = HI_NULL;
    ISP_SYNC_CFG_BUF_NODE_S *pstCfgNode = HI_NULL;
    ISP_BE_REGS_ATTR_S stIspBeRegsAttr = {0};
    ISP_BE_REGS_ATTR_S stIspSnapBeRegsAttr = {0};
    S_ISPBE_REGS_TYPE *apstBeReg[ISP_STRIPING_MAX_NUM] = {HI_NULL};
    S_ISPBE_REGS_TYPE *apstSnapBeReg[ISP_STRIPING_MAX_NUM] = {HI_NULL};
    VI_PIPE MainPipeS;
    ISP_DRV_CTX_S *pstDrvCtxS = HI_NULL;
    VI_PIPE PicturePipe;
    ISP_DRV_CTX_S *pstDrvCtxPic = HI_NULL;
    ISP_RUNNING_MODE_E enPictureRunningMode = ISP_MODE_RUNNING_OFFLINE;
    HI_BOOL bOnlineHavePictruePipe = HI_FALSE;
    HI_U8   u8SyncIndex = 0;
    HI_U8   u8ExpRatioIndex = 0;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstDrvCtx);

    s32Ret = ISP_DRV_GetBeRegsAttr(ViPipe, apstBeReg, &stIspBeRegsAttr);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_SUCCESS;
    }

    if (0 <= pstDrvCtx->stSnapAttr.s32PicturePipeId)
    {
        PicturePipe = pstDrvCtx->stSnapAttr.s32PicturePipeId;
        pstDrvCtxPic = ISP_DRV_GET_CTX(PicturePipe);
        enPictureRunningMode = pstDrvCtxPic->stWorkMode.enIspRunningMode;

        if ((IS_ONLINE_MODE(enPictureRunningMode)) || \
            (IS_SIDEBYSIDE_MODE(enPictureRunningMode)))
        {
            if (PicturePipe != pstDrvCtx->stSnapAttr.s32PreviewPipeId)
            {
                bOnlineHavePictruePipe = HI_TRUE;
            }
        }
    }
    u8BlkDev = stIspBeRegsAttr.u8BlockDev;
    u8BlkNum = stIspBeRegsAttr.u8BlockNum;

    /* config Chn Switch / Exp Ratio / ISP Dgain */
    /* delay of config2valid of isp reg is 1 */
    u8SyncIndex     = ISP_DRV_GetBeSyncIndex(pstDrvCtx);
    u8SyncIndex     = MIN2(u8SyncIndex, CFG2VLD_DLY_LIMIT - 1);
    u8ExpRatioIndex = u8SyncIndex;

    if (IS_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
    {
        /* Channal Switch */
        ISP_DRV_RegConfigChnSel(apstBeReg, pstDrvCtx, u8BlkNum, u8BlkDev);

        /* Ratio */
        if (IS_HALF_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
        {
            if (IS_OFFLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode) || \
                IS_STRIPING_MODE(pstDrvCtx->stWorkMode.enIspRunningMode))
            {
                u8ExpRatioIndex = u8SyncIndex + 2;
                u8ExpRatioIndex = MIN2(u8ExpRatioIndex, CFG2VLD_DLY_LIMIT - 1);
            }
        }

        for (i = 0; i < 3; i++)
        {
            u32Ratio[i] = pstDrvCtx->stSyncCfg.u32ExpRatio[i][u8ExpRatioIndex];
        }

        /* Cal CfgNodeIdx to ispDgain/ratio.. configs */
        if (IS_ONLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode) || \
            IS_SIDEBYSIDE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode))
        {
             u8CfgNodeVC  = ((pstDrvCtx->stSyncCfg.u8Cfg2VldDlyMAX - 1) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1));
        }
        else
        {
             u8CfgNodeVC  = ((pstDrvCtx->stSyncCfg.u8Cfg2VldDlyMAX + 1) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1));
        }

        u8CfgNodeIdx = u8SyncIndex;
    }
    else
    {
        u8CfgNodeIdx = u8SyncIndex;
        u8CfgNodeVC  = 0;
    }

    /* when the data of sensor built-in WDR after decompand is 16bit, the ratio value is as follow. */
    if (IS_BUILT_IN_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
    {
        u32Ratio[0] = BUILT_IN_WDR_RATIO_VS_S;
        u32Ratio[1] = BUILT_IN_WDR_RATIO_S_M;
        u32Ratio[2] = BUILT_IN_WDR_RATIO_M_L;
    }

    for (i = 0; i < 3; i++)
    {
        u32Ratio[i] = CLIP3(u32Ratio[i], EXP_RATIO_MIN, EXP_RATIO_MAX);
    }

    pstCfgNode = pstDrvCtx->stSyncCfg.apstNode[u8CfgNodeIdx];

    if (HI_NULL != pstCfgNode)
    {
        if (pstDrvCtx->stSyncCfg.u8VCCfgNum == u8CfgNodeVC)
        {
            ISP_DRV_RegConfigWdr(ViPipe, apstBeReg, pstDrvCtx, &pstCfgNode->stWDRRegCfg, u8BlkNum, u8BlkDev, u32Ratio);

            /* config Ldci compensation */
            ISP_DRV_RegConfigLdci(apstBeReg, pstDrvCtx, u8BlkNum, u8BlkDev);
            ISP_DRV_RegConfigDe(ViPipe, apstBeReg, pstCfgNode);

            /*config drc strength*/
            ISP_DRV_RegConfigDrc(ViPipe, apstBeReg, pstDrvCtx, &pstCfgNode->stDRCRegCfg, u8BlkNum, u8BlkDev, u32Ratio);
        }
    }

    if (HI_TRUE == bOnlineHavePictruePipe)
    {
        if (ViPipe == pstDrvCtx->stSnapAttr.s32PicturePipeId)
        {
            return HI_SUCCESS;
        }
    }
    /* config isp_dgain & drc strength */
    pstCfgNode = pstDrvCtx->stSyncCfg.apstNode[u8CfgNodeIdx];

    if (SNAP_STATE_CFG == pstDrvCtx->stSnapInfoLoad.enSnapState)
    {
        u32IspDgain = pstDrvCtx->stSnapInfoLoad.stIspCfgInfo.u32IspDgain;

        if (HI_TRUE == pstDrvCtx->stSnapAttr.bLoadCCM)
        {
            for (i = 0; i < CCM_MATRIX_SIZE; i++)
            {
                au16CCM[i] = pstDrvCtx->stSnapInfoLoad.stIspCfgInfo.au16CapCCM[i];
            }

        }
        else
        {
            if (HI_NULL == pstCfgNode)
            {
                return HI_SUCCESS;
            }

            for (i = 0; i < CCM_MATRIX_SIZE; i++)
            {
                au16CCM[i] = pstCfgNode->stAWBRegCfg.au16ColorMatrix[i];
            }

        }

        //if ((1 == g_UpdatePos[ViPipe]) && (3 != pstDrvCtx->stIntSch.u32IspIntStatus))
        //{
        //    pstDrvCtx->stSnapInfoLoad.enSnapState = SNAP_STATE_NULL;
        //}
    }
    else
    {
        if (HI_NULL == pstCfgNode)
        {
            return HI_SUCCESS;
        }

        if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable)
        {
            if (HI_FALSE == pstDrvCtx->stStitchAttr.bMainPipe)
            {
                MainPipeS  = pstDrvCtx->stStitchAttr.as8StitchBindId[0];
                pstDrvCtxS = ISP_DRV_GET_CTX(MainPipeS);
                pstCfgNode = pstDrvCtxS->stSyncCfg.apstNode[u8CfgNodeIdx];

                if (HI_NULL == pstCfgNode)
                {
                    ISP_TRACE(HI_DBG_WARN, "pstCfgNode NULL point!\n");
                    return HI_SUCCESS;
                }
            }
        }

        if (pstDrvCtx->stSyncCfg.u8VCCfgNum == u8CfgNodeVC)
        {
            pstAERegCfg  = &pstCfgNode->stAERegCfg;
            pstAWBRegCfg = &pstCfgNode->stAWBRegCfg;
        }

        if (HI_NULL == pstAERegCfg)
        {
            return HI_SUCCESS;
        }

        if (HI_NULL == pstAWBRegCfg)
        {
            return HI_SUCCESS;
        }

        u32IspDgain = pstAERegCfg->u32IspDgain;

        for (i = 0; i < CCM_MATRIX_SIZE; i++)
        {
            if (HI_NULL != pstAWBRegCfg)
            {
                au16CCM[i] = pstAWBRegCfg->au16ColorMatrix[i];
            }
        }
    }

    u32IspDgain = CLIP3(u32IspDgain, ISP_DIGITAL_GAIN_MIN, ISP_DIGITAL_GAIN_MAX);
    if (HI_TRUE == bOnlineHavePictruePipe)
    {
        if (ViPipe == pstDrvCtx->stSnapAttr.s32PreviewPipeId)
        {
            ISP_CHECK_PIPE(pstDrvCtx->stSnapAttr.s32PicturePipeId);
            s32Ret = ISP_DRV_GetBeRegsAttr(pstDrvCtx->stSnapAttr.s32PicturePipeId, apstSnapBeReg, &stIspSnapBeRegsAttr);
            if (HI_SUCCESS == s32Ret)
            {
                u8SnapBlkDev = stIspSnapBeRegsAttr.u8BlockDev;
                u8SnapBlkNum = stIspSnapBeRegsAttr.u8BlockNum;
                ISP_DRV_RegConfigDgain(apstSnapBeReg, u32IspDgain, u8SnapBlkNum, u8SnapBlkDev);
            }
        }
    }

    for (i = 0; i < 4; i++)
    {
        au32WDRGain[i] = pstDrvCtx->stSyncCfg.u32WDRGain[i][u8SyncIndex];
        au32WDRGain[i] = CLIP3(au32WDRGain[i], ISP_DIGITAL_GAIN_MIN, ISP_DIGITAL_GAIN_MAX);

        if (HI_NULL != pstAWBRegCfg)
        {
            au32WDRWBGain[i] = pstAWBRegCfg->au32WDRWBGain[i];
            au32WBViGain[i]  = pstAWBRegCfg->au32BEWhiteBalanceGain[i];
        }
    }

    ISP_DRV_RegConfigDgain(apstBeReg, u32IspDgain, u8BlkNum, u8BlkDev);

    ISP_DRV_ConfigCCM(pstDrvCtx, au16CCM);

    if (SNAP_STATE_CFG == pstDrvCtx->stSnapInfoLoad.enSnapState)
    {
        for (i = 0; i < 4; i++)
        {
            au32WBViGain[i]  = pstDrvCtx->stSnapInfoLoad.stIspCfgInfo.au32WhiteBalanceGain[i];
        }
        ISP_DRV_RegConfig4Dgain(ViPipe, apstBeReg, pstDrvCtx, au32WDRGain, u8BlkNum, u8BlkDev, au32WDRWBGain, au32WBViGain);
        if ((1 == g_UpdatePos[ViPipe]) && (3 != pstDrvCtx->stIntSch.u32IspIntStatus))
        {
            pstDrvCtx->stSnapInfoLoad.enSnapState = SNAP_STATE_NULL;
        }
    }
    else
    {
        if (ViPipe != pstDrvCtx->stSnapAttr.s32PicturePipeId)
        {
            ISP_DRV_RegConfig4Dgain(ViPipe, apstBeReg, pstDrvCtx, au32WDRGain, u8BlkNum, u8BlkDev, au32WDRWBGain, au32WBViGain);
        }
    }

    for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
    {
        pstDrvCtx->stIspBeSyncPara.au32IspDgain[i] = u32IspDgain;
    }

    pstCfgNode = pstDrvCtx->stSyncCfg.apstNode[0];

    if (HI_NULL == pstCfgNode)
    {
        return HI_SUCCESS;
    }

    /* config Piris */
    if (HI_NULL != pstAERegCfg)
    {
        if (pstAERegCfg->bPirisValid == HI_TRUE)
        {
            if (HI_NULL != pstDrvCtx->stPirisCb.pfn_piris_gpio_update)
            {
                pstDrvCtx->stPirisCb.pfn_piris_gpio_update(ViPipe, &pstAERegCfg->s32PirisPos);
            }
        }
    }
    return HI_SUCCESS;
}

ISP_SYNC_CFG_BUF_NODE_S *ISP_DRV_GetSnsRegConfigNode(ISP_DRV_CTX_S *pstDrvCtx, HI_U8 u8DelayFrmNum)
{
    HI_U8 u8WDRMode = WDR_MODE_NONE;
    HI_U8 u8CfgNodeIdx = 0, u8CfgNodeVC = 0;
    ISP_SYNC_CFG_BUF_NODE_S *pstCfgNode = HI_NULL;
    ISP_SYNC_CFG_BUF_NODE_S *pstPreCfgNode = HI_NULL;

    u8WDRMode = pstDrvCtx->stWDRCfg.u8WDRMode;
    u8CfgNodeIdx = u8DelayFrmNum / DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
    u8CfgNodeVC =  u8DelayFrmNum % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);


    if (pstDrvCtx->stSyncCfg.u8VCCfgNum == u8CfgNodeVC)
    {
        if (u8CfgNodeIdx > CFG2VLD_DLY_LIMIT - 1)
        {
            //ISP_TRACE(HI_DBG_WARN, "DelayFrmNum error!\n");
            return HI_NULL;
        }

        pstCfgNode    = pstDrvCtx->stSyncCfg.apstNode[u8CfgNodeIdx];
        pstPreCfgNode = pstDrvCtx->stSyncCfg.apstNode[u8CfgNodeIdx + 1];

        if (HI_NULL == pstCfgNode)
        {
            return HI_NULL;
        }

        if (HI_NULL == pstPreCfgNode)
        {
        }
        else
        {
            if ((IS_LINEAR_MODE(u8WDRMode)) || (IS_BUILT_IN_WDR_MODE(u8WDRMode)))
            {
                /* not config sensor when cur == pre */
                if (pstCfgNode == pstPreCfgNode)
                {
                    return HI_NULL;
                }
            }
        }
    }

    return pstCfgNode;
}

HI_S32 ISP_DRV_StitchRegsCfgSensor(VI_PIPE ViPipe, ISP_DRV_CTX_S *pstDrvCtx, HI_U32 u32UpdatePos)
{
    HI_S32 i;
    HI_U32 u32SlaveDev = 0;
    VI_PIPE MainPipeS = 0;
    ISP_SYNC_CFG_BUF_NODE_S *pstCurNode = HI_NULL;
    ISP_SYNC_CFG_BUF_NODE_S *pstCfgNode = HI_NULL;

    ISP_I2C_DATA_S *pstI2cData = HI_NULL;
    ISP_SSP_DATA_S *pstSspData = HI_NULL;
    HI_S8 s8I2cDev, s8SspDev, s8SspCs;

    pstCurNode = pstDrvCtx->stSyncCfg.apstNode[0];

    if (HI_NULL == pstCurNode)
    {
        ISP_TRACE(HI_DBG_WARN, "NULL point Stitch!\n");
        return HI_FAILURE;
    }

    if (HI_FALSE == pstCurNode->bValid)
    {
        ISP_TRACE(HI_DBG_WARN, "Invalid Node Stitch!\n");
        return HI_FAILURE;
    }

    if (0 == pstCurNode->stSnsRegsInfo.u32RegNum)
    {
        ISP_TRACE(HI_DBG_WARN, "Err u32RegNum Stitch!\n");
        return HI_FAILURE;
    }

    u32SlaveDev = pstCurNode->stSnsRegsInfo.stSlvSync.u32SlaveBindDev;

    if (ISP_SNS_I2C_TYPE == pstCurNode->stSnsRegsInfo.enSnsType)
    {
        s8I2cDev = pstCurNode->stSnsRegsInfo.unComBus.s8I2cDev;

        if (-1 == s8I2cDev)
        {
            return 0;
        }

        if (HI_FALSE == pstDrvCtx->stStitchAttr.bMainPipe)
        {
            MainPipeS  = pstDrvCtx->stStitchAttr.as8StitchBindId[0];
            pstDrvCtx  = ISP_DRV_GET_CTX(MainPipeS);
            pstCurNode = pstDrvCtx->stSyncCfg.apstNode[0];

            if (HI_NULL == pstCurNode)
            {
                ISP_TRACE(HI_DBG_WARN, "pstCurNodeS NULL point Stitch!\n");
                return HI_FAILURE;
            }
        }

        for (i = 0; i < pstCurNode->stSnsRegsInfo.u32RegNum; i++)
        {
            pstCfgNode = ISP_DRV_GetSnsRegConfigNode(pstDrvCtx, pstCurNode->stSnsRegsInfo.astI2cData[i].u8DelayFrmNum);

            if (!pstCfgNode)
            {
                continue;
            }

            if (HI_TRUE == pstCfgNode->bValid)
            {
                pstI2cData = &pstCfgNode->stSnsRegsInfo.astI2cData[i];

                if ((pstI2cData->bUpdate == HI_TRUE) && (u32UpdatePos == pstI2cData->u8IntPos))
                {
                    if (HI_NULL != pstDrvCtx->stBusCb.pfnISPWriteI2CData)
                    {
                        pstDrvCtx->stBusCb.pfnISPWriteI2CData(s8I2cDev, pstI2cData->u8DevAddr,
                                                              pstI2cData->u32RegAddr, pstI2cData->u32AddrByteNum,
                                                              pstI2cData->u32Data, pstI2cData->u32DataByteNum);
                    }
                }
            }

        }
    }
    else if (ISP_SNS_SSP_TYPE == pstCurNode->stSnsRegsInfo.enSnsType)
    {
        s8SspDev = pstCurNode->stSnsRegsInfo.unComBus.s8SspDev.bit4SspDev;

        if (-1 == s8SspDev)
        {
            return 0;
        }

        s8SspDev = pstCurNode->stSnsRegsInfo.unComBus.s8SspDev.bit4SspDev;
        s8SspCs = pstCurNode->stSnsRegsInfo.unComBus.s8SspDev.bit4SspCs;

        if (HI_FALSE == pstDrvCtx->stStitchAttr.bMainPipe)
        {
            MainPipeS  = pstDrvCtx->stStitchAttr.as8StitchBindId[0];
            pstDrvCtx  = ISP_DRV_GET_CTX(MainPipeS);
            pstCurNode = pstDrvCtx->stSyncCfg.apstNode[0];

            if (HI_NULL == pstCurNode)
            {
                ISP_TRACE(HI_DBG_WARN, "pstCurNodeS NULL point Stitch!\n");
                return HI_FAILURE;
            }
        }

        for (i = 0; i < pstCurNode->stSnsRegsInfo.u32RegNum; i++)
        {
            pstCfgNode = ISP_DRV_GetSnsRegConfigNode(pstDrvCtx, pstCurNode->stSnsRegsInfo.astSspData[i].u8DelayFrmNum);

            if (!pstCfgNode)
            {
                continue;
            }

            if (HI_TRUE == pstCfgNode->bValid)
            {
                pstSspData = &pstCfgNode->stSnsRegsInfo.astSspData[i];

                if ((pstSspData->bUpdate == HI_TRUE) && (u32UpdatePos == pstSspData->u8IntPos))
                {
                    if (HI_NULL != pstDrvCtx->stBusCb.pfnISPWriteSSPData)
                    {
                        pstDrvCtx->stBusCb.pfnISPWriteSSPData(s8SspDev, s8SspCs, pstSspData->u32DevAddr,
                                                              pstSspData->u32DevAddrByteNum, pstSspData->u32RegAddr,
                                                              pstSspData->u32RegAddrByteNum, pstSspData->u32Data,
                                                              pstSspData->u32DataByteNum);
                    }
                }
            }
        }
    }

    /* write slave sns vmax sync*/
    pstCfgNode = ISP_DRV_GetSnsRegConfigNode(pstDrvCtx, pstCurNode->stSnsRegsInfo.stSlvSync.u8DelayFrmNum);

    if (HI_NULL == pstCfgNode)
    {
        return HI_FAILURE;
    }

    if ((HI_TRUE == pstCfgNode->bValid) && (HI_TRUE == pstCfgNode->stSnsRegsInfo.stSlvSync.bUpdate))
    {
        /* adjust the relationship between slavedev and vipipe */
        IO_RW_PT_ADDRESS(VICAP_SLAVE_VSTIME(u32SlaveDev)) = pstCfgNode->stSnsRegsInfo.stSlvSync.u32SlaveVsTime;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_NormalRegsCfgSensor(VI_PIPE ViPipe, ISP_DRV_CTX_S *pstDrvCtx)
{
    HI_S32 i = 0;
    HI_U32 u32SlaveDev = 0;
    ISP_SYNC_CFG_BUF_NODE_S *pstCurNode = HI_NULL;
    ISP_SYNC_CFG_BUF_NODE_S *pstCfgNode = HI_NULL;
    ISP_I2C_DATA_S *pstI2cData = HI_NULL;
    ISP_SSP_DATA_S *pstSspData = HI_NULL;
    HI_S8 s8I2cDev, s8SspDev, s8SspCs;

    pstCurNode = pstDrvCtx->stSyncCfg.apstNode[0];

    if (HI_NULL == pstCurNode)
    {
        ISP_TRACE(HI_DBG_WARN, "NULL point Normal!\n");
        return HI_FAILURE;
    }

    if (HI_FALSE == pstCurNode->bValid)
    {
        ISP_TRACE(HI_DBG_WARN, "Invalid Node Normal!\n");
        return HI_FAILURE;
    }

    if (0 == pstCurNode->stSnsRegsInfo.u32RegNum)
    {
        ISP_TRACE(HI_DBG_WARN, "Err u32RegNum Normal!\n");
        return HI_FAILURE;
    }


    if (ISP_SNS_I2C_TYPE == pstCurNode->stSnsRegsInfo.enSnsType)
    {
        for (i = 0; i < pstCurNode->stSnsRegsInfo.u32RegNum; i++)
        {
            pstCfgNode = ISP_DRV_GetSnsRegConfigNode(pstDrvCtx, pstCurNode->stSnsRegsInfo.astI2cData[i].u8DelayFrmNum);

            if (!pstCfgNode)
            {
                continue;
            }

            s8I2cDev = pstCfgNode->stSnsRegsInfo.unComBus.s8I2cDev;

            if (-1 == s8I2cDev)
            {
                return 0;
            }

            if (HI_TRUE == pstCfgNode->bValid)
            {
                pstI2cData = &pstCfgNode->stSnsRegsInfo.astI2cData[i];

                if ((pstI2cData->bUpdate == HI_TRUE) && (pstDrvCtx->u32IntPos == pstI2cData->u8IntPos))
                {
                    if (HI_NULL != pstDrvCtx->stBusCb.pfnISPWriteI2CData)
                    {
                        pstDrvCtx->stBusCb.pfnISPWriteI2CData(s8I2cDev, pstI2cData->u8DevAddr,
                                                              pstI2cData->u32RegAddr, pstI2cData->u32AddrByteNum,
                                                              pstI2cData->u32Data, pstI2cData->u32DataByteNum);
                    }
                }
            }
        }
    }
    else if (ISP_SNS_SSP_TYPE == pstCurNode->stSnsRegsInfo.enSnsType)
    {
        for (i = 0; i < pstCurNode->stSnsRegsInfo.u32RegNum; i++)
        {
            pstCfgNode = ISP_DRV_GetSnsRegConfigNode(pstDrvCtx, pstCurNode->stSnsRegsInfo.astSspData[i].u8DelayFrmNum);

            if (!pstCfgNode)
            {
                continue;
            }

            s8SspDev = pstCfgNode->stSnsRegsInfo.unComBus.s8SspDev.bit4SspDev;

            if (-1 == s8SspDev)
            {
                return 0;
            }

            s8SspDev = pstCfgNode->stSnsRegsInfo.unComBus.s8SspDev.bit4SspDev;
            s8SspCs = pstCfgNode->stSnsRegsInfo.unComBus.s8SspDev.bit4SspCs;

            if (HI_TRUE == pstCfgNode->bValid)
            {
                pstSspData = &pstCfgNode->stSnsRegsInfo.astSspData[i];

                if ((pstSspData->bUpdate == HI_TRUE) && (pstDrvCtx->u32IntPos == pstSspData->u8IntPos))
                {
                    if (HI_NULL != pstDrvCtx->stBusCb.pfnISPWriteSSPData)
                    {
                        pstDrvCtx->stBusCb.pfnISPWriteSSPData(s8SspDev, s8SspCs, pstSspData->u32DevAddr,
                                                              pstSspData->u32DevAddrByteNum, pstSspData->u32RegAddr,
                                                              pstSspData->u32RegAddrByteNum, pstSspData->u32Data,
                                                              pstSspData->u32DataByteNum);
                    }
                }
            }
        }
    }

    /* write slave sns vmax sync*/
    pstCfgNode = ISP_DRV_GetSnsRegConfigNode(pstDrvCtx, pstCurNode->stSnsRegsInfo.stSlvSync.u8DelayFrmNum);

    if (HI_NULL == pstCfgNode)
    {
        return HI_FAILURE;
    }

    if ((HI_TRUE == pstCfgNode->bValid) && (HI_TRUE == pstCfgNode->stSnsRegsInfo.stSlvSync.bUpdate))
    {
        /* adjust the relationship between slavedev and vipipe */
        u32SlaveDev = pstCfgNode->stSnsRegsInfo.stSlvSync.u32SlaveBindDev;
        IO_RW_PT_ADDRESS(VICAP_SLAVE_VSTIME(u32SlaveDev)) = pstCfgNode->stSnsRegsInfo.stSlvSync.u32SlaveVsTime;
    }

    return HI_SUCCESS;
}


HI_S32 ISP_DRV_RegConfigSensor(VI_PIPE ViPipe, ISP_DRV_CTX_S *pstDrvCtx)
{
    HI_S32 s32Ret = HI_SUCCESS;
    volatile  HI_U32 u32UpdatePos;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstDrvCtx);

    u32UpdatePos = pstDrvCtx->u32IntPos;

    if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable)
    {
        s32Ret = ISP_DRV_StitchRegsCfgSensor(ViPipe, pstDrvCtx, u32UpdatePos);
        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_WARN, "ISP_DRV_StitchRegsCfgSensor failure!\n");
            return s32Ret;
        }
    }
    else
    {
        s32Ret = ISP_DRV_NormalRegsCfgSensor(ViPipe, pstDrvCtx);
        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_WARN, "ISP_DRV_NormalRegsCfgSensor failure!\n");
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_RegisterBusCallBack (VI_PIPE ViPipe,
                                ISP_BUS_TYPE_E enType, ISP_BUS_CALLBACK_S *pstBusCb)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_POINTER(pstBusCb);
    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (ISP_BUS_TYPE_I2C == enType)
    {
        pstDrvCtx->stBusCb.pfnISPWriteI2CData = pstBusCb->pfnISPWriteI2CData;
    }
    else if (ISP_BUS_TYPE_SSP == enType)
    {
        pstDrvCtx->stBusCb.pfnISPWriteSSPData = pstBusCb->pfnISPWriteSSPData;
    }
    else
    {
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        osal_printk("The bus type %d registerd to isp is err!", enType);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
    return HI_SUCCESS;
}

HI_S32 ISP_RegisterPirisCallBack (VI_PIPE ViPipe, ISP_PIRIS_CALLBACK_S *pstPirisCb)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_POINTER(pstPirisCb);
    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->stPirisCb.pfn_piris_gpio_update = pstPirisCb->pfn_piris_gpio_update;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_RegisterViBusCallBack (HI_S32 ViPipe, ISP_VIBUS_CALLBACK_S *pstViBusCb)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    ISP_CHECK_POINTER(pstViBusCb);
    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->stViBusCb.pfnIspBeStaRead = pstViBusCb->pfnIspBeStaRead;
    pstDrvCtx->stViBusCb.pfnIspBeCfgRead = pstViBusCb->pfnIspBeCfgRead;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

/* vi get isp_config from isp*/
HI_S32 ISP_SaveSnapConfig(VI_PIPE ViPipe, ISP_CONFIG_INFO_S *pstSnapInfo)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    HI_U8 u8CfgDlyMax = 0;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstSnapInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    u8CfgDlyMax = MAX2(pstDrvCtx->stSyncCfg.u8Cfg2VldDlyMAX, 2);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    if (0 == g_UpdatePos[ViPipe]) /* frame start */
    {
        osal_memcpy(pstSnapInfo, &pstDrvCtx->astSnapInfoSave[u8CfgDlyMax - 1], sizeof(ISP_CONFIG_INFO_S));
    }
    else                 /* frame end */
    {
        osal_memcpy(pstSnapInfo, &pstDrvCtx->astSnapInfoSave[u8CfgDlyMax - 2], sizeof(ISP_CONFIG_INFO_S));
    }
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

/* vi send Proenable*/
HI_S32 ISP_SetProEnable(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->bProEnable = HI_TRUE;
    pstDrvCtx->bProStart  = HI_FALSE;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
    return HI_SUCCESS;
}

/*vi get pubAttr*/
HI_S32 ISP_GetPubAttr(VI_PIPE ViPipe, ISP_PUB_ATTR_S *pstPubAttr)
{
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstPubAttr);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    osal_memcpy(pstPubAttr, &pstDrvCtx->stProcPubInfo, sizeof(ISP_PUB_ATTR_S));
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

/* vi get ProCtrl*/
HI_BOOL ISP_GetProCtrl(VI_PIPE ViPipe, ISP_PRO_CTRL_S *pstProCtrl)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    HI_U8 u8CfgDlyMax = 0;
    HI_BOOL bProStart = HI_FALSE;

    if ((ViPipe < 0) || (ViPipe >= ISP_MAX_PIPE_NUM))
    {
        ISP_TRACE(HI_DBG_ERR, "Err isp pipe %d!\n", ViPipe);
        return bProStart;
    }

    if (HI_NULL == pstProCtrl)
    {
        ISP_TRACE(HI_DBG_ERR, "Null Pointer!\n");
        return bProStart;
    }

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    u8CfgDlyMax = MAX2(pstDrvCtx->stSyncCfg.u8Cfg2VldDlyMAX, 1);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    if (0 == g_UpdatePos[ViPipe])
    {
        pstProCtrl->u8Vcnum = u8CfgDlyMax;
    }
    else
    {
        pstProCtrl->u8Vcnum = u8CfgDlyMax - 1;
    }
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if (2 == pstDrvCtx->u32ProTrigFlag)
    {
        bProStart = pstDrvCtx->bProStart;
    }
    else
    {
        bProStart = HI_FALSE;
    }
    return bProStart;
}

HI_S32 ISP_SetSnapAttr(VI_PIPE ViPipe, ISP_SNAP_ATTR_S *pstSnapAttr)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    HI_U8 i;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstSnapAttr);

    for (i = 0; i < ISP_MAX_PIPE_NUM; i++)
    {
        pstDrvCtx = ISP_DRV_GET_CTX(i);

        if (!pstDrvCtx->bMemInit)
        {
            continue;
        }

        if ((i == pstSnapAttr->s32PicturePipeId) || (i == pstSnapAttr->s32PreviewPipeId))
        {
            osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
            osal_memcpy(&pstDrvCtx->stSnapAttr, pstSnapAttr, sizeof(ISP_SNAP_ATTR_S));
            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        }
    }
    return HI_SUCCESS;
}

static HI_S32 ISP_CheckProNrParam(ISP_NR_AUTO_ATTR_S *pstNrAttr)
{
    HI_U8 i;

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        if (pstNrAttr->au8ChromaStr[0][i] > 3)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid  au8ChromaStr[0][%d] Input!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstNrAttr->au8ChromaStr[1][i] > 3)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid  au8ChromaStr[1][%d] Input!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstNrAttr->au8ChromaStr[2][i] > 3)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid  au8ChromaStr[2][%d] Input!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstNrAttr->au8ChromaStr[3][i] > 3)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid  au8ChromaStr[3][%d] Input!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstNrAttr->au16CoarseStr[0][i] > 0x360)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid  au16CoarseStr[0][%d] Input!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstNrAttr->au16CoarseStr[1][i] > 0x360)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid  au16CoarseStr[1][%d] Input!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstNrAttr->au16CoarseStr[2][i] > 0x360)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid  au16CoarseStr[2][%d] Input!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstNrAttr->au16CoarseStr[3][i] > 0x360)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid  au16CoarseStr[3][%d] Input!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstNrAttr->au8FineStr[i] > 128)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid  au8FineStr[%d] Input!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstNrAttr->au16CoringWgt[i] > 3200)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid  au16CoringWgt[%d] Input!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SetProNrParam(VI_PIPE ViPipe, const ISP_PRO_BNR_PARAM_S *pstProNrParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ParamIdex = 0;
    HI_U32 u32ParamNum = 0;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_NR_AUTO_ATTR_S *pstProNrAttr = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstProNrParam);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_NULL == pstProNrParam->pastNrAttr)
    {
        ISP_TRACE(HI_DBG_ERR, "NrParam buf address can't null!\n");
        s32Ret =  HI_ERR_ISP_NOT_INIT;
        goto failed0;
    }

    if (HI_FALSE == read_user_linear_space_valid((HI_U8 *)(pstProNrParam->pastNrAttr), sizeof(ISP_NR_AUTO_ATTR_S)))
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d pastShpAttr address is invalid!\n", ViPipe);
        s32Ret =  HI_ERR_ISP_INVALID_ADDR;
        goto failed0;
    }

    ISP_CHECK_BOOL(pstProNrParam->bEnable);

    u32ParamNum = pstProNrParam->u32ParamNum;

    if ((u32ParamNum > PRO_MAX_FRAME_NUM) || (u32ParamNum < 1))
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u32ParamNum value %d! Value range of TextureStr is [1, %d]\n", u32ParamNum, PRO_MAX_FRAME_NUM);
        s32Ret = HI_ERR_ISP_ILLEGAL_PARAM;
        goto failed0;
    }

    pstProNrAttr = osal_vmalloc(sizeof(ISP_NR_AUTO_ATTR_S) * u32ParamNum);
    if (HI_NULL == pstProNrAttr)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d vmalloc for pstProNrAttr failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOMEM;
        goto failed0;
    }

    if (0 != osal_copy_from_user(pstProNrAttr, pstProNrParam->pastNrAttr, \
                                 sizeof(ISP_NR_AUTO_ATTR_S) * u32ParamNum))
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d,copy from user faild !\n", ViPipe);
        s32Ret = HI_FAILURE;
        goto failed0;
    }

    for (u32ParamIdex = 0; u32ParamIdex < u32ParamNum; u32ParamIdex++)
    {
        s32Ret = ISP_CheckProNrParam(&pstProNrAttr[u32ParamIdex]);
        if (HI_SUCCESS != s32Ret)
        {
            goto failed0;
        }
    }

    if (osal_down_interruptible(&pstDrvCtx->stProInfoSem))
    {
        s32Ret = -ERESTARTSYS;
        goto failed0;
    }

    if (HI_FALSE == pstDrvCtx->stProInfo.bInit)
    {
        ISP_TRACE(HI_DBG_WARN, "ProInfo buf doesn't initialized!\n");
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto failed1;
    }

    if (HI_NULL == pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "ShpParam buf don't init ok!\n");
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto failed1;
    }

    ((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->bEnable = pstProNrParam->bEnable;
    ((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->u32ParamNum = u32ParamNum;
    osal_memcpy(&((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->astNrAttr, \
                pstProNrAttr, sizeof(ISP_NR_AUTO_ATTR_S) * u32ParamNum);

    osal_up(&pstDrvCtx->stProInfoSem);

    if (HI_NULL != pstProNrAttr)
    {
        osal_vfree(pstProNrAttr);
    }

    return HI_SUCCESS;

failed1:
    osal_up(&pstDrvCtx->stProInfoSem);
failed0:
    if (HI_NULL != pstProNrAttr)
    {
        osal_vfree(pstProNrAttr);
    }

    return s32Ret;
}

HI_S32 ISP_GetProNrParam(VI_PIPE ViPipe, ISP_PRO_BNR_PARAM_S *pstProNrParam)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstProNrParam);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_NULL == pstProNrParam->pastNrAttr)
    {
        ISP_TRACE(HI_DBG_ERR, "NrParam buf address can't null!\n");
        return HI_ERR_ISP_NOT_INIT;
    }

    if (HI_FALSE == write_user_linear_space_valid((HI_U8 *)(pstProNrParam->pastNrAttr), sizeof(ISP_NR_AUTO_ATTR_S)))
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d pastNrAttr address is invalid!\n", ViPipe);
        return HI_ERR_ISP_INVALID_ADDR;
    }

    if (osal_down_interruptible(&pstDrvCtx->stProInfoSem))
    {
        return -ERESTARTSYS;
    }

    if (HI_FALSE == pstDrvCtx->stProInfo.bInit)
    {
        ISP_TRACE(HI_DBG_WARN, "ProInfo buf doesn't initialized!\n");
        osal_up(&pstDrvCtx->stProInfoSem);
        return HI_ERR_ISP_NOT_INIT;
    }

    if (HI_NULL == pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "ShpParam buf don't init ok!\n");
        osal_up(&pstDrvCtx->stProInfoSem);
        return HI_ERR_ISP_NOT_INIT;
    }

    if (((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->u32ParamNum > PRO_MAX_FRAME_NUM )
    {
        ((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->u32ParamNum = PRO_MAX_FRAME_NUM;
    }

    if (((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->u32ParamNum <  1)
    {
        ((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->u32ParamNum = 1;
    }

    pstProNrParam->bEnable = ((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->bEnable;
    pstProNrParam->u32ParamNum = ((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->u32ParamNum;
    if (0 != osal_copy_to_user(pstProNrParam->pastNrAttr, &((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->astNrAttr, \
                               sizeof(ISP_NR_AUTO_ATTR_S) * (((ISP_PRO_NR_PARAM_S *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr)->u32ParamNum)))
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d,copy to user faild !\n", ViPipe);
        osal_up(&pstDrvCtx->stProInfoSem);
        return HI_FAILURE;
    }

    osal_up(&pstDrvCtx->stProInfoSem);

    return HI_SUCCESS;
}

static HI_S32 ISP_CheckProShpParam(ISP_SHARPEN_AUTO_ATTR_S *pstShpAttr)
{
    HI_U8 i, j;

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        for (j = 0; j < ISP_SHARPEN_GAIN_NUM; j++)
        {
            if (pstShpAttr->au16TextureStr[j][i] > 4095)
            {
                ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of TextureStr is [0, 4095]\n", pstShpAttr->au16TextureStr[j][i]);
                return HI_ERR_ISP_ILLEGAL_PARAM;
            }
            if (pstShpAttr->au16EdgeStr[j][i] > 4095 )
            {
                ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of EdgeStr is [0, 4095]\n", pstShpAttr->au16EdgeStr[j][i]);
                return HI_ERR_ISP_ILLEGAL_PARAM;
            }
            if (pstShpAttr->au8LumaWgt[j][i] > 127 )
            {
                ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of LumaWgt is [0, 127]\n", pstShpAttr->au8LumaWgt[j][i]);
                return HI_ERR_ISP_ILLEGAL_PARAM;
            }
        }
        if (pstShpAttr->au16TextureFreq[i] > 4095)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of TextureFreq is [0, 4095]\n", pstShpAttr->au16TextureFreq[i]);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstShpAttr->au16EdgeFreq[i] > 4095 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of EdgeFreq is [0, 4095]\n", pstShpAttr->au16EdgeFreq[i]);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstShpAttr->au8OverShoot[i] > 127 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of OverShoot is [0, 127]\n", pstShpAttr->au8OverShoot[i]);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstShpAttr->au8UnderShoot[i] > 127 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of UnderShoot is [0, 127]\n", pstShpAttr->au8UnderShoot[i]);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstShpAttr->au8EdgeFiltStr[i] > 63 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of EdgeFiltStr is [0, 63]\n", pstShpAttr->au8EdgeFiltStr[i]);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstShpAttr->au8RGain[i] > HI_ISP_SHARPEN_RGAIN )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of au8RGain is [0, %d]\n", pstShpAttr->au8RGain[i], HI_ISP_SHARPEN_RGAIN);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstShpAttr->au8BGain[i] > HI_ISP_SHARPEN_BGAIN )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of au8BGain is [0, %d]\n", pstShpAttr->au8BGain[i], HI_ISP_SHARPEN_BGAIN);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstShpAttr->au8SkinGain[i] > 31 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid Isp Sharpen value:%d! Value range of au8SkinGain is [0, 31]\n", pstShpAttr->au8SkinGain[i]);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SetProShpParam(VI_PIPE ViPipe, const ISP_PRO_SHARPEN_PARAM_S *pstProShpParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ParamIdex = 0;
    HI_U32 u32ParamNum = 0;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_SHARPEN_AUTO_ATTR_S *pastProShpAttr = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstProShpParam);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_NULL == pstProShpParam->pastShpAttr)
    {
        ISP_TRACE(HI_DBG_ERR, "ShpParam buf address can't null!\n");
        s32Ret = HI_ERR_ISP_NULL_PTR;
        goto failed0;
    }

    if (HI_FALSE == read_user_linear_space_valid((HI_U8 *)(pstProShpParam->pastShpAttr), sizeof(ISP_SHARPEN_AUTO_ATTR_S)))
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d pastShpAttr address is invalid!\n", ViPipe);
        s32Ret = HI_ERR_ISP_INVALID_ADDR;
        goto failed0;
    }

    ISP_CHECK_BOOL(pstProShpParam->bEnable);

    u32ParamNum = pstProShpParam->u32ParamNum;

    if ((u32ParamNum > PRO_MAX_FRAME_NUM) || (u32ParamNum < 1))
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u32ParamNum value %d! Value range of u32ParamNum is [1, %d]\n", u32ParamNum, PRO_MAX_FRAME_NUM);
        s32Ret = HI_ERR_ISP_ILLEGAL_PARAM;
        goto failed0;
    }

    pastProShpAttr = osal_vmalloc(sizeof(ISP_SHARPEN_AUTO_ATTR_S) * u32ParamNum);
    if (HI_NULL == pastProShpAttr)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d vmalloc for pastProShpAttr failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOMEM;
        goto failed0;
    }

    if (0 != osal_copy_from_user(pastProShpAttr, pstProShpParam->pastShpAttr, \
                                 sizeof(ISP_SHARPEN_AUTO_ATTR_S) * u32ParamNum))
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d,copy from user faild !\n", ViPipe);
        s32Ret = HI_FAILURE;
        goto failed0;
    }

    for (u32ParamIdex = 0; u32ParamIdex < u32ParamNum; u32ParamIdex++)
    {
        s32Ret = ISP_CheckProShpParam(&pastProShpAttr[u32ParamIdex]);
        if (HI_SUCCESS != s32Ret)
        {
            goto failed0;
        }
    }

    if (osal_down_interruptible(&pstDrvCtx->stProInfoSem))
    {
        s32Ret = -ERESTARTSYS;
        goto failed0;
    }

    if (HI_FALSE == pstDrvCtx->stProInfo.bInit)
    {
        ISP_TRACE(HI_DBG_WARN, "ProInfo buf doesn't initialized!\n");
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto failed1;
    }

    if (HI_NULL == pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "ShpParam buf don't init ok!\n");
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto failed1;
    }

    ((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->bEnable = pstProShpParam->bEnable;
    ((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->u32ParamNum = u32ParamNum;
    osal_memcpy(&((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->astShpAttr, \
                pastProShpAttr, sizeof(ISP_SHARPEN_AUTO_ATTR_S) * u32ParamNum);

    osal_up(&pstDrvCtx->stProInfoSem);

    if (HI_NULL != pastProShpAttr)
    {
        osal_vfree(pastProShpAttr);
    }

    return HI_SUCCESS;

failed1:
    osal_up(&pstDrvCtx->stProInfoSem);
failed0:
    if (HI_NULL != pastProShpAttr)
    {
        osal_vfree(pastProShpAttr);
    }

    return s32Ret;
}

HI_S32 ISP_GetProShpParam(VI_PIPE ViPipe, ISP_PRO_SHARPEN_PARAM_S *pstProShpParam)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstProShpParam);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_NULL == pstProShpParam->pastShpAttr)
    {
        ISP_TRACE(HI_DBG_ERR, "ShpParam buf address can't null!\n");
        return HI_ERR_ISP_NOT_INIT;
    }

    if (HI_FALSE == write_user_linear_space_valid((HI_U8 *)(pstProShpParam->pastShpAttr), sizeof(ISP_SHARPEN_AUTO_ATTR_S)))
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d pastShpAttr address is invalid!\n", ViPipe);
        return HI_ERR_ISP_INVALID_ADDR;
    }

    if (osal_down_interruptible(&pstDrvCtx->stProInfoSem))
    {
        return -ERESTARTSYS;
    }

    if (HI_FALSE == pstDrvCtx->stProInfo.bInit)
    {
        ISP_TRACE(HI_DBG_WARN, "ProInfo buf doesn't initialized!\n");
        osal_up(&pstDrvCtx->stProInfoSem);
        return HI_ERR_ISP_NOT_INIT;
    }

    if (HI_NULL == pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "ShpParam buf don't init ok!\n");
        osal_up(&pstDrvCtx->stProInfoSem);
        return HI_ERR_ISP_NOT_INIT;
    }

    if (((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->u32ParamNum > PRO_MAX_FRAME_NUM )
    {
        ((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->u32ParamNum = PRO_MAX_FRAME_NUM;
    }

    if (((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->u32ParamNum <  1)
    {
        ((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->u32ParamNum = 1;
    }

    pstProShpParam->bEnable = ((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->bEnable;
    pstProShpParam->u32ParamNum = ((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->u32ParamNum;
    if (0 != osal_copy_to_user(pstProShpParam->pastShpAttr, &((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->astShpAttr, \
                               sizeof(ISP_SHARPEN_AUTO_ATTR_S) * (((ISP_PRO_SHP_PARAM_S *)pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr)->u32ParamNum)))
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe:%d,copy to user faild !\n", ViPipe);
        osal_up(&pstDrvCtx->stProInfoSem);
        return HI_FAILURE;
    }

    osal_up(&pstDrvCtx->stProInfoSem);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeEndIntProc(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_TRUE == pstDrvCtx->bRunOnceOk)
    {
        ISP_DRV_StatBufBusyPut(ViPipe);
        pstDrvCtx->bRunOnceOk = HI_FALSE;
    }

    if (HI_TRUE == pstDrvCtx->bYUVRunOnceOk)
    {
        ISP_DRV_StatBufBusyPut(ViPipe);
        pstDrvCtx->bYUVRunOnceOk = HI_FALSE;
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->bVdBeEnd = HI_TRUE;
    osal_wakeup(&pstDrvCtx->stIspWaitVdBeEnd);
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ProcInit(VI_PIPE ViPipe, ISP_PROC_MEM_S *pstIspProcMem)
{
    HI_S32 s32Ret;
    HI_U64 u64PhyAddr;
    HI_U8  *pu8VirAddr;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_CHAR acName[MAX_MMZ_NAMELEN] = {0};

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstIspProcMem);

    if (0 == g_ProcParam[ViPipe])
    {
        return HI_SUCCESS;
    }

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stPorcMem.bInit);

    osal_snprintf(acName, sizeof(acName), "ISP[%d].Proc", ViPipe);

    s32Ret = CMPI_MmzMallocNocache(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, ISP_PROC_SIZE);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "alloc proc buf err\n");
        return HI_ERR_ISP_NOMEM;
    }

    ((HI_CHAR *)pu8VirAddr)[0] = '\0';
    ((HI_CHAR *)pu8VirAddr)[ISP_PROC_SIZE - 1] = '\0';

    if (osal_down_interruptible(&pstDrvCtx->stProcSem))
    {
        if (0 != u64PhyAddr)
        {
            CMPI_MmzFree(u64PhyAddr, pu8VirAddr);
        }
        return -ERESTARTSYS;
    }

    pstDrvCtx->stPorcMem.bInit          = HI_TRUE;
    pstDrvCtx->stPorcMem.u64ProcPhyAddr = u64PhyAddr;
    pstDrvCtx->stPorcMem.u32ProcSize    = ISP_PROC_SIZE;
    pstDrvCtx->stPorcMem.pProcVirtAddr  = (HI_VOID *)pu8VirAddr;

    osal_memcpy(pstIspProcMem, &pstDrvCtx->stPorcMem, sizeof(ISP_PROC_MEM_S));

    osal_up(&pstDrvCtx->stProcSem);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ProcExit(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U64 u64PhyAddr;
    HI_U8  *pu8VirAddr;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_EXIT_STATE(ViPipe);

    if (0 == g_ProcParam[ViPipe])
    {
        return HI_SUCCESS;
    }

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stPorcMem.bInit);

    u64PhyAddr = pstDrvCtx->stPorcMem.u64ProcPhyAddr;
    pu8VirAddr = (HI_U8 *)pstDrvCtx->stPorcMem.pProcVirtAddr;

    if (osal_down_interruptible(&pstDrvCtx->stProcSem))
    {
        return -ERESTARTSYS;
    }

    pstDrvCtx->stPorcMem.bInit          = HI_FALSE;
    pstDrvCtx->stPorcMem.u64ProcPhyAddr = 0;
    pstDrvCtx->stPorcMem.u32ProcSize    = 0;
    pstDrvCtx->stPorcMem.pProcVirtAddr  = HI_NULL;
    osal_up(&pstDrvCtx->stProcSem);

    if (0 != u64PhyAddr)
    {
        CMPI_MmzFree(u64PhyAddr, pu8VirAddr);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ProcPrintf(VI_PIPE ViPipe, osal_proc_entry_t *s)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U32 u32ProcBufLen;
    const HI_CHAR *pszStr;
    HI_CHAR *pszBuf;

    ISP_CHECK_PIPE(ViPipe);

    if (0 == g_ProcParam[ViPipe])
    {
        return HI_SUCCESS;
    }

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (osal_down_interruptible(&pstDrvCtx->stProcSem))
    {
        return -ERESTARTSYS;
    }

    if (HI_NULL != pstDrvCtx->stPorcMem.pProcVirtAddr)
    {
        pszBuf = osal_kmalloc((PROC_PRT_SLICE_SIZE + 1), osal_gfp_atomic);

        if (!pszBuf)
        {
            ISP_TRACE(HI_DBG_ERR, "ISP_DRV_ProcPrintf malloc slice buf err\n");
            osal_up(&pstDrvCtx->stProcSem);
            return HI_ERR_ISP_NULL_PTR;
        }

        pszBuf[PROC_PRT_SLICE_SIZE] = '\0';
        pszStr = (HI_CHAR *)pstDrvCtx->stPorcMem.pProcVirtAddr;
        u32ProcBufLen = osal_strlen((HI_CHAR *)pstDrvCtx->stPorcMem.pProcVirtAddr);

        while (u32ProcBufLen)
        {
            osal_strncpy(pszBuf, pszStr, PROC_PRT_SLICE_SIZE);
            osal_seq_printf(s, "%s", pszBuf);
            pszStr += PROC_PRT_SLICE_SIZE;

            if (u32ProcBufLen < PROC_PRT_SLICE_SIZE)
            {
                u32ProcBufLen = 0;
            }
            else
            {
                u32ProcBufLen -= PROC_PRT_SLICE_SIZE;
            }
        }

        osal_kfree((HI_VOID *)pszBuf);
    }

    osal_up(&pstDrvCtx->stProcSem);

    return HI_SUCCESS;
}

HI_S32 ISP_UpdateInfoSync(VI_PIPE ViPipe, ISP_DCF_UPDATE_INFO_S *pstIspUpdateInfo)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    HI_S32 i;
    ISP_DCF_UPDATE_INFO_S *pUpdateInfoVirAddr = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (HI_NULL == pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "UpdateInfo buf don't init ok!\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_ERR_ISP_NOT_INIT;
    }

    pUpdateInfoVirAddr = (ISP_DCF_UPDATE_INFO_S *)pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr;

    for (i = ISP_MAX_UPDATEINFO_BUF_NUM - 1; i >= 1; i--)
    {
        osal_memcpy(pUpdateInfoVirAddr + i, pUpdateInfoVirAddr + i - 1, sizeof(ISP_DCF_UPDATE_INFO_S));
    }
    osal_memcpy(pUpdateInfoVirAddr, pstIspUpdateInfo, sizeof(ISP_DCF_UPDATE_INFO_S));
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_frameInfoSync(VI_PIPE ViPipe, ISP_FRAME_INFO_S *pstIspframeInfo)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    HI_S32 i;
    ISP_FRAME_INFO_S *pframeInfoVirAddr = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (HI_NULL == pstDrvCtx->stTransInfo.stFrameInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "UpdateInfo buf don't init ok!\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_ERR_ISP_NOT_INIT;
    }

    pframeInfoVirAddr = (ISP_FRAME_INFO_S *)pstDrvCtx->stTransInfo.stFrameInfo.pVirAddr;

    for (i = ISP_MAX_FRAMEINFO_BUF_NUM - 1; i >= 1; i--)
    {
        osal_memcpy(pframeInfoVirAddr + i, pframeInfoVirAddr + i - 1, sizeof(ISP_FRAME_INFO_S));
    }
    osal_memcpy(pframeInfoVirAddr, pstIspframeInfo, sizeof(ISP_FRAME_INFO_S));
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}


HI_S32 ISP_CalSyncInfoIndex(VI_PIPE ViPipe, HI_S32 *ps32Index)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U8 u8CfgDlyMax = 0;
    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    u8CfgDlyMax = MAX2(pstDrvCtx->stSyncCfg.u8Cfg2VldDlyMAX, 2);
    if (0 == g_UpdatePos[ViPipe])
    {
        *ps32Index = u8CfgDlyMax - 1;
    }
    else
    {
        *ps32Index = u8CfgDlyMax - 2;
    }

    *ps32Index = CLIP3(*ps32Index, 0, CFG2VLD_DLY_LIMIT - 1);

    return HI_SUCCESS;
}

HI_S32 ISP_GetDCFInfo(VI_PIPE ViPipe, ISP_DCF_INFO_S *pstIspDCF)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_DCF_UPDATE_INFO_S *pstIspUpdateInfo = HI_NULL;
    ISP_DCF_CONST_INFO_S *pstIspDCFConstInfo = HI_NULL;
    unsigned long u32Flags;
    HI_S32 s32Index = 0;
    ISP_DCF_UPDATE_INFO_S *pUpdateInfoVirAddr = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_TRANBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstIspDCF);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (HI_NULL == pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "UpdateInfo buf don't init ok!\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_ERR_ISP_NOT_INIT;
    }

    pUpdateInfoVirAddr = (ISP_DCF_UPDATE_INFO_S *)pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr;
    ISP_CalSyncInfoIndex(ViPipe, &s32Index);

    pstIspUpdateInfo = pUpdateInfoVirAddr + s32Index;

    pstIspDCFConstInfo = (ISP_DCF_CONST_INFO_S *)(pUpdateInfoVirAddr + ISP_MAX_UPDATEINFO_BUF_NUM);

    osal_memcpy(&pstIspDCF->stIspDCFConstInfo, pstIspDCFConstInfo, sizeof(ISP_DCF_CONST_INFO_S));
    osal_memcpy(&pstIspDCF->stIspDCFUpdateInfo, pstIspUpdateInfo, sizeof(ISP_DCF_UPDATE_INFO_S));
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_SetDCFInfo(VI_PIPE ViPipe, ISP_DCF_INFO_S *pstIspDCF)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_DCF_UPDATE_INFO_S *pstIspUpdateInfo = HI_NULL;
    ISP_DCF_CONST_INFO_S *pstIspDCFConstInfo = HI_NULL;
    unsigned long u32Flags;
    HI_S32 s32Index = 0;
    ISP_DCF_UPDATE_INFO_S *pUpdateInfoVirAddr = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_TRANBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstIspDCF);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (HI_NULL == pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "UpdateInfo buf don't init ok!\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_ERR_ISP_NOT_INIT;
    }

    pUpdateInfoVirAddr = (ISP_DCF_UPDATE_INFO_S *)pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr;

    ISP_CalSyncInfoIndex(ViPipe, &s32Index);
    pstIspUpdateInfo = pUpdateInfoVirAddr + s32Index;

    pstIspDCFConstInfo = (ISP_DCF_CONST_INFO_S *)(pUpdateInfoVirAddr + ISP_MAX_UPDATEINFO_BUF_NUM);

    osal_memcpy(pstIspDCFConstInfo, &pstIspDCF->stIspDCFConstInfo, sizeof(ISP_DCF_CONST_INFO_S));
    osal_memcpy(pstIspUpdateInfo, &pstIspDCF->stIspDCFUpdateInfo, sizeof(ISP_DCF_UPDATE_INFO_S));
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_GetIspUpdateInfo(VI_PIPE ViPipe, ISP_DCF_UPDATE_INFO_S *pstIspUpdateInfo)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    ISP_DCF_UPDATE_INFO_S *pUpdateInfoVirAddr = HI_NULL;
    HI_S32 s32Index = 0;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_TRANBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstIspUpdateInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (HI_NULL == pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "UpdateInfo buf don't init ok!\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_ERR_ISP_NOT_INIT;
    }

    pUpdateInfoVirAddr = (ISP_DCF_UPDATE_INFO_S *)pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr;

    ISP_CalSyncInfoIndex(ViPipe, &s32Index);
    osal_memcpy(pstIspUpdateInfo, pUpdateInfoVirAddr + s32Index, sizeof(ISP_DCF_UPDATE_INFO_S));

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_SetIspUpdateInfo(VI_PIPE ViPipe, ISP_DCF_UPDATE_INFO_S *pstIspUpdateInfo)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    HI_S32 s32Index = 0;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_TRANBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstIspUpdateInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (NULL == pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "UpdateInfo buf don't init ok!\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_ERR_ISP_NOT_INIT;
    }

    ISP_CalSyncInfoIndex(ViPipe, &s32Index);
    osal_memcpy((ISP_DCF_UPDATE_INFO_S *)pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr + s32Index, \
                pstIspUpdateInfo, sizeof(ISP_DCF_UPDATE_INFO_S));

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ProInfoBufInit(VI_PIPE ViPipe, ISP_PRO_INFO_BUF_S *pstProInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U64 u64PhyAddr, u64Size = 0;
    HI_U8  *pu8VirAddr;
    HI_U32 u32SizeProNrParam, u32SizeProShpParam;
    HI_CHAR acName[MAX_MMZ_NAMELEN] = {0};

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstProInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stProInfo.bInit);

    osal_snprintf(acName, sizeof(acName), "ISP[%d].Pro", ViPipe);

    u32SizeProNrParam  = sizeof(ISP_PRO_NR_PARAM_S);
    u32SizeProShpParam = sizeof(ISP_PRO_SHP_PARAM_S);

    u64Size += u32SizeProNrParam;
    u64Size += u32SizeProShpParam;

    s32Ret = CMPI_MmzMallocNocache(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, u64Size);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] alloc ISP Pro info buf err\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    osal_memset(pu8VirAddr, 0, u64Size);

    if (osal_down_interruptible(&pstDrvCtx->stProInfoSem))
    {
        if (0 != u64PhyAddr)
        {
            CMPI_MmzFree(u64PhyAddr, pu8VirAddr);
        }
        return -ERESTARTSYS;
    }
    pstDrvCtx->stProInfo.bInit                        = HI_TRUE;

    pstDrvCtx->stProInfo.stProNrParamInfo.u64PhyAddr  = u64PhyAddr;
    pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr    = (HI_VOID *)pu8VirAddr;

    pstDrvCtx->stProInfo.stProShpParamInfo.u64PhyAddr = \
            pstDrvCtx->stProInfo.stProNrParamInfo.u64PhyAddr + u32SizeProNrParam;;
    pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr   = \
            (HI_VOID *)((HI_U8 *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr + u32SizeProNrParam);

    osal_memcpy(pstProInfo, &pstDrvCtx->stProInfo, sizeof(ISP_PRO_INFO_BUF_S));

    osal_up(&pstDrvCtx->stProInfoSem);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ProInfoBufExit(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U64 u64PhyAddr;
    HI_U8  *pu8VirAddr;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_EXIT_STATE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stProInfo.bInit);

    u64PhyAddr = pstDrvCtx->stProInfo.stProNrParamInfo.u64PhyAddr;
    pu8VirAddr = (HI_U8 *)pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr;

    if (osal_down_interruptible(&pstDrvCtx->stProInfoSem))
    {
        return -ERESTARTSYS;
    }

    pstDrvCtx->stProInfo.bInit                        = HI_FALSE;

    pstDrvCtx->stProInfo.stProNrParamInfo.u64PhyAddr  = 0;
    pstDrvCtx->stProInfo.stProNrParamInfo.pVirAddr    = HI_NULL;

    pstDrvCtx->stProInfo.stProShpParamInfo.u64PhyAddr = 0;
    pstDrvCtx->stProInfo.stProShpParamInfo.pVirAddr   = HI_NULL;

    osal_up(&pstDrvCtx->stProInfoSem);

    if (0 != u64PhyAddr)
    {
        CMPI_MmzFree(u64PhyAddr, pu8VirAddr);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_TransInfoBufInit(VI_PIPE ViPipe, ISP_TRANS_INFO_BUF_S *pstTransInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U64 u64PhyAddr, u64Size = 0;
    HI_U8  *pu8VirAddr;
    HI_U32 u32SizeDngInfo;
    HI_U32 u32SizeUpdateInfo, u32SizeFrameInfo;
    HI_U32 u32SizeAttachInfo, u32SizeColorGammut;
    HI_CHAR acName[MAX_MMZ_NAMELEN] = {0};
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstTransInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stTransInfo.bInit);

    osal_snprintf(acName, sizeof(acName), "ISP[%d].Trans", ViPipe);

    u32SizeDngInfo     = sizeof(DNG_IMAGE_STATIC_INFO_S);
    u32SizeAttachInfo  = sizeof(ISP_ATTACH_INFO_S);
    u32SizeColorGammut = sizeof(ISP_COLORGAMMUT_INFO_S);
    u32SizeFrameInfo   = sizeof(ISP_FRAME_INFO_S) * ISP_MAX_FRAMEINFO_BUF_NUM;
    u32SizeUpdateInfo  = sizeof(ISP_DCF_UPDATE_INFO_S) * ISP_MAX_UPDATEINFO_BUF_NUM + sizeof(ISP_DCF_CONST_INFO_S);

    u64Size += u32SizeDngInfo;
    u64Size += u32SizeAttachInfo;
    u64Size += u32SizeColorGammut;
    u64Size += u32SizeFrameInfo;
    u64Size += u32SizeUpdateInfo;

    s32Ret = CMPI_MmzMallocNocache(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, u64Size);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] alloc ISP Trans info buf err\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    osal_memset(pu8VirAddr, 0, u64Size);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->stTransInfo.bInit                        = HI_TRUE;

    pstDrvCtx->stTransInfo.stDngInfo.u64PhyAddr         = u64PhyAddr;
    pstDrvCtx->stTransInfo.stDngInfo.pVirAddr           = (HI_VOID *)pu8VirAddr;

    pstDrvCtx->stTransInfo.stAttaInfo.u64PhyAddr        = \
            pstDrvCtx->stTransInfo.stDngInfo.u64PhyAddr + u32SizeDngInfo;
    pstDrvCtx->stTransInfo.stAttaInfo.pVirAddr          = \
            (HI_VOID *)((HI_U8 *)pstDrvCtx->stTransInfo.stDngInfo.pVirAddr + u32SizeDngInfo);

    pstDrvCtx->stTransInfo.stColorGammutInfo.u64PhyAddr = \
            pstDrvCtx->stTransInfo.stAttaInfo.u64PhyAddr + u32SizeAttachInfo;
    pstDrvCtx->stTransInfo.stColorGammutInfo.pVirAddr   = \
            (HI_VOID *)((HI_U8 *)pstDrvCtx->stTransInfo.stAttaInfo.pVirAddr + u32SizeAttachInfo);

    pstDrvCtx->stTransInfo.stFrameInfo.u64PhyAddr       = \
            pstDrvCtx->stTransInfo.stColorGammutInfo.u64PhyAddr + u32SizeColorGammut;;
    pstDrvCtx->stTransInfo.stFrameInfo.pVirAddr         = \
            (HI_VOID *)((HI_U8 *)pstDrvCtx->stTransInfo.stColorGammutInfo.pVirAddr + u32SizeColorGammut);

    pstDrvCtx->stTransInfo.stUpdateInfo.u64PhyAddr      = \
            pstDrvCtx->stTransInfo.stFrameInfo.u64PhyAddr + u32SizeFrameInfo;;
    pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr        = \
            (HI_VOID *)((HI_U8 *)pstDrvCtx->stTransInfo.stFrameInfo.pVirAddr + u32SizeFrameInfo);

    osal_memcpy(pstTransInfo, &pstDrvCtx->stTransInfo, sizeof(ISP_TRANS_INFO_BUF_S));

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_TransInfoBufExit(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U64 u64PhyAddr;
    HI_U8  *pu8VirAddr;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_EXIT_STATE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stTransInfo.bInit);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    u64PhyAddr = pstDrvCtx->stTransInfo.stDngInfo.u64PhyAddr;
    pu8VirAddr = (HI_U8 *)pstDrvCtx->stTransInfo.stDngInfo.pVirAddr;

    pstDrvCtx->stTransInfo.bInit                        = HI_FALSE;

    pstDrvCtx->stTransInfo.stDngInfo.u64PhyAddr         = 0;
    pstDrvCtx->stTransInfo.stDngInfo.pVirAddr           = HI_NULL;

    pstDrvCtx->stTransInfo.stAttaInfo.u64PhyAddr        = 0;
    pstDrvCtx->stTransInfo.stAttaInfo.pVirAddr          = HI_NULL;

    pstDrvCtx->stTransInfo.stColorGammutInfo.u64PhyAddr = 0;
    pstDrvCtx->stTransInfo.stColorGammutInfo.pVirAddr   = HI_NULL;

    pstDrvCtx->stTransInfo.stFrameInfo.u64PhyAddr       = 0;
    pstDrvCtx->stTransInfo.stFrameInfo.pVirAddr         = HI_NULL;

    pstDrvCtx->stTransInfo.stUpdateInfo.u64PhyAddr      = 0;
    pstDrvCtx->stTransInfo.stUpdateInfo.pVirAddr        = HI_NULL;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if (0 != u64PhyAddr)
    {
        CMPI_MmzFree(u64PhyAddr, pu8VirAddr);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SetFrameInfo(VI_PIPE ViPipe, ISP_FRAME_INFO_S *pstIspFrame)
{
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_S32 s32Index = 0;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_TRANBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstIspFrame);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (NULL == pstDrvCtx->stTransInfo.stFrameInfo.pVirAddr)
    {
        ISP_TRACE(HI_DBG_WARN, "frameinfo buf don't init ok!\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_ERR_ISP_NOT_INIT;
    }
    ISP_CalSyncInfoIndex(ViPipe, &s32Index);

    osal_memcpy((ISP_FRAME_INFO_S *)pstDrvCtx->stTransInfo.stFrameInfo.pVirAddr, pstIspFrame, sizeof(ISP_FRAME_INFO_S));
    osal_memcpy((ISP_FRAME_INFO_S *)pstDrvCtx->stTransInfo.stFrameInfo.pVirAddr + s32Index, \
                pstIspFrame, sizeof(ISP_FRAME_INFO_S));

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_GetFrameInfo(VI_PIPE ViPipe, ISP_FRAME_INFO_S *pstIspFrame)
{
    HI_U8 u8ViPipeS;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtxS = HI_NULL;
    unsigned long u32Flags;
    HI_S32 s32Index = 0;
    ISP_FRAME_INFO_S *pFrameInfoVirAddr = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_TRANBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstIspFrame);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if ( (HI_FALSE == pstDrvCtx->bMemInit) && (WDR_MODE_NONE == pstDrvCtx->stWDRAttr.enWDRMode))
    {
        for (u8ViPipeS = 0; u8ViPipeS < ISP_MAX_PIPE_NUM; u8ViPipeS++)
        {
            pstDrvCtxS = ISP_DRV_GET_CTX(u8ViPipeS);

            if ((pstDrvCtxS->bMemInit == HI_TRUE ) && (IS_WDR_MODE(pstDrvCtxS->stWDRAttr.enWDRMode)))
            {

                osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

                if (pstIspFrame && pstDrvCtxS->stTransInfo.stFrameInfo.pVirAddr)
                {
                    pFrameInfoVirAddr = (ISP_FRAME_INFO_S *)pstDrvCtxS->stTransInfo.stFrameInfo.pVirAddr;
                    ISP_CalSyncInfoIndex(ViPipe, &s32Index);
                    osal_memcpy(pstIspFrame, pFrameInfoVirAddr + s32Index, sizeof(ISP_FRAME_INFO_S));
                }

                osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
            }
        }
    }
    else
    {
        osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

        if (pstIspFrame && pstDrvCtx->stTransInfo.stFrameInfo.pVirAddr)
        {
            pFrameInfoVirAddr = (ISP_FRAME_INFO_S *)pstDrvCtx->stTransInfo.stFrameInfo.pVirAddr;
            ISP_CalSyncInfoIndex(ViPipe, &s32Index);
            osal_memcpy(pstIspFrame, pFrameInfoVirAddr + s32Index, sizeof(ISP_FRAME_INFO_S));
        }

        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_GetAttachInfo(VI_PIPE ViPipe, ISP_ATTACH_INFO_S *pstIspAttachInfo)
{
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_TRANBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstIspAttachInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (pstIspAttachInfo && pstDrvCtx->stTransInfo.stAttaInfo.pVirAddr)
    {
        osal_memcpy(pstIspAttachInfo, (ISP_ATTACH_INFO_S *)pstDrvCtx->stTransInfo.stAttaInfo.pVirAddr, \
                    sizeof(ISP_ATTACH_INFO_S));
    }

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_GetColorGamutInfo(VI_PIPE ViPipe, ISP_COLORGAMMUT_INFO_S *pstIspColorGamutInfo)
{
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_TRANBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstIspColorGamutInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (pstIspColorGamutInfo && pstDrvCtx->stTransInfo.stColorGammutInfo.pVirAddr)
    {
        osal_memcpy(pstIspColorGamutInfo, (ISP_COLORGAMMUT_INFO_S *)pstDrvCtx->stTransInfo.stColorGammutInfo.pVirAddr, \
                    sizeof(ISP_COLORGAMMUT_INFO_S));
    }

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetDngInfo(VI_PIPE ViPipe, DNG_IMAGE_STATIC_INFO_S *pstDngInfo)
{
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_TRANBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstDngInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    if (pstDngInfo && pstDrvCtx->stTransInfo.stDngInfo.pVirAddr)
    {
        osal_memcpy(pstDngInfo, (DNG_IMAGE_STATIC_INFO_S *)pstDrvCtx->stTransInfo.stDngInfo.pVirAddr, \
                    sizeof(DNG_IMAGE_STATIC_INFO_S));
    }

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetDngImageDynamicInfo(VI_PIPE ViPipe, DNG_IMAGE_DYNAMIC_INFO_S *pstDngImageDynamicInfo)
{

    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (0 == g_UpdatePos[ViPipe]) /* frame start */
    {
        osal_memcpy(pstDngImageDynamicInfo, &pstDrvCtx->stDngImageDynamicInfo[1], sizeof(DNG_IMAGE_DYNAMIC_INFO_S));
    }
    else
    {
        osal_memcpy(pstDngImageDynamicInfo, &pstDrvCtx->stDngImageDynamicInfo[0], sizeof(DNG_IMAGE_DYNAMIC_INFO_S));
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SetModParam(ISP_MOD_PARAM_S *pstModParam)
{
    VI_PIPE ViPipe;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_POINTER(pstModParam);

    for (ViPipe = 0; ViPipe < ISP_MAX_PIPE_NUM; ViPipe++)
    {
        pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

        if (HI_TRUE == pstDrvCtx->bMemInit)
        {
            ISP_TRACE(HI_DBG_ERR, "Does not support changed after isp init!\n");
            return HI_ERR_ISP_NOT_SUPPORT;
        }
    }

    if ((0 != pstModParam->u32IntBotHalf) && (1 != pstModParam->u32IntBotHalf))
    {
        ISP_TRACE(HI_DBG_ERR, "u32IntBotHalf must be 0 or 1.\n");
        return HI_ERR_ISP_NOT_SUPPORT;
    }

    if ((0 != pstModParam->u32QuickStart) && (1 != pstModParam->u32QuickStart))
    {
        ISP_TRACE(HI_DBG_ERR, "u32QuickStart must be 0 or 1.\n");
        return HI_ERR_ISP_NOT_SUPPORT;
    }

    g_IntBothalf = pstModParam->u32IntBotHalf;
    g_QuickStart = pstModParam->u32QuickStart;
//#ifndef __HuaweiLite__
    if (g_IntBothalf)
    {
        g_UseBothalf = HI_TRUE;
    }
//#endif

    return HI_SUCCESS;
}

HI_S32 ISP_GetModParam(ISP_MOD_PARAM_S *pstModParam)
{
    ISP_CHECK_POINTER(pstModParam);

    pstModParam->u32IntBotHalf = g_IntBothalf;
    pstModParam->u32QuickStart = g_QuickStart;
    return HI_SUCCESS;
}

HI_S32 ISP_SetCtrlParam(VI_PIPE ViPipe, ISP_CTRL_PARAM_S *pstIspCtrlParam)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstIspCtrlParam);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    /*isp proc can be closed before mem init*/
    /*if isp proc is opened, cannot change u32ProcParam to 0 after mem init*/
    if ((0 != g_ProcParam[ViPipe]) && (0 == pstIspCtrlParam->u32ProcParam) && (HI_TRUE == pstDrvCtx->bMemInit))
    {
        ISP_TRACE(HI_DBG_ERR, "Vipipe:%d u32ProcParam do not support to change %d to 0.\n",
                  ViPipe, g_ProcParam[ViPipe]);
        return HI_ERR_ISP_NOT_SUPPORT;
    }

    /*if isp proc is closed, cannot change u32ProcParam to Non-0 after mem init*/
    if ((0 == g_ProcParam[ViPipe]) && (0 != pstIspCtrlParam->u32ProcParam) && (HI_TRUE == pstDrvCtx->bMemInit))
    {
        ISP_TRACE(HI_DBG_ERR, "Vipipe:%d u32ProcParam do not support to change %d to Non-0.\n",
                  ViPipe, g_ProcParam[ViPipe]);
        return HI_ERR_ISP_NOT_SUPPORT;
    }

    if (!pstIspCtrlParam->u32StatIntvl)
    {
        ISP_TRACE(HI_DBG_ERR, "Vipipe:%d u32StatIntvl must be larger than 0.\n", ViPipe);
        return HI_ERR_ISP_NOT_SUPPORT;
    }

    if ((0 != pstIspCtrlParam->u32UpdatePos) && (1 != pstIspCtrlParam->u32UpdatePos))
    {
        ISP_TRACE(HI_DBG_ERR, "Vipipe:%d u32UpdatePos must be 0 or 1.\n", ViPipe);
        return HI_ERR_ISP_NOT_SUPPORT;
    }

    if (g_UpdatePos[ViPipe] != pstIspCtrlParam->u32UpdatePos)
    {
        if (HI_TRUE == pstDrvCtx->bMemInit)
        {
            ISP_TRACE(HI_DBG_ERR, "Vipipe:%d Does not support changed after isp init (u32UpdatePos)!\n", ViPipe);
            return HI_ERR_ISP_NOT_SUPPORT;
        }
    }

    if (g_PwmNumber[ViPipe] != pstIspCtrlParam->u32PwmNumber)
    {
        if (HI_TRUE == pstDrvCtx->bMemInit)
        {
            ISP_TRACE(HI_DBG_ERR, "Vipipe:%d Does not support changed after isp init (u32PwmNumber)!\n", ViPipe);
            return HI_ERR_ISP_NOT_SUPPORT;
        }
    }

    if (g_PortIntDelay[ViPipe] != pstIspCtrlParam->u32PortIntDelay)
    {
        if (HI_TRUE == pstDrvCtx->bMemInit)
        {
            ISP_TRACE(HI_DBG_ERR, "Vipipe:%d Does not support changed after isp init (u32PortIntDelay)!\n", ViPipe);
            return HI_ERR_ISP_NOT_SUPPORT;
        }
    }

    g_ProcParam[ViPipe]    = pstIspCtrlParam->u32ProcParam;
    g_StatIntvl[ViPipe]    = pstIspCtrlParam->u32StatIntvl;
    g_UpdatePos[ViPipe]    = pstIspCtrlParam->u32UpdatePos;
    g_IntTimeout[ViPipe]   = pstIspCtrlParam->u32IntTimeOut;
    g_PwmNumber[ViPipe]    = pstIspCtrlParam->u32PwmNumber;
    g_PortIntDelay[ViPipe] = pstIspCtrlParam->u32PortIntDelay;

    return HI_SUCCESS;
}

HI_S32 ISP_GetCtrlParam(VI_PIPE ViPipe, ISP_CTRL_PARAM_S *pstIspCtrlParam)
{
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstIspCtrlParam);

    pstIspCtrlParam->u32ProcParam    = g_ProcParam[ViPipe];
    pstIspCtrlParam->u32StatIntvl    = g_StatIntvl[ViPipe];
    pstIspCtrlParam->u32UpdatePos    = g_UpdatePos[ViPipe];
    pstIspCtrlParam->u32IntTimeOut   = g_IntTimeout[ViPipe];
    pstIspCtrlParam->u32PwmNumber    = g_PwmNumber[ViPipe];
    pstIspCtrlParam->u32PortIntDelay = g_PortIntDelay[ViPipe];

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StitchSyncEx(VI_PIPE ViPipe)
{
    HI_U8 k;
    VI_PIPE ViPipeId;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtxS = HI_NULL;

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    for (k = 0; k < pstDrvCtx->stStitchAttr.u8StitchPipeNum; k++)
    {
        ViPipeId = pstDrvCtx->stStitchAttr.as8StitchBindId[k];
        pstDrvCtxS = ISP_DRV_GET_CTX(ViPipeId);

        if (HI_TRUE != pstDrvCtxS->bStitchSync)
        {
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;

}

HI_S32 ISP_DRV_StitchSync(VI_PIPE ViPipe)
{
    HI_U8 k;
    VI_PIPE ViPipeId;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtxS = HI_NULL;

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    for (k = 0; k < pstDrvCtx->stStitchAttr.u8StitchPipeNum; k++)
    {
        ViPipeId = pstDrvCtx->stStitchAttr.as8StitchBindId[k];
        pstDrvCtxS = ISP_DRV_GET_CTX(ViPipeId);

        if (HI_TRUE != pstDrvCtxS->bIspInit)
        {
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetVersion(ISP_VERSION_S *pVersion)
{
    ISP_CHECK_POINTER(pVersion);

    osal_memcpy(&gs_stIspLibInfo, pVersion, sizeof(ISP_VERSION_S));

    gs_stIspLibInfo.u32Magic = VERSION_MAGIC + ISP_MAGIC_OFFSET;
    osal_memcpy(pVersion, &gs_stIspLibInfo, sizeof(ISP_VERSION_S));

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_SetDngInfo(VI_PIPE ViPipe, DNG_IMAGE_DYNAMIC_INFO_S *pstDngImgDynInfo)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstDngImgDynInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_memcpy(&pstDrvCtx->stDngImageDynamicInfo[1], &pstDrvCtx->stDngImageDynamicInfo[0], \
                sizeof(DNG_IMAGE_DYNAMIC_INFO_S));
    osal_memcpy(&pstDrvCtx->stDngImageDynamicInfo[0], pstDngImgDynInfo, \
                sizeof(DNG_IMAGE_DYNAMIC_INFO_S));

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetUserStatBuf(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstStat)
{
    ISP_STAT_INFO_S *pstIspStatInfo = HI_NULL;
    ISP_CHECK_POINTER(pstStat);

    ISP_DRV_StatBufUserGet(ViPipe, &pstIspStatInfo);

    if (HI_NULL == pstIspStatInfo)
    {
        return HI_FAILURE;
    }

    osal_memcpy(pstStat, pstIspStatInfo, sizeof(ISP_STAT_INFO_S));

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetRgbirFormat(VI_PIPE ViPipe, ISP_IRBAYER_FORMAT_E *pRgbirFormat)
{
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pRgbirFormat);
    *pRgbirFormat = g_astIspDrvCtx[ViPipe].enIspRgbirFormat;
    return HI_SUCCESS;
}
HI_S32 ISP_DRV_GetStatInfoActive(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstIspStatInfo)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_STAT_INFO_S stActStatInfo;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstIspStatInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (!pstDrvCtx->stStatisticsBuf.pstActStat)
    {
        ISP_TRACE(HI_DBG_WARN, "Pipe[%d] get statistic active buffer err, stat not ready!\n", ViPipe);
        return HI_FAILURE;
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    osal_memcpy(&stActStatInfo, pstDrvCtx->stStatisticsBuf.pstActStat, sizeof(ISP_STAT_INFO_S));
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    osal_memcpy(pstIspStatInfo, &stActStatInfo, sizeof(ISP_STAT_INFO_S));

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_SetRegKernelCfgs(VI_PIPE ViPipe, ISP_KERNEL_REG_CFG_S *pstRegKernelCfg)
{
    HI_U32 u32Flag;
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstRegKernelCfg);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if ((0 != pstDrvCtx->u32RegCfgInfoFlag) &&
        (1 != pstDrvCtx->u32RegCfgInfoFlag))
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] Err u32RegCfgInfoFlag != 0/1 !!!\n", ViPipe);
    }

    u32Flag = 1 - pstDrvCtx->u32RegCfgInfoFlag;
    osal_memcpy(&pstDrvCtx->astKernelCfg[u32Flag], pstRegKernelCfg, sizeof(ISP_KERNEL_REG_CFG_S));

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstDrvCtx->u32RegCfgInfoFlag = u32Flag;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_SetConfigInfo(VI_PIPE ViPipe, ISP_CONFIG_INFO_S *pstIspConfigInfo)
{
    HI_U32 i;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstIspConfigInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    for (i = ISP_SAVEINFO_MAX - 1; i >= 1; i--)
    {
        osal_memcpy(&pstDrvCtx->astSnapInfoSave[i], &pstDrvCtx->astSnapInfoSave[i - 1], sizeof(ISP_CONFIG_INFO_S));
    }

    osal_memcpy(&pstDrvCtx->astSnapInfoSave[0], pstIspConfigInfo, sizeof(ISP_CONFIG_INFO_S));

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetSnapInfo(VI_PIPE ViPipe, ISP_SNAP_INFO_S *pstIspSnapInfo)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstIspSnapInfo);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_memcpy(pstIspSnapInfo, &pstDrvCtx->stSnapInfoLoad, sizeof(ISP_SNAP_INFO_S));

    if (SNAP_STATE_CFG == pstDrvCtx->stSnapInfoLoad.enSnapState)
    {
        if (0 == g_UpdatePos[ViPipe])
        {
            pstDrvCtx->stSnapInfoLoad.enSnapState = SNAP_STATE_NULL;
        }
    }

    return HI_SUCCESS;
}


HI_VOID ISP_DRV_BeBufQueuePutBusy(VI_PIPE ViPipe)
{
    HI_U64  u64PhyAddr;
    HI_U64  u64Size;
    HI_VOID *pVirAddr;
    ISP_BE_BUF_NODE_S *pstNode;
    struct osal_list_head *pListTmp = HI_NULL;
    struct osal_list_head *pListNode = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (!pstDrvCtx->pstUseNode)
    {
        return;
    }

    osal_list_for_each_safe(pListNode, pListTmp, &pstDrvCtx->stBeBufQueue.stBusyList)
    {
        pstNode = osal_list_entry(pListNode, ISP_BE_BUF_NODE_S, list);

        if (0 == pstNode->s32HoldCnt)
        {
            ISP_QueueDelBusyBeBuf(&pstDrvCtx->stBeBufQueue, pstNode);
            ISP_QueuePutFreeBeBuf(&pstDrvCtx->stBeBufQueue, pstNode);
        }
    }

    u64PhyAddr = pstDrvCtx->pstUseNode->stBeCfgBuf.u64PhyAddr;
    pVirAddr   = pstDrvCtx->pstUseNode->stBeCfgBuf.pVirAddr;
    u64Size    = pstDrvCtx->pstUseNode->stBeCfgBuf.u64Size;

    osal_flush_dcache_area(pVirAddr, u64PhyAddr, u64Size);

    //osal_printk("ViPipe:%d FUNC : %s LINE : %d pstCurNode : %p\n",ViPipe,__FUNCTION__,__LINE__,pstDrvCtx->pstUseNode);

    ISP_QueuePutBusyBeBuf(&pstDrvCtx->stBeBufQueue, pstDrvCtx->pstUseNode);

    pstDrvCtx->pstUseNode = HI_NULL;

    return;
}

HI_S32 ISP_DRV_RunOnceProcess(VI_PIPE ViPipe)
{
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_SYNC_CFG_S *pstSyncCfg = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_FALSE == pstDrvCtx->bMemInit)
    {
        return HI_ERR_ISP_MEM_NOT_INIT;
    }

    pstSyncCfg = &pstDrvCtx->stSyncCfg;

    ISP_DRV_GetSyncControlnfo(ViPipe, pstSyncCfg);
    ISP_DRV_CalcSyncCfg(pstSyncCfg);
    ISP_DRV_RegConfigIsp(ViPipe, pstDrvCtx);
    ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    if (HI_TRUE == pstDrvCtx->bRunOnceFlag)
    {
        ISP_DRV_BeBufQueuePutBusy(ViPipe);
    }
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->bRunOnceOk = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_YUVRunOnceProcess(VI_PIPE ViPipe)
{
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_FALSE == pstDrvCtx->bMemInit)
    {
        return HI_ERR_ISP_MEM_NOT_INIT;
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    if (HI_TRUE == pstDrvCtx->bYUVRunOnceFlag)
    {
        ISP_DRV_BeBufQueuePutBusy(ViPipe);
    }
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->bYUVRunOnceOk = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_OptRunOnceInfo(VI_PIPE ViPipe, HI_BOOL *pbRunOnceFlag)
{
    HI_BOOL bRunOnceFlag;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pbRunOnceFlag);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    bRunOnceFlag = *pbRunOnceFlag;

    if (HI_TRUE == bRunOnceFlag)
    {
        if (pstDrvCtx->pstUseNode)
        {
            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] isp is running!\r\n", ViPipe);
            return HI_FAILURE;
        }

        pstDrvCtx->pstUseNode = ISP_QueueGetFreeBeBuf(&pstDrvCtx->stBeBufQueue);

        if (HI_NULL == pstDrvCtx->pstUseNode)
        {
            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] get FreeBeBuf is fail!\r\n", ViPipe);

            return HI_FAILURE;
        }
    }

    pstDrvCtx->bRunOnceFlag = bRunOnceFlag;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_YUVRunOnceInfo(VI_PIPE ViPipe, HI_BOOL *pbRunOnceFlag)
{
    HI_BOOL bYUVRunOnceFlag;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pbRunOnceFlag);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    bYUVRunOnceFlag = *pbRunOnceFlag;

    if (HI_TRUE == bYUVRunOnceFlag)
    {
        if (pstDrvCtx->pstUseNode)
        {
            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] isp is running!\r\n", ViPipe);
            return HI_FAILURE;
        }

        pstDrvCtx->pstUseNode = ISP_QueueGetFreeBeBuf(&pstDrvCtx->stBeBufQueue);

        if (HI_NULL == pstDrvCtx->pstUseNode)
        {
            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] get FreeBeBuf is fail!\r\n", ViPipe);

            return HI_FAILURE;
        }
    }

    pstDrvCtx->bYUVRunOnceFlag = bYUVRunOnceFlag;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_VOID ISP_DRV_StitchBeBufCtl(VI_PIPE ViPipe)
{
    HI_S32  i;
    HI_S32  s32Ret;
    VI_PIPE ViPipeS;
    VI_PIPE MainPipeS;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtxS = HI_NULL;
    unsigned long u32Flags;

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    MainPipeS = pstDrvCtx->stStitchAttr.as8StitchBindId[0];

    osal_spin_lock_irqsave(&g_stIspSyncLock[MainPipeS], &u32Flags);

    if (ISP_BE_BUF_STATE_RUNNING != pstDrvCtx->enIspRunningState)
    {
        osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);
        return;
    }

    pstDrvCtx->enIspRunningState = ISP_BE_BUF_STATE_FINISH;

    s32Ret = ISP_DRV_StitchSync(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);
        return;
    }

    for (i = 0; i < pstDrvCtx->stStitchAttr.u8StitchPipeNum; i++)
    {
        ViPipeS = pstDrvCtx->stStitchAttr.as8StitchBindId[i];
        pstDrvCtxS = ISP_DRV_GET_CTX(ViPipeS);

        if (ISP_BE_BUF_STATE_FINISH != pstDrvCtxS->enIspRunningState)
        {
            osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);
            return;
        }
    }

    for (i = 0; i < pstDrvCtx->stStitchAttr.u8StitchPipeNum; i++)
    {
        ViPipeS = pstDrvCtx->stStitchAttr.as8StitchBindId[i];
        pstDrvCtxS = ISP_DRV_GET_CTX(ViPipeS);

        if (HI_TRUE != pstDrvCtxS->stBeBufInfo.bInit)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] BeBuf (bInit != TRUE) !\n", ViPipe);
            osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);
            return;
        }

        if (HI_TRUE != pstDrvCtxS->bRunOnceFlag)
        {
            ISP_DRV_BeBufQueuePutBusy(ViPipeS);
        }
        pstDrvCtxS->enIspRunningState = ISP_BE_BUF_STATE_INIT;
    }

    osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);

    return;
}

HI_S32 ISP_DRV_WriteBeFreeBuf(VI_PIPE ViPipe)
{
    HI_S32 i;
    HI_S32 s32FreeNum;
    ISP_RUNNING_MODE_E enIspRunningMode;
    ISP_BE_BUF_NODE_S *pstNode = HI_NULL;
    ISP_BE_WO_REG_CFG_S *pstBeRegCfgSrc = HI_NULL;
    ISP_BE_WO_REG_CFG_S *pstBeRegCfgDst = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_BEBUF_INIT(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_NULL == pstDrvCtx->pstUseNode)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] pstCurNode is null for init!\r\n", ViPipe);
        return HI_FAILURE;
    }

    pstBeRegCfgSrc   = pstDrvCtx->pstUseNode->stBeCfgBuf.pVirAddr;
    enIspRunningMode = pstDrvCtx->stWorkMode.enIspRunningMode;

    s32FreeNum = ISP_QueueGetFreeNum(&pstDrvCtx->stBeBufQueue);

    for (i = 0; i < s32FreeNum; i++)
    {
        pstNode = ISP_QueueGetFreeBeBuf(&pstDrvCtx->stBeBufQueue);

        if (HI_NULL == pstNode)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] Get QueueGetFreeBeBuf fail!\r\n", ViPipe);
            return HI_FAILURE;;
        }

        pstBeRegCfgDst = (ISP_BE_WO_REG_CFG_S *)pstNode->stBeCfgBuf.pVirAddr;

        if ((ISP_MODE_RUNNING_SIDEBYSIDE == enIspRunningMode) || (ISP_MODE_RUNNING_STRIPING == enIspRunningMode))
        {
            osal_memcpy(pstBeRegCfgDst, pstBeRegCfgSrc, sizeof(ISP_BE_WO_REG_CFG_S));
        }
        else
        {
            osal_memcpy(&pstBeRegCfgDst->stBeRegCfg[0], &pstBeRegCfgSrc->stBeRegCfg[0], sizeof(S_ISPBE_REGS_TYPE));
        }

        ISP_QueuePutFreeBeBuf(&pstDrvCtx->stBeBufQueue, pstNode);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StitchWriteBeBufAll(VI_PIPE ViPipe)
{
    HI_S32 i;
    HI_S32 s32Ret;
    VI_PIPE ViPipeS;
    VI_PIPE MainPipeS;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtxS = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
    MainPipeS = pstDrvCtx->stStitchAttr.as8StitchBindId[0];

    s32Ret = ISP_DRV_WriteBeFreeBuf(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] ISP_DRV_WriteBeFreeBuf fail!\n", ViPipe);
        return HI_FAILURE;
    }

    osal_spin_lock_irqsave(&g_stIspSyncLock[MainPipeS], &u32Flags);

    s32Ret = ISP_DRV_StitchSyncEx(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);

        return HI_SUCCESS;
    }

    for (i = 0; i < pstDrvCtx->stStitchAttr.u8StitchPipeNum; i++)
    {
        ViPipeS = pstDrvCtx->stStitchAttr.as8StitchBindId[i];
        pstDrvCtxS = ISP_DRV_GET_CTX(ViPipeS);

        if (HI_TRUE != pstDrvCtxS->stBeBufInfo.bInit)
        {
            osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] BeBuf (bInit != TRUE) !\n", ViPipe);
            return HI_FAILURE;
        }

        if (HI_NULL == pstDrvCtxS->pstUseNode)
        {
            osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);
            ISP_TRACE(HI_DBG_ERR, "ISP stitch exit without success! pstUseNode is %p!\n", pstDrvCtxS->pstUseNode);
            return HI_FAILURE;
        }

        ISP_QueuePutBusyBeBuf(&pstDrvCtxS->stBeBufQueue, pstDrvCtxS->pstUseNode);
        pstDrvCtxS->pstUseNode = HI_NULL;
        pstDrvCtxS->enIspRunningState = ISP_BE_BUF_STATE_INIT;
    }

    osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);

    return HI_SUCCESS;

}

HI_S32 ISP_DRV_GetBeFreeBuf(VI_PIPE ViPipe, ISP_BE_WO_CFG_BUF_S *pstBeWoCfgBuf)
{
    osal_spinlock_t *pIspSpinLock = HI_NULL;
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_BE_WO_CFG_BUF_S *pstCurNodeBuf = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_BEBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pstBeWoCfgBuf);
    ISP_CHECK_ONLINE_MODE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    pIspSpinLock = ISP_DRV_GetSpinLock(ViPipe);

    osal_spin_lock_irqsave(pIspSpinLock, &u32Flags);

    if (HI_NULL == pstDrvCtx->pstUseNode)
    {
        osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);
        //ISP_TRACE(HI_DBG_ERR, "Pipe[%d] ISP_DRV_GetBeLastBuf fail!\n", ViPipe);
        return HI_FAILURE;
    }

    pstCurNodeBuf = &pstDrvCtx->pstUseNode->stBeCfgBuf;
    osal_memcpy(pstBeWoCfgBuf, pstCurNodeBuf, sizeof(ISP_BE_WO_CFG_BUF_S));

    osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetBeBufFirst(VI_PIPE ViPipe, HI_U64 *pu64PhyAddr)
{
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_BE_BUF_NODE_S *pstNode = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_BEBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pu64PhyAddr);
    ISP_CHECK_ONLINE_MODE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstNode = ISP_QueueGetFreeBeBuf(&pstDrvCtx->stBeBufQueue);

    if (HI_NULL == pstNode)
    {
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] Get FreeBeBuf to user fail!\r\n", ViPipe);
        return HI_FAILURE;
    }

    pstDrvCtx->pstUseNode = pstNode;

    *pu64PhyAddr = pstDrvCtx->pstUseNode->stBeCfgBuf.u64PhyAddr;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);


    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetBeLastBuf(VI_PIPE ViPipe, HI_U64 *pu64PhyAddr)
{
    HI_U8  i;
    ISP_DRV_CTX_S     *pstDrvCtx = HI_NULL;
    ISP_BE_BUF_NODE_S *pstNode   = HI_NULL;
    ISP_BE_WO_REG_CFG_S   *pstBeRegCfgDst = HI_NULL;
    struct osal_list_head *pListTmp = HI_NULL;
    struct osal_list_head *pListNode = HI_NULL;
    unsigned long u32Flags;
    osal_spinlock_t *pIspSpinLock = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_BEBUF_INIT(ViPipe);
    ISP_CHECK_POINTER(pu64PhyAddr);

    ISP_CHECK_ONLINE_MODE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    //ISP_DRV_GetSpinLock(ViPipe, &IspSpinLock);
    pIspSpinLock = ISP_DRV_GetSpinLock(ViPipe);

    osal_spin_lock_irqsave(pIspSpinLock, &u32Flags);

    osal_list_for_each_safe(pListNode, pListTmp, &pstDrvCtx->stBeBufQueue.stBusyList)
    {
        pstNode = osal_list_entry(pListNode, ISP_BE_BUF_NODE_S, list);

        pstNode->s32HoldCnt = 0;

        ISP_QueueDelBusyBeBuf(&pstDrvCtx->stBeBufQueue, pstNode);
        ISP_QueuePutFreeBeBuf(&pstDrvCtx->stBeBufQueue, pstNode);
    }

    if (!pstDrvCtx->pstUseNode)
    {
        pstNode = ISP_QueueGetFreeBeBufTail(&pstDrvCtx->stBeBufQueue);

        if (HI_NULL == pstNode)
        {
            osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] Get LastBeBuf fail!\r\n", ViPipe);
            return HI_FAILURE;
        }

        pstDrvCtx->pstUseNode = pstNode;
    }

    pstBeRegCfgDst = (ISP_BE_WO_REG_CFG_S *)pstDrvCtx->pstUseNode->stBeCfgBuf.pVirAddr;

    for (i = pstDrvCtx->stWorkMode.u8PreBlockNum; i < pstDrvCtx->stWorkMode.u8BlockNum; i++)
    {
        osal_memcpy(&pstBeRegCfgDst->stBeRegCfg[i], &pstBeRegCfgDst->stBeRegCfg[0], sizeof(S_ISPBE_REGS_TYPE));
    }

    *pu64PhyAddr = pstDrvCtx->pstUseNode->stBeCfgBuf.u64PhyAddr;

    osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeBufRunState(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;
    osal_spinlock_t *pIspSpinLock = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_BEBUF_INIT(ViPipe);
    ISP_CHECK_ONLINE_MODE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    //ISP_DRV_GetSpinLock(ViPipe, &IspSpinLock);
    pIspSpinLock = ISP_DRV_GetSpinLock(ViPipe);

    osal_spin_lock_irqsave(pIspSpinLock, &u32Flags);

    if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable)
    {
        if (ISP_BE_BUF_STATE_INIT != pstDrvCtx->enIspRunningState)
        {
            osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);

            ISP_TRACE(HI_DBG_WARN, "Pipe[%d] isp isn't init state!\n", ViPipe);
            return HI_FAILURE;
        }

        pstDrvCtx->enIspRunningState = ISP_BE_BUF_STATE_RUNNING;
    }

    osal_spin_unlock_irqrestore(pIspSpinLock, &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeBufCtl(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_ONLINE_MODE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_TRUE != pstDrvCtx->stStitchAttr.bStitchEnable)
    {
        if (HI_TRUE != pstDrvCtx->stBeBufInfo.bInit)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] BeBuf (bInit != TRUE) !\n", ViPipe);
            return HI_FAILURE;
        }

        osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
        if (HI_TRUE != pstDrvCtx->bRunOnceFlag)
        {
            ISP_DRV_BeBufQueuePutBusy(ViPipe);
        }
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
    }
    else
    {
        ISP_DRV_StitchBeBufCtl(ViPipe);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_AllBeBufInit(VI_PIPE ViPipe)
{
    HI_S32  s32Ret = HI_SUCCESS;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_ONLINE_MODE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_TRUE != pstDrvCtx->stStitchAttr.bStitchEnable)
    {
        if (HI_TRUE != pstDrvCtx->stBeBufInfo.bInit)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] BeBuf (bInit != TRUE) !\n", ViPipe);
            return HI_FAILURE;
        }

        osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

        s32Ret = ISP_DRV_WriteBeFreeBuf(ViPipe);
        if (HI_SUCCESS != s32Ret)
        {
            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] ISP_DRV_WriteBeFreeBuf fail!\n", ViPipe);
            return s32Ret;
        }

        ISP_QueuePutBusyBeBuf(&pstDrvCtx->stBeBufQueue, pstDrvCtx->pstUseNode);
        pstDrvCtx->pstUseNode = HI_NULL;

        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
    }
    else
    {
        s32Ret = ISP_DRV_StitchWriteBeBufAll(ViPipe);
        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe[%d] ISP_DRV_StitchWriteBeBufAll fail!\n", ViPipe);
            return s32Ret;
        }
    }

    return s32Ret;
}

HI_S32 ISP_DRV_SyncCfgSet(VI_PIPE ViPipe, ISP_SYNC_CFG_BUF_NODE_S *pstIspSyncCfgBuf)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_SYNC_CFG_BUF_S  *pstSyncCfgBuf = HI_NULL;
    ISP_SYNC_CFG_BUF_NODE_S  *pstCurNode = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstIspSyncCfgBuf);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstSyncCfgBuf = &pstDrvCtx->stSyncCfg.stSyncCfgBuf;

    if (ISPSyncBufIsFull(pstSyncCfgBuf))
    {
        ISP_TRACE(HI_DBG_ERR, "Pipe[%d] isp sync buffer is full\n", ViPipe);
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_FAILURE;
    }

    if ((pstIspSyncCfgBuf->stSnsRegsInfo.enSnsType >= ISP_SNS_TYPE_BUTT) || \
        (pstIspSyncCfgBuf->stAERegCfg.enFSWDRMode >= ISP_FSWDR_MODE_BUTT) || \
        (pstIspSyncCfgBuf->stSnsRegsInfo.u8Cfg2ValidDelayMax > CFG2VLD_DLY_LIMIT) || \
        (pstIspSyncCfgBuf->stSnsRegsInfo.u8Cfg2ValidDelayMax < 1) ||
        (pstIspSyncCfgBuf->stSnsRegsInfo.stSlvSync.u8DelayFrmNum > CFG2VLD_DLY_LIMIT) || \
        (pstIspSyncCfgBuf->stSnsRegsInfo.u32RegNum > ISP_MAX_SNS_REGS))
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid SnsRegsInfo!\n");
        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        return HI_FAILURE;
    }

    pstCurNode = &pstSyncCfgBuf->stSyncCfgBufNode[pstSyncCfgBuf->u8BufWRFlag];
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    osal_memcpy(pstCurNode, pstIspSyncCfgBuf, sizeof(ISP_SYNC_CFG_BUF_NODE_S));

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    pstSyncCfgBuf->u8BufWRFlag = (pstSyncCfgBuf->u8BufWRFlag + 1) % ISP_SYNC_BUF_NODE_NUM;
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_SetWdrCfg(VI_PIPE ViPipe, ISP_WDR_CFG_S *pstIspWdrCfg)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstIspWdrCfg);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (pstIspWdrCfg->u8WDRMode >= WDR_MODE_BUTT)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid WDR mode %d!\n", pstIspWdrCfg->u8WDRMode);
        return HI_FAILURE;
    }

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    osal_memcpy(&pstDrvCtx->stWDRCfg, pstIspWdrCfg, sizeof(ISP_WDR_CFG_S));
    ISP_DRV_SwitchMode(ViPipe, pstDrvCtx);
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetStitchAttr(VI_PIPE ViPipe, VI_STITCH_ATTR_S *pstStitchAttr)
{
    HI_U8   i;
    HI_S32  s32Ret;
    ISP_DRV_CTX_S      *pstDrvCtx = HI_NULL;
    VI_STITCH_ATTR_S   stStitchAttr = {0};

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstStitchAttr);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (!CKFN_VI_GetPipeStitchAttr())
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] CKFN_VI_GetPipeWDRAttr is null\n", ViPipe);
        return HI_FAILURE;
    }

    s32Ret = CALL_VI_GetPipeStitchAttr(ViPipe, &stStitchAttr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] CALL_VI_GetPipeStitchAttr failed 0x%x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    if (stStitchAttr.bStitchEnable)
    {
        if ((stStitchAttr.u8StitchPipeNum < 1 ) || (stStitchAttr.u8StitchPipeNum > ISP_MAX_PIPE_NUM))
        {
            ISP_TRACE(HI_DBG_ERR, "pipe[%d] err stitch num %d\n", ViPipe, stStitchAttr.u8StitchPipeNum);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        for (i = 0; i < stStitchAttr.u8StitchPipeNum; i++)
        {
            ISP_CHECK_PIPE(stStitchAttr.as8StitchBindId[i]);
        }
    }
    // TODO: Don't support stitch different wdr mode(check)!

    osal_memcpy(&pstDrvCtx->stStitchAttr, &stStitchAttr, sizeof(VI_STITCH_ATTR_S));
    osal_memcpy(pstStitchAttr, &stStitchAttr, sizeof(VI_STITCH_ATTR_S));

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetPipeSize(VI_PIPE ViPipe, SIZE_S *pstPipeSize)
{
    HI_S32  s32Ret;
    SIZE_S  stPipeSize = {0};

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstPipeSize);
    ISP_CHECK_VIR_PIPE(ViPipe);
    if (!CKFN_VI_GetPipeBindDevSize())
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] CKFN_VI_GetPipeBindDevSize is null\n", ViPipe);
        return HI_FAILURE;
    }

    s32Ret = CALL_VI_GetPipeBindDevSize(ViPipe, &stPipeSize);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] CALL_VI_GetPipeBindDevSize failed 0x%x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    if ((stPipeSize.u32Width < RES_WIDTH_MIN) ||
        (stPipeSize.u32Width > SENSOR_RES_WIDTH_MAX) ||
        (stPipeSize.u32Height < RES_HEIGHT_MIN) ||
        (stPipeSize.u32Height > SENSOR_RES_HEIGHT_MAX))
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d]: Image Width should between [%d, %d], Height should between[%d, %d]\n", ViPipe, \
                  RES_WIDTH_MIN, SENSOR_RES_WIDTH_MAX, RES_HEIGHT_MIN, SENSOR_RES_HEIGHT_MAX);
        return HI_FAILURE;
    }

    osal_memcpy(pstPipeSize, &stPipeSize, sizeof(SIZE_S));

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetHdrAttr(VI_PIPE ViPipe, VI_PIPE_HDR_ATTR_S *pstHDRAttr)
{
    HI_S32  s32Ret;
    VI_PIPE_HDR_ATTR_S stHDRAttr = {0};

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstHDRAttr);

    if (!CKFN_VI_GetPipeHDRAttr())
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] CKFN_VI_GetPipeHDRAttr is null\n", ViPipe);
        return HI_FAILURE;
    }

    s32Ret = CALL_VI_GetPipeHDRAttr(ViPipe, &stHDRAttr);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] CALL_VI_GetPipeHDRAttr failed 0x%x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    osal_memcpy(pstHDRAttr, &stHDRAttr, sizeof(VI_PIPE_HDR_ATTR_S));

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetWdrAttr(VI_PIPE ViPipe, VI_PIPE_WDR_ATTR_S *pstWDRAttr)
{
    HI_U8   i;
    HI_U32  u32Num;
    HI_S32  s32Ret;
    VI_PIPE_WDR_ATTR_S stWDRAttr;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstWDRAttr);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (!CKFN_VI_GetPipeWDRAttr())
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] CKFN_VI_GetPipeWDRAttr is null\n", ViPipe);
        return HI_FAILURE;
    }

    s32Ret = CALL_VI_GetPipeWDRAttr(ViPipe, &stWDRAttr);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] CALL_VI_GetPipeWDRAttr failed 0x%x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    /* Not WDR mode,BindPipe attr update */
    if ((!IS_FS_WDR_MODE(stWDRAttr.enWDRMode)) &&
        (1 != stWDRAttr.stDevBindPipe.u32Num))
    {
        stWDRAttr.stDevBindPipe.u32Num    = 1;
        stWDRAttr.stDevBindPipe.PipeId[0] = ViPipe;
    }

    u32Num = stWDRAttr.stDevBindPipe.u32Num;
    if ((u32Num < 1) || (u32Num > ISP_WDR_CHN_MAX))
    {
        ISP_TRACE(HI_DBG_ERR, "Err wdr bind num %d!\n", u32Num);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    for (i = 0; i < u32Num; i++)
    {
        ISP_CHECK_PIPE(stWDRAttr.stDevBindPipe.PipeId[i]);
    }
    if ((stWDRAttr.enWDRMode > WDR_MODE_BUTT ))
    {
        ISP_TRACE(HI_DBG_ERR, "Err wdr mode %d!\n", stWDRAttr.enWDRMode);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    osal_memcpy(&pstDrvCtx->stWDRAttr, &stWDRAttr, sizeof(VI_PIPE_WDR_ATTR_S));
    osal_memcpy(pstWDRAttr, &stWDRAttr, sizeof(VI_PIPE_WDR_ATTR_S));

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ChnSelectCfg(VI_PIPE ViPipe, HI_U32 u32ChnSel)
{
    HI_U32 i;
    HI_U32 s32Ret = HI_SUCCESS;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    for (i = 0; i < ISP_STRIPING_MAX_NUM; i++)
    {
        pstDrvCtx->astChnSelAttr[i].u32ChannelSel = u32ChnSel;
    }

    if ((IS_ONLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)) ||
        (IS_SIDEBYSIDE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)))
    {
        s32Ret = ISP_DRV_ChnSelectWrite(ViPipe, u32ChnSel);

        if (HI_SUCCESS != s32Ret)
        {
            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
            ISP_TRACE(HI_DBG_ERR, "isp[%d] ChnSelect Write err!\n", ViPipe);
            return s32Ret;
        }
    }

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return s32Ret;
}

HI_S32 ISP_DRV_SetIspRunState(VI_PIPE ViPipe, HI_U64 *pu64HandSignal)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pu64HandSignal);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (ISP_INIT_HAND_SIGNAL == *pu64HandSignal)
    {
        pstDrvCtx->bIspRunFlag = HI_TRUE;
    }
    else if (ISP_EXIT_HAND_SIGNAL == *pu64HandSignal)
    {
        pstDrvCtx->bIspRunFlag = HI_FALSE;
        ISP_DRV_ResetFeCfg(ViPipe);
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set isp run state failed!\n", ViPipe);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ResetCtx(VI_PIPE ViPipe)
{
    HI_U8 i;
    VI_PIPE ViPipeS = 0;
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx  = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtxS = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->bMemInit     = HI_FALSE;
    pstDrvCtx->bIspInit     = HI_FALSE;
    pstDrvCtx->bRunOnceOk   = HI_FALSE;
    pstDrvCtx->bRunOnceFlag = HI_FALSE;
    pstDrvCtx->bYUVRunOnceOk   = HI_FALSE;
    pstDrvCtx->bYUVRunOnceFlag = HI_FALSE;

    if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable)
    {
        if (HI_TRUE == pstDrvCtx->stStitchAttr.bMainPipe)
        {
            for (i = 0; i < pstDrvCtx->stStitchAttr.u8StitchPipeNum; i++)
            {
                ViPipeS = pstDrvCtx->stStitchAttr.as8StitchBindId[i];
                ISP_CHECK_PIPE(ViPipeS);
                pstDrvCtxS = ISP_DRV_GET_CTX(ViPipeS);
                pstDrvCtxS->bStitchSync  = HI_FALSE;
            }
        }
    }

    pstDrvCtx->stStitchAttr.bStitchEnable     = HI_FALSE;

    for (i = 0; i < ISP_STRIPING_MAX_NUM; i++)
    {
        pstDrvCtx->astChnSelAttr[i].u32ChannelSel = 0;
    }

    pstDrvCtx->stSnapAttr.enSnapType          = SNAP_TYPE_NORMAL;
    pstDrvCtx->stSnapAttr.s32PicturePipeId    = -1;
    pstDrvCtx->stSnapAttr.s32PreviewPipeId    = -1;
    pstDrvCtx->stSnapAttr.bLoadCCM            = HI_TRUE;
    pstDrvCtx->stSnapAttr.stProParam.enOperationMode = OPERATION_MODE_AUTO;

    pstDrvCtx->bEdge    = HI_FALSE;
    pstDrvCtx->bVdStart = HI_FALSE;
    pstDrvCtx->bVdEnd   = HI_FALSE;
    pstDrvCtx->bVdBeEnd = HI_FALSE;

    g_IntBothalf = HI_FALSE;
    g_UseBothalf = HI_FALSE;
    g_QuickStart = HI_FALSE;


    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_SetPubAttrInfo(VI_PIPE ViPipe, ISP_PUB_ATTR_S *pstPubAttr)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstPubAttr);

    if ((pstPubAttr->stWndRect.u32Width < RES_WIDTH_MIN) || (pstPubAttr->stWndRect.u32Width > g_au16ResMaxWidth[ViPipe]) || \
        (0 != pstPubAttr->stWndRect.u32Width % 4))
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid Image Width:%d!\n", pstPubAttr->stWndRect.u32Width);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pstPubAttr->stWndRect.u32Height < RES_HEIGHT_MIN) || (pstPubAttr->stWndRect.u32Height > g_au16ResMaxHeight[ViPipe]) || \
        (0 != pstPubAttr->stWndRect.u32Height % 4))
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid Image Height:%d!\n", pstPubAttr->stWndRect.u32Height);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pstPubAttr->stSnsSize.u32Width < RES_WIDTH_MIN) || (pstPubAttr->stSnsSize.u32Width > SENSOR_RES_WIDTH_MAX))
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid Sensor Image Width:%d!\n", pstPubAttr->stSnsSize.u32Width);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pstPubAttr->stSnsSize.u32Height < RES_HEIGHT_MIN) || (pstPubAttr->stSnsSize.u32Height > SENSOR_RES_HEIGHT_MAX))
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid Sensor Image Height:%d!\n", pstPubAttr->stSnsSize.u32Height);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pstPubAttr->stWndRect.s32X < 0) || (pstPubAttr->stWndRect.s32X > g_au16ResMaxWidth[ViPipe] - RES_WIDTH_MIN))
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid Image X:%d!\n", pstPubAttr->stWndRect.s32X);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pstPubAttr->stWndRect.s32Y < 0) || (pstPubAttr->stWndRect.s32Y > g_au16ResMaxHeight[ViPipe] - RES_HEIGHT_MIN))
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid Image Y:%d!\n", pstPubAttr->stWndRect.s32Y);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pstPubAttr->enBayer >= BAYER_BUTT)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid Bayer Pattern:%d!\n", pstPubAttr->enBayer);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pstPubAttr->enWDRMode >= WDR_MODE_BUTT)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid WDR mode %d!\n", pstPubAttr->enWDRMode);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    osal_memcpy(&pstDrvCtx->stProcPubInfo, pstPubAttr, sizeof(ISP_PUB_ATTR_S));
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

static long ISP_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    VI_PIPE ViPipe = ISP_GET_DEV(private_data);

    switch (cmd)
    {
        case ISP_DEV_SET_FD :
        {
            ISP_CHECK_POINTER(arg);
            *((HI_U32 *)(private_data)) = *(HI_U32 *)(HI_UINTPTR_T)arg;

            return HI_SUCCESS;
        }

        case ISP_GET_VERSION :
        {
            ISP_VERSION_S *pVersion = HI_NULL;
            pVersion = (ISP_VERSION_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetVersion(pVersion);
        }

        case ISP_GET_FRAME_EDGE :
        {
            HI_U32 *pu32Status = HI_NULL;
            pu32Status = (HI_U32 *)(HI_UINTPTR_T)arg;

            return ISP_GetFrameEdge(ViPipe, pu32Status);
        }

        case ISP_GET_VD_TIMEOUT:
        {
            ISP_VD_TIMEOUT_S stIspVdTimeOut;
            ISP_VD_TIMEOUT_S *pstVdTimeOut = HI_NULL;

            ISP_CHECK_POINTER(arg);

            pstVdTimeOut = (ISP_VD_TIMEOUT_S *)(HI_UINTPTR_T)arg;
            osal_memcpy(&stIspVdTimeOut, pstVdTimeOut, sizeof(ISP_VD_TIMEOUT_S));
            ISP_GetVdStartTimeOut(ViPipe, stIspVdTimeOut.u32MilliSec, &stIspVdTimeOut.u32IntStatus);
            osal_memcpy(pstVdTimeOut, &stIspVdTimeOut, sizeof(ISP_VD_TIMEOUT_S));

            return HI_SUCCESS;
        }

        case ISP_GET_VD_END_TIMEOUT:
        {
            ISP_VD_TIMEOUT_S stIspVdTimeOut;
            ISP_VD_TIMEOUT_S *pstVdTimeOut = HI_NULL;

            ISP_CHECK_POINTER(arg);

            pstVdTimeOut = (ISP_VD_TIMEOUT_S *)(HI_UINTPTR_T)arg;
            osal_memcpy(&stIspVdTimeOut, pstVdTimeOut, sizeof(ISP_VD_TIMEOUT_S));
            ISP_GetVdEndTimeOut(ViPipe, stIspVdTimeOut.u32MilliSec, &stIspVdTimeOut.u32IntStatus);
            osal_memcpy(pstVdTimeOut, &stIspVdTimeOut, sizeof(ISP_VD_TIMEOUT_S));

            return HI_SUCCESS;
        }

        case ISP_GET_VD_BEEND_TIMEOUT:
        {
            ISP_VD_TIMEOUT_S stIspVdTimeOut;
            ISP_VD_TIMEOUT_S *pstVdTimeOut = HI_NULL;

            ISP_CHECK_POINTER(arg);

            pstVdTimeOut = (ISP_VD_TIMEOUT_S *)(HI_UINTPTR_T)arg;
            osal_memcpy(&stIspVdTimeOut, pstVdTimeOut, sizeof(ISP_VD_TIMEOUT_S));
            ISP_GetVdBeEndTimeOut(ViPipe, stIspVdTimeOut.u32MilliSec, &stIspVdTimeOut.u32IntStatus);
            osal_memcpy(pstVdTimeOut, &stIspVdTimeOut, sizeof(ISP_VD_TIMEOUT_S));

            return HI_SUCCESS;
        }

        case ISP_UPDATE_INFO_SET:
        {
            ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
            ISP_DCF_UPDATE_INFO_S *pstIspUpdateInfo = HI_NULL;
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
            pstIspUpdateInfo = (ISP_DCF_UPDATE_INFO_S *)(HI_UINTPTR_T)arg;
            osal_memcpy(&pstDrvCtx->stUpdateInfo, pstIspUpdateInfo, sizeof(ISP_DCF_UPDATE_INFO_S));
            return HI_SUCCESS;
        }

        case ISP_FRAME_INFO_SET:
        {
            ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
            ISP_FRAME_INFO_S *pstIspFrameInfo = HI_NULL;
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
            pstIspFrameInfo = (ISP_FRAME_INFO_S *)(HI_UINTPTR_T)arg;
            osal_memcpy(&pstDrvCtx->stFrameInfo, pstIspFrameInfo, sizeof(ISP_FRAME_INFO_S));
            return HI_SUCCESS;
        }

        case ISP_FRAME_INFO_GET:
        {
            ISP_FRAME_INFO_S *pstIspFrameInfo = HI_NULL;
            pstIspFrameInfo = (ISP_FRAME_INFO_S *)(HI_UINTPTR_T)arg;

            return ISP_GetFrameInfo(ViPipe, pstIspFrameInfo);
        }

        case ISP_DNG_INFO_SET:
        {
            DNG_IMAGE_DYNAMIC_INFO_S *pstDngImgDynInfo = HI_NULL;
            pstDngImgDynInfo = (DNG_IMAGE_DYNAMIC_INFO_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_SetDngInfo(ViPipe, pstDngImgDynInfo);
        }

        case ISP_DNG_INFO_GET:
        {
            DNG_IMAGE_STATIC_INFO_S *pstDngStaticInfo = HI_NULL;
            pstDngStaticInfo = (DNG_IMAGE_STATIC_INFO_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetDngInfo(ViPipe, pstDngStaticInfo);
        }

        case ISP_SET_INT_ENABLE :
        {
            HI_BOOL bEn = HI_FALSE;
            ISP_CHECK_POINTER(arg);
            bEn = *(HI_BOOL *)(HI_UINTPTR_T)arg;

            return ISP_DRV_SetIntEnable(ViPipe, bEn);
        }

        case ISP_TRANS_BUF_INIT :
        {
            ISP_TRANS_INFO_BUF_S *pstTransInfo = HI_NULL;
            pstTransInfo = (ISP_TRANS_INFO_BUF_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_TransInfoBufInit(ViPipe, pstTransInfo);
        }

        case ISP_TRANS_BUF_EXIT :
        {
            return ISP_DRV_TransInfoBufExit(ViPipe);
        }

        case ISP_PRO_BUF_INIT :
        {
            ISP_PRO_INFO_BUF_S *pstProInfo = HI_NULL;
            pstProInfo = (ISP_PRO_INFO_BUF_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_ProInfoBufInit(ViPipe, pstProInfo);
        }

        case ISP_PRO_BUF_EXIT :
        {
            return ISP_DRV_ProInfoBufExit(ViPipe);
        }

        case ISP_STAT_BUF_INIT :
        {
            HI_U64 *pu64StatPhyAddr = HI_NULL;
            pu64StatPhyAddr = (HI_U64 *)(HI_UINTPTR_T)arg;

            return ISP_DRV_StatBufInit(ViPipe, pu64StatPhyAddr);
        }

        case ISP_STAT_BUF_EXIT :
        {
            return ISP_DRV_StatBufExit(ViPipe);
        }

        case ISP_STAT_BUF_GET :
        {
            ISP_STAT_INFO_S *pstStat = HI_NULL;
            pstStat = (ISP_STAT_INFO_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetUserStatBuf(ViPipe, pstStat);
        }

        case ISP_STAT_BUF_PUT :
        {
            ISP_STAT_INFO_S *pstStatInfo = HI_NULL;
            pstStatInfo = (ISP_STAT_INFO_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_StatBufUserPut(ViPipe, pstStatInfo);
        }

        case ISP_STAT_ACT_GET:
        {
            ISP_STAT_INFO_S *pstIspStatInfo = HI_NULL;
            pstIspStatInfo = (ISP_STAT_INFO_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetStatInfoActive(ViPipe, pstIspStatInfo);
        }

        case ISP_REG_CFG_SET:
        {
            ISP_KERNEL_REG_CFG_S *pstRegKernelCfg = HI_NULL;
            pstRegKernelCfg = (ISP_KERNEL_REG_CFG_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_SetRegKernelCfgs(ViPipe, pstRegKernelCfg);
        }

        case ISP_BE_CFG_BUF_INIT:
        {
            HI_U64 *pu64BePhyAddr = HI_NULL;
            pu64BePhyAddr = (HI_U64 *)(HI_UINTPTR_T)arg;

            return ISP_DRV_BeBufInit(ViPipe, pu64BePhyAddr);
        }

        case ISP_GET_BE_BUF_FIRST:
        {
            HI_U64 *pu64PhyAddr = HI_NULL;
            pu64PhyAddr = (HI_U64 *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetBeBufFirst(ViPipe, pu64PhyAddr);
        }

        case ISP_BE_FREE_BUF_GET:
        {
            ISP_BE_WO_CFG_BUF_S *pstBeWoCfgBuf = HI_NULL;
            pstBeWoCfgBuf = (ISP_BE_WO_CFG_BUF_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetBeFreeBuf(ViPipe, pstBeWoCfgBuf);
        }

        case ISP_BE_LAST_BUF_GET:
        {
            HI_U64 *pu64PhyAddr = HI_NULL;
            pu64PhyAddr = (HI_U64 *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetBeLastBuf(ViPipe, pu64PhyAddr);
        }

        case ISP_BE_CFG_BUF_EXIT:
        {
            return ISP_DRV_BeBufExit(ViPipe);
        }

        case ISP_BE_CFG_BUF_RUNNING:
        {
            return ISP_DRV_BeBufRunState(ViPipe);
        }

        case ISP_BE_CFG_BUF_CTL:
        {
            return ISP_DRV_BeBufCtl(ViPipe);
        }

        case ISP_BE_All_BUF_INIT:
        {
            return ISP_DRV_AllBeBufInit(ViPipe);
        }

        case ISP_SYNC_CFG_SET:
        {
            ISP_SYNC_CFG_BUF_NODE_S *pstIspSyncCfgBuf = HI_NULL;
            pstIspSyncCfgBuf = (ISP_SYNC_CFG_BUF_NODE_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_SyncCfgSet(ViPipe, pstIspSyncCfgBuf);
        }

        case ISP_WDR_CFG_SET:
        {
            ISP_WDR_CFG_S *pstIspWdrCfg = HI_NULL;
            pstIspWdrCfg = (ISP_WDR_CFG_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_SetWdrCfg(ViPipe, pstIspWdrCfg);
        }

        case ISP_BE_SYNC_PARAM_INIT:
        {
            ISP_DRV_CTX_S      *pstDrvCtx      = HI_NULL;
            ISP_BE_SYNC_PARA_S *pstBeSyncParam = HI_NULL;
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
            pstBeSyncParam = (ISP_BE_SYNC_PARA_S *)(HI_UINTPTR_T)arg;
            osal_memcpy(&pstDrvCtx->stIspBeSyncPara, pstBeSyncParam, sizeof(ISP_BE_SYNC_PARA_S));
            return HI_SUCCESS;
        }

        case ISP_RES_SWITCH_SET:
        {
            ISP_CHECK_PIPE(ViPipe);
            ISP_DRV_BeBufSwitchState(ViPipe);
            return HI_SUCCESS;
        }

        case ISP_CHN_SELECT_CFG:
        {
            HI_U32 u32ChannelSel = 0;
            ISP_CHECK_POINTER(arg);
            u32ChannelSel = *(HI_U32 *)(HI_UINTPTR_T)arg;

            return ISP_DRV_ChnSelectCfg(ViPipe, u32ChannelSel);
        }

        case ISP_PROC_INIT:
        {
            ISP_PROC_MEM_S *pstIspProcMem = HI_NULL;
            pstIspProcMem = (ISP_PROC_MEM_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_ProcInit(ViPipe, pstIspProcMem);
        }

        case ISP_PROC_WRITE_ING:
        {
            ISP_CHECK_PIPE(ViPipe);

            if (osal_down_interruptible(&g_astIspDrvCtx[ViPipe].stProcSem))
            {
                return -ERESTARTSYS;
            }
            return HI_SUCCESS;
        }

        case ISP_PROC_WRITE_OK:
        {
            ISP_CHECK_PIPE(ViPipe);

            osal_up(&g_astIspDrvCtx[ViPipe].stProcSem);
            return HI_SUCCESS;
        }

        case ISP_PROC_EXIT:
        {
            return ISP_DRV_ProcExit(ViPipe);
        }

        case ISP_PROC_PARAM_GET:
        {
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            *(HI_U32 *)(HI_UINTPTR_T)arg = g_ProcParam[ViPipe];

            return HI_SUCCESS;
        }

        case ISP_MEM_INFO_SET:
        {
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            ISP_CHECK_BOOL(*(HI_BOOL *)(HI_UINTPTR_T)arg);
            g_astIspDrvCtx[ViPipe].bMemInit = *(HI_BOOL *)(HI_UINTPTR_T)arg;

            return HI_SUCCESS;
        }

        case ISP_MEM_INFO_GET:
        {
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            *(HI_BOOL *)(HI_UINTPTR_T)arg = g_astIspDrvCtx[ViPipe].bMemInit;

            return HI_SUCCESS;
        }

        case ISP_P2EN_INFO_GET:
        {
            HI_BOOL *pbP2En = HI_NULL;

            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            pbP2En = (HI_BOOL *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetP2EnInfo(ViPipe, pbP2En);
        }

        case ISP_SYNC_INIT_SET:
        {
            ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            ISP_CHECK_BOOL(*(HI_BOOL *)(HI_UINTPTR_T)arg);
            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

            pstDrvCtx->bStitchSync = HI_FALSE;
            if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable)
            {
                pstDrvCtx->bStitchSync = *(HI_BOOL *)(HI_UINTPTR_T)arg;
            }

            return HI_SUCCESS;
        }

        case ISP_INIT_INFO_SET:
        {
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            ISP_CHECK_BOOL(*(HI_BOOL *)(HI_UINTPTR_T)arg);
            g_astIspDrvCtx[ViPipe].bIspInit = *(HI_BOOL *)(HI_UINTPTR_T)arg;

            return HI_SUCCESS;
        }

        case ISP_RESET_CTX:
        {
            return ISP_DRV_ResetCtx(ViPipe);
        }

        case ISP_RUN_STATE_SET:
        {
            HI_U64 *pu64HandSignal = HI_NULL;
            pu64HandSignal = (HI_U64 *)(HI_UINTPTR_T)arg;

            return ISP_DRV_SetIspRunState(ViPipe, pu64HandSignal);
        }

        case ISP_CONFIG_INFO_SET:
        {
            ISP_CONFIG_INFO_S *pstIspConfigInfo = HI_NULL;
            pstIspConfigInfo = (ISP_CONFIG_INFO_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_SetConfigInfo(ViPipe, pstIspConfigInfo);
        }

        case ISP_SNAP_INFO_GET:
        {
            ISP_SNAP_INFO_S *pstIspSnapInfo = HI_NULL;
            pstIspSnapInfo = (ISP_SNAP_INFO_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetSnapInfo(ViPipe, pstIspSnapInfo);
        }

        case ISP_PRO_TRIGGER_GET:
        {
            ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
            *(HI_BOOL *)(HI_UINTPTR_T)arg = pstDrvCtx->bProEnable;

            if (HI_TRUE == pstDrvCtx->bProEnable)
            {
                pstDrvCtx->bProEnable = HI_FALSE;
                pstDrvCtx->u32ProTrigFlag = 1;
            }

            return HI_SUCCESS;
        }

        case ISP_SNAP_ATTR_GET:
        {
            ISP_SNAP_ATTR_S *pstIspSnapAttr = HI_NULL;
            ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            pstIspSnapAttr = (ISP_SNAP_ATTR_S *)(HI_UINTPTR_T)arg;
            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
            osal_memcpy(pstIspSnapAttr, &pstDrvCtx->stSnapAttr, sizeof(ISP_SNAP_ATTR_S));

            return HI_SUCCESS;
        }

        case ISP_UPDATE_POS_GET:
        {
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            *(HI_U32 *)(HI_UINTPTR_T)arg = g_UpdatePos[ViPipe];

            return HI_SUCCESS;
        }

        case ISP_FRAME_CNT_GET:
        {
            ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
            *(HI_U32 *)(HI_UINTPTR_T)arg = pstDrvCtx->u32FrameCnt;

            return HI_SUCCESS;
        }
        case ISP_PUB_ATTR_INFO:
        {
            ISP_PUB_ATTR_S *pstIspPubAttr = HI_NULL;
            pstIspPubAttr = (ISP_PUB_ATTR_S *)(HI_UINTPTR_T)arg;

            return ISP_SetPubAttrInfo(ViPipe, pstIspPubAttr);
        }

        case ISP_PWM_NUM_GET:
        {
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            *(HI_U32 *)(HI_UINTPTR_T)arg = g_PwmNumber[ViPipe];

            return HI_SUCCESS;
        }

        case ISP_SET_MOD_PARAM:
        {
            ISP_MOD_PARAM_S *pstModeParam = HI_NULL;
            pstModeParam = (ISP_MOD_PARAM_S *)(HI_UINTPTR_T)arg;

            return ISP_SetModParam(pstModeParam);
        }

        case ISP_GET_MOD_PARAM:
        {
            ISP_MOD_PARAM_S *pstModeParam = HI_NULL;
            pstModeParam = (ISP_MOD_PARAM_S *)(HI_UINTPTR_T)arg;

            return ISP_GetModParam(pstModeParam);
        }

        case ISP_SET_CTRL_PARAM:
        {
            ISP_CTRL_PARAM_S *pstCtrlParam = HI_NULL;
            pstCtrlParam = (ISP_CTRL_PARAM_S *)(HI_UINTPTR_T)arg;

            return ISP_SetCtrlParam(ViPipe, pstCtrlParam);
        }

        case ISP_GET_CTRL_PARAM:
        {
            ISP_CTRL_PARAM_S *pstCtrlParam = HI_NULL;
            pstCtrlParam = (ISP_CTRL_PARAM_S *)(HI_UINTPTR_T)arg;

            return ISP_GetCtrlParam(ViPipe, pstCtrlParam);
        }

        case ISP_WORK_MODE_INIT:
        {
            ISP_BLOCK_ATTR_S *pstIspBlockAttr = HI_NULL;
            pstIspBlockAttr = (ISP_BLOCK_ATTR_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_WorkModeInit(ViPipe, pstIspBlockAttr);
        }

        case ISP_WORK_MODE_EXIT:
        {
            return ISP_DRV_WorkModeExit(ViPipe);
        }

        case ISP_WORK_MODE_GET:
        {
            ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
            ISP_WORKING_MODE_S *pstIspWorkMode = HI_NULL;
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            pstIspWorkMode = (ISP_WORKING_MODE_S *)(HI_UINTPTR_T)arg;
            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
            osal_memcpy(pstIspWorkMode, &pstDrvCtx->stWorkMode, sizeof(ISP_WORKING_MODE_S));

            return HI_SUCCESS;
        }

        case ISP_PRE_BLK_NUM_UPDATE:
        {
            ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
            pstDrvCtx->stWorkMode.u8PreBlockNum = *(HI_U8 *)(HI_UINTPTR_T)arg;

            return HI_SUCCESS;
        }

        case ISP_GET_PIPE_SIZE:
        {
            SIZE_S *pstPipeSize = HI_NULL;
            pstPipeSize = (SIZE_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetPipeSize(ViPipe, pstPipeSize);
        }

        case ISP_GET_HDR_ATTR:
        {
            VI_PIPE_HDR_ATTR_S *pstHDRAttr = HI_NULL;
            pstHDRAttr = (VI_PIPE_HDR_ATTR_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetHdrAttr(ViPipe, pstHDRAttr);
        }

        case ISP_GET_WDR_ATTR:
        {
            VI_PIPE_WDR_ATTR_S *pstWDRAttr = HI_NULL;
            pstWDRAttr = (VI_PIPE_WDR_ATTR_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetWdrAttr(ViPipe, pstWDRAttr);
        }

        case ISP_GET_STITCH_ATTR:
        {
            VI_STITCH_ATTR_S *pstStitchAttr = HI_NULL;
            pstStitchAttr = (VI_STITCH_ATTR_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetStitchAttr(ViPipe, pstStitchAttr);
        }

        case ISP_GET_FUSION_ATTR:
        {
            VI_FUSION_GROUP_S *pstFusionAttr = HI_NULL;
            pstFusionAttr = (VI_FUSION_GROUP_S *)(HI_UINTPTR_T)arg;

            return ISP_DRV_GetFusionAttr(ViPipe, pstFusionAttr);
        }
        case ISP_SYNC_STITCH_PARAM_INIT:
        {
            return ISP_DRV_StitchSyncCtrlInit(ViPipe);
        }

        case ISP_CLUT_BUF_INIT:
        {
            return ISP_DRV_ClutBufInit(ViPipe);
        }

        case ISP_CLUT_BUF_EXIT:
        {
            return ISP_DRV_ClutBufExit(ViPipe);
        }

        case ISP_CLUT_BUF_GET:
        {
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);
            if (HI_FALSE == g_astIspDrvCtx[ViPipe].stClutBufAttr.bInit)
            {
                return HI_ERR_ISP_NOMEM;
            }
            *(HI_U64 *)(HI_UINTPTR_T)arg = g_astIspDrvCtx[ViPipe].stClutBufAttr.stClutBuf.u64PhyAddr;

            return HI_SUCCESS;
        }

        case ISP_OPT_RUNONCE_INFO:
        {
            return ISP_DRV_OptRunOnceInfo(ViPipe, (HI_BOOL *)(HI_UINTPTR_T)arg);
        }

        case ISP_YUV_RUNONCE_INFO:
        {
            return ISP_DRV_YUVRunOnceInfo(ViPipe, (HI_BOOL *)(HI_UINTPTR_T)arg);
        }

        case ISP_KERNEL_RUNONCE:
        {
            return ISP_DRV_RunOnceProcess(ViPipe);
        }
        case ISP_KERNEL_YUV_RUNONCE:
        {
            return ISP_DRV_YUVRunOnceProcess(ViPipe);
        }
        case ISP_SET_PROCALCDONE:
        {
            ISP_CHECK_PIPE(ViPipe);
            g_astIspDrvCtx[ViPipe].bProStart = HI_TRUE;

            return HI_SUCCESS;
        }

        case ISP_STT_BUF_INIT:
        {
            return ISP_DRV_SttBufInit(ViPipe);
        }

        case ISP_STT_ADDR_INIT:
        {
            ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
            ISP_CHECK_PIPE(ViPipe);

            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

            if (HI_FALSE == pstDrvCtx->stStitchAttr.bStitchEnable)
            {
                return ISP_DRV_FeSttAddrInit(ViPipe);
            }
            else
            {
                return ISP_DRV_FeStitchSttAddrInit(ViPipe);
            }
        }

        case ISP_BE_LUT_STT_BUF_GET:
        {
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);

            if (HI_FALSE == g_astIspDrvCtx[ViPipe].stBeLut2SttAttr.bInit)
            {
                return HI_ERR_ISP_NOMEM;
            }
            *(HI_U64 *)(HI_UINTPTR_T)arg = g_astIspDrvCtx[ViPipe].stBeLut2SttAttr.astBeLutSttBuf[0].astLutSttBuf[0].u64PhyAddr;

            return HI_SUCCESS;
        }

        case ISP_STT_BUF_EXIT:
        {
            return ISP_DRV_SttBufExit(ViPipe);
        }

        case ISP_SPECAWB_BUF_INIT:
        {
            return ISP_DRV_SpecAwbBufInit(ViPipe);
        }

        case ISP_SPECAWB_BUF_EXIT:
        {
            return ISP_DRV_SpecAwbBufExit(ViPipe);
        }

        case ISP_SPECAWB_BUF_GET:
        {
#ifdef CONFIG_HI_ISP_SPECAWB_SUPPORT
            ISP_CHECK_PIPE(ViPipe);
            ISP_CHECK_POINTER(arg);
            if (HI_FALSE == g_astIspDrvCtx[ViPipe].stSpecAwbBufAttr.bInit)
            {
                return HI_ERR_ISP_NOMEM;
            }
            *(HI_U64 *)(HI_UINTPTR_T)arg = g_astIspDrvCtx[ViPipe].stSpecAwbBufAttr.stSpecAwbBuf.u64PhyAddr;

            return HI_SUCCESS;
#else
            return HI_ERR_ISP_NOT_SUPPORT;
#endif
        }
        case ISP_SET_RGBIR_FORMAT:
        {
            ISP_CHECK_PIPE(ViPipe);
            g_astIspDrvCtx[ViPipe].enIspRgbirFormat = *(HI_U32 *)(HI_UINTPTR_T)arg;
            return HI_SUCCESS;
        }
        default:
        {
            return VREG_DRV_ioctl(cmd, arg, private_data);
        }

    }

    return 0;
}

#ifdef CONFIG_COMPAT
static long ISP_compat_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    switch (cmd)
    {
        case ISP_DEV_SET_FD :
        {
            break;
        }

        case ISP_GET_VERSION :
        {
            break;
        }

        case ISP_GET_FRAME_EDGE :
        {
            break;
        }

        case ISP_GET_VD_TIMEOUT:
        {
            break;
        }

        case ISP_GET_VD_END_TIMEOUT:
        {
            break;
        }

        case ISP_GET_VD_BEEND_TIMEOUT:
        {
            break;
        }

        case ISP_FRAME_INFO_SET:
        {
            break;
        }

        case ISP_FRAME_INFO_GET:
        {
            break;
        }

        case IOC_NR_ISP_FRAME_CNT_GET:
        {
            break;
        }
        case ISP_SET_INT_ENABLE :
        {
            break;
        }

        case ISP_STAT_BUF_INIT :
        {
            break;
        }

        case ISP_STAT_BUF_EXIT :
        {
            break;
        }

        case ISP_STAT_BUF_GET :
        {
            break;
        }

        case ISP_STAT_BUF_PUT :
        {
            break;
        }

        case ISP_STAT_ACT_GET:
        {
            break;
        }

        case ISP_REG_CFG_SET:
        {
            break;
        }

        case ISP_BE_CFG_BUF_INIT:
        {
            ISP_BE_WO_CFG_BUF_S *pstBeWoCfgBuf = (ISP_BE_WO_CFG_BUF_S *)(HI_UINTPTR_T)arg;
            COMPAT_POINTER(pstBeWoCfgBuf->pVirAddr, HI_VOID *);
            break;
        }

        case ISP_GET_BE_BUF_FIRST:
        {
            break;
        }

        case ISP_BE_FREE_BUF_GET:
        {
            break;
        }

        case ISP_BE_CFG_BUF_EXIT:
        {
            break;
        }

        case ISP_BE_CFG_BUF_RUNNING:
        {
            break;
        }

        case ISP_BE_CFG_BUF_CTL:
        {
            break;
        }

        case ISP_BE_All_BUF_INIT:
        {
            break;
        }

        case ISP_SYNC_CFG_SET:
        {
            break;
        }

        case ISP_WDR_CFG_SET:
        {
            break;
        }

        case ISP_BE_SYNC_PARAM_INIT:
        {
            break;
        }

        case ISP_RES_SWITCH_SET:
        {
            break;
        }

        case ISP_CHN_SELECT_CFG:
        {
            break;
        }

        case ISP_PROC_INIT:
        {
            break;
        }

        case ISP_PROC_WRITE_ING:
        {
            break;
        }

        case ISP_PROC_WRITE_OK:
        {
            break;
        }

        case ISP_PROC_EXIT:
        {
            break;
        }

        case ISP_PROC_PARAM_GET:
        {
            break;
        }

        case ISP_MEM_INFO_SET:
        {
            break;
        }

        case ISP_MEM_INFO_GET:
        {
            break;
        }

        case ISP_P2EN_INFO_GET:
        {
            break;
        }

        case ISP_SYNC_INIT_SET:
        {
            break;
        }

        case ISP_RESET_CTX:
        {
            break;
        }

        case ISP_CONFIG_INFO_SET:
        {
            break;
        }

        case ISP_SNAP_INFO_GET:
        {
            break;
        }

        case ISP_PRO_TRIGGER_GET:
        {
            break;
        }

        case ISP_SNAP_ATTR_GET:
        {
            break;
        }

        case ISP_UPDATE_POS_GET:
        {
            break;
        }

        case ISP_PUB_ATTR_INFO:
        {
            break;
        }

        case ISP_PWM_NUM_GET:
        {
            break;
        }

        case ISP_SET_MOD_PARAM:
        {
            break;
        }

        case ISP_GET_MOD_PARAM:
        {
            break;
        }

        case ISP_WORK_MODE_INIT:
        {
            break;
        }

        case ISP_WORK_MODE_EXIT:
        {
            break;
        }

        case ISP_WORK_MODE_GET:
        {
            break;
        }

        case ISP_GET_PIPE_SIZE:
        {
            break;
        }

        case ISP_GET_HDR_ATTR:
        {
            break;
        }

        case ISP_GET_WDR_ATTR:
        {
            break;
        }

        case ISP_GET_STITCH_ATTR:
        {
            break;
        }

        case ISP_GET_FUSION_ATTR:
        {
            break;
        }

        case ISP_KERNEL_RUNONCE:
        {
            break;
        }

        case ISP_SET_PROCALCDONE:
        {
            break;
        }

        default:
        {
            break;
        }
    }

    return ISP_ioctl(cmd, arg, private_data);
}
#endif

static int ISP_open(void *data)
{
    return 0;
}

static int ISP_close(void *data)
{
    return 0;
}

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
static HI_S32 ISP_Freeze(osal_dev_t *pdev)
{
    return HI_SUCCESS;
}

static HI_S32 ISP_Restore(osal_dev_t *pdev)
{
    VI_PIPE ViPipe;

    for (ViPipe = 0; ViPipe < ISP_MAX_PIPE_NUM; ViPipe++)
    {
        /* enable port int & isp int */
        IO_RW_PT_ADDRESS(VICAP_HD_MASK) |= VICAP_INT_MASK_PT(ViPipe);
        IO_RW_PT_ADDRESS(VICAP_HD_MASK) |= VICAP_INT_MASK_ISP(ViPipe);

        IO_RW_FE_ADDRESS(ViPipe, ISP_INT_FE_MASK) = (0x0);
    }

    return HI_SUCCESS;
}
#else
static HI_S32 ISP_Freeze(osal_dev_t *pdev)
{
    return HI_SUCCESS;
}

static HI_S32 ISP_Restore(osal_dev_t *pdev)
{
    return HI_SUCCESS;
}
#endif

static struct osal_fileops stIspFileOp =
{
    .unlocked_ioctl   = ISP_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl     = ISP_compat_ioctl,
#endif
    .open             = ISP_open,
    .release          = ISP_close
};

struct osal_pmops stHiISPDrvOps =
{
    .pm_freeze      = ISP_Freeze,
    .pm_restore     = ISP_Restore
};

static osal_dev_t *s_pstHiISPDevice = HI_NULL;

static inline int ISP_ISR(int irq, void *id)
{
    HI_U32  i;
    HI_S32  ViDev;
    VI_PIPE ViPipe;
    HI_U32 u32PortIntStatus = 0;
    HI_U32 u32PortIntFStart = 0, u32PortIntErr = 0;
    HI_U32 u32IspRawIntStatus = 0, u32IspIntStatus = 0;
    HI_U32 u32WCHIntStatus = 0, u32WCHIntFStart = 0;
    HI_U64 u64IspIrqBeginTime = 0;
    HI_BOOL bViCapInt = HI_FALSE;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    bViCapInt      = (irq == isp_fe_irq);

    u64IspIrqBeginTime = CALL_SYS_GetTimeStamp();

    /* Isp FE Interrupt Process Begin */
    if (bViCapInt)
    {
        for (i = 0; i < ISP_MAX_PHY_PIPE_NUM; i++)
        {
            ViPipe = i;

            pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
            ViDev     = pstDrvCtx->stWDRAttr.ViDev;

            /* read interrupt status */
            u32PortIntStatus  = IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_INT);
            u32PortIntStatus &= IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_INT_MASK);
            u32PortIntFStart  = u32PortIntStatus & VI_PT_INT_FSTART;
            u32PortIntErr     = u32PortIntStatus & VI_PT_INT_ERR;

            u32WCHIntStatus   = IO_RW_CH_ADDRESS(ViPipe, VI_WCH_INT);
            u32WCHIntFStart   = u32WCHIntStatus & VI_WCH_INT_FSTART;

            u32IspRawIntStatus  = IO_RW_FE_ADDRESS(ViPipe, ISP_INT_FE);
            u32IspIntStatus     = u32IspRawIntStatus & IO_RW_FE_ADDRESS(ViPipe, ISP_INT_FE_MASK);

            pstDrvCtx->stIntSch.u32IspIntStatus  = u32IspIntStatus;
            pstDrvCtx->stIntSch.u32PortIntStatus = u32PortIntFStart;
            pstDrvCtx->stIntSch.u32PortIntErr    = u32PortIntErr;
            pstDrvCtx->stIntSch.u32WCHIntStatus  = u32WCHIntFStart;

            pstDrvCtx->stIspSyncCtrl.u64PTS = u64IspIrqBeginTime;

            /* clear interrupt */

            if (u32PortIntStatus || u32IspRawIntStatus)
            {
                if (u32PortIntStatus)
                {
                    IO_RW_PT_ADDRESS(VI_PT_BASE(ViDev) + VI_PT_INT) = u32PortIntStatus;
                }

                if (u32IspRawIntStatus)
                {
                    IO_RW_FE_ADDRESS(ViPipe, ISP_INT_FE) = u32IspRawIntStatus;
                }
            }
            else
            {
                continue;
            }

            if (u32PortIntErr)
            {
                pstDrvCtx->stDrvDbgInfo.u32IspResetCnt++;
                continue;
            }
        }

        if (!g_UseBothalf)
        {
            ISP_DoIntBottomHalf(irq, id);
        }
#ifdef CONFIG_HI_ISP_LITEOS_BOTTOM_HALF_SUPPORT
#ifdef __HuaweiLite__
        else
        {
            ISP_DRV_WorkQueueRun(ViPipe);
        }
#endif
#endif
    }

    //return (HI_TRUE == g_UseBothalf) ? OSAL_IRQ_WAKE_THREAD : OSAL_IRQ_HANDLED;

    return OSAL_IRQ_WAKE_THREAD;
}

int  ISP_IntBottomHalf(int irq, void *id )
{
    if (g_UseBothalf)
    {
        return ISP_DoIntBottomHalf(irq, id);
    }
    else
    {
        return OSAL_IRQ_HANDLED;
    }
}

HI_VOID ISP_DRV_WakeUpThread(VI_PIPE ViPipe)
{
    HI_BOOL bWakeUpTread = HI_TRUE;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if ((IS_OFFLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)) ||
        (IS_STRIPING_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)))
    {
        if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable)
        {
            if (ISP_BE_BUF_STATE_INIT != pstDrvCtx->enIspRunningState)
            {
                bWakeUpTread = HI_FALSE;
            }
        }
    }

    if (HI_TRUE == bWakeUpTread)
    {
        pstDrvCtx->bEdge = HI_TRUE;
        pstDrvCtx->bVdStart = HI_TRUE;

        osal_wakeup(&pstDrvCtx->stIspWait);
        osal_wakeup(&pstDrvCtx->stIspWaitVdStart);
    }

    return;
}

HI_S32 ISP_DRV_GetUseNode(VI_PIPE ViPipe, HI_U32 u32IspIntStatus, HI_U32 u32WCHIntStatus)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_SYNC_CFG_S *pstSyncCfg = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx  = ISP_DRV_GET_CTX(ViPipe);
    pstSyncCfg = &pstDrvCtx->stSyncCfg;

    if ((IS_OFFLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)) ||
        (IS_STRIPING_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)))
    {
        ISP_CHECK_BEBUF_INIT(ViPipe);

        if (u32IspIntStatus & ISP_1ST_INT)
        {
            if (pstDrvCtx->pstUseNode)
            {
                /* Need to configure the sensor registers and get statistics for AE/AWB. */
                osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

                ISP_DRV_GetSyncControlnfo(ViPipe, pstSyncCfg);
                ISP_DRV_CalcSyncCfg(pstSyncCfg);
                ISP_DRV_RegConfigIsp(ViPipe, pstDrvCtx);
                ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);

                pstDrvCtx->u32Status = u32IspIntStatus;

                osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

                if (0 == pstDrvCtx->u32FrameCnt++ % DIV_0_TO_1(g_StatIntvl[ViPipe]))
                {
                    ISP_DRV_StatBufBusyPut(ViPipe);
                }

                osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
                ISP_DRV_WakeUpThread(ViPipe);
                osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
                IspSyncTaskProcess(ViPipe);

                return HI_FAILURE;
            }

            osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

            pstDrvCtx->pstUseNode = ISP_QueueGetFreeBeBuf(&pstDrvCtx->stBeBufQueue);

            if (HI_NULL == pstDrvCtx->pstUseNode)
            {
                /* Need to configure the sensor registers. */
                ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);
                osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
                ISP_TRACE(HI_DBG_ERR, "Pipe[%d] get FreeBeBuf is fail!\r\n", ViPipe);

                return HI_FAILURE;
            }

            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        }

        if ((pstDrvCtx->bYuvMode == HI_TRUE) && u32WCHIntStatus)
        {
            if (pstDrvCtx->pstUseNode)
            {
                /* Need to configure the sensor registers and get statistics for AE/AWB. */
                osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

                ISP_DRV_GetSyncControlnfo(ViPipe, pstSyncCfg);
                ISP_DRV_CalcSyncCfg(pstSyncCfg);
                ISP_DRV_RegConfigIsp(ViPipe, pstDrvCtx);
                //ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);

                pstDrvCtx->u32Status = u32IspIntStatus;

                osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

                if (0 == pstDrvCtx->u32FrameCnt++ % DIV_0_TO_1(g_StatIntvl[ViPipe]))
                {
                    ISP_DRV_StatBufBusyPut(ViPipe);
                }

                osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
                ISP_DRV_WakeUpThread(ViPipe);
                osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
                IspSyncTaskProcess(ViPipe);

                return HI_FAILURE;
            }

            osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

            pstDrvCtx->pstUseNode = ISP_QueueGetFreeBeBuf(&pstDrvCtx->stBeBufQueue);

            if (HI_NULL == pstDrvCtx->pstUseNode)
            {
                /* Need to configure the sensor registers. */
                // ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);
                osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
                ISP_TRACE(HI_DBG_ERR, "Pipe[%d] get FreeBeBuf is fail!\r\n", ViPipe);

                return HI_FAILURE;
            }

            osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StitchGetUseNode(VI_PIPE ViPipe, HI_U32 u32IspIntStatus)
{
    VI_PIPE MainPipeS;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_SYNC_CFG_S *pstSyncCfg = HI_NULL;
    unsigned long u32Flags;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx  = ISP_DRV_GET_CTX(ViPipe);
    pstSyncCfg = &pstDrvCtx->stSyncCfg;
    MainPipeS  = pstDrvCtx->stStitchAttr.as8StitchBindId[0];

    if ((IS_OFFLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)) ||
        (IS_STRIPING_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)))
    {
        ISP_CHECK_BEBUF_INIT(ViPipe);

        if (u32IspIntStatus & ISP_1ST_INT)
        {
            osal_spin_lock_irqsave(&g_stIspSyncLock[MainPipeS], &u32Flags);

            if (pstDrvCtx->pstUseNode)
            {
                /* Need to configure the sensor registers and get statistics for AE/AWB. */
                ISP_DRV_GetSyncControlnfo(ViPipe, pstSyncCfg);
                ISP_DRV_CalcSyncCfg(pstSyncCfg);
                ISP_DRV_RegConfigIsp(ViPipe, pstDrvCtx);
                ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);

                pstDrvCtx->u32Status = u32IspIntStatus;

                if (0 == pstDrvCtx->u32FrameCnt++ % DIV_0_TO_1(g_StatIntvl[ViPipe]))
                {
                    ISP_DRV_StatBufBusyPut(ViPipe);
                }

                ISP_DRV_WakeUpThread(ViPipe);
                osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);
                IspSyncTaskProcess(ViPipe);

                return HI_FAILURE;
            }

            pstDrvCtx->pstUseNode = ISP_QueueGetFreeBeBuf(&pstDrvCtx->stBeBufQueue);

            if (HI_NULL == pstDrvCtx->pstUseNode)
            {
                /* Need to configure the sensor registers. */
                ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);
                osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);
                ISP_TRACE(HI_DBG_ERR, "Pipe[%d] get FreeBeBuf is fail!\r\n", ViPipe);

                return HI_FAILURE;
            }

            osal_spin_unlock_irqrestore(&g_stIspSyncLock[MainPipeS], &u32Flags);
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetBeBufUseNode(VI_PIPE ViPipe, HI_U32 u32IspIntStatus)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U32 u32WCHIntFStart;
    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
    u32WCHIntFStart = pstDrvCtx->stIntSch.u32WCHIntStatus;
    if (HI_TRUE != pstDrvCtx->stStitchAttr.bStitchEnable)
    {
        return ISP_DRV_GetUseNode(ViPipe, u32IspIntStatus, u32WCHIntFStart);
    }
    else
    {
        return ISP_DRV_StitchGetUseNode(ViPipe, u32IspIntStatus);
    }
}

int  ISP_IrqRoute(VI_PIPE ViPipe)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_SYNC_CFG_S *pstSyncCfg = HI_NULL;
    HI_U64 u64SensorCfgTime1 = 0, u64SensorCfgTime2 = 0;
    HI_U32 u32PortIntFStart;
    HI_U32 u32IspIntStatus;
    HI_U32 u32WCHIntFStart;
    HI_U32 u32SensorCfgInt = 0;
    HI_U64 u64PtTime1 = 0, u64PtTime2 = 0;
    HI_U64 u64IspTime1 = 0, u64IspTime2 = 0;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
    pstSyncCfg = &pstDrvCtx->stSyncCfg;

    u32PortIntFStart = pstDrvCtx->stIntSch.u32PortIntStatus;
    u32IspIntStatus  = pstDrvCtx->stIntSch.u32IspIntStatus;
    u32WCHIntFStart = pstDrvCtx->stIntSch.u32WCHIntStatus;

    if (u32PortIntFStart) /* port int proc */
    {
        pstDrvCtx->stDrvDbgInfo.u32PtIntCnt++;
        u64PtTime1 = CALL_SYS_GetTimeStamp();

        if (pstDrvCtx->stDrvDbgInfo.u64PtLastIntTime) //not first int
        {
            pstDrvCtx->stDrvDbgInfo.u32PtIntGapTime = u64PtTime1 - pstDrvCtx->stDrvDbgInfo.u64PtLastIntTime;

            if (pstDrvCtx->stDrvDbgInfo.u32PtIntGapTime > pstDrvCtx->stDrvDbgInfo.u32PtIntGapTimeMax)
            {
                pstDrvCtx->stDrvDbgInfo.u32PtIntGapTimeMax = pstDrvCtx->stDrvDbgInfo.u32PtIntGapTime;
            }
        }

        pstDrvCtx->stDrvDbgInfo.u64PtLastIntTime = u64PtTime1;
    }

    if (u32IspIntStatus & ISP_1ST_INT) /* isp int proc */
    {
        pstDrvCtx->stDrvDbgInfo.u32IspIntCnt++;
        u64IspTime1 = CALL_SYS_GetTimeStamp();

        if (pstDrvCtx->stDrvDbgInfo.u64IspLastIntTime)  //not first int
        {
            pstDrvCtx->stDrvDbgInfo.u32IspIntGapTime = u64IspTime1 - pstDrvCtx->stDrvDbgInfo.u64IspLastIntTime;

            if (pstDrvCtx->stDrvDbgInfo.u32IspIntGapTime > pstDrvCtx->stDrvDbgInfo.u32IspIntGapTimeMax)
            {
                pstDrvCtx->stDrvDbgInfo.u32IspIntGapTimeMax = pstDrvCtx->stDrvDbgInfo.u32IspIntGapTime;
            }
        }

        pstDrvCtx->stDrvDbgInfo.u64IspLastIntTime = u64IspTime1;
    }

    pstDrvCtx->u32IntPos = 0;

    s32Ret = ISP_DRV_GetBeBufUseNode(ViPipe, u32IspIntStatus);
    if (HI_SUCCESS != s32Ret)
    {
        return OSAL_IRQ_HANDLED;
    }

    if (u32IspIntStatus & ISP_1ST_INT)
    {
        if (ViPipe == pstDrvCtx->stSnapAttr.s32PicturePipeId)
        {
            if (CKFN_VI_GetIspConfig())
            {
                CALL_VI_GetIspConfig(ViPipe, &pstDrvCtx->stSnapInfoLoad);
            }
        }
    }

    if (u32IspIntStatus & ISP_1ST_INT)
    {
        if (IS_FULL_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
        {
            ISP_DRV_GetSyncControlnfo(ViPipe, pstSyncCfg);
            ISP_DRV_CalcSyncCfg(pstSyncCfg);
            ISP_DRV_RegConfigIsp(ViPipe, pstDrvCtx);
            ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);
        }
    }

    if (u32PortIntFStart)
    {
        if (IS_HALF_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
        {
            ISP_DRV_GetSyncControlnfo(ViPipe, pstSyncCfg);
            ISP_DRV_CalcSyncCfg(pstSyncCfg);
            ISP_DRV_RegConfigIsp(ViPipe, pstDrvCtx);
            ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);
        }
    }

    /* * * * * * * * isp_int1_process * * * * * * * */
    if (u32IspIntStatus & ISP_1ST_INT)
    {
        if (IS_LINE_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
        {
            ISP_DRV_GetSyncControlnfo(ViPipe, pstSyncCfg);
            ISP_DRV_CalcSyncCfg(pstSyncCfg);
            ISP_DRV_RegConfigIsp(ViPipe, pstDrvCtx);

            u64SensorCfgTime1 = CALL_SYS_GetTimeStamp();
            ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);
            u64SensorCfgTime2 = CALL_SYS_GetTimeStamp();
            pstDrvCtx->stDrvDbgInfo.u32SensorCfgTime = u64SensorCfgTime2 - u64SensorCfgTime1;
        }
    }

    u32SensorCfgInt = (u32IspIntStatus & ISP_2ND_INT);

    if (0 == g_UpdatePos[ViPipe]) /* frame start */
    {
        u32SensorCfgInt = (u32IspIntStatus & ISP_1ST_INT);
    }

    /* * * * * * * * isp_int2_process * * * * * * * */
    if (u32SensorCfgInt)
    {
        /* In linear mode or built-in WDR mode, config sensor and vi(isp) register with isp_int(frame start interrupt) */
        if (IS_LINEAR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode) || IS_BUILT_IN_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
        {
            ISP_DRV_GetSyncControlnfo(ViPipe, pstSyncCfg);
            ISP_DRV_CalcSyncCfg(pstSyncCfg);
            ISP_DRV_RegConfigIsp(ViPipe, pstDrvCtx);

            u64SensorCfgTime1 = CALL_SYS_GetTimeStamp();
            ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);
            u64SensorCfgTime2 = CALL_SYS_GetTimeStamp();
            pstDrvCtx->stDrvDbgInfo.u32SensorCfgTime = u64SensorCfgTime2 - u64SensorCfgTime1;
        }
    }

    if (u32IspIntStatus & ISP_2ND_INT)
    {
        pstDrvCtx->u32IntPos = 1;
        pstDrvCtx->u32Status = u32IspIntStatus;

        ISP_DRV_RegConfigSensor(ViPipe, pstDrvCtx);

        pstDrvCtx->bVdEnd = HI_TRUE;
        if (1 == pstDrvCtx->u32ProTrigFlag)
        {
            pstDrvCtx->u32ProTrigFlag++;
        }
        ISP_UpdateInfoSync(ViPipe, &pstDrvCtx->stUpdateInfo);
        ISP_frameInfoSync(ViPipe, &pstDrvCtx->stFrameInfo);
        osal_wakeup(&pstDrvCtx->stIspWaitVdEnd);
    }

    if (pstDrvCtx->stDrvDbgInfo.u32SensorCfgTime > pstDrvCtx->stDrvDbgInfo.u32SensorCfgTimeMax)
    {
        pstDrvCtx->stDrvDbgInfo.u32SensorCfgTimeMax = pstDrvCtx->stDrvDbgInfo.u32SensorCfgTime;
    }

    /* * * * * * * * isp_int_process * * * * * * * */
    if (u32IspIntStatus & ISP_1ST_INT) /* ISP int */
    {
        /* N to 1 fullrate frame WDR mode, get statistics only in the last frame(N-1) */
        if (IS_FULL_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
        {
            if (pstDrvCtx->stSyncCfg.u8VCNum != pstDrvCtx->stSyncCfg.u8VCNumMax)
            {
                return OSAL_IRQ_HANDLED;
            }
        }

        pstDrvCtx->u32Status = u32IspIntStatus;

        /* first config register. */
        if (0 == pstDrvCtx->u32FrameCnt++ % DIV_0_TO_1(g_StatIntvl[ViPipe]))
        {
            ISP_DRV_StatBufBusyPut(ViPipe);
        }

        ISP_DRV_WakeUpThread(ViPipe);
        /* Sync  task AF statistics */
        IspSyncTaskProcess(ViPipe);

    }

    /* * * * * * * * isp_int_process * * * * * * * */
    if (u32WCHIntFStart && (pstDrvCtx->bYuvMode == HI_TRUE)) /* WCH int */
    {
        //  osal_printk("u23WCHIntFStart =%d   \r\n", u32WCHIntFStart);
        pstDrvCtx->u32Status = u32WCHIntFStart;

        /* first config register. */
        if (0 == pstDrvCtx->u32FrameCnt++ % DIV_0_TO_1(g_StatIntvl[ViPipe]))
        {
            ISP_DRV_StatBufBusyPut(ViPipe);
        }

        ISP_DRV_WakeUpThread(ViPipe);
        /* Sync  task AF statistics */
        IspSyncTaskProcess(ViPipe);

    }


    if (u32PortIntFStart) /* port int proc */
    {
        u64PtTime2 = CALL_SYS_GetTimeStamp();
        pstDrvCtx->stDrvDbgInfo.u32PtIntTime = u64PtTime2 - u64PtTime1;

        if (pstDrvCtx->stDrvDbgInfo.u32PtIntTime > pstDrvCtx->stDrvDbgInfo.u32PtIntTimeMax)
        {
            pstDrvCtx->stDrvDbgInfo.u32PtIntTimeMax = pstDrvCtx->stDrvDbgInfo.u32PtIntTime;
        }

        if ((u64PtTime2 - pstDrvCtx->stDrvDbgInfo.u64PtLastRateTime) >= 1000000ul)
        {
            pstDrvCtx->stDrvDbgInfo.u64PtLastRateTime = u64PtTime2;
            pstDrvCtx->stDrvDbgInfo.u32PtRate = pstDrvCtx->stDrvDbgInfo.u32PtRateIntCnt;
            pstDrvCtx->stDrvDbgInfo.u32PtRateIntCnt = 0;
        }

        pstDrvCtx->stDrvDbgInfo.u32PtRateIntCnt++;
    }

    if (u32IspIntStatus & ISP_1ST_INT) /* isp int proc */
    {
        u64IspTime2 = CALL_SYS_GetTimeStamp();
        pstDrvCtx->stDrvDbgInfo.u32IspIntTime = u64IspTime2 - u64IspTime1;

        if (pstDrvCtx->stDrvDbgInfo.u32IspIntTime > pstDrvCtx->stDrvDbgInfo.u32IspIntTimeMax)
        {
            pstDrvCtx->stDrvDbgInfo.u32IspIntTimeMax = pstDrvCtx->stDrvDbgInfo.u32IspIntTime;
        }

        if ((u64IspTime2 - pstDrvCtx->stDrvDbgInfo.u64IspLastRateTime) >= 1000000ul)
        {
            pstDrvCtx->stDrvDbgInfo.u64IspLastRateTime = u64IspTime2;
            pstDrvCtx->stDrvDbgInfo.u32IspRate = pstDrvCtx->stDrvDbgInfo.u32IspRateIntCnt;
            pstDrvCtx->stDrvDbgInfo.u32IspRateIntCnt = 0;
        }

        pstDrvCtx->stDrvDbgInfo.u32IspRateIntCnt++;
    }

    return OSAL_IRQ_HANDLED;
}

int  ISP_DoIntBottomHalf(int irq, void *id )
{
    VI_PIPE ViPipe, ViPipeS;
    HI_U32 i, j;
    HI_S32 s32Ret;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_DRV_CTX_S *pstDrvCtxS = HI_NULL;
    HI_U32  u32PortIntStatus = 0;
    HI_U32  u32PortIntErr = 0;
    HI_U32  u32IspIntStatus = 0;
    HI_U32  u32WCHIntFStart = 0;
    for (i = 0; i < ISP_MAX_PHY_PIPE_NUM; i++)
    {
        ViPipe = i;
        pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

        u32IspIntStatus  = pstDrvCtx->stIntSch.u32IspIntStatus;
        u32PortIntStatus = pstDrvCtx->stIntSch.u32PortIntStatus;
        u32PortIntErr    = pstDrvCtx->stIntSch.u32PortIntErr;
        u32WCHIntFStart = pstDrvCtx->stIntSch.u32WCHIntStatus;

        if ((!u32PortIntStatus) && (!u32IspIntStatus) && (!u32WCHIntFStart))
        {
            continue;
        }

        if (!pstDrvCtx->bMemInit)
        {
            continue;
        }

        if (HI_TRUE == pstDrvCtx->stStitchAttr.bStitchEnable)
        {
            if (HI_TRUE == pstDrvCtx->stStitchAttr.bMainPipe)
            {
                s32Ret = ISP_DRV_StitchSync(ViPipe);

                if (HI_SUCCESS != s32Ret)
                {
                    continue;
                }

                for (j = 0; j < pstDrvCtx->stStitchAttr.u8StitchPipeNum; j++)
                {
                    ViPipeS = pstDrvCtx->stStitchAttr.as8StitchBindId[j];
                    pstDrvCtxS = ISP_DRV_GET_CTX(ViPipeS);

                    pstDrvCtxS->stIntSch.u32IspIntStatus  = u32IspIntStatus;
                    pstDrvCtxS->stIntSch.u32PortIntStatus = u32PortIntStatus;
                    pstDrvCtxS->stIntSch.u32PortIntErr    = u32PortIntErr;

                    ISP_IrqRoute(ViPipeS);
                }
            }
            else
            {
                continue;
            }
        }
        else
        {
            ISP_IrqRoute(ViPipe);
        }

    }

    return OSAL_IRQ_HANDLED;

}

#ifdef CONFIG_HI_ISP_LITEOS_BOTTOM_HALF_SUPPORT
static void ISP_DRV_WorkQueueHandler(struct osal_work_struct *pstWorker)
{
    ISP_WORKQUEUE_CTX_S *pstIspWorkQueue = osal_container_of((void *)pstWorker, ISP_WORKQUEUE_CTX_S, worker);
    if (osal_down_interruptible(&pstIspWorkQueue->sem))
    {
        return ;
    }
    ISP_DoIntBottomHalf(0,0);
    osal_up(&pstIspWorkQueue->sem);

    return ;
}
void ISP_DRV_WorkQueueRun(VI_PIPE ViPipe)
{
    osal_schedule_work(&g_astIspWorkQueueCtx.worker);
}
void ISP_DRV_WorkQueueInit(void)
{

    osal_sema_init(&g_astIspWorkQueueCtx.sem, 1);
    OSAL_INIT_WORK(&g_astIspWorkQueueCtx.worker,ISP_DRV_WorkQueueHandler);

}
void ISP_DRV_WorkQueueExit(void)
{
    osal_sema_destory(&g_astIspWorkQueueCtx.sem);
    osal_destroy_work(&g_astIspWorkQueueCtx.worker);

}
#endif

static int ISP_DRV_Init(void)
{
    HI_S32 s32Ret;

    s32Ret = ISP_DRV_BeRemap();

    if (HI_FAILURE == s32Ret)
    {
        return s32Ret;
    }

    s32Ret = ISP_DRV_VicapRemap();
    if (HI_FAILURE == s32Ret)
    {
        return s32Ret;
    }

    s32Ret = ISP_DRV_FeRemap();

    if (HI_FAILURE == s32Ret)
    {
        return s32Ret;
    }
#ifdef CONFIG_HI_ISP_LITEOS_BOTTOM_HALF_SUPPORT
    ISP_DRV_WorkQueueInit();
#endif
    return 0;
}

static int ISP_DRV_Exit(void)
{
    ISP_DRV_BeUnmap();

    ISP_DRV_VicapUnmap();

    ISP_DRV_FeUnmap();
#ifdef CONFIG_HI_ISP_LITEOS_BOTTOM_HALF_SUPPORT
    ISP_DRV_WorkQueueExit();
#endif

    return 0;
}

#ifdef TEST_TIME
static int ISP_Test_Init(void)
{
    CMPI_MmzMallocNocache(HI_NULL, "ISPStatTest", &g_test_phyaddr, (HI_VOID **)&g_test_pviraddr, 256 * 2);
}

static int ISP_Test_Exit(void)
{
    CMPI_MmzFree(g_test_phyaddr, g_test_pviraddr);
}
#endif


#ifndef DISABLE_DEBUG_INFO
static int ISP_ProcShow(osal_proc_entry_t *s)
{
    VI_PIPE ViPipe = 0;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    //extern char proc_title[1024];
    ISP_SNAP_PIPE_MODE_E enSnapPipeMode = ISP_SNAP_NONE;

    //osal_seq_printf(s, "\n[ISP] Version: ["ISP_VERSION"], %s", proc_title);
    osal_seq_printf(s, "\n[ISP] Version: ["ISP_VERSION"], Build Time["__DATE__", "__TIME__"]\n\n");
    osal_seq_printf(s, "\n");

    do
    {
        pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);
        if (!pstDrvCtx->bMemInit)
        {
            continue;
        }

        if ((pstDrvCtx->stSnapAttr.s32PicturePipeId == pstDrvCtx->stSnapAttr.s32PreviewPipeId) && (pstDrvCtx->stSnapAttr.s32PreviewPipeId != -1))
        {
            enSnapPipeMode = ISP_SNAP_PREVIEW_PICTURE;
        }
        else if (pstDrvCtx->stSnapAttr.s32PicturePipeId == ViPipe)
        {
            enSnapPipeMode = ISP_SNAP_PICTURE;
        }
        else if (pstDrvCtx->stSnapAttr.s32PreviewPipeId == ViPipe)
        {
            enSnapPipeMode = ISP_SNAP_PREVIEW;
        }
        else
        {
            enSnapPipeMode = ISP_SNAP_NONE;
        }

        osal_seq_printf(s, "--------------------------------------------------------------------------------------------------\n");
        osal_seq_printf(s, "------------------------------------- ISP PROC PIPE[%d] ------------------------------------------\n", ViPipe);
        osal_seq_printf(s, "--------------------------------------------------------------------------------------------------\n");
        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "-----MODULE/CONTROL PARAM--------------------------------------------------------------------------\n");
        osal_seq_printf(s, " %12s" " %12s" " %12s" " %12s" " %12s" " %12s" " %14s" "\n",  "ProcParam", "StatIntvl", "UpdatePos", \
                        "IntBothalf", "IntTimeout", "PwmNumber", "PortIntDelay");
        osal_seq_printf(s, " %12u" " %12u" " %12u" " %12u" " %12u" " %12u" " %14u" "\n",  g_ProcParam[ViPipe], g_StatIntvl[ViPipe], \
                        g_UpdatePos[ViPipe], g_UseBothalf, g_IntTimeout[ViPipe], g_PwmNumber[ViPipe], g_PortIntDelay[ViPipe]);
        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "-----ISP Mode--------------------------------------------------------------------------------------\n");
        osal_seq_printf(s, " %15s" " %15s" "\n",  "StitchMode", "RunningMode");
        osal_seq_printf(s, " %15s" " %15s" "\n",
                        pstDrvCtx->stStitchAttr.bStitchEnable ? "STITCH" : "NORMAL",
                        (ISP_MODE_RUNNING_OFFLINE    == pstDrvCtx->stWorkMode.enIspRunningMode) ? "OFFLINE"  :
                        (ISP_MODE_RUNNING_ONLINE     == pstDrvCtx->stWorkMode.enIspRunningMode) ? "ONLINE"   :
                        (ISP_MODE_RUNNING_SIDEBYSIDE == pstDrvCtx->stWorkMode.enIspRunningMode) ? "SBS"      :
                        (ISP_MODE_RUNNING_STRIPING   == pstDrvCtx->stWorkMode.enIspRunningMode) ? "STRIPING" : "BUTT");
        osal_seq_printf(s, "\n");
        osal_seq_printf(s, "-----DRV INFO-------------------------------------------------------------------------------------\n");

        osal_seq_printf(s, "%11s" "%11s" "%11s" "%11s" "%11s" "%11s" "%9s" "%12s" "%14s\n"
                        , "ViPipe", "IntCnt", "IntT", "MaxIntT", "IntGapT", "MaxGapT", "IntRat", "IspResetCnt", "IspBeStaLost");

        osal_seq_printf(s, "%11d" "%11d" "%11d" "%11d" "%11d" "%11d" "%9d" "%12d" "%14d\n",
                        ViPipe,
                        pstDrvCtx->stDrvDbgInfo.u32IspIntCnt,
                        pstDrvCtx->stDrvDbgInfo.u32IspIntTime,
                        pstDrvCtx->stDrvDbgInfo.u32IspIntTimeMax,
                        pstDrvCtx->stDrvDbgInfo.u32IspIntGapTime,
                        pstDrvCtx->stDrvDbgInfo.u32IspIntGapTimeMax,
                        pstDrvCtx->stDrvDbgInfo.u32IspRate,
                        pstDrvCtx->stDrvDbgInfo.u32IspResetCnt,
                        pstDrvCtx->stDrvDbgInfo.u32IspBeStaLost);

        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "%11s" "%11s" "%11s" "%11s" "%11s" "%11s" "%9s" "%11s" "%12s\n"
                        , "", "PtIntCnt", "PtIntT", "PtMaxIntT", "PtIntGapT", "PtMaxGapT", "PtIntRat", "SensorCfgT", "SensorMaxT");

        osal_seq_printf(s, "%11s" "%11d" "%11d" "%11d" "%11d" "%11d" "%9d" "%11d" "%12d\n",
                        "",
                        pstDrvCtx->stDrvDbgInfo.u32PtIntCnt,
                        pstDrvCtx->stDrvDbgInfo.u32PtIntTime,
                        pstDrvCtx->stDrvDbgInfo.u32PtIntTimeMax,
                        pstDrvCtx->stDrvDbgInfo.u32PtIntGapTime,
                        pstDrvCtx->stDrvDbgInfo.u32PtIntGapTimeMax,
                        pstDrvCtx->stDrvDbgInfo.u32PtRate,
                        pstDrvCtx->stDrvDbgInfo.u32SensorCfgTime,
                        pstDrvCtx->stDrvDbgInfo.u32SensorCfgTimeMax);

        osal_seq_printf(s, "\n");

        /* TODO: show isp attribute here. width/height/bayer_format, etc..
          * Read parameter from memory directly. */
        osal_seq_printf(s, "-----PubAttr INFO---------------------------------------------------------------------------------\n");

        osal_seq_printf(s, "%12s" "%12s" "%12s" "%12s" "%12s" "%12s" "%12s\n"
                        , "WndX", "WndY", "WndW", "WndH", "SnsW", "SnsH", "Bayer");

        osal_seq_printf(s, "%12d" "%12d" "%12d" "%12d" "%12d" "%12d" "%12s\n\n",
                        pstDrvCtx->stProcPubInfo.stWndRect.s32X,
                        pstDrvCtx->stProcPubInfo.stWndRect.s32Y,
                        pstDrvCtx->stProcPubInfo.stWndRect.u32Width,
                        pstDrvCtx->stProcPubInfo.stWndRect.u32Height,
                        pstDrvCtx->stProcPubInfo.stSnsSize.u32Width,
                        pstDrvCtx->stProcPubInfo.stSnsSize.u32Height,
                        (0 == pstDrvCtx->stProcPubInfo.enBayer) ? "RGGB" :
                        (1 == pstDrvCtx->stProcPubInfo.enBayer) ? "GRBG" :
                        (2 == pstDrvCtx->stProcPubInfo.enBayer) ? "GBRG" :
                        (3 == pstDrvCtx->stProcPubInfo.enBayer) ? "BGGR" : "BUTT");
        osal_seq_printf(s, "\n");

        /* TODO: show isp snap attribute here. SnapType/PipeMode/OPType/ProFrmNum, etc.. */
        osal_seq_printf(s, "-----SNAPATTR INFO--------------------------------------------------------------------------------\n");

        osal_seq_printf(s, "%12s" "%12s" "%12s" "%12s\n"
                        , "SnapType", "PipeMode", "OPType", "ProFrmNum");
        osal_seq_printf(s, "%12s" "%12s" "%12s" "%12d\n\n",
                        (0 == pstDrvCtx->stSnapAttr.enSnapType) ? "NORMAL" :
                        (1 == pstDrvCtx->stSnapAttr.enSnapType) ? "PRO" : "BUTT",
                        (0 == enSnapPipeMode) ? "NONE" :
                        (1 == enSnapPipeMode) ? "PREVIEW" :
                        (2 == enSnapPipeMode) ? "PICTURE" : "PRE_PIC",
                        //(3 == enSnapPipeMode) ? "PRE_PIC" : "BUTT",
                        (0 == pstDrvCtx->stSnapAttr.stProParam.enOperationMode) ? "Auto" : "Manul",
                        pstDrvCtx->stSnapAttr.stProParam.u8ProFrameNum);

        ISP_DRV_ProcPrintf(ViPipe, s);

        osal_seq_printf(s, "--------------------------------------------------------------------------------------------------\n");
        osal_seq_printf(s, "----------------------------------------- ISP PROC END[%d] ---------------------------------------\n", ViPipe);
        osal_seq_printf(s, "--------------------------------------------------------------------------------------------------\n");
        osal_seq_printf(s, "\n\n");
    }
    while ( ++ViPipe < ISP_MAX_PIPE_NUM);

    return 0;
}
#endif

HI_S32 ISP_KernInit(void *p)
{
    HI_U32 ViPipe;

    for (ViPipe = 0; ViPipe < ISP_MAX_PIPE_NUM; ViPipe++)
    {
        osal_memset(&g_astIspDrvCtx[ViPipe].stDrvDbgInfo, 0, sizeof(ISP_DRV_DBG_INFO_S));
    }

    return HI_SUCCESS;
}

HI_VOID ISP_KernExit(void)
{
    HI_U32 ViPipe;

    for (ViPipe = 0; ViPipe < ISP_MAX_PIPE_NUM; ViPipe++)
    {
        if((HI_NULL != reg_vicap_base_va) && (HI_NULL != reg_ispfe_base_va[ViPipe]))
        {
            ISP_DRV_SetIntEnable(ViPipe, HI_FALSE);
        }
        osal_memset(&g_astIspDrvCtx[ViPipe].stDrvDbgInfo, 0, sizeof(ISP_DRV_DBG_INFO_S));
    }

    return ;
}

static HI_U32 ISP_GetVerMagic(HI_VOID)
{
    return VERSION_MAGIC;
}

ISP_EXPORT_FUNC_S g_stIspExpFunc   =
{
    .pfnISPRegisterBusCallBack     = ISP_RegisterBusCallBack,
    .pfnISPRegisterPirisCallBack   = ISP_RegisterPirisCallBack,
    .pfnISPRegisterViBusCallBack   = ISP_RegisterViBusCallBack,
    .pfnISP_GetDCFInfo             = ISP_GetDCFInfo,
    .pfnISP_SetDCFInfo             = ISP_SetDCFInfo,
    .pfnISP_GetIspUpdateInfo       = ISP_GetIspUpdateInfo,
    .pfnISP_SetIspUpdateInfo       = ISP_SetIspUpdateInfo,
    .pfnISP_GetFrameInfo           = ISP_GetFrameInfo,
    .pfnISP_SetFrameInfo           = ISP_SetFrameInfo,
    .pfnISP_GetAttachInfo          = ISP_GetAttachInfo,
    .pfnISP_GetColorGamutInfo      = ISP_GetColorGamutInfo,
    .pfnISP_GetDngImageDynamicInfo = ISP_DRV_GetDngImageDynamicInfo,
    .pfnISP_GetProCtrl             = ISP_GetProCtrl,
    .pfnISP_SetSnapAttr            = ISP_SetSnapAttr,
    .pfnISP_SetProNrParam          = ISP_SetProNrParam,
    .pfnISP_SetProShpParam         = ISP_SetProShpParam,
    .pfnISP_GetProNrParam          = ISP_GetProNrParam,
    .pfnISP_GetProShpParam         = ISP_GetProShpParam,
    .pfnISP_SaveSnapConfig         = ISP_SaveSnapConfig,
    .pfnISP_SetProEnable           = ISP_SetProEnable,
    .pfnISP_DRV_GetReadyBeBuf      = ISP_DRV_GetReadyBeBuf,
    .pfnISP_DRV_PutFreeBeBuf       = ISP_DRV_PutFreeBeBuf,
    .pfnISP_DRV_HoldBusyBeBuf      = ISP_DRV_HoldBusyBeBuf,
    .pfnISP_DRV_GetBeSyncPara      = ISP_DRV_GetBeSyncPara,
    .pfnISP_DRV_BeEndIntProc       = ISP_DRV_BeEndIntProc,
    .pfnISPRegisterSyncTask        = hi_isp_sync_task_register,
    .pfnISPUnRegisterSyncTask      = hi_isp_sync_task_unregister,
    .pfnISP_IntBottomHalf          = ISP_IntBottomHalf,
    .pfnISP_ISR                    = ISP_ISR,
    .pfnISP_GetPubAttr             = ISP_GetPubAttr,
    .pfnISP_DRV_GetRgbirFormat     = ISP_DRV_GetRgbirFormat,
};


static UMAP_MODULE_S s_stModule    =
{
    .enModId                       = HI_ID_ISP,
    .aModName                      = "isp",

    .pfnInit                       = ISP_KernInit,
    .pfnExit                       = ISP_KernExit,
    .pfnVerChecker                 = ISP_GetVerMagic,
    .pstExportFuncs                = &g_stIspExpFunc,
    .pData                         = HI_NULL,
};

int ISP_ModInit(void)
{
    HI_U32  ViPipe;

#ifndef DISABLE_DEBUG_INFO
    osal_proc_entry_t *proc = HI_NULL;
#endif

    s_pstHiISPDevice = osal_createdev("isp_dev");
    s_pstHiISPDevice->fops = &stIspFileOp;
    s_pstHiISPDevice->osal_pmops = &stHiISPDrvOps;
    s_pstHiISPDevice->minor = UMAP_ISP_MINOR_BASE;

    if (osal_registerdevice(s_pstHiISPDevice) < 0)
    {
        HI_PRINT("Kernel: Could not register isp devices\n");
        return HI_FAILURE;
    }

#ifndef DISABLE_DEBUG_INFO
    proc = osal_create_proc_entry(PROC_ENTRY_ISP, HI_NULL);

    if (HI_NULL == proc)
    {
        HI_PRINT("Kernel: Register isp proc failed!\n");
        goto OUT2;
    }

    proc->read = ISP_ProcShow;
#endif

    if (CMPI_RegisterModule(&s_stModule))
    {
        goto OUT1;;
    }

    for (ViPipe = 0; ViPipe < ISP_MAX_PIPE_NUM; ViPipe++)
    {
        osal_spin_lock_init(&g_stIspLock[ViPipe]);
        osal_spin_lock_init(&g_stIspSyncLock[ViPipe]);
    }

    if (0 != ISP_DRV_Init())
    {
        HI_PRINT("isp init failed\n");
        goto OUT1;
    }

#ifdef TEST_TIME
    ISP_Test_Init();
#endif
    HI_PRINT("ISP Mod init!\n");
    return HI_SUCCESS;

OUT1:
#ifndef DISABLE_DEBUG_INFO
    osal_remove_proc_entry(PROC_ENTRY_ISP, HI_NULL);
#endif
OUT2:
    osal_deregisterdevice(s_pstHiISPDevice);
    osal_destroydev(s_pstHiISPDevice);

    HI_PRINT("ISP Mod init failed!\n");
    return HI_FAILURE;
}

void ISP_ModExit(void)
{
    int i;

    ISP_DRV_Exit();

    for (i = 0; i < ISP_MAX_PIPE_NUM; i++)
    {
        osal_spin_lock_destory(&g_stIspLock[i]);
        osal_spin_lock_destory(&g_stIspSyncLock[i]);
    }

    CMPI_UnRegisterModule(HI_ID_ISP);

#ifndef DISABLE_DEBUG_INFO
    osal_remove_proc_entry(PROC_ENTRY_ISP, HI_NULL);
#endif
    osal_deregisterdevice(s_pstHiISPDevice);
    osal_destroydev(s_pstHiISPDevice);

#ifdef TEST_TIME
    ISP_Test_Exit();
#endif

    HI_PRINT("ISP Mod Exit!\n");
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
