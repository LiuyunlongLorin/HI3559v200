/*core.h begin*/
#ifndef _MMC_HOST_H
#define _MMC_HOST_H

#include "mmc/mmc_os_adapt.h"
#include "mmc/card.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef DISABLE
#define DISABLE                    0
#endif
#ifndef ENABLE
#define ENABLE                    1
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

# ifndef false
#  define false     (bool)0
# endif
# ifndef true
#  define true      (bool)1
# endif

enum bus_mode {
    MMC_BUSMODE_OPENDRAIN,
    MMC_BUSMODE_PUSHPULL
};

enum signal_volt {
    SIGNAL_VOLT_3V3 = 0,
    SIGNAL_VOLT_1V8,
    SIGNAL_VOLT_1V2
};

struct mmc_card_list {
    struct mmc_card card;
    int slot_idx;
    struct mmc_card_list *next;
};

struct mmc_suspend_resume {
    int mmc_max_num;
    int flags[5];
#define MMC_UNDO_DETECT      (0<<0)
#define MMC_DO_DETECT        (1<<0)
};

/* mmc host control describe */
struct mmc_host {
    int         idx;
    uint32_t    freq_min;
    uint32_t    freq_max;
    uint32_t    freq_default;
    enum signal_volt volt_default;
    unsigned int    dvdd;
    union mmc_ocr   ocr_default;    /* ocr default */
    /* host correlation */
    union {
        uint32_t caps_data;
        struct caps_bits_data {
            uint32_t cap_4_bit : 1;             /* bit:0 support 4 bit transfer */
            uint32_t cap_8_bit : 1;             /* bit:1 support 8 bit transfer */
            uint32_t cap_mmc_highspeed : 1;     /* bit:2 support mmc high-speed timing */
            uint32_t cap_sd_highspeed : 1;      /* bit:3 support SD high-speed timing */
            uint32_t cap_sdio_irq : 1;          /* bit:4 signal pending SDIO irqs */
            uint32_t cap_only_spi : 1;          /* bit:5 only support spi protocols */
            uint32_t cap_need_poll : 1;         /* bit:6 need polling for card-detection */
            uint32_t cap_nonremovable : 1;      /* bit:7 Nonremoveable eg. eMMC */
            uint32_t cap_wait_while_busy : 1;   /* bit:8 waits while card is busy */
            uint32_t cap_erase : 1;             /* bit:9 allow erase */
            uint32_t cap_1v8_ddr : 1;           /* bit:10 support ddr mode at 1.8V */
            uint32_t cap_1v2_ddr : 1;           /* bit:11 support ddr mode at 1.2V */
            uint32_t cap_power_off_card : 1;    /* bit:12 support power off after boot */
            uint32_t cap_bus_width_test : 1;    /* bit:13 CMD14/CMD19 bus width ok */
            uint32_t cap_UHS_SDR12 : 1;         /* bit:14 support UHS SDR12 mode */
            uint32_t cap_UHS_SDR25 : 1;         /* bit:15 support UHS SDR25 mode */
            uint32_t cap_UHS_SDR50 : 1;         /* bit:16 support UHS SDR50 mode */
            uint32_t cap_UHS_SDR104 : 1;        /* bit:17 support UHS SDR104 mode */
            uint32_t cap_UHS_DDR50 : 1;         /* bit:18 support UHS DDR50 mode */
            uint32_t cap_XPC_330 : 1;           /* bit:19 support >150mA current at 3.3V */
            uint32_t cap_XPC_300 : 1;           /* bit:20 support >150mA current at 3.0V */
            uint32_t cap_XPC_180 : 1;           /* bit:21 support >150mA current at 1.8V */
            uint32_t cap_driver_type_A : 1;     /* bit:22 support driver type A */
            uint32_t cap_driver_type_C : 1;     /* bit:23 support driver type C */
            uint32_t cap_driver_type_D : 1;     /* bit:24 support driver type D */
            uint32_t cap_max_current_200 : 1;   /* bit:25 max current limit 200mA */
            uint32_t cap_max_current_400 : 1;   /* bit:26 max current limit 400mA */
            uint32_t cap_max_current_600 : 1;   /* bit:27 max current limit 600mA */
            uint32_t cap_max_current_800 : 1;   /* bit:28 max current limit 800mA */
            uint32_t cap_CMD23 : 1;             /* bit:29 support CMD23 */
            uint32_t cap_hw_reset : 1;          /* bit:30 support hardware reset */
            uint32_t reserved: 1;               /* bit:31 reserverd */
        } bits;
    }caps;
    union {
        uint32_t caps2_data;
        struct caps2_bits_data {
            uint32_t caps2_bootpart_noacc : 1;    /* bit:0 boot partition no access */
            uint32_t caps2_cache_ctrl : 1;        /* bit:1 allow cache control */
            uint32_t caps2_poweroff_notify : 1;   /* bit:2 support notify power off*/
            uint32_t caps2_no_multi_read : 1;     /* bit:3 not support multiblock read*/
            uint32_t caps2_no_sleep_cmd : 1;      /* bit:4 not support sleep command*/
            uint32_t caps2_HS200_1v8_SDR : 1;     /* bit:5 support*/
            uint32_t caps2_HS200_1v2_SDR : 1;     /* bit:6 support*/
            uint32_t caps2_broken_voltage : 1;    /* bit:7 use broken voltage */
            uint32_t caps2_detect_no_err : 1;     /* bit:8 I/O err check card removal*/
            uint32_t caps2_HC_erase_size : 1;     /* bit:9 High-capacity erase size*/
            uint32_t caps2_speedup_enable : 1;
            uint32_t reserved : 22;               /* bits:10~31, reserved */
        } bits;
    } caps2;

    uint32_t        max_blk_num;                  /* max number of blocks in one req */
    uint32_t        max_blk_size;                 /* max size of one mmc block */
    uint32_t        max_request_size;

    bool   is_mmc_detect_dis;
    struct mmc_card *card_cur;
    struct mmc_card_list *card_list;

#ifdef LOSCFG_DRIVERS_MMC_SPEEDUP
    unsigned int status;
#define MMC_HOST_OK (0<<0)
#define MMC_HOST_ERR (1<<0)
    unsigned int (*get_rdptr)(struct mmc_host *host);
#endif
    bool    is_sdio_irq_pending;
    void    *priv;
};

#define is_mmc_host_spi(host)    ((host)->caps.bits.cap_only_spi)

/* inline functions define for mmc_host */
static inline void *get_mmc_priv(struct mmc_host *host)
{
    return (void *)host->priv;
}
static inline void set_mmc_dis_detect(struct mmc_host *host, bool flag)
{
    host->is_mmc_detect_dis = flag;
}

struct mmc_host *get_mmc_host(uint32_t mmc_idx);

/* *
 * these functions can be called by users
 * */
extern int SD_MMC_Host_init(void);
extern int mmc_host_suspend(void *data);
extern int mmc_host_resume(void *data);
extern int mmc_priv_fn_register(unsigned int id, void *data, void *fn(struct mmc_card *card, void *data));

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _MMC_HOST_H */
