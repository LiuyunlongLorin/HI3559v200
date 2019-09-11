#ifndef __UART_USER_H__
#define __UART_USER_H__

#include "linux/wait.h"
#include "poll.h"
/* baudrate config */
#define UART_CFG_BAUDRATE    0x101

/* DMA CONFIG: receive */
#define UART_DMA_RX_EN    1
#define UART_DMA_RX_DIS    0

#define UART_CFG_DMA_RX    0x102

/* DMA CONFIG: send */
#define UART_DMA_TX_EN    1
#define UART_DMA_TX_DIS    0

#define UART_CFG_DMA_TX    0x103

/* Read Block: */
#define UART_RD_BLOCK    1
#define UART_RD_NONBLOCK    0

#define UART_CFG_RD_BLOCK    0x104

/* ATTRIBUTE CONFIG: data_bits, stop_bits, etc. */
struct uart_attr {
    unsigned int data_bits : 4;    /* bit0~3: data bits */
#define UART_ATTR_DATABIT_8    0
#define UART_ATTR_DATABIT_7    1
#define UART_ATTR_DATABIT_6    2
#define UART_ATTR_DATABIT_5    3

    unsigned int parity : 4;      /* bit4~7: parity */
#define UART_ATTR_PARITY_NONE    0
#define UART_ATTR_PARITY_ODD    1
#define UART_ATTR_PARITY_EVEN    2
#define UART_ATTR_PARITY_MARK    3
#define UART_ATTR_PARITY_SPACE    4

    unsigned int stop_bits : 4;   /* bit8~11: stop bits */
#define UART_ATTR_STOPBIT_1    0
#define UART_ATTR_STOPBIT_1P5    1
#define UART_ATTR_STOPBIT_2    2

    unsigned int rts : 1;    /* bit 12: rts */
#define UART_ATTR_RTS_DIS    0
#define UART_ATTR_RTS_EN    1

    unsigned int cts : 1;    /* bit 13: cts */
#define UART_ATTR_CTS_DIS    0
#define UART_ATTR_CTS_EN    1

    unsigned int fifo_rx_en : 1;    /* bit 14: rx fifo enable */
#define UART_ATTR_RX_FIFO_DIS    0
#define UART_ATTR_RX_FIFO_EN    1

    unsigned int fifo_tx_en : 1;    /* bit 15: tx fifo enable */
#define UART_ATTR_TX_FIFO_DIS    0
#define UART_ATTR_TX_FIFO_EN    1

    unsigned int reserved : 16;    /* bit16~31: reserved */
};
/*
 * uart attribute config cmd,
 * parameter should be 'struct uart_init *'
 * */
#define UART_CFG_ATTR    0x105

#define BUF_SIZE    0x4000    /* receive buf default size: 16K */
struct uart_driver_data;

struct uart_ops {
    //int (*init)(struct uart_driver_data *udd);
    //int (*de_init)(struct uart_driver_data *udd);
    int (*startup)(struct uart_driver_data *udd);
    int (*shutdown)(struct uart_driver_data *udd);

    int (*dma_startup)(struct uart_driver_data *udd, int dir);
    int (*dma_shutdown)(struct uart_driver_data *udd, int dir);
#define UART_DMA_DIR_RX    0
#define UART_DMA_DIR_TX    1

    int (*start_tx)(struct uart_driver_data *udd,
            const char *buf, size_t count);

    int (*config)(struct uart_driver_data *udd);
    /* private operation */
    int (*priv_operator)(struct uart_driver_data *udd, void *data);
};

struct uart_ioc_transfer {
    unsigned int rp;
    unsigned int wp;
    unsigned int flags;
#define BUF_CIRCLED    (1 << 0)
#define BUF_OVERFLOWED    (1 << 1)
#define BUF_EMPTIED    (1 << 2)

    char data[BUF_SIZE];
};

struct __uart_attr {
    unsigned int data_bits : 4;    /* bit0~3: data bits */
#define UART_ATTR_DATABIT_8    0
#define UART_ATTR_DATABIT_7    1
#define UART_ATTR_DATABIT_6    2
#define UART_ATTR_DATABIT_5    3

    unsigned int parity : 4;      /* bit4~7: parity */
#define UART_ATTR_PARITY_NONE    0
#define UART_ATTR_PARITY_ODD    1
#define UART_ATTR_PARITY_EVEN    2
#define UART_ATTR_PARITY_MARK    3
#define UART_ATTR_PARITY_SPACE    4

    unsigned int stop_bits : 4;   /* bit8~11: stop bits */
#define UART_ATTR_STOPBIT_1    0
#define UART_ATTR_STOPBIT_1P5    1
#define UART_ATTR_STOPBIT_2    2

    unsigned int rts : 1;    /* bit 12: rts */
#define UART_ATTR_RTS_DIS    0
#define UART_ATTR_RTS_EN    1

    unsigned int cts : 1;    /* bit 13: cts */
#define UART_ATTR_CTS_DIS    0
#define UART_ATTR_CTS_EN    1

    unsigned int fifo_rx_en : 1;    /* bit 14: rx fifo enable */
#define UART_ATTR_RX_FIFO_DIS    0
#define UART_ATTR_RX_FIFO_EN    1

    unsigned int fifo_tx_en : 1;    /* bit 15: tx fifo enable */
#define UART_ATTR_TX_FIFO_DIS    0
#define UART_ATTR_TX_FIFO_EN    1

    unsigned int reserved : 16;    /* bit16~31: reserved */
};

typedef int (*recv_notify)(struct uart_driver_data *udd,
        const char *buf, size_t count);

struct uart_driver_data {
    unsigned int num;
    unsigned int baudrate;
    struct __uart_attr attr;
    struct uart_ioc_transfer *rx_transfer;
    wait_queue_head_t wait;
	int count;
    int state;
#define UART_STATE_NOT_OPENED   (0)
#define UART_STATE_OPENING       (1)
#define UART_STATE_USEABLE       (2)
#define UART_STATE_SUSPENED     (3)

    unsigned int flags;
#define UART_FLG_DMA_RX   (1 << 0)
#define UART_FLG_DMA_TX   (1 << 1)
#define UART_FLG_RD_BLOCK   (1 << 2)

    recv_notify recv;
    struct uart_ops *ops;
    void *private;
};

/*
 * uart functions which are in uart host driver
 * uart host driver must define them
 * */
/* get uart host numbers */
int get_uart_num(void);
/* get uart driver data(udd) which is defined in uart host drivers,
 * through uart number */
struct uart_driver_data *get_uart_drv_data(int num);
/* uart private cmmand for uart host */
#define UART_CFG_PRIVATE    0x110

int uart_dev_init(void);
int uartdev_de_init(void);
int uart_suspend(void *data);
int uart_resume(void *data);

#endif /* __UART_USER_H__ */
