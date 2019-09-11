#ifndef __HI_MMC_H__
#define __HI_MMC_H__

#include "hisoc/mmc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#undef  SDHC_DUMP_REG

#define SDHC_PTHREAD_EVENT_TIMEOUT     LOS_WAIT_FOREVER

#define SDMCI_STACKSIZE     0x3000
#define SDMCI_ADMA_MAX_DESC    128
#define SDMCI_ADMA_DEF_SIZE   ((SDMCI_ADMA_MAX_DESC * 2 + 1) * 4)


#define SD_HCI_TUNINT_REQ_TIMEOUT  (LOSCFG_BASE_CORE_TICK_PER_SECOND / 20) // 50 ms
#define SD_HCI_REQUEST_TIMEOUT  (LOSCFG_BASE_CORE_TICK_PER_SECOND * 10) // 30 s  FIXME: Forever
#define SD_MCI_TUNINT_REQ_TIMEOUT (LOSCFG_BASE_CORE_TICK_PER_SECOND / 5) //  200ms



#define sdhci_crg_readl(addr) ({unsigned int reg = readl((unsigned int)addr); \
        /*mmc_trace(1, "readl(0x%04X) = 0x%08X", (unsigned int)addr, reg);*/ \
        reg; })

#define sdhci_crg_writel(v, addr) do { \
    writel(v, (unsigned int)addr); \
    /*mmc_trace(1, "writel(0x%04X) = 0x%08X",\
     *             (unsigned int)addr, (unsigned int)(v));*/ \
} while (0)

struct sdhci_next_data {
    unsigned int sg_count;
};

enum dma_data_direction {
    DMA_BIDIRECTIONAL = 0,
    DMA_TO_DEVICE = 1,
    DMA_FROM_DEVICE = 2,
    DMA_NONE = 3,
};

union host_priv_status {
    unsigned int priv_all_status;
    struct priv_status {
        unsigned char curr_detect_status;
#define CARD_IS_PLUGIN  1
#define CARD_IS_PLUGOUT 0
        unsigned char last_detect_status;
        unsigned short detect_change;
    }select;
};
union host_quirks {
    unsigned int quirks_data;
    struct quirks_bit_data {
        unsigned int quirk_clock_before_reset:1;
        unsigned int quirk_force_dma:1;
        unsigned int quirk_no_card_no_reset:1;
        unsigned int quirk_single_power_write:1;
        unsigned int quirk_reset_cmd_data_on_ios:1;
        unsigned int quirk_broken_dma:1;
        unsigned int quirk_broken_adma:1;
        unsigned int quirk_32bit_dma_addr:1;
        unsigned int quirk_32bit_dma_size:1;
        unsigned int quirk_32bit_adma_size:1;
        unsigned int quirk_reset_after_request:1;
        unsigned int quirk_no_simult_vdd_and_power:1;
        unsigned int quirk_broken_timeout_val:1;
        unsigned int quirk_broken_small_pio:1;
        unsigned int quirk_no_busy_irq:1;
        unsigned int quirk_broken_card_detection:1;
        unsigned int quirk_inverted_write_protect:1;
        unsigned int quirk_pio_needs_delay:1;
        unsigned int quirk_force_blk_sz_2048:1;
        unsigned int quirk_no_multiblock:1;
        unsigned int quirk_force_1_bit_data:1;
        unsigned int quirk_delay_after_power:1;
        unsigned int quirk_data_timeout_use_sdclk:1;
        unsigned int quirk_cap_clock_base_broken:1;
        unsigned int quirk_no_endattr_in_nopdesc:1;
        unsigned int quirk_missing_caps:1;
        unsigned int quirk_multiblock_read_acmd12:1;
        unsigned int quirk_no_hispd_bit:1;
        unsigned int quirk_broken_adma_zerolen_desc:1;
        unsigned int quirk_unstable_ro_detect:1;
    }bits;
};


union host_quirks2 {
    unsigned int quirks2_data;
    struct quirks2_bit_data {
        unsigned int quirk2_host_off_card_on:1;
        unsigned int quirk2_host_no_cmd23:1;
        unsigned int quirk2_no_1_8_v:1;
        unsigned int quirk2_preset_value_broken:1;
        unsigned int quirk2_cad_no_needs_bus_on:1;
        unsigned int quirk2_broken_host_control:1;
        unsigned int quirk2_broken_hs200:1;
        unsigned int quirk2_broken_ddr50:1;
        unsigned int quirk2_stop_with_tc:1;
        unsigned int quirk2_rdwr_tx_active_eot:1;
        unsigned int quirk2_slow_int_clr:1;
        unsigned int quirk2_always_use_base_clock:1;
        unsigned int quirk2_ignore_datatout_for_r1bcmd:1;
        unsigned int quirk2_broken_perset_value:1;
        unsigned int quirk2_use_reserved_max_timeout:1;
        unsigned int quirk2_divide_tout_by_4:1;
        unsigned int quirk2_ign_data_end_bit_error:1;
        unsigned int quirk2_adma_skip_data_alignment:1;
        unsigned int quirk2_nonstandard_clock:1;
        unsigned int quirk2_caps_bit63_for_hs400:1;
        unsigned int quirk2_use_reset_workaround:1;
        unsigned int quirk2_broken_led_control:1;
        unsigned int quirk2_non_standard_tuning:1;
        unsigned int quirk2_use_pio_for_emmc_tuning:1;
    }bits;
};

struct sdhc_host {
    union host_quirks quirks;
    union host_quirks2 quirks2;

    struct mmc_host        *mmc;

    struct mmc_request    *mrq;
    struct mmc_cmd    *cmd;
    struct mmc_data        *data;
    void            *base;

    struct scatterlist    *dma_sg;
    unsigned int    dma_sg_num;
    dma_addr_t      dma_paddr;
    unsigned int        *dma_vaddr;
    unsigned int    irq_num;
    unsigned int    irq_enable;

    int     id;
    mmc_sem     sem_id;
    mmc_mutex   thread_mutex;
    mmc_event   sdhci_event;
#define     SDHC_PEND_REQUEST_DONE     (1 << 0)
#define     SDHC_PEND_ACCIDENT         (1 << 1)

    unsigned long dma_mask;

    int flags;
#define SDHC_USE_SDMA  (1<<0)
#define SDHC_USE_ADMA  (1<<1)
#define SDHC_REQ_USE_DMA (1<<2)
#define SDHC_DEVICE_DEAD (1<<3)
#define SDHC_SDR50_NEEDS_TUNING (1<<4)
#define SDHC_NEEDS_RETUNING (1<<5)
#define SDHC_AUTO_CMD12 (1<<6)
#define SDHC_AUTO_CMD23 (1<<7)
#define SDHC_PV_ENABLED (1<<8)
#define SDHC_SDIO_IRQ_ENABLED (1<<9)
#define SDHC_SDR104_NEEDS_TUNING (1<<10)
#define SDHC_USING_RETUNING_TIMER (1<<11)
#define SDHC_USE_ADMA_64BIT  (1<<12)
#define SDHC_HOST_IRQ_STATUS  (1<<13)

    unsigned int version;

    unsigned int max_clk;
    unsigned int clk_mul;

    unsigned int clock;
    unsigned short pwr;

    bool runtime_suspended;
    bool bus_on;
    bool preset_enabled;

    unsigned int data_early:1;
    unsigned int busy_handle:1;

    int sg_count;

    char *adma_desc;
    char *align_buffer;

    unsigned int adma_desc_sz;
    unsigned int adma_desc_line_sz;
    unsigned int align_buf_sz;
    unsigned int align_bytes;
    unsigned int adma_max_desc;

    dma_addr_t adma_addr;
    dma_addr_t align_addr;

    mmc_timer timer;

    unsigned int caps; 
    unsigned int caps1;

    unsigned int            ocr_avail_sdio;
    unsigned int            ocr_avail_sd;
    unsigned int            ocr_avail_mmc;
    unsigned int ocr_mask;

    enum mmc_bus_timing timing;

    unsigned int thread_isr;

    wait_queue_head_t buf_ready_int;

    unsigned int  tuning_count;
    unsigned int  tuning_mode;
#define SDHC_TUNING_MODE_1 0

    struct sdhci_next_data next_data;

    bool is_crypto_en;
    bool crypto_reset_reqd;
    bool sdio_irq_async_status;

    unsigned int auto_cmd_err_sts;
    int reset_wa_applied;
    int reset_wa_cnt;
    union host_priv_status priv_status;
    int wait_for_event;

};


#define SDHC_MAX_DIV_SPEC_200 256
#define SDHC_MAX_DIV_SPEC_300 2046

/*
 * Host SDMA buffer boundary. Valid values from 4K to 512K in powers of 2.
 */
#define SDHC_DEFAULT_BOUNDARY_SIZE  (512 * 1024)
//#define SDHC_DEFAULT_BOUNDARY_ARG   (ilog2(SDHC_DEFAULT_BOUNDARY_SIZE) - 12)
#define SDHC_DEFAULT_BOUNDARY   19 // log2(SDHC_DEFAULT_BOUNDARY_SIZE)
#define SDHC_DEFAULT_BOUNDARY_ARG   (SDHC_DEFAULT_BOUNDARY - 12)


static inline void sdhc_writel(struct sdhc_host *host, unsigned int val, int reg)
{
    writel(val, (AARCHPTR)host->base + reg);
}

static inline void sdhc_writew(struct sdhc_host *host, unsigned short val, int reg)
{
    writew(val, (AARCHPTR)host->base + reg);
}

static inline void sdhc_writeb(struct sdhc_host *host, unsigned char val, int reg)
{
    writeb(val, (AARCHPTR)host->base + reg);
}

static inline unsigned int sdhc_readl(struct sdhc_host *host, int reg)
{
    return readl((AARCHPTR)host->base + reg);
}

static inline unsigned short sdhc_readw(struct sdhc_host *host, int reg)
{
    return readw((AARCHPTR)host->base + reg);
}

static inline unsigned char sdhc_readb(struct sdhc_host *host, int reg)
{
    return readb((AARCHPTR)host->base + reg);
}


unsigned int sdhci_set_clock(struct sdhc_host *host, unsigned int clock);
void sdhci_set_uhs_signaling(struct sdhc_host *host, enum mmc_bus_timing timing);
void sdhci_host_quirks(struct sdhc_host *host);
void sdhci_pltfm_init(struct sdhc_host *host);
void sdhci_hisi_assert_reset_dll(struct sdhc_host *host);
void sdhci_hisi_deassert_reset_dll(struct sdhc_host *host);
void sdhci_hisi_set_drv_dll(struct sdhc_host *host);
void sdhci_hisi_set_samp_phase(struct sdhc_host *host, unsigned int phase);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif

