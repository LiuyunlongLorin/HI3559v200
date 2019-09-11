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

#ifndef _F_UAC_GADGET_H
#define _F_UAC_GADGET_H
#include "gadget/composite.h"

/* private UAC device driver instance structure */
struct fuac_softc {
    unsigned char *              descs;           /* pointer to the concatenated descriptors */
    unsigned int                 desl;            /* length of all descriptors */
    unsigned int                 control;         /* Class-specific control request value */
    struct usbd_endpoint *       out_ep;         /* control transfer endpoint */
    struct usbd_endpoint *       in_ep;           /* Audio Streaming endpoint */

    struct usbd_request          ctrlreq;         /* control request */
    struct usbd_request          outputreq;      /* Audio Control request */
    struct usbd_request          inputreq;       /* Audio Streaming request */
    unsigned int                 dyn_fc;          /* dynamic format change, non-zero indicates that format change occurred */

    struct usbd_gadget_device *  gadget;          /* deivce instance */
    struct usbd_gadget_driver    driver;          /* gadget driver instance */

    volatile  int   connected;       /* Connected to USB host ? */
    unsigned char *              stream_buf;      /* Buffer used for stream transfer */
    unsigned int                 fid;             /* Frame Indentifier */

    struct list_head uac_queue;
    int uac_queue_len;
    volatile unsigned int busy_flag;
    struct uac_queue_node *cur_node;

    struct usb_function func;
} __attribute__((packed));

struct uac_ac_header_descriptor {
    uint8_t           bLength;
    uint8_t           bDescriptorType;
    uint8_t           bDescriptorSubtype;

    uint16_t          bcdADC;
    uint16_t          wTotalLength;

    uint8_t           bInCollection;
    uint8_t           baInterfaceNr[2];
} __attribute__((packed));
struct uac1_input_terminal_descriptor {
        uint8_t  bLength;                 /* in bytes: 12 */
        uint8_t  bDescriptorType;         /* CS_INTERFACE descriptor type */
        uint8_t  bDescriptorSubtype;      /* INPUT_TERMINAL descriptor subtype */
        uint8_t  bTerminalID;             /* Constant uniquely terminal ID */
        uint16_t wTerminalType;           /* USB Audio Terminal Types */
        uint8_t  bAssocTerminal;          /* ID of the Output Terminal associated */
        uint8_t  bNrChannels;             /* Number of logical output channels */
        uint16_t wChannelConfig;
        uint8_t  iChannelNames;
        uint8_t  iTerminal;
} __attribute__ ((packed));

#define UAC_DT_INPUT_TERMINAL_SIZE                        12
#define UAC_DT_OUTPUT_TERMINAL_SIZE                       9
#define UAC_INPUT_TERMINAL_MICROPHONE                   0x201

struct uac_output_terminal_descriptor {
        uint8_t  bLength;                        /* in bytes: 9 */
        uint8_t  bDescriptorType;                /* CS_INTERFACE descriptor type */
        uint8_t  bDescriptorSubtype;        /* OUTPUT_TERMINAL descriptor subtype */
        uint8_t  bTerminalID;        /* Constant uniquely terminal ID */
        uint16_t  wTerminalType;                /* USB Audio Terminal Types */
        uint8_t  bAssocTerminal;                /* ID of the Input Terminal associated */
        uint8_t  bSourceID;                /* ID of the connected Unit or Terminal*/
        uint8_t  iTerminal;
} __attribute__ ((packed));
/* 4.5.2 Class-Specific AS Interface Descriptor */
struct uac_as_header_descriptor {
    uint8_t  bLength;                   /* in bytes: 7 */
    uint8_t  bDescriptorType;           /* USB_DT_CS_INTERFACE */
    uint8_t  bDescriptorSubtype;        /* AS_GENERAL */
    uint8_t  bTerminalLink;             /* Terminal ID of connected Terminal */
    uint8_t  bDelay;                    /* Delay introduced by the data path */
    uint16_t wFormatTag;                /* The Audio Data Format */
} __attribute__((packed));
/* Audio Class-Specific AS Interface Descriptor Subtypes */
#define UAC_AS_GENERAL                     0x01
#define UAC_FORMAT_TYPE                    0x02
#define UAC_FORMAT_SPECIFIC                0x03
struct uac_format_type_i_discrete_descriptor {
        uint8_t  bLength;
        uint8_t  bDescriptorType;
        uint8_t  bDescriptorSubtype;
        uint8_t  bFormatType;
        uint8_t  bNrChannels;
        uint8_t  bSubframeSize;
        uint8_t  bBitResolution;
        uint8_t  bSamFreqType;
        uint8_t  tSamFreq[4][3];
} __attribute__ ((packed));

struct uac_endpoint_descriptor {
    uByte    bLength;
    uByte    bDescriptorType;
    uByte    bEndpointAddress;
#define    UE_GET_DIR(a)    ((a) & 0x80)
#define    UE_SET_DIR(a,d)    ((a) | (((d)&1) << 7))
#define    UE_DIR_IN    0x80        /* IN-token endpoint, fixed */
#define    UE_DIR_OUT    0x00        /* OUT-token endpoint, fixed */
#define    UE_DIR_RX    0xfd        /* for internal use only! */
#define    UE_DIR_TX    0xfe        /* for internal use only! */
#define    UE_DIR_ANY    0xff        /* for internal use only! */
#define    UE_ADDR        0x0f
#define    UE_ADDR_ANY    0xff        /* for internal use only! */
#define    UE_GET_ADDR(a)    ((a) & UE_ADDR)
    uByte    bmAttributes;
#define    UE_XFERTYPE    0x03
#define    UE_CONTROL    0x00
#define    UE_ISOCHRONOUS    0x01
#define    UE_BULK    0x02
#define    UE_INTERRUPT    0x03
#define    UE_BULK_INTR    0xfe        /* for internal use only! */
#define    UE_TYPE_ANY    0xff        /* for internal use only! */
#define    UE_GET_XFERTYPE(a)    ((a) & UE_XFERTYPE)
#define    UE_ISO_TYPE    0x0c
#define    UE_ISO_ASYNC    0x04
#define    UE_ISO_ADAPT    0x08
#define    UE_ISO_SYNC    0x0c
#define    UE_GET_ISO_TYPE(a)    ((a) & UE_ISO_TYPE)
    uWord    wMaxPacketSize;
#define    UE_ZERO_MPS 0xFFFF        /* for internal use only */
    uByte    bInterval;
    uByte  bRefresh;
    uByte  bSynchAddress;
}__attribute__((packed));

struct uac2_iso_endpoint_descriptor {
        uint8_t  bLength;                   /* in bytes: 8 */
        uint8_t  bDescriptorType;           /* USB_DT_CS_ENDPOINT */
        uint8_t  bDescriptorSubtype;        /* EP_GENERAL */
        uint8_t  bmAttributes;
        uint8_t  bmControls;
        uint8_t  bLockDelayUnits;
        uint16_t wLockDelay;
} __attribute__((packed));

/* 4.3.2.5 Feature Unit Descriptor */
struct uac_feature_unit_descriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint8_t bUnitID;
        uint8_t bSourceID;
        uint8_t bControlSize;
        uint16_t bmaControls[1]; /* variable length */
        uint8_t iFeature;
} __attribute__((packed));


#define COPY_UAC_DESCRIPTOR(_to, _fro) do { \
        const uint8_t * _fro_; \
        _fro_ = (const uint8_t *) (_fro); \
        memcpy((_to), (_fro), *_fro_); \
        (_to) += *_fro_; \
    } while (0)
/*
 * UAC Audio Class-specific Descriptor Types, UAC spec. 1.0
 */
#define UAC_CS_UNDEFINED            0x20
#define UAC_CS_DEVICE               0x21
#define UAC_CS_CONFIGURATION        0x22
#define UAC_CS_STRING               0x23
#define UAC_CS_INTERFACE            0x24
#define UAC_CS_ENDPOINT             0x25

/*
 * VideoStreaming Interface Control Selectors, UAC spec. 1.0
 */
#define UAC_VS_CONTROL_UNDEFINED                   0x00
#define UAC_VS_PROBE_CONTROL                       0x01
#define UAC_VS_COMMIT_CONTROL                      0x02
#define UAC_VS_STILL_PROBE_CONTROL                 0x03
#define UAC_VS_STILL_COMMIT_CONTROL                0x04
#define UAC_VS_STILL_IMAGE_TRIGGER_CONTROL         0x05
#define UAC_VS_STREAM_ERROR_CODE_CONTROL           0x06
#define UAC_VS_GENERATE_KEY_FRAME_CONTROL          0x07
#define UAC_VS_UPDATE_FRAME_SEGMENT_CONTROL        0x08
#define UAC_VS_SYNCH_DELAY_CONTROL                 0x09

#define UAC_SETCUR_COMPLETE 0xfe

#define    USB_ENDPOINT_XFER_SYNC        4
/* Audio Class-Specific AC Interface Descriptor Subtypes */
#define UAC_HEADER                      0x01
#define UAC_INPUT_TERMINAL              0x02
#define UAC_OUTPUT_TERMINAL             0x03
#define UAC_MIXER_UNIT                  0x04
#define UAC_SELECTOR_UNIT               0x05
#define UAC_FEATURE_UNIT                0x06
#define UAC1_PROCESSING_UNIT            0x07
#define UAC1_EXTENSION_UNIT             0x08

/* Audio Class-Specific Request Codes */

#define SET_CUR_UAC                     0x1
#define GET_CUR_UAC                     0x81
#define SET_MIN_UAC                     0x2
#define GET_MIN_UAC                     0x82
#define SET_MAX_UAC                     0x3
#define GET_MAX_UAC                     0x83
#define SET_RES_UAC                     0x4
#define GET_RES_UAC                     0x84
#define SET_MEM_UAC                     0x5
#define GET_MEM_UAC                     0x85


#define UAC_CS_INTERFACE            0x24
#define HSETM(_var, _val) _var = { (uint8_t)(_val), (uint8_t)(_val >> 0x8), (uint8_t)(_val >> 16) }
#endif

