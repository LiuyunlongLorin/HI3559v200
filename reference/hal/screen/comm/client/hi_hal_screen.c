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
#include "hi_appcomm_msg_client.h"
#include "hi_hal_common.h"
#include "hi_hal_screen.h"
#include "hi_hal_common_inner.h"
#include "hi_hal_screen_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


/** \addtogroup     SCREEN */
/** @{ */  /** <!-- [SCREEN] */

#define HAL_SCREEN_CHECK_REGISTER(enIdx) \
    do { \
        if (!s_astHALSCREENCtx[enIdx].bRegister){ \
            MLOGD("Screen[%d] has not been registered\n", enIdx); \
            return HI_HAL_ENOREG;  \
        }   \
    } while(0)

#define HAL_SCREEN_CHECK_INIT(enIdx) \
    do { \
        if (!s_astHALSCREENCtx[enIdx].bInit){ \
            MLOGD("Screen[%d] has not been inited\n", enIdx); \
            return HI_HAL_ENOINIT;  \
        }   \
    } while(0)


static HAL_SCREEN_CTX_S s_astHALSCREENCtx[HI_HAL_SCREEN_IDX_BUTT];


HI_S32 HI_HAL_SCREEN_Register(HI_HAL_SCREEN_IDX_E enScreenIndex, const HI_HAL_SCREEN_OBJ_S* pstScreenObj)
{
    HI_APPCOMM_CHECK_POINTER(pstScreenObj, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnDeinit, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnGetAttr, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnGetBackLightState, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnGetContrast, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnGetDisplayState, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnGetLuma, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnGetSaturature, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnInit, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnSetBackLightState, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnSetContrast, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnSetDisplayState, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnSetLuma, HI_HAL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstScreenObj->pfnSetSaturature, HI_HAL_EINVAL);

    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    if (s_astHALSCREENCtx[enScreenIndex].bRegister)
    {
        MLOGD("Screen[%d] has been registered\n", enScreenIndex);
        return HI_HAL_EREGRED;
    }

    memcpy(&s_astHALSCREENCtx[enScreenIndex].stScreenObj, pstScreenObj, sizeof(HI_HAL_SCREEN_OBJ_S));
    s_astHALSCREENCtx[enScreenIndex].bRegister = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_Init(HI_HAL_SCREEN_IDX_E enScreenIndex)
{
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);
    if (s_astHALSCREENCtx[enScreenIndex].bInit)
    {
        MLOGD("Screen[%d] has been inited\n", enScreenIndex);
        return HI_HAL_EINITIALIZED;
    }

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_INIT, &enScreenIndex,
        sizeof(HI_HAL_SCREEN_IDX_E), HI_NULL, HI_NULL, 0);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    s_astHALSCREENCtx[enScreenIndex].bInit = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_ATTR_S* pstAttr)
{
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_GETATTR, &enScreenIndex,
        sizeof(HI_HAL_SCREEN_IDX_E), HI_NULL, pstAttr, sizeof(HI_HAL_SCREEN_ATTR_S));
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetDisplayState(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_STATE_E* penDisplayState)
{
    HI_APPCOMM_CHECK_POINTER(penDisplayState, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_GETDISPLAYSTATE, &enScreenIndex,
        sizeof(HI_HAL_SCREEN_IDX_E), HI_NULL, penDisplayState, sizeof(HI_HAL_SCREEN_STATE_E));
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_STATE_E enDisplayState)
{
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    if (!HI_APPCOMM_CHECK_RANGE(enDisplayState, HI_HAL_SCREEN_STATE_OFF, HI_HAL_SCREEN_STATE_BUIT - 1))
    {
        MLOGE("Invalid screen state[%d]\n", enDisplayState);
        return HI_HAL_EINVAL;
    }
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;
    HAL_SCREEN_STATE_S stScreenState;
    stScreenState.enScreenIndex = enScreenIndex;
    stScreenState.enState = enDisplayState;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_SETDISPLAYSTATE, &stScreenState,
        sizeof(HAL_SCREEN_STATE_S), HI_NULL, HI_NULL, 0);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetBackLightState(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_STATE_E* penBackLightState)
{
    HI_APPCOMM_CHECK_POINTER(penBackLightState, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_GETBACKLIGHTSTATE, &enScreenIndex,
        sizeof(HI_HAL_SCREEN_IDX_E), HI_NULL, penBackLightState, sizeof(HI_HAL_SCREEN_STATE_E));
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_STATE_E enBackLightState)
{
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    if (!HI_APPCOMM_CHECK_RANGE(enBackLightState, HI_HAL_SCREEN_STATE_OFF, HI_HAL_SCREEN_STATE_BUIT - 1))
    {
        MLOGE("Invalid screen state[%d]\n", enBackLightState);
        return HI_HAL_EINVAL;
    }
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;
    HAL_SCREEN_STATE_S stScreenState;
    stScreenState.enScreenIndex = enScreenIndex;
    stScreenState.enState = enBackLightState;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_SETBACKLIGHTSTATE, &stScreenState,
        sizeof(HAL_SCREEN_STATE_S), HI_NULL, HI_NULL, 0);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetLuma(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32* pu32Luma)
{
    HI_APPCOMM_CHECK_POINTER(pu32Luma, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_GETLUMA, &enScreenIndex,
        sizeof(HI_HAL_SCREEN_IDX_E), HI_NULL, pu32Luma, sizeof(HI_U32));
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_SetLuma(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32 u32Luma)
{
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    if (!HI_APPCOMM_CHECK_RANGE(u32Luma, 0, 100))
    {
        MLOGE("Invalid screen luma[%u]\n", u32Luma);
        return HI_HAL_EINVAL;
    }
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;
    HAL_SCREEN_NUMBER_S stLuma;
    stLuma.enScreenIndex = enScreenIndex;
    stLuma.u32Number = u32Luma;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_SETLUMA, &stLuma,
        sizeof(HAL_SCREEN_NUMBER_S), HI_NULL, HI_NULL, 0);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetSaturature(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32* pu32Saturature)
{
    HI_APPCOMM_CHECK_POINTER(pu32Saturature, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_GETSATURATURE, &enScreenIndex,
        sizeof(HI_HAL_SCREEN_IDX_E), HI_NULL, pu32Saturature, sizeof(HI_U32));
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_SetSaturature(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32 u32Saturature)
{
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    if (!HI_APPCOMM_CHECK_RANGE(u32Saturature, 0, 100))
    {
        MLOGE("Invalid screen saturature[%u]\n", u32Saturature);
        return HI_HAL_EINVAL;
    }
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;
    HAL_SCREEN_NUMBER_S stSaturation;
    stSaturation.enScreenIndex = enScreenIndex;
    stSaturation.u32Number = u32Saturature;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_SETSATURATURE, &stSaturation,
        sizeof(HAL_SCREEN_NUMBER_S), HI_NULL, HI_NULL, 0);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetContrast(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32* pu32Contrast)
{
    HI_APPCOMM_CHECK_POINTER(pu32Contrast, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_GETCONTRAST, &enScreenIndex,
        sizeof(HI_HAL_SCREEN_IDX_E), HI_NULL, pu32Contrast, sizeof(HI_U32));
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_SetContrast(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32 u32Contrast)
{
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    if (!HI_APPCOMM_CHECK_RANGE(u32Contrast, 0, 100))
    {
        MLOGE("Invalid screen contrast[%u]\n", u32Contrast);
        return HI_HAL_EINVAL;
    }
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;
    HAL_SCREEN_NUMBER_S stContrast;
    stContrast.enScreenIndex = enScreenIndex;
    stContrast.u32Number = u32Contrast;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_SETCONTRAST, &stContrast,
        sizeof(HAL_SCREEN_NUMBER_S), HI_NULL, HI_NULL, 0);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    return s32Ret;
}

HI_S32 HI_HAL_SCREEN_Deinit(HI_HAL_SCREEN_IDX_E enScreenIndex)
{
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_HAL_SCREEN_DEINIT, &enScreenIndex,
        sizeof(HI_HAL_SCREEN_IDX_E), HI_NULL, HI_NULL, 0);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "HI_MSG_CLIENT_SendSync");

    s_astHALSCREENCtx[enScreenIndex].bInit = HI_FALSE;
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

