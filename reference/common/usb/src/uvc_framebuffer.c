/**
 * @file      uvc_framebuffer.c
 * @brief     uvc framebuffer interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/22
 * @version   1.0

 */

#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include "uvc_framebuffer.h"

#ifdef __cplusplus
#if __cplusplus
 extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** uvc framebuf context */
typedef struct tagUVC_FRAMEBUF_CONTEXT_S
{
    UVC_FRAMEBUF_QUEUES_S stQueues;
    UVC_FRAME_NODE_S* pstNode;
    HI_U32 u32NodeBufSize;
    HI_S32 s32NodeCnt;
} UVC_FRAMEBUF_CONTEXT_S;
static UVC_FRAMEBUF_CONTEXT_S s_stUVCFrameBufCtx;


static HI_S32 UVC_FRAMEBUF_CreateQueueNode(UVC_FRAME_QUEUE_S* pstQueue, UVC_FRAMEBUF_CONTEXT_S* pstCtx)
{
    HI_U32 u32PageSize = getpagesize();
    HI_U32 u32NodeBufSize = HI_APPCOMM_ALIGN(pstCtx->u32NodeBufSize, u32PageSize);
    MLOGD("PageSize[%u] NodeBufSize[%u] NodeCnt[%d]\n", u32PageSize, u32NodeBufSize, pstCtx->s32NodeCnt);

    HI_S32 s32Idx = 0;
    UVC_FRAME_NODE_S* pstNode = NULL;
    HI_MUTEX_LOCK(pstQueue->MutexLock);
    for (s32Idx = 0; s32Idx < pstCtx->s32NodeCnt; ++s32Idx)
    {
        pstNode = &pstCtx->pstNode[s32Idx];
        if (posix_memalign((HI_VOID**)&pstNode->pu8Mem, u32PageSize, u32NodeBufSize))
        {
            HI_MUTEX_UNLOCK(pstQueue->MutexLock);
            MLOGE("pu8Mem allocate failed\n");
            return HI_FAILURE;
        }
        pstNode->u32BufLen = u32NodeBufSize;
        pstNode->u32used = 0;
        pstNode->s32Idx = s32Idx;
        MLOGD("NodeIdx[%d] MemAddr[%p]\n", s32Idx, pstNode->pu8Mem);

        /*put it to the list*/
        HI_List_Add_Tail(&(pstNode->stList), &(pstQueue->stList));
    }
    HI_MUTEX_UNLOCK(pstQueue->MutexLock);
    return HI_SUCCESS;
}

static HI_S32 UVC_FRAMEBUF_DestroyQueueNode(UVC_FRAME_QUEUE_S* pstQueue)
{
    HI_List_Head_S* pstList = NULL;
    UVC_FRAME_NODE_S* pstNode = NULL;
    HI_MUTEX_LOCK(pstQueue->MutexLock);
    HI_List_For_Each(pstList, &(pstQueue->stList))
    {
        pstNode = HI_LIST_ENTRY(pstList, UVC_FRAME_NODE_S, stList);
        MLOGD("NodeIdx[%d] MemAddr[%p]\n", pstNode->s32Idx, pstNode->pu8Mem);
        HI_List_Del(pstList);
        HI_APPCOMM_SAFE_FREE(pstNode->pu8Mem);

        pstList = &pstQueue->stList;
    }
    HI_MUTEX_UNLOCK(pstQueue->MutexLock);
    return HI_SUCCESS;
}

static HI_S32 UVC_FRAMEBUF_InitQueue(UVC_FRAME_QUEUE_S* pstQueue, const HI_CHAR* pszName)
{
    HI_LIST_INIT_HEAD_PTR(&pstQueue->stList);
    snprintf(pstQueue->szName, HI_APPCOMM_COMM_STR_LEN, "%s", pszName);
    HI_MUTEX_INIT_LOCK(pstQueue->MutexLock);
    HI_COND_INIT(pstQueue->CondWait);
    return HI_SUCCESS;
}

static HI_S32 UVC_FRAMEBUF_DeinitQueue(UVC_FRAME_QUEUE_S* pstQueue)
{
    HI_COND_DESTROY(pstQueue->CondWait);
    HI_MUTEX_DESTROY(pstQueue->MutexLock);
    return HI_SUCCESS;
}


HI_S32 UVC_FRAMEBUF_Create(HI_U32 u32BufSize, HI_S32 s32Cnt)
{
    HI_APPCOMM_CHECK_EXPR(s32Cnt > 0, HI_FAILURE);
    MLOGD("BufSize[%u] Cnt[%d]\n", u32BufSize, s32Cnt);

    /* allocate frame node memory */
    s_stUVCFrameBufCtx.pstNode = (UVC_FRAME_NODE_S*)malloc(sizeof(UVC_FRAME_NODE_S) * s32Cnt);
    HI_APPCOMM_CHECK_POINTER(s_stUVCFrameBufCtx.pstNode, HI_FAILURE);

    /* record buffer node size and count */
    s_stUVCFrameBufCtx.u32NodeBufSize = u32BufSize;
    s_stUVCFrameBufCtx.s32NodeCnt = s32Cnt;

    /* init buffer queue */
    UVC_FRAMEBUF_InitQueue(&s_stUVCFrameBufCtx.stQueues.stIdle, "Idle");
    UVC_FRAMEBUF_InitQueue(&s_stUVCFrameBufCtx.stQueues.stReady, "Ready");
    UVC_FRAMEBUF_CreateQueueNode(&s_stUVCFrameBufCtx.stQueues.stIdle, &s_stUVCFrameBufCtx);
    return HI_SUCCESS;
}

HI_VOID UVC_FRAMEBUF_Destroy(HI_VOID)
{
    UVC_FRAMEBUF_DestroyQueueNode(&s_stUVCFrameBufCtx.stQueues.stReady);
    UVC_FRAMEBUF_DestroyQueueNode(&s_stUVCFrameBufCtx.stQueues.stIdle);
    UVC_FRAMEBUF_DeinitQueue(&s_stUVCFrameBufCtx.stQueues.stReady);
    UVC_FRAMEBUF_DeinitQueue(&s_stUVCFrameBufCtx.stQueues.stIdle);
    HI_APPCOMM_SAFE_FREE(s_stUVCFrameBufCtx.pstNode);
}

HI_S32 UVC_FRAMEBUF_GetNodeFromQueue(UVC_FRAME_QUEUE_S* pstQueue, UVC_FRAME_NODE_S** ppstNode)
{
    HI_APPCOMM_CHECK_POINTER(pstQueue, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ppstNode, HI_FAILURE);
    *ppstNode = NULL;

    HI_List_Head_S* pstList = NULL;
    HI_MUTEX_LOCK(pstQueue->MutexLock);
    HI_List_For_Each(pstList, &(pstQueue->stList))
    {
        *ppstNode = HI_LIST_ENTRY(pstList, UVC_FRAME_NODE_S, stList);
        //MLOGD(YELLOW"Queue[%s] NodeIdx[%d] MemAddr[%p]"NONE"\n", pstQueue->szName, (*ppstNode)->s32Idx, (*ppstNode)->pu8Mem);
        HI_List_Del(pstList);
        break;
    }
    HI_MUTEX_UNLOCK(pstQueue->MutexLock);
    if (!(*ppstNode))
    {
        //MLOGD("Queue[%s] failed\n", pstQueue->szName);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 UVC_FRAMEBUF_PutNodeToQueue(UVC_FRAME_QUEUE_S* pstQueue, UVC_FRAME_NODE_S* pstNode)
{
    HI_APPCOMM_CHECK_POINTER(pstQueue, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pstNode, HI_FAILURE);

    //MLOGD(GREEN"Queue[%s] NodeIdx[%d] MemAddr[%p]"NONE"\n", pstQueue->szName, pstNode->s32Idx, pstNode->pu8Mem);
    HI_MUTEX_LOCK(pstQueue->MutexLock);
    HI_List_Add_Tail(&(pstNode->stList), &(pstQueue->stList));
    if (0 == strncmp(pstQueue->szName, s_stUVCFrameBufCtx.stQueues.stReady.szName, HI_APPCOMM_COMM_STR_LEN))
    {
        HI_COND_SIGNAL(pstQueue->CondWait);
    }
    HI_MUTEX_UNLOCK(pstQueue->MutexLock);

    return HI_SUCCESS;
}

UVC_FRAMEBUF_QUEUES_S* UVC_FRAMEBUF_GetQueues(HI_VOID)
{
    return &s_stUVCFrameBufCtx.stQueues;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

