/**
* @file    hi_hal_key.h
* @brief   product hal key interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    Huawei team
* @date      2018/12/19
* @version

*/
#ifndef __HI_HAL_GPS_INNER_H__
#define __HI_HAL_GPS_INNER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#include "hi_hal_gps.h"

typedef struct hiGPSHAL_DEVICE
{
   HI_S32 (*Init)(HI_VOID);
   HI_S32 (*DeInit)(HI_VOID);
   HI_S32 (*GetRawData)(HI_GPSDATA *gpsData);
}HI_GPSHAL_DEVICE;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_GPS_INNER_H__*/
