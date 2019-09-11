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
#include "hi_hal_gpio_inner.h"

#include "hi_hal_key.h"
#include "hi_hal_common.h"
#include "hi_appcomm_log.h"
#include "hi_appcomm_util.h"

#include "hi_hal_adc_inner.h"


#include <sys/ioctl.h>
#include "hi_adc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

/** macro define */
#define HAL_KEY_MEM_DEV  "/dev/mem"
#define HAL_KEY_POWER_CTRL_BASE  (0x12090000) /**< power control base register address */
#define HAL_KEY_PWR_CTRL0_OFFSET (0x0008)     /**< power control0 offset */


#ifdef CONFIG_ADC_ON

#define HAL_KEY_KEY3_ADC_VALUEMIN (85)
#define HAL_KEY_KEY3_ADC_VALUEMAX (100)
#define HAL_KEY_KEY2_ADC_VALUEMIN (160)
#define HAL_KEY_KEY2_ADC_VALUEMAX (175)
#define HAL_KEY_ADC_CHN           (0)

#else

#define HAL_KEY_KEY2_GRP_NUM (10)     /**< key2 gpio group number */
#define HAL_KEY_KEY2_BIT_NUM (3)       /**< key2 gpio bit number */

#endif

#define HAL_KEY_READ_REG(addr)             (*(volatile HI_U32 *)(addr))

#define HAL_KEY_KEY1_GRP_NUM (0)     /**< key1 gpio group number */
#define HAL_KEY_KEY1_BIT_NUM (0)     /**< key1 gpio bit number */



HI_U32  s_u32HALKEYPwrCtrlBase;  /**< address mmap from power control base register address */
HI_U32* s_pu32HALKEYPwrCtrlBase; /**< address mmap from power control base register address */

static HI_S32 s_s32HALKEYfd = HAL_FD_INITIALIZATION_VAL;
static HI_S32 s_s32HALKEYMEMfd = HAL_FD_INITIALIZATION_VAL;

HI_S32 HI_HAL_KEY_Init()
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (HAL_FD_INITIALIZATION_VAL == s_s32HALKEYfd)
    {
        /** Initial GPIO Device */
        s32Ret = HAL_GPIO_Init(&s_s32HALKEYfd);

        if (HI_FAILURE == s32Ret)
        {
            MLOGE("[Error] open gpiodev failed,errno(%d)\n",errno);
            return HI_HAL_EINTER;
        }

        /** Initial Power Ctrl */
        s_s32HALKEYMEMfd = open(HAL_KEY_MEM_DEV, O_RDWR | O_SYNC, 00777);

        if (s_s32HALKEYMEMfd < 0)
        {
            MLOGE("[Error] open memdev failed\n");
            HAL_GPIO_Deinit(s_s32HALKEYfd);
            s_s32HALKEYfd = HAL_FD_INITIALIZATION_VAL;
            return HI_HAL_EINVOKESYS;
        }

#ifdef CONFIG_ADC_ON
        s32Ret = HAL_ADC_Init(HAL_KEY_ADC_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("[Error]ADC[%d] Init Failed.\n\n",HAL_KEY_ADC_CHN);
            close(s_s32HALKEYMEMfd);
            s_s32HALKEYMEMfd = HAL_FD_INITIALIZATION_VAL;
            HAL_GPIO_Deinit(s_s32HALKEYfd);
            s_s32HALKEYfd = HAL_FD_INITIALIZATION_VAL;
        }
#else
        /**Set ADC pin to GPIO to enable Key*/
        himm(0x111f0030, 0x401);
        himm(0x111f0034, 0x401);
#endif

        s_pu32HALKEYPwrCtrlBase = (HI_U32*)mmap(NULL, 0x50, \
                                                PROT_READ | PROT_WRITE, MAP_SHARED, s_s32HALKEYMEMfd, HAL_KEY_POWER_CTRL_BASE);

        if (s_pu32HALKEYPwrCtrlBase == MAP_FAILED)
        {
            MLOGE("[Error] power_ctrl mmap failed,errno(%d)\n",errno);
            close(s_s32HALKEYMEMfd);
            s_s32HALKEYMEMfd = HAL_FD_INITIALIZATION_VAL;
            HAL_GPIO_Deinit(s_s32HALKEYfd);
            s_s32HALKEYfd = HAL_FD_INITIALIZATION_VAL;

            return HI_HAL_EINVOKESYS;
        }


        s_u32HALKEYPwrCtrlBase = (HI_U32)s_pu32HALKEYPwrCtrlBase;
    }
    else
    {
        MLOGE("gpio already be opened\n");
        return HI_HAL_EINITIALIZED;
    }

    return HI_SUCCESS;
}


HI_S32 HI_HAL_KEY_GetState(HI_HAL_KEY_IDX_E enKeyIdx, HI_HAL_KEY_STATE_E* penKeyState)
{
    HI_U32 u32GpioGrpNum, u32GpioBitNum;
    HI_U32 u32Val;
    HI_S32 s32Ret = HI_SUCCESS;
#ifdef CONFIG_ADC_ON
    HI_S32 s32AdcValue = 0;
#endif

    /* init check */
    if (HAL_FD_INITIALIZATION_VAL == s_s32HALKEYfd)
    {
        MLOGE("key not initialized\n");
        return HI_HAL_ENOINIT;
    }

    /* parm penKeyState check */
    if (NULL == penKeyState)
    {
        MLOGE("penKeyState is null\n");
        return HI_HAL_EINVAL;
    }

    switch (enKeyIdx)
    {
        case HI_HAL_KEY_IDX_0:
            u32Val = HAL_KEY_READ_REG(s_u32HALKEYPwrCtrlBase + HAL_KEY_PWR_CTRL0_OFFSET);
            u32Val = (u32Val & (0x01 << 7)) ? 1 : 0;
            break;

        case HI_HAL_KEY_IDX_3:
#ifdef CONFIG_ADC_ON
            HAL_ADC_GetVal(HAL_KEY_ADC_CHN, &s32AdcValue);
            if (s32AdcValue > HAL_KEY_KEY3_ADC_VALUEMIN && s32AdcValue < HAL_KEY_KEY3_ADC_VALUEMAX)
            {
                u32Val = 0;
            }
            else
            {
                u32Val = 1;
            }
#else
            u32Val = 1;
#endif
            break;
        case HI_HAL_KEY_IDX_2:
#ifdef CONFIG_ADC_ON
            HAL_ADC_GetVal(HAL_KEY_ADC_CHN, &s32AdcValue);
            if (s32AdcValue > HAL_KEY_KEY2_ADC_VALUEMIN && s32AdcValue < HAL_KEY_KEY2_ADC_VALUEMAX)
            {
                u32Val = 0;
            }
            else
            {
                u32Val = 1;
            }
#else
            u32GpioGrpNum = HAL_KEY_KEY2_GRP_NUM;
            u32GpioBitNum = HAL_KEY_KEY2_BIT_NUM;
            s32Ret = HAL_GPIO_SetDir(s_s32HALKEYfd, u32GpioGrpNum, u32GpioBitNum, HAL_GPIO_DIR_READ);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("[Error]set gpio dir failed\n");
                return HI_HAL_EGPIO;
            }

            s32Ret = HAL_GPIO_GetBitVal(s_s32HALKEYfd, u32GpioGrpNum, u32GpioBitNum, &u32Val);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("[Error]read gpio data failed\n");
                return HI_HAL_EGPIO;
            }
#endif

             break;
        case HI_HAL_KEY_IDX_1:
            u32GpioGrpNum = HAL_KEY_KEY1_GRP_NUM;
            u32GpioBitNum = HAL_KEY_KEY1_BIT_NUM;
            s32Ret = HAL_GPIO_SetDir(s_s32HALKEYfd, u32GpioGrpNum, u32GpioBitNum, HAL_GPIO_DIR_READ);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("[Error]set gpio dir failed\n");
                return HI_HAL_EGPIO;
            }

            s32Ret = HAL_GPIO_GetBitVal(s_s32HALKEYfd, u32GpioGrpNum, u32GpioBitNum, &u32Val);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("[Error]read gpio data failed\n");
                return HI_HAL_EGPIO;
            }

            break;

        default:
            MLOGE("illeagel enkey(%d) out of range\n",enKeyIdx);
            return HI_HAL_EINVAL;
    }

    *penKeyState = (1 == u32Val) ? HI_HAL_KEY_STATE_UP : HI_HAL_KEY_STATE_DOWN;

    return HI_SUCCESS;
}


HI_S32 HI_HAL_KEY_Deinit()
{
    HI_S32 s32Ret = 0;

    if (HAL_FD_INITIALIZATION_VAL == s_s32HALKEYfd)
    {
        MLOGE("key not initialized,no need to close\n");
        return HI_HAL_ENOINIT;
    }

    /* DeInit Power Ctrl */
    if (s_pu32HALKEYPwrCtrlBase)
    {
        s32Ret = munmap(s_pu32HALKEYPwrCtrlBase, HAL_MMAP_LENGTH);

        if (0 != s32Ret)
        {
            MLOGE("munmap failed,errno(%d)\n",errno);
            return HI_HAL_EINVOKESYS;
        }

        s_pu32HALKEYPwrCtrlBase = NULL;
    }

    if (HAL_FD_INITIALIZATION_VAL != s_s32HALKEYMEMfd)
    {
        close(s_s32HALKEYMEMfd);
        s_s32HALKEYMEMfd = HAL_FD_INITIALIZATION_VAL;
    }

    /* Deinitial GPIO Device */
    s32Ret = HAL_GPIO_Deinit(s_s32HALKEYfd);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("gpio deinit,errno(%d)\n",errno);
        return HI_HAL_EINVOKESYS;
    }
#ifdef CONFIG_ADC_ON
    HAL_ADC_Deinit(HAL_KEY_ADC_CHN);
#endif
    s_s32HALKEYfd = HAL_FD_INITIALIZATION_VAL;

    return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


