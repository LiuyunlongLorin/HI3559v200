/*  extdrv/interface/ssp/hi_ssp.c
 *
 * Copyright (c) 2006 Hisilicon Co., Ltd.
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
 */

#include <linux/kernel.h>
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
#include <linux/delay.h>

#include <asm/io.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>

#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>

#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include "ssp_st7789.h"


#ifdef __HuaweiLite__
#include "asm/io.h"
#endif


#ifdef __HuaweiLite__
#else
#include <mach/io.h>/* for IO_ADDRESS */
#endif

#define SSP_BASE    0x120c1000
#define SSP_SIZE    0x1000          // 4KB


#define DEFAULT_MD_LEN (128)

#ifdef __HuaweiLite__

#define IO_ADDRESS_VERIFY(x) (x)

#else

void __iomem *reg_ssp_base_va;
#define IO_ADDRESS_VERIFY(x) (reg_ssp_base_va + ((x)-(SSP_BASE)))

#endif

/* SSP register definition .*/
#define SSP_CR0              IO_ADDRESS_VERIFY(SSP_BASE + 0x00)
#define SSP_CR1              IO_ADDRESS_VERIFY(SSP_BASE + 0x04)
#define SSP_DR               IO_ADDRESS_VERIFY(SSP_BASE + 0x08)
#define SSP_SR               IO_ADDRESS_VERIFY(SSP_BASE + 0x0C)
#define SSP_CPSR             IO_ADDRESS_VERIFY(SSP_BASE + 0x10)
#define SSP_IMSC             IO_ADDRESS_VERIFY(SSP_BASE + 0x14)
#define SSP_RIS              IO_ADDRESS_VERIFY(SSP_BASE + 0x18)
#define SSP_MIS              IO_ADDRESS_VERIFY(SSP_BASE + 0x1C)
#define SSP_ICR              IO_ADDRESS_VERIFY(SSP_BASE + 0x20)
#define SSP_DMACR            IO_ADDRESS_VERIFY(SSP_BASE + 0x24)

#define SPI_SR_BSY        (0x1 << 4)/* spi busy flag */
#define SPI_SR_TFE        (0x1 << 0)/* Whether to send fifo is empty */
#define SPI_DATA_WIDTH    (9)
#define SPI_SPO           (1)
#define SPI_SPH           (1)
#define SPI_SCR           (8)
#define SPI_CPSDVSR       (8)
#define SPI_FRAMEMODE     (0)

#define MAX_WAIT 10000

#define  ssp_readw(addr,ret)            (ret =(*(volatile unsigned int *)(addr)))
#define  ssp_writew(addr,value)            ((*(volatile unsigned int *)(addr)) = (value))

static int ssp_set_reg(unsigned int Addr, unsigned int Value)
{
#ifdef __HuaweiLite__
    (*(volatile unsigned int *)(Addr)) = Value;
#else
    void* pmem = ioremap_nocache(Addr, DEFAULT_MD_LEN);
    if (pmem == NULL)
    {
        return -1;
    }

    *(unsigned int*)pmem = Value;
    iounmap(pmem);
#endif
    return 0;
}

#if 0
static int ssp_read_reg(unsigned int Addr, unsigned int* pValue)
{
#ifdef __HuaweiLite__
    (*pValue) = (*(volatile unsigned int *)(Addr));
#else
    void* pmem = ioremap_nocache(Addr, DEFAULT_MD_LEN);
    if (pmem == NULL)
    {
        return -1;
    }

    * pValue = *(unsigned int*)pmem;
    iounmap(pmem);
#endif
    return 0;
}
#endif

static int hi_spi_check_timeout(void)
{
    unsigned int value =  0;
    unsigned int tmp = 0;
    while (1)
    {
        ssp_readw(SSP_SR,value);
        if ((value & SPI_SR_TFE) && (!(value & SPI_SR_BSY)))
        {
            break;
        }

        if (tmp++ > MAX_WAIT)
        {
            printk("spi transfer wait timeout!\n");
            return -1;
        }
        udelay(1);
    }
    return 0;
}



static void hi_ssp_writeOnly(int bWriteOnly)
{
    int ret = 0;

    bWriteOnly = 0;

    ssp_readw(SSP_CR1,ret);

    if (bWriteOnly)
    {
        ret = ret | (0x1 << 5);
    }
    else
    {
        ret = ret & (~(0x1 << 5));
    }

    ssp_writew(SSP_CR1,ret);
}


static void hi_ssp_enable(void)
{
    int ret = 0;
    ssp_readw(SSP_CR1,ret);
    ret = (ret & 0xFFFD) | 0x2;

    ret = ret | (0x1 << 4); /* big/little end, 1: little, 0: big */

    ret = ret | (0x1 << 15); /* wait en */

    ssp_writew(SSP_CR1,ret);

    hi_ssp_writeOnly(0);
}


static void hi_ssp_disable(void)
{
    int ret = 0;
    ssp_readw(SSP_CR1,ret);
    ret = ret & (~(0x1 << 1));
    ssp_writew(SSP_CR1,ret);
}

static int hi_ssp_set_frameform(unsigned char framemode,unsigned char spo,unsigned char sph,unsigned char datawidth)
{
    int ret = 0;
    ssp_readw(SSP_CR0,ret);
    if(framemode > 3)
    {
        printk("set frame parameter err.\n");
        return -1;
    }
    ret = (ret & 0xFFCF) | (framemode << 4);
    if((ret & 0x30) == 0)
    {
        if(spo > 1)
        {
            printk("set spo parameter err.\n");
            return -1;
        }
        if(sph > 1)
        {
            printk("set sph parameter err.\n");
            return -1;
        }
        ret = (ret & 0xFF3F) | (sph << 7) | (spo << 6);
    }
    if((datawidth > 16) || (datawidth < 4))
    {
        printk("set datawidth parameter err.\n");
        return -1;
    }
    ret = (ret & 0xFFF0) | (datawidth -1);
    ssp_writew(SSP_CR0,ret);
    return 0;
}


static int hi_ssp_set_serialclock(unsigned char scr,unsigned char cpsdvsr)
{
    int ret = 0;
    ssp_readw(SSP_CR0,ret);
    ret = (ret & 0xFF) | (scr << 8);
    ssp_writew(SSP_CR0,ret);
    if((cpsdvsr & 0x1))
    {
        printk("set cpsdvsr parameter err.\n");
        return -1;
    }
    ssp_writew(SSP_CPSR,cpsdvsr);
    return 0;
}

static int hi_ssp_alt_mode_set(int enable)
{
    int ret = 0;

    ssp_readw(SSP_CR1,ret);
    if (enable)
    {
        ret = ret & (~0x40);
    }
    else
    {
        ret = (ret & 0xFF) | 0x40;
    }
    ssp_writew(SSP_CR1,ret);

    return 0;
}


static void spi_enable(void)
{
    ssp_writew(SSP_CR1, 0x42);
}

static void spi_disable(void)
{
    ssp_writew(SSP_CR1, 0x40);
}

static void spi_write_a9byte(unsigned char cmd_dat,unsigned char dat)
{
    unsigned short spi_data = 0;
    int ret = 0;
    if(cmd_dat)
    {
        spi_data = 1 << 8;
    }
    else
    {
        spi_data = 0 << 8;
    }

    spi_data = spi_data | dat;
    spi_enable();
    ssp_writew(SSP_DR,spi_data);
    ret =  hi_spi_check_timeout();
    if(ret != 0)
    {
        printk("spi_send timeout\n");
    }
    spi_disable();
}

#if 0
static void spi_write_a16byte(unsigned short spi_dat)
{
    spi_enable();
    //hi_ssp_writedata(spi_data);
    ssp_writew(SSP_DR,spi_dat);
    printk("spi_data:0x%x\n",spi_dat);
    msleep(1);
    spi_disable();
}
#endif

static void ssp_write_dat(unsigned char dat)
{
    spi_write_a9byte(1,dat);
}

static void ssp_write_cmd(unsigned char dat)
{
    spi_write_a9byte(0,dat);
}

static long ssp_lcd_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    //printk(KERN_EMERG " cmd = 0x%x,cmd&0xf000  = 0x%x\n",cmd,cmd&0xf000);
    if((cmd&0xf000 )== 0xf000)
    {
        printk(KERN_EMERG "KERN_EMERG cmd = 0x%x,0x%x,0x%x\n",cmd,((cmd&(0x1<<8))>>8),cmd&0xff);
        spi_write_a9byte(((cmd&(0x1<<8))>>8),cmd&0xff);
        return 0;
    }
    switch(cmd)
    {
        case SSP_LCD_SET_BRIGHTNESS:

        break;
        case SSP_LCD_SET_BACKLIGHT:

        break;
        case SSP_LCD_SET_DISPLAY:
 #if 0
        ssp_gpio_dir_config(9,6,1);
        if(arg)
        {
            ssp_write_cmd(0x11);
            ssp_gpio_write(9,6,1);
        }
        else
        {
            ssp_write_cmd(0x10);
            ssp_gpio_write(9,6,0);
        }
  #endif
        break;
        default:
        {
            printk("Kernel: No such ssp command %#x!\n", cmd);
            break;
        }
    }

    return 0;
}

static int ssp_lcd_open(struct inode * inode, struct file * file)
{
        return 0;
}


static int ssp_lcd_close(struct inode * inode, struct file * file)
{
    return 0;
}

static void ssp_set(void)
{
    spi_disable();
    hi_ssp_set_frameform(SPI_FRAMEMODE, SPI_SPO, SPI_SPH, SPI_DATA_WIDTH);
    hi_ssp_set_serialclock(SPI_SCR, SPI_CPSDVSR);
    hi_ssp_alt_mode_set(1);
    hi_ssp_enable();
}

static void lcd_reset(void)
{

    //-----------------------------------------------LED RESET----------------------------------------------//
    ssp_set_reg(0x120d0400, 0x20);
    ssp_set_reg(0x120d0080, 0x20);
    ssp_set_reg(0x120d0080, 0x0);
    msleep(10);
    ssp_set_reg(0x120d0080, 0x20);
    msleep(120);
    //----------------------------------------ST7789S SleepOut Cmd-------------------------------------------//

    //msleep(120);
}



static void lcd_st7789_init_vertical(void)
{
    /*spi_9bit_setting*/
    ssp_set();

    lcd_reset();

    ssp_write_cmd(0x11);
    msleep(120);
    //--------------------------------ST7789S Memory Data Access Control setting----------------------------------//
    //-----------------------Decides RGB/BGR or refresh Left to Right/Bottom to Top and so on---------------------- ---//
#if 0
    ssp_write_cmd(0x36);
    ssp_write_dat(0x00);

    //----------------------------------ST7789S Interface Pixel Format setting--------------------------------------//
    //----------------------------------18bit/Pixel and 262K of RGB interface----------------------------------------//
    ssp_write_cmd(0x3A);
    ssp_write_dat(0x66);
    //----------------------------------------ST7789S Display InversionOn Cmd-------------------------------------//
    ssp_write_cmd(0x21);

    //------------------------------------------ST7789S RAM Control Setting---------------------------------------//
    ssp_write_cmd(0xB0);
    ssp_write_dat(0x11);
    ssp_write_dat(0x04);

    //---------------------------------------ST7789S RAM Interface Control Setting----------------------------------//
    //----------------------------------------RGB Mode:Shift Register----------------------------------------------//
    ssp_write_cmd(0xB1);
    ssp_write_dat(0xC0);
    ssp_write_dat(0x02);
    ssp_write_dat(0x14);

    //-----------------------------------------ST7789S Porch Setting---------------------------------------------//
    ssp_write_cmd(0xB2);
    ssp_write_dat(0x05);
    ssp_write_dat(0x05);
    ssp_write_dat(0x00);
    ssp_write_dat(0x33);
    ssp_write_dat(0x33);

    //-----------------------------------------ST7789S Gate Control Setting----------------------------------------//
    //--------------------------------------------VGL and VGH number--------------------------------------------//
    ssp_write_cmd(0xB7);
    ssp_write_dat(0x64);

    //-----------------------------------------ST7789S VCOM  setting---------------------------------------------//
    ssp_write_cmd(0xBB);
    ssp_write_dat(0x25);

    //---------------------------------------ST7789S LCM Control setting-------------------------------------------//
    ssp_write_cmd(0xC0);
    ssp_write_dat(0x2C);

    //-----------------------------------ST7789S VDV and VRH Command Enable setting--------------------------------//
    ssp_write_cmd(0xC2);
    ssp_write_dat(0x01);

    ssp_write_cmd(0xC3);
    //ssp_write_dat(0x13);
    ssp_write_dat(0x20);

    ssp_write_cmd(0xC4);
    //ssp_write_dat(0x20);
    ssp_write_dat(0x3);
    //----------------------------------ST7789S Normal mode Frame Rate setting--------------------------------------//
    ssp_write_cmd(0xC6);
    ssp_write_dat(0x11);

    //-------------------------------------ST7789S Power Control setting--------------------------------------------//
    ssp_write_cmd(0xD0);
    ssp_write_dat(0xA4);
    ssp_write_dat(0xA1);

    ssp_write_cmd(0xD6);
    ssp_write_dat(0xA1);

    //---------------------------------ST7789S Positive Gamma setting--------------------------------------//
#if 0
    ssp_write_cmd(0xE0);
    ssp_write_dat(0xd0);
    ssp_write_dat(0x00);
    ssp_write_dat(0x00);
    ssp_write_dat(0x08);
    ssp_write_dat(0x11);
    ssp_write_dat(0x1a);
    ssp_write_dat(0x2b);
    ssp_write_dat(0x33);
    ssp_write_dat(0x42);
    ssp_write_dat(0x26);
    ssp_write_dat(0x12);
    ssp_write_dat(0x21);
    ssp_write_dat(0x2f);
    ssp_write_dat(0x11);

    //---------------------------------ST7789S Negative Gamma setting--------------------------------------//
    ssp_write_cmd(0xE1);
    ssp_write_dat(0xd0);
    ssp_write_dat(0x02);
    ssp_write_dat(0x09);
    ssp_write_dat(0x0d);
    ssp_write_dat(0x0d);
    ssp_write_dat(0x27);
    ssp_write_dat(0x2b);
    ssp_write_dat(0x33);
    ssp_write_dat(0x42);
    ssp_write_dat(0x17);
    ssp_write_dat(0x12);
    ssp_write_dat(0x11);
    ssp_write_dat(0x2f);
    ssp_write_dat(0x31);
#endif
    //-----------------------------------ST7789S BrightNess Setting------------------------------------------//
    //ssp_write_cmd(0x51);
    //ssp_write_dat(0xFF);

    //-----------------------------------ST7789S Display ON Cmd--------------------------------------------//
    ssp_write_cmd(0x29);

    //-----------------------------------ST7789S Memory Write Cmd-----------------------------------------//
    //--------------------------------Transfer data from MCU to Frame memory---------------------------------//
    ssp_write_cmd(0x2c);

#else
    //--------------------------------ST7789S Frame rate setting----------------------------------//
    ssp_write_cmd(0x36);
    ssp_write_dat(0x00);
    //--------------------------------ST7789S Frame rate setting----------------------------------//
    ssp_write_cmd(0xb2);
    ssp_write_dat(0x00);
    ssp_write_dat(0x00);
    ssp_write_dat(0x00);
    ssp_write_dat(0x33);
    ssp_write_dat(0x33);

    ssp_write_cmd(0xb7);
    ssp_write_dat(0x35);
    //---------------------------------ST7789S Power setting--------------------------------------//
    ssp_write_cmd(0xb8);
    ssp_write_dat(0x2f);
    ssp_write_dat(0x2b);
    ssp_write_dat(0x2f);

    ssp_write_cmd(0xbb);
    ssp_write_dat(0x24);//vcom

    ssp_write_cmd(0xc0);
    ssp_write_dat(0x2C);

    ssp_write_cmd(0xc3);
    ssp_write_dat(0x20);

    ssp_write_cmd(0xc4);
    ssp_write_dat(0x3);

    ssp_write_cmd(0xc6);
    ssp_write_dat(0x11);

    ssp_write_cmd(0xd0);
    ssp_write_dat(0xa4);
    ssp_write_dat(0xa1);

    ssp_write_cmd(0xe8);
    ssp_write_dat(0x03);

    ssp_write_cmd(0xe9);
    ssp_write_dat(0x0d);
    ssp_write_dat(0x12);
    ssp_write_dat(0x00);
    //--------------------------------ST7789S gamma setting---------------------------------------//
#if 0
    ssp_write_cmd(0xe0);
    ssp_write_dat(0xd0);
    ssp_write_dat(0x00);
    ssp_write_dat(0x00);
    ssp_write_dat(0x08);
    ssp_write_dat(0x11);
    ssp_write_dat(0x1a);
    ssp_write_dat(0x2b);
    ssp_write_dat(0x33);
    ssp_write_dat(0x42);
    ssp_write_dat(0x26);
    ssp_write_dat(0x12);
    ssp_write_dat(0x21);
    ssp_write_dat(0x2f);
    ssp_write_dat(0x11);

    ssp_write_cmd(0xe1);
    ssp_write_dat(0xd0);
    ssp_write_dat(0x02);
    ssp_write_dat(0x09);
    ssp_write_dat(0x0d);
    ssp_write_dat(0x0d);
    ssp_write_dat(0x27);
    ssp_write_dat(0x2b);
    ssp_write_dat(0x33);
    ssp_write_dat(0x42);
    ssp_write_dat(0x17);
    ssp_write_dat(0x12);
    ssp_write_dat(0x11);
    ssp_write_dat(0x2f);
    ssp_write_dat(0x31);
#endif
    ssp_write_cmd(0x21);

    //*********SET RGB Interfae***************
    ssp_write_cmd(0xB0);
    ssp_write_dat(0x11); //set RGB interface and DE mode.
    ssp_write_dat(0x04);
    ssp_write_dat(0x00);

    ssp_write_cmd(0xB1);
    //ssp_write_dat(0x40); //set DE mode ; SET Hs,Vs,DE,DOTCLK signal polarity
    ssp_write_dat(0xC0);
    ssp_write_dat(0x2);  //ori 2
    ssp_write_dat(0x14); //ori 14
    ssp_write_cmd(0x3a);
    ssp_write_dat(0x55); //18 RGB ,55-16BIT RGB
    ssp_write_cmd(0x11); //Exit Sleep
    ssp_write_cmd(0x29); //display on
    ssp_write_cmd(0x2c);
#endif
    return;
}

static void lcd_st7789_backlighton(void)
{
    //ssp_set_reg(0x045f1400, 0x40);
    //ssp_set_reg(0x045f1100, 0x40);
}



#ifdef __HuaweiLite__
static const struct file_operations_vfs ssp_lcd_fops =
{
    ssp_lcd_open,   /* open */
    ssp_lcd_close,  /* close */
    0,              /* read */
    0,              /* write */
    0,              /* seek */
    ssp_lcd_ioctl   /* ioctl */
#ifndef CONFIG_DISABLE_POLL
    , 0             /* poll */
#endif
};

int lcd_dev_register(void)
{
    return register_driver("/dev/ssp_lcd", &ssp_lcd_fops, 0666, 0);
}

int __init hi_ssp_lcd_st7789_init(void)
{
    int ret;

    ret = lcd_dev_register();
    if(0 != ret)
    {
        printk("Kernel: lcd_dev_register failed!\n");
        return -1;
    }

    lcd_st7789_init_vertical();

    lcd_st7789_backlighton();

    printk("Kernel: ssp_lcd initial ok!\n");

    return 0;
}

void __exit hi_ssp_lcd_st7789_exit(void)
{
    hi_ssp_disable();
}

#else

static struct file_operations ssp_lcd_fops = {
    .owner      = THIS_MODULE,
    .unlocked_ioctl = ssp_lcd_ioctl,
    .open       = ssp_lcd_open,
    .release    = ssp_lcd_close
};


static struct miscdevice ssp_lcd_dev = {
   .minor       = MISC_DYNAMIC_MINOR,
   .name        = "ssp_lcd",
   .fops        = &ssp_lcd_fops,
};


static int __init hi_ssp_lcd_st7789_init(void)
{
    int ret;

    reg_ssp_base_va = ioremap_nocache((unsigned long)SSP_BASE, (unsigned long)SSP_SIZE);

    if (!reg_ssp_base_va)
    {
        printk("Kernel: ioremap ssp base failed!\n");
        return -ENOMEM;
    }

    ret = misc_register(&ssp_lcd_dev);
    if(0 != ret)
    {
        printk("Kernel: register ssp_0 device failed!\n");
        return -1;
    }

    lcd_st7789_init_vertical();

    lcd_st7789_backlighton();

    printk("Kernel: ssp_lcd initial ok!\n");

    return 0;
}

static void __exit hi_ssp_lcd_st7789_exit(void)
{

    hi_ssp_disable();
    iounmap((void*)reg_ssp_base_va);
    misc_deregister(&ssp_lcd_dev);
}
#endif


module_init(hi_ssp_lcd_st7789_init);
module_exit(hi_ssp_lcd_st7789_exit);

MODULE_LICENSE("GPL");

