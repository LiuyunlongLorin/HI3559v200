/*sdio.h begin*/
#ifndef _MMC_SDIO_H
#define _MMC_SDIO_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* SDIO command definitions */
 /* Class 9 */
#define SDIO_SEND_OP_COND          5
#define SDIO_RW_DIRECT             52
#define SDIO_RW_EXTENDED           53

/*SDIO resp */
#define SDIO_R4_1V8_PRESENT        (1 << 24)
#define SDIO_R4_MEMORY_PRESENT     (1 << 27)

#define SDIO_R5_COM_CRC_ERROR      (1 << 15)
#define SDIO_R5_ILLEGAL_COMMAND    (1 << 14)
#define SDIO_R5_ERROR              (1 << 11)
#define SDIO_R5_FUNCTION_NUMBER    (1 << 9)
#define SDIO_R5_OUT_OF_RANGE       (1 << 8)
#define SDIO_R5_STATUS(x)          (x & 0xCB00)
#define SDIO_R5_IO_CURRENT_STATE(x)    ((x & 0x3000) >> 12)

/*
 * Card Common Control Registers (CCCR)
 */
#define SDIO_CCCR           0x00

#define  SDIO_CCCR_REV_1_00    0x00 /* CCCR/FBR Version 1.00 */
#define  SDIO_CCCR_REV_1_10    0x01 /* CCCR/FBR Version 1.10 */
#define  SDIO_CCCR_REV_2_00    0x02 /* CCCR/FBR Version 2.00 */
#define  SDIO_CCCR_REV_3_00    0x03 /* CCCR/FBR Version 3.00 */

#define  SDIO_SDIO_REV_1_00    0x00    /* SDIO Spec Version 1.00 */
#define  SDIO_SDIO_REV_1_10    0x01    /* SDIO Spec Version 1.10 */
#define  SDIO_SDIO_REV_1_20    0x02    /* SDIO Spec Version 1.20 */
#define  SDIO_SDIO_REV_2_00    0x03    /* SDIO Spec Version 2.00 */
#define  SDIO_SDIO_REV_3_00    0x04    /* SDIO Spec Version 3.00 */

#define SDIO_CCCR_SD        0x01

#define  SDIO_SD_REV_1_01    0x00    /* SD Physical Spec Version 1.01 */
#define  SDIO_SD_REV_1_10    0x01    /* SD Physical Spec Version 1.10 */
#define  SDIO_SD_REV_2_00    0x02    /* SD Physical Spec Version 2.00 */
#define  SDIO_SD_REV_3_0x    0x03    /* SD Physical Spev Version 3.0x */

#define SDIO_CCCR_IOEx      0x02
#define SDIO_CCCR_IORx      0x03

#define SDIO_CCCR_IENx      0x04
#define SDIO_CCCR_INTx      0x05

#define SDIO_CCCR_ASx       0x06
#define SDIO_CCCR_BUS_IF_C        0x07

#define  SDIO_CCCR_WIDTH_1BIT    0x00
#define  SDIO_CCCR_WIDTH_4BIT    0x02
#define  SDIO_CCCR_WIDTH_8BIT    0x03
#define  SDIO_CCCR_ECSI        0x20
#define  SDIO_CCCR_SCSI        0x40

#define  SDIO_CCCR_ASYNC_INT    0x20

#define  SDIO_CCCR_CD_DISABLE    0x80

#define SDIO_CCCR_CAPS        0x08

#define  SDIO_CCCR_CAP_SDC    0x01
#define  SDIO_CCCR_CAP_SMB    0x02
#define  SDIO_CCCR_CAP_SRW    0x04
#define  SDIO_CCCR_CAP_SBS    0x08
#define  SDIO_CCCR_CAP_S4MI   0x10
#define  SDIO_CCCR_CAP_E4MI   0x20
#define  SDIO_CCCR_CAP_LSC    0x40
#define  SDIO_CCCR_CAP_4BLS   0x80

#define SDIO_CCCR_CIS        0x09    /* common CIS pointer */

#define SDIO_CCCR_BUS_SUS    0x0c
#define SDIO_CCCR_FUNC_SEL   0x0d
#define SDIO_CCCR_EXEC_FLG   0x0e
#define SDIO_CCCR_REDY_FLG   0x0f

#define SDIO_CCCR_FN0_BLK_SZ 0x10

#define SDIO_CCCR_POWER_CTL  0x12

#define  SDIO_POWER_SMPC    0x01
#define  SDIO_POWER_EMPC    0x02

#define SDIO_CCCR_SPEED_SEL  0x13

#define  SDIO_SPEED_SHS      0x01    /* Support High-Speed */
#define  SDIO_SPEED_SDR12       (0<<1)
#define  SDIO_SPEED_SDR25       (1<<1)
#define  SDIO_SPEED_SDR50       (2<<1)
#define  SDIO_SPEED_SDR104      (3<<1)
#define  SDIO_SPEED_DDR50       (4<<1)
#define  SDIO_SPEED_EHS         SDIO_SPEED_SDR25    /* Enable High-Speed */

#define SDIO_CCCR_UHS_I        0x14
#define  SDIO_UHS_SDR50         0x01
#define  SDIO_UHS_SDR104        0x02
#define  SDIO_UHS_DDR50         0x04

#define SDIO_CCCR_DRIVE_STRENGTH 0x15
#define  SDIO_DRIVE_SDTA    (1<<0)
#define  SDIO_DRIVE_SDTC    (1<<1)
#define  SDIO_DRIVE_SDTD    (1<<2)
#define  SDIO_DTSx_SET_TYPE_B    (0 << 4)
#define  SDIO_DTSx_SET_TYPE_A    (1 << 4)
#define  SDIO_DTSx_SET_TYPE_C    (2 << 4)
#define  SDIO_DTSx_SET_TYPE_D    (3 << 4)

#define SDIO_CCCR_IE       0x16
#define  SDIO_CCCR_SAI      0x01
#define  SDIO_CCCR_EAI      0x02

/*
 * Function Basic Registers (FBR)
 */
#define SDIO_FBR_BASE(f)    ((f) * 0x100)
#define  SDIO_FBR_STD_FUNC_IF_C  0x00
#define  SDIO_FBR_SUPPORTS_CSA    0x40
#define  SDIO_FBR_ENABLE_CSA    0x80
#define SDIO_FBR_STD_IF_EXT    0x01
#define SDIO_FBR_POWER_SEL        0x02
#define  SDIO_FBR_POWER_SPS    0x01
#define  SDIO_FBR_POWER_EPS    0x02    /* Enable (low) Power Selection */
#define SDIO_FBR_P_CIS        0x09    /* CIS pointer */
#define SDIO_FBR_P_CSA        0x0C    /* CSA pointer */
#define  SDIO_FBR_CSA_DATA_ACCESS   0x0F
#define  SDIO_FBR_BLKSIZE    0x10    /* block size */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _MMC_SDIO_H */
