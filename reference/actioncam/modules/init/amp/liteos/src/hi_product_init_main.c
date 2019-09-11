/**
 * @file      hi_product_init_main.c
 * @brief     init in HuaweiLite
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 * @version   1.0

 */

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "sys/types.h"
#include "sys/time.h"
#include "unistd.h"
#include "stdio.h"
#include "hi_type.h"
#include "asm/io.h"
#include "string.h"
#include "stdlib.h"
#include "pthread.h"
#include <sys/prctl.h>
#include "shell.h"
#include "hi_appcomm.h"

#include "hi_mapi_log.h"
#include "hi_product_init_os.h"
#include "hi_product_init_chip.h"
#include "hi_product_init_peripheral.h"
#include "hi_product_init_service.h"
#include "hi_timestamp.h"

static HI_S32 PDT_INIT_PreInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    MLOGD("HI_PDT_INIT_OS_PreInit ...\n");
    s32Ret = HI_PDT_INIT_OS_PreInit();//IPC 、串口、虚拟串口初始化
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    MLOGD("HI_PDT_INIT_CHIP_PreInit ...\n");
    s32Ret = HI_PDT_INIT_CHIP_PreInit(); //Sensor 驱动初始化 以及mpp层的驱动初始化
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    MLOGD("HI_PDT_INIT_PERIPHERAL_PreInit ...\n");
    s32Ret = HI_PDT_INIT_PERIPHERAL_PreInit();// 无代码逻辑
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#ifndef CONFIG_PQT_STREAM_SUPPORT_ON
    MLOGD("HI_PDT_INIT_SERVICE_PreInit ...\n");
    s32Ret = HI_PDT_INIT_SERVICE_PreInit();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#endif
    return s32Ret;
}

static HI_S32 PDT_INIT_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    MLOGD("HI_PDT_INIT_OS_Init ...\n");
    s32Ret = HI_PDT_INIT_OS_Init();   //系统时间相关的初始化
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    MLOGD("HI_PDT_INIT_CHIP_Init ...\n");
    s32Ret = HI_PDT_INIT_CHIP_Init(); //无代码逻辑实现
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    MLOGD("HI_PDT_INIT_PERIPHERAL_Init ...\n");
    s32Ret = HI_PDT_INIT_PERIPHERAL_Init();//触摸屏和陀螺仪相关驱动初始化
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#ifndef CONFIG_PQT_STREAM_SUPPORT_ON
    MLOGD("HI_PDT_INIT_SERVICE_Init ...\n");
    s32Ret = HI_PDT_INIT_SERVICE_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#endif
    return s32Ret;
}

static HI_S32 PDT_INIT_PostInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("HI_PDT_INIT_OS_PostInit ...\n");
    HI_TIME_STAMP;
    s32Ret = HI_PDT_INIT_OS_PostInit();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    MLOGD("HI_PDT_INIT_CHIP_PostInit ...\n");
    HI_TIME_STAMP;
    s32Ret = HI_PDT_INIT_CHIP_PostInit();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    MLOGD("HI_PDT_INIT_PERIPHERAL_PostInit ...\n");
    HI_TIME_STAMP;
    s32Ret = HI_PDT_INIT_PERIPHERAL_PostInit();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#ifndef CONFIG_PQT_STREAM_SUPPORT_ON
    MLOGD("HI_PDT_INIT_SERVICE_PostInit ...\n");
    HI_TIME_STAMP;
    s32Ret = HI_PDT_INIT_SERVICE_PostInit();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#endif
    return s32Ret;
}

#ifdef CFG_DEBUG_LOG_ON

static HI_S32 PDT_INIT_SetLog(HI_S32 argc, HI_CHAR* argv[])
{
    if (3 != argc)
    {
        printf(
                "\n"
                "*************************************************\n"
                "Usage: ./setlog [LogOn] [LogTimeOn] [LogLevel]\n"
                "LogLevel: \n"
                "   HI_LOG_LEVEL_FATAL: %d\n"
                "   HI_LOG_LEVEL_ERROR: %d\n"
                "   HI_LOG_LEVEL_WARNING: %d\n"
                "   HI_LOG_LEVEL_INFO: %d\n"
                "   HI_LOG_LEVEL_DEBUG: %d\n"
                "e.g : ./setlog 1 1 3\n"
                "*************************************************\n"
                "\n", HI_LOG_LEVEL_FATAL, HI_LOG_LEVEL_ERROR, HI_LOG_LEVEL_WARNING,
                HI_LOG_LEVEL_INFO, HI_LOG_LEVEL_DEBUG);
        return HI_FAILURE;
    }
    HI_BOOL bLogOn = atoi(argv[0]);
    HI_BOOL bLogTimeOn = atoi(argv[1]);
    HI_LOG_LEVEL_E enLogLevel = atoi(argv[2]);
    HI_APPCOMM_CHECK_EXPR(((enLogLevel >= HI_LOG_LEVEL_FATAL) && (enLogLevel < HI_LOG_LEVEL_BUTT)) ,HI_FAILURE);
    MLOGD("LogOn[%d] LogTimeOn[%d] Level[%d]\n", bLogOn, bLogTimeOn, enLogLevel);
    HI_LOG_Config(bLogOn, bLogTimeOn, enLogLevel);
    return HI_SUCCESS;
}

#endif

static HI_VOID PDT_INIT_CmdReg(HI_VOID)
{
#ifdef CFG_DEBUG_LOG_ON
    osCmdReg(CMD_TYPE_EX, "setlog", 1, (CMD_CBK_FUNC)PDT_INIT_SetLog);
#endif
#ifdef CONFIG_PQT_SUPPORT_ON
    #ifdef CONFIG_PQT_STREAM_SUPPORT_ON
    extern HI_VOID app_stream(HI_S32 argc, HI_CHAR **argv );
    osCmdReg(CMD_TYPE_EX,"pq_stream", 0, (CMD_CBK_FUNC)app_stream);
    #endif
    extern HI_VOID app_control(HI_S32 argc, HI_CHAR **argv );
    osCmdReg(CMD_TYPE_EX,"pq_control", 0, (CMD_CBK_FUNC)app_control);
#endif
}

HI_VOID* PDT_INIT_Main(HI_VOID *arg)
{
    prctl(PR_SET_NAME, (unsigned long) "PDT_INIT_Main", 0, 0, 0);
#ifdef CONFIG_DEBUG
    HI_LOG_Config(HI_TRUE, HI_FALSE, HI_LOG_LEVEL_INFO);
    HI_MAPI_LOG_SetEnabledLevel(HI_MAPI_LOG_LEVEL_DEBUG);
#else
   HI_LOG_Config(HI_TRUE, HI_FALSE, HI_LOG_LEVEL_INFO);
	   HI_MAPI_LOG_SetEnabledLevel(HI_MAPI_LOG_LEVEL_DEBUG);

   // HI_LOG_Config(HI_TRUE, HI_FALSE, HI_LOG_LEVEL_ERROR);
   // HI_MAPI_LOG_SetEnabledLevel(HI_MAPI_LOG_LEVEL_ERR);
#endif
    HI_TIME_STAMP;
    PDT_INIT_PreInit(); //串口相关的初始化、IPCM相关的初始化、Sensor相关的初始化、mpp层驱动相关的初始化
    PDT_INIT_Init();//系统时间初始化、LCD屏初始化、陀螺仪初始化
    PDT_INIT_PostInit();
    HI_TIME_STAMP;
    PDT_INIT_CmdReg();
    HI_TIME_PRINT;
    return HI_NULL;
}

static pthread_t s_AppInitThread = -1;
HI_VOID app_init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    pthread_attr_t attr;

    (HI_VOID)pthread_attr_setstacksize(&attr, 0xc000);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    attr.schedparam.sched_priority = 10;

    s32Ret = pthread_create(&s_AppInitThread, &attr, PDT_INIT_Main, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("pthread_create PDT_INIT_app fail\n");
    }
    pthread_attr_destroy(&attr);
    MLOGD("PDT_INIT_app successfull\n");
    return ;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

