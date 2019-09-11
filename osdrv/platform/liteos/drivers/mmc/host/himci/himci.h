#ifndef __HI_MMC_H__
#define __HI_MMC_H__

#include "hisoc/mmc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define FORCE_ENABLE     1
#define FORCE_DISABLE    0

enum power_status{
   HOST_POWER_OFF,
   HOST_POWER_ON
};

enum card_status{
    CARD_PLUGED = 0,
    CARD_UNPLUGED,
};

enum dma_data_direction {
    DMA_BIDIRECTIONAL = 0,
    DMA_TO_DEVICE = 1,
    DMA_FROM_DEVICE = 2,
    DMA_NONE = 3,
};

enum pre_detect {
   HOST_NOT_PRE_DETECT = 0,
   HOST_PRE_DETECTING = 1,
   HOST_PRE_DETECTED  = 2
};

/* HI MCI CONFIGS*/
#define HI_MCI_DETECT_TIMEOUT    (LOSCFG_BASE_CORE_TICK_PER_SECOND/2) /* 0.5s */
#define HI_MCI_REQUEST_TIMEOUT    (5 * LOSCFG_BASE_CORE_TICK_PER_SECOND) /* 5s */
#define HI_MCI_CARD_COMPLETE_TIMEOUT (1 * LOSCFG_BASE_CORE_TICK_PER_SECOND) /* 1s */
#define HI_MCI_TUNINT_REQ_TIMEOUT (LOSCFG_BASE_CORE_TICK_PER_SECOND / 5)  /* 0.2s */
#define MAX_RETRY_COUNT     100
#define HIMCI_PAGE_SIZE     4096
#define HIMCI_STACKSIZE     0x3000

#define himci_readl(addr) ({unsigned int reg = readl((unsigned int)addr); \
    /*mmc_trace(1, "readl(0x%04X) = 0x%08X", (unsigned int)addr, reg);*/ \
    reg; })

#define himci_writel(v, addr) do { \
    writel(v, (unsigned int)addr); \
    /*mmc_trace(1, "writel(0x%04X) = 0x%08X",\
            (unsigned int)addr, (unsigned int)(v));*/ \
} while (0)


struct himci_des {
    unsigned long idmac_des_ctrl;
    unsigned long idmac_des_buf_size;
    unsigned long idmac_des_buf_addr;
    unsigned long idmac_des_next_addr;
};

#ifdef LOSCFG_DRIVERS_MMC_SPEEDUP
#define ADMA_QUEUE_DEEPTH   (32)
struct himci_entire_des {
    unsigned long ctrl;
    unsigned long cmd_des_addr;
    unsigned long response;
    unsigned long reserved;
};
struct himci_cmd_des {
    unsigned long blk_sz;
    unsigned long byte_cnt;
    unsigned long arg;
    unsigned long cmd;
};
#endif

struct himci_host {
    struct mmc_host        *mmc;

    struct mmc_request    *mrq;
    struct mmc_cmd    *cmd;
    struct mmc_data        *data;
    void            *base;
    unsigned int card_status;
    enum power_status power_status;
    struct scatterlist    *dma_sg;
    unsigned int    dma_sg_num;
    dma_addr_t      dma_paddr;
    unsigned int        *dma_vaddr;
    enum dma_data_direction dma_dir;
    unsigned int    irq_num;
    unsigned int    irq_status;
    BOOL    is_tuning;
    //unsigned long       pending_events;
    int     id;
    mmc_sem     sem_id;
    mmc_mutex   thread_mutex;
    unsigned int port;
    mmc_event   himci_event;
#define    HIMCI_PEND_DTO_B     (0)
#define    HIMCI_PEND_DTO_M     (1 << HIMCI_PEND_DTO_B)
#ifdef LOSCFG_DRIVERS_MMC_SPEEDUP
#define    HIMCI_INTR_WAIT_B    (1)
#define    HIMCI_INTR_WAIT_M    (1 << HIMCI_INTR_WAIT_B)
#define    HIMCI_INTR_TIMEOUT   (HZ*2)
    dma_addr_t  cmd_paddr;
    struct himci_entire_des *wr_ent_des;
    struct himci_cmd_des    *wr_cmd_des;
    struct mmc_request  *pmrq[ADMA_QUEUE_DEEPTH];
    unsigned int    wr_pos;
#endif
};

union cmd_arg_s {
    unsigned int cmd_arg;
    struct cmd_bits_arg {
        unsigned int cmd_index:6;
        unsigned int response_expect:1;
        unsigned int response_length:1;
        unsigned int check_response_crc:1;
        unsigned int data_transfer_expected:1;
        unsigned int read_write:1;
        unsigned int transfer_mode:1;
        unsigned int send_auto_stop:1;
        unsigned int wait_prvdata_complete:1;
        unsigned int stop_abort_cmd:1;
        unsigned int send_initialization:1;
        unsigned int card_number:5;
        unsigned int update_clk_reg_only:1; /* bit 21 */
        unsigned int read_ceata_device:1;
        unsigned int ccs_expected:1;
        unsigned int enable_boot:1;
        unsigned int expect_boot_ack:1;
        unsigned int disable_boot:1;
        unsigned int boot_mode:1;
        unsigned int volt_switch:1;
        unsigned int use_hold_reg:1;
        unsigned int reserved:1;
        unsigned int start_cmd:1; /* HSB */
    } bits;
};

/* function declaration in board.c */
unsigned int hi_mci_clk_div(struct himci_host *host, unsigned int cclk);
void hi_mci_clock_cfg(struct himci_host *host);
void hi_mci_pad_ctrl_cfg(struct himci_host *host, enum signal_volt voltage);
int hi_mci_board_config(struct himci_host *host);
void himci_get_phase(struct himci_host *host, int raise_point,
        int fall_point);

void hi_mci_pre_detect_card(struct himci_host *host);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif

