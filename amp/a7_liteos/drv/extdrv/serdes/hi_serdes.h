/******************************************************************************
  Copyright (C), 2019, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
  File Name     : hi_serdes.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2019/02/26
  Description   : Common Def Of serdes
  History       :
  1.Date        : 2019/02/26
    Author      :
    Modification: Created file
******************************************************************************/

#ifndef __HI_SERDES_H__
#define __HI_SERDES_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*End of #ifdef __cplusplus*/

#define SERDES_DEVICE_TYPE  "241-242"
#define SERDES_DEVICE_ADDR  0x16 /*8bit*/
#define SENSOR_ADDR_BYTE    2
#define SENSOR_DATA_BYTE    1

#define I2C_TM_SCL_TMO      200

#define ISP_MAX_DEV         4

typedef struct hiSERDES_CONFIG_S
{
    HI_U8 txPllValue[6];
    HI_U8 txDPhyValue;
} SERDES_CONFIG_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*End of #ifdef __cplusplus*/

#endif /*End of #ifdef __HI_SERDES_H__*/
