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
#include "hi_hal_motionsensor.h"
#include "asm/io.h"

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

#ifdef CONFIG_MOTIONSENSOR
static HI_S32 Gyro_Dis_init(void)
{
    extern int gyrodis_mod_init(void);
    return gyrodis_mod_init();
}

static HI_S32 MOTIONFUSION_init(void)
{
    extern int motionfusion_mod_init(void);
    return motionfusion_mod_init();
}
#endif

HI_S32 HI_PDT_INIT_PERIPHERAL_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

#if defined(CONFIG_SCREEN)
    MLOGD("-->screen init ...\n");
    extern HI_HAL_SCREEN_OBJ_S stHALSCREENObj;
    s32Ret = HI_HAL_SCREEN_Register(HI_HAL_SCREEN_IDX_0, &stHALSCREENObj);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI_HAL_SCREEN_Register");

    s32Ret= HI_HAL_SCREEN_Init(HI_HAL_SCREEN_IDX_0);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI_HAL_SCREEN_Init");

#if (!defined(BOARD_ACTIONCAM_REFB))
    s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI_HAL_SCREEN_SetBackLightState");
#endif

#endif

#ifdef CONFIG_MOTIONSENSOR
    MLOGI(YELLOW"-->motionsensor init ...\n"NONE);
    s32Ret = HI_HAL_MOTIONSENSOR_Init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI_HAL_MOTIONSENSOR_Init");

    Gyro_Dis_init();
    MOTIONFUSION_init();
    MLOGI(GREEN"---->Lorin add-> motionsensor init success....\n"NONE);
#endif

    return s32Ret;
}

HI_S32 HI_PDT_INIT_PERIPHERAL_PostInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
#if defined(CONFIG_SCREEN) && defined(BOARD_ACTIONCAM_REFB)
        /* screen power on */
    HI_usleep(100000);
    s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "HI_HAL_SCREEN_SetBackLightState");
#endif
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

