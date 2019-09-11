/**
* Copyright (C), 2017-2018, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file    adpt_vcap_inner_config.c
* @brief   NDK mapi functions declaration
* @author  HiMobileCam NDK develop team
* @date    2018/2/7
*/
#include "hi_type.h"
#include "adpt_vcap_inner_config_define.h"
#include "sensor_interface_cfg_params.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_VOID MAPI_VCAP_LoadSensorParam(VCAP_INNER_CONFIG_ATTR_S *pstVcap, HI_U32 u32Index)
{
    pstVcap->astDevAttr[u32Index].au32ComponentMask[0] = 0xFFF00000;
    pstVcap->astDevAttr[u32Index].au32ComponentMask[1] = 0x0;

    pstVcap->astDevAttr[u32Index].bDataReverse = HI_FALSE;
    pstVcap->astDevAttr[u32Index].enDataSeq = VI_DATA_SEQ_YUYV;
    pstVcap->astDevAttr[u32Index].enInputDataType = VI_DATA_TYPE_RGB;
    pstVcap->astDevAttr[u32Index].enIntfMode = VI_MODE_MIPI;
    pstVcap->astDevAttr[u32Index].enScanMode = VI_SCAN_PROGRESSIVE;
    pstVcap->astDevAttr[u32Index].enWorkMode = VI_WORK_MODE_1Multiplex;

    pstVcap->astDevAttr[u32Index].stBasAttr.stRephaseAttr.enHRephaseMode = VI_REPHASE_MODE_NONE;
    pstVcap->astDevAttr[u32Index].stBasAttr.stRephaseAttr.enVRephaseMode = VI_REPHASE_MODE_NONE;

    pstVcap->astDevAttr[u32Index].stSynCfg.enVsync = VI_VSYNC_PULSE;
    pstVcap->astDevAttr[u32Index].stSynCfg.enVsyncNeg = VI_VSYNC_NEG_LOW;
    pstVcap->astDevAttr[u32Index].stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;
    pstVcap->astDevAttr[u32Index].stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;
    pstVcap->astDevAttr[u32Index].stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;
    pstVcap->astDevAttr[u32Index].stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;
}

HI_VOID MAPI_VCAP_LoadBt656Param(VCAP_INNER_CONFIG_ATTR_S *pstVcap, HI_U32 u32Index)
{
    pstVcap->astDevAttr[u32Index].au32ComponentMask[0] = 0x00FF0000;
    pstVcap->astDevAttr[u32Index].au32ComponentMask[1] = 0x0;

    pstVcap->astDevAttr[u32Index].bDataReverse = HI_FALSE;
    pstVcap->astDevAttr[u32Index].enDataSeq = VI_DATA_SEQ_UYVY;
    pstVcap->astDevAttr[u32Index].enInputDataType = VI_DATA_TYPE_YUV;
    pstVcap->astDevAttr[u32Index].enIntfMode = VI_MODE_BT656;
    pstVcap->astDevAttr[u32Index].enScanMode = VI_SCAN_PROGRESSIVE;
    pstVcap->astDevAttr[u32Index].enWorkMode = VI_WORK_MODE_1Multiplex;

    pstVcap->astDevAttr[u32Index].stBasAttr.stRephaseAttr.enHRephaseMode = VI_REPHASE_MODE_NONE;
    pstVcap->astDevAttr[u32Index].stBasAttr.stRephaseAttr.enVRephaseMode = VI_REPHASE_MODE_NONE;

    pstVcap->astDevAttr[u32Index].stSynCfg.enVsync = VI_VSYNC_PULSE;
    pstVcap->astDevAttr[u32Index].stSynCfg.enVsyncNeg = VI_HSYNC_NEG_HIGH;
    pstVcap->astDevAttr[u32Index].stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;
    pstVcap->astDevAttr[u32Index].stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;
    pstVcap->astDevAttr[u32Index].stSynCfg.enVsyncValid = VI_VSYNC_PULSE;
    pstVcap->astDevAttr[u32Index].stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;
}

HI_VOID MAPI_VCAP_LoadParam(VCAP_INNER_CONFIG_ATTR_S *pstVcap, const HI_S32 s32SensorType[])
{
    HI_U32 i;

    for (i = 0; i <= HI_MAPI_VCAP_MAX_DEV_NUM; i++) {
        pstVcap->astDevAttr[i].as32AdChnId[0] = -1;
        pstVcap->astDevAttr[i].as32AdChnId[1] = -1;
        pstVcap->astDevAttr[i].as32AdChnId[2] = -1;
        pstVcap->astDevAttr[i].as32AdChnId[3] = -1;

        pstVcap->astDevAttr[i].stSynCfg.stTimingBlank.u32HsyncHfb = 0;
        pstVcap->astDevAttr[i].stSynCfg.stTimingBlank.u32HsyncAct = 1280;
        pstVcap->astDevAttr[i].stSynCfg.stTimingBlank.u32HsyncHbb = 0;
        pstVcap->astDevAttr[i].stSynCfg.stTimingBlank.u32VsyncVfb = 0;
        pstVcap->astDevAttr[i].stSynCfg.stTimingBlank.u32VsyncVact = 720;
        pstVcap->astDevAttr[i].stSynCfg.stTimingBlank.u32VsyncVact = 0;
        pstVcap->astDevAttr[i].stSynCfg.stTimingBlank.u32VsyncVbfb = 0;
        pstVcap->astDevAttr[i].stSynCfg.stTimingBlank.u32VsyncVbact = 0;
        pstVcap->astDevAttr[i].stSynCfg.stTimingBlank.u32VsyncVbact = 0;
        if (s32SensorType[i] == BT656) {
            MAPI_VCAP_LoadBt656Param(pstVcap, i);
        } else {
            MAPI_VCAP_LoadSensorParam(pstVcap, i);
        }
    }

    for (i = 0; i < HI_MAPI_VCAP_MAX_PIPE_NUM; i++) {
        pstVcap->astPipeAttr[i].bDiscardProPic = HI_TRUE;
        pstVcap->astPipeAttr[i].bNrEn = HI_FALSE;
        pstVcap->astPipeAttr[i].bSharpenEn = HI_FALSE;
        pstVcap->astPipeAttr[i].bYuvSkip = HI_FALSE;
        pstVcap->astPipeAttr[i].enBitWidth = DATA_BITWIDTH_12;
        pstVcap->astPipeAttr[i].enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
        pstVcap->astPipeAttr[i].enPipeBypassMode = VI_PIPE_BYPASS_NONE;

        pstVcap->astPipeAttr[i].stNrAttr.enBitWidth = DATA_BITWIDTH_8;
        pstVcap->astPipeAttr[i].stNrAttr.enCompressMode = COMPRESS_MODE_FRAME;
        pstVcap->astPipeAttr[i].stNrAttr.enNrRefSource = VI_NR_REF_FROM_RFR;
        pstVcap->astPipeAttr[i].stNrAttr.enPixFmt = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }

    for (i = 0; i < HI_MAPI_PIPE_MAX_CHN_NUM; i++) {
        pstVcap->astChnAttr[i].bMirror = HI_FALSE;
        pstVcap->astChnAttr[i].bFlip = HI_FALSE;
        pstVcap->astChnAttr[i].enDynamicRange = DYNAMIC_RANGE_SDR8;
        pstVcap->astChnAttr[i].enVideoFormat = VIDEO_FORMAT_LINEAR;
        pstVcap->astChnAttr[i].u32Depth = 0;
        pstVcap->astChnAttr[i].enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
