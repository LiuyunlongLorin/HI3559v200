#include "hi_queue.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct tagQUEUE_S
{
    pthread_mutex_t mutex;
    HI_S32 frontIdx;      //队头索引
    HI_S32 rearIdx;       //队尾索引
    HI_S32 curLen;    //当前已使用容量
    HI_S32 maxLen;    //最大容量
    HI_S32 nodeSize;
    HI_VOID** node;    //节点指针
} QUEUE_S;


HI_HANDLE HI_QUEUE_Create(HI_U32 nodeSize, HI_U32 maxLen)
{
    QUEUE_S* queue = malloc(sizeof(QUEUE_S));

    if (!queue)
    {
        return 0;
    }

    HI_S32 i = 0;
    queue->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    queue->frontIdx = 0;
    queue->rearIdx = 0;
    queue->curLen = 0;
    queue->maxLen = maxLen;
    queue->nodeSize = nodeSize;
    queue->node = (HI_VOID**)malloc(sizeof(HI_VOID*) * maxLen);

    for (i = 0; i < queue->maxLen; i++)
    {
        queue->node[i] = (HI_VOID*)malloc(nodeSize);
    }

    return (HI_HANDLE)queue;
}

HI_VOID HI_QUEUE_Destroy(HI_HANDLE queueHdl)
{
    if (queueHdl == 0)
    {
        MLOGE("queueHdl is NULL!\n");
        return;
    }

    QUEUE_S* queue = (QUEUE_S*)queueHdl;
    HI_MUTEX_LOCK(queue->mutex);
    HI_S32 i = 0;

    for (i = 0; i < queue->maxLen; i++)
    {
        HI_APPCOMM_SAFE_FREE(queue->node[i]);
    }

    HI_APPCOMM_SAFE_FREE(queue->node);
    HI_MUTEX_UNLOCK(queue->mutex);
    HI_MUTEX_DESTROY(queue->mutex);
    HI_APPCOMM_SAFE_FREE(queue);
}

HI_VOID HI_QUEUE_Clear(HI_HANDLE queueHdl)
{
    if (queueHdl == 0)
    {
        MLOGE("queueHdl is NULL!\n");
        return;
    }

    QUEUE_S* queue = (QUEUE_S*)queueHdl;
    HI_MUTEX_LOCK(queue->mutex);
    queue->curLen = 0;
    queue->frontIdx = 0;
    queue->rearIdx = 0;
    HI_MUTEX_UNLOCK(queue->mutex);
}

HI_S32 HI_QUEUE_GetLen(HI_HANDLE queueHdl)
{
    if (queueHdl == 0)
    {
        MLOGE("queueHdl is NULL!\n");
        return -1;
    }

    QUEUE_S* queue = (QUEUE_S*)queueHdl;
    return (HI_S32)queue->curLen;
}

HI_S32 HI_QUEUE_Push(HI_HANDLE queueHdl, const HI_VOID* node)
{
    if (queueHdl == 0)
    {
        MLOGE("queueHdl is NULL!\n");
        return HI_EINVAL;
    }

    QUEUE_S* queue = (QUEUE_S*)queueHdl;
    HI_MUTEX_LOCK(queue->mutex);

    if (queue->curLen >= queue->maxLen)
    {
        HI_MUTEX_UNLOCK(queue->mutex);
        MLOGE("queue is full!\n");
        return HI_EFULL;
    }

    if (node)
    {
        memcpy(queue->node[queue->rearIdx], node, queue->nodeSize);
    }

    queue->curLen++;
    queue->rearIdx = (queue->rearIdx + 1) % queue->maxLen;
    HI_MUTEX_UNLOCK(queue->mutex);
    return HI_SUCCESS;
}

HI_S32 HI_QUEUE_Pop(HI_HANDLE queueHdl, HI_VOID* node)
{
    if (queueHdl == 0)
    {
        MLOGE("queueHdl is NULL!\n");
        return HI_EINVAL;
    }

    QUEUE_S* queue = (QUEUE_S*)queueHdl;
    HI_MUTEX_LOCK(queue->mutex);

    if (queue->curLen == 0)
    {
        HI_MUTEX_UNLOCK(queue->mutex);
        MLOGE("queue is empity!\n");
        return HI_EEMPTY;
    }

    if (node)
    {
        memcpy(node, queue->node[queue->frontIdx], queue->nodeSize);
    }

    queue->curLen--;
    queue->frontIdx = (queue->frontIdx + 1) % queue->maxLen;
    HI_MUTEX_UNLOCK(queue->mutex);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
