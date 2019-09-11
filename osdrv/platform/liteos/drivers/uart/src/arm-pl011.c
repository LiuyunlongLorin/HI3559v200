/*----------------------------------------------------------------------------
 * Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "asm/delay.h"
#include "asm/dma.h"
#include "asm/io.h"

#include "los_typedef.h"
#include "los_task.h"
#include "los_base.h"
#include "los_event.h"
#include "errno.h"

#include "linux/interrupt.h"
#include "linux/kernel.h"
#include "linux/spinlock.h"
#include "uart.h"
#include "hisoc/uart.h"

#include "uart_dev.h"
#include "arm-pl011.h"
#include "string.h"

#include <linux/module.h>
#include "linux/device.h"
#include "linux/platform_device.h"
#if defined(LOSCFG_DRIVERS_HIDMAC) || defined(LOSCFG_DRIVERS_HIEDMAC)
#include "dmac_ext.h"

/* 1K stack for dma receive task, per 20ms */
#define PL011_DMA_RX_STACKSIZE          0x800
#define UART_TSK_PRIO               8
#define HI_EDMA_AVERSE_SCHEME

/* should aligned by CACHE_ALIGNED_SIZE */
#ifdef HI_EDMA_AVERSE_SCHEME
#define RX_DMA_BUF_ALIGN_SIZE          ALIGN(0x1000, CACHE_ALIGNED_SIZE)
#else

#define RX_DMA_BUF_ALIGN_SIZE          ALIGN(RX_DMA_BUF_SIZE, CACHE_ALIGNED_SIZE)
#endif

extern void dma_cache_inv(AARCHPTR start, AARCHPTR end);
extern int edma_uart2memory(AARCHPTR memaddr, unsigned int peri_addr, unsigned int length);
extern unsigned int edmac_list_write_pointer(unsigned int peri_addr,unsigned int channel);

#ifdef HI_DMA_AVERSE_SCHEME
unsigned int LastAddr = 0;
UINT16 timerId = 0;

void S3_dma_timer(unsigned int arg)
{
    struct timeval curtime;
    (void)gettimeofday(&curtime,NULL);
    unsigned int curmsec = curtime.tv_sec*1000 + curtime.tv_usec/1000;
    struct uart_driver_data *udd = (struct uart_driver_data *)arg;
    struct arm_pl011_port *port;
    struct uart_dma_transfer *udt;
    port = (struct arm_pl011_port *)udd->private;
    if (port)
        udt = port->rx_udt;
    if (NULL == udt) {
        uart_error("udt is null!\n");
        return ;
    }
    unsigned int curAddr = dmac_get_current_dest(udt->channel);
    if(curAddr != LastAddr)
    {
        //Doing Transfer,do nothing.
    }
    else
    {
        //Reset edma destination memory address by restart edma.
        unsigned int edmaCh = udt->channel;
        edma_channel_close(edmaCh);
        int ret = dmac_llip2m_transfer((unsigned long)udt->buf, get_uart_dma_peri(udd->num),
                RX_DMA_BUF_ALIGN_SIZE,
                &(edmaCh));
        if(ret < 0)
        {
            dprintf("Error: Restart uart1-edma failed!\n");
            return ;
        }
        udt->channel = ret;
        udt->tail = 0;
        curAddr = dmac_get_current_dest(udt->channel);
    }
    LastAddr = curAddr;
}

int S3_dma_setTimer(struct uart_driver_data *udd)
{
    if(timerId != 0)
    {
        dprintf("Error: Timer for uart1 has create!\n");
        return -1;
    }
    int ret = LOS_SwtmrCreate(LOSCFG_BASE_CORE_TICK_PER_SECOND * 1 ,LOS_SWTMR_MODE_PERIOD,S3_dma_timer,&timerId,(unsigned int)udd);
    if(ret != 0)
    {
        dprintf("Error: Create timer for uart1 failed!\n");
        return -1;
    }
    ret = LOS_SwtmrStart(timerId);
    if(ret != 0)
    {
        dprintf("Error: Start timer for uart1 failed!\n");
        return -1;
    }
    return 0;
}

int S3_dma_closeTimer(void)
{
    if(timerId == 0)
    {
        return 0;
    }
    int ret = LOS_SwtmrStop(timerId);
    if(ret != 0)
    {
        dprintf("Error: Stop timer for uart1 failed!\n");
        return ret;
    }
    ret = LOS_SwtmrDelete(timerId);
    if(ret != 0)
    {
        dprintf("Error: Stop timer for uart1 failed!\n");
        return ret;
    }
    timerId = 0;
    return 0;
}

int s3_timer_set(struct uart_driver_data *udd, void *data)
{
    if(NULL == udd || NULL == data)
    {
        dprintf("Error: input is null!\n");
        return -1;
    }
    int is_enable = *((int*)data);
    if(is_enable == 1)
    {
        return S3_dma_setTimer(udd);
    }
    return S3_dma_closeTimer();
}
#endif
#ifdef HI_EDMA_AVERSE_SCHEME
static irq_handler_t S3_dma_uart_irq(int irq, void *data)
{
    struct uart_driver_data *udd = (struct uart_driver_data *)data;
    struct arm_pl011_port *port = NULL;
    if (NULL == udd) {
        uart_error("udd is null!\n");
        return IRQ_NONE;
    }
    port = (struct arm_pl011_port *)udd->private;
    writew(0xffff, port->phys_base + UART_CLR);
    writew(0x0, port->phys_base + UART_DMACR);
    writew(0x1, port->phys_base + UART_DMACR);
    int count=0;
    while(count < 100000)
        count++;
    writew(0x0, port->phys_base + UART_DMACR);
    writew(0x1, port->phys_base + UART_DMACR);
    count=0;
    while(count < 100000)
        count++;
    writew(0x0, port->phys_base + UART_DMACR);
    writew(0x1, port->phys_base + UART_DMACR);
    count=0;
    while(count < 100000)
        count++;
}

#endif

static int pl011_dma_copy_to_buf(struct uart_driver_data *udd)
{
    struct arm_pl011_port *port;
    struct uart_dma_transfer *udt;

    unsigned int end_size = 0;
    unsigned int begin_size = 0;
    unsigned int end_tail = 0;

    char *dst_buf = NULL;
    unsigned long src_buf = 0;
    port = (struct arm_pl011_port *)udd->private;
    udt = port->rx_udt;
    src_buf = (unsigned long)udt->buf;

    if (udt->flags & BUF_CIRCLED) {
        /* size:copy length */
        end_size = RX_DMA_BUF_ALIGN_SIZE - udt->tail;
        /* cache invalidate the buf where will be copied */
        dma_cache_inv(TRUNCATE((unsigned int)(src_buf + udt->tail), CACHE_ALIGNED_SIZE),
                (unsigned int)(src_buf + RX_DMA_BUF_ALIGN_SIZE));
        end_tail = udt->tail;
        udt->tail = 0;
    }
    /* size:copy length */
    begin_size = udt->head - udt->tail;
    /* cache invalidate the buf where will be copied */
    dma_cache_inv(TRUNCATE((unsigned int)(src_buf + udt->tail), CACHE_ALIGNED_SIZE),
            ALIGN((unsigned int)(src_buf + udt->head), CACHE_ALIGNED_SIZE));
    dst_buf = LOS_MemAlloc(m_aucSysMem0, end_size + begin_size);
    if (NULL == dst_buf) {
        if (udt->flags & BUF_CIRCLED) {
            udt->tail = end_tail;
        }
        uart_error("alloc for dma copy failed");
        return -ENOMEM;
    }

    if (udt->flags & BUF_CIRCLED) {
        memcpy(dst_buf, (void *)(src_buf + end_tail), end_size);
        udt->flags &= ~BUF_CIRCLED;
    }
    memcpy((void *)(dst_buf + end_size), (void *)(src_buf + udt->tail), begin_size);
    udt->tail = udt->head;
    udd->recv(udd, dst_buf, end_size + begin_size);
    (VOID)LOS_MemFree(m_aucSysMem0, dst_buf);
    return 0;
}

/*
 * thread for uart dma receive, executed per 20ms
 * p0: udd, uart driver data
 */
static void *pl011_dma_rx_thread(AARCHPTR p0)
{
    unsigned int head = 0;
    struct uart_driver_data *udd = (struct uart_driver_data *)p0;
    struct arm_pl011_port *port;
    struct uart_dma_transfer *udt = NULL;

    if (NULL == udd) {
        uart_error("udd is null!\n");
        return (void *)-EFAULT;
    }
    port = (struct arm_pl011_port *)udd->private;
    if (port)
        udt = port->rx_udt;
    if (NULL == udt) {
        uart_error("udt is null!\n");
        return (void *)-EFAULT;
    }

    while(1) {
        /* check out per 20ms */
        msleep(20);
        if (UART_STATE_SUSPENED == udd->state) {
            uart_trace(UART_INFO,"uart %d suspend\n", udd->num);
            continue;
        }


#if LOSCFG_DRIVERS_HIEDMAC
		head = edmac_list_write_pointer(port->phys_base,udt->channel) - (unsigned long)udt->buf;
#endif

		if (!(udt->flags & BUF_CIRCLED)) {
			if (head == udt->tail) {
				continue;/* no new data received */
			} else if (head < udt->tail) {
				/* cycled, receive end and begin */
				udt->flags |= BUF_CIRCLED;
			}
		} else {
			/*
			 * Take care on this case, before this transfer, the layout of buf:
			 *
			 *   buf-start            wp       rp             buf-end
			 *   ######################         #####################
			 *
			 *   after this transfer:
			 *   buf-start                     rp    wp       buf-end
			 *   ####################################################
			 *                                 |______|
			 *                                 data overflow
			 *
			 *   In this case, the rp should be corrected as follow:
			 *   buf-start                           rp/wp    buf-end
			 *   ####################################################
			 *
			 *  thus the rp and wp point to the sampe position.
			 */
			if (head >= udt->tail) {
				uart_error("dma buf overflow!");
				udt->tail = head;
			} else {
				/* cycled receive */
				uart_trace(TRACE_MSG,"info: dma buf cycled");
			}
		}

        udt->head = head;
        /* copy to rx buf */
        pl011_dma_copy_to_buf(udd);
    }
    return NULL;
}
#endif /* LOSCFG_DRIVERS_HIDMAC */

#define FIFO_SIZE    64
static irqreturn_t pl011_irq(int irq, void *data)
{
    unsigned int status = 0;
    unsigned int fr = 0;
    char buf[FIFO_SIZE];
    unsigned int count = 0;
    struct arm_pl011_port *port = NULL;
    struct uart_driver_data *udd = (struct uart_driver_data *)data;
    if (NULL == udd) {
        uart_error("udd is null!\n");
        return IRQ_HANDLED;
    }
    port = (struct arm_pl011_port *)udd->private;
    status = readw(port->phys_base + UART_MIS);
    if (status & (UART_MIS_RX | UART_IMSC_TIMEOUT)) {
        do {
            fr = readb(port->phys_base + UART_FR);
            if (fr & UART_FR_RXFE) break;

            buf[count++] = readb(port->phys_base + UART_DR);
        } while(count < FIFO_SIZE);
        //uart_error("buf:%s,size:%d", buf,count);
        udd->recv(udd, buf, count);
    }
    /* FIXME: send irq */
end:
    /* clear all interrupt */
    writew(0xFFFF, port->phys_base + UART_CLR);
    return IRQ_HANDLED;
}

static int pl011_config_in(struct uart_driver_data *udd)
{
    unsigned int cr, lcrh;
    unsigned int value = 0;
    unsigned int divider = 0;
    unsigned int remainder = 0;
    unsigned int fraction = 0;
    struct arm_pl011_port *port = NULL;

    if (NULL == udd) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }
    port = (struct arm_pl011_port *)udd->private;
    /* get CR */
    cr = readw(port->phys_base + UART_CR);
    /* get LCR_H */
    lcrh = readw(port->phys_base + UART_LCR_H);
    /* uart disable */
    writew(0, port->phys_base + UART_CR);

    /* config cts/rts */
    if (UART_ATTR_CTS_EN == udd->attr.cts)
        cr |= UART_CR_CTS;
    else
        cr &= ~UART_CR_CTS;

    if (UART_ATTR_RTS_EN == udd->attr.rts)
        cr |= UART_CR_RTS;
    else
        cr &= ~UART_CR_RTS;

    lcrh &= ~UART_LCR_H_FIFO_EN;
    writeb(lcrh, port->phys_base + UART_LCR_H);

    cr &= ~UART_CR_EN;
    writew(cr, port->phys_base + UART_CR);

    /* set baud rate */
    value = 16 * udd->baudrate;
    divider = CONFIG_UART_CLK_INPUT / value;
    remainder = CONFIG_UART_CLK_INPUT % value;
    value = (8 * remainder) / udd->baudrate;
    fraction = (value >> 1) + (value & 1);
    writel(divider, port->phys_base + UART_IBRD);
    writel(fraction,port->phys_base + UART_FBRD);

    /* config lcr_h */
    lcrh &= ~UART_LCR_H_8_BIT;
    switch (udd->attr.data_bits) {
        case UART_ATTR_DATABIT_5:
            lcrh |= UART_LCR_H_5_BIT;
            break;
        case UART_ATTR_DATABIT_6:
            lcrh |= UART_LCR_H_6_BIT;
            break;
        case UART_ATTR_DATABIT_7:
            lcrh |= UART_LCR_H_7_BIT;
            break;
        case UART_ATTR_DATABIT_8:
        default:
            lcrh |= UART_LCR_H_8_BIT;
            break;
    }
    lcrh &= ~UART_LCR_H_PEN;
    lcrh &= ~UART_LCR_H_EPS;
    lcrh &= ~UART_LCR_H_SPS;
    switch (udd->attr.parity) {
        case UART_ATTR_PARITY_EVEN:
            lcrh |= UART_LCR_H_PEN;
            lcrh |= UART_LCR_H_EPS;
            lcrh |= UART_LCR_H_FIFO_EN;
            break;

        case UART_ATTR_PARITY_ODD:
            lcrh |= UART_LCR_H_PEN;
            lcrh &= ~UART_LCR_H_EPS;
            lcrh |= UART_LCR_H_FIFO_EN;
            break;

        case UART_ATTR_PARITY_MARK:
            lcrh |= UART_LCR_H_PEN;
            lcrh &= ~UART_LCR_H_EPS;
            lcrh |= UART_LCR_H_FIFO_EN;
            lcrh |= UART_LCR_H_SPS;
            break;

        case UART_ATTR_PARITY_SPACE:
            lcrh |= UART_LCR_H_PEN;
            lcrh |= UART_LCR_H_EPS;
            lcrh |= UART_LCR_H_FIFO_EN;
            lcrh |= UART_LCR_H_SPS;
            break;

        case UART_ATTR_PARITY_NONE:
        default:
            lcrh &= ~UART_LCR_H_PEN;
            lcrh &= ~UART_LCR_H_SPS;
            break;
    }
    switch (udd->attr.stop_bits) {
        case UART_ATTR_STOPBIT_2:
            lcrh |= UART_LCR_H_STP2;
        break;
        case UART_ATTR_STOPBIT_1:
        default:
            lcrh &= ~UART_LCR_H_STP2;
            break;
    }
    if (udd->attr.fifo_rx_en || udd->attr.fifo_tx_en) {
        lcrh |= UART_LCR_H_FIFO_EN;
    }
    writeb(lcrh, port->phys_base + UART_LCR_H);
    cr |= UART_CR_EN;
    /* resume CR */
    writew(cr, port->phys_base + UART_CR);
    return 0;
}

/*
 * request uart dma receive
 */
#if defined(LOSCFG_DRIVERS_HIDMAC) || defined(LOSCFG_DRIVERS_HIEDMAC)
static int pl011_rx_request_dma(struct uart_driver_data *udd)
{
    int ret = 0;
    unsigned int peri_id = 0;

    struct uart_dma_transfer *udt = NULL;
    TSK_INIT_PARAM_S dma_rx_task;
    struct arm_pl011_port *port = NULL;

    if (NULL == udd) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }
    port = (struct arm_pl011_port *)udd->private;
    udt = port->rx_udt;

    memset(&dma_rx_task, 0, sizeof(TSK_INIT_PARAM_S));

    /* allocate memory for dma */
    udt = (struct uart_dma_transfer *)LOS_MemAlloc(m_aucSysMem0, sizeof(struct uart_dma_transfer));
    if (NULL == udt) {
        return -ENOMEM;
    }
    memset(udt, 0, sizeof(struct uart_dma_transfer));
    udt->buf = memalign(CACHE_ALIGNED_SIZE, RX_DMA_BUF_ALIGN_SIZE);
    if (NULL == udt->buf) {
        ret = -ENOMEM;
        goto free_udt;
    }

    peri_id = get_uart_dma_peri(udd->num);
#ifdef LOSCFG_DRIVERS_HIDMAC
    ret = dmac_llip2m_transfer((unsigned long)udt->buf, peri_id,
            RX_DMA_BUF_ALIGN_SIZE,
            &(udt->channel));
#endif
#ifdef LOSCFG_DRIVERS_HIEDMAC
    ret = edma_uart2memory((AARCHPTR)udt->buf, port->phys_base, RX_DMA_BUF_ALIGN_SIZE);

#endif
	if (ret) {
		goto free_dma_rx_buf;
	}
    port->rx_udt = udt;

    /* create task for coping dma buffer */
    dma_rx_task.pfnTaskEntry = (TSK_ENTRY_FUNC)pl011_dma_rx_thread;
    dma_rx_task.uwStackSize  = PL011_DMA_RX_STACKSIZE;
    dma_rx_task.pcName       = "uart_dma_rx";
    dma_rx_task.usTaskPrio   = UART_TSK_PRIO;
    dma_rx_task.uwResved     = LOS_TASK_STATUS_DETACHED;
    dma_rx_task.auwArgs[0]   = (AARCHPTR)udd;
    ret = LOS_TaskCreate(&udt->thread_id, &dma_rx_task);
    if (ret) {
        goto free_dma_rx_buf;
    }
    return ret;

free_dma_rx_buf:
    (VOID)LOS_MemFree(m_aucSysMem0, udt->buf);
    udt->buf = NULL;
free_udt:
    (VOID)LOS_MemFree(m_aucSysMem0, udt);
    port->rx_udt = NULL;
    return ret;
}

/*
 * shutdown dma transfer
 * udd: uart driver data
 */
static int pl011_rx_free_dma(struct uart_driver_data *udd)
{
    unsigned long flags;
    struct arm_pl011_port *port = NULL;
    struct uart_dma_transfer *udt = NULL;

    if (NULL == udd) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }
    port = (struct arm_pl011_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }
    udt = port->rx_udt;

    if (udt) {
        (VOID)LOS_TaskDelete(udt->thread_id);
        (VOID)LOS_MemFree(m_aucSysMem0, udt->buf);
        (VOID)LOS_MemFree(m_aucSysMem0, udt);
    }
    port->rx_udt = NULL;

    return 0;
}
#endif /* LOSCFG_DRIVERS_HIDMAC */


/* uops */
static int pl011_startup(struct uart_driver_data *udd)
{
    int ret = 0;
    unsigned int fr = 0;
    unsigned int cr = 0;
    struct arm_pl011_port *port = NULL;

    if (NULL == udd) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }
    port = (struct arm_pl011_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }
    /* enable the clock */
    LOS_TaskLock();
    uart_clk_cfg(udd->num, true);
    LOS_TaskUnlock();

    /* uart disable */
    writew(0, port->phys_base + UART_CR);

    writew(0xFF, port->phys_base + UART_RSR);
    /* clear all interrupt,set mask */
    writew(0xFFFF, port->phys_base + UART_CLR);

    /* mask all interrupt */
    writew(0x0, port->phys_base + UART_IMSC);

    /* interrupt trigger line RX: >= 7/8, TX <= 1/8 */
    writew(UART_IFLS_RX4_8 | UART_IFLS_TX7_8,
            port->phys_base + UART_IFLS);
#ifdef HI_EDMA_AVERSE_SCHEME
    if(udd->num == 1)
    {
        writew(UART_IFLS_RX1_8 | UART_IFLS_TX7_8,
                port->phys_base + UART_IFLS);
    }
#endif

#if defined(LOSCFG_DRIVERS_HIDMAC) || defined(LOSCFG_DRIVERS_HIEDMAC)
    if (udd->flags & UART_FLG_DMA_RX) {
        if (!(port->flags & PL011_FLG_DMA_RX_REQUESTED)) {
            ret = pl011_rx_request_dma(udd);
            if (!ret) {
                port->flags |= PL011_FLG_DMA_RX_REQUESTED;
                /* enable rx dma */
                writel(UART_DMACR_RX, port->phys_base + UART_DMACR);
            } else {
                udd->flags &= ~UART_FLG_DMA_RX;
            }
        }
    }
#endif /* LOSCFG_DRIVERS_HIDMAC || LOSCFG_DRIVERS_HIEDMAC */
    /* FIXME: not support dma send yet */

#ifdef HI_EDMA_AVERSE_SCHEME
    /* request irq or dma request failed */
    if (!(udd->flags & UART_FLG_DMA_RX)) {
        if (!(port->flags & PL011_FLG_IRQ_REQUESTED)) {
            if(udd->num == 1)
                ret = request_irq(port->irq_num, (irq_handler_t)S3_dma_uart_irq,
                        0, "uart_pl011",udd);
            else
                ret = request_irq(port->irq_num, (irq_handler_t)pl011_irq,
                        0, "uart_pl011",udd);
            if (!ret) {
                port->flags |= PL011_FLG_IRQ_REQUESTED;
                /* enable rx and timeout interrupt */
                writew(UART_IMSC_RX | UART_IMSC_TIMEOUT, port->phys_base + UART_IMSC);
            }
        }
    }
#else
    /* request irq or dma request failed */
    if (!(udd->flags & UART_FLG_DMA_RX)) {
        if (!(port->flags & PL011_FLG_IRQ_REQUESTED)) {
            ret = request_irq(port->irq_num, (irq_handler_t)pl011_irq,
                    0, "uart_pl011",udd);
            if (!ret) {
                port->flags |= PL011_FLG_IRQ_REQUESTED;
                /* enable rx and timeout interrupt */
                writew(UART_IMSC_RX | UART_IMSC_TIMEOUT, port->phys_base + UART_IMSC);
            }
        }
    }
#endif

    cr = readw(port->phys_base + UART_CR);
    cr |= UART_CR_EN | UART_CR_RX_EN | UART_CR_TX_EN;
    writel(cr, port->phys_base + UART_CR);

    pl011_config_in(udd);

    return ret;
}

static int pl011_shutdown(struct uart_driver_data *udd)
{
    unsigned int reg_tmp = 0;
    struct arm_pl011_port *port = NULL;

    if (NULL == udd) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }
    port = (struct arm_pl011_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    writew(0, port->phys_base + UART_IMSC);
    writew(0xFFFF, port->phys_base + UART_CLR);

    if (port->flags & PL011_FLG_DMA_RX_REQUESTED) {
#if defined(LOSCFG_DRIVERS_HIDMAC) || defined(LOSCFG_DRIVERS_HIEDMAC)
        pl011_rx_free_dma(udd);
        port->flags &= ~PL011_FLG_DMA_RX_REQUESTED;
#endif /* LOSCFG_DRIVERS_HIDMAC */
    } else if (port->flags & PL011_FLG_IRQ_REQUESTED) {
        free_irq(port->irq_num, udd);
        port->flags &= ~PL011_FLG_IRQ_REQUESTED;
    }

    reg_tmp = readw(port->phys_base + UART_CR);
    reg_tmp &= ~UART_CR_TX_EN;
    reg_tmp &= ~UART_CR_RX_EN;
    reg_tmp &= ~UART_CR_EN;
    writew(reg_tmp, port->phys_base + UART_CR);

    /* disable break and fifo */
    reg_tmp = readw(port->phys_base + UART_LCR_H);
    reg_tmp &= ~(UART_LCR_H_BREAK);
    reg_tmp &= ~(UART_LCR_H_FIFO_EN);
    writew(reg_tmp, port->phys_base + UART_LCR_H);

    /* shut down the clock */
    LOS_TaskLock();
    uart_clk_cfg(udd->num, false);
    LOS_TaskUnlock();
    return 0;
}

#if defined(LOSCFG_DRIVERS_HIDMAC) || defined(LOSCFG_DRIVERS_HIEDMAC)
static int pl011_dma_startup(struct uart_driver_data *udd, int dir)
{
    int ret;
    unsigned int cr, reg_tmp;
    struct arm_pl011_port *port = NULL;

    if (NULL == udd) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }
    port = (struct arm_pl011_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    /* only support receive dma */
    if (UART_DMA_DIR_RX != dir) {
        return -EFAULT;
    }
    do {
        reg_tmp = readb(port->phys_base + UART_FR);
        if (!(reg_tmp & UART_FR_BUSY))
            break;
        msleep(10);
    } while(1);

    /* disable uart */
    cr = readw(port->phys_base + UART_CR);
    writew(0, port->phys_base + UART_CR);

    ret = pl011_rx_request_dma(udd);
    if (!ret) {
#ifndef HI_EDMA_AVERSE_SCHEME
        /* disable uart irq */
        if (port->flags & PL011_FLG_IRQ_REQUESTED) {
            free_irq(port->irq_num, udd);
            port->flags &= ~PL011_FLG_IRQ_REQUESTED;
            /* disable rx and timeout interrupt */
            reg_tmp = readl(port->phys_base + UART_IMSC);
            reg_tmp &= ~UART_IMSC_RX;
            reg_tmp &= ~UART_IMSC_TIMEOUT;
            writew(reg_tmp, port->phys_base + UART_IMSC);
        }
#endif
        /* enable rx dma */
        reg_tmp = readl(port->phys_base + UART_DMACR);
        reg_tmp |= UART_DMACR_RX;
        writel(reg_tmp, port->phys_base + UART_DMACR);
    }
    /* resume cr, enable rx,tx */
    writel(cr, port->phys_base + UART_CR);

    return ret;
}

static int pl011_dma_shutdown(struct uart_driver_data *udd, int dir)
{
    int ret = 0;
    unsigned int cr, reg_tmp;
    struct arm_pl011_port *port = NULL;

    if (NULL == udd) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }
    port = (struct arm_pl011_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    /* only support receive dma */
    if (UART_DMA_DIR_RX != dir) {
        return -EFAULT;
    }

    do {
        reg_tmp = readb(port->phys_base + UART_FR);
        if (!(reg_tmp & UART_FR_BUSY))
            break;
        msleep(10);
    } while(1);

    /*disable uart*/
    cr = readw(port->phys_base + UART_CR);
    writew(0, port->phys_base + UART_CR);

    if (!(port->flags & PL011_FLG_IRQ_REQUESTED)) {
        ret = request_irq(port->irq_num, (irq_handler_t)pl011_irq,
                0, "uart_pl011",udd);
        if (!ret) {
            port->flags |= PL011_FLG_IRQ_REQUESTED;
            /* enable rx and timeout interrupt */
            reg_tmp = readl(port->phys_base + UART_IMSC);
            reg_tmp |= UART_IMSC_RX;
            reg_tmp |= UART_IMSC_TIMEOUT;
            writew(reg_tmp, port->phys_base + UART_IMSC);

            /*disable rx dma*/
            reg_tmp = readl(port->phys_base + UART_DMACR);
            reg_tmp &= ~UART_DMACR_RX;
            writel(reg_tmp, port->phys_base + UART_DMACR);

            pl011_rx_free_dma(udd);
            port->flags &= ~PL011_FLG_DMA_RX_REQUESTED;
        }
    }
    /* resume cr, enable rx,tx */
    writew(cr, port->phys_base + UART_CR);

    return ret;
}
#endif /* LOSCFG_DRIVERS_HIDMAC */

static int pl011_start_tx(struct uart_driver_data *udd, const char *buf, size_t count)
{
    unsigned int fr = 0;
    unsigned int tx_len = count;
    int idx = 0;
    struct arm_pl011_port *port = NULL;

    if (NULL == udd) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }
    port = (struct arm_pl011_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    while(tx_len) {
        /* if tx fifo is not full, write to fifo */
        do {
            fr = readb(port->phys_base + UART_FR);
        } while(fr & UART_FR_TXFF);

        writeb(*(buf + idx), port->phys_base + UART_DR);
        idx++;
        tx_len--;
    }
    return count;
}

static int pl011_config(struct uart_driver_data *udd)
{
    unsigned int fr = 0;
    struct arm_pl011_port *port = NULL;

    if (NULL == udd) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }
    port = (struct arm_pl011_port *)udd->private;
    /* wait for send finish */
    do {
        fr = readb(port->phys_base + UART_FR);
        if (!(fr & UART_FR_BUSY))
            break;
        msleep(10);
    } while(1);

    return pl011_config_in(udd);
}

static struct uart_ops pl011_uops = {
    .startup        = pl011_startup,
    .shutdown       = pl011_shutdown,
#if defined(LOSCFG_DRIVERS_HIDMAC) || defined(LOSCFG_DRIVERS_HIEDMAC)
    .dma_startup    = pl011_dma_startup,
    .dma_shutdown   = pl011_dma_shutdown,
#endif /* LOSCFG_DRIVERS_HIDMAC */
    .start_tx       = pl011_start_tx,
    .config         = pl011_config,
#ifdef HI_DMA_AVERSE_SCHEME
    .priv_operator  = s3_timer_set,
#endif
};

#define MAX_DEV_NAME_SIZE  32
extern const struct file_operations_vfs uartdev_fops;
static int pl011_probe(struct platform_device *pdev)
{
    struct resource *res = NULL;
    char dev_name[MAX_DEV_NAME_SIZE];
    struct arm_pl011_port *port;
    struct uart_driver_data *udd = (struct uart_driver_data *)dev_get_drvdata(&pdev->dev);

    if (NULL == udd) {
        uart_error("get uart driver data err!");
        return -1;
    }
    port = (struct arm_pl011_port *)LOS_MemAlloc(m_aucSysMem0, sizeof(struct arm_pl011_port));
    if(!port)
        return -1;
    memset(port, 0, sizeof(struct arm_pl011_port));
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(!res)
        goto err;
    port->phys_base = (unsigned long)platform_ioremap_resource(res);
    port->irq_num = platform_get_irq(pdev, 0);
    if(port->irq_num == LOS_NOK)
        goto err;

    udd->private = port;
    udd->ops = &pl011_uops;
    port->udd = udd;
    udd->recv = uart_recv_notify;
    udd->count = 0;

    memset(dev_name, 0, MAX_DEV_NAME_SIZE);
    snprintf(dev_name, MAX_DEV_NAME_SIZE, "/dev/uartdev-%d", udd->num);
    if (register_driver(dev_name, &uartdev_fops, 0666, udd)){
        uart_error("gen /dev/uartdev-%d fail!\n", udd->num);
        goto err;
    }

    return 0;
err:
    (VOID)LOS_MemFree(m_aucSysMem0, port);
    port =  NULL;
    return -1;
}

static int pl011_suspend(struct device *dev)
{
    unsigned int cr, imsc;
    struct uart_driver_data *udd = (struct uart_driver_data *)dev_get_drvdata(dev);
    struct arm_pl011_port *port;

    if (!udd || (UART_STATE_USEABLE != udd->state) || (!udd->private)) {
         return -1;
    }
    port = udd->private;
        /* disable uart */
    cr = readw(port->phys_base + UART_CR);
    cr &= ~(UART_CR_RX_EN | UART_CR_TX_EN | UART_CR_EN);
    writew(cr, port->phys_base + UART_CR);
    imsc = readw(port->phys_base + UART_IMSC);
    imsc &= ~(UART_MIS_TIMEOUT | UART_MIS_RX);
    writew(imsc, port->phys_base + UART_IMSC);
    LOS_TaskLock();
    uart_clk_cfg(udd->num, false);
    LOS_TaskUnlock();
    udd->state = UART_STATE_SUSPENED;
#ifdef LOSCFG_DRIVERS_HIDMAC
    if (udd->flags & UART_FLG_DMA_RX) {
        hiedmac_suspend(port->rx_udt->channel);
    }
#endif /* LOSCFG_DRIVERS_HIDMAC */
    return 0;
}
static int pl011_resume(struct device *dev)
{
    struct uart_driver_data *udd = (struct uart_driver_data *)dev_get_drvdata(dev);
    struct arm_pl011_port *port;
    unsigned int imsc;

    if (!udd || (UART_STATE_SUSPENED != udd->state) || (!udd->private)) {
        return -1;
    }
    port = udd->private;
    pl011_startup(udd);

    hal_interrupt_unmask(port->irq_num);
#ifdef LOSCFG_DRIVERS_HIDMAC
    if (udd->flags & UART_FLG_DMA_RX) {
        hiedmac_resume(port->rx_udt->channel);
        writel(UART_DMACR_RX, port->phys_base + UART_DMACR);
    } else
#endif /* LOSCFG_DRIVERS_HIDMAC */
    {
        imsc = readw(port->phys_base + UART_IMSC);
        imsc |= (UART_MIS_TIMEOUT | UART_MIS_RX);
        writew(imsc, port->phys_base + UART_IMSC);
    }
    udd->state = UART_STATE_USEABLE;
    return 0;
}
static const struct dev_pm_op pl011_dev_pm_ops = {
    .suspend = pl011_suspend,
    .resume = pl011_resume,
};
static int pl011_remove(struct platform_device *dev)
{
    return 0;
}
static struct platform_driver pl011_driver = {
    .probe      = pl011_probe,
    .remove     = pl011_remove,
    .driver     = {
        .name   = "uart-pl011",
        .pm = &pl011_dev_pm_ops,
    },
};
int pl011_init(void)
{
    return platform_driver_register(&pl011_driver);
}

void pl011_exit(void)
{
    platform_driver_unregister(&pl011_driver);
}
module_init(pl011_init);
module_exit(pl011_exit);

int uart_suspend(void *data)
{
    struct device *pDevice = NULL;
    LOS_DL_LIST_FOR_EACH_ENTRY(pDevice, &(pl011_driver.driver.device_list), struct device, driver_node) /*lint !e413*/
        if(pDevice->driver->pm->suspend)
            (void)pDevice->driver->pm->suspend(pDevice);
    return 0;
}
int uart_resume(void *data)
{
    struct device *pDevice = NULL;
    LOS_DL_LIST_FOR_EACH_ENTRY(pDevice, &(pl011_driver.driver.device_list), struct device, driver_node) /*lint !e413*/
        if(pDevice->driver->pm->resume)
            (void)pDevice->driver->pm->resume(pDevice);
    return 0;
}
int uart_dev_init(void)
{
    return pl011_init();
}
