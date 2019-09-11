
/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : sensor_i2c.c

  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/11/07
  Description   :
  History       :
  1.Date        : 2013/11/07
    Author      :
    Modification: Created file

******************************************************************************/


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
#include "hi_type.h"
#include "hi_ssp.h"

#define DEV_NAME "ssp"
#define MAX_ISP_DEV_NUM     8
#define SSP_DEV_NUM         5

#define  ssp_readw(addr,ret)			(ret =(*(volatile unsigned int *)(addr)))
#define  ssp_writew(addr,value)			((*(volatile unsigned int *)(addr)) = (value))

#define  HI_REG_READ(addr,ret)			(ret =(*(volatile unsigned int *)(addr)))
#define  HI_REG_WRITE(addr,value)		((*(volatile unsigned int *)(addr)) = (value))

#define SSP_BASE	        0x120C0000
#define SSP_SIZE	        0x10000	             // 64KB

#define CRG_BASE	        0x12010000
#define CRG_SIZE	        0x1000
#define SSP_CRG_OFFSET      0x01BC

static void __iomem *reg_ssp_base_va;
static void __iomem *reg_crg_base_va;
#define IO_ADDRESS_VERIFY(x) ((HI_UL)reg_ssp_base_va + ((x)-(SSP_BASE)))
#define SSP_CRG_ADDRESS      ((HI_UL)reg_crg_base_va + SSP_CRG_OFFSET)


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

static spinlock_t ssp_lock[SSP_DEV_NUM];
#define SSP_SPIN_LOCK_INIT(ssp_no)    spin_lock_init(&ssp_lock[ssp_no])
#define SSP_SPIN_LOCK(ssp_no, flags)    spin_lock_irqsave(&ssp_lock[ssp_no], flags)
#define SSP_SPIN_UNLOCK(ssp_no, flags)  spin_unlock_irqrestore(&ssp_lock[ssp_no], flags)

static spinlock_t ssp_crg_lock;
#define SSP_CRG_SPIN_LOCK_INIT()    spin_lock_init(&ssp_crg_lock)
#define SSP_CRG_SPIN_LOCK(flags)    spin_lock_irqsave(&ssp_crg_lock, flags)
#define SSP_CRG_SPIN_UNLOCK(flags)  spin_unlock_irqrestore(&ssp_crg_lock, flags)


static void hi_ssp_clock_enable(unsigned int ssp_no)
{
    unsigned int ret = 0;
    unsigned long flags;

    SSP_CRG_SPIN_LOCK(flags);

    HI_REG_READ(SSP_CRG_ADDRESS, ret);

    ret |= 0x1 << (14 + ssp_no);

    HI_REG_WRITE(SSP_CRG_ADDRESS, ret);

    SSP_CRG_SPIN_UNLOCK(flags);
}

static void hi_ssp_clock_disable(unsigned int ssp_no)
{
    unsigned int ret = 0;
    unsigned long flags;

    SSP_CRG_SPIN_LOCK(flags);

    HI_REG_READ(SSP_CRG_ADDRESS, ret);
    ret = ret & (~(0x1 << (14 + ssp_no)));
    HI_REG_WRITE(SSP_CRG_ADDRESS, ret);

    SSP_CRG_SPIN_UNLOCK(flags);
}

static void hi_ssp_clock_reset(unsigned int ssp_no)
{
    unsigned int ret = 0;
    unsigned long flags;
return;
    SSP_CRG_SPIN_LOCK(flags);

    HI_REG_READ(SSP_CRG_ADDRESS, ret);
    ret |= 0x1 << (17 + ssp_no);
    HI_REG_WRITE(SSP_CRG_ADDRESS, ret);

    SSP_CRG_SPIN_UNLOCK(flags);
}

static void hi_ssp_clock_unreset(unsigned int ssp_no)
{
    unsigned int ret = 0;
    unsigned long flags;
    return;

    SSP_CRG_SPIN_LOCK(flags);

    HI_REG_READ(SSP_CRG_ADDRESS, ret);
    ret =  ret & (~(0x1 << (17 + ssp_no)));
    HI_REG_WRITE(SSP_CRG_ADDRESS, ret);

    SSP_CRG_SPIN_UNLOCK(flags);
}

#if 0
static void hi_ssp_disable(unsigned int ssp_no)
{
    int ret = 0;

    ssp_readw(SSP_CR1(ssp_no), ret);
    ret = ret & (~(0x1 << 1));
    ssp_writew(SSP_CR1(ssp_no), ret);
}
#endif

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
static int hi_ssp_set_frameform(unsigned int ssp_no, unsigned char framemode,unsigned char spo,unsigned char sph,unsigned char datawidth)
{
    int ret = 0;
    ssp_readw(SSP_CR0(ssp_no), ret);
    if(framemode > 3)
    {
        SSP_TRACE(SSP_DBG_ERR, "set frame parameter err.\n");
        return -1;
    }
    ret = (ret & 0xFFCF) | (framemode << 4);
    if((ret & 0x30) == 0)
    {
        if(spo > 1)
        {
            SSP_TRACE(SSP_DBG_ERR, "set spo parameter err.\n");
            return -1;
        }
        if(sph > 1)
        {
            SSP_TRACE(SSP_DBG_ERR, "set sph parameter err.\n");
            return -1;
        }
        ret = (ret & 0xFF3F) | (sph << 7) | (spo << 6);
    }
    if((datawidth > 16) || (datawidth < 4))
    {
        SSP_TRACE(SSP_DBG_ERR, "set datawidth parameter err.\n");
        return -1;
    }
    ret = (ret & 0xFFF0) | (datawidth -1);
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
static int hi_ssp_set_serialclock(unsigned int ssp_no, unsigned char scr,unsigned char cpsdvsr)
{
    int ret = 0;
    ssp_readw(SSP_CR0(ssp_no), ret);
    ret = (ret & 0xFF) | (scr << 8);
    ssp_writew(SSP_CR0(ssp_no), ret);
    if((cpsdvsr & 0x1))
    {
        SSP_TRACE(SSP_DBG_ERR, "set cpsdvsr parameter err.\n");
        return -1;
    }
    ssp_writew(SSP_CPSR(ssp_no), cpsdvsr);
    return 0;
}

static int hi_ssp_BigEnd_set(unsigned int ssp_no)
{
    int ret = 0;

    ssp_readw(SSP_CR1(ssp_no), ret);

    ret = (ret & 0xFF) & 0xEF;
    ssp_writew(SSP_CR1(ssp_no),ret);

    return 0;
}


static int hi_ssp_alt_mode_set(unsigned int ssp_no, int enable)
{
	int ret = 0;

	ssp_readw(SSP_CR1(ssp_no), ret);
	if (!enable)
	{
		ret = ret & (~0x40);
	}
	else
	{
	    ret = (ret & 0xFF) | 0x40;
	}
	ssp_writew(SSP_CR1(ssp_no),ret);

    return 0;
}

static unsigned int hi_ssp_is_fifo_busy(unsigned int ssp_no)
{
    unsigned int ret = 0;

    ssp_readw(SSP_SR(ssp_no), ret);

    return (ret & 0x10);
}

static unsigned int hi_ssp_is_fifo_empty(unsigned int ssp_no, int bSend)
{
    int ret = 0;
    ssp_readw(SSP_SR(ssp_no), ret);

    if (bSend)
    {
        if((ret & 0x1) == 0x1) /* send fifo */
            return 0;
        else
            return 1;
    }
    else
    {
        if((ret & 0x4) == 0x4) /* receive fifo */
            return 0;
        else
            return 1;
    }
}

// Fsspclkout = Fsspclk/(cpsdvsr*(1+scr))
static int hi_ssp_init_cfg(unsigned int ssp_no)
{
	unsigned char framemode = 0;
	unsigned char spo = 1;
	unsigned char sph = 1;
	unsigned char datawidth = 8 ;

#ifdef HI_FPGA
	unsigned char scr = 1;
	unsigned char cpsdvsr = 2;
#else
	unsigned char scr = 4;  //4
	unsigned char cpsdvsr = 2; //2
#endif

	hi_ssp_set_frameform(ssp_no, framemode, spo, sph, datawidth);

	hi_ssp_set_serialclock(ssp_no, scr, cpsdvsr);
    hi_ssp_BigEnd_set(ssp_no);
	// altasens mode, which CS won't be pull high between 16bit data transfer
	hi_ssp_alt_mode_set(ssp_no, 0);

    return 0;
}

static void spi_enable(unsigned int ssp_no)
{
    int ret = 0;

    hi_ssp_clock_enable(ssp_no);
    hi_ssp_clock_unreset(ssp_no);
    // little endian
    HI_REG_READ(SSP_CR1(ssp_no), ret);
    ret = (ret & 0xff) | 0x2;
    HI_REG_WRITE(SSP_CR1(ssp_no), ret);//0x2

    hi_ssp_init_cfg(ssp_no);
}

static void spi_disable(unsigned int ssp_no)
{
    int ret = 0;

    HI_REG_READ(SSP_CR1(ssp_no), ret);
    ret = (ret & 0xff) & (~0x2);
    HI_REG_WRITE(SSP_CR1(ssp_no), ret);//0x0

    hi_ssp_clock_reset(ssp_no);
    hi_ssp_clock_disable(ssp_no);
}

unsigned short hi_motionsensor_ssp_read_alt(unsigned int ssp_no, HI_U8 u8reg_addr, HI_U8* u8reg_data, HI_U32 u32cnt, HI_BOOL bFifoMode)
{
	unsigned int ret = 0;
    unsigned long flags;
    HI_U8 buf[2] = {0};
    HI_S32 i = 0;

    SSP_SPIN_LOCK(ssp_no, flags);

    spi_enable(ssp_no);

    for(i = 0; i < u32cnt; i++)
    {
        //spi_enable(ssp_no);

        if(bFifoMode)
        {
            buf[0] = u8reg_addr | 0x80;
            buf[1] = 0x0;
        }
        else
        {
            buf[0] = (u8reg_addr + i) | 0x80;
            buf[1] = 0x0;
        }

        ssp_writew(SSP_DR(ssp_no), buf[0]);

        ssp_writew(SSP_DR(ssp_no), buf[1]);

        while(hi_ssp_is_fifo_busy(ssp_no)){};

        while(hi_ssp_is_fifo_empty(ssp_no, 1)){};

    	while(hi_ssp_is_fifo_empty(ssp_no, 0)){};

        while(!hi_ssp_is_fifo_empty(ssp_no, 0))
        {
    	    ssp_readw(SSP_DR(ssp_no), ret);
    	}

    	//spi_disable(ssp_no);

    	u8reg_data[i] = ret & 0xff;
	}

	spi_disable(ssp_no);

    SSP_SPIN_UNLOCK(ssp_no, flags);

    return 0;
}

int hi_motionsensor_ssp_write_alt(unsigned int ssp_no, HI_U8 u8reg_addr, HI_U8* data)
{
	unsigned int ret;
    unsigned long flags;
    HI_U8 buf[2] = {0};

    SSP_SPIN_LOCK(ssp_no, flags);

    spi_enable(ssp_no);

    buf[0] = u8reg_addr & (~0x80);
    buf[1] = *data;

    ssp_writew(SSP_DR(ssp_no), buf[0]);

    ssp_writew(SSP_DR(ssp_no), buf[1]);

    while(hi_ssp_is_fifo_busy(ssp_no)){};

    while(hi_ssp_is_fifo_empty(ssp_no, 1)){};

	while(hi_ssp_is_fifo_empty(ssp_no, 0)){};

    while(!hi_ssp_is_fifo_empty(ssp_no, 0))
    {
	    ssp_readw(SSP_DR(ssp_no), ret);
	}

	spi_disable(ssp_no);

    SSP_SPIN_UNLOCK(ssp_no, flags);

    ret = 0;
	return ret;
}

#if 0
static int hi_ssp_write(unsigned int ssp_no, unsigned int spi_csn,
            unsigned int addr1, unsigned int addr1bytenum,
            unsigned int addr2, unsigned int addr2bytenum,
            unsigned int data ,unsigned int databytenum)
{
    unsigned short spi_no, devaddr, addr_high, addr_low, data_value;
    unsigned int ret;
    unsigned long flags;
    unsigned int len;

    spi_no = (unsigned short)(ssp_no & 0xff);
    devaddr   = (unsigned short)(addr1 & 0xff);
    addr_high = (unsigned short)((addr2 & 0xff00) >> 8);
    addr_low  = (unsigned short)(addr2 & 0xff);
    data_value = (unsigned short)(data & 0xff);

#if 1
    len = addr1bytenum + addr2bytenum + databytenum;

    SSP_SPIN_LOCK(ssp_no, flags);

	spi_enable(ssp_no);

    //printk("%#x %#x %#x, addr1bytenum: %d addr2bytenum: %d databytenum: %d \n", addr1, addr2, data, addr1bytenum,addr2bytenum,databytenum);
    if (0 != addr1bytenum)
    {
	    ssp_writew(SSP_DR(ssp_no), addr1);
    }
    if (0 != addr2bytenum)
    {
		if (2 == addr2bytenum)
		{
			ssp_writew(SSP_DR(ssp_no), addr_high);
			ssp_writew(SSP_DR(ssp_no), addr_low);
		}
		else
		{
			ssp_writew(SSP_DR(ssp_no), addr2);
		}
    }
    if (0 != databytenum)
    {
	    ssp_writew(SSP_DR(ssp_no), data);
    }

	// wait receive fifo has data
	while (len--)
	{
        //printk("len: %d \n", len);
		while(hi_ssp_is_fifo_empty(ssp_no, 0)){};
		ssp_readw(SSP_DR(ssp_no), ret);
	}

	spi_disable(ssp_no);

    SSP_SPIN_UNLOCK(ssp_no, flags);

	return 0;
#else
    return hi_ssp_write_alt(spi_no, devaddr, addr_high, addr_low, data_value);
#endif
}
#endif

#ifdef __HuaweiLite__
static int ssp_ioctl(struct file *file, int cmd, unsigned long arg)
#else
static long ssp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    unsigned short spi_no, addr_low;
    //unsigned short devaddr, addr_high;
    HI_U8 data;
    SPI_DATA_S __user *argp = (SPI_DATA_S __user*)arg;
    SPI_DATA_S spi_data;

    if (copy_from_user(&spi_data, argp, sizeof(SPI_DATA_S)))
    {
        return -EFAULT;
    }

	switch(cmd)
	{
		case SSP_READ_ALT:
			//MN34041 LSB first!
            spi_no = ((unsigned short)(spi_data.spi_no & 0xff));
            //devaddr= ((unsigned short)(spi_data.dev_addr & 0xff));
            //addr_high = ((unsigned short)((spi_data.reg_addr & 0xff00) >> 8));
            addr_low = ((unsigned short)(spi_data.reg_addr & 0xff));

            if(spi_no >= SSP_DEV_NUM)
            {
                SSP_TRACE(SSP_DBG_ERR, "Kernel: spi_no[%u] is out of range!\n", spi_no);
                return -1;
            }

            hi_motionsensor_ssp_read_alt(spi_no, addr_low, &data, 1, 0);

            spi_data.data = (unsigned int)(data & 0xff);
            if (copy_to_user(argp, &spi_data, sizeof(SPI_DATA_S)))
            {
                return -EFAULT;
            }
			break;

		case SSP_WRITE_ALT:
            spi_no = (unsigned short)(spi_data.spi_no & 0xff) ;
			//devaddr = (unsigned short)(spi_data.dev_addr & 0xff) ;
			//addr_high = (unsigned short)((spi_data.reg_addr & 0xff00) >> 8) ;
		    addr_low = (unsigned short)(spi_data.reg_addr & 0xff) ;
			data    = (unsigned short)(spi_data.data & 0xff) ;
            if(spi_no >= SSP_DEV_NUM)
            {
                SSP_TRACE(SSP_DBG_ERR, "Kernel: spi_no[%u] is out of range!\n", spi_no);
                return -1;
            }
            //hi_ssp_write_alt(spi_no, devaddr, addr_high, addr_low, data);
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
static int ssp_open(struct file* file)
{
    return 0;
}
static int ssp_close(struct file* file)
{
    return 0;
}
const static struct file_operations_vfs ssp_fops =
{
    .open = ssp_open,
    .close = ssp_close,
    .ioctl = ssp_ioctl
};

int hi_motionsensor_ssp_init(void)
{
    int ret;
    unsigned int ssp_no;

    reg_ssp_base_va = (void __iomem*)IO_ADDRESS((unsigned long)SSP_BASE);
	if (!reg_ssp_base_va)
	{
		SSP_TRACE(SSP_DBG_ERR, "Kernel: ioremap ssp base failed!\n");
	    return -1;
	}


    reg_crg_base_va = (void __iomem*)IO_ADDRESS((unsigned long)CRG_BASE);
	if (!reg_crg_base_va)
	{
        reg_ssp_base_va = NULL;
		SSP_TRACE(SSP_DBG_ERR, "Kernel: ioremap ssp crg failed!\n");
	    return -1;
	}


    ret = register_driver("/dev/ssp", &ssp_fops, 0666, 0);
    if (ret)
    {
        reg_ssp_base_va = NULL;
        reg_crg_base_va = NULL;
        SSP_TRACE(SSP_DBG_ERR, "register pwd device failed with %#x!\n", ret);
        return -1;
    }

    for (ssp_no = 0; ssp_no < SSP_DEV_NUM; ssp_no++)
    {
        SSP_SPIN_LOCK_INIT(ssp_no);
    }

    SSP_CRG_SPIN_LOCK_INIT();

    SSP_TRACE(SSP_DBG_INFO, "Kernel: ssp initial ok!\n");

    return 0;
}

void hi_motionsensor_ssp_exit(void)
{
    reg_ssp_base_va = NULL;
    reg_crg_base_va = NULL;
    unregister_driver("/dev/ssp");
}


#else
static int ssp_open(struct inode * inode, struct file * file)
{
    if (!capable(CAP_SYS_RAWIO))
           return -EPERM;
    return 0;
}
static int ssp_close(struct inode * inode, struct file * file)
{
    return 0;
}

static struct file_operations ssp_fops = {
    .owner      = THIS_MODULE,
    .unlocked_ioctl     	= ssp_ioctl,
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
    SSP_TRACE(SSP_DBG_INFO, "%s  %d\n", __FUNCTION__, __LINE__);

    return 0;
}

static struct himedia_ops stSspDrvOps = {
    .pm_freeze = ssp_freeze,
    .pm_restore  = ssp_restore
};

#else

static struct miscdevice ssp_dev = {
   .minor		= MISC_DYNAMIC_MINOR,
   .name		= DEV_NAME,
   .fops  		= &ssp_fops,
};

#endif


/*
 * initializes SSP interface routine.
 *
 * @return value:0--success.
 *
 */
static int __init hi_motionsensor_ssp_init(void)
{
    int ret;
    unsigned int ssp_no;

    reg_ssp_base_va = ioremap_nocache((unsigned long)SSP_BASE, (unsigned long)(SSP_SIZE * SSP_DEV_NUM));
    if (!reg_ssp_base_va)
    {
        SSP_TRACE(SSP_DBG_ERR, "Kernel: ioremap ssp base failed!\n");
        return -ENOMEM;
    }

    reg_crg_base_va = ioremap_nocache((unsigned long)CRG_BASE, (unsigned long)(CRG_SIZE));
    if (!reg_crg_base_va)
    {
        iounmap((void*)reg_ssp_base_va);
        reg_ssp_base_va = NULL;

        SSP_TRACE(SSP_DBG_ERR, "Kernel: ioremap ssp crg failed!\n");
        return -ENOMEM;
    }

    #ifdef CONFIG_HISI_SNAPSHOT_BOOT
    snprintf(s_stSspDevice.devfs_name, sizeof(s_stSspDevice.devfs_name), DEV_NAME);

    s_stSspDevice.minor  = HIMEDIA_DYNAMIC_MINOR;
    s_stSspDevice.fops   = &ssp_fops;
    s_stSspDevice.drvops = &stSspDrvOps;
    s_stSspDevice.owner  = THIS_MODULE;

    ret = himedia_register(&s_stSspDevice);
    #else
    ret = misc_register(&ssp_dev);
    #endif

    if (ret)
    {
        iounmap((void*)reg_ssp_base_va);
        reg_ssp_base_va = NULL;

        iounmap((void*)reg_crg_base_va);
        reg_crg_base_va = NULL;

        SSP_TRACE(SSP_DBG_ERR, "Kernel: register ssp_0 device failed!");
        return -1;
    }

    for (ssp_no = 0; ssp_no < SSP_DEV_NUM; ssp_no++)
    {
        SSP_SPIN_LOCK_INIT(ssp_no);
    }

    SSP_CRG_SPIN_LOCK_INIT();

    SSP_TRACE(SSP_DBG_INFO, "Kernel: ssp initial ok!\n");
    printk(KERN_INFO "load hi_spi.ko ...OK!\n");
    return 0;
}

static void __exit hi_motionsensor_ssp_exit(void)
{
    iounmap((void*)reg_ssp_base_va);
    reg_ssp_base_va = NULL;

    iounmap((void*)reg_crg_base_va);
    reg_crg_base_va = NULL;

    #ifdef CONFIG_HISI_SNAPSHOT_BOOT
    himedia_unregister(&s_stSspDevice);
    #else
    misc_deregister(&ssp_dev);
    #endif

    printk(KERN_INFO "unload hi_spi.ko ...OK!\n");
}

EXPORT_SYMBOL_GPL(hi_motionsensor_ssp_read_alt);
EXPORT_SYMBOL_GPL(hi_motionsensor_ssp_write_alt);

module_init(hi_motionsensor_ssp_init);
module_exit(hi_motionsensor_ssp_exit);
MODULE_DESCRIPTION("ssp driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hisilicon");

#endif

