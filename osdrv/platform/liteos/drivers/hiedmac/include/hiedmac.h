#ifndef __HIEDMAC_H__
#define __HIEDMAC_H__

#include "dmac_ext.h"

#define EDMAC_CHANNEL_NUM 8
typedef AARCHPTR dma_addr_t;
typedef UINT32 dmac_mutex;
typedef EVENT_CB_S dmac_event;
typedef int dma_spinlock_t;

#define dma_mutex_init(mutex)   LOS_MuxCreate(mutex)
#define dma_mutex_lock(mutex, timeout)   LOS_MuxPend(mutex, timeout)
#define dma_mutex_unlock(mutex) LOS_MuxPost(mutex)

#define dma_event_init(event)   LOS_EventInit(event)
#define dma_event_signal(event, bit)    LOS_EventWrite(event, bit)
#define dma_event_wait(event, bit, timeout)  LOS_EventRead(event, bit,\
                                                     LOS_WAITMODE_OR+LOS_WAITMODE_CLR, timeout)
#define dma_event_clear(event, bit)     LOS_EventClear(event, ~bit)

#define dma_spin_lock_init(lock) spin_lock_init(lock)
#define dma_spin_lock(lock) do { LOS_TaskLock();}while(0)
#define dma_spin_unlock(lock) do { LOS_TaskUnlock();}while(0)

#define dma_spin_lock_irqsave(lock, flags)  do { flags = LOS_IntLock();spin_lock(lock);} while(0)
#define dma_spin_unlock_irqrestore(lock, flags)  do { spin_unlock(lock);LOS_IntRestore(flags);} while(0)

/*structure for LLI*/
typedef struct dmac_lli_info {
    //must be 64Byte aligned
    long long next_lli;
    unsigned int reserved[5];
    unsigned int count;
    long long src_addr;
    long long dest_addr;
    unsigned int config;
    unsigned int pad[51];
} dmac_lli;

struct hiedmac_tsf_info {
    unsigned int host_index;
    unsigned int channel;
    unsigned int transfer_type;
    unsigned int peripheralid;
    PERI_REG_ISR*   callback;
    void *         callback_data;
};


struct hiedmac_host {
    int          index;
    unsigned int base;
    int          irq;
#define DMA_MUTEX_WAIT_FOREVER LOS_WAIT_FOREVER
#define DMA_MUTEX_WAIT_DEF_TIME LOSCFG_BASE_CORE_TICK_PER_SECOND*10
    dma_addr_t  dma_lliaddr[EDMAC_CHANNEL_NUM]; 
    char        channel_status[EDMAC_CHANNEL_NUM];
    dma_spinlock_t lock;
    REG_ISR*     function[EDMAC_CHANNEL_NUM];
    dmac_event   wait_event[EDMAC_CHANNEL_NUM];
#define DMA_EVENT_WAIT_DEF_TIME LOSCFG_BASE_CORE_TICK_PER_SECOND*5
#define HIEDMA_EVENT_DONE   0x1
#define HIEDMA_EVENT_ERROR  0x2
    struct hiedmac_tsf_info cur_tsf[EDMAC_CHANNEL_NUM];
    unsigned int channel_num;
};




/*DMAC peripheral structure*/
typedef struct edmac_peripheral {
    /* peripherial ID*/
    unsigned int peri_id;
    /*peripheral data register address*/
    unsigned long peri_addr;
    /* config requset */
    int host_sel;
#define DMAC_HOST0 0
#define DMAC_HOST1 1
#define DMAC_NOT_USE (-1)
    /*default channel configuration word*/
    unsigned int transfer_cfg;
    /*default channel configuration word*/
    unsigned int transfer_width;
    unsigned int dynamic_periphery_num;
} edmac_peripheral;


#endif
