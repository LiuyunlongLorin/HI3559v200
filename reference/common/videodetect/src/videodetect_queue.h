/**
 * @file    videodetect_queue.h
 * @brief   videodetect queue header file
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/2
 * @version   1.0

 */
#include "hi_appcomm.h"
#include "hi_videodetect.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

HI_HANDLE VIDEODETECT_QUEUE_Create(HI_U32 nodeSize, HI_U32 maxLen);

HI_S32 VIDEODETECT_QUEUE_Push(HI_HANDLE queueHdl, HI_VOID* node);

HI_S32 VIDEODETECT_QUEUE_Read(HI_HANDLE queueHdl, HI_U32 bufferCnt, HI_VOID* node, HI_BOOL* readNode);

HI_S32 VIDEODETECT_QUEUE_Pop(HI_HANDLE queueHdl, HI_VOID* node, HI_BOOL* popNode);

HI_VOID VIDEODETECT_QUEUE_Clear(HI_HANDLE queueHdl, HI_VOID* node, HI_U32* bufferCnt);

HI_VOID VIDEODETECT_QUEUE_Destroy(HI_HANDLE queueHdl, HI_VOID* node, HI_U32* bufferCnt);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */
