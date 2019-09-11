#ifndef __HI_QUEUE_H__
#define __HI_QUEUE_H__

#include "hi_appcomm.h"

HI_HANDLE HI_QUEUE_Create(HI_U32 nodeSize,HI_U32 maxLen);

HI_VOID HI_QUEUE_Destroy(HI_HANDLE queueHdl);

HI_VOID HI_QUEUE_Clear(HI_HANDLE queueHdl);

HI_S32 HI_QUEUE_GetLen(HI_HANDLE queueHdl);

HI_S32 HI_QUEUE_Push(HI_HANDLE queueHdl,const HI_VOID* node);

HI_S32 HI_QUEUE_Pop(HI_HANDLE queueHdl,HI_VOID* node);

#endif // __HI_QUEUE_H__
