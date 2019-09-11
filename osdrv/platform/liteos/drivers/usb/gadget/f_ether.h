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
#ifndef F_ETHER_H
#define F_ETHER_H
#include "implementation/freebsd_sys.h"
#include "gadget/u_ether.h"

extern struct fether_rndis_config_desc fether_rndis_confd;

struct fether_rndis_config_desc {
    struct usb_config_descriptor confd;
    struct usb_interface_assoc_descriptor ifcad;
    struct usb_interface_descriptor ifcd;
    struct usb_cdc_header_desc cdc_desc;
    struct usb_cdc_call_mgmt_descriptor cdc_call_desc;
    struct usb_cdc_acm_descriptor cdc_acm_desc;
    struct usb_cdc_union_desc cdc_union_desc;
    struct usb_endpoint_descriptor nepd;
    struct usb_interface_descriptor ifdd;
    struct usb_endpoint_descriptor iepd;
    struct usb_endpoint_descriptor oepd;
} __packed;

struct fether_softc {
    struct eth_dev *dev;
    struct fether_rndis_config_desc *config_dec;
    struct usbd_request ctrlreq;
    struct usbd_gadget_device* gadget;
    struct usbd_gadget_driver driver;

    spinlock_t     lock;
    struct cv task_cv;
    struct mtx task_mtx;
};

#define to_fethersofc(d)  container_of(d, struct fether_softc, driver)

void fether_request_complete(struct usbd_endpoint* ep,
    struct usbd_request *req);

#endif

