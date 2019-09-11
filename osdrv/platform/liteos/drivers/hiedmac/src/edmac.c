#include "los_hw.h"
#include "linux/interrupt.h"
#include "linux/kernel.h"
#include "dmac_ext.h"
#include "los_event.h"
#include "asm/delay.h"
#include "linux/module.h"
#include "linux/device.h"
#include "linux/platform_device.h"
#include "edmac.h"
#include "hisoc/dmac.h"

HIEDMAC_HOST;
HIEDMAC_PERIPHERAL_INFO;

#define TRASFER_TYPE_M2M  0x0
#define TRASFER_TYPE_P2M  0x1
#define TRASFER_TYPE_M2P  0x2
#define PERIPHERALID_INVILD 0xfff

extern void dma_cache_clean(AARCHPTR start, AARCHPTR end);
extern void dma_cache_inv(AARCHPTR start, AARCHPTR end);
void free_dmalli_space(struct hiedmac_tsf_info* tsf_info);
void edmac_channel_free(struct hiedmac_tsf_info* tsf_info);

void m2m_callback(int host_index, unsigned int dma_chn, void* callback_data)
{
    struct hiedmac_host* hiedmac = NULL;

    hiedmac = &hiedmac_driver_data[host_index];
    if (hiedmac->channel_status[dma_chn] == DMAC_CHN_ERROR)
        dma_event_signal(hiedmac->wait_event, HIEDMA_EVENT_ERROR);
    else
        dma_event_signal(hiedmac->wait_event, HIEDMA_EVENT_DONE);
}

/*
 * dmac interrupt handle function
 */
static irqreturn_t edmac_isr(int irq, void *dev)
{
    struct hiedmac_host* hiedmac;
    hiedmac = (struct hiedmac_host*)dev;

    unsigned int channel_err_status[3];
    unsigned int channel_status = 0;
    unsigned int channel_tc_status = 0;
    unsigned int i;
    struct hiedmac_tsf_info* tsf_info = NULL;

    channel_status = hiedmac_readl(hiedmac->base + HIEDMAC_INT_STAT);
    hiedma_debug("%s,%d,channel_status = 0x%x\n",__func__,__LINE__,channel_status);

    if (!channel_status) {
        hiedma_err("channel_status = 0x%x\n", channel_status);
        return IRQ_NONE;
    }

    for (i = 0; i < hiedmac->channel_num; i++)
    {
        if ((channel_status >> i) & 0x1) {
            channel_tc_status = hiedmac_readl(hiedmac->base + HIEDMAC_INT_TC1);
            channel_tc_status = (channel_tc_status >> i) & 0x01;
            if (channel_tc_status)
                hiedmac_writel(hiedmac->base + HIEDMAC_INT_TC1_RAW, channel_tc_status << i);
            hiedma_debug("HIEDMAC_INT_TC1 = 0x%x\n", hiedmac_readl(hiedmac->base + HIEDMAC_INT_TC1));

            channel_tc_status = hiedmac_readl(hiedmac->base + HIEDMAC_INT_TC2);
            channel_tc_status = (channel_tc_status >> i) & 0x01;
            if (channel_tc_status)
                hiedmac_writel(hiedmac->base + HIEDMAC_INT_TC2_RAW, channel_tc_status << i);
            hiedma_debug("HIEDMAC_INT_TC2 = 0x%x\n", hiedmac_readl(hiedmac->base + HIEDMAC_INT_TC2));

            channel_err_status[0] = hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR1);
            channel_err_status[0] = (channel_err_status[0] >> i) & 0x01;
            channel_err_status[1] = hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR2);
            channel_err_status[1] = (channel_err_status[1] >> i) & 0x01;
            channel_err_status[2] = hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR3);
            channel_err_status[2] = (channel_err_status[2] >> i) & 0x01;

            if (channel_err_status[0] | channel_err_status[1] | channel_err_status[2]) {
                hiedma_err("Error in hiedmac %d finish!,ERR1 = 0x%x,ERR2 = 0x%x,ERR3 = 0x%x\n",
                        i, channel_err_status[0],channel_err_status[1],channel_err_status[2]);

                hiedma_debug("Error in hiedmac %d finish!,ERR1 = 0x%x,ERR2 = 0x%x,ERR3 = 0x%x\n",
                        i, channel_err_status[0],channel_err_status[1],channel_err_status[2]);
                hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR1_RAW, 1<<i);
                hiedma_debug("HIEDMAC_INT_ERR1_RAW = 0x%x\n", hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR1_RAW));
                hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR2_RAW, 1<<i);
                hiedma_debug("HIEDMAC_INT_ERR2_RAW = 0x%x\n", hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR2_RAW));
                hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR3_RAW, 1<<i);
                hiedma_debug("HIEDMAC_INT_ERR3_RAW = 0x%x\n", hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR3_RAW));
                tsf_info = &hiedmac->cur_tsf[i];
                if (!tsf_info) {
                    hiedma_err("tsf_info is null!\n");
                    return IRQ_HANDLED;
                }

                hiedmac->channel_status[i] = DMAC_CHN_ERROR;
                if (hiedmac->function[i] != NULL)
                    hiedmac->function[i](hiedmac->index, i, tsf_info->callback_data);

                return IRQ_HANDLED;
            }
            tsf_info = &hiedmac->cur_tsf[i];
            if (!tsf_info) {
                hiedma_err("tsf_info is null!\n");
                return IRQ_HANDLED;
            }

            hiedmac->channel_status[i] = DMAC_CHN_SUCCESS;
            if (hiedmac->function[i] != NULL)
                    hiedmac->function[i](hiedmac->index, i, tsf_info->callback_data);
        }
    }
    return IRQ_HANDLED;
}


static struct hiedmac_tsf_info* get_trasfer_by_peripheraliaddr(unsigned int peri_addr)
{
    struct hiedmac_host* hiedmac = NULL;
    struct hiedmac_tsf_info* tsf_info = NULL;
    int i = 0, channel = 0;
    int peripheralid = PERIPHERALID_INVILD;

    for(i = 0;i < EDMAC_MAX_PERIPHERALS; i++) {
        if (g_peripheral[i].peri_addr == peri_addr)
            break;
    }

    if (i == EDMAC_MAX_PERIPHERALS) {
        hiedma_err("Invalid devaddr\n");
        return NULL;
    }

    peripheralid = i;

    for (i = 0;i < HIEDMAC_NUM; i++) {
        hiedmac = &hiedmac_driver_data[i];
        for (channel = 0; channel < hiedmac->channel_num; channel++) {
            tsf_info = &hiedmac->cur_tsf[channel];
            if (tsf_info->peripheralid == peripheralid) {
                return tsf_info;
            }
        }
    }
    return NULL;
}

/* not register interrupt, user can use this func to check dma transfer status */
int get_transfer_status(unsigned int peri_addr)
{
    struct hiedmac_tsf_info* tsf_info = NULL;
    struct hiedmac_host* hiedmac = NULL;
    unsigned int channel_err_status[3];
    unsigned int channel_status = 0;
    unsigned int channel_tc_status = 0;

    tsf_info = get_trasfer_by_peripheraliaddr(peri_addr);

    if (!tsf_info) {
        hiedma_err("Invalid devaddr\n");
        return -DMAC_PERIPHERAL_ID_INVALID;
    }

    hiedmac = &hiedmac_driver_data[tsf_info->host_index];

    channel_status = hiedmac_readl(hiedmac->base + HIEDMAC_INT_STAT);
    channel_status = (channel_status >> tsf_info->channel) & 0x1;
    if (!channel_status)
        return DMAC_NOT_FINISHED;

    channel_tc_status = hiedmac_readl(hiedmac->base + HIEDMAC_INT_TC1);
    channel_tc_status = (channel_tc_status >> tsf_info->channel) & 0x01;
    if (channel_tc_status)
        hiedmac_writel(hiedmac->base + HIEDMAC_INT_TC1_RAW, channel_tc_status << tsf_info->channel);

    channel_tc_status = hiedmac_readl(hiedmac->base + HIEDMAC_INT_TC2);
    channel_tc_status = (channel_tc_status >> tsf_info->channel) & 0x01;
    if (channel_tc_status)
        hiedmac_writel(hiedmac->base + HIEDMAC_INT_TC2_RAW, channel_tc_status << tsf_info->channel);

    channel_err_status[0] = hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR1);
    channel_err_status[0] = (channel_err_status[0] >> tsf_info->channel) & 0x01;
    channel_err_status[1] = hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR2);
    channel_err_status[1] = (channel_err_status[1] >> tsf_info->channel) & 0x01;
    channel_err_status[2] = hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR3);
    channel_err_status[2] = (channel_err_status[2] >> tsf_info->channel) & 0x01;

    if (channel_err_status[0] | channel_err_status[1] | channel_err_status[2]) {
        hiedma_err("Error in hiedmac %d finish!,ERR1 = 0x%x,ERR2 = 0x%x,ERR3 = 0x%x\n",
                tsf_info->channel, channel_err_status[0],channel_err_status[1],channel_err_status[2]);
        hiedma_debug("Error in hiedmac %d finish!,ERR1 = 0x%x,ERR2 = 0x%x,ERR3 = 0x%x\n",
                tsf_info->channel, channel_err_status[0],channel_err_status[1],channel_err_status[2]);
        hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR1_RAW, 1<<tsf_info->channel);
        hiedma_debug("HIEDMAC_INT_ERR1_RAW = 0x%x\n", hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR1_RAW));
        hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR2_RAW, 1<<tsf_info->channel);
        hiedma_debug("HIEDMAC_INT_ERR2_RAW = 0x%x\n", hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR2_RAW));
        hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR3_RAW, 1<<tsf_info->channel);
        hiedma_debug("HIEDMAC_INT_ERR3_RAW = 0x%x\n", hiedmac_readl(hiedmac->base + HIEDMAC_INT_ERR3_RAW));
        free_dmalli_space(tsf_info);
        edmac_channel_free(tsf_info);
        return DMAC_CHN_ERROR;
    }
    free_dmalli_space(tsf_info);
    edmac_channel_free(tsf_info);
    return DMAC_CHN_SUCCESS;
}

static int edmac_register_isr(struct hiedmac_tsf_info* tsf_info, REG_ISR* pisr)
{
    struct hiedmac_host* hiedmac;
    hiedmac = &hiedmac_driver_data[tsf_info->host_index];

    hiedmac->function[tsf_info->channel] = pisr;
    return 0;
}

/*
 * free channel
 */
void edmac_channel_free(struct hiedmac_tsf_info* tsf_info)
{
    struct hiedmac_host* hiedmac;
    unsigned long flags = 0;
    hiedmac = &hiedmac_driver_data[tsf_info->host_index];

    dma_spin_lock_irqsave(&hiedmac->lock, flags);
    hiedmac->channel_status[tsf_info->channel] = DMAC_CHN_VACANCY;
    memset(tsf_info, 0x0, sizeof(struct hiedmac_tsf_info));
    dma_spin_unlock_irqrestore(&hiedmac->lock, flags);
}

static int edmac_check_request(unsigned int peripheral_addr,
        int direction)
{
    unsigned int i;
    for(i = direction;i < EDMAC_MAX_PERIPHERALS; i+=2) {
        if (g_peripheral[i].peri_addr == peripheral_addr)
            return i;
    }
    hiedma_err("Invalid devaddr\n");
    return -1;
}

dma_addr_t* allocate_dmalli_space(struct hiedmac_tsf_info* tsf_info, unsigned int length)
{
    unsigned int lli_num = 0;
    unsigned int alloc_length = 0;
    struct hiedmac_host* hiedmac = NULL;
    dma_addr_t* alloc_addr = NULL;

    hiedma_debug("length = 0x%x\n", length);
    lli_num = length/(HIEDMAC_TRANS_MAXSIZE);
    if ((length % (HIEDMAC_TRANS_MAXSIZE)) != 0) {
        lli_num++;
    }

    hiedmac = &hiedmac_driver_data[tsf_info->host_index];

    alloc_length = lli_num * sizeof(dmac_lli);
    hiedma_debug("lli_num = %d, alloc_length = 0x%x\n",lli_num, alloc_length);

    alloc_length = ALIGN(alloc_length, CACHE_ALIGNED_SIZE);

    alloc_addr = (dma_addr_t*)memalign(256, alloc_length);
    if (!alloc_addr)
    {
        hiedma_err("can't malloc llimem for dma!\n ");
        return 0;
    }
    memset(alloc_addr, 0, alloc_length);

    hiedmac->dma_lliaddr[tsf_info->channel] = (dma_addr_t)alloc_addr;
    return alloc_addr;
}

void free_dmalli_space(struct hiedmac_tsf_info* tsf_info)
{
    struct hiedmac_host* hiedmac = NULL;
    dma_addr_t* alloc_addr = NULL;

    hiedmac = &hiedmac_driver_data[tsf_info->host_index];
    alloc_addr = (dma_addr_t*)hiedmac->dma_lliaddr[tsf_info->channel];

    hiedma_debug("%s,%d,alloc_addr= 0x%x\n",__func__,__LINE__,alloc_addr);
    if (alloc_addr)
        free(alloc_addr);
    hiedmac->dma_lliaddr[tsf_info->channel] = 0;
}

/*
 * config register for memory to memory DMA tranfer without LLI
 * note:
 * the channel is not actually started yet, you need to call
 * dmac_channelstart to start it up.
 */
int hiedmac_start_m2m(struct hiedmac_tsf_info* tsf_info,
        AARCHPTR psource,
        AARCHPTR pdest,
        unsigned int uwnumtransfers)
{
    struct hiedmac_host* hiedmac;

    hiedmac = &hiedmac_driver_data[tsf_info->host_index];
    if (uwnumtransfers > HIEDMAC_TRANS_MAXSIZE || uwnumtransfers == 0) {
        hiedma_err("Invalidate transfer size,size=%x\n", uwnumtransfers);
        return -EINVAL;
    }
    hiedma_debug("dmac[%d],channel[%d],source=0x%lx,dest=0x%lx,length=%d\n",
            tsf_info->host_index, tsf_info->channel, psource, pdest, uwnumtransfers);

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_L(tsf_info->channel), psource & 0xffffffff);
#ifdef LOSCFG_AARCH64
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_H(tsf_info->channel), (psource >> 32) & 0xffffffff);
#else
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_H(tsf_info->channel), (0));
#endif

    hiedma_debug("HIEDMAC_Cx_SRC_ADDR_H = 0x%x\n",hiedmac_readl(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_H(tsf_info->channel)));
    hiedma_debug("HIEDMAC_Cx_SRC_ADDR_L = 0x%x\n",hiedmac_readl(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_L(tsf_info->channel)));

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_L(tsf_info->channel), pdest & 0xffffffff);
#ifdef LOSCFG_AARCH64
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_H(tsf_info->channel), (pdest >> 32) & 0xffffffff);
#else
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_H(tsf_info->channel), (0) );
#endif

    hiedma_debug("HIEDMAC_Cx_DEST_ADDR_H = 0x%x\n",hiedmac_readl(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_H(tsf_info->channel)));
    hiedma_debug("HIEDMAC_Cx_DEST_ADDR_L = 0x%x\n",hiedmac_readl(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_L(tsf_info->channel)));

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_LLI_L(tsf_info->channel), 0);
    hiedma_debug("HIEDMAC_Cx_LLI_L = 0x%x\n",hiedmac_readl(hiedmac->base + HIEDMAC_Cx_LLI_L(tsf_info->channel)));

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_CNT0(tsf_info->channel), uwnumtransfers);
    hiedma_debug("HIEDMAC_Cx_CNT0 = 0x%x\n",hiedmac_readl(hiedmac->base + HIEDMAC_Cx_CNT0(tsf_info->channel)));
    dsb();

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_CONFIG(tsf_info->channel), HIEDMAC_CxCONFIG_M2M);
    return 0;
}


static int hiedmac_wait(struct hiedmac_tsf_info* tsf_info)
{
    struct hiedmac_host* hiedmac;
    unsigned int ret = 0;
    long long flags;

    hiedmac = &hiedmac_driver_data[tsf_info->host_index];

    ret = dma_event_wait(hiedmac->wait_event, HIEDMA_EVENT_DONE | HIEDMA_EVENT_ERROR, DMA_EVENT_WAIT_DEF_TIME);
    if (ret == HIEDMA_EVENT_ERROR) {
        hiedma_err("wait event error!");
        return -DMAC_CHN_ERROR;
    } else if (ret == LOS_ERRNO_EVENT_READ_TIMEOUT){
        hiedma_err("wait event timeout!");
        return -DMAC_CHN_TIMEOUT;
    }

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_CONFIG(tsf_info->channel), HIEDMAC_Cx_DISABLE);
//    dma_spin_lock_irqsave(&hiedmac->lock, flags);
//    hiedmac->channel_status[tsf_info->channel] = DMAC_CHN_VACANCY;
//    dma_spin_unlock_irqrestore(&hiedmac->lock, flags);
    hiedma_debug("event finish!\n");
    return DMAC_CHN_SUCCESS;
}

int edmac_buildllim2m(struct hiedmac_tsf_info* tsf_info,
        AARCHPTR psource,
        AARCHPTR pdest,
        unsigned int totaltransfersize)
{
    dma_addr_t* alloc_addr = NULL;
    struct hiedmac_host* hiedmac = NULL;
    unsigned int lli_num = 0;
    unsigned int alloc_length = 0;
    unsigned int i = 0;
    AARCHPTR srcaddr,destaddr;
    dmac_lli  *plli = NULL;

    hiedmac = &hiedmac_driver_data[tsf_info->host_index];
    alloc_addr = (dma_addr_t*)(hiedmac->dma_lliaddr[tsf_info->channel]);
    if (alloc_addr == NULL) {
        hiedma_err("plli is NULL!\n");
        return -1;
    }
    plli = (dmac_lli *)alloc_addr;

    lli_num = totaltransfersize/(HIEDMAC_TRANS_MAXSIZE);
    if ((totaltransfersize % (HIEDMAC_TRANS_MAXSIZE)) != 0) {
        lli_num++;
    }
    memset(alloc_addr, 0x0, sizeof(dmac_lli) * lli_num);

    for (i = 0; i < lli_num; i++) {
        plli->next_lli = (AARCHPTR)alloc_addr + (long long)(i+1) * sizeof(dmac_lli);
        if(i < lli_num - 1) {
            plli->next_lli |= (long long)HIEDMAC_LLI_ENABLE;
            plli->count = HIEDMAC_TRANS_MAXSIZE;
        } else {
            plli->next_lli = 0;
            plli->count = totaltransfersize % HIEDMAC_TRANS_MAXSIZE;
        }

        plli->src_addr = psource;
        plli->dest_addr = pdest;
        plli->config = HIEDMAC_CxCONFIG_M2M_LLI;

        hiedma_debug("plli->src_addr = 0x%lx\n",plli->src_addr);
        hiedma_debug("plli->dest_addr = 0x%lx\n",plli->dest_addr);
        hiedma_debug("plli->config = 0x%lx\n",plli->config);
        hiedma_debug("plli->next_lli = 0x%lx\n",plli->next_lli);
        hiedma_debug("plli->count = 0x%lx\n",plli->count);
        psource += plli->count;
        pdest += plli->count;
        plli++;
    }
    hiedma_debug("(AARCHPTR)alloc_addr = 0x%x, (AARCHPTR)alloc_addr + (AARCHPTR)(sizeof(dmac_lli) * lli_num)= 0x%x\n",
            (AARCHPTR)alloc_addr, (AARCHPTR)alloc_addr + (AARCHPTR)(sizeof(dmac_lli) * lli_num));
    dma_cache_clean((AARCHPTR)alloc_addr,
            (AARCHPTR)alloc_addr + (AARCHPTR)(sizeof(dmac_lli) * lli_num));

    return 0;
}

static int edmac_start_lli(struct hiedmac_tsf_info* tsf_info)
{
    struct hiedmac_host* hiedmac;
    dmac_lli  *plli = NULL;

    hiedmac = &hiedmac_driver_data[tsf_info->host_index];

    plli = (dmac_lli*)hiedmac->dma_lliaddr[tsf_info->channel];
    if (plli == NULL) {
        hiedma_err("plli is null !\n");
        return -1;
    }

    hiedma_debug("plli.src_addr: 0x%llx\n", plli->src_addr);
    hiedma_debug("plli.dst_addr: 0x%llx\n", plli->dest_addr);
    hiedma_debug("plli.next_lli: 0x%llx\n", plli->next_lli);
    hiedma_debug("plli.count: 0x%d\n", plli->count);

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_LLI_L(tsf_info->channel), plli->next_lli&0xffffffff);

#ifdef LOSCFG_AARCH64
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_LLI_H(tsf_info->channel), (plli->next_lli>>32)&0xffffffff);
#else
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_LLI_H(tsf_info->channel), (0));
#endif

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_CNT0(tsf_info->channel), plli->count);

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_L(tsf_info->channel), plli->src_addr&(long long)0xffffffff);
#ifdef LOSCFG_AARCH64
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_H(tsf_info->channel), (plli->src_addr>>32)&0xffffffff);
#else
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_H(tsf_info->channel), (0));
#endif

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_L(tsf_info->channel), plli->dest_addr&0xffffffff);
#ifdef LOSCFG_AARCH64
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_H(tsf_info->channel), (plli->dest_addr>>32)&0xffffffff);
#else
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_H(tsf_info->channel), (0));
#endif

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_CONFIG(tsf_info->channel), plli->config | HIEDMAC_CxCONFIG_CHN_START);
    return 0;
}

int hiedmac_start_m2p(struct hiedmac_tsf_info* tsf_info,
        AARCHPTR memaddr,
        unsigned int numtransfers,
        unsigned int next_lli_addr)
{
    struct hiedmac_host* hiedmac;
    unsigned int width;

    hiedmac = &hiedmac_driver_data[tsf_info->host_index];

    width = g_peripheral[tsf_info->peripheralid].transfer_width;

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_L(tsf_info->channel),
            memaddr & 0xffffffff);
#ifdef LOSCFG_AARCH64
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_H(tsf_info->channel),
            (memaddr >> 32) & 0xffffffff);
#else
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_H(tsf_info->channel), 0);
#endif
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_L(tsf_info->channel),
            g_peripheral[tsf_info->peripheralid].peri_addr & 0xffffffff);
#ifdef LOSCFG_AARCH64
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_H(tsf_info->channel),
            (g_peripheral[tsf_info->peripheralid].peri_addr >> 32) & 0xffffffff);
#else
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_H(tsf_info->channel), 0);
#endif

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_LLI_L(tsf_info->channel), 0);

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_CNT0(tsf_info->channel), numtransfers);

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_CONFIG(tsf_info->channel),
            g_peripheral[tsf_info->peripheralid].transfer_cfg | width << EDMA_SRC_WIDTH_OFFSET
            | (g_peripheral[tsf_info->peripheralid].dynamic_periphery_num << PERI_ID_OFFSET)
            | EDMA_CH_ENABLE);
    return 0;
}

int hiedmac_start_p2m(struct hiedmac_tsf_info* tsf_info,
        AARCHPTR memaddr,
        unsigned int numtransfers,
        unsigned int next_lli_addr)
{
    struct hiedmac_host* hiedmac;
    unsigned int width;

    hiedmac = &hiedmac_driver_data[tsf_info->host_index];

    width = g_peripheral[tsf_info->peripheralid].transfer_width;

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_L(tsf_info->channel),
            g_peripheral[tsf_info->peripheralid].peri_addr & 0xffffffff);
#ifdef LOSCFG_AARCH64
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_H(tsf_info->channel),
            (g_peripheral[tsf_info->peripheralid].peri_addr >> 32) & 0xffffffff);
#else
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_SRC_ADDR_H(tsf_info->channel), 0);
#endif

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_L(tsf_info->channel),
            memaddr & 0xffffffff);
#ifdef LOSCFG_AARCH64
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_L(tsf_info->channel),
            (memaddr >> 32) & 0xffffffff);
#else
    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_DEST_ADDR_L(tsf_info->channel), 0);
#endif

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_LLI_L(tsf_info->channel), 0);

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_CNT0(tsf_info->channel), numtransfers);

    hiedmac_writel(hiedmac->base + HIEDMAC_Cx_CONFIG(tsf_info->channel),
            g_peripheral[tsf_info->peripheralid].transfer_cfg | width << EDMA_SRC_WIDTH_OFFSET
            | (g_peripheral[tsf_info->peripheralid].dynamic_periphery_num << PERI_ID_OFFSET)
            | EDMA_CH_ENABLE);
    return 0;
}

int get_edma_host_nu(unsigned int peripheralid)
{
    unsigned int host_sel = 0;
    host_sel = g_peripheral[peripheralid].host_sel;
    if (host_sel == DMAC_HOST0)
        return 0;
    else if (host_sel == DMAC_HOST1)
        return 1;
    else
        hiedma_err("value is not true!\n");
    return -1;
}

static int get_channel_by_host_id(unsigned int host_id)
{
    struct hiedmac_host* hiedmac;
    long long flags;
    unsigned int i;

    hiedmac = &hiedmac_driver_data[host_id];
    for(i = 0; i < EDMAC_CHANNEL_NUM; i++)
    {
        if (hiedmac->channel_status[i] ==  DMAC_CHN_VACANCY) {
            dma_spin_lock_irqsave(&hiedmac->lock, flags);
            hiedmac->channel_status[i] = DMAC_CHN_ALLOCAT;
            dma_spin_unlock_irqrestore(&hiedmac->lock, flags);
            break;
        }
    }
    if (i != EDMAC_CHANNEL_NUM)
        return i;
    else
        return -1;

}

static struct hiedmac_tsf_info*  edmac_channel_allocate(int type, unsigned int peripheralid)
{
    struct hiedmac_host* hiedmac;
    int i = 0, j = 0;
    int ret = -1;
    struct hiedmac_tsf_info* tsf_info = NULL;
    int host_index = 0;

    if (type == TRASFER_TYPE_P2M  || type == TRASFER_TYPE_M2P) {
        ret = get_edma_host_nu(peripheralid);
        if(ret < 0) {
            hiedma_err("get host id fail\n");
            return NULL;
        }
        host_index = ret;
        ret = get_channel_by_host_id(host_index);
        if (ret < 0) {
            hiedma_err("get available channel fail\n");
            return NULL;
        } else {
            hiedmac = &hiedmac_driver_data[host_index];
            tsf_info = &hiedmac->cur_tsf[ret];
            tsf_info->channel = ret;
            tsf_info->host_index = host_index;
            hiedma_debug("%s,%d,tsf_info->channel = %d\n ",__func__,__LINE__,tsf_info->channel);
        }
    } else {
        for (i = 0; i < HIEDMAC_NUM; i++) {
            ret = get_channel_by_host_id(i);
            if (ret >= 0)
                break;
        }
        if (ret != -1) {
            hiedmac = &hiedmac_driver_data[i];
            tsf_info = &hiedmac->cur_tsf[ret];
            tsf_info->host_index = i;
            tsf_info->channel = ret;
            hiedma_debug("%s,%d,tsf_info->channel = %d\n ",__func__,__LINE__,tsf_info->channel);
        } else {
            hiedma_err("get available channel fail\n");
            return NULL;
        }
    }

    hiedma_debug("ret = %d,tsf_info->host_index = 0x%x, channel = %d\n",
            ret,tsf_info->host_index,tsf_info->channel);

    return tsf_info;
}

int edmac_llim2m_transfer(AARCHPTR source,
        AARCHPTR dest,
        unsigned int length)
{
    struct hiedmac_tsf_info* tsf_info = NULL;
    unsigned int left_size, dma_count = 0, dma_size = 0;
    int ret = 0;
    dma_addr_t*  alloc_addr = NULL;

    left_size = length;

    tsf_info = edmac_channel_allocate(TRASFER_TYPE_M2M, PERIPHERALID_INVILD);
    if (!tsf_info) {
        hiedma_err("allocate channel fail\n");
        return -1;
    }

    tsf_info->callback_data = NULL;
    tsf_info->transfer_type = TRASFER_TYPE_M2M;

    alloc_addr = allocate_dmalli_space(tsf_info, left_size);
    if (!alloc_addr) {
        hiedma_err("malloc for lli fail!\n");
        ret = -1;
        goto free_channel;
    }

    dma_cache_clean(source, source + length);
    dma_cache_inv(dest, dest + length);

    edmac_register_isr(tsf_info, &m2m_callback);

    ret = edmac_buildllim2m(tsf_info, source, dest, length);
    if (ret) {
        hiedma_err("build lli error...\n");
        ret = -EIO;
        goto free_lli;
    }

    ret = edmac_start_lli(tsf_info);
    if (ret) {
        hiedma_err("start lli error...\n");
        ret = -EIO;
        goto free_lli;
    }

    if (hiedmac_wait(tsf_info) != DMAC_CHN_SUCCESS) {
        hiedma_err("dma transfer error...\n");
        ret = -1;
    }

free_lli:
    free_dmalli_space(tsf_info);

free_channel:
    edmac_channel_free(tsf_info);
    return ret;
}

int dmac_m2m_transfer(AARCHPTR source,
        AARCHPTR dest,
        unsigned int length)
{
    struct hiedmac_tsf_info* tsf_info = NULL;
    unsigned int left_size, dma_count = 0, dma_size = 0;
    int ret = 0;

    tsf_info = edmac_channel_allocate(TRASFER_TYPE_M2M, PERIPHERALID_INVILD);
    if (!tsf_info) {
        hiedma_err("allocate channel fail\n");
        return -1;
    }

    tsf_info->callback_data = NULL;
    tsf_info->transfer_type = TRASFER_TYPE_M2M;
    dma_cache_clean(source, source + length);
    dma_cache_inv(dest, dest + length);

    left_size = length;

    edmac_register_isr(tsf_info, &m2m_callback);
    while (left_size) {
        if (left_size >= HIEDMAC_TRANS_MAXSIZE)
            dma_size = HIEDMAC_TRANS_MAXSIZE;
        else
            dma_size = left_size;

        hiedmac_start_m2m(tsf_info,
                source + dma_count * HIEDMAC_TRANS_MAXSIZE,
                dest + dma_count * HIEDMAC_TRANS_MAXSIZE,
                dma_size);

        if (hiedmac_wait(tsf_info) != DMAC_CHN_SUCCESS) {
            hiedma_err("dma transfer error...\n");
            edmac_channel_free(tsf_info);
            return -1;
        }
        left_size -= dma_size;
        dma_count++;
    }
    edmac_channel_free(tsf_info);
    return 0;
}

static void periphera_callback(int host_index, unsigned int dma_chn, void* callback_data)
{
    struct hiedmac_host* hiedmac = NULL;
    struct hiedmac_tsf_info* tsf_info = NULL;
    long long flags;
    hiedmac = &hiedmac_driver_data[host_index];

    tsf_info = &hiedmac->cur_tsf[dma_chn];
    if (tsf_info->callback) {
        tsf_info->callback(callback_data,  hiedmac->channel_status[dma_chn]);
        free_dmalli_space(tsf_info);
        edmac_channel_free(tsf_info);
    }
}

int edmac_buildlli(struct hiedmac_tsf_info* tsf_info,
        AARCHPTR memaddr,
        unsigned int totaltransfersize)
{
    dma_addr_t* alloc_addr = NULL;
    struct hiedmac_host* hiedmac = NULL;
    unsigned int lli_num = 0;
    unsigned int alloc_length = 0;
    unsigned int i = 0;
    unsigned int width = 0;
    AARCHPTR srcaddr,dstaddr;
    dmac_lli  *plli = NULL;

    hiedma_debug("memaddr = 0x%x\n", memaddr);
    hiedmac = &hiedmac_driver_data[tsf_info->host_index];
    alloc_addr = (dma_addr_t*)(hiedmac->dma_lliaddr[tsf_info->channel]);
    if (alloc_addr == NULL) {
        hiedma_err("plli is NULL!\n");
        return -1;
    }
    plli = (dmac_lli *)alloc_addr;

    lli_num = totaltransfersize/(HIEDMAC_TRANS_MAXSIZE);
    if ((totaltransfersize % (HIEDMAC_TRANS_MAXSIZE)) != 0) {
        lli_num++;
    }
    memset(alloc_addr, 0x0, sizeof(dmac_lli) * lli_num);

    if (tsf_info->transfer_type == TRASFER_TYPE_P2M) {
        srcaddr = g_peripheral[tsf_info->peripheralid].peri_addr;
        dstaddr = memaddr;
        hiedma_debug("%s,%d,P2M, srcaddr = 0x%x, dstaddr = 0x%x\n", __func__,__LINE__,srcaddr, dstaddr);
    } else if (tsf_info->transfer_type == TRASFER_TYPE_M2P){
        srcaddr = memaddr;
        dstaddr = g_peripheral[tsf_info->peripheralid].peri_addr;
        hiedma_debug("%s,%d,M2P, srcaddr = 0x%x, dstaddr = 0x%x\n", __func__,__LINE__,srcaddr, dstaddr);
    } else {
        hiedma_err("Illegal parameters in transfer_type!\n");
        return -1;
    }
    width = g_peripheral[tsf_info->peripheralid].transfer_width;

    for (i = 0; i < lli_num; i++) {

        plli->next_lli = (AARCHPTR)alloc_addr + (long long)(i+1) * sizeof(dmac_lli);
        if(i < lli_num - 1) {
            plli->next_lli |= (long long)HIEDMAC_LLI_ENABLE;
            plli->count = HIEDMAC_TRANS_MAXSIZE;
        } else {
            plli->next_lli = 0;
            plli->count = (totaltransfersize % HIEDMAC_TRANS_MAXSIZE);
        }

        plli->src_addr = (long long)srcaddr;
        plli->dest_addr = (long long)dstaddr;
        plli->config = g_peripheral[tsf_info->peripheralid].transfer_cfg
            | width << EDMA_SRC_WIDTH_OFFSET
            | g_peripheral[tsf_info->peripheralid].dynamic_periphery_num << PERI_ID_OFFSET;

        hiedma_debug("plli->src_addr = 0x%llx\n",plli->src_addr);
        hiedma_debug("plli->dest_addr = 0x%llx\n",plli->dest_addr);
        hiedma_debug("plli->next_lli = 0x%llx\n",plli->next_lli);
        hiedma_debug("plli->config = 0x%x\n",plli->config);
        hiedma_debug("plli->count = 0x%x\n",plli->count);

        if (TRASFER_TYPE_P2M == tsf_info->transfer_type)
            dstaddr += plli->count;
        else if (TRASFER_TYPE_M2P == tsf_info->transfer_type)
            srcaddr += plli->count;

        plli++;
    }
    hiedma_debug("(AARCHPTR)alloc_addr = 0x%x, (AARCHPTR)alloc_addr + (AARCHPTR)(sizeof(dmac_lli) * lli_num)= 0x%x\n",
            (AARCHPTR)alloc_addr, (AARCHPTR)alloc_addr + (AARCHPTR)(sizeof(dmac_lli) * lli_num));
    dma_cache_clean((AARCHPTR)alloc_addr,
            (AARCHPTR)alloc_addr + (AARCHPTR)(sizeof(dmac_lli) * lli_num));

    return 0;

}

int do_dma_m2p(AARCHPTR memaddr, unsigned int peri_addr, unsigned int length, PERI_REG_ISR* pisr, void* callback_data)
{
    unsigned int dma_size = 0, left_size = length;
    unsigned int dma_count = 0;
    int ret = 0;
    struct hiedmac_tsf_info* tsf_info = NULL;
    dma_addr_t*  alloc_addr = NULL;
    int peripheralid = 0;

#if 0
    tsf_info = malloc(sizeof(struct hiedmac_tsf_info));
    if (tsf_info == NULL) {
        hiedma_err("malloc tsf_info fail !\n");
        return -1;
    }
#endif

    ret = edmac_check_request(peri_addr, EDMAC_TX);
    if (ret < 0) {
        hiedma_err("p2m:Invalid devaddr.\n");
        ret = -1;
        goto end;
    }
    peripheralid = ret;
    hiedma_debug("peripheralid = %d\n", peripheralid);

    tsf_info = edmac_channel_allocate(TRASFER_TYPE_M2P, peripheralid);
    if (!tsf_info) {
        hiedma_err("allocate channel fail\n");
        return -1;
    }
    hiedma_debug("memaddr = %x,peri_addr=%x,length= %d \n",memaddr,peri_addr,length);
    tsf_info->transfer_type = TRASFER_TYPE_M2P;
    tsf_info->callback_data = callback_data;
    tsf_info->callback = pisr;
    tsf_info->peripheralid = peripheralid;

    alloc_addr = allocate_dmalli_space(tsf_info, left_size);
    if (!alloc_addr) {
        hiedma_err("malloc for lli fail!\n");
        ret = -1;
        goto free_channel;
    }

    dma_cache_clean(memaddr, memaddr + length);

    edmac_register_isr(tsf_info, &periphera_callback);

    ret = edmac_buildlli(tsf_info, memaddr, length);
    if (ret) {
        hiedma_err("build lli error...\n");
        ret = -EIO;
        goto free_lli;
    }

    ret = edmac_start_lli(tsf_info);
    if (ret) {
        hiedma_err("start lli error...\n");
        ret = -EIO;
        goto free_lli;
    }
    return 0;
    //dmac_start_llim2p(&tsf_info,);
#if 0
    while (left_size) {
        if (left_size >= HIEDMAC_TRANS_MAXSIZE)
            dma_size = HIEDMAC_TRANS_MAXSIZE;
        else
            dma_size = left_size;

        hiedmac_start_m2p(&tsf_info,
                 memaddr + dma_count * HIEDMAC_TRANS_MAXSIZE,
                 dma_size, 0);

        if (hiedmac_wait(&tsf_info) != DMAC_CHN_SUCCESS) {
            hiedma_err("dma transfer error...\n");
            edmac_channel_free(&tsf_info);
            return -1;
        }
        left_size -= dma_size;
        dma_count++;
    }
#endif

free_lli:
    free_dmalli_space(tsf_info);
free_channel:
    edmac_channel_free(tsf_info);
end:
    return ret;
}

unsigned int edmac_list_write_pointer(unsigned int peri_addr,unsigned int channel)
{
	unsigned int wp;
	struct hiedmac_tsf_info* tsf_info = NULL;
	struct hiedmac_host* hiedmac = NULL;
	tsf_info = get_trasfer_by_peripheraliaddr(peri_addr);
	hiedmac = &hiedmac_driver_data[tsf_info->host_index];
	hiedmac_readw(hiedmac->base + HIEDMAC_Cx_CURR_DEST_ADDR_L(channel), wp);
	return wp;
}

int edmac_uart_buildlli(struct hiedmac_tsf_info* tsf_info,
		AARCHPTR memaddr, unsigned int total_size)
{
	struct hiedmac_host* hiedmac = NULL;
	unsigned int lli_num = 0;
	dmac_lli  *plli = NULL;
	int i;
	int width = 0;
	AARCHPTR srcaddr,dstaddr;
	hiedmac = &hiedmac_driver_data[tsf_info->host_index];

	dma_addr_t *dma_list = (dma_addr_t*)(hiedmac->dma_lliaddr[tsf_info->channel]);
	if (dma_list == NULL) {
		hiedma_err("dma list buf is NULL!\n");
		return -1;
	}

	plli = (dmac_lli *)dma_list;
	lli_num = total_size/4;
	if ((total_size%4) != 0)
		lli_num++;

	memset(dma_list, 0x0, sizeof(dmac_lli) * lli_num);

	srcaddr = g_peripheral[tsf_info->peripheralid].peri_addr;
	dstaddr = memaddr;

	for (i = 0; i < lli_num; i++) {
		plli->next_lli = (AARCHPTR)dma_list + (unsigned int)(i+1) * sizeof(dmac_lli);
		if ( i == (lli_num - 1)) {
			plli->next_lli = (uint32_t)dma_list;
		}

		plli->next_lli |= (long long)HIEDMAC_LLI_ENABLE;
		plli->count = 4;
		plli->src_addr = srcaddr;
		plli->dest_addr = dstaddr;
		plli->config = g_peripheral[tsf_info->peripheralid].transfer_cfg
				| width << EDMA_SRC_WIDTH_OFFSET
				| g_peripheral[tsf_info->peripheralid].dynamic_periphery_num << PERI_ID_OFFSET;

		dstaddr += 4;
		plli++;
	}

	dma_cache_clean((AARCHPTR)dma_list, (AARCHPTR)dma_list + (sizeof(dmac_lli) * lli_num));

	return 0;
}

dma_addr_t *allocate_uart_dmalli_space(struct hiedmac_tsf_info* tsf_info, unsigned int length)
{
	unsigned int lli_num = 0;
	dma_addr_t *dma_list = NULL;
	struct hiedmac_host *hiedmac = NULL;
	unsigned int dma_list_sz = 0;

	lli_num = length/4;
	if ((length % 4) != 0)
		lli_num++;

//	hiedma_err("lli_num %d\n", lli_num);
	hiedmac = &hiedmac_driver_data[tsf_info->host_index];
	dma_list_sz = lli_num * sizeof(dmac_lli);
	dma_list_sz = ALIGN(dma_list_sz, CACHE_ALIGNED_SIZE);

	dma_list = (dma_addr_t*)memalign(256, dma_list_sz);
	if (!dma_list) {
		hiedma_err("dma list alloc failed!\n");
		return NULL;
	}
	memset(dma_list, 0, dma_list_sz);
	hiedmac->dma_lliaddr[tsf_info->channel] = (unsigned int)dma_list;

	dprintf("dmalist:%p\n", dma_list);
	//hiedma_err("poolsz:%x\n", dma_list_sz);
	return dma_list;
}

void free_uart_dmalli_space(struct hiedmac_tsf_info* tsf_info)
{

}

int edma_uart2memory(AARCHPTR memaddr, unsigned int peri_addr, unsigned int length)
{
	int ret = 0;
	int peripheralid = 0;
	struct hiedmac_tsf_info* tsf_info = NULL;
	dma_addr_t *dma_list = NULL;

	//hiedma_err("------ mem %p, peri_addr %p, len %x\n", memaddr, peri_addr, length);
	dprintf("ringbuf %p\n", memaddr);

	peripheralid = edmac_check_request(peri_addr, EDMAC_RX);
	if (peripheralid < 0) {
		hiedma_err("p2m:Invalid uart io address.\n");
		return -1;
	}

	//hiedma_err("id %d\n",peripheralid); 

	tsf_info = edmac_channel_allocate(TRASFER_TYPE_P2M, peripheralid);
	if (!tsf_info) {
		hiedma_err("uart allocate dma channel fail\n");
		return -1;
	}

	tsf_info->peripheralid = peripheralid;
	tsf_info->transfer_type = TRASFER_TYPE_P2M;

	//hiedma_err("id %d\n",peripheralid); 
	dma_list = allocate_uart_dmalli_space(tsf_info, length);
	if (!dma_list) {
		hiedma_err("malloc uart dmalli space failed!\n");
		edmac_channel_free(tsf_info);
		return -1;
	}

	dma_cache_inv(memaddr, memaddr + length);

	ret = edmac_uart_buildlli(tsf_info, memaddr, length);
	if (ret) {
		hiedma_err("build uart edmalli failed\n");
		free_uart_dmalli_space(tsf_info);
		edmac_channel_free(tsf_info);
		return -1;
	}

	ret = edmac_start_lli(tsf_info);
	if (ret) {
		hiedma_err("start uart edma failed!\n");
		free_uart_dmalli_space(tsf_info);
		edmac_channel_free(tsf_info);
		return -1;
	}

	return 0;
}

int do_dma_p2m(AARCHPTR memaddr, unsigned int peri_addr, unsigned int length, PERI_REG_ISR* pisr, void* callback_data)
{
    unsigned int dma_size = 0, left_size = 0;
    unsigned int dma_count = 0;
    int ret = 0;
    struct hiedmac_tsf_info* tsf_info = NULL;
    dma_addr_t*  alloc_addr = NULL;
    int peripheralid = 0;
#if 0
    tsf_info = malloc(sizeof(struct hiedmac_tsf_info));
    if (tsf_info == NULL) {
        hiedma_err("malloc tsf_info fail !\n");
        return -1;
    }
#endif

    ret = edmac_check_request(peri_addr, EDMAC_RX);
    if (ret < 0) {
        hiedma_err("p2m:Invalid devaddr.\n");
        ret = -1;
        goto end;
    }

    peripheralid = (unsigned int)ret;

    tsf_info = edmac_channel_allocate(TRASFER_TYPE_P2M, peripheralid);
    if (!tsf_info) {
        hiedma_err("allocate channel fail\n");
        return -1;
    }
    hiedma_debug("memaddr = %d,peri_addr=%d,length= %d \n",memaddr,peri_addr,length);
    left_size = length;
    tsf_info->transfer_type = TRASFER_TYPE_P2M;
    tsf_info->callback_data = callback_data;
    tsf_info->callback = pisr;
    tsf_info->peripheralid = peripheralid;

    alloc_addr = allocate_dmalli_space(tsf_info, left_size);
    if (!alloc_addr) {
        hiedma_err("malloc for lli fail!\n");
        ret = -1;
        goto free_channel;
    }

    dma_cache_inv(memaddr, memaddr + length);

    edmac_register_isr(tsf_info, &periphera_callback);

    ret = edmac_buildlli(tsf_info, memaddr, length);
    if (ret) {
        hiedma_err("build lli error...\n");
        ret = -EIO;
        goto free_lli;
    }

    ret = edmac_start_lli(tsf_info);
    if (ret) {
        hiedma_err("start lli error...\n");
        ret = -EIO;
        goto free_lli;
    }
    return 0;
#if 0
    while (left_size) {
        if (left_size >= HIEDMAC_TRANS_MAXSIZE)
            dma_size = HIEDMAC_TRANS_MAXSIZE;
        else
            dma_size = left_size;

        hiedmac_start_p2m(&tsf_info,
                 memaddr + dma_count * HIEDMAC_TRANS_MAXSIZE,
                 dma_size, 0);

        if (hiedmac_wait(&tsf_info) != DMAC_CHN_SUCCESS) {
            hiedma_err("dma transfer error...\n");
            edmac_channel_free(&tsf_info);
            return -1;
        }
        left_size -= dma_size;
        dma_count++;
    }
#endif
free_lli:
    free_dmalli_space(tsf_info);
free_channel:
    edmac_channel_free(tsf_info);
end:
    return ret;
}

/*
 *    init dmac register
 *    clear interupt flags
 *    called by dma_driver_init
 */
int dmac_init(struct platform_device *dev)
{
    unsigned int i, tempvalue;
    struct resource *res = NULL;
    struct hiedmac_host* hiedmac;
    int index = 0;
    int ret = 0;

    index = dev->id;

    res = platform_get_resource(dev, IORESOURCE_MEM, 0);
    if(NULL == res)
    {
        hiedma_err("platform_get_resource IORESOURCE_MEM fail!\n");
        return -1;
    }

    hiedmac = &hiedmac_driver_data[index];

    hiedmac->index = index;
    hiedma_debug("hiedmac->index = %d\n",hiedmac->index);

    ret = hiedmac_priv_init(hiedmac, (edmac_peripheral*)&g_peripheral);
    if (ret < 0) {
        hiedma_debug("fail:ret  = %d\n", ret);
        return -ENODEV;
    }

    hiedmac->lock = 0;
    dma_spin_lock_init(&hiedmac->lock);
    hiedmac->base = (unsigned long)platform_ioremap_resource(res);
    hiedmac->irq = platform_get_irq(dev, 0);
    if(LOS_NOK == hiedmac->irq)
    {
        hiedma_err("platform_get_irq fail!\n");
        return -1;
    }
    hiedmac->channel_num = EDMAC_CHANNEL_NUM;
    hiedmac_clk_en(hiedmac->index);

    hiedmac_unreset(hiedmac->index);

    hiedmac_writel(hiedmac->base + HIEDMAC_INT_TC1_RAW, HIEDMAC_ALL_CHAN_CLR);
    hiedmac_writel(hiedmac->base + HIEDMAC_INT_TC2_RAW, HIEDMAC_ALL_CHAN_CLR);

    hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR1_RAW, HIEDMAC_ALL_CHAN_CLR);
    hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR2_RAW, HIEDMAC_ALL_CHAN_CLR);
    hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR3_RAW, HIEDMAC_ALL_CHAN_CLR);

    hiedmac_writel(hiedmac->base + HIEDMAC_INT_TC1_MASK, HIEDMAC_INT_ENABLE_ALL_CHAN);
    hiedmac_writel(hiedmac->base + HIEDMAC_INT_TC2_MASK, HIEDMAC_INT_ENABLE_ALL_CHAN);

    hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR1_MASK, HIEDMAC_INT_ENABLE_ALL_CHAN);
    hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR2_MASK, HIEDMAC_INT_ENABLE_ALL_CHAN);
    hiedmac_writel(hiedmac->base + HIEDMAC_INT_ERR3_MASK, HIEDMAC_INT_ENABLE_ALL_CHAN);

    for (i = 0; i < hiedmac->channel_num; i++) {
        hiedmac_writel(hiedmac->base + HIEDMAC_Cx_CONFIG(i), HIEDMAC_Cx_DISABLE);
        hiedmac->function[i] = NULL;
        dma_event_init(&(hiedmac->wait_event[i]));

        hiedmac->dma_lliaddr[i] = (dma_addr_t)NULL;
        hiedmac->channel_status[i] = DMAC_CHN_VACANCY;
    }

    ret = request_irq(hiedmac->irq, (irq_handler_t)edmac_isr, (unsigned long)0, "hiedma", hiedmac);
    if (ret) {
        hiedma_err("DMA Irq %d request failed, ret = %d\n", hiedmac->irq,ret);
        goto out;
    }
    return 0;
out:
    return -1;
}

int dmac_remove(struct platform_device *dev)
{
    return 0;
}

static int hiedmac_suspend(struct device *dev)
{
    struct platform_device *pdev = to_platform_device(dev);

    return 0;
}

static int hiedmac_resume(struct device *dev)
{
    struct platform_device *pdev = to_platform_device(dev);

    return 0;
}

static const struct dev_pm_op hiedmac_dev_pm_ops = {
    .suspend = hiedmac_suspend,
    .resume = hiedmac_resume,
};

/**/
static int hiedmac_platform_probe(struct platform_device *dev)
{
    return dmac_init(dev);
}

static int hiedmac_platform_remove(struct platform_device *dev)
{
    //return hiedmac_remove(dev);
    return 0;
}

static struct platform_driver hiedmac_driver = {
    .probe      = hiedmac_platform_probe,
    .remove     = hiedmac_platform_remove,
    .driver     = {
        .name   = EDMAC_DRIVER_NAME,
        .pm = &hiedmac_dev_pm_ops,
    },
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

int /*__init*/ hiedmac_platform_init(void)
{
    int ret = 0;
    ret = platform_driver_register(&hiedmac_driver);
    if (ret) {
        hiedma_err("register platform driver failed!");
        goto _error_register_device;
    }
    return ret;


_error_register_device:

    return -1;
}

void /*__exit*/ hiedmac_platform_exit(void)
{
    platform_driver_unregister(&hiedmac_driver);
}

module_init(hiedmac_platform_init);
module_exit(hiedmac_platform_exit);

/*end of file hi_dmac.c*/

//////////////////////////////
///just used for debug
//////////////////////////////
int hiedmac_init(void)
{
    return hiedmac_platform_init();
}

int hiedmac_remove(void)
{
    hiedmac_platform_exit();

    return 0;
}

