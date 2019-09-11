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
#ifndef F_SERIAL_H
#define F_SERIAL_H

#ifdef USB_GLOBAL_INCLUDE_FILE
#include USB_GLOBAL_INCLUDE_FILE
#endif

#include <controller/usb_device/usb_gadget.h>

typedef enum FERIAL_TASK_STATE_{
    FSERIAL_TASK_DISCONNECT = 0,
    FSERIAL_TASK_IDLE,
    FSERIAL_TASK_CONFIG_CHANGE,
    FSERIAL_TASK_REQ_COMMAND_PHASE,
    FSERIAL_TASK_DONE_COMMAND_PHASE,
    FSERIAL_TASK_REQ_DATA_PHASE,
    FSERIAL_TASK_DONE_DATA_PHASE,
    FSERIAL_TASK_REQ_STATUS_PHASE,
    FSERIAL_TASK_DONE_STATUS_PHASE,

    FSERIAL_TASK_REPORT_USB_STATUS,
    FSERIAL_TASK_NUM,
    FSERIAL_TASK_EXIT
}FSERIAL_TASK_STATE;

struct fserial_acm_config_desc {
    struct usb_config_descriptor confd;
    struct usb_interface_assoc_descriptor ifcad;
    struct usb_interface_descriptor ifcd;               /* acm control interface descriptor */
    struct usb_cdc_header_descriptor cdc_desc;
    struct usb_cdc_cm_descriptor cdc_call_desc;
    struct usb_cdc_acm_descriptor cdc_acm_desc;
    struct usb_cdc_union_desc cdc_union_desc;
    struct usb_endpoint_descriptor nepd;
    struct usb_interface_descriptor ifdd;
    struct usb_endpoint_descriptor iepd;
    struct usb_endpoint_descriptor oepd;
} __packed;

typedef struct fserial_softc{
    struct gs_port *port;

    spinlock_t          lock;
    struct cv           task_cv;
    struct mtx          *fs_mtx;

    struct fserial_acm_config_desc* config_dec;
    struct usbd_request ctrlreq;

    struct usbd_gadget_device* gadget;
    struct usbd_gadget_driver driver;    
}fserial_t;

#define to_fserialsofc(d)  container_of(d, struct fserial_softc, driver)

void fserial_request_complete(struct usbd_endpoint*,
                                               struct usbd_request*);


#endif

