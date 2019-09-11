#ifndef __USB_DEV3_DEFS_H__
#define __USB_DEV3_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#ifndef USB_SPEED_UNKNOWN
#define USB_SPEED_UNKNOWN 0
#endif

/* Feature numbers */
#define UF_DEVICE_B_HNP_ENABLE    3
#define UF_DEVICE_A_HNP_SUPPORT    4
#define UF_DEVICE_A_ALT_HNP_SUPPORT 5
#define WUF_WUSB        3
#define  WUF_TX_DRPIE        0x0
#define  WUF_DEV_XMIT_PACKET    0x1
#define  WUF_COUNT_PACKETS    0x2
#define  WUF_CAPTURE_PACKETS    0x3
#define UF_FUNCTION_SUSPEND    0

/* OTG feature selectors */
#define UOTG_B_HNP_ENABLE    3
#define UOTG_A_HNP_SUPPORT    4
#define UOTG_A_ALT_HNP_SUPPORT    5
#define UOTG_NTF_HOST_REL    51
#define UOTG_B3_RSP_ENABLE    52

#define UR_SET_ISOC_DELAY    0x31

/* super-speed companion descriptor related macros */
#define USSE_GET_MAX_STREAMS(a)        ((a) & 0x1f)
#define USSE_SET_MAX_STREAMS(a, b)    ((a) | ((b) & 0x1f))
#define USSE_GET_MAX_PACKET_NUM(a)    ((a) & 0x03)
#define USSE_SET_MAX_PACKET_NUM(a, b)    ((a) | ((b) & 0x03))
/**
 * @file
 *
 * This file contains OS-specific includes and definitions.
 *
 */

#define DWC_DRIVER_VERSION    "2.90b - November 2014"
#define DWC_DRIVER_DESC        "SS USB3 Controller driver"

/* Dwc3_usb error codes */
#define DWC3_USB_E_INVALID          EINVAL
#define DWC3_USB_E_NOT_SUPPORTED    EOPNOTSUPP
#define DWC3_USB_E_BUSY             EBUSY
#define DWC3_USB_E_SHUTDOWN         ESHUTDOWN
#define DWC3_USB_E_DISCONNECT       ECONNRESET

/** Compiler 'packed' attribute for structs */
#define UPACKED    __attribute__ ((__packed__))

/** @{ */
/** Type for DMA addresses */
typedef unsigned long        usb_dwc_dma_t;
#define DWC_DMA_ADDR_INVALID    (~(usb_dwc_dma_t)0)
/** @} */

/**
 * The number of DMA Descriptors (TRBs) to allocate for each endpoint type.
 * NOTE: The driver currently supports more than 1 TRB for Isoc EPs only.
 * So the values for Bulk and Intr must be 1.
 */
#define DWC_NUM_BULK_TRBS    1
#define DWC_NUM_INTR_TRBS    1
#define DWC_NUM_ISOC_TRBS    256

/**
 * These parameters may be specified when loading the module. They define how
 * the DWC_usb3 controller should be configured. The parameter values are passed
 * to the CIL initialization routine dwc_usb3_pcd_common_init().
 */
typedef struct dwc_usb3_core_params {
    int burst;
    int newcore;
    int phy;
    int wakeup;
    int pwrctl;
    int lpmctl;
    int phyctl;
    int usb2mode;
    int hibernate;
    int hiberdisc;
    int clkgatingen;
    int ssdisquirk;
    int nobos;
    int loop;
    int nump;
    int newcsr;
    int rxfsz;
    int txfsz[16];
    int txfsz_cnt;
    int baseline_besl;
    int deep_besl;
    int besl;
    int ebc;
} dwc_usb3_core_params_t;

#ifdef __cplusplus
}
#endif

#endif /* _DWC_LINUX_DEFS_H_ */
