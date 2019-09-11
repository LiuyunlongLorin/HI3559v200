#ifndef __USB_DEV3_OS_H__
#define __USB_DEV3_OS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "asm/delay.h"

/* Takes a usb ep pointer and returns the associated pcd ep pointer */
#define dwc_usb3_get_pcd_ep(usbep) \
    container_of((usbep), usb_dwc_pcd_ep_t, usb_ep)

/* Register read/write. */
#define dwc_rd32(ptrDev, addr)      GET_UINT32(addr)
#define dwc_wr32(ptrDev, addr, val) WRITE_UINT32(val, addr)

static inline void wmb(void)
{
    __asm__ __volatile__ ("" : : "r"(0) : "memory");
}

static inline void dwc_udelay(void * what, unsigned int hl)
{
    LOS_Udelay(hl);
}

static inline void dwc_mdelay(void *dev, unsigned int ms)
{
    LOS_Mdelay(ms);
}

static inline void dwc_msleep(void * dev, unsigned int ms)
{
    msleep(ms);
}

/** Prefix string for print macros. */
#define USB3_DWC    "dwc_usb3: "
struct dev_ice {
    int notused;
};

extern int dwc_usb3_gadget_init(struct usb3_dwc_device *usb3_dev, struct dev_ice *dev);
extern void dwc_usb3_gadget_remove(struct usb3_dwc_device *usb3_dev);
extern int dwc_usb3_create_dev_files(struct dev_ice *dev);
extern void dwc_usb3_remove_dev_files(struct dev_ice *dev);
extern int dwc_usb3_wakeup(struct usbd_gadget_device *gadget);
extern int dwc_wait_pme_thread(void *data);

#ifdef __cplusplus
}
#endif

#endif /* _DWC_LINUX_DEV_H_ */
