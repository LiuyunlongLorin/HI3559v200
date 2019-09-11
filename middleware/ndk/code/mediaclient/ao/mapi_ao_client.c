/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_ao_client.c
 * @brief   NDK ao client functions
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */
#include <stdio.h>
#include "hi_type.h"
#include "hi_mapi_ao_define.h"
#include "hi_ipcmsg.h"
#include "msg_ao.h"
#include "msg_define.h"
#include "mapi_ao_inner.h"
#include "msg_wrapper.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 HI_MAPI_AO_Init(HI_HANDLE AoHdl, const HI_MAPI_AO_ATTR_S *pstAoAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_MAPI_AO_ATTR_S stAoAttr;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_NULL_PTR(pstAoAttr);
    CHECK_MAPI_AO_INIT(MAPI_GetMediaInitStatus());

    /* send ipcmsg  */
    u32ModFd = MODFD(HI_MAPI_MOD_AO, AoHdl, 0);
    memcpy(&stAoAttr, pstAoAttr, sizeof(HI_MAPI_AO_ATTR_S));
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AO_INIT, &stAoAttr, sizeof(HI_MAPI_AO_ATTR_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao init failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AO_Deinit(HI_HANDLE AoHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_INIT(MAPI_GetMediaInitStatus());

    /* send ipcmsg  */
    u32ModFd = MODFD(HI_MAPI_MOD_AO, AoHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AO_DEINIT, NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao deinit failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AO_Start(HI_HANDLE AoHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_INIT(MAPI_GetMediaInitStatus());

    /* send ipcmsg  */
    u32ModFd = MODFD(HI_MAPI_MOD_AO, AoHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AO_START, NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao start failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AO_Stop(HI_HANDLE AoHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_INIT(MAPI_GetMediaInitStatus());

    /* send ipcmsg  */
    u32ModFd = MODFD(HI_MAPI_MOD_AO, AoHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AO_STOP, NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao stop failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AO_SetVolume(HI_HANDLE AoHdl, HI_S32 s32AudioGain)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_INIT(MAPI_GetMediaInitStatus());

    /* send ipcmsg  */
    u32ModFd = MODFD(HI_MAPI_MOD_AO, AoHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AO_SET_VOLUME, &s32AudioGain, sizeof(HI_S32), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao set vol failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AO_GetVolume(HI_HANDLE AoHdl, HI_S32 *ps32AudioGain)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_S32 s32Gain = 0;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_NULL_PTR(ps32AudioGain);
    CHECK_MAPI_AO_INIT(MAPI_GetMediaInitStatus());

    /* send ipcmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_AO, AoHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AO_GET_VOLUME, &s32Gain, sizeof(HI_S32), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao get vol failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }
    *ps32AudioGain = s32Gain;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AO_Mute(HI_HANDLE AoHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_INIT(MAPI_GetMediaInitStatus());

    /* send ipcmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_AO, AoHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AO_MUTE, NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao mute failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AO_Unmute(HI_HANDLE AoHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_INIT(MAPI_GetMediaInitStatus());

    /* send ipcmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_AO, AoHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AO_UNMUTE, NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao unmute failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AO_SendFrame(HI_HANDLE AoHdl, const AUDIO_FRAME_S *pstAudioFrame, HI_U32 u32Timeout)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MSG_AO_FRAME_S stMsgFrm;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_NULL_PTR(pstAudioFrame);
    CHECK_MAPI_AO_INIT(MAPI_GetMediaInitStatus());

    /* send ipcmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_AO, AoHdl, 0);
    memcpy(&(stMsgFrm.stAFrm), pstAudioFrame, sizeof(AUDIO_FRAME_S));
    stMsgFrm.u32Timeout = u32Timeout;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AO_SEND_FRAME, &stMsgFrm, sizeof(MSG_AO_FRAME_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao send frame failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

HI_S32 HI_MAPI_AO_SendSysFrame(HI_HANDLE AoHdl, const AUDIO_FRAME_S *pstAudioFrame, HI_U32 u32Timeout)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MSG_AO_FRAME_S stMsgFrm;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_NULL_PTR(pstAudioFrame);
    CHECK_MAPI_AO_INIT(MAPI_GetMediaInitStatus());

    /* send ipcmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_AO, AoHdl, 0);
    memcpy(&(stMsgFrm.stAFrm), pstAudioFrame, sizeof(AUDIO_FRAME_S));
    stMsgFrm.u32Timeout = u32Timeout;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AO_SEND_SYS_FRAME, &stMsgFrm, sizeof(MSG_AO_FRAME_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao send frame failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
