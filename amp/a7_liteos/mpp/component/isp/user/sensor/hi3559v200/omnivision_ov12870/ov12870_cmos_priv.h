/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : OV12870_cmos_priv.h
  Version       : Initial Draft
  Author        : Hisilicon BVT PQ group
  Created       : 2017/05/08
  Description   : this file was private for OV12870 master mode sensor
  History       :
  1.Date        :
    Author      :
    Modification: Created file
******************************************************************************/
#ifndef __OV12870_CMOS_PRIV_H_
#define __OV12870_CMOS_PRIV_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// usefull macro
#define HIGHER_8BITS(x) (((x) & 0xff0000) >> 16)
#define HIGH_8BITS(x) (((x) & 0xff00) >> 8)
#define LOW_8BITS(x)  ((x) & 0x00ff)
#ifndef MAX
#define MAX(a, b) (((a) < (b)) ?  (b) : (a))
#endif

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ?  (b) : (a))
#endif

#define OV12870_RES_IS_8M(w, h)    ((3840 >= (w)) && (2160 >= (h)))
#define OV12870_RES_IS_2M(w, h)    ((1920 >= (w)) && (1080 >= (h)))
#define OV12870_RES_IS_1M(w, h)    ((1280 >= (w)) && (720 >= (h)))
#define OV12870_RES_IS_12M(w, h)   ((4000 >= (w)) && (3000 >= (h)))
#define OV12870_RES_IS_4M(w, h)    ((2716 >= (w)) && (1524 >= (h)))

#define CHECK_RET(express)\
    do{\
        HI_S32 s32Ret;\
        s32Ret = express;\
        if (HI_SUCCESS != s32Ret)\
        {\
            printf("Failed at %s: LINE: %d with %#x!\n", __FUNCTION__, __LINE__, s32Ret);\
        }\
    }while(0)

#define     OV12870_VMAX_H                             (0x380e)
#define     OV12870_VMAX_L                             (0x380f)


#define     OV12870_COARSE_INTEG_TIME_L                (0x3502)
#define     OV12870_COARSE_INTEG_TIME_M                (0x3501)
#define     OV12870_COARSE_INTEG_TIME_H                (0x3500)
#define     OV12870_ANA_GAIN_GLOBAL_L_0                (0x3509)
#define     OV12870_ANA_GAIN_GLOBAL_H_0                (0x3508)
#define     OV12870_ANA_GAIN_GLOBAL_L_1                (0x350d)
#define     OV12870_ANA_GAIN_GLOBAL_H_1                (0x350c)

#define     OV12870_LONG_EXPOSURE_0                    (0x4202)
#define     OV12870_LONG_EXPOSURE_1                    (0x3410)
#define     OV12870_LONG_EXPOSURE_2                    (0x3412)
#define     OV12870_LONG_EXPOSURE_3                    (0x3413)
#define     OV12870_LONG_EXPOSURE_4                    (0x3414)
#define     OV12870_LONG_EXPOSURE_5                    (0x3415)


typedef struct hiOV12870_SENSOR_REG_S
{
    HI_U16 u16Addr;
    HI_U8  u8Data;
} OV12870_SENSOR_REG_S;

typedef enum
{
    OV12870_1M_240FPS_10BIT_LINEAR_MODE                = 0,
    OV12870_2M_120FPS_10BIT_LINEAR_MODE                = 1,
    OV12870_8M_30FPS_10BIT_LINEAR_MODE                 = 2,
    OV12870_12M_30FPS_10BIT_LINEAR_MODE                = 3,
    OV12870_MODE_BUTT

} OV12870_RES_MODE_E;

typedef struct hiOV12870_VIDEO_MODE_TBL_S
{
    HI_U32 u32VMax;
    HI_U32 u32Offset;
    HI_U32 u32ExpLineMin;
    HI_U32 u32ExpLineLimit;
    HI_FLOAT f32MaxFps;
    HI_U8 u8SnsMode;
    const char *pszModeName;

} OV12870_VIDEO_MODE_TBL_S;



#endif /* __OV12870_CMOS_PRIV_H_ */
