#ifndef __DMAC_EXT_H__
#define __DMAC_EXT_H__
#include "los_typedef.h"

typedef void REG_ISR(int index,unsigned int channel,void* callback_data);
typedef void PERI_REG_ISR (void* callback_data, int return_status);


/*definition for the return value*/
#define HIEDMAC_ERROR_BASE                100
#define DMAC_CHANNEL_INVALID            (HIEDMAC_ERROR_BASE+1)

#define DMAC_TRXFERSIZE_INVALID            (HIEDMAC_ERROR_BASE+2)
#define DMAC_SOURCE_ADDRESS_INVALID        (HIEDMAC_ERROR_BASE+3)
#define DMAC_DESTINATION_ADDRESS_INVALID    (HIEDMAC_ERROR_BASE+4)
#define DMAC_MEMORY_ADDRESS_INVALID        (HIEDMAC_ERROR_BASE+5)
#define DMAC_PERIPHERAL_ID_INVALID        (HIEDMAC_ERROR_BASE+6)
#define DMAC_DIRECTION_ERROR            (HIEDMAC_ERROR_BASE+7)
#define DMAC_TRXFER_ERROR            (HIEDMAC_ERROR_BASE+8)
#define DMAC_LLIHEAD_ERROR            (HIEDMAC_ERROR_BASE+9)
#define DMAC_SWIDTH_ERROR            (HIEDMAC_ERROR_BASE+0xa)
#define DMAC_LLI_ADDRESS_INVALID        (HIEDMAC_ERROR_BASE+0xb)
#define DMAC_TRANS_CONTROL_INVALID        (HIEDMAC_ERROR_BASE+0xc)
#define DMAC_MEMORY_ALLOCATE_ERROR        (HIEDMAC_ERROR_BASE+0xd)
#define DMAC_NOT_FINISHED            (HIEDMAC_ERROR_BASE+0xe)

#define DMAC_TIMEOUT                (HIEDMAC_ERROR_BASE+0xf)
#define DMAC_CHN_SUCCESS            (HIEDMAC_ERROR_BASE+0x10)
#define DMAC_CHN_ERROR                (HIEDMAC_ERROR_BASE+0x11)
#define DMAC_CHN_TIMEOUT            (HIEDMAC_ERROR_BASE+0x12)
#define DMAC_CHN_ALLOCAT            (HIEDMAC_ERROR_BASE+0x13)
#define DMAC_CHN_VACANCY            (HIEDMAC_ERROR_BASE+0x14)

int dmac_m2m_transfer(AARCHPTR source, AARCHPTR dest, unsigned int length);
int do_dma_p2m(AARCHPTR memaddr, unsigned int peri_addr, unsigned int length, PERI_REG_ISR* pisr, void* callback_data);
int do_dma_m2p(AARCHPTR memaddr, unsigned int peri_addr, unsigned int length, PERI_REG_ISR* pisr, void* callback_data);
#if 0
int hi_dmac_init(void);
//int dmac_m2p_transfer(uint32_t *pmem, uint32_t peri_id, uint32_t size);
int dmac_m2p_transfer(uint32_t pmem, uint32_t peri_id, uint32_t size);
int dmac_llip2m_transfer(AARCHPTR memaddr, uint32_t peri_id, uint32_t size, uint32_t *channel);
int dmac_m2m_transfer(AARCHPTR psrc, AARCHPTR pdest, uint32_t size);
int dmac_llim2m_transfer(uint32_t psrc, uint32_t pdest, uint32_t size);
int dmac_channelstart(uint32_t channel);
//int dmac_get_current_src(uint32_t channel);
uint32_t dmac_get_current_dest(uint32_t channel);
int dmac_wait(uint32_t channel);
uint32_t dmac_channel_allocate(void *pisr);
int dmac_channelclose(uint32_t channel);
int dmac_register_isr(uint32_t channel,void *pisr);
int dmac_channel_free(uint32_t channel);

int dmac_start_llim2p(uint32_t channel,
                        uint32_t *pfirst_lli,
                        uint32_t peri_id);

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

#endif

