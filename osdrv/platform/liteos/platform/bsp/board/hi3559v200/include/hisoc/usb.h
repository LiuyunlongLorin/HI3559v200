#ifndef __HISOC_USB_H__
#define    __HISOC_USB_H__

#include "los_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define CONFIG_HIUSB_EHCI_IOBASE                0x100b0000
#define CONFIG_HIUSB_EHCI_IOSIZE                0x00010000
#define CONFIG_HIUSBUDC_REG_BASE_ADDRESS        0x10080000
#define CONFIG_HIUSBUDC_REG_BASE_ADDRESS_LEN    0x40000
#define USB_CACHE_ALIGN_SIZE                    64

#define SKB_DATA_ALIGN(X)       ALIGN(X, USB_CACHE_ALIGN_SIZE)
#define PERI_USB                (IO_ADDRESS(MISC_REG_BASE + 0x78))
#define MISC_USB                (IO_ADDRESS(MISC_REG_BASE + 0x80))
#define PERI_CRG46              (IO_ADDRESS(CRG_REG_BASE + 0xb8))
#define USB_AHB_SRST_REQ        BIT(0)
#define USBPHY_REQ              BIT(1)
#define USBPHY_PORT0_TREQ       BIT(2)
#define USB_CTRL_HUB_REG        BIT(4)
#define USB_CTRL_UTMI0_REG      BIT(5)
#define USB_CKEN                BIT(7)

#define WORDINTERFACE           BIT(0)
#define SS_BURST4_EN            BIT(7)
#define SS_BURST8_EN            BIT(8)
#define SS_BURST16_EN           BIT(9)
#define USBOVR_P_CTRL           BIT(17)
#define USB2_PHY_DPPULL_DOWN    (3 << 26)
#define DWC_OTG_EN              BIT(31)
#define PERI_USB0_MARK          0x00FFFFFF
#define DEV_MODE_MARK           0x80000000

extern void hiusb_start_hcd(void);
extern void hiusb_stop_hcd(void);
extern void hiusb_reset_hcd(void);

extern void hiusb_host2device(void);
extern void hiusb_device2host(void);

extern int hiusb_is_device_mode(void);

extern void usb_otg_run(void);

extern void usb_otg_sw_set_host_state(void);
extern void usb_otg_sw_set_device_state(void);

extern void usb_otg_sw_clear_host_state(void);
extern void usb_otg_sw_clear_device_state(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
