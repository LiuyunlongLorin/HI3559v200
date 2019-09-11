/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_hdmi_define.h
 * @brief   hdmi module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_HDMI_DEFINE_H__
#define __HI_MAPI_HDMI_DEFINE_H__

#include "hi_comm_vo.h"
#include "hi_comm_aio.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_comm_define.h"
#include "hi_comm_hdmi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HDMI */
/** @{ */ /** <!-- [HDMI] */

#define HI_MAPI_HDMI_ENULL_PTR      HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NULL_PTR)
#define HI_MAPI_HDMI_ENOTREADY      HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_SYS_NOTREADY)
#define HI_MAPI_HDMI_EINVALID_DEVID \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_DEVID)
#define HI_MAPI_HDMI_EINVALID_CHNID \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_CHNID)
#define HI_MAPI_HDMI_EEXIST         HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_EXIST)
#define HI_MAPI_HDMI_EUNEXIST       HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_UNEXIST)
#define HI_MAPI_HDMI_ENOT_SUPPORT   HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_SUPPORT)
#define HI_MAPI_HDMI_ENOT_PERM      HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_PERM)
#define HI_MAPI_HDMI_ENOMEM         HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOMEM)
#define HI_MAPI_HDMI_ENOBUF         HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOBUF)
#define HI_MAPI_HDMI_EILLEGAL_PARAM \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_ILLEGAL_PARAM)
#define HI_MAPI_HDMI_EBUSY          HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_BUSY)
#define HI_MAPI_HDMI_EBUF_EMPTY     HI_MAPI_DEF_ERR(HI_MAPI_MOD_HDMI, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_BUF_EMPTY)

typedef struct hiMAPI_HDMI_SINKCAPABILITY_S {
    HI_BOOL bConnected;     /**< Whether the devices are connected. */
    HI_BOOL bSupportHdmi;   /**< Whether the HDMI is supported by the device. If the HDMI is not supported by the device,
                                                     the device is DVI. */
    HI_BOOL bIsSinkPowerOn; /**< Whether the sink device is powered on. */

    VO_INTF_SYNC_E enNativeIntfSync;       /**< Physical resolution of the display device. */
    HI_BOOL abIntfSync[VO_OUTPUT_BUTT];    /**< Video capability set.HI_TRUE: This display sync is supported.HI_FALSE:
                                                                            This display format is not supported. */
    AUDIO_SAMPLE_RATE_E aenSupportSampleRate[HI_MAPI_AO_MAX_SUPPORT_SAMPLERATE_NUM]; /**<Audio sample rate */
    HI_BOOL bSupportHdmi_2_0;                                                        /**< Whether to support HDMI2.0. */
} HI_MAPI_HDMI_SINKCAPABILITY_S;

typedef struct hiMAPIHDMI_ATTR_S {
    HI_BOOL bEnableHdmi;              /**< Whether to forcibly output the video over the HDMI.  */
    HI_BOOL bEnableVideo;             /**< Whether to output video.The value must be HI_TRUE.If the value is HI_FALSE,
                                                                    the HDMI is forcibly set to HI_TRUE. */
    VO_INTF_SYNC_E
    enHdmiSync;        /**< Video norm. This value of the video norm must be consistent with the norm of the video output. */
    HI_BOOL bEnableAudio;             /**< Whether to enable the audio. */
    AUDIO_SAMPLE_RATE_E
    enSampleRate; /**< Audio sampling rate. This parameter needs to be consistent with that of the AO. */
    HI_U32 u32PixClk;                 /**< Pixclk of enVideoFmt(unit is kHz).(This param is valid only when enVideoFmt is
                                                                    HI_HDMI_VIDEO_FMT_VESA_CUSTOMER_DEFINE) */
} HI_MAPI_HDMI_ATTR_S;

/** @} */ /** <!-- ==== HDMI End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MAPI_HDMI_DEFINE_H__ */

