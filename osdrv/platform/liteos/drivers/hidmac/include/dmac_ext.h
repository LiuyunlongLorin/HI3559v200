#ifndef __DMAC_EXT_H__
#define __DMAC_EXT_H__
#include "los_typedef.h"

/*sturcture for LLI*/
typedef struct dmac_lli {
    uint32_t src_addr;            /* source address */
    uint32_t dst_addr;            /* destination address */
    uint32_t next_lli;            /* pointer to next LLI */
    uint32_t lli_transfer_ctrl;    /* control word */
} dmac_lli;

/*sturcture for LLI, for dmac suspend*/
typedef struct dmac_lli_bak{
    uint32_t src_addr;            /* source address */
    uint32_t dst_addr;            /* destination address */
    uint32_t next_lli;            /* pointer to next LLI */
    uint32_t lli_transfer_ctrl;    /* control word */
    uint32_t lli_transfer_config;  /* config word */
} dmac_lli_bak;

typedef struct hi_dma_llihead {
    uint32_t phys;
    uint32_t virt;
} hi_dma_llihead;

int hi_dmac_init(void);
//int dmac_m2p_transfer(uint32_t *pmem, uint32_t peri_id, uint32_t size);
int dmac_m2p_transfer(uint32_t pmem, uint32_t peri_id, uint32_t size);
//int dmac_m2m_transfer(uint32_t *psrc, uint32_t *pdest, uint32_t size);
int dmac_llip2m_transfer(uint32_t pmem, uint32_t peri_id, uint32_t size, uint32_t *channel);
int dmac_m2m_transfer(uint32_t psrc, uint32_t pdest, uint32_t size);
int dmac_llim2m_transfer(uint32_t psrc, uint32_t pdest, uint32_t size);
int dmac_channelstart(uint32_t channel);
int dmac_start_llim2m(uint32_t channel, dmac_lli *pfirst_lli);
//int dmac_get_current_src(uint32_t channel);
uint32_t dmac_get_current_dest(uint32_t channel);
int dmac_wait(uint32_t channel);
uint32_t dmac_channel_allocate(void *pisr);
int allocate_dmalli_space(dmac_lli **ppheadlli, uint32_t length);
int dmac_channelclose(uint32_t chan);
int dmac_register_isr(unsigned int ch, void *pisr);
int dmac_channel_free(uint32_t channel);
void free_dmalli_space(dmac_lli *ppheadlli);

int dmac_start_llim2p(uint32_t channel,
                        uint32_t *pfirst_lli,
                        uint32_t peri_id);

int dmac_buildllim2m(dmac_lli * ppheadlli,
        uint32_t psrc,
        uint32_t pdest,
        uint32_t total_size,
        uint32_t uwnumtransfers);

int  dmac_buildllip2m(dmac_lli *ppheadlli,
        uint32_t uwperipheralid,
        uint32_t memaddr,
        uint32_t totaltransfersize,
        uint32_t uwnumtransfers);

int  dmac_buildllim2p(hi_dma_llihead *ppheadlli,
                    uint32_t *pmemaddr,
                    uint32_t uwperipheralid,
                    uint32_t totaltransfersize,
                    uint32_t uwnumtransfers ,uint32_t burstsize);

int dmac_start_m2p(uint32_t channel,
                    uint32_t pmem,
                    uint32_t peri_id,
                    uint32_t  uwnumtransfers,
                    uint32_t next_lli_addr);

int dmac_start_m2m(uint32_t channel,
                    uint32_t psrc,
                    uint32_t pdest,
                    uint32_t uwnumtransfers);

void dmac_suspend(uint32_t channel);

void dmac_resume(uint32_t channel);
#endif

