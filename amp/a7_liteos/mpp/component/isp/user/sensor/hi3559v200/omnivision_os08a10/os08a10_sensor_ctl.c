#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <ctype.h>
//#include <linux/fb.h>
#include <sys/mman.h>
#include <memory.h>

#include "hi_comm_video.h"
#include "hi_sns_ctrl.h"

#ifdef HI_GPIO_I2C
#include "gpioi2c_ex.h"
#else
#include "hi_i2c.h"
#endif

const unsigned char OS08A10_i2c_addr     =    0x6c;        /* I2C Address of OS08A10 */
const unsigned int  OS08A10_addr_byte    =    2;
const unsigned int  OS08A10_data_byte    =    1;
static int g_fd[ISP_MAX_PIPE_NUM] = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = -1};

extern WDR_MODE_E genSensorMode;
extern HI_U8 gu8SensorImageMode;
extern HI_BOOL bSensorInit;
extern ISP_SNS_STATE_S   *g_pastOS08A10[ISP_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U  g_aunOS08A10BusInfo[];


//sensor fps mode
#define OS08A10_SENSOR_4k2k_30FPS_LINEAR_MODE      (1)
#define OS08A10_SENSOR_1080P120FPS_LINEAR_MODE	   (2)

int OS08A10_i2c_init(VI_PIPE ViPipe)
{
    char acDevFile[16] = {0};
    HI_U8 u8DevNum;

    if (g_fd[ViPipe] >= 0)
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

    u8DevNum = g_aunOS08A10BusInfo[ViPipe].s8I2cDev;
    snprintf(acDevFile, sizeof(acDevFile),  "/dev/i2c-%u", u8DevNum);

    g_fd[ViPipe] = open(acDevFile, O_RDWR, S_IRUSR | S_IWUSR);

    if (g_fd[ViPipe] < 0)
    {
        ISP_TRACE(HI_DBG_ERR, "Open /dev/hi_i2c_drv-%u error!\n", u8DevNum);
        return HI_FAILURE;
    }

    ret = ioctl(g_fd[ViPipe], I2C_SLAVE_FORCE, (OS08A10_i2c_addr>>1));
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

int OS08A10_i2c_exit(VI_PIPE ViPipe)
{
    if (g_fd[ViPipe] >= 0)
    {
        close(g_fd[ViPipe]);
        g_fd[ViPipe] = -1;
        return HI_SUCCESS;
    }
    return HI_FAILURE;
}

int OS08A10_read_register(VI_PIPE ViPipe,int addr)
{
    return HI_SUCCESS;
}
int OS08A10_write_register(VI_PIPE ViPipe, int addr, int data)
{
    if (0 > g_fd[ViPipe])
    {
        return HI_SUCCESS;
    }

#ifdef HI_GPIO_I2C
    i2c_data.dev_addr = OS08A10_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = OS08A10_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = OS08A10_data_byte;

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

    if (OS08A10_addr_byte == 2)
    {
        buf[idx] = (addr >> 8) & 0xff;
        idx++;
        buf[idx] = addr & 0xff;
        idx++;
    }
    else
    {
        //buf[idx] = addr & 0xff;
        //idx++;
    }

    if (OS08A10_data_byte == 2)
    {
        //buf[idx] = (data >> 8) & 0xff;
        //idx++;
        //buf[idx] = data & 0xff;
        //idx++;
    }
    else
    {
        buf[idx] = data & 0xff;
        idx++;
    }

    ret = write(g_fd[ViPipe], buf, OS08A10_addr_byte + OS08A10_data_byte);
    if (ret < 0)
    {
        ISP_TRACE(HI_DBG_ERR, "I2C_WRITE error!\n");
        return HI_FAILURE;
    }

#endif
    return HI_SUCCESS;
}

void OS08A10_standby(VI_PIPE ViPipe)
{
    return;
}

void OS08A10_restart(VI_PIPE ViPipe)
{
    return;
}
static void delay_ms(int ms) {
    usleep(ms * 1000);
}

//void OS08A10_wdr_1080p30_2to1_init(VI_PIPE ViPipe);
void OS08A10_wdr_4k2k30_2to1_init(VI_PIPE ViPipe);
void OS08A10_wdr_4k2k25_2to1_init(VI_PIPE ViPipe);
//void OS08A10_wdr_1080p120_2to1_init(VI_PIPE ViPipe);
//void OS08A10_wdr_720p60_2to1_init(VI_PIPE ViPipe);
//void OS08A10_wdr_1080p30_3to1_init(VI_PIPE ViPipe);
//void OS08A10_wdr_1080p120_3to1_init(VI_PIPE ViPipe);
//void OS08A10_wdr_720p60_3to1_init(VI_PIPE ViPipe);
void OS08A10_linear_4k2k30_init(VI_PIPE ViPipe);
void OS08A10_linear_1080P120_init(VI_PIPE ViPipe);

void OS08A10_linear_4k2k25_init(VI_PIPE ViPipe);
//void OS08A10_linear_1080p60_init(VI_PIPE ViPipe);
//void OS08A10_linear_720p60_init(VI_PIPE ViPipe);

void OS08A10_default_reg_init(VI_PIPE ViPipe)
{
    HI_U32 i;

    for (i = 0; i < g_pastOS08A10[ViPipe]->astRegsInfo[0].u32RegNum; i++)
    {
        OS08A10_write_register(ViPipe, g_pastOS08A10[ViPipe]->astRegsInfo[0].astI2cData[i].u32RegAddr, g_pastOS08A10[ViPipe]->astRegsInfo[0].astI2cData[i].u32Data);
    }
}

void OS08A10_init(VI_PIPE ViPipe)
{
    //HI_U32           i;
    WDR_MODE_E       enWDRMode;
    HI_BOOL          bInit;

    bInit       = g_pastOS08A10[ViPipe]->bInit;
    enWDRMode   = g_pastOS08A10[ViPipe]->enWDRMode;

    /* When sensor first init, config all registers */
    if (HI_FALSE == bInit)
    {
        /*sensor i2c init */
        //printf("binit false OS08A10 i2c init\n");
        OS08A10_i2c_init(ViPipe);
    }

    /* When sensor first init, config all registers */

    if (WDR_MODE_2To1_LINE == enWDRMode)
    {
    		/*
        if (OS08A10_5M_30FPS_10BIT_WDR_MODE == u8ImgMode)
        {
            OS08A10_wdr_5M30_10bit_init(ViPipe);
        }
        else if (OS08A10_4M_25FPS_10BIT_WDR_MODE == u8ImgMode)
        {
            OS08A10_wdr_4M25_10bit_init(ViPipe);
        }
        else
        */
        {
            printf("OS08A10_init Not support this mode\n");
            return;
        }

    }
    else if (WDR_MODE_NONE == enWDRMode)
    {
        ///OS08A10_linear_5M30_12bit_init(ViPipe);
				OS08A10_linear_4k2k30_init(ViPipe);
    }
    else
    {
        printf("OS08A10_init Not support this mode\n");
        return;
    }

    g_pastOS08A10[ViPipe]->bInit = HI_TRUE;

    return ;
}

void OS08A10_exit(VI_PIPE ViPipe)
{
    OS08A10_i2c_exit(ViPipe);

    return;
}


/* 4k2k30 10bit */
void OS08A10_linear_4k2k30_init(VI_PIPE ViPipe)
{
    OS08A10_write_register (ViPipe,0x0103, 0x01);
    OS08A10_write_register (ViPipe,0x0303, 0x01);
    OS08A10_write_register (ViPipe,0x0305, 0x5c);
    OS08A10_write_register (ViPipe,0x0306, 0x00);
    OS08A10_write_register (ViPipe,0x0307, 0x01);
    OS08A10_write_register (ViPipe,0x0308, 0x03);
    OS08A10_write_register (ViPipe,0x0309, 0x04);
    OS08A10_write_register (ViPipe,0x032a, 0x00);
    OS08A10_write_register (ViPipe,0x300f, 0x11);
    OS08A10_write_register (ViPipe,0x3010, 0x01);
    OS08A10_write_register (ViPipe,0x3011, 0x04);
    OS08A10_write_register (ViPipe,0x3012, 0x41);
    OS08A10_write_register (ViPipe,0x3016, 0xf0);
    OS08A10_write_register (ViPipe,0x301e, 0x98);
    OS08A10_write_register (ViPipe,0x3031, 0xa9);
    OS08A10_write_register (ViPipe,0x3603, 0x2c);
    OS08A10_write_register (ViPipe,0x3103, 0x92);
    OS08A10_write_register (ViPipe,0x3104, 0x01);
    OS08A10_write_register (ViPipe,0x3106, 0x10);
    OS08A10_write_register (ViPipe,0x3400, 0x04);
    OS08A10_write_register (ViPipe,0x3025, 0x03);
    OS08A10_write_register (ViPipe,0x3425, 0x51);
    OS08A10_write_register (ViPipe,0x3428, 0x01);
    OS08A10_write_register (ViPipe,0x3406, 0x08);
    OS08A10_write_register (ViPipe,0x3408, 0x03);
    OS08A10_write_register (ViPipe,0x340c, 0xff);
    OS08A10_write_register (ViPipe,0x340d, 0xff);
    OS08A10_write_register (ViPipe,0x031e, 0x09);
    OS08A10_write_register (ViPipe,0x3501, 0x09);
    OS08A10_write_register (ViPipe,0x3502, 0x6e);
    OS08A10_write_register (ViPipe,0x3505, 0x83);
    OS08A10_write_register (ViPipe,0x3508, 0x01);
    OS08A10_write_register (ViPipe,0x3509, 0x00);
    OS08A10_write_register (ViPipe,0x350a, 0x04);
    OS08A10_write_register (ViPipe,0x350b, 0x00);
    OS08A10_write_register (ViPipe,0x350c, 0x00);
    OS08A10_write_register (ViPipe,0x350d, 0x80);
    OS08A10_write_register (ViPipe,0x350e, 0x04);
    OS08A10_write_register (ViPipe,0x350f, 0x00);
    OS08A10_write_register (ViPipe,0x3600, 0x00);
    OS08A10_write_register (ViPipe,0x3605, 0x50);
    OS08A10_write_register (ViPipe,0x3609, 0xb5);
    OS08A10_write_register (ViPipe,0x3610, 0x39);
    OS08A10_write_register (ViPipe,0x360c, 0x01);
    OS08A10_write_register (ViPipe,0x360e, 0x86);
    OS08A10_write_register (ViPipe,0x3628, 0xa4);
    OS08A10_write_register (ViPipe,0x362d, 0x10);
    OS08A10_write_register (ViPipe,0x3660, 0x43);
    OS08A10_write_register (ViPipe,0x3661, 0x06);
    OS08A10_write_register (ViPipe,0x3662, 0x00);
    OS08A10_write_register (ViPipe,0x3663, 0x28);
    OS08A10_write_register (ViPipe,0x3664, 0x0d);
    OS08A10_write_register (ViPipe,0x366a, 0x38);
    OS08A10_write_register (ViPipe,0x366b, 0xa0);
    OS08A10_write_register (ViPipe,0x366d, 0x00);
    OS08A10_write_register (ViPipe,0x366e, 0x00);
    OS08A10_write_register (ViPipe,0x3680, 0x00);
    OS08A10_write_register (ViPipe,0x3705, 0x00);
    OS08A10_write_register (ViPipe,0x3706, 0x35);
    OS08A10_write_register (ViPipe,0x370a, 0x00);
    OS08A10_write_register (ViPipe,0x370b, 0x98);
    OS08A10_write_register (ViPipe,0x3709, 0x49);
    OS08A10_write_register (ViPipe,0x3714, 0x21);
    OS08A10_write_register (ViPipe,0x371c, 0x00);
    OS08A10_write_register (ViPipe,0x371d, 0x08);
    OS08A10_write_register (ViPipe,0x375e, 0x0b);
    OS08A10_write_register (ViPipe,0x3762, 0x11);
    OS08A10_write_register (ViPipe,0x3776, 0x10);
    OS08A10_write_register (ViPipe,0x3781, 0x02);
    OS08A10_write_register (ViPipe,0x3782, 0x04);
    OS08A10_write_register (ViPipe,0x3783, 0x02);
    OS08A10_write_register (ViPipe,0x3784, 0x08);
    OS08A10_write_register (ViPipe,0x3785, 0x08);
    OS08A10_write_register (ViPipe,0x3788, 0x01);
    OS08A10_write_register (ViPipe,0x3789, 0x01);
    OS08A10_write_register (ViPipe,0x3797, 0x04);
    OS08A10_write_register (ViPipe,0x3800, 0x00);
    OS08A10_write_register (ViPipe,0x3801, 0x00);
    OS08A10_write_register (ViPipe,0x3802, 0x00);
    OS08A10_write_register (ViPipe,0x3803, 0x0c);
    OS08A10_write_register (ViPipe,0x3804, 0x0e);
    OS08A10_write_register (ViPipe,0x3805, 0xff);
    OS08A10_write_register (ViPipe,0x3806, 0x08);
    OS08A10_write_register (ViPipe,0x3807, 0x6f);
    OS08A10_write_register (ViPipe,0x3808, 0x0f);
    OS08A10_write_register (ViPipe,0x3809, 0x00);
    OS08A10_write_register (ViPipe,0x380a, 0x08);
    OS08A10_write_register (ViPipe,0x380b, 0x70);
    OS08A10_write_register (ViPipe,0x380c, 0x08);
    OS08A10_write_register (ViPipe,0x380d, 0x18);
    OS08A10_write_register (ViPipe,0x380e, 0x09);
    OS08A10_write_register (ViPipe,0x380f, 0x0d);
    OS08A10_write_register (ViPipe,0x3813, 0x10);
    OS08A10_write_register (ViPipe,0x3814, 0x01);
    OS08A10_write_register (ViPipe,0x3815, 0x01);
    OS08A10_write_register (ViPipe,0x3816, 0x01);
    OS08A10_write_register (ViPipe,0x3817, 0x01);
    OS08A10_write_register (ViPipe,0x381c, 0x00);
    //OS08A10_write_register (ViPipe,0x3820, 0x00);
    //OS08A10_write_register (ViPipe,0x3821, 0x00);
    //mirror
    OS08A10_write_register (ViPipe,0x3820, 0x00);
    OS08A10_write_register (ViPipe,0x3821, 0x04);
    OS08A10_write_register (ViPipe,0x3823, 0x08);
    OS08A10_write_register (ViPipe,0x3826, 0x00);
    OS08A10_write_register (ViPipe,0x3827, 0x08);
    OS08A10_write_register (ViPipe,0x382d, 0x08);
    OS08A10_write_register (ViPipe,0x3832, 0x02);
    OS08A10_write_register (ViPipe,0x383c, 0x48);
    OS08A10_write_register (ViPipe,0x383d, 0xff);
    OS08A10_write_register (ViPipe,0x3d85, 0x0b);
    OS08A10_write_register (ViPipe,0x3d84, 0x40);
    OS08A10_write_register (ViPipe,0x3d8c, 0x63);
    OS08A10_write_register (ViPipe,0x3d8d, 0xd7);
    OS08A10_write_register (ViPipe,0x4000, 0xf8);
    OS08A10_write_register (ViPipe,0x4001, 0x2f);
    OS08A10_write_register (ViPipe,0x400a, 0x01);
    OS08A10_write_register (ViPipe,0x400f, 0xa1);
    OS08A10_write_register (ViPipe,0x4010, 0x12);
    OS08A10_write_register (ViPipe,0x4018, 0x04);
    OS08A10_write_register (ViPipe,0x4008, 0x02);
    OS08A10_write_register (ViPipe,0x4009, 0x0d);
    OS08A10_write_register (ViPipe,0x401a, 0x58);
    OS08A10_write_register (ViPipe,0x4050, 0x00);
    OS08A10_write_register (ViPipe,0x4051, 0x01);
    OS08A10_write_register (ViPipe,0x4028, 0x0f);
    OS08A10_write_register (ViPipe,0x4052, 0x00);
    OS08A10_write_register (ViPipe,0x4053, 0x80);
    OS08A10_write_register (ViPipe,0x4054, 0x00);
    OS08A10_write_register (ViPipe,0x4055, 0x80);
    OS08A10_write_register (ViPipe,0x4056, 0x00);
    OS08A10_write_register (ViPipe,0x4057, 0x80);
    OS08A10_write_register (ViPipe,0x4058, 0x00);
    OS08A10_write_register (ViPipe,0x4059, 0x80);
    OS08A10_write_register (ViPipe,0x430b, 0xff);
    OS08A10_write_register (ViPipe,0x430c, 0xff);
    OS08A10_write_register (ViPipe,0x430d, 0x00);
    OS08A10_write_register (ViPipe,0x430e, 0x00);
    OS08A10_write_register (ViPipe,0x4501, 0x18);
    OS08A10_write_register (ViPipe,0x4502, 0x00);
    OS08A10_write_register (ViPipe,0x4643, 0x00);
    OS08A10_write_register (ViPipe,0x4640, 0x01);
    OS08A10_write_register (ViPipe,0x4641, 0x04);
    OS08A10_write_register (ViPipe,0x4800, 0x04);
    OS08A10_write_register (ViPipe,0x4809, 0x2b);
    OS08A10_write_register (ViPipe,0x4813, 0x90);
    OS08A10_write_register (ViPipe,0x4817, 0x04);
    OS08A10_write_register (ViPipe,0x4833, 0x18);
    OS08A10_write_register (ViPipe,0x4837, 0x15);
    OS08A10_write_register (ViPipe,0x483b, 0x00);
    OS08A10_write_register (ViPipe,0x484b, 0x03);
    OS08A10_write_register (ViPipe,0x4850, 0x7c);
    OS08A10_write_register (ViPipe,0x4852, 0x06);
    OS08A10_write_register (ViPipe,0x4856, 0x58);
    OS08A10_write_register (ViPipe,0x4857, 0xaa);
    OS08A10_write_register (ViPipe,0x4862, 0x0a);
    OS08A10_write_register (ViPipe,0x4869, 0x18);
    OS08A10_write_register (ViPipe,0x486a, 0xaa);
    OS08A10_write_register (ViPipe,0x486e, 0x03);
    OS08A10_write_register (ViPipe,0x486f, 0x55);
    OS08A10_write_register (ViPipe,0x4875, 0xf0);
    OS08A10_write_register (ViPipe,0x5000, 0x89);
    OS08A10_write_register (ViPipe,0x5001, 0x42);
    OS08A10_write_register (ViPipe,0x5004, 0x40);
    OS08A10_write_register (ViPipe,0x5005, 0x00);
    OS08A10_write_register (ViPipe,0x5180, 0x00);
    OS08A10_write_register (ViPipe,0x5181, 0x10);
    OS08A10_write_register (ViPipe,0x580b, 0x03);
    OS08A10_write_register (ViPipe,0x4700, 0x2b);
    OS08A10_write_register (ViPipe,0x4e00, 0x2b);
    OS08A10_default_reg_init(ViPipe);
    delay_ms(5);  //delay 5ms
    OS08A10_write_register (ViPipe,0x0100, 0x01);

    printf("=++++++++==OS08A10 4k2k 30fps 10bit LINE Init OK!===\n");
    return;
}

void OS08A10_linear_1080P120_init(VI_PIPE ViPipe)
{
    OS08A10_write_register(ViPipe, 0x0103, 0x01);
    OS08A10_write_register(ViPipe, 0x0303, 0x01);
    OS08A10_write_register(ViPipe, 0x0305, 0x5c);
    OS08A10_write_register(ViPipe, 0x0306, 0x00);
    OS08A10_write_register(ViPipe, 0x0308, 0x03);
    OS08A10_write_register(ViPipe, 0x0309, 0x04);
    OS08A10_write_register(ViPipe, 0x032a, 0x00);
    OS08A10_write_register(ViPipe, 0x300f, 0x11);
    OS08A10_write_register(ViPipe, 0x3010, 0x01);
    OS08A10_write_register(ViPipe, 0x3011, 0x04);
    OS08A10_write_register(ViPipe, 0x3012, 0x41);
    OS08A10_write_register(ViPipe, 0x3016, 0xf0);
    OS08A10_write_register(ViPipe, 0x301e, 0x98);
    OS08A10_write_register(ViPipe, 0x3031, 0xa9);
    OS08A10_write_register(ViPipe, 0x3603, 0x2c);
    OS08A10_write_register(ViPipe, 0x3103, 0x92);
    OS08A10_write_register(ViPipe, 0x3104, 0x01);
    OS08A10_write_register(ViPipe, 0x3106, 0x10);
    OS08A10_write_register(ViPipe, 0x3400, 0x04);
    OS08A10_write_register(ViPipe, 0x3025, 0x03);
    OS08A10_write_register(ViPipe, 0x3425, 0x51);
    OS08A10_write_register(ViPipe, 0x3428, 0x01);
    OS08A10_write_register(ViPipe, 0x3406, 0x08);
    OS08A10_write_register(ViPipe, 0x3408, 0x03);
    OS08A10_write_register(ViPipe, 0x340c, 0xff);
    OS08A10_write_register(ViPipe, 0x340d, 0xff);
    OS08A10_write_register(ViPipe, 0x031e, 0x09);
    OS08A10_write_register(ViPipe, 0x3501, 0x04);
    OS08A10_write_register(ViPipe, 0x3502, 0x62);
    OS08A10_write_register(ViPipe, 0x3505, 0x83);
    OS08A10_write_register(ViPipe, 0x3508, 0x04);
    OS08A10_write_register(ViPipe, 0x3509, 0x00);
    OS08A10_write_register(ViPipe, 0x350a, 0x04);
    OS08A10_write_register(ViPipe, 0x350b, 0x00);
    OS08A10_write_register(ViPipe, 0x350c, 0x00);
    OS08A10_write_register(ViPipe, 0x350d, 0x80);
    OS08A10_write_register(ViPipe, 0x350e, 0x04);
    OS08A10_write_register(ViPipe, 0x350f, 0x00);
    OS08A10_write_register(ViPipe, 0x3600, 0x09);
    OS08A10_write_register(ViPipe, 0x3605, 0x50);
    OS08A10_write_register(ViPipe, 0x3609, 0xb5);
    OS08A10_write_register(ViPipe, 0x3610, 0x39);
    OS08A10_write_register(ViPipe, 0x360c, 0x01);
    OS08A10_write_register(ViPipe, 0x360e, 0x86);
    OS08A10_write_register(ViPipe, 0x3628, 0xa4);
    OS08A10_write_register(ViPipe, 0x362d, 0x10);
    OS08A10_write_register(ViPipe, 0x3660, 0x43);
    OS08A10_write_register(ViPipe, 0x3661, 0x06);
    OS08A10_write_register(ViPipe, 0x3662, 0x00);
    OS08A10_write_register(ViPipe, 0x3663, 0x28);
    OS08A10_write_register(ViPipe, 0x3664, 0x0d);
    OS08A10_write_register(ViPipe, 0x366a, 0x38);
    OS08A10_write_register(ViPipe, 0x366b, 0xa0);
    OS08A10_write_register(ViPipe, 0x366d, 0x00);
    OS08A10_write_register(ViPipe, 0x366e, 0x00);
    OS08A10_write_register(ViPipe, 0x3680, 0x00);
    OS08A10_write_register(ViPipe, 0x3701, 0x02);
    OS08A10_write_register(ViPipe, 0x373b, 0x02);
    OS08A10_write_register(ViPipe, 0x373c, 0x02);
    OS08A10_write_register(ViPipe, 0x3736, 0x02);
    OS08A10_write_register(ViPipe, 0x3737, 0x02);
    OS08A10_write_register(ViPipe, 0x3705, 0x00);
    OS08A10_write_register(ViPipe, 0x3706, 0x35);
    OS08A10_write_register(ViPipe, 0x370a, 0x00);
    OS08A10_write_register(ViPipe, 0x370b, 0x98);
    OS08A10_write_register(ViPipe, 0x3709, 0x49);
    OS08A10_write_register(ViPipe, 0x3714, 0x22);
    OS08A10_write_register(ViPipe, 0x371c, 0x00);
    OS08A10_write_register(ViPipe, 0x371d, 0x08);
    OS08A10_write_register(ViPipe, 0x375e, 0x0b);
    OS08A10_write_register(ViPipe, 0x3762, 0x11);
    OS08A10_write_register(ViPipe, 0x3776, 0x10);
    OS08A10_write_register(ViPipe, 0x3781, 0x02);
    OS08A10_write_register(ViPipe, 0x3782, 0x04);
    OS08A10_write_register(ViPipe, 0x3783, 0x02);
    OS08A10_write_register(ViPipe, 0x3784, 0x08);
    OS08A10_write_register(ViPipe, 0x3785, 0x08);
    OS08A10_write_register(ViPipe, 0x3788, 0x01);
    OS08A10_write_register(ViPipe, 0x3789, 0x01);
    OS08A10_write_register(ViPipe, 0x3797, 0x04);
    OS08A10_write_register(ViPipe, 0x3800, 0x00);
    OS08A10_write_register(ViPipe, 0x3801, 0x00);
    OS08A10_write_register(ViPipe, 0x3802, 0x00);
    OS08A10_write_register(ViPipe, 0x3803, 0x0c);
    OS08A10_write_register(ViPipe, 0x3804, 0x0e);
    OS08A10_write_register(ViPipe, 0x3805, 0xff);
    OS08A10_write_register(ViPipe, 0x3806, 0x08);
    OS08A10_write_register(ViPipe, 0x3807, 0x6f);
    OS08A10_write_register(ViPipe, 0x3808, 0x07);
    OS08A10_write_register(ViPipe, 0x3809, 0x80);
    OS08A10_write_register(ViPipe, 0x380a, 0x04);
    OS08A10_write_register(ViPipe, 0x380b, 0x38);
    OS08A10_write_register(ViPipe, 0x380c, 0x04);
    OS08A10_write_register(ViPipe, 0x380d, 0x0c);
    OS08A10_write_register(ViPipe, 0x380e, 0x04);
    OS08A10_write_register(ViPipe, 0x380f, 0x86);
    OS08A10_write_register(ViPipe, 0x3813, 0x08);
    OS08A10_write_register(ViPipe, 0x3814, 0x03);
    OS08A10_write_register(ViPipe, 0x3815, 0x01);
    OS08A10_write_register(ViPipe, 0x3816, 0x03);
    OS08A10_write_register(ViPipe, 0x3817, 0x01);
    OS08A10_write_register(ViPipe, 0x381c, 0x00);
    //OS08A10_write_register(ViPipe,0x3820, 0x01);
    //OS08A10_write_register(ViPipe,0x3821, 0x01);

    OS08A10_write_register (ViPipe, 0x3820, 0x00);
    OS08A10_write_register (ViPipe, 0x3821, 0x04);

    OS08A10_write_register(ViPipe, 0x3823, 0x08);
    OS08A10_write_register(ViPipe, 0x3826, 0x00);
    OS08A10_write_register(ViPipe, 0x3827, 0x08);
    OS08A10_write_register(ViPipe, 0x382d, 0x08);
    OS08A10_write_register(ViPipe, 0x3832, 0x02);
    OS08A10_write_register(ViPipe, 0x383c, 0x48);
    OS08A10_write_register(ViPipe, 0x383d, 0xff);
    OS08A10_write_register(ViPipe, 0x3d85, 0x0b);
    OS08A10_write_register(ViPipe, 0x3d84, 0x40);
    OS08A10_write_register(ViPipe, 0x3d8c, 0x63);
    OS08A10_write_register(ViPipe, 0x3d8d, 0xd7);
    OS08A10_write_register(ViPipe, 0x4000, 0xf8);
    OS08A10_write_register(ViPipe, 0x4001, 0x2f);
    OS08A10_write_register(ViPipe, 0x400a, 0x01);
    OS08A10_write_register(ViPipe, 0x400f, 0xa1);
    OS08A10_write_register(ViPipe, 0x4010, 0x12);
    OS08A10_write_register(ViPipe, 0x4018, 0x04);
    OS08A10_write_register(ViPipe, 0x4008, 0x02);
    OS08A10_write_register(ViPipe, 0x4009, 0x05);
    OS08A10_write_register(ViPipe, 0x401a, 0x58);
    OS08A10_write_register(ViPipe, 0x4050, 0x00);
    OS08A10_write_register(ViPipe, 0x4051, 0x01);
    OS08A10_write_register(ViPipe, 0x4028, 0x0f);
    OS08A10_write_register(ViPipe, 0x4052, 0x00);
    OS08A10_write_register(ViPipe, 0x4053, 0x80);
    OS08A10_write_register(ViPipe, 0x4054, 0x00);
    OS08A10_write_register(ViPipe, 0x4055, 0x80);
    OS08A10_write_register(ViPipe, 0x4056, 0x00);
    OS08A10_write_register(ViPipe, 0x4057, 0x80);
    OS08A10_write_register(ViPipe, 0x4058, 0x00);
    OS08A10_write_register(ViPipe, 0x4059, 0x80);
    OS08A10_write_register(ViPipe, 0x430b, 0xff);
    OS08A10_write_register(ViPipe, 0x430c, 0xff);
    OS08A10_write_register(ViPipe, 0x430d, 0x00);
    OS08A10_write_register(ViPipe, 0x430e, 0x00);
    OS08A10_write_register(ViPipe, 0x4501, 0x98);
    OS08A10_write_register(ViPipe, 0x4502, 0x00);
    OS08A10_write_register(ViPipe, 0x4643, 0x00);
    OS08A10_write_register(ViPipe, 0x4640, 0x01);
    OS08A10_write_register(ViPipe, 0x4641, 0x04);
    OS08A10_write_register(ViPipe, 0x4800, 0x04);
    OS08A10_write_register(ViPipe, 0x4809, 0x2b);
    OS08A10_write_register(ViPipe, 0x4813, 0x90);
    OS08A10_write_register(ViPipe, 0x4817, 0x04);
    OS08A10_write_register(ViPipe, 0x4833, 0x18);
    OS08A10_write_register(ViPipe, 0x4837, 0x0a);
    OS08A10_write_register(ViPipe, 0x483b, 0x00);
    OS08A10_write_register(ViPipe, 0x484b, 0x03);
    OS08A10_write_register(ViPipe, 0x4850, 0x7c);
    OS08A10_write_register(ViPipe, 0x4852, 0x06);
    OS08A10_write_register(ViPipe, 0x4856, 0x58);
    OS08A10_write_register(ViPipe, 0x4857, 0xaa);
    OS08A10_write_register(ViPipe, 0x4862, 0x0a);
    OS08A10_write_register(ViPipe, 0x4869, 0x18);
    OS08A10_write_register(ViPipe, 0x486a, 0xaa);
    OS08A10_write_register(ViPipe, 0x486e, 0x03);
    OS08A10_write_register(ViPipe, 0x486f, 0x55);
    OS08A10_write_register(ViPipe, 0x4875, 0xf0);
    OS08A10_write_register(ViPipe, 0x5000, 0x89);
    OS08A10_write_register(ViPipe, 0x5001, 0x42);
    OS08A10_write_register(ViPipe, 0x5004, 0x40);
    OS08A10_write_register(ViPipe, 0x5005, 0x00);
    OS08A10_write_register(ViPipe, 0x5180, 0x00);
    OS08A10_write_register(ViPipe, 0x5181, 0x10);
    OS08A10_write_register(ViPipe, 0x580b, 0x03);
    OS08A10_write_register(ViPipe, 0x4700, 0x2b);
    OS08A10_write_register(ViPipe, 0x4e00, 0x2b);
    OS08A10_default_reg_init(ViPipe);
    delay_ms(5);  //delay 5ms
    OS08A10_write_register(ViPipe, 0x0100, 0x01);
    OS08A10_write_register(ViPipe, 0x0100, 0x01);
    OS08A10_write_register(ViPipe, 0x0100, 0x01);
    OS08A10_write_register(ViPipe, 0x0100, 0x01);

    printf("===OS08A10 1080P 120fps 10bit LINE Init OK!===---------\n");
    return;
}

/* 4k2k25 10bit */
void OS08A10_linear_4k2k25_init(VI_PIPE ViPipe)
{
    OS08A10_write_register (ViPipe,0x0103, 0x01);
    OS08A10_write_register (ViPipe,0x0303, 0x01);
    OS08A10_write_register (ViPipe,0x0305, 0x5c);
    OS08A10_write_register (ViPipe,0x0306, 0x00);
    OS08A10_write_register (ViPipe,0x0307, 0x01);
    OS08A10_write_register (ViPipe,0x0308, 0x03);
    OS08A10_write_register (ViPipe,0x0309, 0x04);
    OS08A10_write_register (ViPipe,0x032a, 0x00);
    OS08A10_write_register (ViPipe,0x300f, 0x11);
    OS08A10_write_register (ViPipe,0x3010, 0x01);
    OS08A10_write_register (ViPipe,0x3011, 0x04);
    OS08A10_write_register (ViPipe,0x3012, 0x41);
    OS08A10_write_register (ViPipe,0x3016, 0xf0);
    OS08A10_write_register (ViPipe,0x301e, 0x98);
    OS08A10_write_register (ViPipe,0x3031, 0xa9);
    OS08A10_write_register (ViPipe,0x3603, 0x2c);
    OS08A10_write_register (ViPipe,0x3103, 0x92);
    OS08A10_write_register (ViPipe,0x3104, 0x01);
    OS08A10_write_register (ViPipe,0x3106, 0x10);
    OS08A10_write_register (ViPipe,0x3400, 0x04);
    OS08A10_write_register (ViPipe,0x3025, 0x03);
    OS08A10_write_register (ViPipe,0x3425, 0x51);
    OS08A10_write_register (ViPipe,0x3428, 0x01);
    OS08A10_write_register (ViPipe,0x3406, 0x08);
    OS08A10_write_register (ViPipe,0x3408, 0x03);
    OS08A10_write_register (ViPipe,0x340c, 0xff);
    OS08A10_write_register (ViPipe,0x340d, 0xff);
    OS08A10_write_register (ViPipe,0x031e, 0x09);
    OS08A10_write_register (ViPipe,0x3501, 0x09);
    OS08A10_write_register (ViPipe,0x3502, 0x6e);
    OS08A10_write_register (ViPipe,0x3505, 0x83);
    OS08A10_write_register (ViPipe,0x3508, 0x01);
    OS08A10_write_register (ViPipe,0x3509, 0x00);
    OS08A10_write_register (ViPipe,0x350a, 0x04);
    OS08A10_write_register (ViPipe,0x350b, 0x00);
    OS08A10_write_register (ViPipe,0x350c, 0x00);
    OS08A10_write_register (ViPipe,0x350d, 0x80);
    OS08A10_write_register (ViPipe,0x350e, 0x04);
    OS08A10_write_register (ViPipe,0x350f, 0x00);
    OS08A10_write_register (ViPipe,0x3600, 0x00);
    OS08A10_write_register (ViPipe,0x3605, 0x50);
    OS08A10_write_register (ViPipe,0x3609, 0xb5);
    OS08A10_write_register (ViPipe,0x3610, 0x39);
    OS08A10_write_register (ViPipe,0x360c, 0x01);
    OS08A10_write_register (ViPipe,0x360e, 0x86);
    OS08A10_write_register (ViPipe,0x3628, 0xa4);
    OS08A10_write_register (ViPipe,0x362d, 0x10);
    OS08A10_write_register (ViPipe,0x3660, 0x43);
    OS08A10_write_register (ViPipe,0x3661, 0x06);
    OS08A10_write_register (ViPipe,0x3662, 0x00);
    OS08A10_write_register (ViPipe,0x3663, 0x28);
    OS08A10_write_register (ViPipe,0x3664, 0x0d);
    OS08A10_write_register (ViPipe,0x366a, 0x38);
    OS08A10_write_register (ViPipe,0x366b, 0xa0);
    OS08A10_write_register (ViPipe,0x366d, 0x00);
    OS08A10_write_register (ViPipe,0x366e, 0x00);
    OS08A10_write_register (ViPipe,0x3680, 0x00);
    OS08A10_write_register (ViPipe,0x3705, 0x00);
    OS08A10_write_register (ViPipe,0x3706, 0x35);
    OS08A10_write_register (ViPipe,0x370a, 0x00);
    OS08A10_write_register (ViPipe,0x370b, 0x98);
    OS08A10_write_register (ViPipe,0x3709, 0x49);
    OS08A10_write_register (ViPipe,0x3714, 0x21);
    OS08A10_write_register (ViPipe,0x371c, 0x00);
    OS08A10_write_register (ViPipe,0x371d, 0x08);
    OS08A10_write_register (ViPipe,0x375e, 0x0b);
    OS08A10_write_register (ViPipe,0x3762, 0x11);
    OS08A10_write_register (ViPipe,0x3776, 0x10);
    OS08A10_write_register (ViPipe,0x3781, 0x02);
    OS08A10_write_register (ViPipe,0x3782, 0x04);
    OS08A10_write_register (ViPipe,0x3783, 0x02);
    OS08A10_write_register (ViPipe,0x3784, 0x08);
    OS08A10_write_register (ViPipe,0x3785, 0x08);
    OS08A10_write_register (ViPipe,0x3788, 0x01);
    OS08A10_write_register (ViPipe,0x3789, 0x01);
    OS08A10_write_register (ViPipe,0x3797, 0x04);
    OS08A10_write_register (ViPipe,0x3800, 0x00);
    OS08A10_write_register (ViPipe,0x3801, 0x00);
    OS08A10_write_register (ViPipe,0x3802, 0x00);
    OS08A10_write_register (ViPipe,0x3803, 0x0c);
    OS08A10_write_register (ViPipe,0x3804, 0x0e);
    OS08A10_write_register (ViPipe,0x3805, 0xff);
    OS08A10_write_register (ViPipe,0x3806, 0x08);
    OS08A10_write_register (ViPipe,0x3807, 0x6f);
    OS08A10_write_register (ViPipe,0x3808, 0x0f);
    OS08A10_write_register (ViPipe,0x3809, 0x00);
    OS08A10_write_register (ViPipe,0x380a, 0x08);
    OS08A10_write_register (ViPipe,0x380b, 0x70);
    OS08A10_write_register (ViPipe,0x380c, 0x09);
    OS08A10_write_register (ViPipe,0x380d, 0xb6);
    OS08A10_write_register (ViPipe,0x380e, 0x09);
    OS08A10_write_register (ViPipe,0x380f, 0x0d);
    OS08A10_write_register (ViPipe,0x3813, 0x10);
    OS08A10_write_register (ViPipe,0x3814, 0x01);
    OS08A10_write_register (ViPipe,0x3815, 0x01);
    OS08A10_write_register (ViPipe,0x3816, 0x01);
    OS08A10_write_register (ViPipe,0x3817, 0x01);
    OS08A10_write_register (ViPipe,0x381c, 0x00);
    //OS08A10_write_register (ViPipe,0x3820, 0x00);
    //OS08A10_write_register (ViPipe,0x3821, 0x00);
    //mirror
    OS08A10_write_register (ViPipe,0x3820, 0x00);
    OS08A10_write_register (ViPipe,0x3821, 0x04);
    OS08A10_write_register (ViPipe,0x3823, 0x08);
    OS08A10_write_register (ViPipe,0x3826, 0x00);
    OS08A10_write_register (ViPipe,0x3827, 0x08);
    OS08A10_write_register (ViPipe,0x382d, 0x08);
    OS08A10_write_register (ViPipe,0x3832, 0x02);
    OS08A10_write_register (ViPipe,0x383c, 0x48);
    OS08A10_write_register (ViPipe,0x383d, 0xff);
    OS08A10_write_register (ViPipe,0x3d85, 0x0b);
    OS08A10_write_register (ViPipe,0x3d84, 0x40);
    OS08A10_write_register (ViPipe,0x3d8c, 0x63);
    OS08A10_write_register (ViPipe,0x3d8d, 0xd7);
    OS08A10_write_register (ViPipe,0x4000, 0xf8);
    OS08A10_write_register (ViPipe,0x4001, 0x2f);
    OS08A10_write_register (ViPipe,0x400a, 0x01);
    OS08A10_write_register (ViPipe,0x400f, 0xa1);
    OS08A10_write_register (ViPipe,0x4010, 0x12);
    OS08A10_write_register (ViPipe,0x4018, 0x04);
    OS08A10_write_register (ViPipe,0x4008, 0x02);
    OS08A10_write_register (ViPipe,0x4009, 0x0d);
    OS08A10_write_register (ViPipe,0x401a, 0x58);
    OS08A10_write_register (ViPipe,0x4050, 0x00);
    OS08A10_write_register (ViPipe,0x4051, 0x01);
    OS08A10_write_register (ViPipe,0x4028, 0x0f);
    OS08A10_write_register (ViPipe,0x4052, 0x00);
    OS08A10_write_register (ViPipe,0x4053, 0x80);
    OS08A10_write_register (ViPipe,0x4054, 0x00);
    OS08A10_write_register (ViPipe,0x4055, 0x80);
    OS08A10_write_register (ViPipe,0x4056, 0x00);
    OS08A10_write_register (ViPipe,0x4057, 0x80);
    OS08A10_write_register (ViPipe,0x4058, 0x00);
    OS08A10_write_register (ViPipe,0x4059, 0x80);
    OS08A10_write_register (ViPipe,0x430b, 0xff);
    OS08A10_write_register (ViPipe,0x430c, 0xff);
    OS08A10_write_register (ViPipe,0x430d, 0x00);
    OS08A10_write_register (ViPipe,0x430e, 0x00);
    OS08A10_write_register (ViPipe,0x4501, 0x18);
    OS08A10_write_register (ViPipe,0x4502, 0x00);
    OS08A10_write_register (ViPipe,0x4643, 0x00);
    OS08A10_write_register (ViPipe,0x4640, 0x01);
    OS08A10_write_register (ViPipe,0x4641, 0x04);
    OS08A10_write_register (ViPipe,0x4800, 0x04);
    OS08A10_write_register (ViPipe,0x4809, 0x2b);
    OS08A10_write_register (ViPipe,0x4813, 0x90);
    OS08A10_write_register (ViPipe,0x4817, 0x04);
    OS08A10_write_register (ViPipe,0x4833, 0x18);
    OS08A10_write_register (ViPipe,0x4837, 0x15);
    OS08A10_write_register (ViPipe,0x483b, 0x00);
    OS08A10_write_register (ViPipe,0x484b, 0x03);
    OS08A10_write_register (ViPipe,0x4850, 0x7c);
    OS08A10_write_register (ViPipe,0x4852, 0x06);
    OS08A10_write_register (ViPipe,0x4856, 0x58);
    OS08A10_write_register (ViPipe,0x4857, 0xaa);
    OS08A10_write_register (ViPipe,0x4862, 0x0a);
    OS08A10_write_register (ViPipe,0x4869, 0x18);
    OS08A10_write_register (ViPipe,0x486a, 0xaa);
    OS08A10_write_register (ViPipe,0x486e, 0x03);
    OS08A10_write_register (ViPipe,0x486f, 0x55);
    OS08A10_write_register (ViPipe,0x4875, 0xf0);
    OS08A10_write_register (ViPipe,0x5000, 0x89);
    OS08A10_write_register (ViPipe,0x5001, 0x42);
    OS08A10_write_register (ViPipe,0x5004, 0x40);
    OS08A10_write_register (ViPipe,0x5005, 0x00);
    OS08A10_write_register (ViPipe,0x5180, 0x00);
    OS08A10_write_register (ViPipe,0x5181, 0x10);
    OS08A10_write_register (ViPipe,0x580b, 0x03);
    OS08A10_write_register (ViPipe,0x4700, 0x2b);
    OS08A10_write_register (ViPipe,0x4e00, 0x2b);
    OS08A10_default_reg_init(ViPipe);
    delay_ms(5);  //delay 5ms
    OS08A10_write_register (ViPipe,0x0100, 0x01);

    printf("===OS08A10 4k2k 25fps 10bit LINE Init OK!===\n");
    printf("===MIPI data rate = 736Mbps/lane =====\n");
    return;
}

//@@ Res 3840x 2160_30fps_MIPI 1440Mbps STG_HDR_VC_Mode
//;HTS = 1036, VTS = 2316
void OS08A10_wdr_4k2k30_2to1_init(VI_PIPE ViPipe)
{
#if 1 /* stone */
    OS08A10_write_register (ViPipe, 0x0103, 0x01);
    OS08A10_write_register (ViPipe, 0x0303, 0x01);
    OS08A10_write_register (ViPipe, 0x0305, 0x5c);
    OS08A10_write_register (ViPipe, 0x0306, 0x00);
    OS08A10_write_register (ViPipe, 0x0308, 0x03);
    OS08A10_write_register (ViPipe, 0x0309, 0x04);
    OS08A10_write_register (ViPipe, 0x032a, 0x00);
    OS08A10_write_register (ViPipe, 0x300f, 0x11);
    OS08A10_write_register (ViPipe, 0x3010, 0x01);
    OS08A10_write_register (ViPipe, 0x3012, 0x41);
    OS08A10_write_register (ViPipe, 0x3016, 0xf0);
    OS08A10_write_register (ViPipe, 0x301e, 0x98);
    OS08A10_write_register (ViPipe, 0x3031, 0xa9);
    OS08A10_write_register (ViPipe, 0x3103, 0x92);
    OS08A10_write_register (ViPipe, 0x3104, 0x01);
    OS08A10_write_register (ViPipe, 0x3106, 0x10);
    OS08A10_write_register (ViPipe, 0x340c, 0xff);
    OS08A10_write_register (ViPipe, 0x340d, 0xff);
    OS08A10_write_register (ViPipe, 0x031e, 0x09);
    OS08A10_write_register (ViPipe, 0x3501, 0x08);
    OS08A10_write_register (ViPipe, 0x3502, 0xe5);
    OS08A10_write_register (ViPipe, 0x3505, 0x83);
    OS08A10_write_register (ViPipe, 0x3508, 0x01);
    OS08A10_write_register (ViPipe, 0x3509, 0x00);
    OS08A10_write_register (ViPipe, 0x350a, 0x04);
    OS08A10_write_register (ViPipe, 0x350b, 0x00);
    OS08A10_write_register (ViPipe, 0x350c, 0x00);
    OS08A10_write_register (ViPipe, 0x350d, 0x80);
    OS08A10_write_register (ViPipe, 0x350e, 0x04);
    OS08A10_write_register (ViPipe, 0x350f, 0x00);
    OS08A10_write_register (ViPipe, 0x3600, 0x00);
    OS08A10_write_register (ViPipe, 0x3605, 0x50);
    OS08A10_write_register (ViPipe, 0x3609, 0xb5);
    OS08A10_write_register (ViPipe, 0x3610, 0x69);
    OS08A10_write_register (ViPipe, 0x360c, 0x01);
    OS08A10_write_register (ViPipe, 0x360e, 0x86);
    OS08A10_write_register (ViPipe, 0x3628, 0xa4);
    OS08A10_write_register (ViPipe, 0x362d, 0x10);
    OS08A10_write_register (ViPipe, 0x3660, 0x42);
    OS08A10_write_register (ViPipe, 0x3661, 0x07);
    OS08A10_write_register (ViPipe, 0x3662, 0x00);
    OS08A10_write_register (ViPipe, 0x3663, 0x28);
    OS08A10_write_register (ViPipe, 0x3664, 0x0d);
    OS08A10_write_register (ViPipe, 0x366a, 0x38);
    OS08A10_write_register (ViPipe, 0x366b, 0xa0);
    OS08A10_write_register (ViPipe, 0x366d, 0x00);
    OS08A10_write_register (ViPipe, 0x366e, 0x00);
    OS08A10_write_register (ViPipe, 0x3680, 0x00);
    OS08A10_write_register (ViPipe, 0x3701, 0x02);
    OS08A10_write_register (ViPipe, 0x373b, 0x02);
    OS08A10_write_register (ViPipe, 0x373c, 0x02);
    OS08A10_write_register (ViPipe, 0x3736, 0x02);
    OS08A10_write_register (ViPipe, 0x3737, 0x02);
    OS08A10_write_register (ViPipe, 0x3705, 0x00);
    OS08A10_write_register (ViPipe, 0x3706, 0x35);
    OS08A10_write_register (ViPipe, 0x370a, 0x00);
    OS08A10_write_register (ViPipe, 0x370b, 0x98);
    OS08A10_write_register (ViPipe, 0x3709, 0x49);
    OS08A10_write_register (ViPipe, 0x3714, 0x21);
    OS08A10_write_register (ViPipe, 0x371c, 0x00);
    OS08A10_write_register (ViPipe, 0x371d, 0x08);
    OS08A10_write_register (ViPipe, 0x375e, 0x0b);
    OS08A10_write_register (ViPipe, 0x3776, 0x10);
    OS08A10_write_register (ViPipe, 0x3781, 0x02);
    OS08A10_write_register (ViPipe, 0x3782, 0x04);
    OS08A10_write_register (ViPipe, 0x3783, 0x02);
    OS08A10_write_register (ViPipe, 0x3784, 0x08);
    OS08A10_write_register (ViPipe, 0x3785, 0x08);
    OS08A10_write_register (ViPipe, 0x3788, 0x01);
    OS08A10_write_register (ViPipe, 0x3789, 0x01);
    OS08A10_write_register (ViPipe, 0x3797, 0x04);
    OS08A10_write_register (ViPipe, 0x3800, 0x00);
    OS08A10_write_register (ViPipe, 0x3801, 0x00);
    OS08A10_write_register (ViPipe, 0x3802, 0x00);
    OS08A10_write_register (ViPipe, 0x3803, 0x0c);
    OS08A10_write_register (ViPipe, 0x3804, 0x0e);
    OS08A10_write_register (ViPipe, 0x3805, 0xff);
    OS08A10_write_register (ViPipe, 0x3806, 0x08);
    OS08A10_write_register (ViPipe, 0x3807, 0x6f);
    OS08A10_write_register (ViPipe, 0x3808, 0x0f);
    OS08A10_write_register (ViPipe, 0x3809, 0x00);
    OS08A10_write_register (ViPipe, 0x380a, 0x08);
    OS08A10_write_register (ViPipe, 0x380b, 0x70);
    OS08A10_write_register (ViPipe, 0x380c, 0x04);
    OS08A10_write_register (ViPipe, 0x380d, 0x0c);
    OS08A10_write_register (ViPipe, 0x380e, 0x09);
    OS08A10_write_register (ViPipe, 0x380f, 0x0d);
    OS08A10_write_register (ViPipe, 0x3813, 0x10);
    OS08A10_write_register (ViPipe, 0x3814, 0x01);
    OS08A10_write_register (ViPipe, 0x3815, 0x01);
    OS08A10_write_register (ViPipe, 0x3816, 0x01);
    OS08A10_write_register (ViPipe, 0x3817, 0x01);
    OS08A10_write_register (ViPipe, 0x381c, 0x08);
    OS08A10_write_register (ViPipe, 0x3820, 0x00);
    OS08A10_write_register (ViPipe, 0x3821, 0x24); //mirror
    OS08A10_write_register (ViPipe, 0x3823, 0x08);
    OS08A10_write_register (ViPipe, 0x3826, 0x00);
    OS08A10_write_register (ViPipe, 0x3827, 0x08);
    OS08A10_write_register (ViPipe, 0x382d, 0x08);
    OS08A10_write_register (ViPipe, 0x3832, 0x02);
    OS08A10_write_register (ViPipe, 0x383c, 0x48);
    OS08A10_write_register (ViPipe, 0x383d, 0xff);
    OS08A10_write_register (ViPipe, 0x3d85, 0x0b);
    OS08A10_write_register (ViPipe, 0x3d84, 0x40);
    OS08A10_write_register (ViPipe, 0x3d8c, 0x63);
    OS08A10_write_register (ViPipe, 0x3d8d, 0xd7);
    OS08A10_write_register (ViPipe, 0x4000, 0xf8);
    OS08A10_write_register (ViPipe, 0x4001, 0x2f);
    OS08A10_write_register (ViPipe, 0x400a, 0x01);
    OS08A10_write_register (ViPipe, 0x400f, 0xa1);
    OS08A10_write_register (ViPipe, 0x4010, 0x12);
    OS08A10_write_register (ViPipe, 0x4018, 0x04);
    OS08A10_write_register (ViPipe, 0x4008, 0x02);
    OS08A10_write_register (ViPipe, 0x4009, 0x0d);
    OS08A10_write_register (ViPipe, 0x401a, 0x58);
    OS08A10_write_register (ViPipe, 0x4050, 0x00);
    OS08A10_write_register (ViPipe, 0x4051, 0x01);
    OS08A10_write_register (ViPipe, 0x4028, 0x0f);
    OS08A10_write_register (ViPipe, 0x4052, 0x00);
    OS08A10_write_register (ViPipe, 0x4053, 0x80);
    OS08A10_write_register (ViPipe, 0x4054, 0x00);
    OS08A10_write_register (ViPipe, 0x4055, 0x80);
    OS08A10_write_register (ViPipe, 0x4056, 0x00);
    OS08A10_write_register (ViPipe, 0x4057, 0x80);
    OS08A10_write_register (ViPipe, 0x4058, 0x00);
    OS08A10_write_register (ViPipe, 0x4059, 0x80);
    OS08A10_write_register (ViPipe, 0x430b, 0xff);
    OS08A10_write_register (ViPipe, 0x430c, 0xff);
    OS08A10_write_register (ViPipe, 0x430d, 0x00);
    OS08A10_write_register (ViPipe, 0x430e, 0x00);
    OS08A10_write_register (ViPipe, 0x4501, 0x18);
    OS08A10_write_register (ViPipe, 0x4502, 0x00);
    OS08A10_write_register (ViPipe, 0x4643, 0x00);
    OS08A10_write_register (ViPipe, 0x4640, 0x01);
    OS08A10_write_register (ViPipe, 0x4641, 0x04);
    OS08A10_write_register (ViPipe, 0x4800, 0x04);
    OS08A10_write_register (ViPipe, 0x4809, 0x2b);
    OS08A10_write_register (ViPipe, 0x4813, 0x98);
    OS08A10_write_register (ViPipe, 0x4817, 0x04);
    OS08A10_write_register (ViPipe, 0x4833, 0x18);
    OS08A10_write_register (ViPipe, 0x4837, 0x0a);
    OS08A10_write_register (ViPipe, 0x483b, 0x00);
    OS08A10_write_register (ViPipe, 0x484b, 0x03);
    OS08A10_write_register (ViPipe, 0x4850, 0x7c);
    OS08A10_write_register (ViPipe, 0x4852, 0x06);
    OS08A10_write_register (ViPipe, 0x4856, 0x58);
    OS08A10_write_register (ViPipe, 0x4857, 0xaa);
    OS08A10_write_register (ViPipe, 0x4862, 0x0a);
    OS08A10_write_register (ViPipe, 0x4869, 0x18);
    OS08A10_write_register (ViPipe, 0x486a, 0xaa);
    OS08A10_write_register (ViPipe, 0x486e, 0x07);
    OS08A10_write_register (ViPipe, 0x486f, 0x55);
    OS08A10_write_register (ViPipe, 0x4875, 0xf0);
    OS08A10_write_register (ViPipe, 0x5000, 0x89);
    OS08A10_write_register (ViPipe, 0x5001, 0x42);
    OS08A10_write_register (ViPipe, 0x5004, 0x40);
    OS08A10_write_register (ViPipe, 0x5005, 0x00);
    OS08A10_write_register (ViPipe, 0x5180, 0x00);
    OS08A10_write_register (ViPipe, 0x5181, 0x10);
    OS08A10_write_register (ViPipe, 0x580b, 0x03);
    OS08A10_write_register (ViPipe, 0x4700, 0x2b);
    OS08A10_write_register (ViPipe, 0x4e00, 0x2b);
    OS08A10_default_reg_init(ViPipe);
    delay_ms(5);  //delay 5ms
    OS08A10_write_register (ViPipe, 0x0100, 0x01);
#else
    OS08A10_write_register (ViPipe, 0x0103, 0x01);
    OS08A10_write_register (ViPipe, 0x0303, 0x01);
    OS08A10_write_register (ViPipe, 0x0305, 0x5c);
    OS08A10_write_register (ViPipe, 0x0306, 0x00);
    OS08A10_write_register (ViPipe, 0x0307, 0x00);
    OS08A10_write_register (ViPipe, 0x0308, 0x03);
    OS08A10_write_register (ViPipe, 0x0309, 0x04);
    OS08A10_write_register (ViPipe, 0x032a, 0x00);
    OS08A10_write_register (ViPipe, 0x300f, 0x11);
    OS08A10_write_register (ViPipe, 0x3010, 0x01);
    OS08A10_write_register (ViPipe, 0x3011, 0x04);
    OS08A10_write_register (ViPipe, 0x3012, 0x41);
    OS08A10_write_register (ViPipe, 0x3016, 0xf0);
    OS08A10_write_register (ViPipe, 0x301e, 0x98);
    OS08A10_write_register (ViPipe, 0x3031, 0xa9);
    OS08A10_write_register (ViPipe, 0x3603, 0x2c);
    OS08A10_write_register (ViPipe, 0x3103, 0x92);
    OS08A10_write_register (ViPipe, 0x3104, 0x01);
    OS08A10_write_register (ViPipe, 0x3106, 0x10);
    OS08A10_write_register (ViPipe, 0x3400, 0x04);
    OS08A10_write_register (ViPipe, 0x3025, 0x03);
    OS08A10_write_register (ViPipe, 0x3425, 0x51);
    OS08A10_write_register (ViPipe, 0x3428, 0x01);
    OS08A10_write_register (ViPipe, 0x3406, 0x08);
    OS08A10_write_register (ViPipe, 0x3408, 0x03);
    OS08A10_write_register (ViPipe, 0x340c, 0xff);
    OS08A10_write_register (ViPipe, 0x340d, 0xff);
    OS08A10_write_register (ViPipe, 0x031e, 0x09);
    OS08A10_write_register (ViPipe, 0x3501, 0x08);
    OS08A10_write_register (ViPipe, 0x3502, 0xe5);
    OS08A10_write_register (ViPipe, 0x3505, 0x83);
    OS08A10_write_register (ViPipe, 0x3508, 0x01);
    OS08A10_write_register (ViPipe, 0x3509, 0x00);
    OS08A10_write_register (ViPipe, 0x350a, 0x04);
    OS08A10_write_register (ViPipe, 0x350b, 0x00);
    OS08A10_write_register (ViPipe, 0x350c, 0x00);
    OS08A10_write_register (ViPipe, 0x350d, 0x80);
    OS08A10_write_register (ViPipe, 0x350e, 0x04);
    OS08A10_write_register (ViPipe, 0x350f, 0x00);
    OS08A10_write_register (ViPipe, 0x3600, 0x00);
    OS08A10_write_register (ViPipe, 0x3605, 0x50);
    OS08A10_write_register (ViPipe, 0x3609, 0xb5);
    OS08A10_write_register (ViPipe, 0x3610, 0x69);
    OS08A10_write_register (ViPipe, 0x360c, 0x01);
    OS08A10_write_register (ViPipe, 0x360e, 0x86);
    OS08A10_write_register (ViPipe, 0x3628, 0xa4);
    OS08A10_write_register (ViPipe, 0x362d, 0x10);
    OS08A10_write_register (ViPipe, 0x3660, 0x42);
    OS08A10_write_register (ViPipe, 0x3661, 0x07);
    OS08A10_write_register (ViPipe, 0x3662, 0x00);
    OS08A10_write_register (ViPipe, 0x3663, 0x28);
    OS08A10_write_register (ViPipe, 0x3664, 0x0d);
    OS08A10_write_register (ViPipe, 0x366a, 0x38);
    OS08A10_write_register (ViPipe, 0x366b, 0xa0);
    OS08A10_write_register (ViPipe, 0x366d, 0x00);
    OS08A10_write_register (ViPipe, 0x366e, 0x00);
    OS08A10_write_register (ViPipe, 0x3680, 0x00);
    OS08A10_write_register (ViPipe, 0x3705, 0x00);
    OS08A10_write_register (ViPipe, 0x3706, 0x35);
    OS08A10_write_register (ViPipe, 0x370a, 0x00);
    OS08A10_write_register (ViPipe, 0x370b, 0x98);
    OS08A10_write_register (ViPipe, 0x3709, 0x49);
    OS08A10_write_register (ViPipe, 0x3714, 0x21);
    OS08A10_write_register (ViPipe, 0x371c, 0x00);
    OS08A10_write_register (ViPipe, 0x371d, 0x08);
    OS08A10_write_register (ViPipe, 0x375e, 0x0b);
    OS08A10_write_register (ViPipe, 0x3762, 0x11);
    OS08A10_write_register (ViPipe, 0x3776, 0x10);
    OS08A10_write_register (ViPipe, 0x3781, 0x02);
    OS08A10_write_register (ViPipe, 0x3782, 0x04);
    OS08A10_write_register (ViPipe, 0x3783, 0x02);
    OS08A10_write_register (ViPipe, 0x3784, 0x08);
    OS08A10_write_register (ViPipe, 0x3785, 0x08);
    OS08A10_write_register (ViPipe, 0x3788, 0x01);
    OS08A10_write_register (ViPipe, 0x3789, 0x01);
    OS08A10_write_register (ViPipe, 0x3797, 0x04);
    OS08A10_write_register (ViPipe, 0x3800, 0x00);
    OS08A10_write_register (ViPipe, 0x3801, 0x00);
    OS08A10_write_register (ViPipe, 0x3802, 0x00);
    OS08A10_write_register (ViPipe, 0x3803, 0x0c);
    OS08A10_write_register (ViPipe, 0x3804, 0x0e);
    OS08A10_write_register (ViPipe, 0x3805, 0xff);
    OS08A10_write_register (ViPipe, 0x3806, 0x08);
    OS08A10_write_register (ViPipe, 0x3807, 0x6f);
    OS08A10_write_register (ViPipe, 0x3808, 0x0f);
    OS08A10_write_register (ViPipe, 0x3809, 0x00);
    OS08A10_write_register (ViPipe, 0x380a, 0x08);
    OS08A10_write_register (ViPipe, 0x380b, 0x70);
    OS08A10_write_register (ViPipe, 0x380c, 0x04);
    OS08A10_write_register (ViPipe, 0x380d, 0x0c);
    OS08A10_write_register (ViPipe, 0x380e, 0x09);
    OS08A10_write_register (ViPipe, 0x380f, 0x0d);
    OS08A10_write_register (ViPipe, 0x3813, 0x10);
    OS08A10_write_register (ViPipe, 0x3814, 0x01);
    OS08A10_write_register (ViPipe, 0x3815, 0x01);
    OS08A10_write_register (ViPipe, 0x3816, 0x01);
    OS08A10_write_register (ViPipe, 0x3817, 0x01);
    OS08A10_write_register (ViPipe, 0x381c, 0x08);
    OS08A10_write_register (ViPipe, 0x3820, 0x00);
    OS08A10_write_register (ViPipe, 0x3821, 0x20);
    OS08A10_write_register (ViPipe, 0x3823, 0x08);
    OS08A10_write_register (ViPipe, 0x3826, 0x00);
    OS08A10_write_register (ViPipe, 0x3827, 0x08);
    OS08A10_write_register (ViPipe, 0x382d, 0x08);
    OS08A10_write_register (ViPipe, 0x3832, 0x02);
    OS08A10_write_register (ViPipe, 0x383c, 0x48);
    OS08A10_write_register (ViPipe, 0x383d, 0xff);
    OS08A10_write_register (ViPipe, 0x3d85, 0x0b);
    OS08A10_write_register (ViPipe, 0x3d84, 0x40);
    OS08A10_write_register (ViPipe, 0x3d8c, 0x63);
    OS08A10_write_register (ViPipe, 0x3d8d, 0xd7);
    OS08A10_write_register (ViPipe, 0x4000, 0xf8);
    OS08A10_write_register (ViPipe, 0x4001, 0x2f);
    OS08A10_write_register (ViPipe, 0x400a, 0x01);
    OS08A10_write_register (ViPipe, 0x400f, 0xa1);
    OS08A10_write_register (ViPipe, 0x4010, 0x12);
    OS08A10_write_register (ViPipe, 0x4018, 0x04);
    OS08A10_write_register (ViPipe, 0x4008, 0x02);
    OS08A10_write_register (ViPipe, 0x4009, 0x0d);
    OS08A10_write_register (ViPipe, 0x401a, 0x58);
    OS08A10_write_register (ViPipe, 0x4050, 0x00);
    OS08A10_write_register (ViPipe, 0x4051, 0x01);
    OS08A10_write_register (ViPipe, 0x4028, 0x0f);
    OS08A10_write_register (ViPipe, 0x4052, 0x00);
    OS08A10_write_register (ViPipe, 0x4053, 0x80);
    OS08A10_write_register (ViPipe, 0x4054, 0x00);
    OS08A10_write_register (ViPipe, 0x4055, 0x80);
    OS08A10_write_register (ViPipe, 0x4056, 0x00);
    OS08A10_write_register (ViPipe, 0x4057, 0x80);
    OS08A10_write_register (ViPipe, 0x4058, 0x00);
    OS08A10_write_register (ViPipe, 0x4059, 0x80);
    OS08A10_write_register (ViPipe, 0x430b, 0xff);
    OS08A10_write_register (ViPipe, 0x430c, 0xff);
    OS08A10_write_register (ViPipe, 0x430d, 0x00);
    OS08A10_write_register (ViPipe, 0x430e, 0x00);
    OS08A10_write_register (ViPipe, 0x4501, 0x18);
    OS08A10_write_register (ViPipe, 0x4502, 0x00);
    OS08A10_write_register (ViPipe, 0x4643, 0x00);
    OS08A10_write_register (ViPipe, 0x4640, 0x01);
    OS08A10_write_register (ViPipe, 0x4641, 0x04);
    OS08A10_write_register (ViPipe, 0x4800, 0x04);
    OS08A10_write_register (ViPipe, 0x4809, 0x2b);
    OS08A10_write_register (ViPipe, 0x4813, 0x98);
    OS08A10_write_register (ViPipe, 0x4817, 0x04);
    OS08A10_write_register (ViPipe, 0x4833, 0x18);
    OS08A10_write_register (ViPipe, 0x4837, 0x0a);
    OS08A10_write_register (ViPipe, 0x483b, 0x00);
    OS08A10_write_register (ViPipe, 0x484b, 0x03);
    OS08A10_write_register (ViPipe, 0x4850, 0x7c);
    OS08A10_write_register (ViPipe, 0x4852, 0x06);
    OS08A10_write_register (ViPipe, 0x4856, 0x58);
    OS08A10_write_register (ViPipe, 0x4857, 0xaa);
    OS08A10_write_register (ViPipe, 0x4862, 0x0a);
    OS08A10_write_register (ViPipe, 0x4869, 0x18);
    OS08A10_write_register (ViPipe, 0x486a, 0xaa);
    OS08A10_write_register (ViPipe, 0x486e, 0x07);
    OS08A10_write_register (ViPipe, 0x486f, 0x55);
    OS08A10_write_register (ViPipe, 0x4875, 0xf0);
    OS08A10_write_register (ViPipe, 0x5000, 0x89);
    OS08A10_write_register (ViPipe, 0x5001, 0x42);
    OS08A10_write_register (ViPipe, 0x5004, 0x40);
    OS08A10_write_register (ViPipe, 0x5005, 0x00);
    OS08A10_write_register (ViPipe, 0x5180, 0x00);
    OS08A10_write_register (ViPipe, 0x5181, 0x10);
    OS08A10_write_register (ViPipe, 0x580b, 0x03);
    OS08A10_write_register (ViPipe, 0x4700, 0x2b);
    OS08A10_write_register (ViPipe, 0x4e00, 0x2b);
    OS08A10_default_reg_init(ViPipe);
    delay_ms(5);  //delay 5ms
    OS08A10_write_register (ViPipe, 0x0100, 0x01);
#endif
    printf("===OS08A10 sensor 4k2k30fps 10bit 2to1 WDR(60fps->30fps) init success!=====\n");
    //printf("===MIPI data rate = 1472Mbps/lane =====\n");

    return;
}

void OS08A10_wdr_4k2k25_2to1_init(VI_PIPE ViPipe)
{
    OS08A10_write_register (ViPipe,0x0103, 0x01);
    OS08A10_write_register (ViPipe,0x0303, 0x01);
    OS08A10_write_register (ViPipe,0x0305, 0x4b);
    OS08A10_write_register (ViPipe,0x0306, 0x00);
    OS08A10_write_register (ViPipe,0x0307, 0x00);
    OS08A10_write_register (ViPipe,0x0308, 0x03);
    OS08A10_write_register (ViPipe,0x0309, 0x04);
    OS08A10_write_register (ViPipe,0x032a, 0x00);
    OS08A10_write_register (ViPipe,0x300f, 0x11);
    OS08A10_write_register (ViPipe,0x3010, 0x01);
    OS08A10_write_register (ViPipe,0x3011, 0x04);
    OS08A10_write_register (ViPipe,0x3012, 0x41);
    OS08A10_write_register (ViPipe,0x3016, 0xf0);
    OS08A10_write_register (ViPipe,0x301e, 0x98);
    OS08A10_write_register (ViPipe,0x3031, 0xa9);
    OS08A10_write_register (ViPipe,0x3603, 0x28);
    OS08A10_write_register (ViPipe,0x3103, 0x92);
    OS08A10_write_register (ViPipe,0x3104, 0x01);
    OS08A10_write_register (ViPipe,0x3106, 0x10);
    OS08A10_write_register (ViPipe,0x3400, 0x00);
    OS08A10_write_register (ViPipe,0x3025, 0x02);
    OS08A10_write_register (ViPipe,0x3425, 0x00);
    OS08A10_write_register (ViPipe,0x3428, 0x00);
    OS08A10_write_register (ViPipe,0x3406, 0x08);
    OS08A10_write_register (ViPipe,0x3408, 0x01);
    OS08A10_write_register (ViPipe,0x340c, 0xff);
    OS08A10_write_register (ViPipe,0x340d, 0xff);
    OS08A10_write_register (ViPipe,0x031e, 0x09);
    OS08A10_write_register (ViPipe,0x3501, 0x08);
    OS08A10_write_register (ViPipe,0x3502, 0xe5);
    OS08A10_write_register (ViPipe,0x3505, 0x83);
    OS08A10_write_register (ViPipe,0x3508, 0x01);
    OS08A10_write_register (ViPipe,0x3509, 0x00);
    OS08A10_write_register (ViPipe,0x350a, 0x04);
    OS08A10_write_register (ViPipe,0x350b, 0x00);
    OS08A10_write_register (ViPipe,0x350c, 0x00);
    OS08A10_write_register (ViPipe,0x350d, 0x80);
    OS08A10_write_register (ViPipe,0x350e, 0x04);
    OS08A10_write_register (ViPipe,0x350f, 0x00);
    OS08A10_write_register (ViPipe,0x3600, 0x00);
    OS08A10_write_register (ViPipe,0x3605, 0x50);
    OS08A10_write_register (ViPipe,0x3609, 0xb5);
    OS08A10_write_register (ViPipe,0x3610, 0x69);
    OS08A10_write_register (ViPipe,0x360c, 0x01);
    OS08A10_write_register (ViPipe,0x360e, 0x86);
    OS08A10_write_register (ViPipe,0x3628, 0xa4);
    OS08A10_write_register (ViPipe,0x362d, 0x10);
    OS08A10_write_register (ViPipe,0x3660, 0x42);
    OS08A10_write_register (ViPipe,0x3661, 0x07);
    OS08A10_write_register (ViPipe,0x3662, 0x00);
    OS08A10_write_register (ViPipe,0x3663, 0x28);
    OS08A10_write_register (ViPipe,0x3664, 0x0d);
    OS08A10_write_register (ViPipe,0x366a, 0x38);
    OS08A10_write_register (ViPipe,0x366b, 0xa0);
    OS08A10_write_register (ViPipe,0x366d, 0x00);
    OS08A10_write_register (ViPipe,0x366e, 0x00);
    OS08A10_write_register (ViPipe,0x3680, 0x00);
    OS08A10_write_register (ViPipe,0x3705, 0x00);
    OS08A10_write_register (ViPipe,0x3706, 0x35);
    OS08A10_write_register (ViPipe,0x370a, 0x00);
    OS08A10_write_register (ViPipe,0x370b, 0x98);
    OS08A10_write_register (ViPipe,0x3709, 0x49);
    OS08A10_write_register (ViPipe,0x3714, 0x21);
    OS08A10_write_register (ViPipe,0x371c, 0x00);
    OS08A10_write_register (ViPipe,0x371d, 0x08);
    OS08A10_write_register (ViPipe,0x375e, 0x0b);
    OS08A10_write_register (ViPipe,0x3762, 0x12);
    OS08A10_write_register (ViPipe,0x3776, 0x10);
    OS08A10_write_register (ViPipe,0x3781, 0x02);
    OS08A10_write_register (ViPipe,0x3782, 0x04);
    OS08A10_write_register (ViPipe,0x3783, 0x02);
    OS08A10_write_register (ViPipe,0x3784, 0x08);
    OS08A10_write_register (ViPipe,0x3785, 0x08);
    OS08A10_write_register (ViPipe,0x3788, 0x01);
    OS08A10_write_register (ViPipe,0x3789, 0x01);
    OS08A10_write_register (ViPipe,0x3797, 0x04);
    OS08A10_write_register (ViPipe,0x3800, 0x00);
    OS08A10_write_register (ViPipe,0x3801, 0x00);
    OS08A10_write_register (ViPipe,0x3802, 0x00);
    OS08A10_write_register (ViPipe,0x3803, 0x0c);
    OS08A10_write_register (ViPipe,0x3804, 0x0e);
    OS08A10_write_register (ViPipe,0x3805, 0xff);
    OS08A10_write_register (ViPipe,0x3806, 0x08);
    OS08A10_write_register (ViPipe,0x3807, 0x6f);
    OS08A10_write_register (ViPipe,0x3808, 0x0f);
    OS08A10_write_register (ViPipe,0x3809, 0x00);
    OS08A10_write_register (ViPipe,0x380a, 0x08);
    OS08A10_write_register (ViPipe,0x380b, 0x70);
    OS08A10_write_register (ViPipe,0x380c, 0x04);
    OS08A10_write_register (ViPipe,0x380d, 0xdb);
    OS08A10_write_register (ViPipe,0x380e, 0x09);
    OS08A10_write_register (ViPipe,0x380f, 0x0d);
    OS08A10_write_register (ViPipe,0x3813, 0x10);
    OS08A10_write_register (ViPipe,0x3814, 0x01);
    OS08A10_write_register (ViPipe,0x3815, 0x01);
    OS08A10_write_register (ViPipe,0x3816, 0x01);
    OS08A10_write_register (ViPipe,0x3817, 0x01);
    OS08A10_write_register (ViPipe,0x381c, 0x08);
    //OS08A10_write_register (ViPipe,0x3820, 0x00);
    //OS08A10_write_register (ViPipe,0x3821, 0x00);
    //mirror
    OS08A10_write_register (ViPipe,0x3820, 0x00);
    OS08A10_write_register (ViPipe,0x3821, 0x24);
    OS08A10_write_register (ViPipe,0x3823, 0x08);
    OS08A10_write_register (ViPipe,0x3826, 0x00);
    OS08A10_write_register (ViPipe,0x3827, 0x08);
    OS08A10_write_register (ViPipe,0x382d, 0x08);
    OS08A10_write_register (ViPipe,0x3832, 0x02);
    OS08A10_write_register (ViPipe,0x383c, 0x48);
    OS08A10_write_register (ViPipe,0x383d, 0xff);
    OS08A10_write_register (ViPipe,0x3d85, 0x0b);
    OS08A10_write_register (ViPipe,0x3d84, 0x40);
    OS08A10_write_register (ViPipe,0x3d8c, 0x63);
    OS08A10_write_register (ViPipe,0x3d8d, 0xd7);
    OS08A10_write_register (ViPipe,0x4000, 0xf8);
    OS08A10_write_register (ViPipe,0x4001, 0x2f);
    OS08A10_write_register (ViPipe,0x400a, 0x01);
    OS08A10_write_register (ViPipe,0x400f, 0xa1);
    OS08A10_write_register (ViPipe,0x4010, 0x12);
    OS08A10_write_register (ViPipe,0x4018, 0x04);
    OS08A10_write_register (ViPipe,0x4008, 0x02);
    OS08A10_write_register (ViPipe,0x4009, 0x0d);
    OS08A10_write_register (ViPipe,0x401a, 0x58);
    OS08A10_write_register (ViPipe,0x4050, 0x00);
    OS08A10_write_register (ViPipe,0x4051, 0x01);
    OS08A10_write_register (ViPipe,0x4028, 0x0f);
    OS08A10_write_register (ViPipe,0x4052, 0x00);
    OS08A10_write_register (ViPipe,0x4053, 0x80);
    OS08A10_write_register (ViPipe,0x4054, 0x00);
    OS08A10_write_register (ViPipe,0x4055, 0x80);
    OS08A10_write_register (ViPipe,0x4056, 0x00);
    OS08A10_write_register (ViPipe,0x4057, 0x80);
    OS08A10_write_register (ViPipe,0x4058, 0x00);
    OS08A10_write_register (ViPipe,0x4059, 0x80);
    OS08A10_write_register (ViPipe,0x430b, 0xff);
    OS08A10_write_register (ViPipe,0x430c, 0xff);
    OS08A10_write_register (ViPipe,0x430d, 0x00);
    OS08A10_write_register (ViPipe,0x430e, 0x00);
    OS08A10_write_register (ViPipe,0x4501, 0x18);
    OS08A10_write_register (ViPipe,0x4502, 0x00);
    OS08A10_write_register (ViPipe,0x4643, 0x00);
    OS08A10_write_register (ViPipe,0x4640, 0x01);
    OS08A10_write_register (ViPipe,0x4641, 0x04);
    OS08A10_write_register (ViPipe,0x4800, 0x04);
    OS08A10_write_register (ViPipe,0x4809, 0x2b);
    OS08A10_write_register (ViPipe,0x4813, 0x98);
    OS08A10_write_register (ViPipe,0x4817, 0x04);
    OS08A10_write_register (ViPipe,0x4833, 0x18);
    OS08A10_write_register (ViPipe,0x4837, 0x0d);
    OS08A10_write_register (ViPipe,0x483b, 0x00);
    OS08A10_write_register (ViPipe,0x484b, 0x03);
    OS08A10_write_register (ViPipe,0x4850, 0x7c);
    OS08A10_write_register (ViPipe,0x4852, 0x06);
    OS08A10_write_register (ViPipe,0x4856, 0x58);
    OS08A10_write_register (ViPipe,0x4857, 0xaa);
    OS08A10_write_register (ViPipe,0x4862, 0x0a);
    OS08A10_write_register (ViPipe,0x4869, 0x18);
    OS08A10_write_register (ViPipe,0x486a, 0xaa);
    OS08A10_write_register (ViPipe,0x486e, 0x07);
    OS08A10_write_register (ViPipe,0x486f, 0x55);
    OS08A10_write_register (ViPipe,0x4875, 0xf0);
    OS08A10_write_register (ViPipe,0x5000, 0x89);
    OS08A10_write_register (ViPipe,0x5001, 0x42);
    OS08A10_write_register (ViPipe,0x5004, 0x40);
    OS08A10_write_register (ViPipe,0x5005, 0x00);
    OS08A10_write_register (ViPipe,0x5180, 0x00);
    OS08A10_write_register (ViPipe,0x5181, 0x10);
    OS08A10_write_register (ViPipe,0x580b, 0x03);
    OS08A10_write_register (ViPipe,0x4700, 0x2b);
    OS08A10_write_register (ViPipe,0x4e00, 0x2b);
    OS08A10_default_reg_init(ViPipe);
    delay_ms(5);  //delay 5ms
    OS08A10_write_register (ViPipe,0x0100, 0x01);

    printf("===OS08A10 sensor 4k2k25fps 10bit 2to1 WDR(50fps->25fps) init success!=====\n");
    printf("===MIPI data rate = 1200Mbps/lane =====\n");

    return;

}

#if 0
void OS08A10_wdr_1080p60_2to1_init(VI_PIPE ViPipe)
{
 #if 0
    OS08A10_write_register (ViPipe,0x3000, 0x01); /* standby */
    OS08A10_write_register (ViPipe,0x3002, 0x00); /* XTMSTA */

    OS08A10_write_register (ViPipe,0x3005, 0x01);
    OS08A10_write_register (ViPipe,0x3007, 0x00);
    OS08A10_write_register (ViPipe,0x3009, 0x01);
    OS08A10_write_register (ViPipe,0x300a, 0xf0);
    OS08A10_write_register (ViPipe,0x300c, 0x11);
    OS08A10_write_register (ViPipe,0x300f, 0x00);
    OS08A10_write_register (ViPipe,0x3010, 0x21);
    OS08A10_write_register (ViPipe,0x3012, 0x64);
    OS08A10_write_register (ViPipe,0x3016, 0x09);
    OS08A10_write_register (ViPipe,0x3018, 0x65);
    OS08A10_write_register (ViPipe,0x3019, 0x04);

    OS08A10_write_register (ViPipe,0x301c, 0x98); /* HMAX */
    OS08A10_write_register (ViPipe,0x301d, 0x08); /* HMAX */

    OS08A10_write_register (ViPipe,0x3045, 0x05);
    OS08A10_write_register (ViPipe,0x3046, 0x01);
    OS08A10_write_register (ViPipe,0x304b, 0x0a);

    OS08A10_write_register (ViPipe,0x305c, 0x18);
    OS08A10_write_register (ViPipe,0x305d, 0x03);
    OS08A10_write_register (ViPipe,0x305e, 0x20);
    OS08A10_write_register (ViPipe,0x305f, 0x01);

    OS08A10_write_register (ViPipe,0x3070, 0x02);
    OS08A10_write_register (ViPipe,0x3071, 0x11);

    OS08A10_write_register (ViPipe,0x309b, 0x10);
    OS08A10_write_register (ViPipe,0x309c, 0x22);

    OS08A10_write_register (ViPipe,0x30a2, 0x02);
    OS08A10_write_register (ViPipe,0x30a6, 0x20);
    OS08A10_write_register (ViPipe,0x30a8, 0x20);

    OS08A10_write_register (ViPipe,0x30aa, 0x20);
    OS08A10_write_register (ViPipe,0x30ac, 0x20);
    OS08A10_write_register (ViPipe,0x30b0, 0x43);

    OS08A10_write_register (ViPipe,0x3106, 0x11);
    OS08A10_write_register (ViPipe,0x3119, 0x9e);
    OS08A10_write_register (ViPipe,0x311c, 0x1e);
    OS08A10_write_register (ViPipe,0x311e, 0x08);

    OS08A10_write_register (ViPipe,0x3128, 0x05);
    OS08A10_write_register (ViPipe,0x3129, 0x00);
    OS08A10_write_register (ViPipe,0x313d, 0x83);
    OS08A10_write_register (ViPipe,0x3150, 0x03);

    OS08A10_write_register (ViPipe,0x315e, 0x1a);
    OS08A10_write_register (ViPipe,0x3164, 0x1a);
    OS08A10_write_register (ViPipe,0x317c, 0x00);
    OS08A10_write_register (ViPipe,0x317e, 0x00);
    OS08A10_write_register (ViPipe,0x31ec, 0x00);

    OS08A10_write_register (ViPipe,0x32b8, 0x50);
    OS08A10_write_register (ViPipe,0x32b9, 0x10);
    OS08A10_write_register (ViPipe,0x32ba, 0x00);
    OS08A10_write_register (ViPipe,0x32bb, 0x04);

    OS08A10_write_register (ViPipe,0x32c8, 0x50);
    OS08A10_write_register (ViPipe,0x32c9, 0x10);
    OS08A10_write_register (ViPipe,0x32ca, 0x00);
    OS08A10_write_register (ViPipe,0x32cb, 0x04);

    OS08A10_write_register (ViPipe,0x332c, 0xd3);
    OS08A10_write_register (ViPipe,0x332d, 0x10);
    OS08A10_write_register (ViPipe,0x332e, 0x0d);

    OS08A10_write_register (ViPipe,0x3358, 0x06);
    OS08A10_write_register (ViPipe,0x3359, 0xe1);
    OS08A10_write_register (ViPipe,0x335a, 0x11);

    OS08A10_write_register (ViPipe,0x3360, 0x1e);
    OS08A10_write_register (ViPipe,0x3361, 0x61);
    OS08A10_write_register (ViPipe,0x3362, 0x10);

    OS08A10_write_register (ViPipe,0x33b0, 0x50);
    OS08A10_write_register (ViPipe,0x33b2, 0x1a);
    OS08A10_write_register (ViPipe,0x33b3, 0x04);

    OS08A10_write_register (ViPipe,0x3405, 0x10);
    OS08A10_write_register (ViPipe,0x3407, 0x03);
    OS08A10_write_register (ViPipe,0x3414, 0x0a);
    OS08A10_write_register (ViPipe,0x3415, 0x00);
    OS08A10_write_register (ViPipe,0x3418, 0xb2);
    OS08A10_write_register (ViPipe,0x3419, 0x08);

    OS08A10_write_register (ViPipe,0x3441, 0x0c);
    OS08A10_write_register (ViPipe,0x3442, 0x0c);
    OS08A10_write_register (ViPipe,0x3443, 0x03);
    OS08A10_write_register (ViPipe,0x3444, 0x20);
    OS08A10_write_register (ViPipe,0x3445, 0x25);
    OS08A10_write_register (ViPipe,0x3446, 0x57);
    OS08A10_write_register (ViPipe,0x3447, 0x00);
    OS08A10_write_register (ViPipe,0x3448, 0x37);
    OS08A10_write_register (ViPipe,0x3449, 0x00);
    OS08A10_write_register (ViPipe,0x344a, 0x1f);

    OS08A10_write_register (ViPipe,0x344b, 0x00);
    OS08A10_write_register (ViPipe,0x344c, 0x1f);
    OS08A10_write_register (ViPipe,0x344d, 0x00);
    OS08A10_write_register (ViPipe,0x344e, 0x1f);
    OS08A10_write_register (ViPipe,0x344f, 0x00);
    OS08A10_write_register (ViPipe,0x3450, 0x77);
    OS08A10_write_register (ViPipe,0x3451, 0x00);
    OS08A10_write_register (ViPipe,0x3452, 0x1f);
    OS08A10_write_register (ViPipe,0x3453, 0x00);
    OS08A10_write_register (ViPipe,0x3454, 0x17);
    OS08A10_write_register (ViPipe,0x3455, 0x00);
    OS08A10_write_register (ViPipe,0x3472, 0x9c);
    OS08A10_write_register (ViPipe,0x3473, 0x07);
    OS08A10_write_register (ViPipe,0x3480, 0x49);
    OS08A10_default_reg_init(ViPipe);
    OS08A10_write_register (ViPipe,0x3000, 0x00); /* standby */
    delay_ms(20);
    OS08A10_write_register (ViPipe,0x3002, 0x00); /* master mode start */

    printf("===OS08A10 sensor 1080P30fps 12bit 2to1 WDR(60fps->30fps) init success!=====\n");

    bSensorInit = HI_TRUE;

    return;
 #endif
    OS08A10_write_register (ViPipe,0x3000, 0x01); /* standby */
    OS08A10_write_register (ViPipe,0x3002, 0x01); /* XTMSTA */

    OS08A10_write_register (ViPipe,0x3005, 0x00);
    OS08A10_write_register (ViPipe,0x3007, 0x00);
    OS08A10_write_register (ViPipe,0x3009, 0x01);
    OS08A10_write_register (ViPipe,0x300a, 0x3c);
    OS08A10_write_register (ViPipe,0x300c, 0x11);
    OS08A10_write_register (ViPipe,0x300f, 0x00);
    OS08A10_write_register (ViPipe,0x3010, 0x21);
    OS08A10_write_register (ViPipe,0x3012, 0x64);
    OS08A10_write_register (ViPipe,0x3016, 0x09);
    OS08A10_write_register (ViPipe,0x3018, 0xC4); /* VMAX */
    OS08A10_write_register (ViPipe,0x3019, 0x04); /* VMAX */
    OS08A10_write_register (ViPipe,0x301c, 0xEC); /* HMAX */
    OS08A10_write_register (ViPipe,0x301d, 0x07); /* HMAX */

#if 1
    OS08A10_write_register (ViPipe,0x3020, 0x03);  /* SHS1 */
    OS08A10_write_register (ViPipe,0x3021, 0x00);
    //OS08A10_write_register (ViPipe,0x3022, 0x00);
    OS08A10_write_register (ViPipe,0x3024, 0x99);  /* SHS2 */
    OS08A10_write_register (ViPipe,0x3025, 0x00);
    //OS08A10_write_register (ViPipe,0x3025, 0x00);
    OS08A10_write_register (ViPipe,0x3028, 0x00);  /* SHS3 */
    OS08A10_write_register (ViPipe,0x3029, 0x00);
    //OS08A10_write_register (ViPipe,0x302A, 0x00);
    OS08A10_write_register (ViPipe,0x3030, 0x93);  /* RHS1 */
    OS08A10_write_register (ViPipe,0x3031, 0x00);
    OS08A10_write_register (ViPipe,0x3034, 0x00);  /* RHS2 */
    OS08A10_write_register (ViPipe,0x3035, 0x00);
#endif

    OS08A10_write_register (ViPipe,0x3045, 0x05);
    OS08A10_write_register (ViPipe,0x3046, 0x00);
    OS08A10_write_register (ViPipe,0x304b, 0x0a);
    OS08A10_write_register (ViPipe,0x305c, 0x18);
    OS08A10_write_register (ViPipe,0x305d, 0x03);
    OS08A10_write_register (ViPipe,0x305e, 0x20);
    OS08A10_write_register (ViPipe,0x305f, 0x01);
    OS08A10_write_register (ViPipe,0x3070, 0x02);
    OS08A10_write_register (ViPipe,0x3071, 0x11);
    OS08A10_write_register (ViPipe,0x309b, 0x10);
    OS08A10_write_register (ViPipe,0x309c, 0x22);
    OS08A10_write_register (ViPipe,0x30a2, 0x02);
    OS08A10_write_register (ViPipe,0x30a6, 0x20);
    OS08A10_write_register (ViPipe,0x30a8, 0x20);
    OS08A10_write_register (ViPipe,0x30aa, 0x20);
    OS08A10_write_register (ViPipe,0x30ac, 0x20);
    OS08A10_write_register (ViPipe,0x30b0, 0x43);

    OS08A10_write_register (ViPipe,0x3106, 0x11);
    OS08A10_write_register (ViPipe,0x3119, 0x9e);
    OS08A10_write_register (ViPipe,0x311c, 0x1e);
    OS08A10_write_register (ViPipe,0x311e, 0x08);
    OS08A10_write_register (ViPipe,0x3128, 0x05);
    OS08A10_write_register (ViPipe,0x3129, 0x1d);
    OS08A10_write_register (ViPipe,0x313d, 0x83);
    OS08A10_write_register (ViPipe,0x3150, 0x03);
    OS08A10_write_register (ViPipe,0x315e, 0x1a);
    OS08A10_write_register (ViPipe,0x3164, 0x1a);
    OS08A10_write_register (ViPipe,0x317c, 0x12);
    OS08A10_write_register (ViPipe,0x317e, 0x00);
    OS08A10_write_register (ViPipe,0x31ec, 0x37);

    OS08A10_write_register (ViPipe,0x32b8, 0x50);
    OS08A10_write_register (ViPipe,0x32b9, 0x10);
    OS08A10_write_register (ViPipe,0x32ba, 0x00);
    OS08A10_write_register (ViPipe,0x32bb, 0x04);
    OS08A10_write_register (ViPipe,0x32c8, 0x50);
    OS08A10_write_register (ViPipe,0x32c9, 0x10);
    OS08A10_write_register (ViPipe,0x32ca, 0x00);
    OS08A10_write_register (ViPipe,0x32cb, 0x04);

    OS08A10_write_register (ViPipe,0x332c, 0xd3);
    OS08A10_write_register (ViPipe,0x332d, 0x10);
    OS08A10_write_register (ViPipe,0x332e, 0x0d);
    OS08A10_write_register (ViPipe,0x3358, 0x06);
    OS08A10_write_register (ViPipe,0x3359, 0xe1);
    OS08A10_write_register (ViPipe,0x335a, 0x11);
    OS08A10_write_register (ViPipe,0x3360, 0x1e);
    OS08A10_write_register (ViPipe,0x3361, 0x61);
    OS08A10_write_register (ViPipe,0x3362, 0x10);
    OS08A10_write_register (ViPipe,0x33b0, 0x50);
    OS08A10_write_register (ViPipe,0x33b2, 0x1a);
    OS08A10_write_register (ViPipe,0x33b3, 0x04);

    OS08A10_write_register (ViPipe,0x3405, 0x10);
    OS08A10_write_register (ViPipe,0x3407, 0x03);
    OS08A10_write_register (ViPipe,0x3414, 0x0a);
    OS08A10_write_register (ViPipe,0x3415, 0x00);
    OS08A10_write_register (ViPipe,0x3418, 0x32);  /* Y_OUT_SIZE */
    OS08A10_write_register (ViPipe,0x3419, 0x09);  /* Y_OUT_SIZE */
    OS08A10_write_register (ViPipe,0x3441, 0x0a);
    OS08A10_write_register (ViPipe,0x3442, 0x0a);
    OS08A10_write_register (ViPipe,0x3443, 0x03);
    OS08A10_write_register (ViPipe,0x3444, 0x20);
    OS08A10_write_register (ViPipe,0x3445, 0x25);
    OS08A10_write_register (ViPipe,0x3446, 0x57);
    OS08A10_write_register (ViPipe,0x3447, 0x00);
    OS08A10_write_register (ViPipe,0x3448, 0x37);
    OS08A10_write_register (ViPipe,0x3449, 0x00);
    OS08A10_write_register (ViPipe,0x344a, 0x1f);
    OS08A10_write_register (ViPipe,0x344b, 0x00);
    OS08A10_write_register (ViPipe,0x344c, 0x1f);
    OS08A10_write_register (ViPipe,0x344d, 0x00);
    OS08A10_write_register (ViPipe,0x344e, 0x1f);
    OS08A10_write_register (ViPipe,0x344f, 0x00);
    OS08A10_write_register (ViPipe,0x3450, 0x77);
    OS08A10_write_register (ViPipe,0x3451, 0x00);
    OS08A10_write_register (ViPipe,0x3452, 0x1f);
    OS08A10_write_register (ViPipe,0x3453, 0x00);
    OS08A10_write_register (ViPipe,0x3454, 0x17);
    OS08A10_write_register (ViPipe,0x3455, 0x00);
    OS08A10_write_register (ViPipe,0x3472, 0x9c);
    OS08A10_write_register (ViPipe,0x3473, 0x07);
    OS08A10_write_register (ViPipe,0x3480, 0x49);
    OS08A10_default_reg_init(ViPipe);

    OS08A10_write_register (ViPipe,0x3000, 0x00); /* standby */
    delay_ms(20);
    OS08A10_write_register (ViPipe,0x3002, 0x00); /* master mode start */

    printf("===OS08A10 sensor 1080P30fps 10bit 2to1 WDR(60fps->30fps) init success!=====\n");

    return;
}
#endif

#if 0
void OS08A10_wdr_1080p30_3to1_init(VI_PIPE ViPipe)
{
    OS08A10_write_register (ViPipe,0x3000, 0x01); /* standby */
    OS08A10_write_register (ViPipe,0x3002, 0x01); /* XTMSTA */

    //12bit
    OS08A10_write_register (ViPipe,0x3007, 0x00);
    OS08A10_write_register (ViPipe,0x300c, 0x21);
    OS08A10_write_register (ViPipe,0x300f, 0x00);
    OS08A10_write_register (ViPipe,0x3010, 0x21);
    OS08A10_write_register (ViPipe,0x3012, 0x64);
    OS08A10_write_register (ViPipe,0x3016, 0x09);
    OS08A10_write_register (ViPipe,0x3020, 0x04);
    OS08A10_write_register (ViPipe,0x3021, 0x00);
    OS08A10_write_register (ViPipe,0x3024, 0xF2);
    OS08A10_write_register (ViPipe,0x3025, 0x01);
    OS08A10_write_register (ViPipe,0x3028, 0x57);
    OS08A10_write_register (ViPipe,0x3029, 0x02);
    OS08A10_write_register (ViPipe,0x3030, 0xED);
    OS08A10_write_register (ViPipe,0x3031, 0x01);
    OS08A10_write_register (ViPipe,0x3034, 0x30);
    OS08A10_write_register (ViPipe,0x3035, 0x02);
    OS08A10_write_register (ViPipe,0x3045, 0x05);
    OS08A10_write_register (ViPipe,0x304b, 0x0a);
    OS08A10_write_register (ViPipe,0x305c, 0x18);
    OS08A10_write_register (ViPipe,0x305d, 0x03);
    OS08A10_write_register (ViPipe,0x305e, 0x20);
    OS08A10_write_register (ViPipe,0x305f, 0x01);
    OS08A10_write_register (ViPipe,0x3070, 0x02);
    OS08A10_write_register (ViPipe,0x3071, 0x11);
    OS08A10_write_register (ViPipe,0x309b, 0x10);
    OS08A10_write_register (ViPipe,0x309c, 0x22);
    OS08A10_write_register (ViPipe,0x30a2, 0x02);
    OS08A10_write_register (ViPipe,0x30a6, 0x20);
    OS08A10_write_register (ViPipe,0x30a8, 0x20);
    OS08A10_write_register (ViPipe,0x30aa, 0x20);
    OS08A10_write_register (ViPipe,0x30ac, 0x20);
    OS08A10_write_register (ViPipe,0x30b0, 0x43);
    OS08A10_write_register (ViPipe,0x3106, 0x33);
    OS08A10_write_register (ViPipe,0x3119, 0x9e);
    OS08A10_write_register (ViPipe,0x311c, 0x1e);
    OS08A10_write_register (ViPipe,0x311e, 0x08);
    OS08A10_write_register (ViPipe,0x3128, 0x05);
    OS08A10_write_register (ViPipe,0x313d, 0x83);
    OS08A10_write_register (ViPipe,0x3150, 0x03);
    OS08A10_write_register (ViPipe,0x315e, 0x1a);
    OS08A10_write_register (ViPipe,0x3164, 0x1a);
    OS08A10_write_register (ViPipe,0x317e, 0x00);
    OS08A10_write_register (ViPipe,0x32b8, 0x50);
    OS08A10_write_register (ViPipe,0x32b9, 0x10);
    OS08A10_write_register (ViPipe,0x32ba, 0x00);
    OS08A10_write_register (ViPipe,0x32bb, 0x04);
    OS08A10_write_register (ViPipe,0x32c8, 0x50);
    OS08A10_write_register (ViPipe,0x32c9, 0x10);
    OS08A10_write_register (ViPipe,0x32ca, 0x00);
    OS08A10_write_register (ViPipe,0x32cb, 0x04);
    OS08A10_write_register (ViPipe,0x332c, 0xd3);
    OS08A10_write_register (ViPipe,0x332d, 0x10);
    OS08A10_write_register (ViPipe,0x332e, 0x0d);
    OS08A10_write_register (ViPipe,0x3358, 0x06);
    OS08A10_write_register (ViPipe,0x3359, 0xe1);
    OS08A10_write_register (ViPipe,0x335a, 0x11);
    OS08A10_write_register (ViPipe,0x3360, 0x1e);
    OS08A10_write_register (ViPipe,0x3361, 0x61);
    OS08A10_write_register (ViPipe,0x3362, 0x10);
    OS08A10_write_register (ViPipe,0x33b0, 0x50);
    OS08A10_write_register (ViPipe,0x33b2, 0x1a);
    OS08A10_write_register (ViPipe,0x33b3, 0x04);

    OS08A10_write_register (ViPipe,0x3418, 0x24); /**** Y_OUT_SIZE *****/
    OS08A10_write_register (ViPipe,0x3419, 0x0F);

    OS08A10_write_register (ViPipe,0x3444, 0x20);
    OS08A10_write_register (ViPipe,0x3445, 0x25);
    OS08A10_write_register (ViPipe,0x3480, 0x49);
    OS08A10_default_reg_init(ViPipe);
    OS08A10_write_register (ViPipe,0x3000, 0x00); /* standby */
    delay_ms(20);
    OS08A10_write_register (ViPipe,0x3002, 0x00); /* master mode start */

    printf("===OS08A10 OS08A10 1080P15fps 12bit 3to1 WDR(30fps->7p5fps) init success!=====\n");

    return;
}

#endif

#if 0
void OS08A10_wdr_720p60_2to1_init(VI_PIPE ViPipe)
{
    OS08A10_write_register (ViPipe,0x3000, 0x01); /* standby */
    OS08A10_write_register (ViPipe,0x3002, 0x01); /* XTMSTA */

    //12bit
    OS08A10_write_register (ViPipe,0x3005, 0x01);
    OS08A10_write_register (ViPipe,0x3007, 0x10);
    OS08A10_write_register (ViPipe,0x3009, 0x01);
    OS08A10_write_register (ViPipe,0x300a, 0xf0);
    OS08A10_write_register (ViPipe,0x300c, 0x11);
    OS08A10_write_register (ViPipe,0x300f, 0x00);
    OS08A10_write_register (ViPipe,0x3010, 0x21);
    OS08A10_write_register (ViPipe,0x3012, 0x64);
    OS08A10_write_register (ViPipe,0x3016, 0x09);
    OS08A10_write_register (ViPipe,0x3018, 0xee);
    OS08A10_write_register (ViPipe,0x3019, 0x02);
    OS08A10_write_register (ViPipe,0x301c, 0xe4);
    OS08A10_write_register (ViPipe,0x301d, 0x0c);
    OS08A10_write_register (ViPipe,0x3045, 0x05);
    OS08A10_write_register (ViPipe,0x3046, 0x01);
    OS08A10_write_register (ViPipe,0x304b, 0x0a);

    OS08A10_write_register (ViPipe,0x305c, 0x20); //INCKSEL1
    OS08A10_write_register (ViPipe,0x305d, 0x03); //INCKSEL2
    OS08A10_write_register (ViPipe,0x305e, 0x20); //INCKSEL3
    OS08A10_write_register (ViPipe,0x305f, 0x01); //INCKSEL4

    OS08A10_write_register (ViPipe,0x3070, 0x02);
    OS08A10_write_register (ViPipe,0x3071, 0x11);
    OS08A10_write_register (ViPipe,0x309b, 0x10);
    OS08A10_write_register (ViPipe,0x309c, 0x22);
    OS08A10_write_register (ViPipe,0x30a2, 0x02);
    OS08A10_write_register (ViPipe,0x30a6, 0x20);
    OS08A10_write_register (ViPipe,0x30a8, 0x20);
    OS08A10_write_register (ViPipe,0x30aa, 0x20);
    OS08A10_write_register (ViPipe,0x30ac, 0x20);
    OS08A10_write_register (ViPipe,0x30b0, 0x43);

    //Add
    OS08A10_write_register (ViPipe,0x3106, 0x11);

    OS08A10_write_register (ViPipe,0x3119, 0x9e);
    OS08A10_write_register (ViPipe,0x311c, 0x1e);
    OS08A10_write_register (ViPipe,0x311e, 0x08);
    OS08A10_write_register (ViPipe,0x3128, 0x05);
    OS08A10_write_register (ViPipe,0x3129, 0x00);
    OS08A10_write_register (ViPipe,0x313d, 0x83);
    OS08A10_write_register (ViPipe,0x3150, 0x03);
    OS08A10_write_register (ViPipe,0x315e, 0x1a);
    OS08A10_write_register (ViPipe,0x3164, 0x1a);
    OS08A10_write_register (ViPipe,0x317c, 0x00);
    OS08A10_write_register (ViPipe,0x317e, 0x00);
    OS08A10_write_register (ViPipe,0x31ec, 0x00);

    OS08A10_write_register (ViPipe,0x32b8, 0x50);
    OS08A10_write_register (ViPipe,0x32b9, 0x10);
    OS08A10_write_register (ViPipe,0x32ba, 0x00);
    OS08A10_write_register (ViPipe,0x32bb, 0x04);
    OS08A10_write_register (ViPipe,0x32c8, 0x50);
    OS08A10_write_register (ViPipe,0x32c9, 0x10);
    OS08A10_write_register (ViPipe,0x32ca, 0x00);
    OS08A10_write_register (ViPipe,0x32cb, 0x04);

    OS08A10_write_register (ViPipe,0x332c, 0xd3);
    OS08A10_write_register (ViPipe,0x332d, 0x10);
    OS08A10_write_register (ViPipe,0x332e, 0x0d);
    OS08A10_write_register (ViPipe,0x3358, 0x06);
    OS08A10_write_register (ViPipe,0x3359, 0xe1);
    OS08A10_write_register (ViPipe,0x335a, 0x11);
    OS08A10_write_register (ViPipe,0x3360, 0x1e);
    OS08A10_write_register (ViPipe,0x3361, 0x61);
    OS08A10_write_register (ViPipe,0x3362, 0x10);
    OS08A10_write_register (ViPipe,0x33b0, 0x50);
    OS08A10_write_register (ViPipe,0x33b2, 0x1a);
    OS08A10_write_register (ViPipe,0x33b3, 0x04);

    OS08A10_write_register (ViPipe,0x3405, 0x10);
    OS08A10_write_register (ViPipe,0x3407, 0x03);
    OS08A10_write_register (ViPipe,0x3414, 0x04);
    OS08A10_write_register (ViPipe,0x3418, 0xc6);
    OS08A10_write_register (ViPipe,0x3419, 0x05);
    OS08A10_write_register (ViPipe,0x3441, 0x0c);
    OS08A10_write_register (ViPipe,0x3442, 0x0c);
    OS08A10_write_register (ViPipe,0x3443, 0x03);
    OS08A10_write_register (ViPipe,0x3444, 0x20);
    OS08A10_write_register (ViPipe,0x3445, 0x25);
    OS08A10_write_register (ViPipe,0x3446, 0x4f);
    OS08A10_write_register (ViPipe,0x3447, 0x00);
    OS08A10_write_register (ViPipe,0x3448, 0x2f);
    OS08A10_write_register (ViPipe,0x3449, 0x00);
    OS08A10_write_register (ViPipe,0x344a, 0x17);
    OS08A10_write_register (ViPipe,0x344b, 0x00);
    OS08A10_write_register (ViPipe,0x344c, 0x17);
    OS08A10_write_register (ViPipe,0x344d, 0x00);
    OS08A10_write_register (ViPipe,0x344e, 0x17);
    OS08A10_write_register (ViPipe,0x344f, 0x00);
    OS08A10_write_register (ViPipe,0x3450, 0x57);
    OS08A10_write_register (ViPipe,0x3451, 0x00);
    OS08A10_write_register (ViPipe,0x3452, 0x17);
    OS08A10_write_register (ViPipe,0x3453, 0x00);
    OS08A10_write_register (ViPipe,0x3454, 0x17);
    OS08A10_write_register (ViPipe,0x3455, 0x00);
    OS08A10_write_register (ViPipe,0x3472, 0x1c);
    OS08A10_write_register (ViPipe,0x3473, 0x05);
    OS08A10_write_register (ViPipe,0x3480, 0x49);
    OS08A10_default_reg_init(ViPipe);
    OS08A10_write_register (ViPipe,0x3000, 0x00); /* standby */
    delay_ms(20);
    OS08A10_write_register (ViPipe,0x3002, 0x00); /* master mode start */

    printf("===OS08A10 OS08A10 720P30fps 12bit 2to1 WDR(60fps->30fps) init success!=====\n");

    return;
}

#endif

#if 0
void OS08A10_wdr_720p60_3to1_init(VI_PIPE ViPipe)
{
    OS08A10_write_register (ViPipe,0x3000, 0x01); /* standby */
    OS08A10_write_register (ViPipe,0x3002, 0x01); /* XTMSTA */

    //12bit
    OS08A10_write_register (ViPipe,0x3005, 0x01);
    OS08A10_write_register (ViPipe,0x3007, 0x10);
    OS08A10_write_register (ViPipe,0x3009, 0x01);
    OS08A10_write_register (ViPipe,0x300a, 0xf0);
    OS08A10_write_register (ViPipe,0x300c, 0x31);
    OS08A10_write_register (ViPipe,0x300f, 0x00);
    OS08A10_write_register (ViPipe,0x3010, 0x21);
    OS08A10_write_register (ViPipe,0x3012, 0x64);
    OS08A10_write_register (ViPipe,0x3016, 0x09);
    OS08A10_write_register (ViPipe,0x3018, 0xee);
    OS08A10_write_register (ViPipe,0x3019, 0x02);
    OS08A10_write_register (ViPipe,0x301c, 0xe4);
    OS08A10_write_register (ViPipe,0x301d, 0x0c);
    OS08A10_write_register (ViPipe,0x3045, 0x05);
    OS08A10_write_register (ViPipe,0x3046, 0x01);
    OS08A10_write_register (ViPipe,0x304b, 0x0a);

    OS08A10_write_register (ViPipe,0x305c, 0x20); //INCKSEL1
    OS08A10_write_register (ViPipe,0x305d, 0x03); //INCKSEL2
    OS08A10_write_register (ViPipe,0x305e, 0x20); //INCKSEL3
    OS08A10_write_register (ViPipe,0x305f, 0x01); //INCKSEL4

    OS08A10_write_register (ViPipe,0x3070, 0x02);
    OS08A10_write_register (ViPipe,0x3071, 0x11);
    OS08A10_write_register (ViPipe,0x309b, 0x10);
    OS08A10_write_register (ViPipe,0x309c, 0x22);
    OS08A10_write_register (ViPipe,0x30a2, 0x02);
    OS08A10_write_register (ViPipe,0x30a6, 0x20);
    OS08A10_write_register (ViPipe,0x30a8, 0x20);
    OS08A10_write_register (ViPipe,0x30aa, 0x20);
    OS08A10_write_register (ViPipe,0x30ac, 0x20);
    OS08A10_write_register (ViPipe,0x30b0, 0x43);

    //Add
    OS08A10_write_register (ViPipe,0x3106, 0x33);

    OS08A10_write_register (ViPipe,0x3119, 0x9e);
    OS08A10_write_register (ViPipe,0x311c, 0x1e);
    OS08A10_write_register (ViPipe,0x311e, 0x08);
    OS08A10_write_register (ViPipe,0x3128, 0x05);
    OS08A10_write_register (ViPipe,0x3129, 0x00);
    OS08A10_write_register (ViPipe,0x313d, 0x83);
    OS08A10_write_register (ViPipe,0x3150, 0x03);
    OS08A10_write_register (ViPipe,0x315e, 0x1a);
    OS08A10_write_register (ViPipe,0x3164, 0x1a);
    OS08A10_write_register (ViPipe,0x317c, 0x00);
    OS08A10_write_register (ViPipe,0x317e, 0x00);
    OS08A10_write_register (ViPipe,0x31ec, 0x00);

    OS08A10_write_register (ViPipe,0x32b8, 0x50);
    OS08A10_write_register (ViPipe,0x32b9, 0x10);
    OS08A10_write_register (ViPipe,0x32ba, 0x00);
    OS08A10_write_register (ViPipe,0x32bb, 0x04);
    OS08A10_write_register (ViPipe,0x32c8, 0x50);
    OS08A10_write_register (ViPipe,0x32c9, 0x10);
    OS08A10_write_register (ViPipe,0x32ca, 0x00);
    OS08A10_write_register (ViPipe,0x32cb, 0x04);

    OS08A10_write_register (ViPipe,0x332c, 0xd3);
    OS08A10_write_register (ViPipe,0x332d, 0x10);
    OS08A10_write_register (ViPipe,0x332e, 0x0d);
    OS08A10_write_register (ViPipe,0x3358, 0x06);
    OS08A10_write_register (ViPipe,0x3359, 0xe1);
    OS08A10_write_register (ViPipe,0x335a, 0x11);
    OS08A10_write_register (ViPipe,0x3360, 0x1e);
    OS08A10_write_register (ViPipe,0x3361, 0x61);
    OS08A10_write_register (ViPipe,0x3362, 0x10);
    OS08A10_write_register (ViPipe,0x33b0, 0x50);
    OS08A10_write_register (ViPipe,0x33b2, 0x1a);
    OS08A10_write_register (ViPipe,0x33b3, 0x04);

    OS08A10_write_register (ViPipe,0x3405, 0x10);
    OS08A10_write_register (ViPipe,0x3407, 0x03);
    OS08A10_write_register (ViPipe,0x3414, 0x04);
    OS08A10_write_register (ViPipe,0x3418, 0xb5);
    OS08A10_write_register (ViPipe,0x3419, 0x08);
    OS08A10_write_register (ViPipe,0x3441, 0x0c);
    OS08A10_write_register (ViPipe,0x3442, 0x0c);
    OS08A10_write_register (ViPipe,0x3443, 0x03);
    OS08A10_write_register (ViPipe,0x3444, 0x20);
    OS08A10_write_register (ViPipe,0x3445, 0x25);
    OS08A10_write_register (ViPipe,0x3446, 0x4f);
    OS08A10_write_register (ViPipe,0x3447, 0x00);
    OS08A10_write_register (ViPipe,0x3448, 0x2f);
    OS08A10_write_register (ViPipe,0x3449, 0x00);
    OS08A10_write_register (ViPipe,0x344a, 0x17);
    OS08A10_write_register (ViPipe,0x344b, 0x00);
    OS08A10_write_register (ViPipe,0x344c, 0x17);
    OS08A10_write_register (ViPipe,0x344d, 0x00);
    OS08A10_write_register (ViPipe,0x344e, 0x17);
    OS08A10_write_register (ViPipe,0x344f, 0x00);
    OS08A10_write_register (ViPipe,0x3450, 0x57);
    OS08A10_write_register (ViPipe,0x3451, 0x00);
    OS08A10_write_register (ViPipe,0x3452, 0x17);
    OS08A10_write_register (ViPipe,0x3453, 0x00);
    OS08A10_write_register (ViPipe,0x3454, 0x17);
    OS08A10_write_register (ViPipe,0x3455, 0x00);
    OS08A10_write_register (ViPipe,0x3472, 0x1c);
    OS08A10_write_register (ViPipe,0x3473, 0x05);
    OS08A10_write_register (ViPipe,0x3480, 0x49);
    OS08A10_default_reg_init(ViPipe);
    OS08A10_write_register (ViPipe,0x3000, 0x00); /* standby */
    delay_ms(20);
    OS08A10_write_register (ViPipe,0x3002, 0x00); /* master mode start */

    printf("===OS08A10 OS08A10 720P15fps 12bit 3to1 WDR(60fps->15fps) init success!=====\n");
    return;
}
#endif

#if 0
void OS08A10_wdr_1080p120_2to1_init(VI_PIPE ViPipe)
{
    OS08A10_write_register (ViPipe,0x3000, 0x01); /* standby */
    OS08A10_write_register (ViPipe,0x3002, 0x01); /* XTMSTA */

    OS08A10_write_register (ViPipe,0x3005, 0x00);
    OS08A10_write_register (ViPipe,0x3007, 0x00);
    OS08A10_write_register (ViPipe,0x3009, 0x00);
    OS08A10_write_register (ViPipe,0x300a, 0x3c);
    OS08A10_write_register (ViPipe,0x300c, 0x11);
    OS08A10_write_register (ViPipe,0x300f, 0x00);
    OS08A10_write_register (ViPipe,0x3010, 0x21);
    OS08A10_write_register (ViPipe,0x3012, 0x64);
    OS08A10_write_register (ViPipe,0x3016, 0x09);
    OS08A10_write_register (ViPipe,0x3018, 0x65);
    OS08A10_write_register (ViPipe,0x3019, 0x04);
    OS08A10_write_register (ViPipe,0x301c, 0xF6);
    OS08A10_write_register (ViPipe,0x301d, 0x03);
    OS08A10_write_register (ViPipe,0x3020, 0x02);
    OS08A10_write_register (ViPipe,0x3024, 0xc9);
    OS08A10_write_register (ViPipe,0x3030, 0x0b);
    OS08A10_write_register (ViPipe,0x3045, 0x05);
    OS08A10_write_register (ViPipe,0x3046, 0x00);
    OS08A10_write_register (ViPipe,0x304b, 0x0a);
    OS08A10_write_register (ViPipe,0x305c, 0x18);
    OS08A10_write_register (ViPipe,0x305d, 0x03);
    OS08A10_write_register (ViPipe,0x305e, 0x20);
    OS08A10_write_register (ViPipe,0x305f, 0x01);
    OS08A10_write_register (ViPipe,0x3070, 0x02);
    OS08A10_write_register (ViPipe,0x3071, 0x11);
    OS08A10_write_register (ViPipe,0x309b, 0x10);
    OS08A10_write_register (ViPipe,0x309c, 0x22);
    OS08A10_write_register (ViPipe,0x30a2, 0x02);
    OS08A10_write_register (ViPipe,0x30a6, 0x20);
    OS08A10_write_register (ViPipe,0x30a8, 0x20);
    OS08A10_write_register (ViPipe,0x30aa, 0x20);
    OS08A10_write_register (ViPipe,0x30ac, 0x20);
    OS08A10_write_register (ViPipe,0x30b0, 0x43);
    OS08A10_write_register (ViPipe,0x3106, 0x11);
    OS08A10_write_register (ViPipe,0x3119, 0x9e);
    OS08A10_write_register (ViPipe,0x311c, 0x1e);
    OS08A10_write_register (ViPipe,0x311e, 0x08);
    OS08A10_write_register (ViPipe,0x3128, 0x05);
    OS08A10_write_register (ViPipe,0x3129, 0x1d);
    OS08A10_write_register (ViPipe,0x313d, 0x83);
    OS08A10_write_register (ViPipe,0x3150, 0x03);
    OS08A10_write_register (ViPipe,0x315e, 0x1a);
    OS08A10_write_register (ViPipe,0x3164, 0x1a);
    OS08A10_write_register (ViPipe,0x317c, 0x12);
    OS08A10_write_register (ViPipe,0x317e, 0x00);
    OS08A10_write_register (ViPipe,0x31ec, 0x37);

    OS08A10_write_register (ViPipe,0x32b8, 0x50);
    OS08A10_write_register (ViPipe,0x32b9, 0x10);
    OS08A10_write_register (ViPipe,0x32ba, 0x00);
    OS08A10_write_register (ViPipe,0x32bb, 0x04);
    OS08A10_write_register (ViPipe,0x32c8, 0x50);
    OS08A10_write_register (ViPipe,0x32c9, 0x10);
    OS08A10_write_register (ViPipe,0x32ca, 0x00);
    OS08A10_write_register (ViPipe,0x32cb, 0x04);

    OS08A10_write_register (ViPipe,0x332c, 0xd3);
    OS08A10_write_register (ViPipe,0x332d, 0x10);
    OS08A10_write_register (ViPipe,0x332e, 0x0d);
    OS08A10_write_register (ViPipe,0x3358, 0x06);
    OS08A10_write_register (ViPipe,0x3359, 0xe1);
    OS08A10_write_register (ViPipe,0x335a, 0x11);
    OS08A10_write_register (ViPipe,0x3360, 0x1e);
    OS08A10_write_register (ViPipe,0x3361, 0x61);
    OS08A10_write_register (ViPipe,0x3362, 0x10);
    OS08A10_write_register (ViPipe,0x33b0, 0x50);
    OS08A10_write_register (ViPipe,0x33b2, 0x1a);
    OS08A10_write_register (ViPipe,0x33b3, 0x04);

    OS08A10_write_register (ViPipe,0x3405, 0x00);
    OS08A10_write_register (ViPipe,0x3407, 0x03);
    OS08A10_write_register (ViPipe,0x3414, 0x0a);
    OS08A10_write_register (ViPipe,0x3418, 0xb2);
    OS08A10_write_register (ViPipe,0x3419, 0x08);
    OS08A10_write_register (ViPipe,0x3441, 0x0a);
    OS08A10_write_register (ViPipe,0x3442, 0x0a);
    OS08A10_write_register (ViPipe,0x3443, 0x03);
    OS08A10_write_register (ViPipe,0x3444, 0x20);
    OS08A10_write_register (ViPipe,0x3445, 0x25);
    OS08A10_write_register (ViPipe,0x3446, 0x77);
    OS08A10_write_register (ViPipe,0x3447, 0x00);
    OS08A10_write_register (ViPipe,0x3448, 0x67);
    OS08A10_write_register (ViPipe,0x3449, 0x00);
    OS08A10_write_register (ViPipe,0x344a, 0x47);
    OS08A10_write_register (ViPipe,0x344b, 0x00);
    OS08A10_write_register (ViPipe,0x344c, 0x37);
    OS08A10_write_register (ViPipe,0x344d, 0x00);
    OS08A10_write_register (ViPipe,0x344e, 0x3f);
    OS08A10_write_register (ViPipe,0x344f, 0x00);
    OS08A10_write_register (ViPipe,0x3450, 0xff);
    OS08A10_write_register (ViPipe,0x3451, 0x00);
    OS08A10_write_register (ViPipe,0x3452, 0x3f);
    OS08A10_write_register (ViPipe,0x3453, 0x00);
    OS08A10_write_register (ViPipe,0x3454, 0x37);
    OS08A10_write_register (ViPipe,0x3455, 0x00);
    OS08A10_write_register (ViPipe,0x3472, 0x9c);
    OS08A10_write_register (ViPipe,0x3473, 0x07);
    OS08A10_write_register (ViPipe,0x3480, 0x49);
    OS08A10_default_reg_init(ViPipe);
    OS08A10_write_register (ViPipe,0x3000, 0x00); /* standby */
    delay_ms(20);
    OS08A10_write_register (ViPipe,0x3002, 0x00); /* master mode start */

    printf("===OS08A10 OS08A10 1080P60fps 10bit 2to1 WDR(120fps->60fps) init success!=====\n");
    return;
}
#endif

#if 0
void OS08A10_wdr_1080p120_3to1_init(VI_PIPE ViPipe)
{
    OS08A10_write_register (ViPipe,0x3000, 0x01); /* standby */
    OS08A10_write_register (ViPipe,0x3002, 0x01); /* XTMSTA */

    OS08A10_write_register (ViPipe,0x3005, 0x00);
    OS08A10_write_register (ViPipe,0x3007, 0x00);
    OS08A10_write_register (ViPipe,0x3009, 0x00);
    OS08A10_write_register (ViPipe,0x300a, 0x3c);
    OS08A10_write_register (ViPipe,0x300c, 0x21);
    OS08A10_write_register (ViPipe,0x300f, 0x00);
    OS08A10_write_register (ViPipe,0x3010, 0x21);
    OS08A10_write_register (ViPipe,0x3012, 0x64);
    OS08A10_write_register (ViPipe,0x3016, 0x09);
    OS08A10_write_register (ViPipe,0x3018, 0x65);
    OS08A10_write_register (ViPipe,0x3019, 0x04);
    OS08A10_write_register (ViPipe,0x301c, 0x4c);
    OS08A10_write_register (ViPipe,0x301d, 0x04);

#if 1
    OS08A10_write_register (ViPipe,0x3020, 0x04);  /* SHS1 */
    OS08A10_write_register (ViPipe,0x3021, 0x00);
    //OS08A10_write_register (ViPipe,0x3022, 0x00);
    OS08A10_write_register (ViPipe,0x3024, 0xf2);  /* SHS2 */
    OS08A10_write_register (ViPipe,0x3025, 0x01);
    //OS08A10_write_register (ViPipe,0x3025, 0x00);
    OS08A10_write_register (ViPipe,0x3028, 0x57);  /* SHS3 */
    OS08A10_write_register (ViPipe,0x3029, 0x02);
    //OS08A10_write_register (ViPipe,0x302A, 0x00);
    OS08A10_write_register (ViPipe,0x3030, 0xed);  /* RHS1 */
    OS08A10_write_register (ViPipe,0x3031, 0x01);
    OS08A10_write_register (ViPipe,0x3034, 0x30);  /* RHS2 */
    OS08A10_write_register (ViPipe,0x3035, 0x02);
#else
    OS08A10_write_register (ViPipe,0x3020, 0x04);
    OS08A10_write_register (ViPipe,0x3024, 0x89);
    OS08A10_write_register (ViPipe,0x3028, 0x93);
    OS08A10_write_register (ViPipe,0x3029, 0x01);
    OS08A10_write_register (ViPipe,0x3030, 0x85);
    OS08A10_write_register (ViPipe,0x3034, 0x92);
#endif

    OS08A10_write_register (ViPipe,0x3045, 0x05);
    OS08A10_write_register (ViPipe,0x3046, 0x00);
    OS08A10_write_register (ViPipe,0x304b, 0x0a);
    OS08A10_write_register (ViPipe,0x305c, 0x18);
    OS08A10_write_register (ViPipe,0x305d, 0x03);
    OS08A10_write_register (ViPipe,0x305e, 0x20);
    OS08A10_write_register (ViPipe,0x305f, 0x01);
    OS08A10_write_register (ViPipe,0x3070, 0x02);
    OS08A10_write_register (ViPipe,0x3071, 0x11);
    OS08A10_write_register (ViPipe,0x309b, 0x10);
    OS08A10_write_register (ViPipe,0x309c, 0x22);
    OS08A10_write_register (ViPipe,0x30a2, 0x02);
    OS08A10_write_register (ViPipe,0x30a6, 0x20);
    OS08A10_write_register (ViPipe,0x30a8, 0x20);
    OS08A10_write_register (ViPipe,0x30aa, 0x20);
    OS08A10_write_register (ViPipe,0x30ac, 0x20);
    OS08A10_write_register (ViPipe,0x30b0, 0x43);
    OS08A10_write_register (ViPipe,0x3106, 0x33);
    OS08A10_write_register (ViPipe,0x3119, 0x9e);
    OS08A10_write_register (ViPipe,0x311c, 0x1e);
    OS08A10_write_register (ViPipe,0x311e, 0x08);
    OS08A10_write_register (ViPipe,0x3128, 0x05);
    OS08A10_write_register (ViPipe,0x3129, 0x1d);
    OS08A10_write_register (ViPipe,0x313d, 0x83);
    OS08A10_write_register (ViPipe,0x3150, 0x03);
    OS08A10_write_register (ViPipe,0x315e, 0x1a);
    OS08A10_write_register (ViPipe,0x3164, 0x1a);
    OS08A10_write_register (ViPipe,0x317c, 0x12);
    OS08A10_write_register (ViPipe,0x317e, 0x00);
    OS08A10_write_register (ViPipe,0x31ec, 0x37);

    OS08A10_write_register (ViPipe,0x32b8, 0x50);
    OS08A10_write_register (ViPipe,0x32b9, 0x10);
    OS08A10_write_register (ViPipe,0x32ba, 0x00);
    OS08A10_write_register (ViPipe,0x32bb, 0x04);
    OS08A10_write_register (ViPipe,0x32c8, 0x50);
    OS08A10_write_register (ViPipe,0x32c9, 0x10);
    OS08A10_write_register (ViPipe,0x32ca, 0x00);
    OS08A10_write_register (ViPipe,0x32cb, 0x04);

    OS08A10_write_register (ViPipe,0x332c, 0xd3);
    OS08A10_write_register (ViPipe,0x332d, 0x10);
    OS08A10_write_register (ViPipe,0x332e, 0x0d);
    OS08A10_write_register (ViPipe,0x3358, 0x06);
    OS08A10_write_register (ViPipe,0x3359, 0xe1);
    OS08A10_write_register (ViPipe,0x335a, 0x11);
    OS08A10_write_register (ViPipe,0x3360, 0x1e);
    OS08A10_write_register (ViPipe,0x3361, 0x61);
    OS08A10_write_register (ViPipe,0x3362, 0x10);
    OS08A10_write_register (ViPipe,0x33b0, 0x50);
    OS08A10_write_register (ViPipe,0x33b2, 0x1a);
    OS08A10_write_register (ViPipe,0x33b3, 0x04);

    OS08A10_write_register (ViPipe,0x3405, 0x00);
    OS08A10_write_register (ViPipe,0x3407, 0x03);
    OS08A10_write_register (ViPipe,0x3414, 0x0a);
    OS08A10_write_register (ViPipe,0x3418, 0x55);
    OS08A10_write_register (ViPipe,0x3419, 0x11);
    OS08A10_write_register (ViPipe,0x3441, 0x0a);
    OS08A10_write_register (ViPipe,0x3442, 0x0a);
    OS08A10_write_register (ViPipe,0x3443, 0x03);
    OS08A10_write_register (ViPipe,0x3444, 0x20);
    OS08A10_write_register (ViPipe,0x3445, 0x25);
    OS08A10_write_register (ViPipe,0x3446, 0x77);
    OS08A10_write_register (ViPipe,0x3447, 0x00);
    OS08A10_write_register (ViPipe,0x3448, 0x67);
    OS08A10_write_register (ViPipe,0x3449, 0x00);
    OS08A10_write_register (ViPipe,0x344a, 0x47);
    OS08A10_write_register (ViPipe,0x344b, 0x00);
    OS08A10_write_register (ViPipe,0x344c, 0x37);
    OS08A10_write_register (ViPipe,0x344d, 0x00);
    OS08A10_write_register (ViPipe,0x344e, 0x3f);
    OS08A10_write_register (ViPipe,0x344f, 0x00);
    OS08A10_write_register (ViPipe,0x3450, 0xff);
    OS08A10_write_register (ViPipe,0x3451, 0x00);
    OS08A10_write_register (ViPipe,0x3452, 0x3f);
    OS08A10_write_register (ViPipe,0x3453, 0x00);
    OS08A10_write_register (ViPipe,0x3454, 0x37);
    OS08A10_write_register (ViPipe,0x3455, 0x00);
    OS08A10_write_register (ViPipe,0x3472, 0x9c);
    OS08A10_write_register (ViPipe,0x3473, 0x07);
    OS08A10_write_register (ViPipe,0x3480, 0x49);
    OS08A10_default_reg_init(ViPipe);
    OS08A10_write_register (ViPipe,0x3000, 0x00); /* standby */
    delay_ms(20);
    OS08A10_write_register (ViPipe,0x3002, 0x00); /* master mode start */

    printf("===OS08A10 OS08A10 1080P30fps 10bit 3to1 WDR(120fps->30fps) init success!=====\n");

    return;
}

#endif

