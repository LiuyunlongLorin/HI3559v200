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
#ifndef __HI_HAL_GPS_H__
#define __HI_HAL_GPS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "hi_type.h"

#define HI_HAL_GPS_DATA_SIZE 1

typedef struct hiGPSDATA
{
    HI_U32 wantReadLen;   /**want read length */
    HI_U32 actualReadLen; /**actual read length */
    HI_U8  rawData[HI_HAL_GPS_DATA_SIZE];
} HI_GPSDATA;

HI_S32 HI_HAL_GPS_Init(HI_VOID);

HI_S32 HI_HAL_GPS_Deinit(HI_VOID);

HI_S32 HI_HAL_GPS_GetRawData(HI_GPSDATA *gpsData);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_GPS_H__*/
