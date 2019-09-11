#ifndef _MMC_SD_H
#define _MMC_SD_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* SD commands  definitions */
  /* class 0 */
#define SD_SEND_RELATIVE_ADDR     3
#define SD_SEND_IF_COND           8
#define SD_SWITCH_VOLTAGE         11

  /* class 10 */
#define SD_SWITCH_FUNC                 6
    /*
     * SD_SWITCH_FUNC mode
     */
    #define SD_SWITCH_FUNC_CHECK        0
    #define SD_SWITCH_FUNC_SET          1
    /*
     * SD_SWITCH_FUNC function groups
     */
    #define SD_SWITCH_FUNC_GRP_ACCESS    0
    /*
     * SD_SWITCH_FUNC access modes
     */
    #define SD_SWITCH_FUNC_ACCESS_DEF    0
    #define SD_SWITCH_FUNC_ACCESS_HS     1


  /* class 5 */
#define SD_ERASE_WR_BLK_START    32
#define SD_ERASE_WR_BLK_END      33

  /* Application commands */
#define SD_ACMD_SET_BUS_WIDTH      6
#define SD_ACMD_SD_STATUS         13
#define SD_ACMD_SEND_NUM_WR_BLKS  22
#define SD_ACMD_OP_COND           41
#define SD_ACMD_SEND_SCR          51

/*
 * SCR field definitions
 */
#define SD_SCR_SPEC_VER_0        0
#define SD_SCR_SPEC_VER_1        1
#define SD_SCR_SPEC_VER_2        2

/*
 * SD bus widths
 */
#define SD_BUS_WIDTH_1        0
#define SD_BUS_WIDTH_4        2

/*FIXME: it's not a good way to define here*/
#define R1_ILLEGAL_COMMAND  (1 << 22)   /* er, b */
#define R1_CARD_ECC_FAILED  (1 << 21)   /* ex, c */
#define R1_CC_ERROR     (1 << 20)   /* erx, c */
#define R1_ERROR        (1 << 19)   /* erx, c */
#define R1_CURRENT_STATE(x) ((x & 0x00001E00) >> 9) /* sx, b (4 bits) */
#define R1_READY_FOR_DATA   (1 << 8)    /* sx, a */
#define CARD_STATE_IDLE   0
#define CARD_STATE_READY  1
#define CARD_STATE_IDENT  2
#define CARD_STATE_STBY   3
#define CARD_STATE_TRAN   4
#define CARD_STATE_DATA   5
#define CARD_STATE_RCV    6
#define CARD_STATE_PRG    7
#define CARD_STATE_DIS    8
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _MMC_SD_H */
