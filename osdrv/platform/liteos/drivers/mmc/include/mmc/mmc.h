
#ifndef _MMC_MMC_H
#define _MMC_MMC_H
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */
/* MMC command definitions */
   /* Class 0 and Class 1 */
#define GO_IDLE_STATE         0
#define SEND_OP_COND          1
#define ALL_SEND_CID          2
#define SET_RELATIVE_ADDR     3
#define MMC_SET_DSR           4

#define SWITCH_FUNC           6
/*
 * SWITCH_FUNC access modes
 */
 #define SWITCH_FUNC_CMD_SET       0x00
 #define SWITCH_FUNC_SET_BITS      0x01
 #define SWITCH_FUNC_CLEAR_BITS    0x02
 #define SWITCH_FUNC_WRITE_BYTE    0x03

#define SELECT_CARD           7
#define SEND_EXT_CSD          8
#define SEND_CSD              9
#define SEND_CID              10
#define READ_DAT_UNTIL_STOP   11
#define STOP_TRANSMISSION     12
#define SEND_STATUS           13
#define BUSTEST_R             14
#define GO_INACTIVE_STATE     15
#define BUSTEST_W             19

#define SPI_READ_OCR          58
#define SPI_CRC_ON_OFF        59

  /* class 2 */
#define SET_BLOCKLEN         16
#define READ_SINGLE_BLOCK    17
#define READ_MULTIPLE_BLOCK  18
#define SEND_TUNING_BLOCK    19
#define SEND_TUNING_BLOCK_HS200    21

  /* class 3 */
#define WRITE_DAT_UNTIL_STOP 20

  /* class 4 */
#define SET_BLOCK_COUNT      23
#define WRITE_BLOCK          24
#define WRITE_MULTIPLE_BLOCK 25
#define PROGRAM_CID          26
#define PROGRAM_CSD          27

  /* class 6 */
#define SET_WRITE_PROT       28
#define CLR_WRITE_PROT       29
#define SEND_WRITE_PROT      30

  /* class 5 */
#define ERASE_GROUP_START    35
#define ERASE_GROUP_END      36
//#define MMC_ERASE            38

  /* class 9 */
//#define MMC_FAST_IO              39
#define GO_IRQ_STATE         40

  /* class 7 */
#define LOCK_UNLOCK          42

  /* class 8 */
#define APP_CMD              55
#define GEN_CMD              56

#define RSP_R1_OUT_OF_RANGE        (1 << 31)    /* er, c */
#define RSP_R1_ADDRESS_ERROR    (1 << 30)    /* erx, c */
#define RSP_R1_BLOCK_LEN_ERROR    (1 << 29)    /* er, c */
#define RSP_R1_ERASE_SEQ_ERROR      (1 << 28)    /* er, c */
#define RSP_R1_ERASE_PARAM        (1 << 27)    /* ex, c */
#define RSP_R1_WP_VIOLATION        (1 << 26)    /* erx, c */
#define RSP_R1_CARD_IS_LOCKED    (1 << 25)    /* sx, a */
#define RSP_R1_LOCK_UNLOCK_FAILED    (1 << 24)    /* erx, c */
#define RSP_R1_COM_CRC_ERROR    (1 << 23)    /* er, b */
#define RSP_R1_ILLEGAL_COMMAND    (1 << 22)    /* er, b */
#define RSP_R1_CARD_ECC_FAILED    (1 << 21)    /* ex, c */
#define RSP_R1_CC_ERROR        (1 << 20)    /* erx, c */
#define RSP_R1_ERROR        (1 << 19)    /* erx, c */
#define RSP_R1_UNDERRUN        (1 << 18)    /* ex, c */
#define RSP_R1_OVERRUN        (1 << 17)    /* ex, c */
#define RSP_R1_CID_CSD_OVERWRITE    (1 << 16)    /* erx, c, CID/CSD overwrite */
#define RSP_R1_WP_ERASE_SKIP    (1 << 15)    /* sx, c */
#define RSP_R1_CARD_ECC_DISABLED    (1 << 14)    /* sx, a */
#define RSP_R1_ERASE_RESET        (1 << 13)    /* sr, c */
#define RSP_R1_READY_FOR_DATA    (1 << 8)    /* sx, a */
#define RSP_R1_SWITCH_ERROR        (1 << 7)    /* sx, c */
#define RSP_R1_APP_CMD        (1 << 5)    /* sr, c */
#define RSP_R1_STATUS(x)            (x & 0xFFFFE000)
#define RSP_R1_CURRENT_STATE(x)    ((x & 0x00001E00) >> 9)    /* sx, b (4 bits) */

#define RSP_R1_STATE_IDLE    0
#define RSP_R1_STATE_READY    1
#define RSP_R1_STATE_IDENT    2
#define RSP_R1_STATE_STBY    3
#define RSP_R1_STATE_TRAN    4
#define RSP_R1_STATE_DATA    5
#define RSP_R1_STATE_RCV    6
#define RSP_R1_STATE_PRG    7
#define RSP_R1_STATE_DIS    8

/*
 * CSD field definitions
 */
#define CSD_STRUCT_VER_1_0  0
#define CSD_STRUCT_VER_1_1  1
#define CSD_STRUCT_VER_1_2  2
#define CSD_STRUCT_EXT_CSD  3

#define CSD_SPEC_VER_0      0
#define CSD_SPEC_VER_1      1
#define CSD_SPEC_VER_2      2
#define CSD_SPEC_VER_3      3
#define CSD_SPEC_VER_4      4

/*
 * Card Command Classes (CCC)
 */
#define CCC_BASIC           (1<<0)
/* CCC_reserved */
#define CCC_BLOCK_READ      (1<<2)
/* CCC_reserved */
#define CCC_BLOCK_WRITE     (1<<4)
#define CCC_ERASE           (1<<5)
#define CCC_WRITE_PROT      (1<<6)
#define CCC_LOCK_CARD       (1<<7)
#define CCC_APP_SPEC        (1<<8)
#define CCC_IO_MODE         (1<<9)
#define CCC_SWITCH          (1<<10)


/*
 * Extended CSD fields
 */

#define E_CSD_FLUSH_CACHE        32      /* W */
#define E_CSD_CACHE_CTRL        33      /* R/W */
#define E_CSD_POWER_OFF_NOTIFICATION    34    /* R/W */
#define E_CSD_DATA_SECTOR_SIZE    61    /* R */
#define E_CSD_GP_SIZE_MULT        143    /* R/W */
#define E_CSD_PARTITION_ATTRIBUTE    156    /* R/W */
#define E_CSD_PARTITION_SUPPORT    160    /* RO */
#define E_CSD_HPI_MGMT        161    /* R/W */
#define E_CSD_RST_N_FUNCTION        162    /* R/W */
#define E_CSD_SANITIZE_START        165     /* W */
#define E_CSD_WR_REL_PARAM        166    /* RO */
#define E_CSD_BOOT_WP            173    /* R/W */
#define E_CSD_ERASE_GROUP_DEF        175    /* R/W */
#define E_CSD_PART_CONFIG        179    /* R/W */
#define E_CSD_ERASED_MEM_CONT        181    /* RO */
#define E_CSD_BUS_WIDTH        183    /* R/W */
#define E_CSD_HS_TIMING        185    /* R/W */
#define E_CSD_POWER_CLASS        187    /* R/W */
#define E_CSD_REV            192    /* RO */
#define E_CSD_STRUCTURE        194    /* RO */
#define E_CSD_CARD_TYPE        196    /* RO */
#define E_CSD_OUT_OF_INTERRUPT_TIME    198    /* RO */
#define E_CSD_PART_SWITCH_TIME        199     /* RO */
#define E_CSD_PWR_CL_52_195        200    /* RO */
#define E_CSD_PWR_CL_26_195        201    /* RO */
#define E_CSD_PWR_CL_52_360        202    /* RO */
#define E_CSD_PWR_CL_26_360        203    /* RO */
#define E_CSD_SEC_CNT            212    /* RO, 4 bytes */
#define E_CSD_S_A_TIMEOUT        217    /* RO */
#define E_CSD_REL_WR_SEC_C        222    /* RO */
#define E_CSD_HC_WP_GRP_SIZE        221    /* RO */
#define E_CSD_ERASE_TIMEOUT_MULT    223    /* RO */
#define E_CSD_HC_ERASE_GRP_SIZE    224    /* RO */
#define E_CSD_BOOT_MULT        226    /* RO */
#define E_CSD_SEC_TRIM_MULT        229    /* RO */
#define E_CSD_SEC_ERASE_MULT        230    /* RO */
#define E_CSD_SEC_FEATURE_SUPPORT    231    /* RO */
#define E_CSD_TRIM_MULT        232    /* RO */
#define E_CSD_PWR_CL_200_195        236    /* RO */
#define E_CSD_PWR_CL_200_360        237    /* RO */
#define E_CSD_PWR_CL_DDR_52_195    238    /* RO */
#define E_CSD_PWR_CL_DDR_52_360    239    /* RO */
#define E_CSD_POWER_OFF_LONG_TIME    247    /* RO */
#define E_CSD_GENERIC_CMD6_TIME    248    /* RO */
#define E_CSD_CACHE_SIZE        249    /* RO, 4 bytes */
#define E_CSD_TAG_UNIT_SIZE        498    /* RO */
#define E_CSD_DATA_TAG_SUPPORT    499    /* RO */
#define E_CSD_HPI_FEATURES        503    /* RO */

/*
 * Extended CSD field definitions
 */

#define E_CSD_WR_REL_PARAM_EN        (1<<2)

#define E_CSD_BOOT_WP_B_PWR_WP_DIS    (0x40)
#define E_CSD_BOOT_WP_B_PERM_WP_DIS    (0x10)
#define E_CSD_BOOT_WP_B_PERM_WP_EN    (0x04)
#define E_CSD_BOOT_WP_B_PWR_WP_EN    (0x01)

#define E_CSD_PART_CONFIG_ACC_MASK    (0x7)
#define E_CSD_PART_CONFIG_ACC_BOOT0    (0x1)
#define E_CSD_PART_CONFIG_ACC_GP0    (0x4)

#define E_CSD_PART_SUPPORT_PART_EN    (0x1)

#define E_CSD_CMD_SET_NORMAL        (1<<0)
#define E_CSD_CMD_SET_SECURE        (1<<1)
#define E_CSD_CMD_SET_CPSECURE    (1<<2)

#define E_CSD_CARD_TYPE_26    (1<<0)    /* Card can run at 26MHz */
#define E_CSD_CARD_TYPE_52    (1<<1)    /* Card can run at 52MHz */
#define E_CSD_CARD_TYPE_MASK    0x3F    /* Mask out reserved bits */
#define E_CSD_CARD_TYPE_DDR_1_8V  (1<<2)   /* Card can run at 52MHz */
                         /* DDR mode @1.8V or 3V I/O */
#define E_CSD_CARD_TYPE_DDR_1_2V  (1<<3)   /* Card can run at 52MHz */
                         /* DDR mode @1.2V I/O */
#define E_CSD_CARD_TYPE_DDR_52       (E_CSD_CARD_TYPE_DDR_1_8V  \
                    | E_CSD_CARD_TYPE_DDR_1_2V)
#define E_CSD_CARD_TYPE_SDR_1_8V    (1<<4)    /* Card can run at 200MHz */
#define E_CSD_CARD_TYPE_SDR_1_2V    (1<<5)    /* Card can run at 200MHz */
                        /* SDR mode @1.2V I/O */

#define E_CSD_CARD_TYPE_SDR_200    (E_CSD_CARD_TYPE_SDR_1_8V | \
                     E_CSD_CARD_TYPE_SDR_1_2V)

#define E_CSD_CARD_TYPE_SDR_ALL    (E_CSD_CARD_TYPE_SDR_200 | \
                     E_CSD_CARD_TYPE_52 | \
                     E_CSD_CARD_TYPE_26)

#define    E_CSD_CARD_TYPE_SDR_1_2V_ALL    (E_CSD_CARD_TYPE_SDR_1_2V | \
                     E_CSD_CARD_TYPE_52 | \
                     E_CSD_CARD_TYPE_26)

#define    E_CSD_CARD_TYPE_SDR_1_8V_ALL    (E_CSD_CARD_TYPE_SDR_1_8V | \
                     E_CSD_CARD_TYPE_52 | \
                     E_CSD_CARD_TYPE_26)

#define E_CSD_CARD_TYPE_SDR_1_2V_DDR_1_8V    (E_CSD_CARD_TYPE_SDR_1_2V | \
                         E_CSD_CARD_TYPE_DDR_1_8V | \
                         E_CSD_CARD_TYPE_52 | \
                         E_CSD_CARD_TYPE_26)

#define E_CSD_CARD_TYPE_SDR_1_8V_DDR_1_8V    (E_CSD_CARD_TYPE_SDR_1_8V | \
                         E_CSD_CARD_TYPE_DDR_1_8V | \
                         E_CSD_CARD_TYPE_52 | \
                         E_CSD_CARD_TYPE_26)

#define E_CSD_CARD_TYPE_SDR_1_2V_DDR_1_2V    (E_CSD_CARD_TYPE_SDR_1_2V | \
                         E_CSD_CARD_TYPE_DDR_1_2V | \
                         E_CSD_CARD_TYPE_52 | \
                         E_CSD_CARD_TYPE_26)

#define E_CSD_CARD_TYPE_SDR_1_8V_DDR_1_2V    (E_CSD_CARD_TYPE_SDR_1_8V | \
                         E_CSD_CARD_TYPE_DDR_1_2V | \
                         E_CSD_CARD_TYPE_52 | \
                         E_CSD_CARD_TYPE_26)

#define E_CSD_CARD_TYPE_SDR_1_2V_DDR_52    (E_CSD_CARD_TYPE_SDR_1_2V | \
                         E_CSD_CARD_TYPE_DDR_52 | \
                         E_CSD_CARD_TYPE_52 | \
                         E_CSD_CARD_TYPE_26)

#define E_CSD_CARD_TYPE_SDR_1_8V_DDR_52    (E_CSD_CARD_TYPE_SDR_1_8V | \
                         E_CSD_CARD_TYPE_DDR_52 | \
                         E_CSD_CARD_TYPE_52 | \
                         E_CSD_CARD_TYPE_26)

#define E_CSD_CARD_TYPE_SDR_ALL_DDR_1_8V    (E_CSD_CARD_TYPE_SDR_200 | \
                         E_CSD_CARD_TYPE_DDR_1_8V | \
                         E_CSD_CARD_TYPE_52 | \
                         E_CSD_CARD_TYPE_26)

#define E_CSD_CARD_TYPE_SDR_ALL_DDR_1_2V    (E_CSD_CARD_TYPE_SDR_200 | \
                         E_CSD_CARD_TYPE_DDR_1_2V | \
                         E_CSD_CARD_TYPE_52 | \
                         E_CSD_CARD_TYPE_26)

#define E_CSD_CARD_TYPE_SDR_ALL_DDR_52    (E_CSD_CARD_TYPE_SDR_200 | \
                         E_CSD_CARD_TYPE_DDR_52 | \
                         E_CSD_CARD_TYPE_52 | \
                         E_CSD_CARD_TYPE_26)

#define E_CSD_BUS_WIDTH_1    0    /* Card is in 1 bit mode */
#define E_CSD_BUS_WIDTH_4    1    /* Card is in 4 bit mode */
#define E_CSD_BUS_WIDTH_8    2    /* Card is in 8 bit mode */
#define E_CSD_DDR_BUS_WIDTH_4    5    /* Card is in 4 bit DDR mode */
#define E_CSD_DDR_BUS_WIDTH_8    6    /* Card is in 8 bit DDR mode */

#define E_CSD_SEC_ER_EN    BIT(0)
#define E_CSD_SEC_BD_BLK_EN    BIT(2)
#define E_CSD_SEC_GB_CL_EN    BIT(4)
#define E_CSD_SEC_SANITIZE    BIT(6)  /* v4.5 only */

#define E_CSD_RST_N_EN_MASK    0x3
#define E_CSD_RST_N_ENABLED    1    /* RST_n is enabled on card */

#define E_CSD_NO_POWER_NOTIFICATION    0
#define E_CSD_POWER_ON        1
#define E_CSD_POWER_OFF_SHORT        2
#define E_CSD_POWER_OFF_LONG        3

#define E_CSD_PWR_CL_8BIT_MASK    0xF0    /* 8 bit PWR CLS */
#define E_CSD_PWR_CL_4BIT_MASK    0x0F    /* 8 bit PWR CLS */
#define E_CSD_PWR_CL_8BIT_SHIFT    4
#define E_CSD_PWR_CL_4BIT_SHIFT    0

#define MMC_SDR_MODE        0
#define MMC_1_2V_DDR_MODE    1
#define MMC_1_8V_DDR_MODE    2
#define MMC_1_2V_SDR_MODE    3
#define MMC_1_8V_SDR_MODE    4

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _MMC_MMC_H */
