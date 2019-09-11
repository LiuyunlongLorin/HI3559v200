/**
* @file    hi_hal_gauge.c
* @brief   hal gauge implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/17
* @version   1.0

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "hi_hal_common_inner.h"

#include "hi_type.h"
#include "hi_hal_gauge.h"
#include "hi_appcomm_log.h"
#include "hi_appcomm_util.h"
#include "hi_hal_common.h"
#include "hi_hal_adc_inner.h"


#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif  /* End of #ifdef __cplusplus */

#define HAL_GAUGE_ADC_CHN                (1)

#define HAL_GAUGE_VALUE_MAX              (800)
#define HAL_GAUGE_VALUE_MIN              (600)

static HI_BOOL s_bGaugeInitFlag = HI_FALSE;
/** macro define */
HI_S32 HI_HAL_GAUGE_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (HI_TRUE == s_bGaugeInitFlag)
    {
        MLOGD("GAUGE has Already Init.\n");
        return HI_HAL_EINITIALIZED;
    }

    /**Set ADC CH1 Piuout*/
    himm(0X111F0034, 0x400);

    s32Ret = HAL_ADC_Init(HAL_GAUGE_ADC_CHN);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("[Error]ADC[%d] Init Failed.\n\n",HAL_GAUGE_ADC_CHN);
        return HI_HAL_EINTER;
    }

    s_bGaugeInitFlag = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_HAL_GAUGE_GetLevel(HI_S32* ps32Level) /* percent*/
{
    HI_S32 s32Ret = 0;
    HI_S32 s32Value = 0;
    /* parm ps32Level check */
    if (NULL == ps32Level)
    {
        MLOGE("ps32Level is null\n");
        return HI_HAL_EINVAL;
    }

    /* init check */
    if (HI_TRUE != s_bGaugeInitFlag)
    {
        MLOGE("gauge not initialized\n");
        return HI_HAL_ENOINIT;
    }

    s32Ret = HAL_ADC_GetVal(HAL_GAUGE_ADC_CHN, &s32Value);
    if(HI_FAILURE == s32Ret)
    {
        MLOGE("get value error\n");
        return HI_HAL_EINTER;
    }

    /**change  voltage  to guage percent */
    /**Gauge curve:  f(x) = 2x + 600, min->2V, max->2.9V*/
    if (s32Value > HAL_GAUGE_VALUE_MAX)
    {
        s32Value = HAL_GAUGE_VALUE_MAX;
    }
    else if (s32Value < HAL_GAUGE_VALUE_MIN)
    {
        s32Value = HAL_GAUGE_VALUE_MIN;
    }

    *ps32Level = (s32Value - 600) / 2;
    return HI_SUCCESS;
}

HI_S32 HI_HAL_GAUGE_GetChargeState(HI_BOOL* pbCharge)
{
    HI_BOOL bCharge = HI_FALSE;
    //HI_S32 s32Value = 0;
    /* parm ps32Level check */
    if (NULL == pbCharge)
    {
        MLOGE("pbCharge is null\n");
        return HI_HAL_EINVAL;
    }

    /* init check */
    if (HI_TRUE != s_bGaugeInitFlag)
    {
        MLOGE("gauge not initialized\n");
        return HI_HAL_ENOINIT;
    }

#if 0
    himd(0x120D0004, &s32Value);
    if (0 == s32Value)
    {
        bCharge = HI_TRUE;
    }
#endif
    *pbCharge = bCharge;

    return HI_SUCCESS;
}

HI_S32 HI_HAL_GAUGE_Deinit(HI_VOID)
{
    /* Close gauge Device */
    if (HI_TRUE != s_bGaugeInitFlag)
    {
        MLOGW("gauge not initialized, no need to close\n");
        return HI_HAL_ENOINIT;
    }
    HAL_ADC_Deinit(HAL_GAUGE_ADC_CHN);
    s_bGaugeInitFlag = HI_FALSE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif  /* End of #ifdef __cplusplus */

