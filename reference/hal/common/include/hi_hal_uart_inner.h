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
#ifndef __HI_HAL_UART__H__
#define __HI_HAL_UART__H__
#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

typedef enum hiUARTHAL_BIT_RATE
{
    HAL_UART_BITRATE_300,
    HAL_UART_BITRATE_1200,
    HAL_UART_BITRATE_2400,
    HAL_UART_BITRATE_4800,
    HAL_UART_BITRATE_9600,
    HAL_UART_BITRATE_19200,
    HAL_UART_BITRATE_38400,
    HAL_UART_BITRATE_115200,
    HAL_UART_BITRATE_BUTT,
} UARTHAL_BIT_RATE;

typedef enum hiUARTHAL_DATA_BIT
{
    HAL_UART_DATABIT_7,
    HAL_UART_DATABIT_8,
    HAL_UART_DATABIT_BUTT,
} UARTHAL_DATA_BIT;

typedef enum hiUARTHAL_STOP_BIT
{
    HAL_UART_STOPBIT_1,
    HAL_UART_STOPBIT_2,
    HAL_UART_STOPBIT_BUTT,
} UARTHAL_STOP_BIT;

typedef enum hiUARTHAL_PARITY
{
    HAL_UART_PARITY_N,
    HAL_UART_PARITY_O,
    HAL_UART_PARITY_E,
    HAL_UART_PARITY_S,
    HAL_UART_PARITY_BUTT,
} UARTHAL_PARITY;

typedef struct hiUARTHAL_ATTR
{
    UARTHAL_BIT_RATE bitRate;
    UARTHAL_DATA_BIT dataBits;
    UARTHAL_STOP_BIT stopBits;
    UARTHAL_PARITY parity; //Even (E), odd (O), no (N)
}UARTHAL_ATTR;

HI_S32 HAL_UART_Init(HI_CHAR *uartDev, HI_S32 *uartFd);

HI_S32 HAL_UART_SetAttr(HI_S32 uartFd, UARTHAL_ATTR *uartAttr);

HI_S32 HAL_UART_DeInit(HI_S32 uartFd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_UART__H__*/

