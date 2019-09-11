/**
 * @file      uvc_framebuffer.h
 * @brief     uvc framebuffer interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/22
 * @version   1.0

 */

#ifndef __UVC_FRAMEBUFFER_H__
#define __UVC_FRAMEBUFFER_H__

#include <string.h>
#include <pthread.h>
#include "hi_appcomm.h"
#include "hi_list.h"
#include "hi_uvc.h"

#ifdef __cplusplus
#if __cplusplus
 extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** frame node */
typedef struct tagUVC_FRAME_NODE_S
{
    HI_List_Head_S stList;
    HI_U32  u32BufLen;
    HI_U32  u32used;
    HI_U8*  pu8Mem;
    HI_S32  s32Idx;
    HI_U32  offset;
} UVC_FRAME_NODE_S;

/** frame queue */
typedef struct tagUVC_FRAME_QUEUE_S
{
    HI_List_Head_S stList;
    HI_CHAR szName[HI_APPCOMM_COMM_STR_LEN];
    pthread_mutex_t MutexLock;
    pthread_cond_t  CondWait;
} UVC_FRAME_QUEUE_S;

/** frame cache */
typedef struct tagUVC_FRAMEBUF_QUEUES_S
{
    UVC_FRAME_QUEUE_S stIdle;
    UVC_FRAME_QUEUE_S stReady;
} UVC_FRAMEBUF_QUEUES_S;


/**
 * @brief     create frame buffer, allocate memory
 * @param[in] u32BufSize : buffer node size
 * @param[in] s32Cnt : buffer node count
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/22
 */
HI_S32  UVC_FRAMEBUF_Create(HI_U32 u32BufSize, HI_S32 s32Cnt);

/**
 * @brief     destroy frame buffer, free memory
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/22
 */
HI_VOID UVC_FRAMEBUF_Destroy(HI_VOID);

/**
 * @brief     get frame node from specified queue
 * @param[in] pstQueue : queue instance
 * @param[out]ppstNode : frame node
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/22
 */
HI_S32  UVC_FRAMEBUF_GetNodeFromQueue(UVC_FRAME_QUEUE_S* pstQueue, UVC_FRAME_NODE_S** ppstNode);

/**
 * @brief     put frame node to specified queue
 * @param[in] pstQueue : queue instance
 * @param[in] pstNode : frame node instance
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/22
 */
HI_S32  UVC_FRAMEBUF_PutNodeToQueue(UVC_FRAME_QUEUE_S* pstQueue, UVC_FRAME_NODE_S* pstNode);

/**
 * @brief     get frame buffer queues
 * @return    frame buffer queues instance
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/22
 */
UVC_FRAMEBUF_QUEUES_S* UVC_FRAMEBUF_GetQueues(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __UVC_FRAMEBUFFER_H__ */

