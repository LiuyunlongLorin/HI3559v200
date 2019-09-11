/**
 * @file      hi_product_init_service.c
 * @brief     service init source
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 * @version   1.0

 */

#include "asm/io.h"
#include "hi_product_init_os.h"
#include "hi_product_init_service.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_log.h"
#include "hi_appcomm_msg_server.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


HI_S32 HI_PDT_INIT_SERVICE_PreInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    return s32Ret;
}

HI_S32 HI_PDT_INIT_SERVICE_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* mapi log level */
    HI_MAPI_LOG_SetEnabledLevel(HI_MAPI_LOG_LEVEL_WARN);

    s32Ret = HI_MAPI_Sys_Init();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_Sys_Init");

    /* msg */
    HI_MSG_SERVER_Init();
    HI_TIME_STAMP;
#ifdef CONFIG_SCREEN
    MLOGD("msg hal screen init ...\n");
    extern HI_S32 MSG_HAL_SCREEN_ServiceInit(HI_VOID);
    MSG_HAL_SCREEN_ServiceInit();
#endif
    return s32Ret;
}

HI_S32 HI_PDT_INIT_SERVICE_PostInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
	return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

