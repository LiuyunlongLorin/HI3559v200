/**
* @file    hi_hal_adc_inner.h
* @brief   product inner hal adc interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/18
* @version   1.0

*/
#ifndef __HI_HAL_ADC_INNER__H__
#define __HI_HAL_ADC_INNER__H__
#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_ADC */
/** @{ */  /** <!-- [HAL_ADC] */

/**
* @brief    hal ADC initialization, open ADC device
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/19
*/
HI_S32 HAL_ADC_Init(HI_HANDLE AdcHdl);

/**
* @brief   Get Chn Value
* @param[in] s32Chn
* @param[out] ps32Value
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/19
*/
HI_S32 HAL_ADC_GetVal(HI_HANDLE AdcHdl, HI_S32* ps32Value);


/**
* @brief    hal ADC deinitialization,close fd
* @param[in] s32fd
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/19
*/
HI_S32 HAL_ADC_Deinit(HI_HANDLE AdcHdl);
/** @}*/  /** <!-- ==== HAL_ADC End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_ADC_INNER__H__*/

