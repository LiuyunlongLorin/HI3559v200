/*  ssp_sony.c
 *
 * Copyright (c) 2016 Hisilicon Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 * History:
 *      21-April-2006 create this file
 *      21- June-2016 support multi device, and adapt for HuaweiLite
 */

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#ifndef __HuaweiLite__
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#ifndef CONFIG_HISI_SNAPSHOT_BOOT
#include <linux/miscdevice.h>
#endif

#include <linux/proc_fs.h>
#include <linux/poll.h>

#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>

#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
#include "himedia.h"
static struct himedia_device s_stSspDevice;
#endif

#define SSP_DBG_ERR     KERN_ALERT
#define SSP_DBG_INFO    KERN_DEBUG
#define SSP_TRACE(level, fmt, ...) printk(level fmt, ##__VA_ARGS__)

#else

#ifndef __iomem
#define __iomem
#endif

#define SSP_DBG_ERR     "err"
#define SSP_DBG_INFO    "inf"
#define SSP_TRACE(level, fmt, ...)   dprintf(fmt, ##__VA_ARGS__)
#endif

#include "hi_ssp.h"
#include "isp_ext.h"

#define DEV_NAME "ssp"
#define MAX_ISP_DEV_NUM     6
#define SSP_DEV_NUM         5

#define  ssp_readw(addr,ret)            (ret =(*(volatile unsigned int *)(addr)))
#define  ssp_writew(addr,value)         ((*(volatile unsigned int *)(addr)) = (value))

#define  HI_REG_READ(addr,ret)          (ret =(*(volatile unsigned int *)(addr)))
#define  HI_REG_WRITE(addr,value)       ((*(volatile unsigned int *)(addr)) = (value))

#define SSP_BASE    0x12120000
#define SSP_SIZE    0x10000              // 64KB
#define SSP_INT     41                  // Interrupt No.

static void __iomem *reg_ssp_base_va;
#define IO_ADDRESS_VERIFY(x) ((void __iomem *)((unsigned char __iomem *)reg_ssp_base_va + ((x)-(SSP_BASE))))


/* SSP register definition .*/
#define SSP_CR0(ssp_no)     IO_ADDRESS_VERIFY(SSP_BASE + 0x00 + (ssp_no * 0x1000))
#define SSP_CR1(ssp_no)     IO_ADDRESS_VERIFY(SSP_BASE + 0x04 + (ssp_no * 0x1000))
#define SSP_DR(ssp_no)      IO_ADDRESS_VERIFY(SSP_BASE + 0x08 + (ssp_no * 0x1000))
#define SSP_SR(ssp_no)      IO_ADDRESS_VERIFY(SSP_BASE + 0x0C + (ssp_no * 0x1000))
#define SSP_CPSR(ssp_no)    IO_ADDRESS_VERIFY(SSP_BASE + 0x10 + (ssp_no * 0x1000))
#define SSP_IMSC(ssp_no)    IO_ADDRESS_VERIFY(SSP_BASE + 0x14 + (ssp_no * 0x1000))
#define SSP_RIS(ssp_no)     IO_ADDRESS_VERIFY(SSP_BASE + 0x18 + (ssp_no * 0x1000))
#define SSP_MIS(ssp_no)     IO_ADDRESS_VERIFY(SSP_BASE + 0x1C + (ssp_no * 0x1000))
#define SSP_ICR(ssp_no)     IO_ADDRESS_VERIFY(SSP_BASE + 0x20 + (ssp_no * 0x1000))
#define SSP_DMACR(ssp_no)   IO_ADDRESS_VERIFY(SSP_BASE + 0x24 + (ssp_no * 0x1000))

//unsigned int ssp_dmac_rx_ch,ssp_dmac_tx_ch;

static spinlock_t g_stSspSonyLock;
#define SSP_SPIN_LOCK_INIT()    spin_lock_init(&g_stSspSonyLock)
#define SSP_SPIN_LOCK(flags)    spin_lock_irqsave(&g_stSspSonyLock, flags)
#define SSP_SPIN_UNLOCK(flags)  spin_unlock_irqrestore(&g_stSspSonyLock, flags)

static void hi_ssp_disable(unsigned int ssp_no)
{
    int ret = 0;
    ssp_readw(SSP_CR1(ssp_no), ret);
    ret = ret & (~(0x1 << 1));
    ssp_writew(SSP_CR1(ssp_no), ret);
}

/*
 * set SSP frame form routine.
 *
 * @param framemode: frame form
 * 00: Motorola SPI frame form.
 * when set the mode,need set SSPCLKOUT phase and SSPCLKOUT voltage level.
 * 01: TI synchronous serial frame form
 * 10: National Microwire frame form
 * 11: reserved
 * @param sphvalue: SSPCLKOUT phase (0/1)
 * @param sp0: SSPCLKOUT voltage level (0/1)
 * @param datavalue: data bit
 * 0000: reserved    0001: reserved    0010: reserved    0011: 4bit data
 * 0100: 5bit data   0101: 6bit data   0110:7bit data    0111: 8bit data
 * 1000: 9bit data   1001: 10bit data  1010:11bit data   1011: 12bit data
 * 1100: 13bit data  1101: 14bit data  1110:15bit data   1111: 16bit data
 *
 * @return value: 0--success; -1--error.
 *
 */
static int hi_ssp_set_frameform(unsigned int ssp_no, unsigned char framemode, unsigned char spo, unsigned char sph, unsigned char datawidth)
{
    int ret = 0;
    ssp_readw(SSP_CR0(ssp_no), ret);
    if (framemode > 3)
    {
        SSP_TRACE(SSP_DBG_ERR, "set frame parameter err.\n");
        return -1;
    }
    ret = (ret & 0xFFCF) | (framemode << 4);
    if ((ret & 0x30) == 0)
    {
        if (spo > 1)
        {
            SSP_TRACE(SSP_DBG_ERR, "set spo parameter err.\n");
            return -1;
        }
        if (sph > 1)
        {
            SSP_TRACE(SSP_DBG_ERR, "set sph parameter err.\n");
            return -1;
        }
        ret = (ret & 0xFF3F) | (sph << 7) | (spo << 6);
    }
    if ((datawidth > 16) || (datawidth < 4))
    {
        SSP_TRACE(SSP_DBG_ERR, "set datawidth parameter err.\n");
        return -1;
    }
    ret = (ret & 0xFFF0) | (datawidth - 1);
    ssp_writew(SSP_CR0(ssp_no), ret);
    return 0;
}

/*
 * set SSP serial clock rate routine.
 *
 * @param scr: scr value.(0-255,usually it is 0)
 * @param cpsdvsr: Clock prescale divisor.(2-254 even)
 *
 * @return value: 0--success; -1--error.
 *
 */
static int hi_ssp_set_serialclock(unsigned int ssp_no, unsigned char scr, unsigned char cpsdvsr)
{
    int ret = 0;
    ssp_readw(SSP_CR0(ssp_no), ret);
    ret = (ret & 0xFF) | (scr << 8);
    ssp_writew(SSP_CR0(ssp_no), ret);
    if ((cpsdvsr & 0x1))
    {
        SSP_TRACE(SSP_DBG_ERR, "set cpsdvsr parameter err.\n");
        return -1;
    }
    ssp_writew(SSP_CPSR(ssp_no), cpsdvsr);
    return 0;
}

static int hi_ssp_alt_mode_set(unsigned int ssp_no, int enable)
{
    int ret = 0;

    ssp_readw(SSP_CR1(ssp_no), ret);
    if (enable)
    {
        ret = ret & (~0x40);
    }
    else
    {
        ret = (ret & 0xFF) | 0x40;
    }
    ssp_writew(SSP_CR1(ssp_no), ret);

    return 0;
}


static unsigned int hi_ssp_is_fifo_empty(unsigned int ssp_no, int bSend)
{
    int ret = 0;
    ssp_readw(SSP_SR(ssp_no), ret);

    if (bSend)
    {
        if ((ret & 0x1) == 0x1) /* send fifo */
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if ((ret & 0x4) == 0x4) /* receive fifo */
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

static void spi_enable(unsigned int ssp_no)
{
    // little endian
    HI_REG_WRITE(SSP_CR1(ssp_no), 0x42);
}

static void spi_disable(unsigned int ssp_no)
{
    HI_REG_WRITE(SSP_CR1(ssp_no), 0x40);
}


// Fsspclkout = Fsspclk/(cpsdvsr*(1+scr))
int spo = 1;
int sph = 1;

#ifndef __HuaweiLite__

module_param(spo, int, S_IRUGO);
module_param(sph, int, S_IRUGO);
#endif

static int hi_ssp_init_cfg(unsigned int ssp_no)
{
    unsigned char framemode = 0;
    unsigned char spo = 0;
    unsigned char sph = 0;
    unsigned char datawidth = 8 ;

#ifdef HI_FPGA
    unsigned char scr = 1;
    unsigned char cpsdvsr = 2;
#else
    unsigned char scr = 1;  //8
    unsigned char cpsdvsr = 8;
#endif

    spi_disable(ssp_no);

    if (0 == ssp_no)
    {
        spo = 0;
        sph = 0;
        hi_ssp_set_frameform(ssp_no, framemode, spo, sph, datawidth);
    }
    else
    {
        spo = 1;
        sph = 1;
        hi_ssp_set_frameform(ssp_no, framemode, spo, sph, datawidth);
    }

    hi_ssp_set_serialclock(ssp_no, scr, cpsdvsr);

    // altasens mode, which CS won't be pull high between 16bit data transfer
    hi_ssp_alt_mode_set(ssp_no, 0);

    //hi_ssp_enable(ssp_no);

    return 0;
}


static unsigned short hi_ssp_read_alt(unsigned int ssp_no, unsigned short devaddr, unsigned short addr_high, unsigned short addr_low)
{
    unsigned int ret;
    unsigned short value = 0;
    unsigned short dontcare = 0x00;
    unsigned long flags;

    SSP_SPIN_LOCK(flags);

    spi_enable(ssp_no);
    //ssp_writew(SSP_DR(ssp_no), devaddr);
    //ssp_writew(SSP_DR(ssp_no), addr_high);

    if (0 == ssp_no)
    {
        ssp_writew(SSP_DR(ssp_no), (addr_low & 0x7f));
    }
    else
    {
        ssp_writew(SSP_DR(ssp_no), ((addr_low << 1) | 0x01));
    }


    ssp_writew(SSP_DR(ssp_no), dontcare);

    while (hi_ssp_is_fifo_empty(ssp_no, 0)) {};
    ssp_readw(SSP_DR(ssp_no), ret);

    //printk("FUNC:%s LINE:%d addr_low=0x%x ret=0x%x\n",__FUNCTION__,__LINE__,(addr_low & 0x7f), ret);
    while (hi_ssp_is_fifo_empty(ssp_no, 0)) {};
    ssp_readw(SSP_DR(ssp_no), ret);

    //printk("FUNC:%s LINE:%d ret=0x%x\n",__FUNCTION__,__LINE__,ret);
    //    while(hi_ssp_is_fifo_empty(ssp_no, 0)){};
    //  ssp_readw(SSP_DR(ssp_no), ret);
    //
    //    while(hi_ssp_is_fifo_empty(ssp_no, 0)){};
    //  ssp_readw(SSP_DR(ssp_no), ret);

    spi_disable(ssp_no);
    value = (unsigned short)(ret & 0xff);

    SSP_SPIN_UNLOCK(flags);

    return value;
}

static int hi_ssp_write_alt(unsigned int ssp_no, unsigned short devaddr, unsigned short addr_high, unsigned short addr_low, unsigned short data)
{
    unsigned int ret;
    unsigned long flags;

    SSP_SPIN_LOCK(flags);

    spi_enable(ssp_no);

    //ssp_writew(SSP_DR(ssp_no), devaddr);
    //ssp_writew(SSP_DR(ssp_no), addr_high);

    if (0 == ssp_no)
    {
        ssp_writew(SSP_DR(ssp_no), (addr_low | 0x80));
    }
    else
    {
        ssp_writew(SSP_DR(ssp_no), ((addr_low << 1) & 0xfe));
    }


    ssp_writew(SSP_DR(ssp_no), data);

    //printk("====FUNC:%s LINE:%d addr_low=0x%x data=0x%x\n",__FUNCTION__,__LINE__,(addr_low | 0x80),data);
    // wait receive fifo has data
    while (hi_ssp_is_fifo_empty(ssp_no, 0)) {};
    ssp_readw(SSP_DR(ssp_no), ret);

    //printk("FUNC:%s LINE:%d ret=0x%x\n",__FUNCTION__,__LINE__,ret);
    // wait receive fifo has data
    while (hi_ssp_is_fifo_empty(ssp_no, 0)) {};
    ssp_readw(SSP_DR(ssp_no), ret);

    //printk("FUNC:%s LINE:%d ret=0x%x\n",__FUNCTION__,__LINE__,ret);
    //while(hi_ssp_is_fifo_empty(ssp_no, 0)){};
    //ssp_readw(SSP_DR(ssp_no), ret);
    //
    //   // wait receive fifo has data
    //while(hi_ssp_is_fifo_empty(ssp_no, 0)){};
    //ssp_readw(SSP_DR(ssp_no), ret);

    spi_disable(ssp_no);

    SSP_SPIN_UNLOCK(flags);

    ret = 0;
    return ret;
}

static int hi_ssp_write(unsigned int ssp_no, unsigned int spi_csn,
                        unsigned int addr1, unsigned int addr1bytenum,
                        unsigned int addr2, unsigned int addr2bytenum,
                        unsigned int data , unsigned int databytenum)
{
    unsigned short spi_no, devaddr, addr_high, addr_low, data_value;

    spi_no = (unsigned short)(ssp_no & 0xff);
    devaddr   = (unsigned short)(addr1 & 0xff);
    addr_high = (unsigned short)((addr2 & 0xff00) >> 8);
    addr_low  = (unsigned short)(addr2 & 0xff);
    data_value = (unsigned short)(data & 0xff);

    return hi_ssp_write_alt(spi_no, devaddr, addr_high, addr_low, data_value);
}

#ifdef __HuaweiLite__
static int ssp_ioctl(struct file *file, int cmd, unsigned long arg)
#else
static long ssp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    unsigned short spi_no, devaddr, addr_high, addr_low, data;
    SPI_DATA_S __user *argp = (SPI_DATA_S __user *)arg;
    SPI_DATA_S spi_data;

    if (copy_from_user(&spi_data, argp, sizeof(SPI_DATA_S)))
    {
        return -EFAULT;
    }

    switch (cmd)
    {
        case SSP_READ_ALT:
            //MN34041 LSB first!
            spi_no = ((unsigned short)(spi_data.spi_no & 0xff));
            devaddr = ((unsigned short)(spi_data.dev_addr & 0xff));
            addr_high = ((unsigned short)((spi_data.reg_addr & 0xff00) >> 8));
            addr_low = ((unsigned short)(spi_data.reg_addr & 0xff));

            if (spi_no >= SSP_DEV_NUM)
            {
                SSP_TRACE(SSP_DBG_ERR, "Kernel: spi_no[%u] is out of range!\n", spi_no);
                return -1;
            }

            data = hi_ssp_read_alt(spi_no, devaddr, addr_high, addr_low);

            spi_data.data = (unsigned int)(data & 0xff);
            if (copy_to_user(argp, &spi_data, sizeof(SPI_DATA_S)))
            {
                return -EFAULT;
            }
            break;

        case SSP_WRITE_ALT:
            spi_no = (unsigned short)(spi_data.spi_no & 0xff) ;
            devaddr = (unsigned short)(spi_data.dev_addr & 0xff) ;
            addr_high = (unsigned short)((spi_data.reg_addr & 0xff00) >> 8) ;
            addr_low = (unsigned short)(spi_data.reg_addr & 0xff) ;
            data    = (unsigned short)(spi_data.data & 0xff) ;

            //printk("FUNC:%s LINE:%d reg_addr=0x%x data=0x%x\n",__FUNCTION__,__LINE__,spi_data.reg_addr,spi_data.data);
            if (spi_no >= SSP_DEV_NUM)
            {
                SSP_TRACE(SSP_DBG_ERR, "Kernel: spi_no[%u] is out of range!\n", spi_no);
                return -1;
            }
            hi_ssp_write_alt(spi_no, devaddr, addr_high, addr_low, data);
            break;

        default:
        {
            SSP_TRACE(SSP_DBG_ERR, "Kernel: No such ssp command %#x!\n", cmd);
            return -1;
        }
    }

    return 0;
}


#ifdef __HuaweiLite__
static int ssp_open(struct file *file)
{
    return 0;
}
static int ssp_close(struct file *file)
{
    return 0;
}
const static struct file_operations_vfs ssp_fops =
{
    .open = ssp_open,
    .close = ssp_close,
    .ioctl = ssp_ioctl
};

int hi_ssp_init(void *pArgs)
{
    unsigned int ssp_no;
    int ret;

    ISP_BUS_CALLBACK_S stBusCb = {0};
    stBusCb.pfnISPWriteSSPData = hi_ssp_write;

    if ((NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)) && CKFN_ISP_RegisterBusCallBack())
    {
        ISP_DEV IspDev;
        for (IspDev = 0;  IspDev < MAX_ISP_DEV_NUM; IspDev++)
        {
            CALL_ISP_RegisterBusCallBack(IspDev, ISP_BUS_TYPE_SSP, &stBusCb);
        }
    }
    else
    {
        SSP_TRACE(SSP_DBG_ERR, "register ssp_write_callback to isp failed, ssp init is failed!\n");
        return -1;
    }

    reg_ssp_base_va = (void __iomem *)IO_ADDRESS((unsigned long)SSP_BASE);

    ret = register_driver("/dev/ssp", &ssp_fops, 0666, 0);
    if (ret)
    {
        SSP_TRACE(SSP_DBG_ERR, "register pwd device failed with %#x!\n", ret);
        return -1;
    }

    for (ssp_no = 0; ssp_no < SSP_DEV_NUM; ssp_no++)
    {
        ret = hi_ssp_init_cfg(ssp_no);
        if (ret)
        {
            SSP_TRACE(SSP_DBG_ERR, "Debug: ssp initial failed!\n");
            return -1;
        }
    }

    SSP_SPIN_LOCK_INIT();
    SSP_TRACE(SSP_DBG_INFO, "Kernel: ssp initial ok!\n");

    return 0;
}

void hi_ssp_exit(void)
{
    unsigned int ssp_no;

    for (ssp_no = 0; ssp_no < SSP_DEV_NUM; ssp_no++)
    {
        hi_ssp_disable(ssp_no);
    }

    reg_ssp_base_va = NULL;
    unregister_driver("/dev/ssp");
}


#else
static int ssp_open(struct inode *inode, struct file *file)
{
    return 0;
}
static int ssp_close(struct inode *inode, struct file *file)
{
    return 0;
}

static struct file_operations ssp_fops =
{
    .owner      = THIS_MODULE,
    .unlocked_ioctl         = ssp_ioctl,
    .open       = ssp_open,
    .release    = ssp_close
};

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
static int ssp_freeze(struct himedia_device *pdev)
{
    SSP_TRACE(SSP_DBG_INFO, "%s  %d\n", __FUNCTION__, __LINE__);

    return 0;
}

static int ssp_restore(struct himedia_device *pdev)
{
    unsigned int ssp_no;
    //  int ret;

    for (ssp_no = 0; ssp_no < SSP_DEV_NUM; ssp_no++)
    {
#if 0
        ret = hi_ssp_init_cfg(ssp_no);
        if (ret)
        {
            SSP_TRACE(SSP_DBG_ERR, "Debug: ssp initial failed!\n");
            return -1;
        }
#else
        hi_ssp_init_cfg(ssp_no);
#endif
    }

    SSP_TRACE(SSP_DBG_INFO, "%s  %d\n", __FUNCTION__, __LINE__);

    return 0;
}
#endif


#ifdef CONFIG_HISI_SNAPSHOT_BOOT
static struct himedia_ops stSspDrvOps =
{
    .pm_freeze = ssp_freeze,
    .pm_restore  = ssp_restore
};
#else
static struct miscdevice ssp_dev =
{
    .minor       = MISC_DYNAMIC_MINOR,
    .name        = DEV_NAME,
    .fops        = &ssp_fops,
};
#endif


/*
 * initializes SSP interface routine.
 *
 * @return value:0--success.
 *
 */
static int __init hi_ssp_init(void)
{
    unsigned int ssp_no;
    int ret;
#if 0
    KCOM_HI_DMAC_INIT();

    if (sspinitialized == 0)
    {
        reg = readl(IO_ADDRESS_VERIFY(0x101E0040));
        reg &= 0xfffcf3ff;
        reg |= 0x00010800;
        writel(reg, IO_ADDRESS_VERIFY(0x101E0040));
        sspinitialized = 1;

        printk("Load hi_ssp.ko success.  \t(%s)\n", VERSION_STRING);

        return 0;
    }
    else
    {
        printk("SSP has been initialized.\n");
        return 0;
    }
#else

    ISP_BUS_CALLBACK_S stBusCb = {0};
    stBusCb.pfnISPWriteSSPData = hi_ssp_write;

    if ((NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)) && CKFN_ISP_RegisterBusCallBack())
    {
        ISP_DEV IspDev;
        for (IspDev = 0;  IspDev < MAX_ISP_DEV_NUM; IspDev++)
        {
            CALL_ISP_RegisterBusCallBack(IspDev, ISP_BUS_TYPE_SSP, &stBusCb);
        }
    }
    else
    {
        SSP_TRACE(SSP_DBG_ERR, "register ssp_write_callback to isp failed, ssp init is failed!\n");
        return -1;
    }

    reg_ssp_base_va = ioremap_nocache((unsigned long)SSP_BASE, (unsigned long)(SSP_SIZE * SSP_DEV_NUM));
    if (!reg_ssp_base_va)
    {
        SSP_TRACE(SSP_DBG_ERR, "Kernel: ioremap ssp base failed!\n");
        return -ENOMEM;
    }

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
    snprintf(s_stSspDevice.devfs_name, sizeof(s_stSspDevice.devfs_name), DEV_NAME);

    s_stSspDevice.minor  = HIMEDIA_DYNAMIC_MINOR;
    s_stSspDevice.fops   = &ssp_fops;
    s_stSspDevice.drvops = &stSspDrvOps;
    s_stSspDevice.owner  = THIS_MODULE;

    ret = himedia_register(&s_stSspDevice);
    if (ret)
    {
        SSP_TRACE(SSP_DBG_ERR, "Kernel: register ssp_0 device failed!");
        return -1;
    }
#else
    ret = misc_register(&ssp_dev);
    if (0 != ret)
    {
        SSP_TRACE(SSP_DBG_ERR, "Kernel: register ssp_0 device failed!\n");
        return -1;
    }
#endif

    for (ssp_no = 0; ssp_no < SSP_DEV_NUM; ssp_no++)
    {
#if 0
        ret = hi_ssp_init_cfg(ssp_no);
        if (ret)
        {
            SSP_TRACE(SSP_DBG_ERR, "Debug: ssp initial failed!\n");
            return -1;
        }
#else
        hi_ssp_init_cfg(ssp_no);
#endif
    }

    SSP_SPIN_LOCK_INIT();
    SSP_TRACE(SSP_DBG_INFO, "Kernel: ssp initial ok!\n");

    return 0;
#endif
}

static void __exit hi_ssp_exit(void)
{
    unsigned int ssp_no;

#if 0
    sspinitialized = 0;
    hi_ssp_dmac_exit();
    KCOM_HI_DMAC_EXIT();
#endif

    for (ssp_no = 0; ssp_no < SSP_DEV_NUM; ssp_no++)
    {
        hi_ssp_disable(ssp_no);
    }

    iounmap((void *)reg_ssp_base_va);

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
    himedia_unregister(&s_stSspDevice);
#else
    misc_deregister(&ssp_dev);
#endif
}

module_init(hi_ssp_init);
module_exit(hi_ssp_exit);
MODULE_DESCRIPTION("ssp driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hisilicon");

#endif

