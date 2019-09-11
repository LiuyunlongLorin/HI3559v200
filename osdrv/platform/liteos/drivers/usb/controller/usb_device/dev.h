#ifndef __USB_DEV3_DEV_H__
#define __USB_DEV3_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif

/*#include "implementation/_os_ref.h"
#include "implementation/_intr_ref.h"
//#include "implementation/_queue_ref.h"
#include "implementation/_bsd_kernel_ref.h"*/


/* 64bit */
#ifdef LOSCFG_AARCH64
#define DWC_USB3_64B_ARCH
#endif

/**
 * This structure is a wrapper that encapsulates the driver components used to
 * manage a single DWC_usb3 controller.
 */
typedef struct usb3_dwc_device {
    /**
     * OS-specific stuff. KEEP THIS AT THE VERY BEGINNING OF THE DEVICE
     * STRUCT. OSes such as FreeBSD and NetBSD require this.
     */

    /** Base address returned from ioremap() */
    volatile unsigned char *base;
    volatile unsigned char *gasket_base;

    /** Offset to 'gasket' registers (Synopsys FPGA only) */
    int gasket_ofs;

    /** Device context */
    void *dev;

    /** IRQ resource */
    int irq;

    /** Count of threads inside Gadget API */
    int hiber_cnt;

    /** Hibernation state */
    int hibernate;

#define DWC_HIBER_AWAKE        0
#define DWC_HIBER_ENTER_NOSAVE    1
#define DWC_HIBER_ENTER_SAVE    2
#define DWC_HIBER_SLEEPING    3
#define DWC_HIBER_WAIT_LINK_UP    4
#define DWC_HIBER_WAIT_U0    5
#define DWC_HIBER_SS_DIS_QUIRK    6

    int pme_ready;

    /** PCD structure */
    struct usb3_dwc_pcd pcd;

    /** Value from SNPSID register */
    unsigned int snpsid;

    /** Parameters that define how the core should be configured */
    dwc_usb3_core_params_t *core_params;

    /** Core Global registers starting at offset 100h */
    dwc_usb3_core_global_regs_t *core_global_regs;

    /** @{ */
#define DWC_EVENT_BUF_SIZE    256    // size in dwords
#define DWC_NUM_EVENT_BUFS    1
    /** Event Buffers for receiving interrupts. Up to 32 buffers are
     * supported by the hardware, but we only use 1.
     */
    unsigned int *event_ptr[DWC_NUM_EVENT_BUFS];
    unsigned int *event_buf[DWC_NUM_EVENT_BUFS];
    usb_dwc_dma_t event_buf_dma[DWC_NUM_EVENT_BUFS];
    /** @} */

    /** Total RAM for FIFOs (Bytes) */
    unsigned short total_fifo_size;

    /** Size of Rx FIFO (Bytes) */
    unsigned short rx_fifo_size;

    /** @{ */
    /** Hardware Configuration - stored here for convenience */
    unsigned int hwparams0;
    unsigned int hwparams1;
    unsigned int hwparams2;
    unsigned int hwparams3;
    unsigned int hwparams4;
    unsigned int hwparams5;
    unsigned int hwparams6;
    unsigned int hwparams7;
    unsigned int hwparams8;
    /** @} */

    /** @{ */
    /** Register state, saved across core hibernation */
    unsigned int dcfg_save;
    unsigned int dctl_save;
    unsigned int gtxfifosiz0_save;
    unsigned int gtxfifosiz1_save;
    unsigned int gtxfifosiz2_save;
    unsigned int gtxfifosiz3_save;
    unsigned int grxfifosiz0_save;
    unsigned int guctl_save;
    unsigned int guctl1_save;
    /** @} */

    /** @{ */
    /** Hooks for customizing device initialization. See
     *  dwc_usb3_pcd_device_init() in cil.c to see how these work.
     */
    void (*soft_reset_hook)(struct usb3_dwc_device *dev, int softrst, int rstor);
    void (*phy_config_hook)(struct usb3_dwc_device *dev, int softrst, int rstor);
    void (*fifo_sizing_hook)(struct usb3_dwc_device *dev, int softrst, int rstor);
    void (*gctl_init_hook)(struct usb3_dwc_device *dev, int softrst, int rstor);
    void (*set_address_hook)(struct usb3_dwc_device *dev, int softrst, int rstor);
    void (*ep0_start_hook)(struct usb3_dwc_device *dev, int softrst, int rstor);
    /** @} */

    /** Value to write into the DCTL HIRD_Thresh field on register
     * initialization. If 0 then a default value of 0x1c will be used.
     */
    unsigned int hird_thresh;

    /** Values to write into GSBUSCFG0 and GSBUSCFG1 on initialization or
     * when exiting from hibernation. 'program_gsbuscfg' below must also be
     * set to 1 to enable the writing of these values.
     */
    unsigned int gsbuscfg0;
    unsigned int gsbuscfg1;

    /** True if common functionality has been initialized */
    unsigned int cmn_initialized        : 1;

    /** True if Gadget has been initialized */
    unsigned int gadget_initialized        : 1;

    /** True if PCD has been initialized */
    unsigned int pcd_initialized        : 1;

    /** True if common IRQ handler has been installed */
    unsigned int cmn_irq_installed        : 1;

    /** True if sysfs functions have been installed */
    unsigned int sysfs_initialized        : 1;

    /** True if waiting for connect before resuming from hibernation */
    unsigned int hiber_wait_connect        : 1;

    /** True if waiting for U0 state before sending remote wake */
    unsigned int hiber_wait_u0        : 1;

    /** True if GBUSCFG0/GBUSCFG1 should be written with the above
     * values when exiting hibernation */
    unsigned int program_gsbuscfg        : 1;
} dwc_usb3_device_t;

#ifdef __cplusplus
}
#endif

#endif /* _DWC_DEV_H_ */
