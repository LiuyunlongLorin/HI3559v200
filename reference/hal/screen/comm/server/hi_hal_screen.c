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
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "hi_hal_screen.h"
#include "hi_hal_common.h"
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
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnInit)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnInit();
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnInit");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

    s_astHALSCREENCtx[enScreenIndex].bInit = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_ATTR_S* pstAttr)
{
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetAttr)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetAttr(pstAttr);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnGetAttr");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetDisplayState(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_STATE_E* penDisplayState)
{
    HI_APPCOMM_CHECK_POINTER(penDisplayState, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetDisplayState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetDisplayState(penDisplayState);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnGetDisplayState");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

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
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetDisplayState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetDisplayState(enDisplayState);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnSetDisplayState");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetBackLightState(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_STATE_E* penBackLightState)
{
    HI_APPCOMM_CHECK_POINTER(penBackLightState, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetBackLightState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetBackLightState(penBackLightState);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnGetBackLightState");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

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
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;
    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetBackLightState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetBackLightState(enBackLightState);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnSetBackLightState");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetLuma(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32* pu32Luma)
{
    HI_APPCOMM_CHECK_POINTER(pu32Luma, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetLuma)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetLuma(pu32Luma);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnGetLuma");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

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
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetLuma)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetLuma(u32Luma);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnSetLuma");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetSaturature(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32* pu32Saturature)
{
    HI_APPCOMM_CHECK_POINTER(pu32Saturature, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetSaturature)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetSaturature(pu32Saturature);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnGetSaturature");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

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
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetSaturature)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetSaturature(u32Saturature);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnSetSaturature");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_GetContrast(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32* pu32Contrast)
{
    HI_APPCOMM_CHECK_POINTER(pu32Contrast, HI_HAL_EINVAL);
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_REGISTER(enScreenIndex);
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetContrast)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetContrast(pu32Contrast);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnGetContrast");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

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
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetContrast)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetContrast(u32Contrast);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnSetContrast");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_SCREEN_Deinit(HI_HAL_SCREEN_IDX_E enScreenIndex)
{
    HAL_SCREEN_CHECK_IDX(enScreenIndex);
    HAL_SCREEN_CHECK_INIT(enScreenIndex);

    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnDeinit)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnDeinit();
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_HAL_EINTER, "pfnDeinit");
    }
    else
    {
        MLOGE("Screen[%d] Null ptr.\n", enScreenIndex);
        return HI_HAL_EINVAL;
    }

    s_astHALSCREENCtx[enScreenIndex].bInit = HI_FALSE;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

