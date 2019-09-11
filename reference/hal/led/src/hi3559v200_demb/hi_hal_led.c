/**
* @file    hi_hal_key.c
* @brief   HAL key implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/16
* @version   1.0

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "hi_hal_common_inner.h"

#include "hi_hal_led.h"
#include "hi_hal_common.h"
#include "hi_appcomm_log.h"
#include "hi_appcomm_util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

/** macro define */
#define HAL_LED_IDX0_ADDR        (0x111F0000)
#define HAL_LED_IDX1_ADDR        (0x111F0004)

#define HAL_LED_ORIGINAL_VALUE   (0x400)
#define HAL_LED_LIGHTON_VALUE    (0x500)
#define HAL_LED_LIGHTOFF_VALUE   (0x600)



static HI_BOOL s_abLedInitState[HI_HAL_LED_IDX_BUIT];


HI_S32 HI_HAL_LED_Init(HI_HAL_LED_IDX_E enLedIdx)
{
    if (s_abLedInitState[enLedIdx]  == HI_TRUE)
    {
        MLOGE("halled has already been started\n");
        return HI_HAL_EINITIALIZED;
    }

    /**Set LED0 & LED1 to be orignal*/
    if (HI_HAL_LED_IDX_0 == enLedIdx)
    {
        himm(HAL_LED_IDX0_ADDR, HAL_LED_ORIGINAL_VALUE);
    }
    else if (HI_HAL_LED_IDX_1 == enLedIdx)
    {
        himm(HAL_LED_IDX1_ADDR, HAL_LED_ORIGINAL_VALUE);
    }
    else
    {
        MLOGE("invalid LedIdx\n");
        return HI_HAL_EINVAL;
    }

    s_abLedInitState[enLedIdx]  = HI_TRUE;
    return HI_SUCCESS;
}


HI_S32 HI_HAL_LED_SetState(HI_HAL_LED_IDX_E enLedIdx, HI_HAL_LED_STATE_E enLedState)
{
    /* init check */
    if (HI_TRUE != s_abLedInitState[enLedIdx])
    {
        MLOGE("led not initialized\n");
        return HI_HAL_ENOINIT;
    }

    switch (enLedIdx)
    {
        case HI_HAL_LED_IDX_0:
            if (HI_HAL_LED_STATE_ON == enLedState)
            {
                himm(HAL_LED_IDX0_ADDR, HAL_LED_LIGHTON_VALUE);
            }
            else if (HI_HAL_LED_STATE_OFF == enLedState)
            {
                himm(HAL_LED_IDX0_ADDR, HAL_LED_LIGHTOFF_VALUE);
            }
            else
            {
                MLOGE("illeagel enLedState(%d) out of range\n",enLedState);
                return HI_HAL_EINVAL;
            }
            break;
        case HI_HAL_LED_IDX_1:
            if (HI_HAL_LED_STATE_ON == enLedState)
            {
                himm(HAL_LED_IDX1_ADDR, HAL_LED_LIGHTON_VALUE);
            }
            else if (HI_HAL_LED_STATE_OFF == enLedState)
            {
                himm(HAL_LED_IDX1_ADDR, HAL_LED_LIGHTOFF_VALUE);
            }
            else
            {
                MLOGE("illeagel enLedState(%d) out of range\n",enLedState);
                return HI_HAL_EINVAL;
            }
            break;
        default:
            MLOGE("illeagel enLed(%d) out of range\n",enLedIdx);
            return HI_HAL_EINVAL;
    }
    return HI_SUCCESS;
}


HI_S32 HI_HAL_LED_Deinit(HI_HAL_LED_IDX_E enLedIdx)
{
    /* init check */
    if (HI_TRUE != s_abLedInitState[enLedIdx])
    {
        MLOGE("led not initialized\n");
        return HI_HAL_ENOINIT;
    }

    /**Set LED0 & LED1 to be orignal*/
    if (HI_HAL_LED_IDX_0 == enLedIdx)
    {
        himm(HAL_LED_IDX0_ADDR, HAL_LED_ORIGINAL_VALUE);
    }
    else if (HI_HAL_LED_IDX_1 == enLedIdx)
    {
        himm(HAL_LED_IDX1_ADDR, HAL_LED_ORIGINAL_VALUE);
    }
    else
    {
        MLOGE("invalid LedIdx\n");
        return HI_HAL_EINVAL;
    }

    s_abLedInitState[enLedIdx] = HI_FALSE;
    return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


