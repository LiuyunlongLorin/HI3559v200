/************************************************************************
*             Copyright (C)  2018, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: hi_comm_motionfusion.h
* Description: init draft
*
*************************************************************************/
#ifndef __HI_COMM_MOTIONFUSION_H__
#define __HI_COMM_MOTIONFUSION_H__

#include "hi_type.h"
#include "hi_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define                                          EN_ERR_GYRO_NOTWORK  20
#define                                          EN_ERR_ACC_NOTWORK   21
#define                                          EN_ERR_MODE          22
#define                                          EN_ERR_USECASE       23

#define HI_ERR_MOTIONFUSION_NOT_CONFIG           HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)
#define HI_ERR_MOTIONFUSION_NOBUF                HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
#define HI_ERR_MOTIONFUSION_BUF_EMPTY            HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
#define HI_ERR_MOTIONFUSION_NULL_PTR             HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define HI_ERR_MOTIONFUSION_ILLEGAL_PARAM        HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define HI_ERR_MOTIONFUSION_BUF_FULL             HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
#define HI_ERR_MOTIONFUSION_SYS_NOTREADY         HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define HI_ERR_MOTIONFUSION_NOT_SUPPORT          HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
#define HI_ERR_MOTIONFUSION_NOT_PERMITTED        HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define HI_ERR_MOTIONFUSION_BUSY                 HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define HI_ERR_MOTIONFUSION_INVALID_CHNID        HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
#define HI_ERR_MOTIONFUSION_CHN_UNEXIST          HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
#define HI_ERR_MOTIONFUSION_GYRO_NOTWORK         HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_GYRO_NOTWORK)
#define HI_ERR_MOTIONFUSION_ACC_NOTWORK          HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_ACC_NOTWORK)
#define HI_ERR_MOTIONFUSION_INVALID_MODE         HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_MODE)
#define HI_ERR_MOTIONFUSION_INVALID_USECASE      HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_USECASE)

#define MFUSION_MAX_CHN_NUM 2

#define HI_TRACE_MOTIONFUSION(level, fmt, ...)\
    do{ \
        HI_TRACE(level, HI_ID_MOTIONFUSION,"[Func]:%s [Line]:%d [Info]:"fmt,__FUNCTION__, __LINE__,##__VA_ARGS__);\
    }while(0)

#define AXIS_NUM         3
#define MATRIX_NUM       9
#define MATRIX_TEMP_NUM  9
#define TEMP_LUT_SAMPLES 30

#define MFUSION_TEMP_GYRO  0x1
#define MFUSION_TEMP_ACC   0x2
#define MFUSION_TEMP_MAGN  0x4
#define MFUSION_TEMP_ALL  0x7

#define MFUSION_DEVICE_GYRO 0x1
#define MFUSION_DEVICE_ACC  0x2
#define MFUSION_DEVICE_MAGN 0x4
#define MFUSION_DEVICE_ALL  0x7

#define SIXSIDE_MATRIX_GRADINT  15
#define ZERO_OFFSET_GRADINT  15
#define TEMP_OFFSET_GRADINT  15

#define MFUSION_COMM_BUFFER_SIZE 128
#define MFUSION_LUT_STATUS_NUM   2

typedef HI_S32 IMU_DRIFT[AXIS_NUM];
typedef HI_S32 IMU_MATRIX[MATRIX_NUM];
typedef HI_S32 IMU_MATRIX_TEMP[MATRIX_TEMP_NUM];

typedef struct hiMFUSION_STEADY_DETECT_ATTR_S
{
    HI_U32 u32SteadyTimeThr;                           /* RW; continues steady time (in sec) threshold for steady detection*/

    HI_S32 s32GyroOffset;                              /* RW; max GYRO ZRO tolerance presented in datasheet,
                                                                                                   with (ADC Word Length - 1) decimal bits*/
    HI_S32 s32AccOffset;                               /* RW; max ACC ZRO tolerance presented in datasheet,
                                                                                                   with (ADC Word Length - 1) decimal bits*/
    HI_S32 s32GyroRMS;                                 /* RW; gyro RMS noise of under the current filter BW,
                                                                                                   with (ADC Word Length - 1) decimal bits*/
    HI_S32 s32AccRMS;                                  /* RW; ACC RMS noise of under the current filter BW
                                                                                                   with (ADC Word Length - 1) decimal bits*/
    HI_S32 s32GyroOffsetFactor;                        /* RW; scale factor of GyroOffset for steady detection,
                                                                                                   larger -> higher recall, but less the precision*/
    HI_S32 s32AccOffsetFactor;                         /* RW; scale factor of AccOffset for steady detection,
                                                                                                   larger -> higher recall, but less the precision*/
    HI_S32 s32GyroRMSFactor;                           /* RW; scale factor of GyroRMS for steady detection,
                                                                                                   larger -> higher recall, but less the precision*/
    HI_S32 s32AccRMSFactor;                            /* RW; scale factor of AccRMS for steady detection,
                                                                                                   larger -> higher recall, but less the precision*/
} MFUSION_STEADY_DETECT_ATTR_S;

typedef struct hiMFUSION_ATTR_S
{
    HI_U32            u32DeviceMask;        /*Device Mask: Gyro,Acc or Magn */
    HI_U32            u32TemperatureMask;   /*Temperature Mask: Gyro Temperature ,Acc Temperatureor Magn Temperature */
    MFUSION_STEADY_DETECT_ATTR_S  stSteadyDetectAttr;
} MFUSION_ATTR_S;

/*angle data per sample*/
typedef struct hiMFUSION_SAMPLE_DATA_S
{
    HI_S32      s32XData;
    HI_S32      s32YData;
    HI_S32      s32ZData;
    HI_S32      s32Temp;
    HI_U64      u64PTS;
} MFUSION_SAMPLE_DATA_S;

typedef struct hiMFUSION_GYRO_BUFFER_S
{
    MFUSION_SAMPLE_DATA_S astGyroData[MFUSION_COMM_BUFFER_SIZE];   /* RW; Gyro data  */
    HI_U32           u32BuffDataNum;                               /* RW; data length of the occupied buffer */
    HI_U32           u32BuffRepNum;                                /* RW; data start position of invalid, incase of PTS overlap */
} MFUSION_GYRO_BUFFER_S;

typedef struct hiMFUSION_ACC_BUFFER_S
{
    MFUSION_SAMPLE_DATA_S astAccData[MFUSION_COMM_BUFFER_SIZE];
    HI_U32                u32BuffDataNum;                          /* RW; data length of the occupied buffer */
    HI_U32                u32BuffRepNum;                           /* RW; data start position of invalid, incase of PTS overlap */
} MFUSION_ACC_BUFFER_S;

typedef enum hiMFUSION_TEMP_DRIFT_MODE_E
{
    IMU_TEMP_DRIFT_CURV = 0,                                       /* Polynomial mode */
    IMU_TEMP_DRIFT_LUT,                                            /* Lookup Table mode */
    IMU_TEMP_DRIFT_BUTT
} MFUSION_TEMP_DRIFT_MODE_E;

typedef struct hiMFUSION_TEMP_DRIFT_LUT_S
{
    HI_S32 as32IMULut[TEMP_LUT_SAMPLES][AXIS_NUM];                       /* RW;Temperature Drift Lookup Table  */
    HI_S32 as32GyroLutStatus[TEMP_LUT_SAMPLES][MFUSION_LUT_STATUS_NUM];  /* RW;the 1st col is the time (in sec) for which the
                                                                                                                                 sample has not been updated. the 2nd col is the
                                                                                                                                 nearest temperature sample during update*/
    HI_S32 s32RangeMin;                                                  /* RW;Temperature Range Minimum  */
    HI_S32 s32RangeMax;                                                  /* RW;Temperature Range Maximum  */
    HI_U32 u32Step;
} MFUSION_TEMP_DRIFT_LUT_S;

typedef struct hiMFUSION_TEMP_DRIFT_S
{
    MFUSION_TEMP_DRIFT_MODE_E enMode;                              /* RW;Temperature Drift mode  */
    union
    {
        IMU_MATRIX_TEMP aTempMatrix;                               /* RW;Temperature Drift Polynomial Matrix data */
        MFUSION_TEMP_DRIFT_LUT_S stTempLut;                        /* RW;Temperature Drift  Lookup Table data */
    };
} MFUSION_TEMP_DRIFT_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

