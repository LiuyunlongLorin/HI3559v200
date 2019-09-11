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

#ifndef __USB_GADGET_H__
#define __USB_GADGET_H__

#include "implementation/freebsd_sys.h"
#include "fs/fs.h"

/* Added in device3.0 */
/* A dma_addr_t can hold any valid DMA or bus address for the platform */
#if defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER) || defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER_FOR_PORT2)
typedef AARCHPTR dma_addr_t;
#endif

#define USB_COMP_EP0_BUFSIZ 1024

struct usbd_endpoint;

struct usbd_request {
    void *buf;
    unsigned    nlength;  //fill by function
    unsigned    alength;  //fill by udc

    /* Added in device3.0 */
#if defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER) || defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER_FOR_PORT2)
    dma_addr_t  dma;
#endif

    struct scatterlist *sg;
    unsigned    num_sgs;
    unsigned    num_mapped_sgs;

    unsigned    stream_id:16;
    unsigned    no_interrupt:1;
    unsigned    zero:1;
    unsigned    short_not_ok:1;

    void(*complete)(struct usbd_endpoint* ep,
                    struct usbd_request *req);
    void *context;      //point usbd_ep*
    int status;         //fill by udc
    int is_complete;

    struct list_head list;
};

struct usbd_endpoint_ops {
    int (*enable) (struct usbd_endpoint *ep);
    int (*disable) (struct usbd_endpoint *ep);

    int (*queue) (struct usbd_endpoint *ep, struct usbd_request *req);
    int (*dequeue) (struct usbd_endpoint *ep, struct usbd_request *req);

    int (*set_halt) (struct usbd_endpoint *ep, int value);
    int (*set_wedge) (struct usbd_endpoint *ep);

    int (*fifo_status) (struct usbd_endpoint *ep);
    void (*fifo_flush) (struct usbd_endpoint *ep);
};

struct usbd_endpoint {
    void                *driver_data;
    char                name[8];
    struct list_head    ep_node;

    /* Added in device3.0 */
#if defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER) || defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER_FOR_PORT2)
    unsigned    maxpacket:16;
    unsigned    maxpacket_limit:16;
    unsigned    max_streams:16;
    unsigned    mult:2;
    unsigned    maxburst:5;
#endif

    struct usbd_endpoint_ops *ops;
    struct usb_endpoint_descriptor *desc;
    struct usbd_request    *handle_req; //set&clear by udc
    void    *drv_context;   //for function
    void    *ep_link;       //link pcd_ep

    /* Added in device3.0 */
#if defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER) || defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER_FOR_PORT2)
    const struct usb_endpoint_ss_comp_descriptor *comp_desc;
#endif
};

struct usbd_gadget_device{
    struct usbd_endpoint *ep0;
    struct list_head ep_list;    /* of usbd_ep */
    enum usb_dev_speed speed;   //fs,hs

    /* Added in device3.0 */
#if defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER) || defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER_FOR_PORT2)
    enum usb_dev_speed      max_speed;
#endif

    enum usb_dev_state state;   //attach,detach,pw,addr,config
    const char  *name;
    unsigned out_epnum;
    unsigned in_epnum;
    void* drvdata;
} ;

struct usbd_gadget_driver {
    char *function;

    /* Added in device3.0 */
#if defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER) || defined(LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER_FOR_PORT2)
    enum usb_dev_speed    max_speed;
#endif

    int(*bind)(struct usbd_gadget_device *gadget,
               struct usbd_gadget_driver *driver);
    void(*unbind)(struct usbd_gadget_device *);
    int(*setup)(struct usbd_gadget_device *,
                const struct usb_device_request *);
    void (*disconnect)(struct usbd_gadget_device *);
    void (*suspend)(struct usbd_gadget_device *);
    void (*resume)(struct usbd_gadget_device *);
    void (*reset)(struct usbd_gadget_device *);

    /* FIXME support safe rmmod */
    /* struct device_driver    driver; */
};

int usbd_gadget_attach_driver(void* context,
        struct usbd_gadget_driver *driver);
int usbd_gadget_detach_driver(void * context,
        struct usbd_gadget_driver *driver);

int usbd_enable_endpoint(struct usbd_endpoint* ep);
int usbd_disable_endpoint(struct usbd_endpoint* ep);
int usbd_endpoint_request(struct usbd_endpoint* ep,
                           struct usbd_request* req);
int usbd_set_endpoint_halt(struct usbd_endpoint* ep);
int usbd_clear_endpoint_halt(struct usbd_endpoint* ep);

struct usbd_endpoint*
usbd_endpoint_autoconfig( struct usbd_gadget_device* gadget,
                          struct usb_endpoint_descriptor* desc);
void usbd_endpoint_autoconfig_reset(struct usbd_gadget_device * gadget);

/**
 * gadget_is_superspeed() - return true if the hardware handles superspeed
 * @g: controller that might support superspeed
 */
static inline int gadget_is_superspeed(struct usbd_gadget_device *gadget)
{
    return 0;
}

#endif
