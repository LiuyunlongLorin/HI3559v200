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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"

#include "fs/fs.h"
#include "fcntl.h"

#include "hisoc/uart.h"
#include "poll.h"

#include "uart.h"
#include "uart_dev.h"

static int uartdev_open(FAR struct file *filep)
{
    int ret = 0;
    struct inode * inode = filep ->f_inode;
    struct wait_queue_head *wait;
    struct uart_driver_data *udd = (struct uart_driver_data *)inode->i_private;
    if (NULL == udd) {
        uart_error("invalid i_private!");
        return -EINVAL;
    }
    wait = &udd->wait;
    if (UART_STATE_NOT_OPENED == udd->state) {
        udd->state = UART_STATE_OPENING;
        (void)LOS_EventInit(&wait->stEvent);
        (VOID)pthread_mutex_init(&wait->mutex, (const pthread_mutexattr_t *)NULL);
        LOS_ListInit(&wait->poll_queue);
        udd->rx_transfer = (struct uart_ioc_transfer *)LOS_MemAlloc(m_aucSysMem0, sizeof(struct uart_ioc_transfer));
        if (NULL == udd->rx_transfer) {
            uart_error("alloc transfer failed!");
            return -ENOMEM;
        }
        memset(udd->rx_transfer, 0, sizeof(struct uart_ioc_transfer));
        if (udd->ops->startup && udd->ops->startup(udd)) {
            uart_error("startup failed...");
            ret = -EFAULT;
            goto free_transfer;
        }
    }
    udd->state = UART_STATE_USEABLE;
    udd->count++;
    return 0;

free_transfer:
    (VOID)LOS_MemFree(m_aucSysMem0, udd->rx_transfer);
    udd->rx_transfer = NULL;
    return ret;
}

static int uartdev_release(FAR struct file *filep)
{
    struct inode * inode = filep -> f_inode ;
    struct wait_queue_head *wait;
    struct uart_driver_data *udd = (struct uart_driver_data *)inode->i_private;
    if (NULL == udd) {
        uart_error("uart_driver_data is NULL");
        return -EINVAL;
    }
    if ((--udd->count) != 0)
        return 0;

    wait = &udd->wait;

    if (udd->flags & UART_FLG_DMA_RX) {
        if (udd->ops->dma_shutdown)
            udd->ops->dma_shutdown(udd, UART_DMA_DIR_RX);
    }
    if (udd->flags & UART_FLG_DMA_TX) {
        if (udd->ops->dma_shutdown)
            udd->ops->dma_shutdown(udd, UART_DMA_DIR_TX);
    }

    LOS_ListDelete(&wait->poll_queue);
    pthread_mutex_destroy(&wait->mutex);
    (void *)LOS_EventDestroy(&wait->stEvent);

    if (udd->ops->shutdown)
        udd->ops->shutdown(udd);

    if (udd->rx_transfer)
        (VOID)LOS_MemFree(m_aucSysMem0, udd->rx_transfer);
    udd->rx_transfer = NULL;
    udd->state = UART_STATE_NOT_OPENED;

    return 0;
}

static ssize_t uartdev_read(FAR struct file *filep, FAR char *buf, size_t count)
{
    int ret = 0;
    struct inode * inode = filep->f_inode;

    struct uart_driver_data *udd = (struct uart_driver_data *)inode->i_private;
    if (NULL == udd) {
        uart_error("uart_driver_data is NULL");
        return -EINVAL;
    }
    if (UART_STATE_USEABLE != udd->state)
        return -EFAULT;

    if ((udd->flags & UART_FLG_RD_BLOCK)
            && (uart_rx_buf_empty(udd))) {
        (void)LOS_EventRead(&udd->wait.stEvent,
                0x1, LOS_WAITMODE_OR, LOS_WAIT_FOREVER);
    }

    ret = uart_dev_read(udd, buf, count);

    if ((udd->flags & UART_FLG_RD_BLOCK)
            && (uart_rx_buf_empty(udd))) {
        (void)LOS_EventClear(&udd->wait.stEvent, ~(0x1));
    }
    return ret;
}

static ssize_t uartdev_write(struct file *filep, const char *buf, size_t count)
{
    int ret = 0;
    struct inode * inode = filep->f_inode;
    struct uart_driver_data *udd = (struct uart_driver_data *)inode->i_private;
    if (NULL == udd) {
        uart_error("uart_driver_data is NULL");
        return -EINVAL;
    }

    if (UART_STATE_USEABLE != udd->state)
        return -EFAULT;

    if (udd->ops->start_tx)
        ret = udd->ops->start_tx(udd, buf, count);

    return ret;
}

static int uartdev_ioctl(FAR struct file *filep, int cmd, unsigned long arg)
{
    int ret = 0;
    struct inode * inode = filep ->f_inode;
    struct uart_driver_data *udd = (struct uart_driver_data *)inode->i_private;

    if (NULL == udd) {
        uart_error("uart_driver_data is NULL");
        return -EINVAL;
    }

    if (UART_STATE_USEABLE != udd->state)
        return -EFAULT;

    switch (cmd) {
        case UART_CFG_BAUDRATE:
            /*
             * baudrate should be between 0 and CONFIG_MAX_BAUDRATE
             */
            if ((arg <= CONFIG_MAX_BAUDRATE) && (arg > 0)) {
                udd->baudrate = arg;
                if (udd->ops->config && udd->ops->config(udd)) {
                    uart_error("invalid  baudrate, which is:%d\n", arg);
                }
            } else {
                uart_error("invalid  baudrate, which is:%d\n", arg);
                ret = -EINVAL;
            }
            break;

        case UART_CFG_DMA_RX:
            if (UART_DMA_RX_EN == arg) {
                if (!(udd->flags & UART_FLG_DMA_RX)) {
                    if (udd->ops->dma_startup) {
                        ret = udd->ops->dma_startup(udd, UART_DMA_DIR_RX);
                        if (!ret)
                            udd->flags |= UART_FLG_DMA_RX;
                        else
                            uart_error("dma startup failed!");
                    } else {
                        uart_error("dma receive not supported!");
                    }
                }
            } else if (UART_DMA_RX_DIS == arg) {
                if (udd->ops->dma_shutdown) {
                    udd->ops->dma_shutdown(udd, UART_DMA_DIR_RX);
                    udd->flags &= ~UART_FLG_DMA_RX;
                } else {
                    uart_error("dma receive not supported!");
                }
            } else {
                uart_error("invalid parameter!");
                return -EINVAL;
            }
            break;

        case UART_CFG_DMA_TX:
            if (UART_DMA_TX_EN == arg) {
                if (!(udd->flags & UART_FLG_DMA_TX)) {
                    if (udd->ops->dma_startup) {
                        ret = udd->ops->dma_startup(udd, UART_DMA_DIR_TX);
                        if (!ret)
                            udd->flags |= UART_FLG_DMA_TX;
                        else
                            uart_error("dma startup failed!");
                    } else {
                        uart_error("dma send not supported!");
                    }
                }
            } else if (UART_DMA_TX_DIS == arg) {
                if (udd->ops->dma_shutdown) {
                    udd->ops->dma_shutdown(udd, UART_DMA_DIR_TX);
                    udd->flags &= ~UART_FLG_DMA_TX;
                } else {
                    uart_error("dma send not supported!");
                }
            } else {
                uart_error("invalid parameter!");
                return -EINVAL;
            }
            break;
        case UART_CFG_RD_BLOCK:
            if (UART_RD_BLOCK == arg)
                udd->flags |= UART_FLG_RD_BLOCK;
            else if (UART_RD_NONBLOCK == arg) {
                udd->flags &= ~UART_FLG_RD_BLOCK;
                (void)LOS_EventWrite(&udd->wait.stEvent, 0x1);
            }
            break;

        case UART_CFG_ATTR:
            memcpy((void *)&udd->attr, (void *)arg, sizeof(struct uart_attr));
            if (udd->ops->config
                    && udd->ops->config(udd)) {
                uart_error("config failed!");
                ret = -EFAULT;
            }
            break;
        case UART_CFG_PRIVATE:
            if (udd->ops->priv_operator)
                ret = udd->ops->priv_operator(udd, (void *)arg);
        break;
            /* add more configs */

        default:
            uart_error("unknow ioctl cmd:%d\n", cmd);
            ret = -EINVAL;
            break;
    }
    return ret;
}

extern void poll_wait(struct file *filp,
        wait_queue_head_t *wait_address, poll_table *p);
static int uartdev_poll(struct file *filep, poll_table *table)
{
    struct inode * inode = filep ->f_inode;
    struct uart_driver_data *udd = (struct uart_driver_data *)inode->i_private;
    if (NULL == udd) {
        uart_error("uart_driver_data is NULL");
        return -EINVAL;
    }
    if (UART_STATE_USEABLE != udd->state)
        return -EFAULT;

    poll_wait(filep, &udd->wait, table);

    if (!uart_rx_buf_empty(udd)) {
        return POLLIN | POLLRDNORM;
    }
    return 0;
}

const struct file_operations_vfs uartdev_fops =
{
    .open   = uartdev_open,
    .close  = uartdev_release,
    .read   = uartdev_read,
    .write  = uartdev_write,
    .seek   = NULL,
    .ioctl  = uartdev_ioctl,
#ifndef CONFIG_DISABLE_POLL
    .poll   = uartdev_poll,
#endif
    .unlink = NULL
};

