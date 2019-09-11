/**
* @file    hi_hal_gps.c
* @brief   product gps interface
*
* Copyright (c) 2018 Huawei Tech.Co.,Ltd
*
* @author    Huawei team
* @date      2018/12/19
* @version

*/
#include "hi_appcomm_log.h"
#include "hi_hal_gps.h"
#include "hi_hal_gps_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

extern HI_GPSHAL_DEVICE  g_gpsDevice;

HI_S32 HI_HAL_GPS_Init(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;

    ret = g_gpsDevice.Init();
    if(HI_SUCCESS != ret)
    {
        MLOGE("Gps Device Init failed\n");
        return ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_GPS_Deinit(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;

    ret = g_gpsDevice.DeInit();

    if (HI_SUCCESS != ret)
    {
        MLOGE("Gps Device DeInit failed\n");
        return ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_GPS_GetRawData(HI_GPSDATA *gpsData)
{
    HI_S32 ret = HI_SUCCESS;

    ret = g_gpsDevice.GetRawData(gpsData);
    if (HI_SUCCESS != ret)
    {
        MLOGE("Gps Device Get RawData failed\n");
        return ret;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
