/**
 * @file    hi_osd_server.c
 * @brief   osd module server interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 * @version   1.0

 */
#include "hi_osd.h"
#include "hi_appcomm_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "OSD"

static HI_S32 OSD_MSG_SetAttr(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,
                                HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 s32OsdIdx = pstPrivData->as32PrivData[1];
    return HI_OSD_SetAttr(s32OsdIdx, pvRequest);
}

static HI_S32 OSD_MSG_GetAttr(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,
                                HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 s32OsdIdx = pstPrivData->as32PrivData[1];
    HI_OSD_ATTR_S* pstAttr = (HI_OSD_ATTR_S*)malloc(sizeof(HI_OSD_ATTR_S));
    *ppvResponse = pstAttr;
    *pu32RespLen = sizeof(HI_OSD_ATTR_S);
    return HI_OSD_GetAttr(s32OsdIdx, pstAttr);
}

static HI_S32 OSD_MSG_Start(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,
                                HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 s32OsdIdx = pstPrivData->as32PrivData[1];
    return HI_OSD_Start(s32OsdIdx);
}

static HI_S32 OSD_MSG_Stop(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,
                                HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 s32OsdIdx = pstPrivData->as32PrivData[1];
    return HI_OSD_Stop(s32OsdIdx);
}

static HI_S32 OSD_MSG_Batch(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,
                                HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 s32Batch = pstPrivData->as32PrivData[1];
    HI_BOOL bShow = pstPrivData->as32PrivData[2];
    return HI_OSD_Batch(s32Batch, bShow);
}

HI_S32 OSD_MSG_Register(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32MsgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_OSD, OSD_CMD_SET_ATTR);
    s32Ret |= HI_MSG_RegisterMsgProc(s32MsgID, OSD_MSG_SetAttr, NULL);

    s32MsgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_OSD, OSD_CMD_GET_ATTR);
    s32Ret |= HI_MSG_RegisterMsgProc(s32MsgID, OSD_MSG_GetAttr, NULL);

    s32MsgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_OSD, OSD_CMD_START);
    s32Ret |= HI_MSG_RegisterMsgProc(s32MsgID, OSD_MSG_Start, NULL);

    s32MsgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_OSD, OSD_CMD_STOP);
    s32Ret |= HI_MSG_RegisterMsgProc(s32MsgID, OSD_MSG_Stop, NULL);

    s32MsgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_OSD, OSD_CMD_BATCH);
    s32Ret |= HI_MSG_RegisterMsgProc(s32MsgID, OSD_MSG_Batch, NULL);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

