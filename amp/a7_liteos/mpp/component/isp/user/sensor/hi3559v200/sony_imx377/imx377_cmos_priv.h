/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : IMX377_cmos_priv.h
  Version       : Initial Draft
  Author        : Hisilicon BVT PQ group
  Created       : 2017/05/08
  Description   : this file was private for IMX377 slave mode sensor
  History       :
  1.Date        :
    Author      :
    Modification: Created file
******************************************************************************/
#ifndef __IMX377_CMOS_PRIV_H_
#define __IMX377_CMOS_PRIV_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// usefull macro
#define HIGH_8BITS(x) (((x) & 0xff00) >> 8)
#define LOW_8BITS(x)  ((x) & 0x00ff)
#ifndef MAX
#define MAX(a, b) (((a) < (b)) ?  (b) : (a))
#endif

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ?  (b) : (a))
#endif

#define IMX377_RES_IS_8M(w, h)    ((4096 >= (w)) && (2160 >= (h)))
#define IMX377_RES_IS_2M(w, h)    ((2048 >= (w)) && (1080 >= (h)))
#define IMX377_RES_IS_1M(w, h)    ((1364 >= (w)) && (720 >= (h)))
#define IMX377_RES_IS_12M(w, h)   ((4100 >= (w)) && (3000 >= (h)))

#define CHECK_RET(express)\
    do{\
        HI_S32 s32Ret;\
        s32Ret = express;\
        if (HI_SUCCESS != s32Ret)\
        {\
            printf("Failed at %s: LINE: %d with %#x!\n", __FUNCTION__, __LINE__, s32Ret);\
        }\
    }while(0)

#define     IMX377_SHR_L                (0x300b)
#define     IMX377_SHR_H                (0x300c)
#define     IMX377_PGC_L                (0x3009)
#define     IMX377_PGC_H                (0x300a)
#define     IMX377_SVR_L                (0x300d)
#define     IMX377_SVR_H                (0x300e)
#define     IMX377_DGAIN                (0x3011)
#define     IMX377_VMAX                 (0x30F7)

//sensor gain
#define IMX377_AGAIN_MAX    (21504)     //the max again is 21845
#define IMX377_DGAIN_1X     (1024)
#define IMX377_DGAIN_2X     (2048)
#define IMX377_DGAIN_4X     (4096)
#define IMX377_DGAIN_8X     (8192)


typedef struct hiIMX377_SENSOR_REG_S
{

    HI_U16 u16Addr;
    HI_U8  u8Data;
} IMX377_SENSOR_REG_S;

typedef enum
{
    IMX377_8M_30FPS_10BIT_LINEAR_MODE                = 0,
    IMX377_8M_60FPS_10BIT_LINEAR_MODE                = 1,
    IMX377_1080P_120FPS_12BIT_LINEAR_MODE_WEIGHT     = 2,
    IMX377_1080P_120FPS_12BIT_LINEAR_MODE_NORMAL     = 3,
    IMX377_720P_240FPS_10BIT_LINEAR_MODE             = 4,
    IMX377_720x1364P_300FPS_12BIT_LINEAR_MODE        = 5,
    IMX377_12M_30FPS_12BIT_LINEAR_MODE               = 6,
    IMX377_8M_30FPS_12BIT_LINEAR_MODE                = 7,
    IMX377_12M_20FPS_10BIT_LINEAR_MODE               = 8,
    IMX377_MODE_BUTT

} IMX377_RES_MODE_E;

typedef struct hiIMX377_VIDEO_MODE_TBL_S
{
    HI_U32 u32VMax;
    HI_FLOAT fOffset;
    HI_U32 u32ExpLineMin;
    HI_U32 u32ExpLineLimit;
    HI_U32 u32MaxFps;
    HI_U8 u8SnsMode;
    const char *pszModeName;

} IMX377_VIDEO_MODE_TBL_S;



#endif /* __IMX377_CMOS_PRIV_H_ */
