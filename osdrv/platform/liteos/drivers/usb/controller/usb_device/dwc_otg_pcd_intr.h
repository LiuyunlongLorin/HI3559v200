#ifndef __DWC_OTG_PCD_IRQ_H__
#define __DWC_OTG_PCD_IRQ_H__

#include "controller/usb_device/dwc_otg_pcd.h"
int dwc_otg_irq(void * dwc_arg);
void endpoint0_startup_receiving_setup(dwc_otg_pcd_t* pcd);

/**
 * States of EP0.
 */
typedef enum ep0_state {
    EP0_DISCONNECT = 0,        /* no host */
    EP0_IDLE,
    EP0_IN_DATA_PHASE,
    EP0_OUT_DATA_PHASE,
    EP0_IN_STATUS_PHASE,
    EP0_OUT_STATUS_PHASE,
    EP0_STALL,
} ep0state_e;

#endif
