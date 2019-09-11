/* ./drivers/hidmac/hi_dmac.h
 *
 *
 * History:
 *      17-August-2006 create this file
 */
#ifndef __HI_DMAC_H__
#define __HI_DMAC_H__

#define DMAC_DRIVER_NAME "hi-dmac"


#define  hidmac_writew(addr, value)\
        ((*(volatile unsigned int *)((long)addr)) = (value))
#define  hidmac_readw(addr, v)\
        (v = (*(volatile unsigned int *)((long)addr)))

#define DMAC_CONFIGURATIONx_HALT_DMA_ENABLE    (0x01<<18)
#define DMAC_CONFIGURATIONx_ACTIVE             (0x01L<<17)
#define DMAC_CONFIGURATIONx_CHANNEL_ENABLE     (0x1)
#define DMAC_CONFIGURATIONx_CHANNEL_DISABLE    (0x0)

#define DMAC_MAX_LLI_SIZE   PAGE_SIZE
/*definition for the return value*/
#define HIDMAC_ERROR_BASE                100
#define DMAC_CHANNEL_INVALID            (HIDMAC_ERROR_BASE+1)

#define DMAC_TRXFERSIZE_INVALID            (HIDMAC_ERROR_BASE+2)
#define DMAC_SOURCE_ADDRESS_INVALID        (HIDMAC_ERROR_BASE+3)
#define DMAC_DESTINATION_ADDRESS_INVALID    (HIDMAC_ERROR_BASE+4)
#define DMAC_MEMORY_ADDRESS_INVALID        (HIDMAC_ERROR_BASE+5)
#define DMAC_PERIPHERAL_ID_INVALID        (HIDMAC_ERROR_BASE+6)
#define DMAC_DIRECTION_ERROR            (HIDMAC_ERROR_BASE+7)
#define DMAC_TRXFER_ERROR            (HIDMAC_ERROR_BASE+8)
#define DMAC_LLIHEAD_ERROR            (HIDMAC_ERROR_BASE+9)
#define DMAC_SWIDTH_ERROR            (HIDMAC_ERROR_BASE+0xa)
#define DMAC_LLI_ADDRESS_INVALID        (HIDMAC_ERROR_BASE+0xb)
#define DMAC_TRANS_CONTROL_INVALID        (HIDMAC_ERROR_BASE+0xc)
#define DMAC_MEMORY_ALLOCATE_ERROR        (HIDMAC_ERROR_BASE+0xd)
#define DMAC_NOT_FINISHED            (HIDMAC_ERROR_BASE+0xe)

#define DMAC_TIMEOUT                (HIDMAC_ERROR_BASE+0xf)
#define DMAC_CHN_SUCCESS            (HIDMAC_ERROR_BASE+0x10)
#define DMAC_CHN_ERROR                (HIDMAC_ERROR_BASE+0x11)
#define DMAC_CHN_TIMEOUT            (HIDMAC_ERROR_BASE+0x12)
#define DMAC_CHN_ALLOCAT            (HIDMAC_ERROR_BASE+0x13)
#define DMAC_CHN_VACANCY            (HIDMAC_ERROR_BASE+0x14)

#define DMAC_CONFIGURATIONx_ACTIVE_NOT        0

typedef struct mem_addr {
    unsigned int memaddr_base;
    unsigned int memsize;
} mem_addr;

typedef unsigned int dma_addr_t;
/* #define PAGE_SIZE 0x1000 */

#endif /* End  */

