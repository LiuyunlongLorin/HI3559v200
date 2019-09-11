/*mmc_card.h begin*/

#ifndef __MMC_CARD_H__
#define __MMC_CARD_H__

#include "mmc/host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

enum card_type {
    CARD_TYPE_UNKNOW = 0,
    CARD_TYPE_MMC,
    CARD_TYPE_SD,
    CARD_TYPE_SDIO,
    CARD_TYPE_COMBO,
};

enum mmc_vdd {
    VDD_1V5 = 0, VDD_1V55, VDD_1V6, VDD_1V65, VDD_1V7, VDD_1V8,
    VDD_1V9, VDD_2V0, VDD_2V1, VDD_2V2, VDD_2V3, VDD_2V4, VDD_2V5,
    VDD_2V6, VDD_2V7, VDD_2V8, VDD_2V9, VDD_3V0, VDD_3V1, VDD_3V2,
    VDD_3V3, VDD_3V4, VDD_3V5, VDD_3V6
};

enum mmc_power_mode {
    POWER_OFF = 0,
    POWER_UP,
    POWER_ON
};

enum mmc_chip_select {
    CS_DONTCARE = 0,
    CS_HIGH,
    CS_LOW
};

enum mmc_bus_width {
    BUS_WIDTH_1 = 0,
    BUS_WIDTH_4 = 2,
    BUS_WIDTH_8 = 3
};

enum mmc_bus_timing {
    TIMING_MMC_DS = 0,
    TIMING_MMC_HS,
    TIMING_SD_HS,
    TIMING_UHS_SDR12,
    TIMING_UHS_SDR25,
    TIMING_UHS_SDR50,
    TIMING_UHS_SDR104,
    TIMING_UHS_DDR50,
    TIMING_UHS_DDR52,
    TIMING_MMC_HS200,  /*for emmc*/
    TIMING_MMC_HS400,  /*for emmc*/
};

struct mmc_iocfg {
    uint32_t    clock;  /* Speed of the clock in Hz to move data */
    enum mmc_vdd    vdd;    /* Voltage to apply to the power pins/ */
    enum mmc_power_mode power_mode;
    enum mmc_chip_select chip_select;
    enum mmc_bus_width bus_width;
    enum mmc_bus_timing timing;
};

union card_state {
    unsigned int state_data;
    struct state_bits_data {
        unsigned int present : 1;
        unsigned int readonly : 1;
        unsigned int highspeed : 1;
        unsigned int blockaddr : 1;
        unsigned int ddr_mode : 1;
        unsigned int highspeed_ddr : 1;
        unsigned int ultra : 1;
        unsigned int sdxc : 1;
        unsigned int hs200 : 1;
        unsigned int sleep : 1;
        unsigned int removeable : 1;
        unsigned int not_std_sdio : 1;
        unsigned int blksz_for_byte_mode : 1;
        unsigned int reserverd : 19;
    } bits;
};

/* OCR bit definitions */
#define MMC_1V65_1V95      0x00000080    /* VDD 1.65V ~ 1.95V */
#define MMC_2V0_2V1        0x00000100    /* VDD 2.0V ~ 2.1V */
#define MMC_2V1_2V2        0x00000200    /* VDD 2.1V ~ 2.2V */
#define MMC_2V2_2V3        0x00000400    /* VDD 2.2V ~ 2.3V */
#define MMC_2V3_2V4        0x00000800    /* VDD 2.3V ~ 2.4V */
#define MMC_2V4_2V5        0x00001000    /* VDD 2.4V ~ 2.5V */
#define MMC_2V5_2V6        0x00002000    /* VDD 2.5V ~ 2.6V */
#define MMC_2V6_2V7        0x00004000    /* VDD 2.6V ~ 2.7V */
#define MMC_2V7_2V8        0x00008000    /* VDD 2.7V ~ 2.8V */
#define MMC_2V8_2V9        0x00010000    /* VDD 2.8V ~ 2.9V */
#define MMC_2V9_3V0        0x00020000    /* VDD 2.9V ~ 3.0V */
#define MMC_3V0_3V1        0x00040000    /* VDD 3.0V ~ 3.1V */
#define MMC_3V1_3V2        0x00080000    /* VDD 3.1V ~ 3.2V */
#define MMC_3V2_3V3        0x00100000    /* VDD 3.2V ~ 3.3V */
#define MMC_3V3_3V4        0x00200000    /* VDD 3.3V ~ 3.4V */
#define MMC_3V4_3V5        0x00400000    /* VDD 3.4V ~ 3.5V */
#define MMC_3V5_3V6        0x00800000    /* VDD 3.5V ~ 3.6V */
#define SD_OCR_S18R        0x01000000    /* 1.8V switching request */
#define SD_ROCR_S18A       SD_OCR_S18R   /* 1.8V switching accepted by card */
#define SD_OCR_XPC         0x10000000    /* SDXC power control */
#define SD_OCR_CCS         0x40000000    /* Card Capacity Status */
#define MMC_CARD_BUSY      0x80000000    /* Card Power up status bit */

/*ocr register describe */
union mmc_ocr {
    uint32_t    ocr_data;
    struct ocr_bits_data {
        uint32_t reserved0 : 4;
        uint32_t reserved1 : 1;
        uint32_t reserved2 : 1;
        uint32_t reserved3 : 1;

        uint32_t vdd_1v65_1v95 : 1;     /* bit:7  voltage 1.65 ~ 1.95 */
        uint32_t vdd_2v0_2v1 : 1;       /* bit:8  voltage 2.0 ~ 2.1 */
        uint32_t vdd_2v1_2v2 : 1;       /* bit:9  voltage 2.1 ~ 2.2 */
        uint32_t vdd_2v2_2v3 : 1;       /* bit:10  voltage 2.2 ~ 2.3 */
        uint32_t vdd_2v3_2v4 : 1;       /* bit:11  voltage 2.3 ~ 2.4 */
        uint32_t vdd_2v4_2v5 : 1;       /* bit:12  voltage 2.4 ~ 2.5 */
        uint32_t vdd_2v5_2v6 : 1;       /* bit:13  voltage 2.5 ~ 2.6 */
        uint32_t vdd_2v6_2v7 : 1;       /* bit:14  voltage 2.6 ~ 2.7 */
        uint32_t vdd_2v7_2v8 : 1;       /* bit:15  voltage 2.7 ~ 2.8 */
        uint32_t vdd_2v8_2v9 : 1;       /* bit:16  voltage 2.8 ~ 2.9 */
        uint32_t vdd_2v9_3v0 : 1;       /* bit:17  voltage 2.9 ~ 3.0 */
        uint32_t vdd_3v0_3v1 : 1;       /* bit:18  voltage 3.0 ~ 3.1 */
        uint32_t vdd_3v1_3v2 : 1;       /* bit:19  voltage 3.1 ~ 3.2 */
        uint32_t vdd_3v2_3v3 : 1;       /* bit:20  voltage 3.2 ~ 3.3 */
        uint32_t vdd_3v3_3v4 : 1;       /* bit:21  voltage 3.3 ~ 3.4 */
        uint32_t vdd_3v4_3v5 : 1;       /* bit:22  voltage 3.4 ~ 3.5 */
        uint32_t vdd_3v5_3v6 : 1;       /* bit:23  voltage 3.5 ~ 3.6 */
        uint32_t S18 : 1;               /* bit:24  switch to 1.8v accepted */
        uint32_t reserved4 : 2;
        uint32_t SDIO_MEM_PRE : 1;      /* bit:27 sdio memory present */
        uint32_t SD_XPC : 1;            /* bit:28 XPC for ACMD41 */
        uint32_t SD_UHSII : 1;          /* bit:29 UHSII for resp of ACMD41 */
        uint32_t HCS : 1;               /* bit:30 support high capacity  */
        uint32_t busy : 1;              /* bit:31 This bit is set to LOW if the card
                                           has not finished the power up routine */
    }bits;
};

/* register define */
struct cid {
    uint32_t mid;
    char     pnm[8];
    uint32_t serial;
    uint16_t oid;
    uint16_t year;
    uint8_t hwrev;
    uint8_t fwrev;
    uint8_t month;
};

struct csd {
    uint8_t     structure;
    uint8_t     spec_vers;
    uint16_t cmdclass;
    uint16_t tacc_clks;
    uint32_t tacc_ns;
    uint32_t c_size;
    uint32_t r2w_factor;
    uint32_t max_dtr;
    uint32_t erase_size;        /* In sectors */
    uint32_t read_blkbits;
    uint32_t write_blkbits;
    uint32_t capacity;
    uint32_t read_partial:1,
         rd_misalign:1,
         wr_partial:1,
         wr_misalign:1;
};

struct extcsd {
    uint8_t            rev;
    uint8_t            erase_group_def;
    uint8_t            sec_feature_support;
    uint8_t            rel_sectors;
    uint8_t            rel_param;
    uint8_t            part_config;
    uint8_t            cache_ctrl;
    uint8_t            rst_n_function;
    uint32_t        part_time_ms;        /* ms */
    uint32_t        sa_timeout_ns;        /* 100ns */
    uint32_t        generic_cmd6_time_ms;    /* 10ms */
    uint32_t        power_off_longtime_ms;     /* ms */
    uint32_t        hs_max_dtr;
    uint32_t        sectors;
    uint32_t        card_type;
    uint32_t        hc_erase_size;        /* In sectors */
    uint32_t        hc_erase_timeout;    /* In milliseconds */
    uint32_t        sec_trim_mult;    /* Secure trim multiplier  */
    uint32_t        sec_erase_mult;    /* Secure erase multiplier */
    uint32_t        trim_timeout;        /* In milliseconds */
    BOOL            enhanced_area_en;    /* enable bit */
    uint64_t        enhanced_area_offset;    /* Units: Byte */
    uint32_t        enhanced_area_size;    /* Units: KB */
    uint32_t        cache_size;        /* Units: KB */
    BOOL            hpi_en;            /* HPI enablebit */
    BOOL            hpi;            /* HPI support bit */
    uint32_t        hpi_cmd;        /* cmd used as HPI */
    uint32_t        data_sector_size;       /* 512 bytes or 4KB */
    uint32_t        data_tag_unit_size;     /* DATA TAG UNIT size */
    uint32_t        boot_ro_lock;        /* ro lock support */
    uint32_t        boot_size;
    uint8_t            raw_partition_support;    /* 160 */
    uint8_t            raw_erased_mem_count;    /* 181 */
    uint8_t            raw_ext_csd_structure;    /* 194 */
    uint8_t            raw_card_type;        /* 196 */
    uint8_t            out_of_int_time;    /* 198 */
    uint8_t            raw_s_a_timeout;        /* 217 */
    uint8_t            raw_hc_erase_gap_size;    /* 221 */
    uint8_t            raw_erase_timeout_mult;    /* 223 */
    uint8_t            raw_hc_erase_grp_size;    /* 224 */
    uint8_t            raw_sec_trim_mult;    /* 229 */
    uint8_t            raw_sec_erase_mult;    /* 230 */
    uint8_t            raw_sec_feature_support;/* 231 */
    uint8_t            raw_trim_mult;        /* 232 */
    uint8_t            raw_sectors[4];        /* 212 - 4 bytes */

    //uint32_t        feature_support;
//#define MMC_DISCARD_FEATURE    BIT(0)                  /* CMD38 feature */
};

struct sd_scr_r {
    uint8_t     sda_vsn;
    uint8_t     sda_spec3;
    uint8_t     bus_widths;
#define SD_SCR_BUS_WIDTH_1    (0x1<<0)
#define SD_SCR_BUS_WIDTH_4    (0x1<<2)
    uint8_t     cmds;
#define SD_SCR_CMD20_SUPPORT   (0x1<<0)
#define SD_SCR_CMD23_SUPPORT   (0x1<<1)
};

struct sd_ssr_r {
    uint8_t         dat_bus_width;
    uint8_t         secured_mode;
    uint16_t        card_type;
    uint16_t        protected_area;
    uint8_t         speed_class;
    uint8_t         perf_move;
    uint16_t        erase_size;
    uint8_t         erase_timeout;
    uint8_t         erase_offset;
    uint8_t         uhs_speed_grade;
    uint8_t         uhs_au_value;
    uint32_t        au_value;
};

enum max_dtr{
    HIGH_SPEED_MAX_DTR = 50000000,
    UHS_SDR104_MAX_DTR = 208000000,
    UHS_SDR50_MAX_DTR = 100000000,
    UHS_DDR50_MAX_DTR = 50000000,
    UHS_SDR25_MAX_DTR = 50000000,
    UHS_SDR12_MAX_DTR = 25000000
};

enum sd_bus_speed_mode{
    SD_MODE_UHS_SDR12 = 0,
    SD_MODE_HIGH_SPEED = 1,
    SD_MODE_UHS_SDR25 = 1,
    SD_MODE_UHS_SDR50 = 2,
    SD_MODE_UHS_SDR104 = 3,
    SD_MODE_UHS_DDR50 = 4
};

union sd3_bus_mode {
    uint32_t data;
    struct data_bit {
        uint32_t    uhs_sdr12 : 1;
        uint32_t    hs_uhs_sdr25 : 1;
        uint32_t    uhs_sdr50: 1;
        uint32_t    uhs_sdr104 : 1;
        uint32_t    uhs_ddr50 : 1;

        uint32_t    reserved : 27;
    }bits;
};

enum drv_type{
    SD_DRIVER_TYPE_B = 0x01,
    SD_DRIVER_TYPE_A = 0x02,
    SD_DRIVER_TYPE_C = 0x04,
    SD_DRIVER_TYPE_D = 0x08
};

enum curr_limit{
    SD_MAX_CURRENT_200 = (1<<0),
    SD_MAX_CURRENT_400 = (1<<1),
    SD_MAX_CURRENT_600 = (1<<2),
    SD_MAX_CURRENT_800 = (1<<3)
};

#define SD_SET_CURR_LIMIT_200    0
#define SD_SET_CURR_LIMIT_400    1
#define SD_SET_CURR_LIMIT_600    2
#define SD_SET_CURR_LIMIT_800    3

struct sd_sw_caps {
    enum max_dtr  hs_max_dtr;
    enum max_dtr  uhs_max_dtr;
    union sd3_bus_mode sd3_bus_mode;
    enum drv_type sd3_drv_type;
    enum curr_limit sd3_curr_limit;
};

struct sdio_card_cccr {
    uint32_t        sdio_vsn;
    uint32_t        sd_vsn;
    unsigned int    multi_blk:1,
                sdio_low_speed:1,
                sdio_wide_bus:1,
                sdio_high_power:1,
                sdio_high_speed:1,
                sdio_disable_cd:1;
};

struct sdio_card_cis {
    uint16_t    vendor;
    uint16_t    device;
    uint16_t    blksize;
    uint32_t    max_dtr;
};

struct mmc_card_reg {
    union mmc_ocr ocr;
    uint32_t rca;
    uint32_t cid_raw[4];
    uint32_t csd_raw[4];
    uint32_t scr_raw[2];
    uint16_t dsr_raw;
    struct cid cid;
    struct csd csd;
    struct extcsd ext_csd;
    struct sd_scr_r scr;
    struct sd_ssr_r ssr;
    struct sd_sw_caps sw_caps;
    struct sdio_card_cccr cccr;
    struct sdio_card_cis cis;
};

#define SDIO_MAX_FUNC    7
struct mmc_card
{
    struct mmc_host *host;

    enum card_type type;
    uint64_t capacity;   /* in bytes */
    int card_idx;

    void * partition;   /* TODO: */
    union card_state state;
    struct mmc_iocfg iocfg;
    struct mmc_card_reg card_reg;

    enum sd_bus_speed_mode bus_speed_mode;
    uint32_t erase_size;
    uint32_t erased_byte;
    /* for sdio device */
    uint32_t  sdio_func_num;
    struct sdio_func    *sdio_funcs[SDIO_MAX_FUNC];
    struct sdio_func    *sdio_irq;
    struct sdio_func_tuple    *tuple_link;
    int sdio_irq_count;
    uint32_t sdio_task;   /* FIXME: */
    mmc_event sdio_event;
    bool is_sdio_event_useable;

    unsigned int    claimer;
    unsigned int    claim_cnt;    /* "claim" nesting count */
    unsigned int    claimed:1;    /* host exclusively claimed */
    unsigned int    use_spi_crc:1;
    unsigned int    old_card:1;
    mmc_mutex lock;


    void* card_data;
    struct mmc_block* block;
};

#define set_card_type(card, ty)   ((card)->type = (ty))
#define get_card_type(card)   ((card)->type)
#define is_card_unknow(card)     ((card)->type == CARD_TYPE_UNKNOW)
#define is_card_mmc(card)     ((card)->type == CARD_TYPE_MMC)
#define is_card_sd(card)      ((card)->type == CARD_TYPE_SD)
#define is_card_sdio(card)    ((card)->type == CARD_TYPE_SDIO)
#define is_card_combo(card)    ((card)->type == CARD_TYPE_COMBO)

#define is_card_present(c)    ((c)->state.bits.present)
#define is_card_readonly(c)    ((c)->state.bits.readonly)
#define is_card_highspeed(c)    ((c)->state.bits.highspeed)
#define is_card_blkaddr(c)    ((c)->state.bits.blockaddr)
#define is_card_ddr_mode(c)    ((c)->state.bits.ddr_mode)
#define is_card_uhs(c)        ((c)->state.bits.ultra)
#define is_card_ext_capacity(c) ((c)->state.bits.sdxc)
#define is_card_hs200(c)    ((c)->state.bits.hs200)
#define is_card_removed(c)    ((c)->state.bits.removed)
#define is_card_sleep(c)    ((c)->state.bits.sleep)
#define is_card_removeable(c)    ((c)->state.bits.removeable)
#define is_card_not_std_sdio(c)  ((c)->state.bits.not_std_sdio)
#define is_card_blksz_for_byte_mode(c)  ((c)->state.bits.blksz_for_byte_mode)

#define set_card_present(c)    ((c)->state.bits.present = 1)
#define set_card_readonly(c) ((c)->state.bits.readonly = 1)
#define set_card_highspeed(c) ((c)->state.bits.highspeed = 1)
#define set_card_hs200(c)    ((c)->state.bits.hs200 = 1)
#define set_card_blockaddr(c) ((c)->state.bits.blockaddr = 1)
#define set_card_ddr_mode(c) ((c)->state.bits.ddr_mode = 1)
#define set_card_uhs(c) ((c)->state.bits.ultra = 1)
#define set_card_ext_capacity(c) ((c)->state.bits.sdxc = 1)
#define set_card_removed(c) ((c)->state.bits.removed = 1)
#define set_card_sleep(c) ((c)->state.bits.sleep = 1)
#define set_card_removeable(c)  ((c)->state.bits.removeable = 1)
#define set_card_not_std_sdio(c) ((c)->state.bits.not_std_sdio = 1)
#define set_card_blksz_for_byte_mode(c) ((c)->state.bits.blksz_for_byte_mode = 1)

#define clr_card_present(c)    ((c)->state.bits.present = 0)
#define clr_card_readonly(c) ((c)->state.bits.readonly = 0)
#define clr_card_highspeed(c) ((c)->state.bits.highspeed = 0)
#define clr_card_hs200(c)    ((c)->state.bits.hs200 = 0)
#define clr_card_blockaddr(c) ((c)->state.bits.blockaddr = 0)
#define clr_card_ddr_mode(c) ((c)->state.bits.ddr_mode = 0)
#define clr_card_uhs(c) ((c)->state.bits.ultra = 0)
#define clr_card_ext_capacity(c) ((c)->state.bits.sdxc = 0)
#define clr_card_removed(c) ((c)->state.bits.removed = 0)
#define clr_card_sleep(c)    ((c)->state.bits.sleep = 0)
#define clr_card_removeable(c)    ((c)->state.bits.removeable = 0)
#define clr_card_not_std_sdio(c)    ((c)->state.bits.not_std_sdio = 0)
#define clr_card_blksz_for_byte_mode(c)    ((c)->state.bits.blksz_for_byte_mode = 0)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* __CARD_H__ */
