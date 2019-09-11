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
#include "hi_product_scene_msg_server.h"
#include "hi_product_scene_msg_define.h"
#include "hi_appcomm_msg_server.h"

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
#define HI_MODULE "SCENE_SERVER"

static HI_S32 MSG_PDT_SCENE_InitCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PDT_SCENE_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 MSG_PDT_SCENE_SetSceneModeCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_PDT_SCENE_EINTER);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_MODE_S* pstSceneMode = (HI_PDT_SCENE_MODE_S*)pvRequest;

    s32Ret = HI_PDT_SCENE_SetSceneMode(pstSceneMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 MSG_PDT_SCENE_PauseCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_PDT_SCENE_EINTER);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_BOOL* pbEnable = (HI_BOOL*) pvRequest;

    s32Ret =  HI_PDT_SCENE_Pause(*pbEnable);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 MSG_PDT_SCENE_DeinitCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret =  HI_PDT_SCENE_Deinit();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 MSG_PDT_SCENE_GetSceneModeCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pu32RespLen, HI_PDT_SCENE_EINTER);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_MODE_S* pstSceneMode = (HI_PDT_SCENE_MODE_S*)malloc(sizeof(HI_PDT_SCENE_MODE_S));
    if(pstSceneMode == NULL)
    {
        MLOGE("malloc memory failed\n");
        return HI_FAILURE;
    }

    s32Ret = HI_PDT_SCENE_GetSceneMode(pstSceneMode);
    if (s32Ret != HI_SUCCESS)
    {
        HI_APPCOMM_SAFE_FREE(pstSceneMode);
        MLOGE("HI_PDT_SCENE_GetSceneMode failed\n");
        return HI_FAILURE;
    }

    *pu32RespLen = sizeof(HI_PDT_SCENE_MODE_S);
    *ppvResponse = pstSceneMode;

    return HI_SUCCESS;
}

static HI_S32 MSG_PDT_SCENE_SetDISCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_PDT_SCENE_EINTER);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_ENABLE_S* pstDisState = (HI_PDT_SCENE_PIPE_ENABLE_S*) pvRequest;

    s32Ret = HI_PDT_SCENE_SetDIS(pstDisState->VcapPipeHdl, pstDisState->bEnable);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 MSG_PDT_SCENE_SetISOCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_PDT_SCENE_EINTER);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_NUMBER_S* pstISO = (HI_PDT_SCENE_PIPE_NUMBER_S*) pvRequest;

    s32Ret = HI_PDT_SCENE_SetISO(pstISO->VcapPipeHdl, pstISO->u32Number);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 MSG_PDT_SCENE_SetExpTimeCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_PDT_SCENE_EINTER);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_NUMBER_S* pstExpTime = (HI_PDT_SCENE_PIPE_NUMBER_S*) pvRequest;

    s32Ret = HI_PDT_SCENE_SetExpTime(pstExpTime->VcapPipeHdl, pstExpTime->u32Number);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 MSG_PDT_SCENE_SetEVCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_PDT_SCENE_EINTER);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_EV_S* pstEV = (HI_PDT_SCENE_PIPE_EV_S*) pvRequest;

    s32Ret = HI_PDT_SCENE_SetEV(pstEV->VcapPipeHdl, pstEV->enEV);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 MSG_PDT_SCENE_SetWBCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_PDT_SCENE_EINTER);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_NUMBER_S* pstWB = (HI_PDT_SCENE_PIPE_NUMBER_S*) pvRequest;

    s32Ret = HI_PDT_SCENE_SetWB(pstWB->VcapPipeHdl, pstWB->u32Number);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 MSG_PDT_SCENE_SetMETRYCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_PDT_SCENE_EINTER);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_METRY_S* pstMetryParam = (HI_PDT_SCENE_PIPE_METRY_S*) pvRequest;

    s32Ret = HI_PDT_SCENE_SetMetry(pstMetryParam->VcapPipeHdl, &pstMetryParam->stMetry);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_MSG_SERVER_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    MLOGD("Param input is OK!\n");

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_INIT, MSG_PDT_SCENE_InitCallback, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_SETSCENEMODE, MSG_PDT_SCENE_SetSceneModeCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_PAUSE, MSG_PDT_SCENE_PauseCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_DEINIT, MSG_PDT_SCENE_DeinitCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_GETSCENEMODE, MSG_PDT_SCENE_GetSceneModeCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_SETDIS, MSG_PDT_SCENE_SetDISCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_SETISO, MSG_PDT_SCENE_SetISOCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_SETEXPTIME, MSG_PDT_SCENE_SetExpTimeCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_SETEV, MSG_PDT_SCENE_SetEVCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_SETWB, MSG_PDT_SCENE_SetWBCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_PDT_SCENE_SETMETRY, MSG_PDT_SCENE_SetMETRYCallback,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    return s32Ret;
}
