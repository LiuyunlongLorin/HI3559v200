/**
 * @file      hi_product_param_bin2image.c
 * @brief     param bin2image tool
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/6/19
 * @version   1.0

 */
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>
#include <semaphore.h>

#include "hi_eventhub.h"
#include "hi_product_param.h"
#include "hi_appcomm.h"
#include "hi_appcomm_util.h"
#include "hi_product_param_debug.h"
#include "hi_timestamp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

HI_S32 main(HI_S32 s32Argc, HI_CHAR* pszArgv[])
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_EVTHUB_Init();
    HI_PDT_PARAM_RegisterEvent();

    /* init Param */
    s32Ret = HI_PDT_PARAM_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    /* param set default */
    s32Ret = HI_PDT_PARAM_SetDefault();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    sleep(2);
    HI_PDT_PARAM_Deinit();
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

