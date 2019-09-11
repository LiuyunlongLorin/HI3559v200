#ifndef __USB_DEV3_HW_H__
#define __USB_DEV3_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <los_typedef.h>
#include "implementation/_types_ref.h"

typedef enum dwc_grxthrcfg_data {
    USB_DWC_RXTHRCTL_MAX_RX_BURST_SIZE_BITS        = 0x00f80000,
    USB_DWC_RXTHRCTL_MAX_RX_BURST_SIZE_SHIFT    = 19,

    USB_DWC_RXTHRCTL_RX_PKT_CNT_BITS        = 0x0f000000,
    USB_DWC_RXTHRCTL_RX_PKT_CNT_SHIFT        = 24,

    USB_DWC_RXTHRCTL_RX_PKT_CNT_EN_BIT        = 0x20000000,
} dwc_grxthrcfg_data_t;

typedef enum dwc_gctl_data {
    USB_DWC_GCTL_DSBL_CLCK_GTNG_BIT    = 0x00000001,
    USB_DWC_GCTL_GBL_HIBER_EN_BIT    = 0x00000002,
    USB_DWC_GCTL_DIS_SCRAMBLE_BIT    = 0x00000008,
    USB_DWC_GCTL_SCALE_DOWN_BITS    = 0x00000030,
    USB_DWC_GCTL_DEBUG_ATTACH_BIT    = 0x00000100,
    USB_DWC_GCTL_PRT_CAP_DIR_BITS    = 0x00003000,
    USB_DWC_GCTL_PRT_CAP_DIR_SHIFT    = 12,
    USB_DWC_GCTL_PRT_CAP_DEVICE        = 2,
    USB_DWC_GCTL_PWR_DN_SCALE_BITS    = 0xfff80000,
    USB_DWC_GCTL_PWR_DN_SCALE_SHIFT    = 19,
} dwc_gctl_data_t;

typedef enum dwc_gsts_data {
    USB_DWC_GSTS_HOST_MODE        = 1,
} dwc_gsts_data_t;

typedef enum dwc_ghwparams0_data {
    USB_DWC_HWP0_MDWIDTH_BITS        = 0x0000ff00,
    USB_DWC_HWP0_MDWIDTH_SHIFT        = 8,
} dwc_ghwparams0_data_t;

typedef enum dwc_ghwparams1_data {
    USB_DWC_HWP1_EN_PWROPT_BITS            = 0x03000000,
    USB_DWC_HWP1_EN_PWROPT_SHIFT        = 24,
    USB_DWC_EN_PWROPT_HIBERNATION    = 2,
} dwc_ghwparams1_data_t;

typedef enum dwc_ghwparams3_data {
    USB_DWC_HWP3_SSPHY_IFC_BITS        = 0x00000003,
} dwc_ghwparams3_data_t;

typedef enum dwc_ghwparams7_data {
    USB_DWC_HWP7_RAM1_DEPTH_BITS    = 0x0000ffff,
    USB_DWC_HWP7_RAM1_DEPTH_SHIFT    = 0,

    USB_DWC_HWP7_RAM2_DEPTH_BITS    = 0xffff0000,
    USB_DWC_HWP7_RAM2_DEPTH_SHIFT    = 16,
} dwc_ghwparams7_data_t;

typedef enum dwc_gusb2phycfg_data {
    USB_DWC_USB2PHYCFG_16B_PHY_IF_BIT        = 0x00000008,
    USB_DWC_USB2PHYCFG_SUS_PHY_BIT        = 0x00000040,
    USB_DWC_USB2PHYCFG_ENBL_SLP_M_BIT        = 0x00000100,
    USB_DWC_USB2PHYCFG_USB_TRD_TIM_BITS        = 0x00003c00,
    USB_DWC_USB2PHYCFG_USB_TRD_TIM_SHIFT    = 10,
} dwc_gusb2phycfg_data_t;

typedef enum dwc_gusb3pipectl_data {
    USB_DWC_PIPECTL_TX_DEMPH_SHIFT        = 1,
    USB_DWC_PIPECTL_LFPS_FILTER_BIT        = 0x00000200,
    USB_DWC_PIPECTL_SUS_PHY_BIT            = 0x00020000,
} dwc_gusb3pipectl_data_t;

typedef enum dwc_gfifosize_data {
    USB_DWC_FIFOSZ_DEPTH_SHIFT        = 0,
    USB_DWC_FIFOSZ_STARTADDR_BITS    = 0xffff0000,
    USB_DWC_FIFOSZ_STARTADDR_SHIFT    = 16,
} dwc_gfifosize_data_t;

typedef enum dwc_geventsiz_data {
    USB_DWC_EVENTSIZ_INT_MSK_BIT    = 0x80000000,
} dwc_geventsiz_data_t;

typedef enum dwc_geventcnt_data {
    USB_DWC_EVENTCNT_CNT_BITS        = 0x0000ffff,
} dwc_geventcnt_data_t;

typedef enum dwc_gevent_data {
    USB_DWC_EVENT_NON_EP_BIT            = 0x01,
    USB_DWC_EVENT_INTTYPE_BITS            = 0xfe,
    USB_DWC_EVENT_INTTYPE_SHIFT            = 1,
    USB_DWC_EVENT_DEV_INT        = 0,
} dwc_gevent_data_t;

typedef enum dwc_devt_data {
    USB_DWC_DEVT_BITS                = 0x00000f00,
    USB_DWC_DEVT_SHIFT                = 8,

    USB_DWC_DEVT_DISCONN        = 0,
    USB_DWC_DEVT_USBRESET        = 1,
    USB_DWC_DEVT_CONNDONE        = 2,
    USB_DWC_DEVT_ULST_CHNG        = 3,
    USB_DWC_DEVT_WKUP            = 4,
    USB_DWC_DEVT_HIBER_REQ        = 5,
    USB_DWC_DEVT_U3_L2L1_SUSP        = 6,
    USB_DWC_DEVT_SOF            = 7,
    USB_DWC_DEVT_ERRATICERR        = 9,
    USB_DWC_DEVT_CMD_CMPL        = 10,
    USB_DWC_DEVT_OVERFLOW        = 11,

    USB_DWC_DEVT_ULST_STATE_BITS        = 0x000f0000,
    USB_DWC_DEVT_ULST_STATE_SHIFT        = 16,
    USB_DWC_LINK_STATE_U0        = 0,
    USB_DWC_LINK_STATE_U3        = 3,
    USB_DWC_LINK_STATE_SLEEP        = 2,

    USB_DWC_DEVT_ULST_SS_BIT            = 0x00100000,
    USB_DWC_DEVT_ULST_SS_SHIFT            = 20,

#define USB_DWC_DEVT_HIBER_STATE_BITS    USB_DWC_DEVT_ULST_STATE_BITS
#define USB_DWC_DEVT_HIBER_STATE_SHIFT    USB_DWC_DEVT_ULST_STATE_SHIFT
#define USB_DWC_DEVT_HIBER_SS_BIT        USB_DWC_DEVT_ULST_SS_BIT
} dwc_devt_data_t;

typedef enum dwc_depevt_data {
    USB_DWC_DEPEVT_EPNUM_BITS            = 0x0000003e,
    USB_DWC_DEPEVT_EPNUM_SHIFT            = 1,

    USB_DWC_DEPEVT_INTTYPE_BITS            = 0x000003c0,
    USB_DWC_DEPEVT_INTTYPE_SHIFT        = 6,

    USB_DWC_DEPEVT_XFER_CMPL        = 1,
    USB_DWC_DEPEVT_XFER_IN_PROG        = 2,
    USB_DWC_DEPEVT_XFER_NRDY        = 3,
    USB_DWC_DEPEVT_FIFOXRUN        = 4,
    USB_DWC_DEPEVT_EPCMD_CMPL        = 7,

    USB_DWC_DEPEVT_ISOC_UFRAME_NUM_BITS        = 0xffff0000,
    USB_DWC_DEPEVT_ISOC_UFRAME_NUM_SHIFT    = 16,
} dwc_depevt_data_t;

typedef struct dwc_geventbuf_data {
    volatile u32 geventadr_lo;
    volatile u32 geventadr_hi;
    volatile u32 geventsiz;
    volatile u32 geventcnt;
} dwc_geventbuf_data_t;

typedef struct dwc_usb3_core_global_regs {
#define USB_DWC_CORE_GLOBAL_REG_OFFSET    0x100
    volatile u32 gsbuscfg0;
    volatile u32 gsbuscfg1;
    volatile u32 gtxthrcfg;
    volatile u32 grxthrcfg;
    volatile u32 gctl;
    volatile u32 gevten;
    volatile u32 gsts;
    volatile u32 guctl1;
    volatile u32 gsnpsid;
    volatile u32 ggpio;
    volatile u32 guid;
    volatile u32 guctl;
    volatile u32 gbuserraddrlo;
    volatile u32 gbuserraddrhi;
    volatile u32 gprtbimap_lo;
    volatile u32 gprtbimap_hi;
    volatile u32 ghwparams0;
    volatile u32 ghwparams1;
    volatile u32 ghwparams2;
    volatile u32 ghwparams3;
    volatile u32 ghwparams4;
    volatile u32 ghwparams5;
    volatile u32 ghwparams6;
    volatile u32 ghwparams7;
    volatile u32 gdbgfifospace;
    volatile u32 gdbgltssm;
    volatile u32 gdbglnmcc;
    volatile u32 gdbgbmu;
    volatile u32 gdbglspmux;
    volatile u32 gdbglsp;
    volatile u32 gdbgepinfo0;
    volatile u32 gdbgepinfo1;
    volatile u32 gprtbimap_hs_lo;
    volatile u32 gprtbimap_hs_hi;
    volatile u32 gprtbimap_fs_lo;
    volatile u32 gprtbimap_fs_hi;
    volatile u32 reserved4[12];
    volatile u32 gusb3rmmictl[16];
    volatile u32 gusb2phycfg[16];
    volatile u32 gusb2i2cctl[16];
    volatile u32 gusb2phyacc[16];
    volatile u32 gusb3pipectl[16];
    volatile u32 gtxfifosiz[32];
    volatile u32 grxfifosiz[32];
    struct dwc_geventbuf_data geventbuf[32];
    volatile u32 ghwparams8;
} dwc_usb3_core_global_regs_t;

typedef enum dwc_dcfg_data {
    USB_DWC_DCFG_DEVSPD_BITS        = 0x000007,
    USB_DWC_DCFG_DEVSPD_SHIFT        = 0,

    USB_DWC_SPEED_HS_PHY_30MHZ_OR_60MHZ        = 0,
    USB_DWC_SPEED_FS_PHY_30MHZ_OR_60MHZ        = 1,
    USB_DWC_SPEED_LS_PHY_6MHZ            = 2,
    USB_DWC_SPEED_FS_PHY_48MHZ            = 3,
    USB_DWC_SPEED_SS_PHY_125MHZ_OR_250MHZ    = 4,

    USB_DWC_DCFG_DEVADDR_BITS        = 0x0003f8,
    USB_DWC_DCFG_DEVADDR_SHIFT        = 3,

    USB_DWC_DCFG_NUM_RCV_BUF_BITS    = 0x3e0000,
    USB_DWC_DCFG_NUM_RCV_BUF_SHIFT    = 17,

    USB_DWC_DCFG_LPM_CAP_BIT        = 0x400000,
} dwc_dcfg_data_t;

typedef enum dwc_dctl_data {
    USB_DWC_DCTL_TSTCTL_BITS            = 0x0000001e,
    USB_DWC_DCTL_TSTCTL_SHIFT            = 1,
    USB_DWC_DCTL_ULST_CHNG_REQ_BITS        = 0x000001e0,
    USB_DWC_DCTL_ULST_CHNG_REQ_SHIFT        = 5,
    USB_DWC_LINK_STATE_REQ_RX_DETECT        = 5,
    USB_DWC_DCTL_ACCEPT_U1_EN_BIT        = 0x00000200,
    USB_DWC_DCTL_INIT_U1_EN_BIT            = 0x00000400,
    USB_DWC_DCTL_INIT_U2_EN_BIT            = 0x00001000,
    USB_DWC_DCTL_L1_HIBER_EN_BIT        = 0x00040000,
    USB_DWC_DCTL_KEEP_CONNECT_BIT        = 0x00080000,
    USB_DWC_DCTL_APP_L1_RES_BIT            = 0x00800000,
    USB_DWC_DCTL_HIRD_THR_BITS            = 0x1f000000,
    USB_DWC_DCTL_HIRD_THR_SHIFT            = 24,
    USB_DWC_DCTL_CSFT_RST_BIT            = 0x40000000,
    USB_DWC_DCTL_RUN_STOP_BIT            = 0x80000000,
} dwc_dctl_data_t;

typedef enum dwc_devten_data {
    USB_DWC_DEVTEN_DISCONN_BIT        = 0x0001,
    USB_DWC_DEVTEN_USBRESET_BIT        = 0x0002,
    USB_DWC_DEVTEN_CONNDONE_BIT        = 0x0004,
    USB_DWC_DEVTEN_ULST_CHNG_BIT    = 0x0008,
    USB_DWC_DEVTEN_WKUP_BIT        = 0x0010,
    USB_DWC_DEVTEN_HIBER_REQ_BIT    = 0x0020,
    USB_DWC_DEVTEN_U3_L2L1_SUSP_BIT    = 0x0040,
} dwc_devten_data_t;

typedef enum dwc_dsts_data {
    USB_DWC_DSTS_CONNSPD_BITS        = 0x00000007,
    USB_DWC_DSTS_CONNSPD_SHIFT        = 0,
    USB_DWC_DSTS_SOF_FN_BITS        = 0x0001fff8,
    USB_DWC_DSTS_SOF_FN_SHIFT        = 3,
    USB_DWC_DSTS_USBLNK_STATE_BITS    = 0x003c0000,
    USB_DWC_DSTS_USBLNK_STATE_SHIFT    = 18,
} dwc_dsts_data_t;

typedef enum dwc_dgcmdpar_data {
    USB_DWC_DGCMD_FUNCTION_WAKE_DEV_NOTIF     = 1,
    USB_DWC_DGCMD_LATENCY_TOL_DEV_NOTIF       = 2,
    USB_DWC_DGCMDPAR_DEV_NOTIF_PARAM_SHIFT    = 4,
} dwc_dgcmdpar_data_t;

typedef enum dwc_dgcmd_data {
    USB_DWC_DGCMD_XMIT_FUNC_WAKE_DEV_NOTIF        = 3,
    USB_DWC_DGCMD_SET_SCRATCHPAD_ARRAY_ADR_LO        = 4,
    USB_DWC_DGCMD_XMIT_DEV_NOTIF            = 7,
    USB_DWC_DGCMD_SELECTED_FIFO_FLUSH            = 9,
    USB_DWC_DGCMD_ACT_BIT            = 0x400,
} dwc_dgcmd_data_t;

typedef volatile struct dwc_usb3_dev_global_regs {
#define USB_DWC_DEV_GLOBAL_REG_OFFSET    0x700
    volatile u32 dcfg;
    volatile u32 dctl;
    volatile u32 devten;
    volatile u32 dsts;
    volatile u32 dgcmdpar;
    volatile u32 dgcmd;
    volatile u32 reserved[2];
    volatile u32 dalepena;
} dwc_usb3_dev_global_regs_t;

typedef enum dwc_depcfgpar1_data {
    USB_DWC_EPCFG1_XFER_CMPL_BIT    = 0x00000100,
    USB_DWC_EPCFG1_XFER_IN_PROG_BIT    = 0x00000200,
    USB_DWC_EPCFG1_XFER_NRDY_BIT    = 0x00000400,
    USB_DWC_EPCFG1_EBC_MODE_BIT        = 0x00008000,
    USB_DWC_EPCFG1_BINTERVAL_SHIFT    = 16,
    USB_DWC_EPCFG1_STRM_CAP_BIT        = 0x01000000,
    USB_DWC_EPCFG1_EP_DIR_BIT        = 0x02000000,
    USB_DWC_EPCFG1_EP_NUM_SHIFT        = 26,
} dwc_depcfgpar1_data_t;

typedef enum dwc_depcfgpar0_data {
    USB_DWC_EPCFG0_EPTYPE_SHIFT            = 1,
    USB_DWC_USB3_EP_TYPE_CONTROL    = 0,
    USB_DWC_EPCFG0_MPS_SHIFT            = 3,
    USB_DWC_EPCFG0_TXFNUM_SHIFT            = 17,
    USB_DWC_EPCFG0_BRSTSIZ_SHIFT        = 22,
    USB_DWC_EPCFG0_CFG_ACTION_BITS        = 0xc0000000,
    USB_DWC_EPCFG0_CFG_ACTION_SHIFT        = 30,
    USB_DWC_CFG_ACTION_RESTORE        = 1,
    USB_DWC_CFG_ACTION_MODIFY        = 2,
} dwc_depcfgpar0_data_t;

typedef enum dwc_depcmd_data {
    USB_DWC_EPCMD_SET_EP_CFG    = 1,
    USB_DWC_EPCMD_SET_XFER_CFG    = 2,
    USB_DWC_EPCMD_GET_EP_STATE    = 3,
    USB_DWC_EPCMD_SET_STALL    = 4,
    USB_DWC_EPCMD_CLR_STALL    = 5,
    USB_DWC_EPCMD_START_XFER    = 6,
    USB_DWC_EPCMD_UPDATE_XFER    = 7,
    USB_DWC_EPCMD_END_XFER    = 8,
    USB_DWC_EPCMD_START_NEW_CFG    = 9,

    USB_DWC_EPCMD_ACT_BIT        = 0x400,
    USB_DWC_EPCMD_HP_FRM_BIT        = 0x800,
    USB_DWC_EPCMD_STR_NUM_OR_UF_SHIFT    = 16,

    USB_DWC_EPCMD_XFER_RSRC_IDX_BITS    = 0x007f0000,
    USB_DWC_EPCMD_XFER_RSRC_IDX_SHIFT    = 16,
} dwc_depcmd_data_t;

typedef volatile struct usb_dwc_dev_ep_regs {

#define USB_DWC_DEV_OUT_EP_REG_OFFSET    0x800
#define USB_DWC_DEV_IN_EP_REG_OFFSET    0x810
#define USB_DWC_EP_REG_OFFSET        0x20

    volatile u32 depcmdpar2;
    volatile u32 depcmdpar1;
    volatile u32 depcmdpar0;
    volatile u32 depcmd;
    volatile u32 reserved[4];
} usb_dwc_dev_ep_regs_t;

typedef enum dwc_desc_sts_data {
    USB_DWC_DSCSTS_XFRCNT_BITS    = 0x00ffffff,
    USB_DWC_DSCSTS_XFRCNT_SHIFT    = 0,

    USB_DWC_DSCSTS_PCM1_SHIFT    = 24,

    USB_DWC_DSCSTS_TRBRSP_BITS    = 0xf0000000,
    USB_DWC_DSCSTS_TRBRSP_SHIFT    = 28,

    USB_DWC_TRBRSP_MISSED_ISOC_IN    = 1,
    USB_DWC_TRBRSP_SETUP_PEND        = 2,
} dwc_desc_sts_data_t;

typedef enum dwc_desc_ctl_data {
    USB_DWC_DSCCTL_HWO_BIT        = 0x00000001,
    USB_DWC_DSCCTL_LST_BIT        = 0x00000002,
    USB_DWC_DSCCTL_CHN_BIT        = 0x00000004,
    USB_DWC_DSCCTL_CSP_BIT        = 0x00000008,
    USB_DWC_DSCCTL_TRBCTL_SHIFT        = 4,

    USB_DWC_DSCCTL_TRBCTL_NORMAL         = 1,
    USB_DWC_DSCCTL_TRBCTL_SETUP            = 2,
    USB_DWC_DSCCTL_TRBCTL_STATUS_2        = 3,
    USB_DWC_DSCCTL_TRBCTL_STATUS_3        = 4,
    USB_DWC_DSCCTL_TRBCTL_CTLDATA_1ST        = 5,
    USB_DWC_DSCCTL_TRBCTL_ISOC_1ST        = 6,
    USB_DWC_DSCCTL_TRBCTL_LINK            = 8,

    USB_DWC_DSCCTL_ISP_BIT        = 0x00000400,
#define USB_DWC_DSCCTL_IMI_BIT    USB_DWC_DSCCTL_ISP_BIT
    USB_DWC_DSCCTL_IOC_BIT        = 0x00000800,
    USB_DWC_DSCCTL_STRMID_SOFN_BITS    = 0x3fffc000,
    USB_DWC_DSCCTL_STRMID_SOFN_SHIFT    = 14,
} dec_desc_ctl_data_t;

typedef struct usb_dwc_dma_desc {
    u32    bptl;
    u32    bpth;
    u32    status;
    u32    control;
} usb_dwc_dma_desc_t;

#ifdef __cplusplus
}
#endif

#endif /* _DWC_USB3_REGS_H_ */
