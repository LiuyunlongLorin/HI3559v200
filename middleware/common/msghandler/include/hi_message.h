/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_messagehandler.h
* @brief     HFSM module header file
* @author    HiMobileCam middleware develop team
* @date      2017.11.30
*/
#ifndef MESSAGE_H
#define MESSAGE_H
#include "hi_mw_type.h"

#define MESSAGE_PAYLOAD_LEN        (512)
typedef struct hi_MESSAGE_S{
    HI_S32 what;
    HI_S32 arg1;
    HI_S32 arg2;
    HI_S32 s32Result;
    HI_U64 u64CreateTime;
    HI_CHAR aszPayload[MESSAGE_PAYLOAD_LEN];
}HI_MESSAGE_S;

#endif
