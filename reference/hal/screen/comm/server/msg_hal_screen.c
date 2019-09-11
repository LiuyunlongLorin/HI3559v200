/**
* @file     msg_hal_screen.c
* @brief   hal screen implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/

#include <string.h>
#include <stdlib.h>

#include "hi_hal_screen.h"
#include "hi_hal_screen_inner.h"
#include "hi_appcomm_msg_server.h"
#include "hi_hal_common.h"
#include "hi_hal_common_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCREEN */
/** @{ */  /** <!-- [SCREEN] */

static HI_S32 MSG_HAL_SCREEN_InitCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_HAL_SCREEN_IDX_E* penScreenIndex = (HI_HAL_SCREEN_IDX_E*)pvRequest;

    s32Ret = HI_HAL_SCREEN_Init(*penScreenIndex);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_HAL_EINTER);

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_GetAttrCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pu32RespLen, HI_HAL_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HI_HAL_SCREEN_IDX_E* penScreenIndex = (HI_HAL_SCREEN_IDX_E*)pvRequest;
    HI_HAL_SCREEN_ATTR_S* pstScreenAttr = (HI_HAL_SCREEN_ATTR_S*)malloc(sizeof(HI_HAL_SCREEN_ATTR_S));

    s32Ret = HI_HAL_SCREEN_GetAttr(*penScreenIndex, pstScreenAttr);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_SCREEN_GetAttr Error");
        HI_APPCOMM_SAFE_FREE(pstScreenAttr);
        return HI_HAL_EINTER;
    }

    *pu32RespLen = sizeof(HI_HAL_SCREEN_ATTR_S);
    *ppvResponse = pstScreenAttr;

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_GetDisplayStateCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pu32RespLen, HI_HAL_EINVAL);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_HAL_SCREEN_IDX_E* penScreenIndex = (HI_HAL_SCREEN_IDX_E*)pvRequest;
    HI_HAL_SCREEN_STATE_E* penScreenState = (HI_HAL_SCREEN_STATE_E*)malloc(sizeof(HI_HAL_SCREEN_STATE_E));

    s32Ret = HI_HAL_SCREEN_GetDisplayState(*penScreenIndex, penScreenState);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_SCREEN_GetDisplayState Error");
        HI_APPCOMM_SAFE_FREE(penScreenState);
        return HI_HAL_EINTER;
    }

    *pu32RespLen = sizeof(HI_HAL_SCREEN_STATE_E);
    *ppvResponse = penScreenState;

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_SetDisplayStateCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HAL_SCREEN_STATE_S* pstIndexDispState = (HAL_SCREEN_STATE_S*)pvRequest;

    s32Ret = HI_HAL_SCREEN_SetDisplayState(pstIndexDispState->enScreenIndex, pstIndexDispState->enState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_HAL_EINTER);

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_GetBackLightStateCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pu32RespLen, HI_HAL_EINVAL);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_HAL_SCREEN_IDX_E* penScreenIndex = (HI_HAL_SCREEN_IDX_E*)pvRequest;
    HI_HAL_SCREEN_STATE_E* penScreenState = (HI_HAL_SCREEN_STATE_E*)malloc(sizeof(HI_HAL_SCREEN_STATE_E));

    s32Ret = HI_HAL_SCREEN_GetBackLightState(*penScreenIndex, penScreenState);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_SCREEN_GetBackLightState Error");
        HI_APPCOMM_SAFE_FREE(penScreenState);
        return HI_HAL_EINTER;
    }

    *pu32RespLen = sizeof(HI_HAL_SCREEN_STATE_E);
    *ppvResponse = penScreenState;

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_SetBackLightStateCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HAL_SCREEN_STATE_S* pstIndexDispState = (HAL_SCREEN_STATE_S*)pvRequest;

    s32Ret = HI_HAL_SCREEN_SetBackLightState(pstIndexDispState->enScreenIndex, pstIndexDispState->enState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_HAL_EINTER);

    return HI_SUCCESS;
}


static HI_S32 MSG_HAL_SCREEN_GetLumaCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pu32RespLen, HI_HAL_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HI_HAL_SCREEN_IDX_E* penScreenIndex = (HI_HAL_SCREEN_IDX_E*)pvRequest;

    HI_U32* pu32Luma = (HI_U32*)malloc(sizeof(HI_U32));

    s32Ret = HI_HAL_SCREEN_GetLuma(*penScreenIndex, pu32Luma);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_SCREEN_GetLuma Error");
        HI_APPCOMM_SAFE_FREE(pu32Luma);
        return HI_HAL_EINTER;
    }

    *pu32RespLen = sizeof(HI_U32);
    *ppvResponse = pu32Luma;

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_SetLumaCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HAL_SCREEN_NUMBER_S* pstIndexNumber = (HAL_SCREEN_NUMBER_S*)pvRequest;

    s32Ret = HI_HAL_SCREEN_SetLuma(pstIndexNumber->enScreenIndex, pstIndexNumber->u32Number);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_HAL_EINTER);

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_GetSaturatureCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pu32RespLen, HI_HAL_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HI_HAL_SCREEN_IDX_E* penScreenIndex = (HI_HAL_SCREEN_IDX_E*)pvRequest;

    HI_U32* pu32Saturature = (HI_U32*)malloc(sizeof(HI_U32));

    s32Ret = HI_HAL_SCREEN_GetSaturature(*penScreenIndex, pu32Saturature);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_SCREEN_GetSaturature Error");
        HI_APPCOMM_SAFE_FREE(pu32Saturature);
        return HI_HAL_EINTER;
    }

    *pu32RespLen = sizeof(HI_U32);
    *ppvResponse = pu32Saturature;

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_SetSaturatureCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HAL_SCREEN_NUMBER_S* pstIndexNumber = (HAL_SCREEN_NUMBER_S*)pvRequest;

    s32Ret = HI_HAL_SCREEN_SetSaturature(pstIndexNumber->enScreenIndex, pstIndexNumber->u32Number);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_HAL_EINTER);

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_GetContrastCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pu32RespLen, HI_HAL_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HI_HAL_SCREEN_IDX_E* penScreenIndex = (HI_HAL_SCREEN_IDX_E*)pvRequest;

    HI_U32* pu32Contrast = (HI_U32*)malloc(sizeof(HI_U32));

    s32Ret = HI_HAL_SCREEN_GetContrast(*penScreenIndex, pu32Contrast);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_SCREEN_GetContrast Error");
        HI_APPCOMM_SAFE_FREE(pu32Contrast);
        return HI_HAL_EINTER;
    }

    *pu32RespLen = sizeof(HI_U32);
    *ppvResponse = pu32Contrast;

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_SetContrastCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HAL_SCREEN_NUMBER_S* pstIndexNumber = (HAL_SCREEN_NUMBER_S*)pvRequest;

    s32Ret = HI_HAL_SCREEN_SetContrast(pstIndexNumber->enScreenIndex, pstIndexNumber->u32Number);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_HAL_EINTER);

    return HI_SUCCESS;
}

static HI_S32 MSG_HAL_SCREEN_DeinitCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest,
                            HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_APPCOMM_CHECK_POINTER(pvRequest, HI_HAL_EINVAL);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_HAL_SCREEN_IDX_E* penScreenIndex = (HI_HAL_SCREEN_IDX_E*)pvRequest;

    s32Ret = HI_HAL_SCREEN_Deinit(*penScreenIndex);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_HAL_EINTER);

    return HI_SUCCESS;
}

HI_S32 MSG_HAL_SCREEN_ServiceInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_INIT, MSG_HAL_SCREEN_InitCallback, NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_GETATTR, MSG_HAL_SCREEN_GetAttrCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_GETDISPLAYSTATE, MSG_HAL_SCREEN_GetDisplayStateCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_SETDISPLAYSTATE, MSG_HAL_SCREEN_SetDisplayStateCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_GETBACKLIGHTSTATE, MSG_HAL_SCREEN_GetBackLightStateCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_SETBACKLIGHTSTATE, MSG_HAL_SCREEN_SetBackLightStateCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_GETLUMA, MSG_HAL_SCREEN_GetLumaCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_SETLUMA, MSG_HAL_SCREEN_SetLumaCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_GETSATURATURE, MSG_HAL_SCREEN_GetSaturatureCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_SETSATURATURE, MSG_HAL_SCREEN_SetSaturatureCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_GETCONTRAST, MSG_HAL_SCREEN_GetContrastCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_SETCONTRAST, MSG_HAL_SCREEN_SetContrastCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    s32Ret = HI_MSG_RegisterMsgProc(MSG_HAL_SCREEN_DEINIT, MSG_HAL_SCREEN_DeinitCallback,NULL);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE(RED"msg RegReq failed\n\n"NONE);
    }

    return s32Ret;
}

