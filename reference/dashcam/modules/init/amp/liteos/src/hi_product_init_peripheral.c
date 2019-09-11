/**
 * @file      hi_product_init_peripheral.c
 * @brief     peripheral init source
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 * @version   1.0

 */

#include "hi_product_init_os.h"
#include "hi_product_init_peripheral.h"
#include "hi_hal_screen.h"
#include "asm/io.h"
#include "hi_product_param.h"
#include "hi_mapi_hal_ahd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 HI_PDT_INIT_PERIPHERAL_PreInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    return s32Ret;
}

HI_S32 HI_PDT_INIT_PERIPHERAL_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

#if defined(CONFIG_SCREEN)
    MLOGI("-->screen init ...\n");
    extern HI_HAL_SCREEN_OBJ_S stHALSCREENObj;
    s32Ret = HI_HAL_SCREEN_Register(HI_HAL_SCREEN_IDX_0, &stHALSCREENObj);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI_HAL_SCREEN_Register");

    s32Ret= HI_HAL_SCREEN_Init(HI_HAL_SCREEN_IDX_0);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI_HAL_SCREEN_Init");

    s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI_HAL_SCREEN_SetBackLightState");

    HI_U32 u32value = 0;
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS ,&u32value);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI_PDT_PARAM_GetCommParam");
    s32Ret = HI_HAL_SCREEN_SetLuma(HI_HAL_SCREEN_IDX_0,u32value);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI_HAL_SCREEN_SetLuma");
#endif
#ifdef CONFIG_SNS1_BT656
#ifdef CONFIG_AHD_N3
        extern HI_HAL_AHD_DEV_S g_halAhdN3Obj;
        HI_MAPI_VCAP_RegAhdModule(1,&g_halAhdN3Obj);
#else
        MLOGE("******must define which ahd device*******\n");
        s32Ret = HI_FAILURE;
#endif
#endif
    return s32Ret;
}

HI_S32 HI_PDT_INIT_PERIPHERAL_PostInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

