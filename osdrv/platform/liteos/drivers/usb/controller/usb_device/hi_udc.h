#ifndef _HI_UDC_H
#define _HI_UDC_H

#define HI3516A_USB_MAX_DEVICES (USB_MIN_DEVICES + 1)

#include <controller/usb_device/dwc_otg_pcd.h>
#include "controller/usb_device/usb_gadget.h"

struct hiudc_sof_record{
    #define MAX_SOF_RECORD_NUM 5
    int Record[MAX_SOF_RECORD_NUM];
    int curRecord;
};

struct hi_udc_softc {
    dwc_otg_pcd_t sc_pcd;            /* must be first */
    struct usb_callout timeout_handle;
    struct mtx bus_mtx;
    struct hiudc_sof_record sofrec;
};

#endif
