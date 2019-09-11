#include "errno.h"
#include "string.h"
#include "uart.h"
#include "uart_dev.h"

int uart_dev_read(struct uart_driver_data *udd, char *buf, size_t count)
{
    struct uart_ioc_transfer *transfer = udd->rx_transfer;
    unsigned int wp, rp;
    unsigned long data;
    if (NULL == transfer) {
        uart_error("transfer is null");
        return -EFAULT;
    }
    wp = transfer->wp;
    rp = transfer->rp;
    data = (unsigned long)transfer->data;

    if (!(transfer->flags & BUF_CIRCLED)) {
        if (count >= (wp - rp)) {
            count = wp - rp;
        }
        memcpy(buf, (void *)(data + rp), count);
        transfer->rp += count;
        return count;
    } else {
        if (count < (BUF_SIZE - rp)) {
            memcpy(buf, (void *)(data + rp), count);
            transfer->rp += count;
            return count;
        } else {
            unsigned int copy_size = BUF_SIZE - rp;
            unsigned int left_size = count - copy_size;
            memcpy(buf, (void *)(data + rp), copy_size);
            rp = 0;
            if (left_size > wp) {
                left_size = wp;
            }
            memcpy((void *)(buf + copy_size), (void *)(data + rp), left_size);
            transfer->rp = left_size;
            transfer->flags &= ~BUF_CIRCLED;
            return (copy_size + left_size);
        }
    }
}

static int notify(struct wait_queue_head *wait)
{
    if (NULL == wait) {
        return -EINVAL;
    }

    (void *)LOS_EventWrite(&wait->stEvent, 0x1);
    notify_poll(wait);
    return 0;
}

int uart_recv_notify(struct uart_driver_data *udd, const char *buf, size_t count)
{
    struct uart_ioc_transfer *transfer = udd->rx_transfer;
    unsigned int wp, rp;
    unsigned long data;
    if (NULL == transfer) {
        uart_error("transfer is null");
        return -EFAULT;
    }
    wp = transfer->wp;
    rp = transfer->rp;
    data = (unsigned long)transfer->data;

    if (!(transfer->flags & BUF_CIRCLED)) {
        if (count < (BUF_SIZE - wp)) {
            memcpy((void *)(data + wp), buf, count);
            transfer->wp += count;
            notify(&udd->wait);
            return count;
        } else {
            unsigned int copy_size = BUF_SIZE - wp;
            unsigned int left_size = count - copy_size;
            memcpy((void *)(data + wp), buf, copy_size);
            wp = 0;
            if (left_size > rp) {
                /* overflowed. some new data will be discarded */
                uart_error("[%d]rx buf overflow", udd->num);
                left_size = rp;
            }
            memcpy((void *)(data + wp), (void *)(buf + copy_size), left_size);
            transfer->wp = left_size;
            transfer->flags |= BUF_CIRCLED;
            notify(&udd->wait);
            return (copy_size + left_size);
        }
    } else {
        if (count > (rp - wp)) {
            /* overflowed. some new data will be discarded */
            uart_error("[%d]rx buf overflow", udd->num);
            count = rp - wp;
        }
        memcpy((void *)(data + wp), buf, count);
        transfer->wp += count;
        notify(&udd->wait);
        return count;
    }
}

int uart_rx_buf_empty(struct uart_driver_data *udd)
{
    struct uart_ioc_transfer *transfer = udd->rx_transfer;
    return (transfer->wp == transfer->rp);
}

