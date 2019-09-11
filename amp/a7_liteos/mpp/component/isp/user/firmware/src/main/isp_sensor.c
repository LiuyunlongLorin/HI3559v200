/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_sensor.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/05/06
  Description   :
  History       :
  1.Date        : 2013/05/06
    Author      :
    Modification: Created file

******************************************************************************/

#include "isp_sensor.h"
#include "yuv_cmos_ex.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct hiISP_SENSOR_S {
    ISP_SNS_ATTR_INFO_S     stSnsAttrInfo;
    ISP_SENSOR_REGISTER_S   stRegister;
    ISP_CMOS_DEFAULT_S      stSnsDft;
    ISP_CMOS_BLACK_LEVEL_S  stSnsBlackLevel;    /* some sensors's black level will be changed with iso */
    ISP_SNS_REGS_INFO_S     stSnsRegInfo;
    ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsImageMode;
} ISP_SENSOR_S;

ISP_SENSOR_S *g_pastSensorCtx[ISP_MAX_PIPE_NUM] = { HI_NULL };

#define SENSOR_GET_CTX(dev, pstCtx) (pstCtx = g_pastSensorCtx[dev])
#define SENSOR_SET_CTX(dev, pstCtx) (g_pastSensorCtx[dev] = pstCtx)
#define SENSOR_RESET_CTX(dev)       (g_pastSensorCtx[dev] = HI_NULL)

HI_S32 ISP_SensorCtxInit(VI_PIPE ViPipe)
{
    ISP_SENSOR_S *pstSensorCtx = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensorCtx);

    if (HI_NULL == pstSensorCtx) {
        pstSensorCtx = (ISP_SENSOR_S *)ISP_MALLOC(sizeof(ISP_SENSOR_S));
        if (HI_NULL == pstSensorCtx) {
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] SensorCtx malloc memory failed!\n", ViPipe);
            return HI_ERR_ISP_NOMEM;
        }
    }

    memset(pstSensorCtx, 0, sizeof(ISP_SENSOR_S));

    SENSOR_SET_CTX(ViPipe, pstSensorCtx);

    return HI_SUCCESS;
}

HI_S32 ISP_SensorRegCallBack(VI_PIPE ViPipe, ISP_SNS_ATTR_INFO_S *pstSnsAttrInfo, ISP_SENSOR_REGISTER_S *pstRegister)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_SENSOR_S *pstSensor = HI_NULL;

    s32Ret = ISP_SensorCtxInit(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        return s32Ret;
    }

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    memcpy(&pstSensor->stSnsAttrInfo, pstSnsAttrInfo, sizeof(ISP_SNS_ATTR_INFO_S));
    memcpy(&pstSensor->stRegister, pstRegister, sizeof(ISP_SENSOR_REGISTER_S));

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_sensor_global_init) {
        pstSensor->stRegister.stSnsExp.pfn_cmos_sensor_global_init(ViPipe);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SensorUnRegCallBack(VI_PIPE ViPipe)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_FREE(pstSensor);
    SENSOR_RESET_CTX(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_SensorUpdateAll(VI_PIPE ViPipe)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_get_isp_default) {
        pstSensor->stRegister.stSnsExp.pfn_cmos_get_isp_default(ViPipe, &pstSensor->stSnsDft);
    } else {
        ISP_TRACE(HI_DBG_ERR, "Get isp[%d] default value error!\n", ViPipe);
        return HI_FAILURE;
    }

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_get_isp_black_level) {
        pstSensor->stRegister.stSnsExp.pfn_cmos_get_isp_black_level(ViPipe, &pstSensor->stSnsBlackLevel);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SensorUpdateAll_YUV(VI_PIPE ViPipe)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    s32Ret = ISP_GetYUVDefault(&pstSensor->stSnsDft);

    pstSensor->stSnsBlackLevel.au16BlackLevel[0] = 257;
    pstSensor->stSnsBlackLevel.au16BlackLevel[1] = 257;
    pstSensor->stSnsBlackLevel.au16BlackLevel[2] = 257;
    pstSensor->stSnsBlackLevel.au16BlackLevel[3] = 257;
    pstSensor->stSnsBlackLevel.bUpdate = HI_TRUE;

    return s32Ret;
}

HI_S32 ISP_SensorGetId(VI_PIPE ViPipe, SENSOR_ID *pSensorId)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    *pSensorId = pstSensor->stSnsAttrInfo.eSensorId;

    return HI_SUCCESS;
}

HI_S32 ISP_SensorGetBlc(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S **ppstSnsBlackLevel)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    *ppstSnsBlackLevel = &pstSensor->stSnsBlackLevel;

    return HI_SUCCESS;
}

HI_S32 ISP_SensorGetDefault(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S **ppstSnsDft)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    *ppstSnsDft = &pstSensor->stSnsDft;

    return HI_SUCCESS;
}

HI_S32 ISP_SensorGetMaxResolution(VI_PIPE ViPipe, ISP_CMOS_SENSOR_MAX_RESOLUTION_S **ppstSnsMaxResolution)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    *ppstSnsMaxResolution = &pstSensor->stSnsDft.stSensorMaxResolution;

    return HI_SUCCESS;
}

HI_S32 ISP_SensorGetSnsReg(VI_PIPE ViPipe, ISP_SNS_REGS_INFO_S **ppstSnsRegInfo)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    *ppstSnsRegInfo = &pstSensor->stSnsRegInfo;

    return HI_SUCCESS;
}

HI_S32 ISP_SensorInit(VI_PIPE ViPipe)
{
    HI_S8 s8SspDev;
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    /* if I2C or SSP Dev is -1, don't init sensor */
    {
        ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = NULL;

        ISP_SensorUpdateSnsReg(ViPipe);
        ISP_SensorGetSnsReg(ViPipe, &pstSnsRegsInfo);
        s8SspDev = pstSnsRegsInfo->unComBus.s8SspDev.bit4SspDev;

        if ((ISP_SNS_I2C_TYPE == pstSnsRegsInfo->enSnsType) &&
            (pstSnsRegsInfo->unComBus.s8I2cDev == -1)) {
            return HI_SUCCESS;
        }

        if ((ISP_SNS_SSP_TYPE == pstSnsRegsInfo->enSnsType) &&
            (s8SspDev == -1)) {
            return HI_SUCCESS;
        }
    }

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_sensor_init) {
        pstSensor->stRegister.stSnsExp.pfn_cmos_sensor_init(ViPipe);
    } else {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SensorSwitch(VI_PIPE ViPipe)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_sensor_init) {
        pstSensor->stRegister.stSnsExp.pfn_cmos_sensor_init(ViPipe);
    } else {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SensorExit(VI_PIPE ViPipe)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_sensor_exit) {
        pstSensor->stRegister.stSnsExp.pfn_cmos_sensor_exit(ViPipe);
    } else {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SensorUpdateBlc(VI_PIPE ViPipe)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_get_isp_black_level) {
        /* sensor should record the present iso, and calculate new black level. */
        pstSensor->stRegister.stSnsExp.pfn_cmos_get_isp_black_level(ViPipe, &pstSensor->stSnsBlackLevel);
    } else {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SensorUpdateDefault(VI_PIPE ViPipe)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_get_isp_default) {
        pstSensor->stRegister.stSnsExp.pfn_cmos_get_isp_default(ViPipe, &pstSensor->stSnsDft);
    } else {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SensorSetWDRMode(VI_PIPE ViPipe, HI_U8 u8Mode)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_set_wdr_mode) {
        if (HI_SUCCESS != pstSensor->stRegister.stSnsExp.pfn_cmos_set_wdr_mode(ViPipe, u8Mode)) {
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SensorSetImageMode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSnsImageMode)
{
    ISP_SENSOR_S *pstSensor = NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_set_image_mode) {
        return pstSensor->stRegister.stSnsExp.pfn_cmos_set_image_mode(ViPipe, pstSnsImageMode);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SensorSetPixelDetect(VI_PIPE ViPipe, HI_BOOL bEnable)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_set_pixel_detect) {
        pstSensor->stRegister.stSnsExp.pfn_cmos_set_pixel_detect(ViPipe, bEnable);
    } else {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SensorUpdateSnsReg(VI_PIPE ViPipe)
{
    ISP_SENSOR_S *pstSensor = HI_NULL;

    SENSOR_GET_CTX(ViPipe, pstSensor);
    ISP_CHECK_POINTER(pstSensor);

    if (HI_NULL != pstSensor->stRegister.stSnsExp.pfn_cmos_get_sns_reg_info) {
        pstSensor->stRegister.stSnsExp.pfn_cmos_get_sns_reg_info(ViPipe, &pstSensor->stSnsRegInfo);
    } else {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
