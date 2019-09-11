/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_list.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       :
  Description   :
  History       :
  1.Date        : 2016/10/08
    Author      :
    Modification: Created file

******************************************************************************/
#ifndef __ISP_LIST_H__
#define __ISP_LIST_H__
#include "hi_osal.h"
#include "mkp_isp.h"

typedef struct hi_ISP_BE_BUF_NODE_S
{
    HI_S32                  s32HoldCnt;
    ISP_BE_WO_CFG_BUF_S     stBeCfgBuf;
    struct osal_list_head   list;
} ISP_BE_BUF_NODE_S;

typedef struct hi_ISP_BE_BUF_QUEUE_S
{
    ISP_BE_BUF_NODE_S      *pstNodeBuf;
    struct osal_list_head   stFreeList;
    struct osal_list_head   stBusyList;
    osal_spinlock_t         stSpinLock;
    HI_U32                  u32FreeNum;
    HI_U32                  u32BusyNum;
    HI_U32                  u32MaxNum;

} ISP_BE_BUF_QUEUE_S;

HI_S32 ISP_CreatBeBufQueue(ISP_BE_BUF_QUEUE_S *pstQueue, HI_U32 u32MaxNum);

HI_VOID ISP_DestroyBeBufQueue(ISP_BE_BUF_QUEUE_S *pstQueue);

ISP_BE_BUF_NODE_S *ISP_QueueGetFreeBeBuf(ISP_BE_BUF_QUEUE_S *pstQueue);
ISP_BE_BUF_NODE_S *ISP_QueueGetFreeBeBufTail(ISP_BE_BUF_QUEUE_S *pstQueue);

HI_VOID  ISP_QueuePutBusyBeBuf(ISP_BE_BUF_QUEUE_S *pstQueue, ISP_BE_BUF_NODE_S  *pstNode);

ISP_BE_BUF_NODE_S *ISP_QueueGetBusyBeBuf(ISP_BE_BUF_QUEUE_S *pstQueue);

ISP_BE_BUF_NODE_S *ISP_QueueQueryBusyBeBuf(ISP_BE_BUF_QUEUE_S *pstQueue);

HI_VOID ISP_QueueDelBusyBeBuf(ISP_BE_BUF_QUEUE_S *pstQueue, ISP_BE_BUF_NODE_S  *pstNode);

HI_VOID ISP_QueuePutFreeBeBuf(ISP_BE_BUF_QUEUE_S *pstQueue, ISP_BE_BUF_NODE_S  *pstNode);

HI_U32 ISP_QueueGetFreeNum(ISP_BE_BUF_QUEUE_S *pstQueue);

HI_U32 ISP_QueueGetBusyNum(ISP_BE_BUF_QUEUE_S *pstQueue);



#endif /* End of #ifndef __ISP_LIST_H__ */
