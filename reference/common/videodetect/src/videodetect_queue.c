#include "videodetect_queue.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct tagVIDEODETECT_QUEUE_S
{
    pthread_mutex_t mutex;
    HI_S32 frontIdx;      /**<front index, means (the newest node index + 1)*/
    HI_S32 rearIdx;       /**<rear index, means the oldest node index*/
    HI_S32 curLen;        /**<current used length*/
    HI_S32 maxLen;        /**<max length*/
    HI_S32 nodeSize;      /**<size of node*/
    HI_VOID** node;       /**<node*/
} VIDEODETECT_QUEUE_S;


HI_HANDLE VIDEODETECT_QUEUE_Create(HI_U32 nodeSize, HI_U32 maxLen)
{
    VIDEODETECT_QUEUE_S* Queue = malloc(sizeof(VIDEODETECT_QUEUE_S));

    if (!Queue)
    {
        return 0;
    }

    HI_S32 i = 0;
    Queue->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    Queue->frontIdx = 0;
    Queue->rearIdx = 0;
    Queue->curLen = 0;
    Queue->maxLen = maxLen;
    Queue->nodeSize = nodeSize;
    Queue->node = (HI_VOID**)malloc(sizeof(HI_VOID*) * maxLen);

    for (i = 0; i < Queue->maxLen; i++)
    {
        Queue->node[i] = (HI_VOID*)malloc(nodeSize);
    }

    return (HI_HANDLE)Queue;
}

HI_VOID VIDEODETECT_QUEUE_Destroy(HI_HANDLE queueHdl, HI_VOID* node, HI_U32* bufferCnt)
{
    if (queueHdl == 0)
    {
        MLOGE("bufferHdl is NULL!\n");
        return;
    }

    VIDEODETECT_QUEUE_S* queue = (VIDEODETECT_QUEUE_S*)queueHdl;
    HI_MUTEX_LOCK(queue->mutex);
    HI_S32 i = 0;
    HI_S32 s32NewestIndex = 0;

    HI_U32 u32BufferCnt = 0;

    if(queue->curLen != 0)
    {
        s32NewestIndex = queue->frontIdx;
        for (i = 0; i < queue->curLen; i++)
        {
            if (s32NewestIndex > 0)
            {
                s32NewestIndex = s32NewestIndex - 1;
            }
            else if (s32NewestIndex == 0)
            {
                s32NewestIndex = queue->maxLen - 1;
            }

            memcpy((node+i * queue->nodeSize), queue->node[s32NewestIndex], queue->nodeSize);
            u32BufferCnt++;
        }
    }

    *bufferCnt = u32BufferCnt;
    for (i = 0; i < queue->maxLen; i++)
    {
        HI_APPCOMM_SAFE_FREE(queue->node[i]);
    }

    HI_APPCOMM_SAFE_FREE(queue->node);
    HI_MUTEX_UNLOCK(queue->mutex);
    HI_MUTEX_DESTROY(queue->mutex);
    HI_APPCOMM_SAFE_FREE(queue);
}

HI_VOID VIDEODETECT_QUEUE_Clear(HI_HANDLE queueHdl, HI_VOID* node, HI_U32* bufferCnt)
{
    if (queueHdl == 0)
    {
        MLOGE("bufferHdl is NULL!\n");
        return;
    }

    VIDEODETECT_QUEUE_S* queue = (VIDEODETECT_QUEUE_S*)queueHdl;
    HI_MUTEX_LOCK(queue->mutex);
    HI_S32 i = 0;
    HI_S32 s32NewestIndex = 0;

    HI_U32 u32BufferCnt = 0;

    if(queue->curLen != 0)
    {
        s32NewestIndex = queue->frontIdx;
        for (i = 0; i < queue->curLen; i++)
        {
            if (s32NewestIndex > 0)
            {
                s32NewestIndex = s32NewestIndex - 1;
            }
            else if (s32NewestIndex == 0)
            {
                s32NewestIndex = queue->maxLen - 1;
            }

            memcpy((node+i * queue->nodeSize), queue->node[s32NewestIndex], queue->nodeSize);
            u32BufferCnt++;
        }
    }

    *bufferCnt = u32BufferCnt;
    queue->curLen = 0;
    queue->frontIdx = 0;
    queue->rearIdx = 0;
    HI_MUTEX_UNLOCK(queue->mutex);
}


HI_S32 VIDEODETECT_QUEUE_Push(HI_HANDLE queueHdl, HI_VOID* node)
{
    if (queueHdl == 0 || HI_NULL == node)
    {
        MLOGE("queueHdl or node is NULL !\n");
        return HI_EINVAL;
    }

    VIDEODETECT_QUEUE_S* queue = (VIDEODETECT_QUEUE_S*)queueHdl;
    HI_MUTEX_LOCK(queue->mutex);

    /**if full, then newest will replace the oldest*/
    if (queue->curLen > queue->maxLen)
    {
        HI_MUTEX_UNLOCK(queue->mutex);
        MLOGE("out of maxlen!\n");
        return HI_EINVAL;
    }
    else if (queue->curLen < queue->maxLen)
    {
        memcpy(queue->node[queue->frontIdx], node, queue->nodeSize);
        queue->curLen++;
        queue->frontIdx = (queue->frontIdx + 1) % queue->maxLen;
    }
    else if (queue->curLen == queue->maxLen)
    {
        memcpy(queue->node[queue->rearIdx], node, queue->nodeSize);
        queue->rearIdx = (queue->rearIdx + 1) % queue->maxLen;
        queue->frontIdx = queue->rearIdx;
    }
    HI_MUTEX_UNLOCK(queue->mutex);
    return HI_SUCCESS;
}

HI_S32 VIDEODETECT_QUEUE_Pop(HI_HANDLE queueHdl, HI_VOID* node, HI_BOOL* popNode)
{
    if (queueHdl == 0 || HI_NULL == node || HI_NULL == popNode)
    {
        MLOGE("queueHdl or node or popNode is NULL!\n");
        return HI_EINVAL;
    }

    VIDEODETECT_QUEUE_S* queue = (VIDEODETECT_QUEUE_S*)queueHdl;
    HI_MUTEX_LOCK(queue->mutex);

    /**only full can be pop one node*/
    if (queue->curLen == 0)
    {
        *popNode = HI_FALSE;
        HI_MUTEX_UNLOCK(queue->mutex);
        MLOGW("queue is empity!\n");
        return HI_EEMPTY;
    }
    else if (queue->curLen != queue->maxLen)
    {
        *popNode = HI_FALSE;
        HI_MUTEX_UNLOCK(queue->mutex);
        //MLOGI("queue is not full, don't pop any node!\n");

        return HI_SUCCESS;
    }
    else if (queue->curLen == queue->maxLen)
    {
        memcpy(node, queue->node[queue->rearIdx], queue->nodeSize);
        queue->rearIdx = (queue->rearIdx + 1) % queue->maxLen;
        queue->curLen--;
    }
    *popNode = HI_TRUE;
    HI_MUTEX_UNLOCK(queue->mutex);
    return HI_SUCCESS;
}


HI_S32 VIDEODETECT_QUEUE_Read(HI_HANDLE queueHdl, HI_U32 bufferCnt, HI_VOID* node, HI_BOOL* readNode)
{
    if (queueHdl == 0 || HI_NULL == node || HI_NULL == readNode )
    {
        MLOGE("queueHdl or node or popNode is NULL!\n");
        return HI_EINVAL;
    }

    VIDEODETECT_QUEUE_S* queue = (VIDEODETECT_QUEUE_S*)queueHdl;
    HI_S32 i = 0;
    HI_S32 s32NewestIndex = 0;
    if (bufferCnt <= 0 || bufferCnt > queue->maxLen)
    {
        MLOGE("bufferCnt is not leagal!\n");
        return HI_EINVAL;
    }

    HI_MUTEX_LOCK(queue->mutex);

    if (queue->curLen == 0)
    {
        HI_MUTEX_UNLOCK(queue->mutex);
        MLOGW("queue is empity!\n");
        return HI_EEMPTY;
    }

    /**if bufferCnt bigger then currentLen, return nothing*/
    if (bufferCnt > queue->curLen)
    {
        *readNode = HI_FALSE;
        HI_MUTEX_UNLOCK(queue->mutex);
        return HI_SUCCESS;
    }

    s32NewestIndex = queue->frontIdx;
    for (i = 0; i < bufferCnt; i++)
    {
        if (s32NewestIndex > 0)
        {
            s32NewestIndex = s32NewestIndex - 1;
        }
        else if (s32NewestIndex == 0)
        {
            s32NewestIndex = queue->maxLen - 1;
        }

        memcpy((node+i * queue->nodeSize), queue->node[s32NewestIndex], queue->nodeSize);
    }

    *readNode = HI_TRUE;
    HI_MUTEX_UNLOCK(queue->mutex);

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

