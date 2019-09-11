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

#ifndef _F_UVC_GADGET_H
#define _F_UVC_GADGET_H

#include <core/usb_endian.h>
#include "implementation/freebsd_sys.h"
#include "gadget/usbd_video.h"
#include "core/usb.h"
#include "controller/usb_device/usb_gadget.h"
#include "gadget/composite.h"

/*
 * USB Video Class -- Class-specific VC Interface Header Descriptor
 * Refer to USB Video Class Specification 1.5, section 3.7.2
 */
struct uvc_vc_header_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u16          bcdUVC;
    u16          wTotalLength;
    u32          dwClockFrequency;
    u8           bInCollection;
    u8           baInterfaceNr[1];
} __attribute__((packed));

/*
 * UVC Video Class-specific Descriptor Types, UVC spec. 1.5, section A.4
 */
#define USB_UVC_CS_UNDEFINED            0x20
#define USB_UVC_CS_DEVICE               0x21
#define USB_UVC_CS_CONFIGURATION        0x22
#define USB_UVC_CS_STRING               0x23
#define USB_UVC_CS_INTERFACE            0x24
#define USB_UVC_CS_ENDPOINT             0x25

/*
 * UVC Video Class-specific Descriptor Subtypes, UVC spec. 1.5, section A.5
 */
#define UVC_VC_DESCRIPTOR_UNDEFINED    0x0
#define UVC_VC_HEADER                  0x1
#define UVC_VC_INPUT_TERMINAL          0x2
#define UVC_VC_OUTPUT_TERMINAL         0x3
#define UVC_VC_SELECTOR_UNIT           0x4
#define UVC_VC_PROCESSING_UNIT         0x5
#define UVC_VC_EXTENSION_UNIT          0x6
#define UVC_VC_ENCODING_UNIT           0x7

/*
 * UVC input terminal descriptor, refer to UVC Specification
 * 1.5, section 3.7.2.1 for the details
 */
struct uvc_it_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u8           bTerminalID;
    u16          wTerminalType;
    u8           bAssocTerminal;
    u8           iTerminal;
} __attribute__((packed));

/*
 * UVC output terminal descriptor, refer to UVC specification
 * 1.5, section 3.7.2.2 for the details
 */
struct uvc_ot_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u8           bTerminalID;
    u16          wTerminalType;
    u8           bAssocTerminal;
    u8           bSourceID;
    u8           iTerminal;
} __attribute__((packed));

/*
 * USB Terminal types, UVC spec. 1.5, section B.1
 */
#define USB_UVC_TT_VENDOR_SPECIFIC         0x0100
#define USB_UVC_TT_STREAMING               0x0101

/*
 * UVC camera terminal descriptor, refer to UVC specification
 * 1.5, section 3.7.2.3 for the details
 */
struct uvc_ct_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u8           bTerminalID;
    u16          wTerminalType;
    u8           bAssocTerminal;
    u8           iTerminal;
    u16          wObjectiveFocalLengthMin;
    u16          wObjectiveFocalLengthMax;
    u16          wOcularFocalLength;
    u8           bControlSize;
    u8           bmControls[3];
} __attribute__((packed));

/*
 * Camera Terminal Control Selectors, UVC spec. 1.5, section A.9.4
 */
#define USBD_UVC_CT_CONTROL_UNDEFINED                0x00
#define USBD_UVC_CT_SCANNING_MODE_CONTROL            0x01
#define USBD_UVC_CT_AE_MODE_CONTROL                  0x02
#define USBD_UVC_CT_AE_PRIORITY_CONTROL              0x03
#define USBD_UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL   0x04
#define USBD_UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL   0x05
#define USBD_UVC_CT_FOCUS_ABSOLUTE_CONTROL           0x06
#define USBD_UVC_CT_FOCUS_RELATIVE_CONTROL           0x07
#define USBD_UVC_CT_FOCUS_AUTO_CONTROL               0x08
#define USBD_UVC_CT_IRIS_ABSOLUTE_CONTROL            0x09
#define USBD_UVC_CT_IRIS_RELATIVE_CONTROL            0x0a
#define USBD_UVC_CT_ZOOM_ABSOLUTE_CONTROL            0x0b
#define USBD_UVC_CT_ZOOM_RELATIVE_CONTROL            0x0c
#define USBD_UVC_CT_PANTILT_ABSOLUTE_CONTROL         0x0d
#define USBD_UVC_CT_PANTILT_RELATIVE_CONTROL         0x0e
#define USBD_UVC_CT_ROLL_ABSOLUTE_CONTROL            0x0f
#define USBD_UVC_CT_ROLL_RELATIVE_CONTROL            0x10
#define USBD_UVC_CT_PRIVACY_CONTROL                  0x11
#define USBD_UVC_CT_FOCUS_SIMPLE_CONTROL             0x12
#define USBD_UVC_CT_WINDOW_CONTROL                   0x13
#define USBD_UVC_CT_REGION_OF_INTEREST_CONTROL       0x14

/*
 * Input Terminal Types, UVC spec. 1.5, section B.2
 */
#define USB_UVC_ITT_VENDOR_SPECIFIC        0x0200
#define USB_UVC_ITT_CAMERA                 0x0201
#define USB_UVC_ITT_MEDIA_TRANSPORT_INPUT  0x0202

/*
 * Selector Unit Descriptor, UVC spec., section 3.7.2.4
 */
#define DEFINE_UVC_SELECTOR_DESCRIPTOR(p_) \
struct uvc_selector_descriptor ## p_ { \
    u8           bLength; \
    u8           bDescriptorType; \
    u8           bDescriptorSubtype; \
    u8           bUnitID; \
    u8           bNrInPins; \
    u8           baSourceID[p_]; \
    u8           iSelector; \
} __attribute__((packed))

DEFINE_UVC_SELECTOR_DESCRIPTOR(2);

/*
 * Processing Unit Descriptor, UVC spec. 1.5, section 3.7.2.5
 */
struct uvc_processing_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u8           bUnitID;
    u8           bSourceID;
    u16          wMaxMultiplier;
    u8           bControlSize;
    u8           bmControls[3];
    u8           iProcessing;
    u8           bmVideoStandards;
} __attribute__((packed));

/*
 * UVC Processing Unit Control Selectors, UVC spec. 1.5, section A.9.5
 */
#define USBD_UVC_PU_CONTROL_UNDEFINED                        0x00
#define USBD_UVC_PU_BACKLIGHT_COMPENSATION_CONTROL           0x01
#define USBD_UVC_PU_BRIGHTNESS_CONTROL                       0x02
#define USBD_UVC_PU_CONTRAST_CONTROL                         0x03
#define USBD_UVC_PU_GAIN_CONTROL                             0x04
#define USBD_UVC_PU_POWER_LINE_FREQUENCY_CONTROL             0x05
#define USBD_UVC_PU_HUE_CONTROL                              0x06
#define USBD_UVC_PU_SATURATION_CONTROL                       0x07
#define USBD_UVC_PU_SHARPNESS_CONTROL                        0x08
#define USBD_UVC_PU_GAMMA_CONTROL                            0x09
#define USBD_UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL        0x0a
#define USBD_UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTORL   0x0b
#define USBD_UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL          0x0c
#define USBD_UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL     0x0d
#define USBD_UVC_PU_DIGITAL_MULTIPLIER_CONTROL               0x0e
#define USBD_UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL         0x0f
#define USBD_UVC_PU_HUE_AUTO_CONTROL                         0x10
#define USBD_UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL            0x11
#define USBD_UVC_PU_ANALOG_LOCK_STATUS_CONTROL               0x12
#define USBD_UVC_PU_CONTRAST_AUTO_CONTROL                    0x13

/*
 * UVC Extension Unit Descriptor, UVC spec. 1.5, section 3.7.2.7
 */
#define DECLARE_UVC_EXTENSION_UNIT(p_, n_) \
struct uvc_extension_unit_descriptor ## p_ ## x ## n_ { \
    u8           bLength;    \
    u8           bDescriptorType;    \
    u8           bDescriptorSubtype;    \
    u8           bUnitID;    \
    u8           guidExtensionCode[16];    \
    u8           bNumControls;    \
    u8           bNrInPins;    \
    u8           baSourceID[ p_ ];    \
    u8           bControlSize;    \
    u8           bmControls[ n_ ];    \
    u8           iExtension;    \
} __attribute__((packed))

DECLARE_UVC_EXTENSION_UNIT(1, 2);

/*
 * UVC Video Streaming interface header descriptor, UVC spec., section 3.9.2.1
 */
struct uvc_vs_header_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u8           bNumFormats;
    u16          wTotalLength;
    u8           bEndpointAddress;
    u8           bmInfo;
    u8           bTerminalLink;
    u8           bStillCaptureMethod;
    u8           bTriggerSupport;
    u8           bTriggerUsage;
    u8           bControlSize;
    u8           bmaControls[2][1];
} __attribute__((packed));

/*
 * UVC stream Header definitions, refer to `USB_Video_Payload_H264_1.5, section 2.2
 */
#define UVC_STREAM_HEADER_FID                    0x01
#define UVC_STREAM_HEADER_EOF                    0x02
#define UVC_STREAM_HEADER_PTS                    0x04
#define UVC_STREAM_HEADER_SCR                    0x08
#define UVC_STREAM_HEADER_EOS                    0x10
#define UVC_STREAM_HEADER_STI                    0x20
#define UVC_STREAM_HEADER_ERR                    0x40
#define UVC_STREAM_HEADER_EOH                    0x80

/*
 * Video Class-specific VS interface Descriptor Subtypes, UVC spec. 1.5, section A.6
 */
#define USB_UVC_VS_UNDEFINED                         0x00
#define USB_UVC_VS_INPUT_HEADER                      0x01
#define USB_UVC_VS_OUTPUT_HEADER                     0x02
#define USB_UVC_VS_STILL_IMAGE_FRAME                 0x03
#define USB_UVC_VS_FORMAT_UNCOMPRESSED               0x04
#define USB_UVC_VS_FRAME_UNCOMPRESSED                0x05
#define USB_UVC_VS_FORMAT_MJPEG                      0x06
#define USB_UVC_VS_FRAME_MJPEG                       0x07

#define USB_UVC_VS_FORMAT_MPEG2TS                    0x0a
#define USB_UVC_VS_FORMAT_DV                         0x0c
#define USB_UVC_VS_COLOR_FORMAT                      0x0d
#define USB_UVC_VS_FORMAT_FRAME_BASED                0x10
#define USB_UVC_VS_FRAME_FRAME_BASED                 0x11
#define USB_UVC_VS_FORMAT_STREAM_BASED               0x12
#define USB_UVC_VS_FORMAT_H264                       0x13

#define USB_UVC_VS_FRAME_H264                        0x14
#define USB_UVC_VS_FORMAT_H264_SIMULCAST             0x15
#define USB_UVC_VS_FORMAT_VP8                        0x16
#define USB_UVC_VS_FRAME_VP8                         0x17
#define USB_UVC_VS_FORMAT_VP8_SIMULCAST              0x18

/*
 * UVC uncompressed video frame descriptor, USB Video Payload Uncompressed 1.5, section 3.1.2
 */
#define DEFINE_UVC_UNCOMPRESSED_FRAME_DESCRIPTOR(p_) \
struct uvc_uncompressed_frame_descriptor ## p_ { \
    u8           bLength; \
    u8           bDescriptorType; \
    u8           bDescriptorSubtype; \
    u8           bFrameIndex; \
    u8           bmCapabilities; \
    u16          wWidth; \
    u16          wHeight; \
    u32          dwMinBitRate; \
    u32          dwMaxBitRate; \
    u32          dwMaxVideoFrameBufferSize; \
    u32          dwDefaultFrameInterval; \
    u8           bFrameIntervalType; \
    u32          dwFrameInterval[p_]; \
} __attribute__((packed))

DEFINE_UVC_UNCOMPRESSED_FRAME_DESCRIPTOR(3);

/*
 * UVC uncompressed video format descriptor, USB Video Payload Uncompressed 1.5, section 3.1.1
 */
struct uvc_uncompressed_format_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u8           bFormatIndex;
    u8           bNumFrameDescriptors;
    u8           guidFormat[16];
    u8           bBitsPerPixel;
    u8           bDefaultFrameIndex;
    u8           bAspectRatioX;
    u8           bAspectRatioY;
    u8           bmInterlaceFlags;
    u8           bCopyProtect;
} __attribute__((packed));

/* MJPEG Payload - 3.1.1. MJPEG Video Format Descriptor */
struct uvc_format_mjpeg {
    u8  bLength;
    u8  bDescriptorType;
    u8  bDescriptorSubType;
    u8  bFormatIndex;
    u8  bNumFrameDescriptors;
    u8  bmFlags;
    u8  bDefaultFrameIndex;
    u8  bAspectRatioX;
    u8  bAspectRatioY;
    u8  bmInterfaceFlags;
    u8  bCopyProtect;
} __attribute__((packed));

/*
 * UVC stream based format descriptor
 * USB_Video_Payload_Stream_Based_1.1, section 3.1.1
 * USB_Video_Payload_Stream_Based_1.5, section 3.1.1
 */
struct uvc_stream_based_format_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u8           bFormatIndex;
    u8           guidFormat[16];
    u32          dwPacketLength;
} __attribute__((packed));

/*
 * frame based Payload Video Format Descriptor, USB_Video_Payload_Frame_Based_1.1, section 3.1.1
 */
struct uvc_frame_based_format_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u8           bFormatIndex;
    u8           bNumFrameDescriptors;
    u8           guidFormat[16];
    u8           bBitsPerPixel;
    u8           bDefaultFrameIndex;
    u8           bAspectRatioX;
    u8           bAspectRatioY;
    u8           bmInterlaceFlags;
    u8           bCopyProtect;
    u8           bVariableSize;
} __attribute__((packed));

/*
 * Frame Based payload Video Frame Descriptors, USB_Video_Payload_Frame_Based_1.1, section 3.1.2
 */
#define DECLARE_UVC_FRAME_BASED_FRAME_DESCRITPROR(n_) \
struct uvc_frame_based_frame_descriptor ## n_ {    \
    u8           bLength;    \
    u8           bDescriptorType;    \
    u8           bDescriptorSubtype;    \
    u8           bFrameIndex;    \
    u8           bmCapabilities;    \
    u16          wWidth;    \
    u16          wHeight;    \
    u32          dwMinBitRate;    \
    u32          dwMaxBitRate;    \
    u32          dwDefaultFrameInterval;    \
    u8           bFrameIntervalType;    \
    u32          dwBytesPerLine;    \
    u32          dwFrameInterval[n_];    \
} __attribute__((packed))

DECLARE_UVC_FRAME_BASED_FRAME_DESCRITPROR(3);

/*
 * H.264 Payload Video Format Descriptor, USB_Video_Payload_H264_1.5, section 3.1.1
 */
struct uvc_h264_format_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u8           bFormatIndex;
    u8           bNumFrameDescriptors;
    u8           bDefaultFrameIndex;
    u8           bMaxCodecConfigDelay;
    u8           bmSupportedSliceModes;
    u8           bmSupportedSyncFrameTypes;
    u8           bResolutionScaling;
    u8           Reserved1;
    u8           bmSupportedRateControlModes;
    u16          wMaxMBperSec_No[4];
    u16          wMaxMBperSec_Temp[4];
    u16          wMaxMBperSec_Qual[4];
    u16          wMaxMBperSec_Spat[4];
    u16          wMaxMBperSec_Full[4];
} __attribute__((packed));

/*
 * H.264 Payload Video Frame Descriptor, section 3.1.2
 */
#define DECLARE_UVC_H264_FRAME_DESCRIPTOR(n_) \
struct uvc_h264_frame_descriptor ## n_ {    \
    u8           bLength;    \
    u8           bDescriptorType;    \
    u8           bDescriptorSubtype;    \
    u8           bFrameIndex;    \
    u16          wWidth;    \
    u16          wHeight;    \
    u16          wSARwidth;    \
    u16          wSARheight;    \
    u16          wProfile;    \
    u8           bLevelIDC;    \
    u16          wConstrainedToolset;    \
    u32          bmSupportedUsages;    \
    u16          bmCapabilities;    \
    u32          bmSVCCapabilities;    \
    u32          bmMVCCapabilities;    \
    u32          dwMinBitRate;    \
    u32          dwMaxBitRate;    \
    u32          dwDefaultFrameInterval;    \
    u8           bNumFrameIntervals;    \
    u8           dwFrameIntervals[n_]; \
} __attribute__((packed))

DECLARE_UVC_H264_FRAME_DESCRIPTOR(1);

#define DECLARE_UVC_MJPEG_FRAME_DESCRIPTOR(n_)   \
struct uvc_mjpg_frame_descriptor ## n_ {                \
    u8     bLength;   \
    u8     bDescriptorType;   \
    u8     bDescriptorSubType;    \
    u8     bFrameIndex;   \
    u8     bmCapabilities;    \
    u16    wWidth;    \
    u16    wHeight;   \
    u32    dwMinBitRate;   \
    u32    dwMaxBitRate;   \
    u32    dwMaxVideoFrameBufferSize;        \
    u32    dwDefaultFrameInterval; \
    u8     bFrameIntervalType; \
    u32    dwFrameInterval[n_];    \
} __attribute__ ((packed))

DECLARE_UVC_MJPEG_FRAME_DESCRIPTOR(1);

/*
 * UVC color-matching descriptor, UVC spec., section 3.9.2.6
 */
struct uvc_color_matching_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u8           bColorPrimaries;
    u8           bTransferCharacteristics;
    u8           bMatrixCoefficients;
} __attribute__((packed));

/* Color Primaries and the reference white */
#define UVC_COLOR_UNSPECIFIED                      0x0
#define UVC_COLOR_BT709_SRGB                       0x1
#define UVC_COLOR_BT4702M                          0x2
#define UVC_COLOR_BT4702BG                         0x3
#define UVC_COLOR_SMPTE_170M                       0x4
#define UVC_COLOR_SMPTE_240M                       0x5
/* Color transfer characteristics, gamma function ??? */
#define UVC_COLOR_LINEAR                           0x6
#define UVC_COLOR_SRGB                             0x7
/* Matrix Coefficients */
#define UVC_COLOR_MC_UNSPECIFIED                   0x0
#define UVC_COLOR_MC_BT709                         0x1
#define UVC_COLOR_MC_FCC                           0x2
#define UVC_COLOR_MC_BT4702BG                      0x3
#define UVC_COLOR_MC_SMPTE_170M                    0x4
#define UVC_COLOR_MC_SMPTE_240M                    0x5

#define fourcc(a, b, c, d) \
    ((u32)(a) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

#define V4L2_PIX_FMT_YUYV fourcc('Y', 'U', 'Y', 'V')/* 16  YUV 4:2:2     */
#define V4L2_PIX_FMT_MJPEG fourcc('M', 'J', 'P', 'G')/* Motion-JPEG   */
#define V4L2_PIX_FMT_H264 fourcc('H', '2', '6', '4')/* H264 with start codes */

/*
 * UVC VideoControl interrupt endpoint descriptor, UVC spec. 1.5, section 3.8.2.2
 */
struct uvc_vc_int_endpoint_descriptor {
    u8           bLength;
    u8           bDescriptorType;
    u8           bDescriptorSubtype;
    u16          wMaxTransferSize;
} __attribute__((packed));

/*
 * Video Class-specific Endpoint Descriptor Subtypes, UVC spec. 1.5, section A.7
 */
#define UVC_EP_UNDEFINED                           0x0
#define UVC_EP_GENERAL                             0x1
#define UVC_EP_ENDPOINT                            0x2
#define UVC_EP_INTERRUPT                           0x3

#define COPY_UVC_DESCRIPTOR(_to, _fro) do { \
        const u8 * _fro_; \
        _fro_ = (const u8 *) (_fro); \
        memcpy((_to), (_fro), *_fro_); \
        (_to) += *_fro_; \
    } while (0)

/*
 * VideoStreaming Interface Control Selectors, UVC spec. 1.5, section A.9.8
 */
#define USB_UVC_VS_CONTROL_UNDEFINED                   0x00
#define USB_UVC_VS_PROBE_CONTROL                       0x01
#define USB_UVC_VS_COMMIT_CONTROL                      0x02
#define USB_UVC_VS_STILL_PROBE_CONTROL                 0x03
#define USB_UVC_VS_STILL_COMMIT_CONTROL                0x04
#define USB_UVC_VS_STILL_IMAGE_TRIGGER_CONTROL         0x05
#define USB_UVC_VS_STREAM_ERROR_CODE_CONTROL           0x06
#define USB_UVC_VS_GENERATE_KEY_FRAME_CONTROL          0x07
#define USB_UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL        0x08
#define USB_UVC_VS_SYNCH_DELAY_CONTROL                 0x09

#define USB_UVC_INTERFACE_CONTROL    0x0
#define USB_UVC_INTERFACE_STREAMING  0x1

/*
 * Video Class-specific Request Codes, UVC spec. 1.5, section A.8
 */
#define UVC_RC_SETCUR            0x01
#define UVC_RC_SETCUR_ALL        0x11
#define UVC_RC_GETCUR            0x81
#define UVC_RC_GETMIN            0x82
#define UVC_RC_GETMAX            0x83
#define UVC_RC_GETRES            0x84
#define UVC_RC_GETLEN            0x85
#define UVC_RC_GETINFO           0x86
#define UVC_RC_GETDEF            0x87
#define UVC_RC_GETCUR_ALL        0x91
#define UVC_RC_GETMIN_ALL        0x92
#define UVC_RC_GETMAX_ALL        0x93
#define UVC_RC_GETRES_ALL        0x94
#define UVC_RC_GETDEF_ALL        0x95

#define UVC_VC_SETCUR_COMPLETE  0Xff

/*
 * Video probe and commit controls, UVC spec. 1.1, section 4.3.1.1
 */
struct uvc_probe_commit_control1_1 {
    u16          bmHint;
    u8           bFormatIndex;
    u8           bFrameIndex;
    u32          dwFrameInterval;
    u16          wKeyFrameRate;
    u16          wPFrameRate;
    u16          wCompQuality;
    u16          wCompWindowSize;
    u16          wDelay;
    u32          dwMaxVideoFrameSize;
    u32          dwMaxPayloadTransferSize;
    u32          dwClockFrequency;
    u8           bmFramingInfo;
    u8           bPreferedVersion;
    u8           bMinVersion;
    u8           bMaxVersion;
} __attribute__ ((packed));

/*
 * Video probe and commit controls, UVC spec. 1.5, section 4.3.1.1
 */
struct uvc_probe_commit_control1_5 {
    u16          bmHint;
    u8           bFormatIndex;
    u8           bFrameIndex;
    u32          dwFrameInterval;
    u16          wKeyFrameRate;
    u16          wPFrameRate;
    u16          wCompQuality;
    u16          wCompWindowSize;
    u16          wDelay;
    u32          dwMaxVideoFrameSize;
    u32          dwMaxPayloadTransferSize;
    u32          dwClockFrequency;
    u8           bmFramingInfo;
    u8           bPreferedVersion;
    u8           bMinVersion;
    u8           bMaxVersion;
    u8           bUsage;
    u8           bBitDepthLuma;
    u8           bmSettings;
    u8           bMaxNumberOfRefFramesPlus1;
    u8           bmRateControlModes;
    u8           bmLayoutPerStream;
} __attribute__((packed));

enum STREAM_STATUS {
    STREAM_ON = 0,
    STREAM_OFF,
};

struct uvc_probe_t
{
    u8 set;
    u8 get;
    u8 max;
    u8 min;
};

/* private UVC device driver instance structure */
struct fuvc_softc {
    unsigned char *              descs;           /* pointer to the concatenated descriptors */
    unsigned int                 desl;            /* length of all descriptors */
    unsigned int                 control;         /* Class-specific control request value */
    struct uvc_probe_commit_control1_1     probe; /* probe control structure */
    struct uvc_probe_commit_control1_1    commit; /* probe control structure */
    struct usbd_endpoint *       ctrl_ep;         /* control transfer endpoint */
    struct usbd_endpoint *       vs_ep;           /* Video Streaming endpoint */

    struct usbd_request          ctrlreq;         /* control request */
    struct usbd_request          vc_ctrlreq;      /* Video Control request */
    struct usbd_request          streamreq;       /* Video Streaming request */
    unsigned int                 dyn_fc;          /* dynamic format change, non-zero indicates that format change occurred */

    unsigned char *              stream_buf;      /* Buffer used for stream transfer */
    struct usbd_gadget_device *  gadget;          /* deivce instance */
    struct usbd_gadget_driver    driver;          /* gadget driver instance */

    struct uvc_uncompressed_frame_descriptor3 * frame_desc; /* pointer to the frame descriptor */
    int                          connected;       /* Connected to USB host ? */
    EVENT_CB_S                   connect_event;   /* Event for USB host conected! */

#define FUVC_TASK_ID_TERMINATE   0xFFFFFFFF
#define FUVC_TASK_ID_TERMINATED  0xFFFFFFFE
    UINT32                       uvc_tid;         /* UVC working task */
    void *                       uvc_handle;      /* UVC transmission handle */
    unsigned int                 fid;             /* Frame Indentifier */
    unsigned long long           last_complete;   /* Last Video stream complete tick */

    struct uvc_format_info format_info;
    unsigned int bulk_size;
    unsigned int imgsize;
    struct uvc_probe_t probe_status;

    unsigned int zero_packet_flag;
    enum STREAM_STATUS transfer_status;

    unsigned int uvc_vc_id;
    struct usb_function func;
} __attribute__((packed));


/*
 * structure used when transimitting video stream with `nocp method
 */
struct nocp_reserve {
    unsigned char * res;            /* pointer to the reserved data */
    unsigned int resl;              /* reserved data length */
#define NOCP_RESERVE_MAX            0x4
    unsigned char res_buf[NOCP_RESERVE_MAX];     /* reserved data */
};


/*
 * UVC handle structure, includes various related information to
 * transmit video stream over USB protocol.
 */
struct uvc_handle {
    u32 vid_w;                 /* Video Frame width in pixels */
    u32 vid_h;                 /* Video Frame height in pixels */
    u32 vid_f;                 /* Video Frame format */
    u32 state;                 /* state of UVC handle */
    uvc_continue_func copy_func;    /* video transmission video frame copy function */
    uvc_continue_func next_func;    /* video transmission video next frame function */
    void * _priv;                   /* video transmission private pointer */
    EVENT_CB_S uvc_event;

    u32 retransmission;
    u32 used;
    u32 offset;


    u32 frame_off;             /* transfer offset in a single video frame */
    struct uvc_transfer_data tdata; /* transfer data information */
    struct nocp_reserve reserve;    /* reserve structure */
    struct fuvc_softc * fuvc;       /* pointer to the UVC function structure */
};

static inline struct fuvc_softc * get_fuvc_from_gadget(struct usbd_gadget_driver * drv)
{
    struct fuvc_softc * ret;
    struct usbd_gadget_driver * _drv;

    ret = (struct fuvc_softc *) drv;
    _drv = &(ret->driver);

    ret = (struct fuvc_softc *) (((unsigned long) drv) -
        (((unsigned long) _drv) - ((unsigned long) ret)));
    return ret;
}

#include <gadget/usbd_video.h>

/* get the frame size of opened UVC handle */
extern int uvc_frame_size_get(uvc_t hdl, u32 * pw, u32 * ph, u32 * pf);

/*
 * function to check whether we've already connected to USB host,
 * return zero if not yet connected, non-zero if connected.
 */
extern int fuvc_host_connected(struct fuvc_softc * fuvc);

/*
 * function to initiate video stream transmission
 */
extern int fuvc_transfer_initiate(struct fuvc_softc * fuvc);

struct uvc_isoc_transfer {
    unsigned char * data;          /* where the data should be copied or stored */
    u32 length;               /* length of data expected or valid */
    u32 reserved;             /* how much data should be reserved */
    u32 res_next;             /* how much data expected to be reserved upon next request */
    u32 last;                 /* last piece of data in the frame? */
};

/*
 * transfer continue function, defined in `usbd_video.c
 */
extern int uvc_continue_transfer(struct fuvc_softc * fuvc, struct uvc_isoc_transfer * tran);

/* USB device UVC working task */
extern void * usbd_uvc_task(AARCHPTR arg0, AARCHPTR arg1, AARCHPTR arg2, AARCHPTR arg3);

struct fuvc_frame_info
{
    unsigned int width;
    unsigned int height;
    unsigned int intervals[8];
};

struct fuvc_format_info
{
    unsigned int    fcc;
    const struct fuvc_frame_info *frames;
};

extern void fuvc_frame_descriptors_get(struct fuvc_format_info *format_info);

#endif

