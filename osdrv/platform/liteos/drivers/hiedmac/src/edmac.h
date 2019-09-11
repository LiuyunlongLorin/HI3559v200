/* ./drivers/hidmac/hi_dmac.h
 *
 *
 * History:
 *      17-August-2006 create this file
 */
#ifndef __EDMAC_H__
#define __EDMAC_H__

#define  hiedmac_readw(addr, v)\
	        (v = (*(volatile unsigned int *)((long)addr)))

#define hiedmac_readl(addr) ({unsigned int reg = readl((unsigned int)(addr)); \
            reg; })

#define hiedmac_writel(addr, v) do { \
        writel(v, (unsigned int)(addr)); \
} while (0)

#define hiedma_err(msg...) do { \
    dprintf("%s:%d: ", __FUNCTION__, __LINE__);\
    dprintf(msg); \
    dprintf("\n"); \
} while (0)

//#define EDMA_DEBUG
#ifdef EDMA_DEBUG
    #define hiedma_debug(msg...) do { \
                dprintf("%s:%d: ", __FUNCTION__, __LINE__);\
                dprintf(msg); \
                dprintf("\n"); \
            } while (0)
#else
    #define hiedma_debug(msg...) do { }while(0)
#endif


#define MAX_TRANSFER_BYTES  0xffff

/* reg offset */
#define HIEDMAC_INT_STAT                  (0x0)
#define HIEDMAC_INT_TC1                   (0x4)
#define HIEDMAC_INT_TC2                   (0x8)
#define HIEDMAC_INT_ERR1                  (0xc)
#define HIEDMAC_INT_ERR2                  (0x10)
#define HIEDMAC_INT_ERR3                  (0x14)

#define HIEDMAC_INT_TC1_MASK              (0x18)
#define HIEDMAC_INT_TC2_MASK              (0x1c)
#define HIEDMAC_INT_ERR1_MASK             (0x20)
#define HIEDMAC_INT_ERR2_MASK             (0x24)
#define HIEDMAC_INT_ERR3_MASK             (0x28)

#define HIEDMAC_INT_TC1_RAW               (0x600)
#define HIEDMAC_INT_TC2_RAW               (0x608)
#define HIEDMAC_INT_ERR1_RAW              (0x610)
#define HIEDMAC_INT_ERR2_RAW              (0x618)
#define HIEDMAC_INT_ERR3_RAW              (0x620)

#define HIEDMAC_Cx_CURR_CNT0(cn)          (0x404+cn*0x20)
#define HIEDMAC_Cx_CURR_SRC_ADDR_L(cn)    (0x408+cn*0x20)
#define HIEDMAC_Cx_CURR_SRC_ADDR_H(cn)    (0x40c+cn*0x20)
#define HIEDMAC_Cx_CURR_DEST_ADDR_L(cn)    (0x410+cn*0x20)
#define HIEDMAC_Cx_CURR_DEST_ADDR_H(cn)    (0x414+cn*0x20)

#define HIEDMAC_SREQ                      (0x660)
#define HIEDMAC_LSREQ                     (0x664)
#define HIEDMAC_BREQ                      (0x668)
#define HIEDMAC_LBREQ                     (0x66c)

#define HIEDMAC_CH_PRI                    (0x688)
#define HIEDMAC_CH_STAT                   (0x690)
#define HIEDMAC_DMA_CTRL                  (0x698)

#define HIEDMAC_Cx_BASE(cn)               (0x800+cn*0x40)
#define HIEDMAC_Cx_LLI_L(cn)              (0x800+cn*0x40)
#define HIEDMAC_Cx_LLI_H(cn)              (0x804+cn*0x40)
#define HIEDMAC_Cx_CNT0(cn)               (0x81c+cn*0x40)
#define HIEDMAC_Cx_SRC_ADDR_L(cn)         (0x820+cn*0x40)
#define HIEDMAC_Cx_SRC_ADDR_H(cn)         (0x824+cn*0x40)
#define HIEDMAC_Cx_DEST_ADDR_L(cn)         (0x828+cn*0x40)
#define HIEDMAC_Cx_DEST_ADDR_H(cn)         (0x82c+cn*0x40)
#define HIEDMAC_Cx_CONFIG(cn)             (0x830+cn*0x40)

#define HIEDMAC_ALL_CHAN_CLR		(0xff)
#define HIEDMAC_INT_ENABLE_ALL_CHAN	(0xff)


#define HIEDMAC_CONFIG_SRC_INC          (1<<31)
#define HIEDMAC_CONFIG_DST_INC          (1<<30)

#define HIEDMAC_CONFIG_SRC_WIDTH_SHIFT  (16)
#define HIEDMAC_CONFIG_DST_WIDTH_SHIFT  (12)
#define HIEDMAC_WIDTH_8BIT              (0x0)
#define HIEDMAC_WIDTH_16BIT             (0x1)
#define HIEDMAC_WIDTH_32BIT             (0x10)
#define HIEDMAC_WIDTH_64BIT             (0x11)

#define HIEDMAC_MAX_BURST_WIDTH         (16)
#define HIEDMAC_MIN_BURST_WIDTH         (1)
#define HIEDMAC_CONFIG_SRC_BURST_SHIFT  (24)
#define HIEDMAC_CONFIG_DST_BURST_SHIFT  (20)

#define HIEDMAC_LLI_ALIGN   0x40
#define HIEDMAC_LLI_DISABLE 0x0
#define HIEDMAC_LLI_ENABLE 0x2

#define HIEDMAC_CXCONFIG_SIGNAL_SHIFT   (0x4)
#define HIEDMAC_CXCONFIG_MEM_TYPE       (0x0)
#define HIEDMAC_CXCONFIG_DEV_MEM_TYPE   (0x1)
#define HIEDMAC_CXCONFIG_TSF_TYPE_SHIFT (0x2)
#define HIEDMAC_CxCONFIG_LLI_START      (0x1)

#define HIEDMAC_CXCONFIG_ITC_EN		(0x1)
#define HIEDMAC_CXCONFIG_ITC_EN_SHIFT	(0x1)

#define HIEDMAC_CxCONFIG_M2M            0xCFF00001
#define HIEDMAC_CxCONFIG_M2M_LLI        0xCFF00000
#define HIEDMAC_CxCONFIG_CHN_START  0x1
#define HIEDMAC_Cx_DISABLE           0x0

#define HIEDMAC_M2M          0x0
#define HIEDMAC_NOM2M        0x1

#define HIEDMAC_TRANS_MAXSIZE     (64*1024-1)

#endif /* End of #ifndef __HI_INC_ECSDMACC_H__ */

