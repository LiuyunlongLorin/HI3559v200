/**
* @file    hi_gps.c
* @brief   product gps interface
*
* Copyright (c) 2018 Huawei Tech.Co.,Ltd
*
* @author    Huawei team
* @date      2018/12/19
* @version

*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include<errno.h>

#include "hi_appcomm_util.h"
#include "hi_appcomm_log.h"
#include "hi_hal_gps_inner.h"
#include "hi_hal_uart_inner.h"
#include "hi_hal_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static HI_S32 g_uartFd = HAL_FD_INITIALIZATION_VAL;

static HI_S32 GpsDeviceInit(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;

#ifdef BOARD_DASHCAM_REFB
#define HAL_GPS_DEV "/dev/ttyAMA2"
    /* pinmux config UART4 for Hi3559V200 */
    himm(0x111f0000, 0x532); //RX
    himm(0x111f0004, 0x432); //TX

#else
#define HAL_GPS_DEV "/dev/ttyAMA2"
    /* pinmux config UART1 for Hi3559V200 */
    himm(0x111f0008, 0x531); //RX
    himm(0x111f000c, 0x431); //TX

#endif

    if (g_uartFd != HAL_FD_INITIALIZATION_VAL)
    {
        MLOGI("GPS already init\n");
        return HI_SUCCESS;
    }

    ret = HAL_UART_Init(HAL_GPS_DEV, &g_uartFd);
    if(HI_SUCCESS != ret)
    {
        MLOGI("uart init failed\n");
        return ret;
    }

    UARTHAL_ATTR uartAttr = {0};
    uartAttr.bitRate= HAL_UART_BITRATE_9600;
    uartAttr.dataBits= HAL_UART_DATABIT_8;
    uartAttr.stopBits= HAL_UART_STOPBIT_1;
    uartAttr.parity= HAL_UART_PARITY_N;

    ret = HAL_UART_SetAttr(g_uartFd, &uartAttr);
    if(HI_SUCCESS != ret)
    {
        MLOGI("uart set attr failed\n");
        return ret;
    }

    return HI_SUCCESS;
}

static HI_S32 GpsDeviceDeInit(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    if (g_uartFd == HAL_FD_INITIALIZATION_VAL)
    {
        MLOGE("Device has already deinit or Uninitialized!\n");
        return HI_SUCCESS;
    }

    ret = HAL_UART_DeInit(g_uartFd);
    if(HI_SUCCESS != ret)
    {
        MLOGI("uart Deinit failed\n");
        return ret;
    }

    g_uartFd = HAL_FD_INITIALIZATION_VAL;
    return HI_SUCCESS;
}

static HI_S32 GpsGetRawData(HI_GPSDATA *gpsData)
{
    HI_S32 ret = HI_SUCCESS;

    if(NULL == gpsData)
    {
       MLOGE("gpsData is NULL pointer!\n");
       return HI_HAL_EINVAL;
    }

    if(gpsData->wantReadLen > HI_HAL_GPS_DATA_SIZE)
    {
        MLOGE("Too much data acquired at one time len:%d!\n", gpsData->wantReadLen);
        return HI_HAL_EINVAL;
    }

    if(g_uartFd == HAL_FD_INITIALIZATION_VAL)
    {
        return HI_HAL_ENOINIT;
    }

    ret = read(g_uartFd, gpsData->rawData, gpsData->wantReadLen);

    if (ret < 0)
    {
        if (EAGAIN == errno)
        {
            MLOGE("No data readable!\n");
            gpsData->actualReadLen = 0;
            return HI_SUCCESS;
        }else
        {
            perror("read data faild!\n");
            return HI_HAL_EINVOKESYS;
        }
    }

    gpsData->actualReadLen = ret;
    return HI_SUCCESS;
}

HI_GPSHAL_DEVICE g_gpsDevice =
{
    .Init = GpsDeviceInit,
    .DeInit = GpsDeviceDeInit,
    .GetRawData = GpsGetRawData,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
