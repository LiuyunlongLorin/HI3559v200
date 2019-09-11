#ifndef __HIETH_CTRL_H
#define __HIETH_CTRL_H

#include "hieth.h"
#include "hisi_mac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef unsigned long dma_addr_t;

#define GLB_HOSTMAC_L32    0x1300
#define BITS_HOSTMAC_L32    MK_BITS(0, 32)
#define GLB_HOSTMAC_H16    0x1304
#define BITS_HOSTMAC_H16    MK_BITS(0, 16)

#define GLB_SOFT_RESET    0x1308
#define BITS_ETH_SOFT_RESET_ALL    MK_BITS(0, 1)
#define BITS_ETH_SOFT_RESET_UP    MK_BITS(2, 1)
#define BITS_ETH_SOFT_RESET_DOWN    MK_BITS(3, 1)

#define GLB_FWCTRL    0x1310
#define BITS_VLAN_ENABLE    MK_BITS(0, 1)
#define BITS_FW2CPU_ENA_U    MK_BITS(5, 1)
#define BITS_FW2CPU_ENA_UP    MK_BITS(5, 1)
#define BITS_FW2CPU_ENA_D    MK_BITS(9, 1)
#define BITS_FW2CPU_ENA_DOWN    MK_BITS(9, 1)
#define BITS_FWALL2CPU_U    MK_BITS(7, 1)
#define BITS_FWALL2CPU_UP    MK_BITS(7, 1)
#define BITS_FWALL2CPU_D    MK_BITS(11, 1)
#define BITS_FWALL2CPU_DOWN    MK_BITS(11, 1)
#define BITS_FW2OTHPORT_ENA_U    MK_BITS(4, 1)
#define BITS_FW2OTHPORT_ENA_D    MK_BITS(8, 1)
#define BITS_FW2OTHPORT_FORCE_U    MK_BITS(6, 1)
#define BITS_FW2OTHPORT_FORCE_D    MK_BITS(10, 1)

#define GLB_MACTCTRL    0x1314
#define BITS_MACT_ENA_U    MK_BITS(7, 1)
#define BITS_MACT_ENA_D    MK_BITS(15, 1)
#define BITS_BROAD2CPU_U    MK_BITS(5, 1)
#define BITS_BROAD2CPU_UP    MK_BITS(5, 1)
#define BITS_BROAD2CPU_D    MK_BITS(13, 1)
#define BITS_BROAD2CPU_DOWN    MK_BITS(13, 1)
#define BITS_BROAD2OTHPORT_U    MK_BITS(4, 1)
#define BITS_BROAD2OTHPORT_D    MK_BITS(12, 1)
#define BITS_MULTI2CPU_U    MK_BITS(3, 1)
#define BITS_MULTI2CPU_D    MK_BITS(11, 1)
#define BITS_MULTI2OTHPORT_U    MK_BITS(2, 1)
#define BITS_MULTI2OTHPORT_D    MK_BITS(10, 1)
#define BITS_UNI2CPU_U    MK_BITS(1, 1)
#define BITS_UNI2CPU_D    MK_BITS(9, 1)
#define BITS_UNI2OTHPORT_U    MK_BITS(0, 1)
#define BITS_UNI2OTHPORT_D    MK_BITS(8, 1)

/* ENDIAN */
#define GLB_ENDIAN_MOD    0x1318
#define BITS_ENDIAN    MK_BITS(0, 2)
#define HIETH_BIG_ENDIAN    0
#define HIETH_LITTLE_ENDIAN    3

/* IRQs */
#define GLB_RO_IRQ_STAT    0x1330
#define GLB_RW_IRQ_ENA    0x1334
#define GLB_RW_IRQ_RAW    0x1338

/* IRQs mask bits */
#define BITS_IRQS_U        MK_BITS(0, 8)
#define BITS_VLAN_IRQS        MK_BITS(11, 1)
#define BITS_MDIO_IRQS        MK_BITS(13, 2)
#define BITS_IRQS_ENA_D        MK_BITS(17, 1)
#define BITS_IRQS_ENA_U        MK_BITS(18, 1)
#define BITS_IRQS_ENA_ALLPORT    MK_BITS(19, 1)
#define BITS_IRQS_D        MK_BITS(20, 8)

#define BITS_IRQS_MASK_U    (0xFF)
#define BITS_IRQS_MASK_D    (0xFF<<20)

/* IRQs bit name */
#define HIETH_INT_RX_RDY_U    (1<<0)
#define HIETH_INT_RX_RDY_D    (1<<20)
#define HIETH_INT_TX_FIN_U    (1<<1)
#define HIETH_INT_TX_FIN_D    (1<<21)
#define HIETH_INT_LINK_CH_U    (1<<2)
#define HIETH_INT_LINK_CH_D    (1<<22)
#define HIETH_INT_SPEED_CH_U    (1<<3)
#define HIETH_INT_SPEED_CH_D    (1<<23)
#define HIETH_INT_DUPLEX_CH_U    (1<<4)
#define HIETH_INT_DUPLEX_CH_D    (1<<24)
#define HIETH_INT_STATE_CH_U    (1<<5)
#define HIETH_INT_STATE_CH_D    (1<<25)
#define HIETH_INT_TXQUE_RDY_U    (1<<6)
#define HIETH_INT_TXQUE_RDY_D    (1<<26)
#define HIETH_INT_MULTI_RXRDY_U    (1<<7)
#define HIETH_INT_MULTI_RXRDY_D    (1<<27)
#define HIETH_INT_TX_ERR_U    (1<<8)
#define HIETH_INT_TX_ERR_D    (1<<28)

#define HIETH_INT_MDIO_FINISH    (1<<12)
#define HIETH_INT_UNKNOW_VLANID    (1<<13)
#define HIETH_INT_UNKNOW_VLANM    (1<<14)

#define GLB_DN_HOSTMAC_L32    0x1340
#define GLB_DN_HOSTMAC_H16    0x1344
#define GLB_DN_HOSTMAC_ENA    0x1348
#define BITS_DN_HOST_ENA    MK_BITS(0, 1)

#define GLB_MAC_L32_BASE    (0x1400)
#define GLB_MAC_H16_BASE    (0x1404)
#define GLB_MAC_L32_BASE_D    (0x1400 + 16 * 0x8)
#define GLB_MAC_H16_BASE_D    (0x1404 + 16 * 0x8)
#define BITS_MACFLT_HI16    MK_BITS(0, 16)
#define BITS_MACFLT_FW2CPU_U   MK_BITS(21, 1)
#define BITS_MACFLT_FW2PORT_U   MK_BITS(20, 1)
#define BITS_MACFLT_ENA_U   MK_BITS(17, 1)
#define BITS_MACFLT_FW2CPU_D   MK_BITS(19, 1)
#define BITS_MACFLT_FW2PORT_D   MK_BITS(18, 1)
#define BITS_MACFLT_ENA_D   MK_BITS(16, 1)

/* Tx/Rx Queue depth */
#define U_GLB_QLEN_SET    0x0344
#define D_GLB_QLEN_SET    0x2344
#define BITS_TXQ_DEP    MK_BITS(0, 6)
#define BITS_RXQ_DEP    MK_BITS(8, 6)

#define U_GLB_FC_LEVEL        0x0348
#define D_GLB_FC_LEVEL        0x2348
#define BITS_FC_DEACTIVE_THR    MK_BITS(0, 6)
#define BITS_FC_ACTIVE_THR    MK_BITS(8, 6)
#define BITS_FC_EN        MK_BITS(14, 1)

#define BITS_PAUSE_EN           MK_BITS(18, 1)

/* Rx (read only) Queue-ID and LEN */
#define U_GLB_RO_IQFRM_DES    0x0354
#define D_GLB_RO_IQFRM_DES    0x2354

/*rx buffer addr.*/
#define U_GLB_RXFRM_SADDR 0x0350
#define D_GLB_RXFRM_SADDR 0x2350
/* bits of UD_GLB_RO_IQFRM_DES */
#define BITS_RXPKG_LEN        MK_BITS(0, 11)
#define BITS_RXPKG_ID        MK_BITS(12, 6)
#define BITS_FRM_VLAN_VID    MK_BITS(18, 1)
#define BITS_FD_VID_VID        MK_BITS(19, 1)
#define BITS_FD_VLANID        MK_BITS(20, 12)
#define BITS_RXPKG_LEN_OFFSET        0
#define BITS_RXPKG_LEN_MASK        0xFFF
#define BITS_PAYLOAD_ERR_OFFSET        20
#define BITS_PAYLOAD_ERR_MASK        0x1
#define BITS_HEADER_ERR_OFFSET        21
#define BITS_HEADER_ERR_MASK        0x1
#define BITS_PAYLOAD_DONE_OFFSET    22
#define BITS_PAYLOAD_DONE_MASK        0x1
#define BITS_HEADER_DONE_OFFSET        23
#define BITS_HEADER_DONE_MASK        0x1

/* Rx ADDR */
#define U_GLB_IQ_ADDR    0x0358
#define D_GLB_IQ_ADDR    0x2358

/* Tx ADDR and LEN */
#define U_GLB_EQ_ADDR    0x0360
#define D_GLB_EQ_ADDR    0x2360
#define U_GLB_EQFRM_LEN    0x0364
#define D_GLB_EQFRM_LEN    0x2364
/* bits of UD_GLB_EQFRM_LEN */
#ifdef HIETH_TSO_SUPPORTED
#define BITS_TXINQ_LEN    MK_BITS(0, 32)
#else
#define BITS_TXINQ_LEN    MK_BITS(0, 11)
#endif

#ifdef HIETH_TSO_SUPPORTED
/* TSO debug enable */
#define U_GLB_TSO_DBG_EN        0x03A4
#define D_GLB_TSO_DBG_EN        0x23A4
#define BITS_TSO_DBG_EN            MK_BITS(31, 1)
/* TSO debug state */
#define U_GLB_TSO_DBG_STATE        0x03A8
#define D_GLB_TSO_DBG_STATE        0x23A8
#define BITS_TSO_DBG_STATE        MK_BITS(31, 1)
/* TSO debug addr */
#define U_GLB_TSO_DBG_ADDR        0x03AC
#define D_GLB_TSO_DBG_ADDR        0x23AC
/* TSO debug tx info */
#define U_GLB_TSO_DBG_TX_INFO        0x03B0
#define D_GLB_TSO_DBG_TX_INFO        0x23B0
/* TSO debug tx err */
#define U_GLB_TSO_DBG_TX_ERR        0x03B4
#define D_GLB_TSO_DBG_TX_ERR        0x23B4
#endif

/* Rx/Tx Queue ID */
#define U_GLB_RO_QUEUE_ID    0x0368
#define D_GLB_RO_QUEUE_ID    0x2368
/* bits of UD_GLB_RO_QUEUE_ID */
#define BITS_TXOUTQ_ID    MK_BITS(0, 6)
#define BITS_TXINQ_ID    MK_BITS(8, 6)
#define BITS_RXINQ_ID    MK_BITS(16, 6)

/* Rx/Tx Queue staus  */
#define U_GLB_RO_QUEUE_STAT    0x036C
#define D_GLB_RO_QUEUE_STAT    0x236C
/* bits of UD_GLB_RO_QUEUE_STAT */
/* check this bit to see if we can add a Tx package */
#define BITS_XMITQ_RDY    MK_BITS(24, 1)
/* check this bit to see if we can add a Rx addr */
#define BITS_RECVQ_RDY    MK_BITS(25, 1)
/* counts in queue, include currently sending */
#define BITS_XMITQ_CNT_INUSE    MK_BITS(0, 6)
/* counts in queue, include currently receving */
#define BITS_RECVQ_CNT_RXOK    MK_BITS(8, 6)

#ifdef HIETH_TSO_SUPPORTED
#define E_MAC_TX_FAIL   2
#define E_MAC_SW_GSO    3
#endif

#define HIETH_CSUM_ENABLE    1
#define HIETH_CSUM_DISABLE   0
#if LWIP_TX_CSUM_OFFLOAD
#define HIETH_IPV4_VERSION_HW 0
#define HIETH_IPV6_VERSION_HW 1
#define HIETH_TRANS_TCP_TYPE_HW 0
#define HIETH_TRANS_UDP_TYPE_HW 1
#endif
#define FCS_BYTES    4

/* Rx COE control */
#define U_GLB_RX_COE_CTRL        0x0380
#define D_GLB_RX_COE_CTRL        0x2380
#define BITS_COE_IPV6_UDP_ZERO_DROP    MK_BITS(13, 1)
#define BITS_COE_PAYLOAD_DROP        MK_BITS(14, 1)
#define BITS_COE_IPHDR_DROP        MK_BITS(15, 1)

/*get pkgs received address*/
#define hw_get_rxpkg_addr(ld) hieth_readl(ld, UD_REG_NAME(GLB_RXFRM_SADDR))

#define is_recv_packet(ld) \
        (hieth_readl(ld, GLB_RW_IRQ_RAW) & \
                     (UD_BIT_NAME(HIETH_INT_RX_RDY)))
#define hw_set_rxpkg_finish(ld) \
         hieth_writel(ld, UD_BIT_NAME(HIETH_INT_RX_RDY), GLB_RW_IRQ_RAW)

#define hw_get_rxpkg_len(ld) \
        hieth_readl_bits(ld, UD_REG_NAME(GLB_RO_IQFRM_DES), \
                             BITS_RXPKG_LEN)

#define hw_get_rxpkg_info(ld) \
        hieth_readl(ld, UD_REG_NAME(GLB_RO_IQFRM_DES))

#define hw_xmitq_cnt_inuse(ld) \
        hieth_readl_bits(ld, UD_REG_NAME(GLB_RO_QUEUE_STAT), \
                            BITS_XMITQ_CNT_INUSE)
#define hw_recv_cnt_inuse(ld) \
        hieth_readl_bits(ld, UD_REG_NAME(GLB_RO_QUEUE_STAT), \
                            BITS_RECVQ_CNT_RXOK)

#define hw_xmitq_pkg(ld, addr, len) do { \
            hieth_writel(ld, (addr), UD_REG_NAME(GLB_EQ_ADDR)); \
            hieth_writel_bits(ld, (len), \
                UD_REG_NAME(GLB_EQFRM_LEN), BITS_TXINQ_LEN); \
        } while (0)

struct hieth_priv_s;
void hieth_hw_mac_core_reset(void);
void hieth_hw_external_phy_reset(void);
void hieth_hw_mac_core_init(struct hieth_netdev_local *ld);

int _test_xmit_queue_ready(struct hieth_netdev_local *ld);

/* for each bits, set '1' enable the intterrupt, and '0' takes no effects */
/* return last irq_enable status */
int hieth_irq_enable(struct hieth_netdev_local *ld, int irqs);

/* return last irq_enable status */
int hieth_irq_disable(struct hieth_netdev_local *ld, int irqs);

/* return irqstatus */
int hieth_read_irqstatus(struct hieth_netdev_local *ld);

int hieth_read_raw_irqstatus(struct hieth_netdev_local *ld);
int hieth_clear_irqstatus(
        struct hieth_netdev_local *ld,
        int irqs);    /* return irqstatus after clean */

int hieth_set_endian_mode(struct hieth_netdev_local *ld, int mode);

/* Tx/Rx queue operation */
int hieth_set_hwq_depth(struct hieth_netdev_local *ld);

#define HIETH_INVALID_RXPKG_LEN(len) \
        (!((len) >= 42 && (len) <= HIETH_MAX_FRAME_SIZE))

int hieth_hw_xmitq_ready(struct hieth_netdev_local *ld);

int hieth_hw_recv_tryup(struct hieth_netdev_local *ld);
int hieth_hw_set_macaddress(
        struct hieth_netdev_local *ld,
        int ena, unsigned char *mac);
int hieth_hw_get_macaddress(struct hieth_netdev_local *ld, unsigned char *mac);

int hieth_feed_hw(struct hieth_netdev_local *ld, struct hieth_priv_s *priv);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* __HIETH_CTRL_H */

/* vim: set ts=8 sw=8 tw=78: */
