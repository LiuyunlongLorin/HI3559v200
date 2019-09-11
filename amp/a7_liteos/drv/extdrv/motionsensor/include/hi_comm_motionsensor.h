/******************************************************************************

Copyright (C), 2018, Hisilicon Tech. Co., Ltd.

******************************************************************************
File Name     : hi_comm_motionsensor.h
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2018/07/30
Description   :
History       :
1.Date        : 2018/07/30
Author        : 
Modification: Created file

******************************************************************************/
#ifndef _HI_COMM_MOTIONSENSOR_H__
#define _HI_COMM_MOTIONSENSOR_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define MAX_DATA_NUM        128
#define GRADIENT            (0x1<<10)

#define MSENSOR_TEMP_GYRO   0x1
#define MSENSOR_TEMP_ACC    0x2
#define MSENSOR_TEMP_MAGN   0x4
#define MSENSOR_TEMP_ALL    0x7

#define MSENSOR_DEVICE_GYRO 0x1 
#define MSENSOR_DEVICE_ACC  0x2
#define MSENSOR_DEVICE_MAGN 0x4
#define MSENSOR_DEVICE_ALL  0x7

typedef struct hiMSENSOR_ATTR_S
{
    HI_U32            u32DeviceMask;
    HI_U32            u32TemperatureMask;
} MSENSOR_ATTR_S;

typedef struct hiMSENSOR_BUF_ATTR_S
{
    HI_U64   u64PhyAddr;
    HI_VOID* pVirAddr; 
    HI_U32   u32Buflen;
}MSENSOR_BUF_ATTR_S;

typedef struct hiMSENSOR_SAMPLE_DATA_S
{
    HI_S32 s32XData;
    HI_S32 s32YData;
    HI_S32 s32ZData;
    HI_S32 s32Temp;
    HI_U64 u64PTS;
} MSENSOR_SAMPLE_DATA_S;

typedef struct hiMSENSOR_GYRO_BUFFER_S
{
    MSENSOR_SAMPLE_DATA_S astGyroData[MAX_DATA_NUM];
    HI_U32                u32BuffDataNum;                                // data num in buffer
} MSENSOR_GYRO_BUFFER_S;

typedef struct hiMSENSOR_ACC_BUFFER_S
{
    MSENSOR_SAMPLE_DATA_S astAccData[MAX_DATA_NUM];
    HI_U32                u32BuffDataNum;                          //data num in buffer
} MSENSOR_ACC_BUFFER_S;

typedef struct hiMSENSOR_MAGN_BUFFER_S
{
    MSENSOR_SAMPLE_DATA_S astMagnData[MAX_DATA_NUM];
    HI_U32                u32BuffDataNum;                          //data num in buffer
} MSENSOR_MAGN_BUFFER_S;


typedef struct hiGYRO_CONFIG_S
{
    HI_U64 u64ODR;
    HI_U64 u64FSR;
    HI_U8  u8DataWidth;
    HI_S32 s32TempMax;
	HI_S32 s32TempMin;
} GYRO_CONFIG_S;


typedef struct hiACC_CONFIG_S
{
    HI_U64 u64ODR;
    HI_U64 u64FSR;
	HI_U8  u8DataWidth;
	HI_S32 s32TempMax;
	HI_S32 s32TempMin;	
} ACC_CONFIG_S;

typedef struct hiMAGN_CONFIG_S
{
    HI_U64 u64ODR;
	HI_U64 u64FSR;
	HI_U8  u8DataWidth;
	HI_S32 s32TempMax;
	HI_S32 s32TempMin;	
} MAGN_CONFIG_S;
 

typedef struct hiMSENSOR_CONFIG_S
{
	GYRO_CONFIG_S           stGyroConfig;
	ACC_CONFIG_S            stAccConfig;
	MAGN_CONFIG_S           stMagnConfig;
} MSENSOR_CONFIG_S;

typedef struct hiMSENSOR_PARAM_S
{
	MSENSOR_BUF_ATTR_S      stMSensorBufAttr;
    MSENSOR_CONFIG_S        stMSensorConfig;
    MSENSOR_ATTR_S          stMSensorAttr;
} MSENSOR_PARAM_S;


typedef enum hiMSENSOR_DATA_TYPE_E
{
    MSENSOR_DATA_GYRO = 0,
    MSENSOR_DATA_ACC,
    MSENSOR_DATA_MAGN,
    MSENSOR_DATA_BUTT
} MSENSOR_DATA_TYPE_E;

typedef struct hiMSENSOR_DATA_ADDR_S
{
    HI_S32* ps32XPhyAddr;
    HI_S32* ps32YPhyAddr;
    HI_S32* ps32ZPhyAddr;
    HI_S32* ps32TempPhyAddr;
    HI_U64* pu64PTSPhyAddr;
	HI_U32  u32Num;      /* Number of valid data */
} MSENSOR_DATA_ADDR_S;

typedef struct hiMSENSOR_DATA_INFO_S
{
    HI_S32                   s32ID;
    MSENSOR_DATA_TYPE_E      enDataType;
    MSENSOR_DATA_ADDR_S      astMSensorData[2];
    HI_U64                   u64BeginPts;
    HI_U64                   u64EndPts;
    HI_S64                   s64AddrOffset;
} MSENSOR_DATA_INFO_S;


typedef struct hiMSENSOR_DATA_S
{
   MSENSOR_ATTR_S              stMSensorAttr;
   MSENSOR_GYRO_BUFFER_S       stMsensorGyroBuffer;
   MSENSOR_ACC_BUFFER_S        stMsensorAccBuffer;
   MSENSOR_MAGN_BUFFER_S       stMsensorMagnBuffer;
}MSENSOR_DATA_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif


