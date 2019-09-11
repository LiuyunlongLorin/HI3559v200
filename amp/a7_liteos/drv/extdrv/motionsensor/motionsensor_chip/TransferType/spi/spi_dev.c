#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/delay.h>
//#include <linux/moduleparam.h>

#include <asm/uaccess.h>
#include "motionsensor.h"

#include "spi_dev.h"
#include <linux/spinlock.h>

static unsigned bus_num = 2;
static unsigned csn = 0;
#define REG_WIDTH 1
//static osal_mutex_t spi_mutex;
//static osal_spinlock_t lock;
static spinlock_t lock_rd;

#ifndef __HuaweiLite__
module_param(bus_num, uint, S_IRUGO);
MODULE_PARM_DESC(bus_num, "spi bus number");
module_param(csn, uint, S_IRUGO);
MODULE_PARM_DESC(csn, "chip select number");

struct spi_master *hi_master;
//static DEFINE_MUTEX(spi_mutex);

int ssp_func_write(struct spi_device* hi_spi, HI_U8 u8addr, HI_U8* data, HI_U32 cnt)
{
    struct spi_master* master = hi_master;
    //struct spi_device *spi = hi_spi;
    static struct spi_transfer t;
    static struct spi_message msg;
    static HI_U8 buf[4] = {0};
    HI_S16 s16Ret = 0;
    HI_UL flags;

    if (!hi_spi)
    {
        return -ENODEV;
    }

    /* check spi_message is or no finish */
    spin_lock_irqsave(&master->queue_lock, flags);

    if (msg.state != NULL)
    {
        print_info("msg.state not null!!\n");
        //spin_unlock_irqrestore(&master->queue_lock, flags);
        return -EFAULT;
    }

    spin_unlock_irqrestore(&master->queue_lock, flags);

    //hi_spi->mode = SPI_MODE_3 | SPI_LSB_FIRST;
    //memset(buf, 0, sizeof(buf));
    buf[0] = u8addr;
    buf[0] &= (~0x80);
    //buf[1] = u8addr;
    buf[1] = *data;

    t.tx_buf = buf;
    //t.rx_buf = buf;
    t.len = 2;

    spi_message_init(&msg);
    spi_message_add_tail(&t, &msg);


    msg.state = &msg;
    //s16Ret = spi_async(hi_spi, &msg);
    s16Ret = spi_sync(hi_spi, &msg);

    if (s16Ret)
    {
        print_info( " spi_async() error(%d)!\n", s16Ret);
        return -EAGAIN;
    }

    return s16Ret;
}

HI_S32 ssp_func_read(struct spi_device* hi_spi, HI_U8 u8Addr, HI_U8* data, HI_U32 cnt)
{
    struct spi_master* master = hi_master;
    //struct spi_device *spi = hi_spi;
    static struct spi_transfer t;
    static struct spi_message msg;
    static HI_U8 buf[4] = {0};
    HI_S16 s16Ret = 0;
    HI_UL flags;
    static HI_U8 body[1024] = {0};

    if (!hi_spi)
    {
        return -ENODEV;
    }


    /* check spi_message is or no finish */

    spin_lock_irqsave(&master->queue_lock, flags);

    if (msg.state != NULL)
    {
        print_info("msg.state not null!!(%x)\n", u8Addr);
        spin_unlock_irqrestore(&master->queue_lock, flags);
        return -EFAULT;
    }

    spin_unlock_irqrestore(&master->queue_lock, flags);

    buf[0] = u8Addr;
    buf[0] |= 0x80;
    buf[1] = 0;

    t.tx_buf = buf;
    t.rx_buf = body;
    t.len = cnt + 1;

    spi_message_init(&msg);
    spi_message_add_tail(&t, &msg);

    msg.state = &msg;

    //s16Ret = spi_async(hi_spi, &msg);
    s16Ret = spi_sync(hi_spi, &msg);

    if (s16Ret)
    {
        print_info( " spi_async() error(%d)!\n", s16Ret);
        return -EAGAIN;
    }

    //*data = buf[1];
    memcpy(data, body + 1, cnt);
    return s16Ret;
}

HI_S32 MotionSersor_SPI_write(struct spi_device* hi_spi, HI_U8 u8Addr, HI_U8* data, HI_U32 u32cnt)
{
    return ssp_func_write(hi_spi, u8Addr, data, u32cnt);
}

HI_S32 MotionSersor_SPI_read(struct spi_device* hi_spi, HI_U8 u8Addr, HI_U8* data, HI_U32 u32cnt)
{
    HI_S16 s16Ret;

    //osal_mutex_lock(&spi_mutex);
    s16Ret = ssp_func_read(hi_spi, u8Addr, data, u32cnt);
    //osal_mutex_unlock(&spi_mutex);
    return s16Ret;
}


extern struct bus_type spi_bus_type;
HI_S32 MotionSersor_SPI_init(struct spi_device** hi_spi)
{
    int status = 0;
    struct device* d;
    char* spi_name;

    hi_master = spi_busnum_to_master(bus_num);

    if (hi_master)
    {
        spi_name = osal_kmalloc(strlen(dev_name(&hi_master->dev)) + 10 , osal_gfp_kernel);

        if (!spi_name)
        {
            status = -ENOMEM;
            goto ERR0;
        }
        osal_memset(spi_name,0,strlen(dev_name(&hi_master->dev)) + 10);

        sprintf(spi_name, "%s.%u", dev_name(&hi_master->dev), csn);
        print_info("------lorin add spi_name = %s ----\n", spi_name);
        d = bus_find_device_by_name(&spi_bus_type, NULL, spi_name);

        if (d == NULL)
        {
            status = -ENXIO;
            goto ERR1;
        }
        *hi_spi = to_spi_device(d);

        if (*hi_spi == NULL)
        {
            status = -ENXIO;
            goto ERR2;
        }
    }
    else
    {
        status = -ENXIO;
        goto ERR0;
    }

    status = 0;
    //osal_mutex_init(&spi_mutex);
    spin_lock_init(&lock_rd);
ERR2:
    put_device(d);
ERR1:
    osal_kfree(spi_name);
ERR0:
    return status;
}

HI_S32 MotionSersor_SPI_deinit(struct spi_device* spi_device)
{
    //osal_mutex_destory(&spi_mutex);
    return HI_SUCCESS;
}
#else

int ssp_func_read(HI_U8 u8Addr, HI_U8* data, HI_U32 u32cnt, HI_U32 u32SpiNum)
{
    HI_U8 buf[0x4] = {0};
    static HI_U8 body[1024] = {0};
    static struct spi_ioc_transfer transfer[1];
    int retval = 0;

    transfer[0].tx_buf = buf;
    transfer[0].rx_buf = body;
    transfer[0].len = REG_WIDTH + u32cnt;
    transfer[0].cs_change = 1;
    transfer[0].speed = 10000000;
    memset(buf, 0, sizeof(buf));

    buf[0] = (u8Addr & 0xff) | 0x80 ;
    buf[1] = 0x0;

    retval = spi_dev_set(u32SpiNum, csn, &transfer[0]);

    osal_memcpy(data, body + 1, u32cnt);

    if (retval == transfer[0].len)
    { return HI_SUCCESS; }

    return HI_FAILURE;
    //return retval ;
}



int ssp_func_write(HI_U8 u8Addr, HI_U8* data, HI_U32 u32cnt, HI_U32 u32SpiNum)
{
    unsigned char buf[0x10];
    struct spi_ioc_transfer transfer[1];
    int retval = 0;
    transfer[0].tx_buf = buf;
    transfer[0].rx_buf = buf;
    transfer[0].len = REG_WIDTH + u32cnt;
    transfer[0].cs_change = 1;
    transfer[0].speed = 10000000;
    memset(buf, 0, sizeof(buf));

    buf[0] = (u8Addr & 0xff) & (~0x80) ;

    buf[1] = *data;

    retval = spi_dev_set(u32SpiNum, csn, &transfer[0]);

    if (retval == transfer[0].len)
    { return HI_SUCCESS; }

    return HI_FAILURE;
}


HI_S32 MotionSersor_SPI_write(HI_U8 u8Addr, HI_U8* data, HI_U32 u32cnt, HI_U32 u32SpiNum)
{
    return ssp_func_write(u8Addr, data, u32cnt, u32SpiNum);
}

HI_S32 MotionSersor_SPI_read(HI_U8 u8Addr, HI_U8* data, HI_U32 u32cnt, HI_U32 u32SpiNum)
{
    HI_S32 s32Ret;
	HI_UL flags;

    spin_lock_irqsave(&lock_rd, flags);
    s32Ret = ssp_func_read(u8Addr, data, u32cnt, u32SpiNum);
	spin_unlock_irqrestore(&lock_rd, flags);

    return s32Ret;
}

HI_S32 MotionSersor_SPI_init(HI_VOID)
{
    spin_lock_init(&lock_rd);
    return HI_SUCCESS;
}

HI_S32 MotionSersor_SPI_deinit(HI_VOID)
{

    return HI_SUCCESS;
}

#endif


