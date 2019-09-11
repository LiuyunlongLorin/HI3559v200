#ifndef __HI_I2C_H__
#define __HI_I2C_H__
#include "asm/platform.h"
#include "linux/interrupt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define I2C_DRIVER_NAME "hibvt-i2c"

/*
 * I2C Registers offsets
 */
#define HIBVT_I2C_GLB       0x0
#define HIBVT_I2C_SCL_H     0x4
#define HIBVT_I2C_SCL_L     0x8
#define HIBVT_I2C_DATA1     0x10
#define HIBVT_I2C_TXF       0x20
#define HIBVT_I2C_RXF       0x24
#define HIBVT_I2C_CMD_BASE  0x30
#define HIBVT_I2C_LOOP1     0xb0
#define HIBVT_I2C_DST1      0xb4
#define HIBVT_I2C_TX_WATER  0xc8
#define HIBVT_I2C_RX_WATER  0xcc
#define HIBVT_I2C_CTRL1     0xd0
#define HIBVT_I2C_CTRL2     0xd4
#define HIBVT_I2C_STAT      0xd8
#define HIBVT_I2C_INTR_RAW  0xe0
#define HIBVT_I2C_INTR_EN   0xe4
#define HIBVT_I2C_INTR_STAT 0xe8

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

/* 
 * I2C Global Config Register -- HIBVT_I2C_GLB
 * */
#define GLB_EN_MASK     BIT(0)
#define GLB_SDA_HOLD_MASK   0xffff00
#define GLB_SDA_HOLD_SHIFT  (8)

/*
 *  I2C Timing CMD Register -- HIBVT_I2C_CMD_BASE + n * 4 (n = 0, 1, 2, ... 31)
 * */
#define CMD_EXIT    0x0
#define CMD_TX_S    0x1
#define CMD_TX_D1_2 0x4
#define CMD_TX_D1_1 0x5
#define CMD_TX_FIFO 0x9
#define CMD_RX_FIFO 0x12
#define CMD_RX_ACK  0x13
#define CMD_IGN_ACK 0x15
#define CMD_TX_ACK  0x16
#define CMD_TX_NACK 0x17
#define CMD_JMP1    0x18
#define CMD_UP_TXF  0x1d
#define CMD_TX_RS   0x1e
#define CMD_TX_P    0x1f


/*
 * I2C Control Register 1 -- HIBVT_I2C_CTRL1
 * */
#define CTRL1_CMD_START_MASK    BIT(0)
#define CTRL1_DMA_MASK          (BIT(9) | BIT(8))
#define CTRL1_DMA_R         (BIT(9) | BIT(8))
#define CTRL1_DMA_W         (BIT(9))

/*
 *I2C Status Register -- HIBVT_I2C_STAT
 * */
#define STAT_RXF_NOE_MASK   BIT(16) /* RX FIFO not empty flag */
#define STAT_TXF_NOF_MASK   BIT(19) /* TX FIFO not full flag */

/*
 * I2C Interrupt status and mask Register --
 * HIBVT_I2C_INTR_RAW, HIBVT_I2C_STAT, HIBVT_I2C_INTR_STAT
 */
#define INTR_ABORT_MASK     (BIT(0) | BIT(11))
#define INTR_RX_MASK        BIT(2)
#define INTR_TX_MASK        BIT(4)
#define INTR_CMD_DONE_MASK  BIT(12)
#define INTR_USE_MASK       (INTR_ABORT_MASK \
        |INTR_RX_MASK \
        | INTR_TX_MASK \
        | INTR_CMD_DONE_MASK)
#define INTR_ALL_MASK       0xffffffff

#define I2C_DEFAULT_FREQUENCY   100000
#define I2C_TXF_DEPTH       64
#define I2C_RXF_DEPTH       64
#define I2C_TXF_WATER       32
#define I2C_RXF_WATER       32
#define I2C_WAIT_TIMEOUT    (LOSCFG_BASE_CORE_TICK_PER_SECOND * 3)
#define I2C_TIMEOUT_COUNT    0x10000
#define I2C_IRQ_TIMEOUT     (msecs_to_jiffies(1000))
/*for i2c rescue*/
#define CHECK_SDA_IN_SHIFT     (16)
#define GPIO_MODE_SHIFT        (8)
#define FORCE_SCL_OEN_SHIFT    (4)
#define FORCE_SDA_OEN_SHIFT    (0)

//struct hibvt_i2c_dev {
//    struct i2c_adapter  adapter;
//    volatile unsigned char  *regbase;
//
//    unsigned int      clk;
//    int         irq;
//
//    unsigned int        freq;
//    struct i2c_msg      *msg;
//    unsigned int        msg_num;
//    unsigned int        msg_idx;
//    unsigned int        msg_buf_ptr;
//    EVENT_CB_S   msg_event;
//#define I2C_WAIT_RESPOND (1<<0)
//    int         status;
//};

struct hibvt_platform_i2c {
    unsigned int        msg_buf_ptr;
    int         status;
    EVENT_CB_S   msg_event;
    #define I2C_WAIT_RESPOND (1<<0)
};

struct i2c_hibvt_platform_data {
    unsigned int        freq;
    unsigned int      clk;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
