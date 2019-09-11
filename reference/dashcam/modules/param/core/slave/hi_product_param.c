/**
 * @file      hi_product_param.c
 * @brief     product parameter module interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "hi_product_param_inner.h"
#include "hi_product_param.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


/**--------------------- Common  Configure  ------------------------- */

/** parameter context */
static PDT_PARAM_CONTEXT_S s_stPARAMCtx = {
    .bInit = HI_FALSE,
    .mutexLock = PTHREAD_MUTEX_INITIALIZER,
    .pstCfg = NULL
};

/**------------------- Common Configure End  ------------------------- */


/**------------------- Common Interface  ------------------------- */

inline PDT_PARAM_CONTEXT_S *PDT_PARAM_GetCtx(HI_VOID)
{
    return &s_stPARAMCtx;
}

HI_S32 HI_PDT_PARAM_Init(HI_VOID)
{
    /* Check Module Init Status */
    if (s_stPARAMCtx.bInit) {
        MLOGW("has already init\n");
        return HI_PDT_PARAM_EINITIALIZED;
    }
#ifdef CFG_RAW_PARAM_ON
    s_stPARAMCtx.pstCfg = (PDT_PARAM_CFG_S *)PDT_PARAM_PHY_ADDR;
    if ((s_stPARAMCtx.pstCfg->stHead.u32MagicStart) != PDT_PARAM_MAGIC_START
            || (s_stPARAMCtx.pstCfg->u32MagicEnd != PDT_PARAM_MAGIC_END)) {
        MLOGE("\n Error!!! param can't resume!!! Please check the param data\n");
        MLOGE("\n Error!!! Media service will be started in LINUX\n");
        s_stPARAMCtx.pstCfg = NULL;
        return HI_FAILURE;
    }
#endif
    s_stPARAMCtx.bInit = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_Deinit(HI_VOID)
{
    if (HI_FALSE == s_stPARAMCtx.bInit) {
        MLOGD("has already deinit or not init yet\n");
        return HI_SUCCESS;
    }
    s_stPARAMCtx.bInit = HI_FALSE;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

