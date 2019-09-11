/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
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

#ifndef _USB_F_DFU_H_
#define _USB_F_DFU_H_

#include "implementation/freebsd_sys.h"
#include "composite.h"

#define USB_DFU_DT_FUNC            0x21
#define USB_DFU_USB_BUFSIZ            4096

#define USB_DFU_BIT_WILL_DETACH        (0x1 << 3)
#define USB_DFU_BIT_MANIFESTATION_TOLERANT    (0x1 << 2)
#define USB_DFU_BIT_CAN_UPLOAD        (0x1 << 1)
#define USB_DFU_BIT_CAN_DNLOAD        0x1

/* The status of Flash upgrade state */
#define DFU_FLASH_STATE_UPGRADING   1
#define DFU_FLASH_STATE_UPGRADED    0
#define DFU_FLASH_STATE_ERROR          -1

#define DFU_DETACH        0x00
#define DFU_DNLOAD        0x01
#define DFU_UPLOAD        0x02
#define DFU_GETSTATUS        0x03
#define DFU_CLRSTATUS        0x04
#define DFU_GETSTATE        0x05
#define DFU_ABORT        0x06
#define REQ_DFU_FLASH_STATE 0x07

#define USB_DFU_STATUS_OK            0x00
#define USB_DFU_STATUS_errUNKNOWN        0x0e

#define DFU_RET_STALL            -1
#define DFU_RET_ZLP                0
#define DFU_RET_STAT_LEN            6

enum usb_dfu_state {
    USB_DFU_STATE_appIDLE        = 0,
    USB_DFU_STATE_appDETACH        = 1,
    USB_DFU_STATE_dfuIDLE        = 2,
    USB_DFU_STATE_dfuDNLOAD_SYNC    = 3,
    USB_DFU_STATE_dfuDNBUSY        = 4,
    USB_DFU_STATE_dfuDNLOAD_IDLE    = 5,
    USB_DFU_STATE_dfuMANIFEST_SYNC    = 6,
    USB_DFU_STATE_dfuMANIFEST        = 7,
    USB_DFU_STATE_dfuMANIFEST_WAIT_RST    = 8,
    USB_DFU_STATE_dfuUPLOAD_IDLE    = 9,
    USB_DFU_STATE_dfuERROR        = 10,
};

struct usb_dfu_status {
    u8                bStatus;
    u8                bwPollTimeout[3];
    u8                bState;
    u8                bString;
} __packed;

struct dfu_function_descriptor {
    u8                bLength;
    u8                bDescriptorType;
    u8                bmAttributes;
    u16               wDetachTimeOut;
    u16               wTransferSize;
    u16               bcdDFUVersion;
} __packed;


#define USB_DFU_POLL_TIMEOUT_MASK           (0xFFFFFFUL)
#endif /* _USB_F_DFU_H_ */
