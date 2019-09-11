/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2017>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#ifndef F_UVC1_1_H264_H
#define F_UVC1_1_H264_H

#define USBD_BMHINTS_RESOLUTION        (0x0001)
#define USBD_BMHINTS_PROFILE           (0x0002)
#define USBD_BMHINTS_RATECONTROL       (0x0004)
#define USBD_BMHINTS_USAGE             (0x0008)
#define USBD_BMHINTS_SLICEMODE         (0x0010)
#define USBD_BMHINTS_SLICEUNITS        (0x0020)
#define USBD_BMHINTS_MVCVIEW           (0x0040)
#define USBD_BMHINTS_TEMPORAL          (0x0080)
#define USBD_BMHINTS_SNR               (0x0100)
#define USBD_BMHINTS_SPATIAL           (0x0200)
#define USBD_BMHINTS_SPATIAL_RATIO     (0x0400)
#define USBD_BMHINTS_FRAME_INTERVAL    (0x0800)
#define USBD_BMHINTS_LEAKY_BKT_SIZE    (0x1000)
#define USBD_BMHINTS_BITRATE           (0x2000)
#define USBD_BMHINTS_ENTROPY           (0x4000)
#define USBD_BMHINTS_IFRAMEPERIOD      (0x8000)

/* wSliceMode defines */

#define USBD_SLICEMODE_BITSPERSLICE    (0x0001)
#define USBD_SLICEMODE_MBSPERSLICE     (0x0002)
#define USBD_SLICEMODE_SLICEPERFRAME   (0x0003)

#define USBD_USAGETYPE_REALTIME        (0x01)
#define USBD_USAGETYPE_BROADCAST       (0x02)
#define USBD_USAGETYPE_STORAGE         (0x03)
#define USBD_USAGETYPE_UCCONFIG_0      (0x04)
#define USBD_USAGETYPE_UCCONFIG_1      (0x05)
#define USBD_USAGETYPE_UCCONFIG_2Q     (0x06)
#define USBD_USAGETYPE_UCCONFIG_2S     (0x07)
#define USBD_USAGETYPE_UCCONFIG_3      (0x08)

/* bRateControlMode defines */

#define USBD_RATECONTROL_CBR           (0x01)
#define USBD_RATECONTROL_VBR           (0x02)
#define USBD_RATECONTROL_CONST_QP      (0x03)
#define USBD_RATECONTROL_FIXED_FRM_FLG (0x10)

/* bStreamFormat defines */

#define USBD_STREAMFORMAT_ANNEXB       (0x00)
#define USBD_STREAMFORMAT_NAL          (0x01)

/* bEntropyCABAC defines */

#define USBD_ENTROPY_CAVLC             (0x00)
#define USBD_ENTROPY_CABAC             (0x01)

/* bTimingstamp defines */

#define USBD_TIMESTAMP_SEI_DISABLE     (0x00)
#define USBD_TIMESTAMP_SEI_ENABLE      (0x01)

/* bPreviewFlipped defines */

#define USBD_PREFLIPPED_DISABLE        (0x00)
#define USBD_PREFLIPPED_HORIZONTAL     (0x01)

/* wLayerID Macro */

/*                              wLayerID
  |------------+------------+------------+----------------+------------|
  |  Reserved  |  StreamID  | QualityID  |  DependencyID  | TemporalID |
  |  (3 bits)  |  (3 bits)  | (3 bits)   |  (4 bits)      | (3 bits)   |
  |------------+------------+------------+----------------+------------|
  |15        13|12        10|9          7|6              3|2          0|
  |------------+------------+------------+----------------+------------|
*/

#define USBD_xLayerID(stream_id, quality_id, dependency_id, temporal_id) ((((stream_id)&7)<<10)|(((quality_id)&7)<<7)|(((dependency_id)&15)<<3)|((temporal_id)&7))

/* id extraction from wLayerID */

#define USBD_xStream_id(layer_id)      (((layer_id)>>10)&7)
#define USBD_xQuality_id(layer_id)     (((layer_id)>>7)&7)
#define USBD_xDependency_id(layer_id)  (((layer_id)>>3)&15)
#define USBD_xTemporal_id(layer_id)    ((layer_id)&7)

/* UVC H.264 control selectors */

typedef enum _uvcx_control_selector_t
{
    USBD_UVCX_VIDEO_CONFIG_PROBE         = 0x01,
    USBD_UVCX_VIDEO_CONFIG_COMMIT        = 0x02,
    USBD_UVCX_RATE_CONTROL_MODE          = 0x03,
    USBD_UVCX_TEMPORAL_SCALE_MODE        = 0x04,
    USBD_UVCX_SPATIAL_SCALE_MODE         = 0x05,
    USBD_UVCX_SNR_SCALE_MODE             = 0x06,
    USBD_UVCX_LTR_BUFFER_SIZE_CONTROL    = 0x07,
    USBD_UVCX_LTR_PICTURE_CONTROL        = 0x08,
    USBD_UVCX_PICTURE_TYPE_CONTROL       = 0x09,
    USBD_UVCX_VERSION                    = 0x0A,
    USBD_UVCX_ENCODER_RESET              = 0x0B,
    USBD_UVCX_FRAMERATE_CONFIG           = 0x0C,
    USBD_UVCX_VIDEO_ADVANCE_CONFIG       = 0x0D,
    USBD_UVCX_BITRATE_LAYERS             = 0x0E,
    USBD_UVCX_QP_STEPS_LAYERS            = 0x0F
} uvcx_control_selector_t;


struct uvcx_video_config_probe_commit {
    unsigned int    dwFrameInterval;
    unsigned int    dwBitRate;
    unsigned short    bmHints;
    unsigned short    wConfigurationIndex;
    unsigned short    wWidth;
    unsigned short    wHeight;
    unsigned short    wSliceUnits;
    unsigned short    wSliceMode;
    unsigned short    wProfile;
    unsigned short    wIFramePeriod;
    unsigned short    wEstimatedVideoDelay;
    unsigned short    wEstimatedMaxConfigDelay;
    unsigned char    bUsageType;
    unsigned char    bRateControlMode;
    unsigned char    bTemporalScaleMode;
    unsigned char    bSpatialScaleMode;
    unsigned char    bSNRScaleMode;
    unsigned char    bStreamMuxOption;
    unsigned char    bStreamFormat;
    unsigned char    bEntropyCABAC;
    unsigned char    bTimestamp;
    unsigned char    bNumOfReorderFrames;
    unsigned char    bPreviewFlipped;
    unsigned char    bView;
    unsigned char    bReserved1;
    unsigned char    bReserved2;
    unsigned char    bStreamID;
    unsigned char    bSpatialLayerRatio;
    unsigned short    wLeakyBucketSize;
} __attribute__((packed));


typedef struct _uvcx_rate_control_mode_t
{
    unsigned short    wLayerID;
    unsigned char    bRateControlMode;
} uvcx_rate_control_mode_t;


typedef struct _uvcx_temporal_scale_mode_t
{
    unsigned short    wLayerID;
    unsigned char    bTemporalScaleMode;
} uvcx_temporal_scale_mode_t;


typedef struct _uvcx_spatial_scale_mode_t
{
    unsigned short    wLayerID;
    unsigned char    bSpatialScaleMode;
} uvcx_spatial_scale_mode_t;


typedef struct _uvcx_snr_scale_mode_t
{
    unsigned short    wLayerID;
    unsigned char    bSNRScaleMode;
    unsigned char    bMGSSublayerMode;
} uvcx_snr_scale_mode_t;


typedef struct _uvcx_ltr_buffer_size_control_t
{
    unsigned short    wLayerID;
    unsigned char    bLTRBufferSize;
    unsigned char    bLTREncoderControl;
} uvcx_ltr_buffer_size_control_t;

typedef struct _uvcx_ltr_picture_control
{
    unsigned short    wLayerID;
    unsigned char    bPutAtPositionInLTRBuffer;
    unsigned char    bEncodeUsingLTR;
} uvcx_ltr_picture_control;


typedef struct _uvcx_picture_type_control_t
{
    unsigned short    wLayerID;
    unsigned short    wPicType;
} uvcx_picture_type_control_t;


typedef struct _uvcx_version_t
{
    unsigned short    wVersion;
} uvcx_version_t;


typedef struct _uvcx_encoder_reset
{
    unsigned short    wLayerID;
} uvcx_encoder_reset;


typedef struct _uvcx_framerate_config_t
{
    unsigned short    wLayerID;
    unsigned int    dwFrameInterval;
} uvcx_framerate_config_t;


typedef struct _uvcx_video_advance_config_t
{
    unsigned short    wLayerID;
    unsigned int    dwMb_max;
    unsigned char    blevel_idc;
    unsigned char    bReserved;
} uvcx_video_advance_config_t;


typedef struct _uvcx_bitrate_layers_t
{
    unsigned short    wLayerID;
    unsigned int    dwPeakBitrate;
    unsigned int    dwAverageBitrate;
} uvcx_bitrate_layers_t;


typedef struct _uvcx_qp_steps_layers_t
{
    unsigned short    wLayerID;
    unsigned char    bFrameType;
    unsigned char    bMinQp;
    unsigned char    bMaxQp;
} uvcx_qp_steps_layers_t;


#ifdef _WIN32
// GUID of the UVC H.264 extension unit: {A29E7641-DE04-47E3-8B2B-F4341AFF003B}
DEFINE_GUID(GUID_UVCX_H264_XU, 0xA29E7641, 0xDE04, 0x47E3, 0x8B, 0x2B, 0xF4, 0x34, 0x1A, 0xFF, 0x00, 0x3B);
#endif

#endif  /*_UVC_H264_H_*/
