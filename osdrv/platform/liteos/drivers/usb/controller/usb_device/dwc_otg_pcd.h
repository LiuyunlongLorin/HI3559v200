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
#ifndef __DWC_OTG_PCD_H__
#define __DWC_OTG_PCD_H__

#include <linux/spinlock.h>
#include <controller/usb_device/usb_gadget.h>

#define DWC_REG_BASE  CONFIG_HIUSBUDC_REG_BASE_ADDRESS

#define PCD_MAX_EPS_NUM  31 /* 1(ep0) + 15(ep-in) + 15(ep-out)*/

typedef struct dwc_otg_pcd_ep {
    UINT8  num;
    UINT32 dma_addr;
    UINT8 *start_xfer_buff;
    UINT8 *xfer_buff;

    unsigned is_in : 1;
    unsigned is_active : 1;
    unsigned is_stopped : 1;
    unsigned is_disabling : 1;
    unsigned tx_fifo_num : 4;

    unsigned xfer_len : 19;
    unsigned xfer_count : 19;
    unsigned xfer_type : 2;
#define USB_EP_TYPE_CONTROL    0
#define USB_EP_TYPE_ISOC       1
#define USB_EP_TYPE_BULK       2
#define USB_EP_TYPE_INTR       3

    unsigned data_pid_start : 1;
    unsigned even_odd_frame : 1;
    unsigned maxpacket : 11;
    unsigned sent_zlp : 1;
    unsigned total_len : 19;
    void* private_data;

    /** bInterval */
    uint32_t bInterval;
    /** Next frame num to setup next ISOC transfer */
    uint32_t frame_num;
    /** Indicates SOF number overrun in DSTS */
    uint8_t frm_overrun;
} dwc_otg_pcd_ep_t;

typedef struct dwc_otg_pcd_s{
    void *setup_pkt;
     struct {
         UINT8     type;
         UINT8     setup_complete;
         UINT8     out_complete;
         UINT8     in_complete;
     }cmdtype;

    int ep0state;
    unsigned status;
    volatile char *buf;
    int length;

    int xfer_len;

    dwc_otg_pcd_ep_t pcd_eps[PCD_MAX_EPS_NUM];
    struct usbd_endpoint usbd_eps[PCD_MAX_EPS_NUM];
    struct usbd_gadget_device gadget;   //instance for xxfunction access
    struct usbd_gadget_driver *driver;  //pointer to xxfunction drv

    uint8_t pcd_num_eps;
    uint8_t num_in_eps;
    uint8_t num_out_eps;

    short isPktResended;
    unsigned origiSum;
    unsigned sequenceNo;
    int      xferNeedReply;

    unsigned request_config : 1;
    unsigned request_enable : 1;
    unsigned request_reserv : 30;

    spinlock_t    irq_lock;
    spinlock_t     lock;

    void* context;
    /** Frame number while entering to ISR - needed for ISOCs **/
    uint32_t frame_num;
}dwc_otg_pcd_t;

/***************************************************************
Control and Status Register (GOTGCTL)
The OTG Control and Status register controls the behavior and
reflects the status of the OTG function of the core.
@@ Size: 32 bits
@@ Offset: 0x0
@@ Memory Access: R/W
****************************************************************/
typedef union gotgctl_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned sesreqscs_R : 1;
        unsigned sesreq_RW : 1;
        unsigned reserved2_7 : 6;
        unsigned hstnegscs_R : 1;
        unsigned hnpreq_RW : 1;
        unsigned hstsethnpen_RW : 1;
        unsigned devhnpen_RW : 1;
        unsigned reserved12_15 : 4;
        unsigned conidsts_R : 1;
        unsigned reserved17 : 1;
        unsigned asesvld_R : 1;
        unsigned bsesvld_R : 1;
        unsigned currmod_RW : 1;
        unsigned reserved21_31 : 11;
    } b;
} gotgctl_data_t;

/***************************************************************
Interrupt Register (GOTGINT)
The application reads this register whenever there is an OTG
interrupt and clears the bits in this register to clear the
OTG interrupt.
@@ Size: 32 bits
@@ Offset: 0x4
@@ Memory Access: R/W
****************************************************************/
typedef union gotgint_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
    unsigned reserved0_1 : 2;
    unsigned sesenddet_R_SS_WC : 1;
    unsigned reserved3_7 : 5;
    unsigned sesreqsucstschng_R_SS_WC : 1;
    unsigned hstnegsucstschng_R_SS_WC : 1;
    unsigned reserver10_16 : 7;
    unsigned hstnegdet_R_SS_WC : 1;
    unsigned adevtoutchng_R_SS_WC : 1;
    unsigned debdone_R_SS_WC : 1;
    unsigned reserved31_20 : 12;

    } b;
} gotgint_data_t;


/***************************************************************
AHB Configuration Register (GAHBCFG)
This register can be used to configure the core after power-on
or a change in mode. This register mainly contains AHB
system-related configuration parameters. Do not change this
register after the initial programming. The application must
program this register before starting any transactions on either
the AHB or the USB.
@@ Size: 32 bits
@@ Offset: 0x8
@@ Memory Access: R/W
****************************************************************/
typedef union gahbcfg_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned glblintrmsk_RW : 1;
#define GAHBCFG_GLBINT_ENABLE        1

        unsigned hburstlen_RW : 4;
#define GAHBCFG_DMA_BURST_SINGLE    0
#define GAHBCFG_DMA_BURST_INCR      1
#define GAHBCFG_DMA_BURST_INCR4     3
#define GAHBCFG_DMA_BURST_INCR8     5
#define GAHBCFG_DMA_BURST_INCR16    7

        unsigned dma_enable_RW : 1;
#define GAHBCFG_DMA_ENABLE            1

        unsigned reserved : 1;
        unsigned nptxfemplvl_txfemplvl_RW : 1;

        unsigned ptxfemplvl_RW : 1;
#define GAHBCFG_TXFEMPTYLVL_EMPTY        1
#define GAHBCFG_TXFEMPTYLVL_HALF_EMPTY    0

        unsigned reserved9_31 : 23;
    }b;
} gahbcfg_data_t;

/***************************************************************
USB Configuration Register (GUSBCFG)
This register can be used to configure the core after power-on
or a changing to Host mode or Device mode. It contains USB and
USB-PHY related configuration parameters. The application must
program this register before starting any transactions on
either the AHB or the USB. Do not make changes to this register
after the initial programming.
@@ Size: 32 bits
@@ Offset: 0xc
@@ Memory Access: R/W
****************************************************************/
typedef union gusbcfg_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned toutcal_RW : 3;
        unsigned phyif_RW : 1;
        unsigned ulpi_utmi_sel_RW : 1;
        unsigned fsintf_RW : 1;
        unsigned physel_RW : 1;
        unsigned ddrsel_RW : 1;
        unsigned srpcap_RW : 1;
        unsigned hnpcap_RW : 1;
        unsigned usbtrdtim_RW : 4;
        unsigned nptxfrwnden_RW : 1;
        unsigned phylpwrclkselect_RW : 1;
        unsigned otgutmifsselect_RW : 1;
        unsigned ulpi_fsls_RW : 1;
        unsigned ulpi_auto_res_RW : 1;
        unsigned ulpi_clk_sus_m_RW : 1;
        unsigned ulpi_ext_vbus_drv_RW : 1;
        unsigned ulpi_int_vbus_indicator_RW : 1;
        unsigned term_sel_dl_pulse_RW : 1;
        unsigned reserved : 9;
    } b;
} gusbcfg_data_t;

/***************************************************************
Reset Register (GRSTCTL)
The application uses this register to reset various hardware
features inside the core.
@@ Size: 32 bits
@@ Offset: 0x10
@@ Memory Access: R/W
****************************************************************/
typedef union grstctl_data
{
    /** raw word */
    unsigned int d32;
    /** raw bits */
    struct
    {
        unsigned csftrst_RW : 1;
        unsigned hsftrst_R : 1;
        unsigned hstfrm_R : 1;
        unsigned intknqflsh_R_WS_SC : 1;
        unsigned rxfflsh_RW : 1;
        unsigned txfflsh_R_WS_SC : 1;
        unsigned txfnum_RW : 5;
        unsigned reserved11_29 : 19;
        unsigned dmareq_R : 1;
        unsigned ahbidle_R : 1;
    } b;
} grstctl_t;


/***************************************************************
Interrupt Mask Register (GINTMSK)
This register works with the ¡°Interrupt Register (GINTSTS)¡± to
interrupt the application. When an interrupt bit is masked, the
interrupt associated with that bit is not generated. However,
the GINTSTS register bit corresponding to that interrupt is
still set.
@@ Size: 32 bits
@@ Offset: 0x18
@@ Memory Access: R/
****************************************************************/
typedef union gintmsk_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned reserved0 : 1;
        unsigned modemismatch_RW : 1;
        unsigned otgintr_RW : 1;
        unsigned sofintr_RW : 1;
        unsigned rxstsqlvl_RW : 1;
        unsigned nptxfempty_RW : 1;
        unsigned ginnakeff_RW : 1;
        unsigned goutnakeff_RW : 1;
        unsigned reserved8 : 1;
        unsigned i2cintr_RW : 1;
        unsigned erlysuspend_RW : 1;
        unsigned usbsuspend_RW : 1;
        unsigned usbreset_RW : 1;
        unsigned enumdone_RW : 1;
        unsigned isooutdrop_RW : 1;
        unsigned eopframe_RW : 1;
        unsigned reserved16 : 1;
        unsigned epmismatch_RW : 1;
        unsigned inepintr_RW : 1;
        unsigned outepintr_RW : 1;
        unsigned incomplisoin_RW : 1;
        unsigned incomplisoout_RW : 1;
        unsigned reserved22_23 : 2;
        unsigned portintr_RW : 1;
        unsigned hcintr_RW : 1;
        unsigned ptxfempty_RW : 1;
        unsigned reserved27 : 1;
        unsigned conidstschng_RW : 1;
        unsigned disconnect_RW : 1;
        unsigned sessreqintr_RW : 1;
        unsigned wkupintr_RW : 1;
    } b;
} gintmsk_data_t;

/***************************************************************
Interrupt Register (GINTSTS)
This register interrupts the application for system-level events
in the current mode (Device mode or Host mode).
@@ Size: 32 bits
@@ Offset: 0x14
@@ Memory Access: R/W
****************************************************************/
typedef union gintsts_data
{
    /** raw word */
    UINT32 d32;
    #define SOF_INTR_MASK 0x0008

    /** raw bits */
    struct
    {
#define DWC_OTG_HOST_MODE 1
        unsigned curmode_R : 1;
        unsigned modemismatch_R_SS_WC : 1;
        unsigned otgintr_R : 1;
        unsigned sofintr_R_SS_WC : 1;
        unsigned rxstsqlvl_R : 1;
        unsigned nptxfempty_R : 1;
        unsigned ginnakeff_R : 1;
        unsigned goutnakeff_R : 1;
        unsigned lpickint_R_SS_WC : 1;
        unsigned i2cintr_R_SS_WC : 1;
        unsigned erlysuspend_R_SS_WC : 1;
        unsigned usbsuspend_R_SS_WC : 1;
        unsigned usbreset_R_SS_WC : 1;
        unsigned enumdone_R_SS_WC : 1;
        unsigned isooutdrop_R_SS_WC : 1;
        unsigned eopframe_R_SS_WC : 1;
        unsigned intokenrx_R_SS_WC : 1;
        unsigned epmismatch_R : 1;
        unsigned inepint_R: 1;
        unsigned outepintr_R : 1;
        unsigned incomplisoin_R_SS_WC : 1;
        unsigned incomplisoout_R_SS_WC : 1;
        unsigned fetsusp_R_SS_WC : 1;
        unsigned resetdet_R_SS_WC : 1;
        unsigned portintr_R : 1;
        unsigned hcintr_R : 1;
        unsigned ptxfempty_R : 1;
        unsigned lpm_int_R_SS_WC : 1;
        unsigned conidstschngR_SS_WC : 1;
        unsigned disconnectR_SS_WC : 1;
        unsigned sessreqintrR_SS_WC : 1;
        unsigned wkupintrR_SS_WC : 1;
    } b;
} gintsts_data_t;

/***************************************************************
Non-Periodic Transmit FIFO Size Register (GNPTXFSIZ)
The application can program the RAM size and the memory start
address for the Non-periodic TxFIFO.
@@ Size: 32 bits
@@ Offset: 0x28
@@ Memory Access: R/W
****************************************************************/
typedef union fifosize_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned startaddr_RW : 16;
        unsigned depth_RW : 16;
    } b;
} fifosize_data_t;

/***************************************************************
User HW Config1 Register (GHWCFG1)
This register contains the logical endpoint direction(s) selected
using coreConsultant.
@@ Size: 32 bits
@@ Offset: 0x44
@@ Memory Access: R
@@ Value After Reset: Configurable
****************************************************************/
typedef union hwcfg1_data {
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct {
        unsigned ep_dir0_R:2;
        #define EP_DIR_BIDIR    0x00
        #define EP_DIR_IN       0x01
        #define EP_DIR_OUT      0x02

        unsigned ep_dir1_R:2;
        unsigned ep_dir2_R:2;
        unsigned ep_dir3_R:2;
        unsigned ep_dir4_R:2;
        unsigned ep_dir5_R:2;
        unsigned ep_dir6_R:2;
        unsigned ep_dir7_R:2;
        unsigned ep_dir8_R:2;
        unsigned ep_dir9_R:2;
        unsigned ep_dir10_R:2;
        unsigned ep_dir11_R:2;
        unsigned ep_dir12_R:2;
        unsigned ep_dir13_R:2;
        unsigned ep_dir14_R:2;
        unsigned ep_dir15_R:2;
    } b;
} hwcfg1_data_t;


/***************************************************************
User HW Config2 Register (GHWCFG2)
This register contains configuration options selected using
coreConsultant.
@@ Size: 32 bits
@@ Offset: 0x48
@@ Memory Access: R
@@ Value After Reset: Configurable
****************************************************************/
typedef union hwcfg2_data {
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct {
        unsigned op_mode_R:3;
        #define HWCFG2_OPMODE_HNP_SRP_CAPABLE_OTG 0
        #define HWCFG2_OPMODE_SRP_ONLY_CAPABLE_OTG 1
        #define HWCFG2_OPMODE_NO_HNP_SRP_CAPABLE_OTG 2
        #define HWCFG2_OPMODE_SRP_CAPABLE_DEVICE 3
        #define HWCFG2_OPMODE_NO_SRP_CAPABLE_DEVICE 4
        #define HWCFG2_OPMODE_SRP_CAPABLE_HOST 5
        #define HWCFG2_OPMODE_NO_SRP_CAPABLE_HOST 6

        unsigned architecture_R:2;
        unsigned point2point_R:1;

        unsigned hs_phy_type_R:2;
        #define HWCFG2_HS_PHY_TYPE_NOT_SUPPORTED 0
        #define HWCFG2_HS_PHY_TYPE_UTMI 1
        #define HWCFG2_HS_PHY_TYPE_ULPI 2
        #define HWCFG2_HS_PHY_TYPE_UTMI_ULPI 3

        unsigned fs_phy_type_R:2;
        unsigned num_dev_ep_R:4;
        unsigned num_host_chan_R:4;
        unsigned perio_ep_supported_R:1;
        unsigned dynamic_fifo_R:1;
        unsigned multi_proc_int_R:1;
        unsigned reserved21:1;
        unsigned nonperio_tx_q_depth_R:2;
        unsigned host_perio_tx_q_depth_R:2;
        unsigned dev_token_q_depth_R:5;
        unsigned otg_enable_ic_usb_R:1;
    } b;
} hwcfg2_data_t;

/***************************************************************
Device Configuration Register (DCFG)
This register configures the core in Device mode after power-on
or after certain control commands or enumeration. Do not make
changes to this register after initial programming.
@@ Size: 32 bits
@@ Offset: 0x800
@@ Memory Access: R/W
****************************************************************/
typedef union dcfg_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned devspd_RW : 2;

        unsigned nzstsouthshk_RW : 1;
#define DCFG_SEND_STALL 1

        unsigned ena32khzs_RW : 1;
        unsigned devaddr_RW : 7;

        unsigned perfrint_RW : 2;
#define DCFG_FRAME_INTERVAL_80 0
#define DCFG_FRAME_INTERVAL_85 1
#define DCFG_FRAME_INTERVAL_90 2
#define DCFG_FRAME_INTERVAL_95 3

        unsigned endevoutnak_RW : 1;
        unsigned reserved14_17 : 4;
        unsigned epmscnt_RW : 5;
        unsigned descdma_RW : 1;
        unsigned perschintvl_RW : 2;
        unsigned resvalid_RW : 6;
    } b;
} dcfg_data_t;

/***************************************************************
Device Control Register (DCTL)
@@ Size: 32 bits
@@ Offset: 0x804
@@ Memory Access: R/W
****************************************************************/
typedef union dctl_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned rmtwkupsig_RW : 1;
        unsigned sftdiscon_RW : 1;
        unsigned gnpinnaksts_R : 1;
        unsigned goutnaksts_R : 1;
        unsigned tstctl_RW : 3;
        unsigned sgnpinnak_W : 1;
        unsigned cgnpinnak_W : 1;
        unsigned sgoutnak_W : 1;
        unsigned cgoutnak_W : 1;
        unsigned pwronprgdone_RW : 1;

        /** Reserved */
        unsigned reserved:1;
        /** Global Multi Count */
        unsigned gmc:2;
        /** Ignore Frame Number for ISOC EPs */
        unsigned ifrmnum:1;
        /** NAK on Babble */
        unsigned nakonbble:1;
        /** Enable Continue on BNA */
        unsigned encontonbna:1;
        /** Enable deep sleep besl reject feature*/
        unsigned besl_reject:1;
        unsigned reserved17_31:13;

    } b;
} dctl_data_t;

/***************************************************************
Device Status Register (DSTS)
This register indicates the status of the core with respect to
USB-related events. It must be read on interrupts from Device
All Interrupts (DAINT) register.
@@ Size: 32 bits
@@ Offset: 0x808
@@ Memory Access: R
****************************************************************/
typedef union dsts_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned suspsts_R : 1;

        unsigned enumspd_R : 2;
#define DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ 0
#define DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ 1
#define DSTS_ENUMSPD_LS_PHY_6MHZ           2
#define DSTS_ENUMSPD_FS_PHY_48MHZ          3

        unsigned errticerr_R : 1;
        unsigned reserved4_7: 4;
        unsigned soffn_R : 14;
        unsigned reserved22_31 : 10;
    } b;
} dsts_data_t;


/***************************************************************
Device Endpoint-n Interrupt Register (DIEPINTn/DOEPINTn)
@@ Endpoint_number: 0 ¡Ü  n ¡Ü  15
@@ Offset for IN endpoints: 908h + (Endpoint_number * 20h)
@@ Offset for OUT endpoints: B08h + (Endpoint_number * 20h)
****************************************************************/
typedef union diepint_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned xfercompl_R : 1;
        unsigned epdisabled_R_SS_WC : 1;
        unsigned ahberr_R_SS_WC : 1;
        unsigned timeout_R_SS_WC : 1;
        unsigned intktxfemp_R_SS_WC : 1;
        unsigned intknepmis_R_SS_WC : 1;
        unsigned inepnakeff_R_SS_WC : 1;
        unsigned emptyintr_R : 1;
        unsigned txfifoundrn_R_SS_WC : 1;
        unsigned bnaintr_R_SS_WC : 1;

        unsigned reserved10_12:3;
        /** BNA Interrupt mask */
        unsigned nak_R_SS_WC:1;

        unsigned reserved14_31 : 18;
	} b;
} diepint_data_t;
typedef union diepint_data diepmsk_data_t;


/***************************************************************
Device Endpoint-n Interrupt Register (DIEPINTn/DOEPINTn)
@@ Endpoint_number: 0 ¡Ü  n ¡Ü  15
@@ Offset for IN endpoints: 908h + (Endpoint_number * 20h)
@@ Offset for OUT endpoints: B08h + (Endpoint_number * 20h)
****************************************************************/
typedef union doepint_data
{
    /** raw word */
    volatile UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned xfercompl_R : 1;
        unsigned epdisabled_R_SS_WC : 1;
        unsigned ahberr_R_SS_WC : 1;
        unsigned setup_R_SS_WC : 1;
        unsigned outtknepdis_R_SS_WC : 1;
        unsigned stsphsercvd_R_SS_WC : 1;
        unsigned back2backsetup_R_SS_WC : 1;
        unsigned reserved7 : 1;
        unsigned outpkterr_R_SS_WC : 1;
        unsigned bna_R_SS_WC : 1;
        unsigned reserved10 : 1;
        unsigned pktdrpsts_R_SS_WC : 1;
        unsigned babble_R_SS_WC : 1;
        unsigned nak_R_SS_WC : 1;
        unsigned nyet_R_SS_WC : 1;
        unsigned sr_R_SS_WC : 1;
        unsigned reserved16_31 : 16;
    } b;
} doepint_data_t;

typedef union doepint_data doepmsk_data_t;


/***************************************************************
Device All Endpoints Interrupt Register (DAINT)
When a significant event occurs on an endpoint, a Device All
Endpoints Interrupt register interrupts the application using
the Device OUT Endpoints Interrupt bit or Device IN Endpoints
Interrupt bit of the Core Interrupt register (GINTSTS.OEPInt or
GINTSTS.IEPInt, respectively).
@@ Size: 32 bits
@@ Offset: 0x818
@@ Memory Access: R

Device All Endpoints Interrupt Mask Register (DAINTMSK)
The Device Endpoint Interrupt Mask register works with the Device
Endpoint Interrupt register to interrupt the application when an
event occurs on a device endpoint. However, the Device All
Endpoints Interrupt (DAINT) register bit corresponding to that
interrupt is still set.
@@ Size: 32 bits
@@ Offset: 0x81c
@@ Memory Access: R/W
****************************************************************/
typedef union daint_data
{
    /** raw word */
    UINT32 d32;

    struct
    {
        unsigned in : 16;
        unsigned out : 16;
    } ep;

    /** raw bits */
    struct
    {
        /** IN Endpoint bits */
        unsigned iep0  : 1;
        unsigned iep1  : 1;
        unsigned iep2  : 1;
        unsigned iep3  : 1;
        unsigned iep4  : 1;
        unsigned iep5  : 1;
        unsigned iep6  : 1;
        unsigned iep7  : 1;
        unsigned iep8  : 1;
        unsigned iep9  : 1;
        unsigned iep10 : 1;
        unsigned iep11 : 1;
        unsigned iep12 : 1;
        unsigned iep13 : 1;
        unsigned iep14 : 1;
        unsigned iep15 : 1;

        /** OUT Endpoint bits */
        unsigned oep0  : 1;
        unsigned oep1  : 1;
        unsigned oep2  : 1;
        unsigned oep3  : 1;
        unsigned oep4  : 1;
        unsigned oep5  : 1;
        unsigned oep6  : 1;
        unsigned oep7  : 1;
        unsigned oep8  : 1;
        unsigned oep9  : 1;
        unsigned oep10 : 1;
        unsigned oep11 : 1;
        unsigned oep12 : 1;
        unsigned oep13 : 1;
        unsigned oep14 : 1;
        unsigned oep15 : 1;
    } b;
} daint_data_t;

/***************************************************************
Device Endpoint-n Control Register (DIEPCTLn/DOEPCTLn)
The application uses this register to control the behavior of
each logical endpoint other than endpoint 0.
@@ Endpoint_number: 1 ¡Ü  n ¡Ü  15
@@ Offset for IN endpoints: 900h + (Endpoint_number * 20h)
@@ Offset for OUT endpoints: B00h + (Endpoint_number * 20h)
****************************************************************/
typedef union depctl_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct
    {
        unsigned mps_RW : 11;
#define DEP0CTL_MPS_64    0
#define DEP0CTL_MPS_32    1
#define DEP0CTL_MPS_16    2
#define DEP0CTL_MPS_8     3

        unsigned nextep_RW : 4;
        unsigned usbactep_R_W_SC : 1;
        unsigned dpid_R : 1;
        unsigned naksts_R : 1;
        unsigned eptype_RW : 2;
        unsigned snp_RW : 1;
        unsigned stall_RW : 1;
        unsigned txfnum_RW : 4;
        unsigned cnak_W : 1;
        unsigned snak_W : 1;
        unsigned setd0pid_W : 1;
        unsigned setd1pid_W : 1;
        unsigned epdis_R_WS_SC : 1;
        unsigned epena_R_WS_SC : 1;
    } b;
} depctl_data_t;

/***************************************************************
Device Endpoint-n Transfer Size Register (DIEPTSIZn/DOEPTSIZn)
The application must modify this register before enabling the
endpoint. Once the endpoint is enabled using Endpoint Enable bit
of the Device Endpoint-n Control registers
(DIEPCTLn.EPEna/DOEPCTLn.EPEna), the core modifies this register.
The application can only read this register once the core has
cleared the Endpoint Enable bit.
@@ Size: 32 bits
@@ Offset: 0x930
@@ Memory Access: R/W
@@ Endpoint_number: 1 ¡Ü  n ¡Ü  15
@@ Offset for IN endpoints: 910h + (Endpoint_number * 20h)
@@ Offset for OUT endpoints: B10h + (Endpoint_number * 20h)
****************************************************************/
typedef union deptsiz_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct {
        unsigned xfersize_RW : 19;
        #define DEPSZ_MAX_PKT_CNT 1023

        unsigned pktcnt_RW : 10;
        unsigned mc_RW : 2;
        unsigned reserved : 1;
    } b;
} deptsiz_data_t;

/***************************************************************
Device Endpoint 0 Transfer Size Register (DIEPTSIZ0/DOEPTSIZ0)
The application must modify this register before enabling
endpoint 0. Once endpoint 0 is enabled using Endpoint Enable
bit of the Device Control Endpoint 0 Control registers
(DIEPCTL0.EPEna/DOEPCTL0.EPEna), the core modifies this
register. The application can only read this register once the
core has cleared the Endpoint Enable bit.
@@ Size: 32 bits
@@ Offset: 0x910
@@ Memory Access: R/W
@@ Offset for IN endpoints: 910h
@@ Offset for OUT endpoints: B10h
****************************************************************/
typedef union deptsiz0_data
{
    /** raw word */
    UINT32 d32;
    /** raw bits */
    struct {
        unsigned xfersize_RW : 7;
        unsigned reserved7_18 : 12;
        unsigned pktcnt_RW : 1;
        unsigned reserved20_28 : 9;
        unsigned supcnt_RW : 2;
        unsigned reserved31;
    } b;
} deptsiz0_data_t;

extern uint32_t DWC_READ_REG32(uint32_t x) ;
extern void DWC_WRITE_REG32(uint32_t x, uint32_t v);
extern void DWC_MODIFY_REG32(uint32_t x, uint32_t c, uint32_t s);

int dwc_otg_initial(dwc_otg_pcd_t* pcd);
int dwc_otg_get_soffn(void);
void dwc_otg_core_reset(void);
void dwc_otg_enable_common_interrupts(void);
void dwc_otg_enable_device_interrupts(void);
void dwc_otg_enable_global_interrupts(void);

void dwc_otg_core_init(dwc_otg_pcd_t * pcd);
void dwc_otg_core_dev_init(dwc_otg_pcd_t * pcd);
int dwc_otg_endpoint_enable(struct usbd_endpoint* ep);
int dwc_otg_endpoint_disable(struct usbd_endpoint* ep);
int dwc_otg_ep_req_start(struct usbd_endpoint* usbd_ep,struct usbd_request* req);
int dwc_otg_ep_halt(struct usbd_endpoint* usbd_ep, int value);
void dwc_otg_core_do_dev_disconnect( dwc_otg_pcd_t *pcd);
void dwc_otg_core_do_pcd_setup(dwc_otg_pcd_t *pcd, struct usb_device_request * ctrl);
void dwc_otg_core_ep_complete(dwc_otg_pcd_t *pcd, dwc_otg_pcd_ep_t *dep);

void dwc_otg_flush_tx_fifo( const int _num ) ;
void dwc_otg_flush_rx_fifo(void) ;
void dwc_otg_ep_start_transfer(dwc_otg_pcd_ep_t *_ep);
void dwc_otg_endpoint_activate(dwc_otg_pcd_ep_t *ep);
void dwc_otg_power_off_phy(void);
void dwc_otg_pullup(int is_on);

#endif
