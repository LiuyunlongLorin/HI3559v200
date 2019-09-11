#ifndef __HIGMAC_H__
#define __HIGMAC_H__

#include <linux/kernel.h>
#include "phy_fix.h"
#include "hisoc/net.h"
//#include <machine/types.h>
//#include <sys/bsdtypes.h>

//#include <sys/socket.h>
#include <linux/platform_device.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define HIGMAC_DRIVER_NAME    "hi_gmac_v200"

//#define HIGMAC_TSO_DEBUG
/* #undef HIGMAC_TSO_DEBUG */
#define HIGMAC_TSO_SUPPORTED

#ifdef HIGMAC_TSO_SUPPORTED
#include "tso.h"
#endif

#define HIETH_INVALID_RXPKG_LEN(len)    \
        (!((len) >= 42 && (len) <= HIETH_MAX_FRAME_SIZE))

#define HIGMAC_IOSIZE            (0x1000)
#define HIGMAC_OFFSET            (HIGMAC_IOSIZE)

#define RX_BQ_IN_INT            (1<<17)
#define TX_RQ_IN_INT            (1<<19)
#define RX_BQ_IN_TIMEOUT_INT        (1<<28)
#define TX_RQ_IN_TIMEOUT_INT        (1<<29)

#define RX_OUTCFF_WR_DESC_ENA        (1<<3)
#define RX_CFF_RD_DESC_ENA        (1<<2)
#define TX_OUTCFF_WR_DESC_ENA        (1<<1)
#define TX_CFF_RD_DESC_ENA        (1<<0)

#ifdef LOSCFG_ARCH_CORTEX_A53_AARCH64
#define HIGMAC_SYNC_BARRIER() do { isb(); dsb(sy); dmb(ishst); } while (0)
#else
#define HIGMAC_SYNC_BARRIER() do { isb(); dsb(); dmb(); } while (0)
#endif

enum if_mode {/* interface mode */
    interface_mode_mii,
    interface_mode_rmii,
    interface_mode_rgmii,
    interface_mode_butt
};

#define RGMII_SPEED_1000        0x2c
#define RGMII_SPEED_100            0x2f
#define RGMII_SPEED_10            0x2d
#define MII_SPEED_100            0x0f
#define MII_SPEED_10            0x0d
#define RMII_SPEED_100            0x8f
#define RMII_SPEED_10            0x8d
#define GMAC_SPEED_1000            0x05
#define GMAC_SPEED_100            0x01
#define GMAC_SPEED_10            0x00
#define GMAC_FULL_DUPLEX        (1 << 4)

/* Interface Mode definitions */
typedef enum {
    PHY_INTERFACE_MODE_MII,
    PHY_INTERFACE_MODE_RMII,
    PHY_INTERFACE_MODE_RGMII,
    PHY_INTERFACE_MODE_MAX,
} phy_interface_t;

static inline const char *phy_modes(phy_interface_t interface)
{
        switch (interface) {
        case PHY_INTERFACE_MODE_MII:
                return "mii";
        case PHY_INTERFACE_MODE_RMII:
                return "rmii";
        case PHY_INTERFACE_MODE_RGMII:
                return "rgmii";
        default:
                return "unknown";
        }
}

enum speed_mode {
    speed_mode_10M,
    speed_mode_100M,
    speed_mode_1000M,
    speed_mode_butt
};

enum {/* DEFAULT: duplex_full */
    port_mode_10_mii = 0x1D,
    port_mode_100_mii = 0x1F,
    port_mode_10_rgmii = 0x3D,
    port_mode_100_rgmii = 0x3F,
    port_mode_1000_rgmii = 0x3C,
    port_mode_10_rmii = 0x9D,
    port_mode_100_rmii = 0x9F,
    port_mode_butt = 0xFF
};

#define HIGMAC_LINKED        (1 << 0)
#define HIGMAC_DUP_FULL        (1 << 1)
#define HIGMAC_SPD_10M        (1 << 2)
#define HIGMAC_SPD_100M        (1 << 3)
#define HIGMAC_SPD_1000M    (1 << 4)

#define DEFAULT_INTEPHY_LINK_STAT    (HIGMAC_DUP_FULL)

#define DEFAULT_LINK_STAT    ((ld->phy->phy_id == HISILICON_PHY_ID_FESTAV200)\
        ? DEFAULT_INTEPHY_LINK_STAT : 0)

#define RX_BQ_INT_THRESHOLD    0x40//0x40 /* TODO: */
#define TX_RQ_INT_THRESHOLD    0x20 /* TODO: */

#define HIGMAC_MAX_QUEUE_DEPTH    (2048)

#define HIGMAC_HWQ_RX_FQ_DEPTH    (1024)
#define HIGMAC_HWQ_RX_BQ_DEPTH    (HIGMAC_HWQ_RX_FQ_DEPTH)
#define HIGMAC_HWQ_TX_BQ_DEPTH    (1024)
#define HIGMAC_HWQ_TX_RQ_DEPTH    (HIGMAC_HWQ_TX_BQ_DEPTH)

#define HIGMAC_MONITOR_TIMER    (msecs_to_jiffies(200))

#define MAX_RX_POOLS        SZ_1K
#define HIETH_MAX_FRAME_SIZE    (1600)
#define SKB_SIZE        (HIETH_MAX_FRAME_SIZE)

#define DESC_VLD_FREE        0
#define DESC_VLD_BUSY        1

#define DESC_FL_FIRST        2
#define DESC_FL_MID        0
#define DESC_FL_LAST        1
#define DESC_FL_FULL        3

typedef struct {
        spinlock_t lock;
        unsigned long flags;
} HISI_NET_SPINLOCK_T;

#ifdef HISI_NET_USE_MUTEX
#define HISI_NET_LOCK_T(net_lock) pthread_mutex_t net_lock
#define HISI_NET_LOCK_INIT(net_lock) do { mutex_init(net_lock); } while (0)
#define HISI_NET_LOCK_GET(net_lock) do { mutex_lock(net_lock); } while (0)
#define HISI_NET_LOCK_PUT(net_lock) do { mutex_unlock(net_lock); } while (0)
#endif

#ifdef HISI_NET_USE_SPINLOCK
#define HISI_NET_LOCK_T(net_lock) HISI_NET_SPINLOCK_T net_lock
#define HISI_NET_LOCK_INIT(net_lock) do { spin_lock_init(&((net_lock)->lock)); } while (0)
#define HISI_NET_LOCK_GET(net_lock) do { spin_lock_irqsave(&((net_lock)->lock), (net_lock)->flags); } while (0)
#define HISI_NET_LOCK_PUT(net_lock) do { spin_unlock_irqrestore(&(net_lock)->lock, (net_lock)->flags); } while (0)
#endif

#ifdef HISI_NET_NON_USE_LOCK
#define HISI_NET_LOCK_T(net_lock)
#define HISI_NET_LOCK_INIT(net_lock) do { } while (0)
#define HISI_NET_LOCK_GET(net_lock) do { } while (0)
#define HISI_NET_LOCK_PUT(net_lock) do { } while (0)
#endif

#ifdef CONFIG_HIGMAC_DESC_4_WORD
#define DESC_WORD_SHIFT         (2)
#else
#define DESC_WORD_SHIFT         (3)
#endif
#define DESC_BYTE_SHIFT         (DESC_WORD_SHIFT + 2)
#define DESC_WORD_CNT           (1 << DESC_WORD_SHIFT)
#define DESC_SIZE               (1 << DESC_BYTE_SHIFT)

#ifdef CONFIG_HIGMAC_DESC_4_WORD
struct higmac_desc {
    unsigned int data_buff_addr;

    unsigned int buffer_len:11;
    unsigned int reserve2:5;
    unsigned int data_len:11;
    unsigned int reserve1:2;
    unsigned int fl:2;
    unsigned int descvid:1;

    unsigned int reserve_desc2;
    unsigned int reserve_desc3;
};

struct higmac_tso_desc {
    unsigned int data_buff_addr;

    union {
        struct {
            unsigned int prot_hdr_len:4;
            unsigned int ip_hdr_len:4;
            unsigned int prot_type:1;
            unsigned int ip_ver:1;
            unsigned int vlan_flag:1;
            unsigned int nfrags_num:5;
            unsigned int data_len:11;
            unsigned int reservel:1;
            unsigned int tso_flag:1;
            unsigned int coe_flag:1;
            unsigned int sg_flag:1;
            unsigned int hw_own:1;
        } tx;
        unsigned int val;
    } desc1;

    unsigned int reserve_desc2;
    unsigned int tx_err;
};
#else
struct higmac_desc {
    unsigned int data_buff_addr;

    unsigned int buffer_len:11;
    unsigned int reserve2:5;
    unsigned int data_len:11;
    unsigned int reserve1:2;
    unsigned int fl:2;
    unsigned int descvid:1;

    unsigned int reserve_desc2;
    unsigned int reserve_desc3;

    unsigned int reserve_desc4;
    unsigned int reserve_desc5;
    unsigned int reserve_desc6;
    unsigned int reserve_desc7;
};

struct higmac_tso_desc {
    unsigned int data_buff_addr;

    union {
        struct {
            unsigned int prot_hdr_len:4;
            unsigned int ip_hdr_len:4;
            unsigned int prot_type:1;
            unsigned int ip_ver:1;
            unsigned int vlan_flag:1;
            unsigned int nfrags_num:5;
            unsigned int data_len:11;
            unsigned int reservel:1;
            unsigned int tso_flag:1;
            unsigned int coe_flag:1;
            unsigned int sg_flag:1;
            unsigned int hw_own:1;
        } tx;
        unsigned int val;
    } desc1;

    unsigned int reserve_desc2;
    unsigned int reserve_desc3;

    unsigned int tx_err;
    unsigned int reserve_desc5;
    unsigned int reserve_desc6;
    unsigned int reserve_desc7;
};
#endif

//#define SKB_MAGIC    ((struct sk_buff *)0x5a)

#define QUEUE_NUMS    (4)

struct higmac_netdev_local
{
#ifdef HIGMAC_TSO_SUPPORTED
#define HIGMAC_SG_DESC_ADD    (64U)
    struct sg_desc *dma_sg_desc;// ____cacheline_aligned;
    unsigned long dma_sg_phy;
    unsigned int sg_head;
    unsigned int sg_tail;
    unsigned int sg_count;
#endif
    char    *gmac_iobase;
    unsigned int phy_id;
    struct {
        struct higmac_desc *desc;
        AARCHPTR    phys_addr;

        /* how many desc in the desc pool */
        unsigned int    count;
        struct los_sk_buff **skb;

        /* sizeof(desc) * count */
        unsigned int    size;

        /* debug info */
        //unsigned int    use_cnt;
    } pool[QUEUE_NUMS];

    int phy_mode;
    int phy_addr;
#define rx_fq        pool[0]
#define rx_bq        pool[1]
#define tx_bq        pool[2]
#define tx_rq        pool[3]
    int tx_busy;
    HISI_NET_LOCK_T(tx_lock);
    HISI_NET_LOCK_T(rx_lock);
};


struct higmac_platform_data
{
    struct higmac_netdev_local stNetdevLocal;
    EVENT_CB_S stEvent;
};

#define EVENT_NET_TX_RX       0x1
#define EVENT_NET_CAN_SEND    0x2

#ifdef HIGMAC_TSO_SUPPORTED
enum tso_version {
    VER_NO_TSO = 0x0,
    VER_BYTE_SPLICE = 0x100,
    VER_SG_COE = 0x200,
    VER_TSO = 0x300,
};
#endif

struct higmac_board_info {
    //struct net_device *netdev;
    char *gmac_name;
    char *mii_name;
    int phy_addr;
    //phy_interface_t phy_intf;
    //struct sockaddr macaddr;
#ifdef HIGMAC_TSO_SUPPORTED
    unsigned int    tso_ver;
#endif
};

#define MAX_GMAC_NUMS        (2)

#ifdef CONFIG_ARCH_S40
#define CONFIG_GMAC_NUMS    (2)
#else
#define CONFIG_GMAC_NUMS    (1)
#endif

#if 1
/* describe board configuration: phy_addr, phy_intf and mdio usage */
extern struct higmac_board_info higmac_board_info[MAX_GMAC_NUMS];

#ifndef HISI_NET_NON_USE_LOCK
extern HISI_NET_LOCK_T(eth_crg_lock);
#endif
#ifdef HIGMAC_TSO_SUPPORTED
int higmac_tx_avail(struct higmac_netdev_local *ld);
#endif
/* trace rx desc's feed and reclaim action */
#define DEBUG_HW_RX_DESC    (1 << 0)
#define DEBUG_HW_TX_DESC    (1 << 1)
#define DEBUG_HW_IRQ        (1 << 2)
#define DEBUG_AUTOEEE        (1 << 3)
#define DEBUG_DEFAULT        (0)/* TODO: change it to 0 */
/* TODO: add more debug option here */
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
