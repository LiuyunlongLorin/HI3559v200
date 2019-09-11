
/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : os04b_2l_sensor_ctl.c

  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2017/08/22
  Description   : Sony os04b sensor driver
  History       :
  1.Date        : 2017/08/22
  Author        : yy
  Modification  : Created file

******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "hi_comm_video.h"
#include "hi_sns_ctrl.h"
#include "mpi_sys.h"

#ifdef HI_GPIO_I2C
#include "gpioi2c_ex.h"
#else
#include "hi_i2c.h"
#endif

const unsigned char os04b_2l_i2c_addr     =    0x78; /* I2C Address of OS04b */       //0x78
const unsigned int  os04b_2l_addr_byte    =    1;
const unsigned int  os04b_2l_data_byte    =    1;

static int g_fd[ISP_MAX_PIPE_NUM] = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = -1};
extern ISP_SNS_STATE_S         *g_pastOs04b_2l[ISP_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U        g_aunOs04b_2lBusInfo[];

#define Os04b_2l_4M_2560x1440_10bit_linear30   (0)

/* VI 297M */
void os04b_2l_init_4M_2560x1440_10bit_linear(VI_PIPE ViPipe);

int os04b_2l_i2c_init(VI_PIPE ViPipe)
{
    char acDevFile[16] = {0};
    HI_U8 u8DevNum;

    if(g_fd[ViPipe] >= 0)
    {
        return HI_SUCCESS;
    }
#ifdef HI_GPIO_I2C
    int ret;

    g_fd[ViPipe] = open("/dev/gpioi2c_ex", O_RDONLY, S_IRUSR);
    if (g_fd[ViPipe] < 0)
    {
        ISP_TRACE(HI_DBG_ERR, "Open gpioi2c_ex error!\n");
        return HI_FAILURE;
    }
#else
    int ret;

    u8DevNum = g_aunOs04b_2lBusInfo[ViPipe].s8I2cDev;
    snprintf(acDevFile, sizeof(acDevFile),  "/dev/i2c-%u", u8DevNum);

    g_fd[ViPipe] = open(acDevFile, O_RDWR, S_IRUSR | S_IWUSR);

    if (g_fd[ViPipe] < 0)
    {
        ISP_TRACE(HI_DBG_ERR, "Open /dev/hi_i2c_drv-%u error!\n", u8DevNum);
        return HI_FAILURE;
    }

    ret = ioctl(g_fd[ViPipe], I2C_SLAVE_FORCE, (os04b_2l_i2c_addr>>1));
    if (ret < 0)
    {
        ISP_TRACE(HI_DBG_ERR, "I2C_SLAVE_FORCE error!\n");
        close(g_fd[ViPipe]);
        g_fd[ViPipe] = -1;
        return ret;
    }
#endif

    return HI_SUCCESS;
}

int os04b_2l_i2c_exit(VI_PIPE ViPipe)
{
    if (g_fd[ViPipe] >= 0)
    {
        close(g_fd[ViPipe]);
        g_fd[ViPipe] = -1;
        return HI_SUCCESS;
    }
    return HI_FAILURE;
}

int os04b_2l_read_register(VI_PIPE ViPipe, int addr)
{
    // TODO:

    return HI_SUCCESS;
}

int os04b_2l_write_register(VI_PIPE ViPipe, int addr, int data)
{
    if (0 > g_fd[ViPipe])
    {
        return HI_SUCCESS;
    }

#ifdef HI_GPIO_I2C
    i2c_data.dev_addr = os04b_2l_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = os04b_2l_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = os04b_2l_data_byte;

    ret = ioctl(g_fd[ViPipe], GPIO_I2C_WRITE, &i2c_data);

    if (ret)
    {
        ISP_TRACE(HI_DBG_ERR, "GPIO-I2C write faild!\n");
        return ret;
    }
#else

	int idx = 0;
	int ret;
	char buf[8];

	if (os04b_2l_addr_byte == 2)
	{
		buf[idx] = (addr >> 8) & 0xff;
		idx++;
		buf[idx] = addr & 0xff;
		idx++;
	}
	else
	{
		buf[idx] = addr & 0xff;
		idx++;
	}

	if (os04b_2l_data_byte == 2)
	{
		buf[idx] = (data >> 8) & 0xff;
		idx++;
		buf[idx] = data & 0xff;
		idx++;
	}
	else
	{
		buf[idx] = data & 0xff;
		idx++;
	}

	ret = write(g_fd[ViPipe], buf, (os04b_2l_addr_byte + os04b_2l_data_byte));
	if (ret < 0)
	{
		ISP_TRACE(HI_DBG_ERR, "I2C_WRITE error! addr=0x%x data=0x%x\n",addr,data);
		return HI_FAILURE;
	}
	
#endif
    return HI_SUCCESS;
}

void os04b_2l_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
    switch (eSnsMirrorFlip)
    {
        default:
        case ISP_SNS_NORMAL:
            os04b_2l_write_register(ViPipe, 0xfd, 0x01);
            os04b_2l_write_register(ViPipe, 0x3f, 0x01);
            os04b_2l_write_register(ViPipe, 0x01, 0x01);
            os04b_2l_write_register(ViPipe, 0xfd, 0x02);
            os04b_2l_write_register(ViPipe, 0x62, 0x09);
            os04b_2l_write_register(ViPipe, 0x63, 0x00);
            os04b_2l_write_register(ViPipe, 0xfd, 0x01);
            break;

        case ISP_SNS_MIRROR:
            os04b_2l_write_register(ViPipe, 0xfd, 0x01);
            os04b_2l_write_register(ViPipe, 0x3f, 0x00);
            os04b_2l_write_register(ViPipe, 0x01, 0x01);
            os04b_2l_write_register(ViPipe, 0xfd, 0x02);
            os04b_2l_write_register(ViPipe, 0x62, 0x09);
            os04b_2l_write_register(ViPipe, 0x63, 0x00);
            os04b_2l_write_register(ViPipe, 0xfd, 0x01);
            break;

        case ISP_SNS_FLIP:
            os04b_2l_write_register(ViPipe, 0xfd, 0x01);
            os04b_2l_write_register(ViPipe, 0x3f, 0x03);
            os04b_2l_write_register(ViPipe, 0x01, 0x01);
            os04b_2l_write_register(ViPipe, 0xfd, 0x02);
            os04b_2l_write_register(ViPipe, 0x62, 0xa8);
            os04b_2l_write_register(ViPipe, 0x63, 0x05);
            os04b_2l_write_register(ViPipe, 0xfd, 0x01);
            break;

        case ISP_SNS_MIRROR_FLIP:
            os04b_2l_write_register(ViPipe, 0xfd, 0x01);
            os04b_2l_write_register(ViPipe, 0x3f, 0x02);
            os04b_2l_write_register(ViPipe, 0x01, 0x01);
            os04b_2l_write_register(ViPipe, 0xfd, 0x02);
            os04b_2l_write_register(ViPipe, 0x62, 0xa8);
            os04b_2l_write_register(ViPipe, 0x63, 0x05);
            os04b_2l_write_register(ViPipe, 0xfd, 0x01);
            break;
    }

    return;
}

static void delay_ms(int ms)
{
    usleep(ms * 1000);
}

void os04b_2l_standby(VI_PIPE ViPipe)
{
     // TODO:
    return;
}

void os04b_2l_restart(VI_PIPE ViPipe)
{
    // TODO:
    return;
}

void os04b_2l_prog(VI_PIPE ViPipe, int *rom)
{
    int i = 0;
    while (1)
    {
        int lookup = rom[i++];
        int addr = (lookup >> 16) & 0xFFFF;
        int data = lookup & 0xFFFF;
        if (addr == 0xFFFE)
        {
            delay_ms(data);
        }
        else if (addr == 0xFFFF)
        {
            return;
        }
        else
        {
            os04b_2l_write_register(ViPipe, addr, data);
        }
    }
}
void os04b_2l_default_reg_init(VI_PIPE ViPipe)
{
    HI_U32 i;

    for (i = 0; i < g_pastOs04b_2l[ViPipe]->astRegsInfo[0].u32RegNum; i++)
    {
        os04b_2l_write_register(ViPipe, g_pastOs04b_2l[ViPipe]->astRegsInfo[0].astI2cData[i].u32RegAddr, g_pastOs04b_2l[ViPipe]->astRegsInfo[0].astI2cData[i].u32Data);
    }
}

void os04b_2l_init(VI_PIPE ViPipe)
{
    HI_BOOL          bInit;
    HI_U8            u8ImgMode;
	bInit       = g_pastOs04b_2l[ViPipe]->bInit;
    u8ImgMode = g_pastOs04b_2l[ViPipe]->u8ImgMode;
    //HI_U32  i;
	
    /* 2. os04b i2c init */
    os04b_2l_i2c_init(ViPipe);

    /* When sensor first init, config all registers */
    if (HI_FALSE == bInit)
    {
    switch(u8ImgMode)
    {
        case Os04b_2l_4M_2560x1440_10bit_linear30:
            os04b_2l_init_4M_2560x1440_10bit_linear(ViPipe);
            break;
            
        default:
                ISP_TRACE(HI_DBG_ERR, "Not Support Image Mode %d\n", u8ImgMode);
            break;
        }
    }
	/* When sensor switch mode(linear<->WDR or resolution), config different registers(if possible) */
    else
    {
        switch (u8ImgMode)
        {
            case Os04b_2l_4M_2560x1440_10bit_linear30:
                os04b_2l_init_4M_2560x1440_10bit_linear(ViPipe);
                break;

            default:
                ISP_TRACE(HI_DBG_ERR, "Not Support Image Mode %d\n", u8ImgMode);
                break;
        }

    }

    /*for (i = 0; i < g_pastOs05a[ViPipe]->astRegsInfo[0].u32RegNum; i++)
    {
        os05a_write_register(ViPipe, g_pastOs05a[ViPipe]->astRegsInfo[0].astI2cData[i].u32RegAddr, g_pastOs05a[ViPipe]->astRegsInfo[0].astI2cData[i].u32Data);
    }*/
	
	g_pastOs04b_2l[ViPipe]->bInit = HI_TRUE;
	
    return ;
}

void os04b_2l_exit(VI_PIPE ViPipe)
{
    os04b_2l_i2c_exit(ViPipe);
    return;
}

void os04b_2l_init_4M_2560x1440_10bit_linear(VI_PIPE ViPipe)
{   

	os04b_2l_write_register(ViPipe,0xfd,0x01);
	os04b_2l_write_register(ViPipe,0x20,0x00);
	os04b_2l_write_register(ViPipe,0xfd,0x00);
	os04b_2l_write_register(ViPipe,0x34,0x71);
	os04b_2l_write_register(ViPipe,0x32,0x01);
	os04b_2l_write_register(ViPipe,0x33,0x01);
	os04b_2l_write_register(ViPipe,0x2e,0x0c);
	os04b_2l_write_register(ViPipe,0xfd,0x01);
	os04b_2l_write_register(ViPipe,0x03,0x01);
	os04b_2l_write_register(ViPipe,0x04,0xc6);
	os04b_2l_write_register(ViPipe,0x06,0x0b);
	os04b_2l_write_register(ViPipe,0x0a,0x50);
	os04b_2l_write_register(ViPipe,0x38,0x20);
	os04b_2l_write_register(ViPipe,0x39,0x08);
	os04b_2l_write_register(ViPipe,0x31,0x01);
	os04b_2l_write_register(ViPipe,0x24,0xff);
	os04b_2l_write_register(ViPipe,0x01,0x01);
	os04b_2l_write_register(ViPipe,0x11,0x59);
	os04b_2l_write_register(ViPipe,0x13,0xf4);
	os04b_2l_write_register(ViPipe,0x14,0xff);
	os04b_2l_write_register(ViPipe,0x19,0xf2);
	os04b_2l_write_register(ViPipe,0x16,0x68);
	os04b_2l_write_register(ViPipe,0x1a,0x5e);
	os04b_2l_write_register(ViPipe,0x1c,0x1a);
	os04b_2l_write_register(ViPipe,0x1d,0xd6);
	os04b_2l_write_register(ViPipe,0x1f,0x17);
	os04b_2l_write_register(ViPipe,0x20,0x99);
	os04b_2l_write_register(ViPipe,0x26,0x76);
	os04b_2l_write_register(ViPipe,0x27,0x0c);
	os04b_2l_write_register(ViPipe,0x29,0x3b);
	os04b_2l_write_register(ViPipe,0x2a,0x00);
	os04b_2l_write_register(ViPipe,0x2b,0x8e);
	os04b_2l_write_register(ViPipe,0x2c,0x0b);
	os04b_2l_write_register(ViPipe,0x2e,0x02);
	os04b_2l_write_register(ViPipe,0x44,0x03);
	os04b_2l_write_register(ViPipe,0x45,0xbe);
	os04b_2l_write_register(ViPipe,0x50,0x06);
	os04b_2l_write_register(ViPipe,0x51,0x18);
	os04b_2l_write_register(ViPipe,0x52,0x10);
	os04b_2l_write_register(ViPipe,0x53,0x08);
	os04b_2l_write_register(ViPipe,0x55,0x15);
	os04b_2l_write_register(ViPipe,0x56,0x00);
	os04b_2l_write_register(ViPipe,0x57,0x0d);
	os04b_2l_write_register(ViPipe,0x59,0x00);
	os04b_2l_write_register(ViPipe,0x5a,0x04);
	os04b_2l_write_register(ViPipe,0x5b,0x00);
	os04b_2l_write_register(ViPipe,0x5c,0xb8);
	os04b_2l_write_register(ViPipe,0x5d,0x00);
	os04b_2l_write_register(ViPipe,0x65,0x00);
	os04b_2l_write_register(ViPipe,0x67,0x00);
	os04b_2l_write_register(ViPipe,0x66,0x2a);
	os04b_2l_write_register(ViPipe,0x68,0x2c);
	os04b_2l_write_register(ViPipe,0x69,0x16);
	os04b_2l_write_register(ViPipe,0x6a,0x10);
	os04b_2l_write_register(ViPipe,0x6b,0x03);
	os04b_2l_write_register(ViPipe,0x6c,0x18);
	os04b_2l_write_register(ViPipe,0x71,0x42);
	os04b_2l_write_register(ViPipe,0x72,0x04);
	os04b_2l_write_register(ViPipe,0x73,0x30);
	os04b_2l_write_register(ViPipe,0x74,0x03);
	os04b_2l_write_register(ViPipe,0x77,0x28);
	os04b_2l_write_register(ViPipe,0x7b,0x00);
	os04b_2l_write_register(ViPipe,0x7f,0x18);
	os04b_2l_write_register(ViPipe,0x83,0xf0);
	os04b_2l_write_register(ViPipe,0x85,0x10);
	os04b_2l_write_register(ViPipe,0x86,0xf0);
	os04b_2l_write_register(ViPipe,0x8a,0x33);
	os04b_2l_write_register(ViPipe,0x8b,0x33);
	os04b_2l_write_register(ViPipe,0x28,0x04);
	os04b_2l_write_register(ViPipe,0x34,0x00);
	os04b_2l_write_register(ViPipe,0x35,0x08);
	os04b_2l_write_register(ViPipe,0x36,0x0a);
	os04b_2l_write_register(ViPipe,0x37,0x00);
	os04b_2l_write_register(ViPipe,0x4a,0x00);
	os04b_2l_write_register(ViPipe,0x4b,0x04);
	os04b_2l_write_register(ViPipe,0x4c,0x05);
	os04b_2l_write_register(ViPipe,0x4d,0xa0);
	os04b_2l_write_register(ViPipe,0x01,0x01);
	os04b_2l_write_register(ViPipe,0x8e,0x0a);
	os04b_2l_write_register(ViPipe,0x8f,0x00);
	os04b_2l_write_register(ViPipe,0x90,0x05);
	os04b_2l_write_register(ViPipe,0x91,0xa0);
	os04b_2l_write_register(ViPipe,0xa1,0x04);
	os04b_2l_write_register(ViPipe,0xc4,0x80);
	os04b_2l_write_register(ViPipe,0xc5,0x80);
	os04b_2l_write_register(ViPipe,0xc6,0x80);
	os04b_2l_write_register(ViPipe,0xc7,0x80);
	os04b_2l_write_register(ViPipe,0xfb,0x03);
	os04b_2l_write_register(ViPipe,0xfa,0x16);
	os04b_2l_write_register(ViPipe,0xfa,0x14);
	os04b_2l_write_register(ViPipe,0xf5,0x02);
	os04b_2l_write_register(ViPipe,0xf0,0x40);
	os04b_2l_write_register(ViPipe,0xf1,0x40);
	os04b_2l_write_register(ViPipe,0xf2,0x40);
	os04b_2l_write_register(ViPipe,0xf3,0x40);
	os04b_2l_write_register(ViPipe,0xb1,0x01);
	os04b_2l_write_register(ViPipe,0xb6,0x80);
	os04b_2l_write_register(ViPipe,0xfd,0x00);
	os04b_2l_write_register(ViPipe,0x36,0x01);
	os04b_2l_write_register(ViPipe,0x34,0x72);
	os04b_2l_write_register(ViPipe,0x34,0x71);
	os04b_2l_write_register(ViPipe,0x36,0x00);
	os04b_2l_write_register(ViPipe,0xfd,0x01);
	os04b_2l_write_register(ViPipe,0xfb,0x03);
	os04b_2l_write_register(ViPipe,0xfd,0x03);
	os04b_2l_write_register(ViPipe,0xc0,0x01);
	os04b_2l_write_register(ViPipe,0xfd,0x02);
	os04b_2l_write_register(ViPipe,0xa8,0x01);
	os04b_2l_write_register(ViPipe,0xa9,0x00);
	os04b_2l_write_register(ViPipe,0xaa,0x08);
	os04b_2l_write_register(ViPipe,0xab,0x00);
	os04b_2l_write_register(ViPipe,0xac,0x08);
	os04b_2l_write_register(ViPipe,0xad,0x05);
	os04b_2l_write_register(ViPipe,0xae,0xa0);
	os04b_2l_write_register(ViPipe,0xaf,0x0a);
	os04b_2l_write_register(ViPipe,0xb0,0x00);
	os04b_2l_write_register(ViPipe,0x62,0x09);
	os04b_2l_write_register(ViPipe,0x63,0x00);
	os04b_2l_write_register(ViPipe,0xfd,0x01);
	os04b_2l_write_register(ViPipe,0xb1,0x03);
    
    printf("-------OV sssos04b_2l_init_4M_2560x1440_10bit_linear Initial OK!-------\n");
}
