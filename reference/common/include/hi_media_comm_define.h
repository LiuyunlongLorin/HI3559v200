/**
 * @file      hi_media_comm_define.h
 * @brief     media common struct definition
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/2
 * @version   1.0

 */

#ifndef __HI_MEDIA_COMM_DEFINE_H__
#define __HI_MEDIA_COMM_DEFINE_H__

#include "hi_appcomm.h"
#include "hi_osd.h"
#include "hi_mapi_vcap_define.h"
#include "hi_mapi_vproc_define.h"
#include "hi_mapi_venc_define.h"
#include "hi_mapi_disp_define.h"
#include "hi_mapi_acap_define.h"
#include "hi_mapi_aenc_define.h"
#include "hi_mapi_aenc_adpt.h"
#include "hi_mapi_ao_define.h"

#ifdef __cplusplus
#if __cplusplus
 extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     MEDIA */
/** @{ */  /** <!-- [MEDIA] */

/** media vi-vpss mode */
typedef struct hiMEDIA_VI_VPSS_MODE_S
{
    HI_HANDLE VcapPipeHdl;
    VI_VPSS_MODE_E enMode;
} HI_MEDIA_VI_VPSS_MODE_S;

/** sensor attribute */
typedef struct hiMEDIA_SENSOR_ATTR_S
{
    SIZE_S stResolution;
    WDR_MODE_E enWdrMode;
    HI_U32 u32SnsMode;
} HI_MEDIA_SENSOR_ATTR_S;

/** media pixel format */
typedef enum hiMEDIA_PIXEL_FMT_E
{
    HI_MEDIA_PIXEL_FMT_YUV_SEMIPLANAR_420 = 0,
    HI_MEDIA_PIXEL_FMT_YUV_SEMIPLANAR_422,
    HI_MEDIA_PIXEL_FMT_ABGR_1555,
    HI_MEDIA_PIXEL_FMT_ABGR_8888,
    HI_MEDIA_PIXEL_FMT_RGB_BAYER_8BPP,  /**<Raw 8bit*/
    HI_MEDIA_PIXEL_FMT_RGB_BAYER_10BPP, /**<Raw 10bit*/
    HI_MEDIA_PIXEL_FMT_RGB_BAYER_12BPP, /**<Raw 12bit*/
    HI_MEDIA_PIXEL_FMT_RGB_BAYER_14BPP, /**<Raw 14bit*/
    HI_MEDIA_PIXEL_FMT_RGB_BAYER_16BPP, /**<Raw 16bit*/
    HI_MEDIA_PIXEL_FMT_BUTT
} HI_MEDIA_PIXEL_FMT_E;

/** vpss attribute */
typedef struct hiMEDIA_VPSS_ATTR_S
{
    HI_U32 u32MaxW;
    HI_U32 u32MaxH;
    FRAME_RATE_CTRL_S stFrameRate;
    HI_MEDIA_PIXEL_FMT_E enPixelFormat;
    HI_BOOL bNrEn;
    VPSS_NR_ATTR_S  stNrAttr;
}HI_MEDIA_VPSS_ATTR_S;

/** media-venc H264 cbr attribute */
typedef struct hiMEDIA_VENC_ATTR_H264_CBR_S
{
    HI_MAPI_VENC_ATTR_CBR_S stAttr;
    HI_U32 u32MaxQp;      /* RW; Range:(MinQp, 51];the max QP value */
    HI_U32 u32MinQp;      /* RW; Range:[0, 51]; the min QP value */
    HI_U32 u32MaxIQp;     /* RW; Range:(MinQp, 51]; max qp for i frame */
    HI_U32 u32MinIQp;     /* RW; Range:[0, 51]; min qp for i frame */
} HI_MEDIA_VENC_ATTR_H264_CBR_S;

/** media-venc H264 vbr attribute */
typedef struct hiMEDIA_VENC_ATTR_H264_VBR_S
{
    HI_MAPI_VENC_ATTR_VBR_S stAttr;
    HI_U32 u32MaxQp;      /* RW; Range:(MinQp, 51]; the max P B qp */
    HI_U32 u32MinQp;      /* RW; Range:[0, 51]; the min P B qp */
    HI_U32 u32MaxIQp;     /* RW; Range:(MinIQp, 51]; the max I qp */
    HI_U32 u32MinIQp;     /* RW; Range:[0, 51]; the min I qp */
} HI_MEDIA_VENC_ATTR_H264_VBR_S;

/** media-venc H264 qvbr attribute */
typedef struct hiMEDIA_VENC_ATTR_H264_QVBR_S
{
    HI_MAPI_VENC_ATTR_QVBR_S stAttr;
    HI_U32 u32MaxQp;      /* RW; Range:(MinQp, 51]; the max P B qp */
    HI_U32 u32MinQp;      /* RW; Range:[0, 51]; the min P B qp */
    HI_U32 u32MaxIQp;     /* RW; Range:(MinIQp, 51]; the max I qp */
    HI_U32 u32MinIQp;     /* RW; Range:[0, 51]; the min I qp */

    HI_S32 s32BitPercentUL;     /* RW; Range:[30, 180]; Indicate the ratio of bitrate  upper limit*/
    HI_S32 s32BitPercentLL;     /* RW; Range:[30, 180]; Indicate the ratio of bitrate lower limit,
                                       can not be larger than s32BitPercentUL*/
    HI_S32 s32PsnrFluctuateUL;  /* RW; Range:[18, 40];
                                       Reduce the target bitrate when the value of psnr approch the upper limit*/
    HI_S32 s32PsnrFluctuateLL;  /* RW; Range:[18, 40];
                                       Increase the target bitrate when the value of psnr approch the lower limit,
                                       can not be larger than s32PsnrFluctuateUL*/
} HI_MEDIA_VENC_ATTR_H264_QVBR_S;

typedef struct hiMEDIA_VENC_ATTR_H264_CBR_S  HI_MEDIA_VENC_ATTR_H265_CBR_S;
typedef struct hiMEDIA_VENC_ATTR_H264_VBR_S  HI_MEDIA_VENC_ATTR_H265_VBR_S;
typedef struct hiMEDIA_VENC_ATTR_H264_QVBR_S  HI_MEDIA_VENC_ATTR_H265_QVBR_S;

/** media-venc MJPEG cbr attribute */
typedef struct hiMEDIA_VENC_ATTR_MJPEG_CBR_S
{
    HI_MAPI_VENC_ATTR_CBR_S stAttr;
    HI_U32 u32MaxQfactor; /* the max Qfactor value */
    HI_U32 u32MinQfactor; /* the min Qfactor value */
} HI_MEDIA_VENC_ATTR_MJPEG_CBR_S;

/** media-venc MJPEG vbr attribute */
typedef struct hiMEDIA_VENC_ATTR_MJPEG_VBR_S
{
    HI_MAPI_VENC_ATTR_VBR_S stAttr;
    HI_U32 u32MaxQfactor; /* the max Qfactor value */
    HI_U32 u32MinQfactor; /* the min Qfactor value */
} HI_MEDIA_VENC_ATTR_MJPEG_VBR_S;

/** media-venc rate control attribute */
typedef struct hiMEDIA_VENC_RC_ATTR_S
{
    HI_MAPI_VENC_RC_MODE_E enRcMode;
    union tagMEDIA_VENC_ATTR
    {
        HI_MEDIA_VENC_ATTR_H264_CBR_S  stH264Cbr;
        HI_MEDIA_VENC_ATTR_H264_VBR_S  stH264Vbr;
        HI_MEDIA_VENC_ATTR_H264_QVBR_S stH264QVbr;
        HI_MEDIA_VENC_ATTR_H265_CBR_S  stH265Cbr;
        HI_MEDIA_VENC_ATTR_H265_VBR_S  stH265Vbr;
        HI_MEDIA_VENC_ATTR_H265_QVBR_S stH265QVbr;
        HI_MEDIA_VENC_ATTR_MJPEG_CBR_S stMjpegCbr;
        HI_MEDIA_VENC_ATTR_MJPEG_VBR_S stMjpegVbr;
    } unAttr;
    VENC_HIERARCHICAL_QP_S stHierarchicalQp;
} HI_MEDIA_VENC_RC_ATTR_S;

/** video encoder attribute */
typedef struct hiMEDIA_VENC_ATTR_S
{
    HI_MAPI_VENC_TYPE_ATTR_S stTypeAttr;
    HI_MEDIA_VENC_RC_ATTR_S  stRcAttr;
} HI_MEDIA_VENC_ATTR_S;

/** Video Information */
typedef struct hiMEDIA_VIDEOINFO_S
{
    HI_MEDIA_VENC_ATTR_S stVencAttr;
} HI_MEDIA_VIDEOINFO_S;

/** Audio Infomation */
typedef struct hiMEDIA_AUDIOINFO_S
{
    HI_U32 u32SampleRate;      /**<audio sample rate :8000,16000,48000 */
    HI_U32 u32AvgBytesPerSec;  /**<eg, 16000,24000,32000,40000,48000 */
    HI_U32 u32PtNumPerFrm;     /**<sampling point number per frame */
    HI_U16 u16SampleBitWidth;  /**<eg,sample bit width:  8,16,32 */
    HI_MAPI_AUDIO_FORMAT_E enAencFormat;    /**<audio encoding type */
    HI_MAPI_AUDIO_SOUND_MODE_E enSoundMode; /**<sound mode, such as single channel, stereo */
} HI_MEDIA_AUDIOINFO_S;

/** @}*/  /** <!-- ==== MEDIA End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MEDIA_COMM_DEFINE_H__ */

