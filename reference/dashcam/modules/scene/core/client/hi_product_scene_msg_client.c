/**
* @file    hi_hal_screen.c
* @brief   hal screen implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/
#include <string.h>

#include "hi_product_scene.h"
#include "hi_product_scene_msg_client.h"
#include "hi_product_scene_msg_define.h"
#include "hi_appcomm_msg_client.h"
#include "hi_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCENE */
/** @{ */  /** <!-- [SCENE] */
/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "SCENE_CLIENT"

static HI_S32 MSG_PDT_SCENE_ReduceFrameCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_EVENT_S stEvent;
    memset(&stEvent, '\0', sizeof(stEvent));

    stEvent.EventID = HI_EVENT_PDT_SCENE_FRAMERATE_CHANGE;
    stEvent.arg1 = 60;
    s32Ret = HI_EVTHUB_Publish(&stEvent);
    MLOGI("Event Published. 60\n");

    return s32Ret;
}

static HI_S32 MSG_PDT_SCENE_AddFrameCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_EVENT_S stEvent;
    memset(&stEvent, '\0', sizeof(stEvent));

    stEvent.EventID = HI_EVENT_PDT_SCENE_FRAMERATE_CHANGE;
    stEvent.arg1 = 120;
    s32Ret = HI_EVTHUB_Publish(&stEvent);
    MLOGI("Event Published. 120\n");

    return s32Ret;
}

HI_S32 HI_PDT_SCENE_MSG_CLIENT_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_FRAMERATE_FROM120TO60, MSG_PDT_SCENE_ReduceFrameCallback, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_FRAMERATE_FROM60TO120, MSG_PDT_SCENE_AddFrameCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    return s32Ret;
}
