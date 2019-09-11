#ifndef __USB_DEV3_CIL_H__
#define __USB_DEV3_CIL_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int dwc_usb3_pcd_check_snpsid(dwc_usb3_device_t *pdev, unsigned int addr_ofs);
extern int dwc_usb3_pcd_common_init(dwc_usb3_device_t *pdev,
                    volatile unsigned char *base, dwc_usb3_core_params_t *core_params);
extern void dwc_usb3_pcd_common_remove(dwc_usb3_device_t *pdev);
extern void dwc_usb3_pcd_device_init(dwc_usb3_device_t *pdev, int soft_reset, int restore);
extern void dwc_usb3_pcd_device_remove(dwc_usb3_device_t *pdev);
extern int dwc_usb3_pcd_get_link_state(usb_dwc_pcd_t *pcd_t);
extern void dwc_usb3_pcd_set_link_state(usb_dwc_pcd_t *pcd_t, int state);
extern void dwc_usb3_pcd_remote_wake(usb_dwc_pcd_t *pcd_t, int func);
extern void dwc_usb3_pcd_do_test_mode(unsigned long data);

extern void dwc_usb3_fill_desc(usb_dwc_dma_desc_t *pdesc, usb_dwc_dma_t addr,
                   unsigned int dma_len, unsigned int stream, unsigned int type,
                   unsigned int ctrlbits, int own);
extern void dwc_usb3_start_desc_chain(usb_dwc_dma_desc_t *pdesc);
extern void dwc_usb3_end_desc_chain(usb_dwc_dma_desc_t *pdesc);
extern void dwc_usb3_enable_desc(usb_dwc_dma_desc_t *pdesc);
extern void dwc_usb3_disable_desc(usb_dwc_dma_desc_t *pdesc);
extern int dwc_usb3_xmit_fn_remote_wake(usb_dwc_pcd_t *pcd, unsigned int intf);
extern int dwc_usb3_xmit_ltm(usb_dwc_pcd_t *pcd, unsigned int val);
extern int dwc_usb3_xmit_host_role_request(usb_dwc_pcd_t *pcd, unsigned int param);
extern int dwc_usb3_set_scratchpad_buf_array(usb_dwc_pcd_t *pcd, usb_dwc_dma_t addr);
extern int dwc_usb3_flush_fifo(usb_dwc_pcd_t *pcd, unsigned int fifo_sel);
extern int dwc_usb3_dep_cfg(usb_dwc_pcd_t *pcd, usb_dwc_dev_ep_regs_t *ep_reg,
                unsigned int dep_cfg0, unsigned int dep_cfg1, unsigned int dep_cfg2);
extern int dwc_usb3_dep_xfercfg(usb_dwc_pcd_t *pcd, usb_dwc_dev_ep_regs_t *ep_reg,
                unsigned int dep_strmcfg);
extern unsigned int dwc_usb3_dep_getepstate(usb_dwc_pcd_t *pcd, usb_dwc_dev_ep_regs_t *ep_reg);
extern int dwc_usb3_dep_sstall(usb_dwc_pcd_t *pcd, usb_dwc_dev_ep_regs_t *ep_reg);
extern int dwc_usb3_dep_cstall(usb_dwc_pcd_t *pcd, usb_dwc_dev_ep_regs_t *ep_reg,
                   int clr_pend);
extern int dwc_usb3_dep_startxfer(usb_dwc_pcd_t *pcd, usb_dwc_dev_ep_regs_t *ep_reg,
                  usb_dwc_dma_t addr, unsigned int stream_or_uf);
extern int dwc_usb3_dep_updatexfer(usb_dwc_pcd_t *pcd, usb_dwc_dev_ep_regs_t *ep_reg,
                   unsigned int tri);
extern int dwc_usb3_dep_endxfer(usb_dwc_pcd_t *pcd, usb_dwc_dev_ep_regs_t *ep_reg,
                unsigned int tri, int flags, void *condition);
#define DWC_ENDXFER_FORCE    1
#define DWC_ENDXFER_NODELAY    2

extern int dwc_usb3_dep_startnewcfg(usb_dwc_pcd_t *pcd, usb_dwc_dev_ep_regs_t *ep_reg,
                    unsigned int rsrcidx);
extern int dwc_usb3_enable_ep(usb_dwc_pcd_t *pcd_t, usb_dwc_pcd_ep_t *ep);
extern int dwc_usb3_disable_ep(usb_dwc_pcd_t *pcd_t, usb_dwc_pcd_ep_t *ep);
extern int dwc_usb3_get_device_speed(usb_dwc_pcd_t *pcd_t);
extern int dwc_usb3_get_frame(usb_dwc_pcd_t *pcd_t);
extern void dwc_usb3_set_address(usb_dwc_pcd_t *pcd_t, int addr);
extern void dwc_usb3_ena_usb2_suspend(usb_dwc_pcd_t *pcd_t);
extern void dwc_usb3_dis_usb2_suspend(usb_dwc_pcd_t *pcd_t);
extern void dwc_usb3_accept_u1(usb_dwc_pcd_t *pcd_t);
extern void dwc_usb3_accept_u2(usb_dwc_pcd_t *pcd_t);
extern void dwc_usb3_enable_u1(usb_dwc_pcd_t *pcd_t);
extern void dwc_usb3_enable_u2(usb_dwc_pcd_t *pcd_t);
extern void dwc_usb3_disable_u1(usb_dwc_pcd_t *pcd_t);
extern void dwc_usb3_disable_u2(usb_dwc_pcd_t *pcd_t);
extern int dwc_usb3_u1_enabled(usb_dwc_pcd_t *pcd_t);
extern int dwc_usb3_u2_enabled(usb_dwc_pcd_t *pcd_t);
extern void dwc_usb3_clr_eps_enabled(usb_dwc_pcd_t *pcd_t);

#define dwc_usb3_is_hwo(pdesc)    ((pdesc)->control & USB_DWC_DSCCTL_HWO_BIT)
#define dwc_usb3_is_ioc(pdesc)    ((pdesc)->control & USB_DWC_DSCCTL_IOC_BIT)

#define dwc_usb3_get_xfercnt(pdesc)                \
    ((pdesc)->status >> USB_DWC_DSCSTS_XFRCNT_SHIFT &        \
     USB_DWC_DSCSTS_XFRCNT_BITS >> USB_DWC_DSCSTS_XFRCNT_SHIFT)

#define dwc_usb3_get_xfersts(pdesc)                \
    ((pdesc)->status >> USB_DWC_DSCSTS_TRBRSP_SHIFT &        \
     USB_DWC_DSCSTS_TRBRSP_BITS >> USB_DWC_DSCSTS_TRBRSP_SHIFT)

#define dwc_usb3_get_xfersofn(pdesc)                \
    ((pdesc)->control >> DWC_DSCCTL_STRMID_SOFN_SHIFT &    \
     USB_DWC_DSCCTL_STRMID_SOFN_BITS >> USB_DWC_DSCCTL_STRMID_SOFN_SHIFT)

#define dwc_usb3_get_eventsofn(event)                \
    ((event) >> USB_DWC_DEPEVT_ISOC_UFRAME_NUM_SHIFT &    \
     USB_DWC_DEPEVT_ISOC_UFRAME_NUM_BITS >> USB_DWC_DEPEVT_ISOC_UFRAME_NUM_SHIFT)

extern void dwc_usb3_set_tx_fifo_size(dwc_usb3_device_t *pdev, int *sz);
extern void dwc_usb3_set_rx_fifo_size(dwc_usb3_device_t *pdev, int size);
extern void dwc_usb3_init_eventbuf(dwc_usb3_device_t *pdev, int bufno,
                   unsigned int *addr, int sz, usb_dwc_dma_t dma_addr);
extern void dwc_usb3_dis_flush_eventbuf_intr(dwc_usb3_device_t *pdev, int bufno);
extern void dwc_usb3_enable_common_interrupts(dwc_usb3_device_t *pdev);
extern void dwc_usb3_enable_device_interrupts(dwc_usb3_device_t *pdev);
extern int dwc_usb3_handle_event(dwc_usb3_device_t *pdev);
extern int dwc_usb3_irq(dwc_usb3_device_t *pdev, int irq);

static __inline unsigned int dwc_usb3_mode(dwc_usb3_device_t *pdev)
{
    return dwc_rd32(pdev, &pdev->core_global_regs->gsts) & 0x1;
}

static __inline int dwc_usb3_is_device_mode(dwc_usb3_device_t *pdev)
{
    return dwc_usb3_mode(pdev) != USB_DWC_GSTS_HOST_MODE;
}

static __inline int dwc_usb3_is_host_mode(dwc_usb3_device_t *pdev)
{
    return dwc_usb3_mode(pdev) == USB_DWC_GSTS_HOST_MODE;
}

#ifdef __cplusplus
}
#endif

#endif /* _CIL_H_ */
