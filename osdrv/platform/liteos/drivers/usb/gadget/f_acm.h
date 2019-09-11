#ifndef F_ACM_H
#define F_ACM_H

#ifdef USB_GLOBAL_INCLUDE_FILE
#include USB_GLOBAL_INCLUDE_FILE
#endif

#include "gadget/u_serial.h"

#ifdef LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER
#define DWC3_USB_SERIAL
#endif

#ifdef DWC3_USB_SERIAL
#define MAX_PACKET_SIZE 0x400
#else
#define MAX_PACKET_SIZE 0x200
#endif

struct acm_hs_function_descriptor {
    struct usb_interface_assoc_descriptor ifcad;
    struct usb_interface_descriptor ifcd;               /* acm control interface descriptor */
    struct usb_cdc_header_descriptor cdc_desc;
    struct usb_cdc_cm_descriptor cdc_call_desc;
    struct usb_cdc_acm_descriptor cdc_acm_desc;
    struct usb_cdc_union_desc cdc_union_desc;
    struct usb_endpoint_descriptor nepd;
#ifdef DWC3_USB_SERIAL
    struct usb_endpoint_ss_comp_descriptor ncompd;
#endif
    struct usb_interface_descriptor ifdd;
    struct usb_endpoint_descriptor iepd;
#ifdef DWC3_USB_SERIAL
    struct usb_endpoint_ss_comp_descriptor icompd;
#endif
    struct usb_endpoint_descriptor oepd;
#ifdef DWC3_USB_SERIAL
    struct usb_endpoint_ss_comp_descriptor ocompd;
#endif
} __packed;

struct acm_hs_descriptor {
    struct usb_config_descriptor            *acm_config;
    struct acm_hs_function_descriptor       *acm_func;
} __packed;

struct f_acm {
    struct gserial  port;
    UINT8           ctrl_id,data_id;
    UINT8           port_num;
    UINT8           pending;

    /* lock is mostly for pending and notify_req ... they get accessed
     * by callbacks both from tty (open/close/break) under its spinlock,
     * and notify_req.complete() which can't use that lock.
     */
    spinlock_t lock;

    struct usbd_endpoint  *notify;
    struct usbd_request   *notify_req;

    struct usb_cdc_line_state  port_line_state;   /* 8-N-1 etc */

    /* SetControlLineState request */
    UINT16          port_handshake_bits;
#define ACM_CTRL_RTS    (1 << 1)    /* unused with full duplex */
#define ACM_CTRL_DTR    (1 << 0)    /* host is ready for data r/w */

    /* SerialState notification */
    UINT16          serial_state;
#define ACM_CTRL_OVERRUN    (1 << 6)
#define ACM_CTRL_PARITY     (1 << 5)
#define ACM_CTRL_FRAMING    (1 << 4)
#define ACM_CTRL_RI         (1 << 3)
#define ACM_CTRL_BRK        (1 << 2)
#define ACM_CTRL_DSR        (1 << 1)
#define ACM_CTRL_DCD        (1 << 0)

};

#endif

