#ifndef __STG_MESSAGE_H__
#define __STG_MESSAGE_H__

#include "hi_type.h"

typedef HI_VOID (*HI_MSG_CALLBACK_FN)(HI_VOID* pPrivate);

typedef struct tagSTG_MSG_INFO_S
{
    HI_S32 s32What;
    HI_U32 u32Arg1;
    HI_U32 u32Arg2;
    HI_VOID* pMsgData;
    HI_U32 u32MsgDataLen;

    //below is private info,do not change outside
    HI_BOOL bNeedReply;
    HI_MSG_CALLBACK_FN pfnCallback;
    HI_VOID* pPrivate;
}STG_MSG_INFO_S;

#endif //__STG_MESSAGE_H__