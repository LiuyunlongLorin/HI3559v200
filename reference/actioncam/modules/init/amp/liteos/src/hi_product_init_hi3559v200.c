/**
 * @file      hi_product_init_chip.c
 * @brief     chip init source
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 * @version   1.0

 */
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "hi_product_init_os.h"
#include "hi_product_init_chip.h"
#include "hi_product_media_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define PERI_CRG40 0x120100A0
#define SYS_WRITEL(Addr, Value) ((*(volatile unsigned int *)(Addr)) = (Value))
#define SYS_READ(Addr)          (*((volatile int *)(Addr)))


#define MMZ_START_ADDRESS CFG_LITEOS_MMZ_START
#define MMZ_SIZE          CFG_LITEOS_MMZ_LEN


#define MAX_SENSOR_NUM        (1)
#define MAX_SENSOR_NAME_LEN   (32)
#define MAX_CHIP_NAME_LEN     (32)

typedef enum hiPDT_INIT_SENSOR_TYPE_E
{
    PDT_INIT_SENSOR_TYPE_I2C = 1,
    PDT_INIT_SENSOR_TYPE_SPI ,
    PDT_INIT_SENSOR_TYPE_BUTT
} PDT_INIT_SENSOR_TYPE_E;

typedef enum hiPDT_INIT_SENSOR_CLK_E
{
    PDT_INIT_SENSOR_CLK_74_25MHz = 0x0,      /**<74.25MHz*/
    PDT_INIT_SENSOR_CLK_72MHz = 0x1,         /**<72MHz*/
    PDT_INIT_SENSOR_CLK_54MHz = 0x2,         /**<54MHz*/
    PDT_INIT_SENSOR_CLK_50MHz = 0x3,         /**<50MHz*/
    PDT_INIT_SENSOR_CLK_24MHz = 0x4,         /**<24MHz*/
    PDT_INIT_SENSOR_CLK_37_125MHz = 0x8,     /**<37.125MHz*/
    PDT_INIT_SENSOR_CLK_36MHz = 0x9,         /**<36MHz*/
    PDT_INIT_SENSOR_CLK_27MHz = 0xa,         /**<27MHz*/
    PDT_INIT_SENSOR_CLK_25MHz = 0xb,         /**<25MHz*/
    PDT_INIT_SENSOR_CLK_12MHz = 0xc,         /**<12MHz*/
    PDT_INIT_SENSOR_CLK_BUTT
} PDT_INIT_SENSOR_CLK_E;


typedef struct hiPDT_INIT_SENSOR_CFG_S
{
    PDT_INIT_SENSOR_TYPE_E   enSensortype;
    HI_U32    u32SensorBusId;
    PDT_INIT_SENSOR_CLK_E    enSensorClk;
    HI_CHAR   szSensorName[MAX_SENSOR_NAME_LEN];
} PDT_INIT_SENSOR_CFG_S;


static PDT_INIT_SENSOR_CFG_S g_stSensorCfg[MAX_SENSOR_NUM];

#if 0
/**< 9022 i2c 11*/
static void i2c11_pin_mux(void)
{
    himm(0x1F0010F4, 0x00001C00);
    himm(0x1F0010F0, 0x00001C00);
}
#endif

#if defined(CONFIG_SCREEN_TYPE_MIPI)
/**< mipi tx mod*/
static void vo_mipi_tx_mode(void)
{
    HI_S32 s32Value = 0;
    himd(0x0451011C, &s32Value);
    s32Value |= (1 << 0);
    himm(0x0451011C, s32Value);

    himm(0x1f001088, 0x0);

    himm(0x12146004, 0x1);
    himm(0x12146400, 0x1);
    himm(0x12140010, 0x4);
    himm(0x12140400, 0x4);
}
#endif

/**< LCD 8bit mod*/
#if (defined(CONFIG_SCREEN_TYPE_LCD8BIT) || defined(CONFIG_SCREEN_TYPE_LCD6BIT))
static void vo_lcd8bit_mode(void)
{
    himm(0x112F0068 ,0x4f4);//LCD_CLK B19
    himm(0x112F0084 ,0x454);//LCD_DE A15
    himm(0x112F007c ,0x474);//LCD_HSYNC B16
    himm(0x112F0088 ,0x674);//LCD_VSYNC B15
    himm(0x112F0080 ,0x674);//LCD_DATA0 C16
    himm(0x112F0074 ,0x474);//LCD_DATA1 A17
    himm(0x112F0078 ,0x474);//LCD_DATA2 B17
    himm(0x112F006c ,0x474);//LCD_DATA3 B18
    himm(0x112F0070 ,0x474);//LCD_DATA4 C18
    himm(0x112F0064 ,0x674);//LCD_DATA5 A19
    //TODO: 8Bit
}
#endif

/**<  bt1120 mod*/
#if 0
static void vo_bt1120_mode(void)
{
    himm(0x047e0020, 0x000000f4); //1A19 VOU1120_CLK

    himm(0x047e001c, 0x00000454); //1C17 VOU1120_DATA0
    himm(0x047e0018, 0x00000454); //1B17 VOU1120_DATA1
    himm(0x047e0014, 0x00000454); //1A17 VOU1120_DATA2
    himm(0x047e0010, 0x00000454); //1B16 VOU1120_DATA3
    himm(0x047e000c, 0x00000454); //1A16 VOU1120_DATA4
    himm(0x047e0008, 0x00000454); //1B15 VOU1120_DATA5
    himm(0x047e0004, 0x00000454); //1A15 VOU1120_DATA6
    himm(0x047e0000, 0x00000454); //1C15 VOU1120_DATA7

#if 0
    //DEBM board: VOU1120_8~VOU1120_15
    himm(0x047e0060, 0x00000454); //1F19 VOU1120_DATA8
    himm(0x047e005c, 0x00000454); //1F18 VOU1120_DATA9
    himm(0x047e0054, 0x00000454); //1E18 VOU1120_DATA10
    himm(0x047e0058, 0x00000454); //1B19 VOU1120_DATA11
    himm(0x047e0028, 0x00000454); //1D19 VOU1120_DATA12
    himm(0x047e0024, 0x00000454); //1C19 VOU1120_DATA13
    himm(0x047e0030, 0x00000454); //C36  VOU1120_DATA14
    himm(0x047e002c, 0x00000454); //B36  VOU1120_DATA15

#else
    /* DMEBLITE board pin pwr config */
    himm(0x04528048, 0x00000037); //MISC_CTRL18

    //DMEBLITE board: VOU1120_8~VOU1120_15
    himm(0x047e006c, 0x00000454); //A31  VOU1120_DATA8
    himm(0x047e0068, 0x00000454); //B30  VOU1120_DATA9
    himm(0x047e0078, 0x00000454); //B29  VOU1120_DATA10
    himm(0x047e0074, 0x00000454); //A29  VOU1120_DATA11
    himm(0x047e0064, 0x00000454); //B28  VOU1120_DATA12
    himm(0x047e0080, 0x00000454); //B27  VOU1120_DATA13
    himm(0x047e007c, 0x00000454); //A27  VOU1120_DATA14
    himm(0x047e0070, 0x00000454); //B26  VOU1120_DATA15
#endif
    //printf( "============vo_output_mode done=============\n");
}
#endif

/**<  LCD 24bit mod*/
#if defined(CONFIG_SCREEN_TYPE_LCD24BIT)
static void vo_lcd24bit_mode(void)
{
    himm(0x047E0020, 0x000004f3); //1A19 LCD_CLK
    //himm(0x047E0048, 0x000004f3); //D37 LCD_DE
    himm(0x047E0058, 0x000004f6); //1B19 LCD_DE

    himm(0x047E004C, 0x000004f3); //D36 LCD_HSYNC
    himm(0x047E0034, 0x000004f3); //1C18 LCD_VSYNC

    himm(0x047E0000, 0x000004f3); //1C15 LCD_DATA0
    himm(0x047E0004, 0x000004f3); //1A15 LCD_DATA1
    himm(0x047E0008, 0x000004f3); //1B15 LCD_DATA2
    himm(0x047E000C, 0x000004f3); //1A16 LCD_DATA3
    himm(0x047E0010, 0x000004f3); //1B16 LCD_DATA4
    himm(0x047E0014, 0x000004f3); //1A17 LCD_DATA5
    himm(0x047E0018, 0x000004f3); //1B17 LCD_DATA6
    himm(0x047E001C, 0x000004f3); //1C17 LCD_DATA7

    himm(0x047E0024, 0x000004f7); //1C19 LCD_DATA8
    himm(0x047E0028, 0x000004f7); //1D19 LCD_DATA9
    himm(0x047E002C, 0x000004f7); //B36 LCD_DATA10
    himm(0x047E0030, 0x000004f7); //C36 LCD_DATA11
    himm(0x047E0070, 0x000004f7); //B26 LCD_DATA12
    himm(0x047E0064, 0x000004f7); //B28 LCD_DATA13
    himm(0x047E0074, 0x000004f7); //A29 LCD_DATA14
    himm(0x047E0078, 0x000004f7); //B29 LCD_DATA15
    himm(0x047E0068, 0x000004f7); //B30 LCD_DATA16
    himm(0x047E006C, 0x000004f7); //A31 LCD_DATA17
    himm(0x047E0094, 0x000004f7); //B32 LCD_DATA18
    himm(0x047E0098, 0x000004f7); //B33 LCD_DATA19
    himm(0x047E0088, 0x000004f7); //A33 LCD_DATA20
    himm(0x047E008C, 0x000004f7); //B34 LCD_DATA21
    himm(0x047E0090, 0x000004f7); //B35 LCD_DATA22
    himm(0x047E0084, 0x000004f7); //A35 LCD_DATA23
}
#endif

/**<  LCD 24bit mod*/
#if defined(CONFIG_SCREEN_TYPE_LCD16BIT)
static void vo_lcd16bit_mode(void)
{
    himm(0x112F0034 ,0x4f2);//LCD_CLK B19
    himm(0x112F0058 ,0x4f2);//LCD_DE A15
    himm(0x112F004c ,0x4f2);//LCD_HSYNC B16
    himm(0x112F0054 ,0x4f2);//LCD_VSYNC B15

    himm(0x112F0048 ,0x4f2);//LCD_DATA0 C16
    himm(0x112F0040 ,0x6f2);//LCD_DATA1 A17
    himm(0x112F0044 ,0x6f2);//LCD_DATA2 B17
    himm(0x112F005c ,0x6f2);//LCD_DATA3 B18
    himm(0x112F003c ,0x6f2);//LCD_DATA4 C18

    himm(0x112F0038 ,0x4f2);//LCD_DATA5 A19
    himm(0x112F0050 ,0x6f2);//LCD_DATA6 A19
    himm(0x112F0060 ,0x4f2);//LCD_DATA7 A19
    himm(0x112F0084, 0x6f2); //1C19 LCD_DATA8
    himm(0x112F0088, 0x6f2); //1D19 LCD_DATA9
    himm(0x112F007C, 0x4f2); //B36 LCD_DATA10


    himm(0x112F0080, 0x4f2); //C36 LCD_DATA11
    himm(0x112F0074, 0x4f2); //B26 LCD_DATA12
    himm(0x112F0078, 0x4f2); //B28 LCD_DATA13
    himm(0x112F006c, 0x4f2); //A29 LCD_DATA14
    himm(0x112F0070, 0x4f2); //B29 LCD_DATA15

    //himm(0x112F0068, 0x6f2); //B29 LCD_DATA16
    //himm(0x112F0064, 0x6f2); //B29 LCD_DATA17
}
#endif


/**< I2C 0 -> ? */
static void i2c0_pin_mux(void)
{
    himm(0x114F0074, 0x412);
    himm(0x114F0078, 0x412);
}

/**< I2C 1 --> sensor */
static void i2c1_pin_mux(void)
{
    himm(0x114F007C, 0x412);
    himm(0x114F0080, 0x512);
}
/**< I2C 2 --> sensor */
 void i2c2_pin_mux(void)
{
    himm(0x111F0018, 0x511);
    himm(0x111F001C, 0x511);
}

/**< I2C 3 --> sensor */
 void i2c3_pin_mux(void)
{
    himm(0x112F0090, 0x511);
    himm(0x112F008C, 0x511);
}

/**< SPI 0 */
static void spi0_pin_mux(void)
{
    himm(0x114F0074, 0x411);
    himm(0x114F0078, 0x411);
    himm(0x114F007C, 0x411);
    himm(0x114F0080, 0x411);
}

/**< SPI 1 */
static void spi1_pin_mux(void)
{
    himm(0x112F0020, 0x4f1);
    himm(0x112F0024, 0x4f1);
    himm(0x112F002c, 0x14f1);
    himm(0x112F0028, 0x4f1);
}

#if defined(BOARD_ACTIONCAM_REFB)
static void spi2_pin_mux(void)
{
    himm(0x114F0050, 0x4f4);
    himm(0x114F0054, 0x4f4);
    himm(0x114F0058, 0x4f4);
    himm(0x114F005C, 0x4f4);
}
#endif

/**< HMDI  */
static void hmdi_pin_mode(void)
{
    himm(0x114F0000, 0x431);
    himm(0x114F0004, 0x431);
    himm(0x114F0008, 0x631);
    himm(0x114F000C, 0x621);
}

/**< I2S PIN  */
void i2s_pin_mux(void)
{
    himm(0x112F0020, 0x663);
    himm(0x112F0024, 0x673);
    himm(0x112F0028, 0x573);
    himm(0x112F002C, 0x473);
    himm(0x112F0030, 0x573);
}

void vi_mipi_rx_mux(void)
{
    himm(0x114F0028, 0x400);
    himm(0x114F002C, 0x400);
    himm(0x114F0030, 0x400);
    himm(0x114F0034, 0x400);
    himm(0x114F0038, 0x400);
    himm(0x114F003C, 0x400);
    himm(0x114F0018, 0x400);
    himm(0x114F001C, 0x400);
    himm(0x114F0020, 0x400);
    himm(0x114F0024, 0x400);
}

void vi_slave_mode_mux(void)
{
    himm(0x114F0060, 0x662);
    himm(0x114F0064, 0x672);
}

static void Lcd_Pin_Mux(void)
{
#if defined(BOARD_DEMB)
    spi1_pin_mux();
#elif defined(BOARD_ACTIONCAM_REFB)
    spi2_pin_mux();
#endif

#if defined(CONFIG_SCREEN_TYPE_MIPI)
    //vo_mipi_tx_mode();
#elif (defined(CONFIG_SCREEN_TYPE_LCD8BIT) || defined(CONFIG_SCREEN_TYPE_LCD6BIT))
    vo_lcd8bit_mode();
#elif defined(CONFIG_SCREEN_TYPE_LCD24BIT)
    vo_lcd24bit_mode();
#elif defined(CONFIG_SCREEN_TYPE_LCD16BIT)
    vo_lcd16bit_mode();
#endif
}

static void sensor_pin_mux(void)
{
    himm(0x114F006C, 0x461);
    himm(0x114F0070, 0x671);
}

static HI_VOID sensor_pinmux_config(void)
{
    HI_U32 i = 0;

    sensor_pin_mux();

    for (i = 0; i < MAX_SENSOR_NUM; i++)
    {
        if (g_stSensorCfg[i].enSensortype == PDT_INIT_SENSOR_TYPE_I2C)
        {
            if (0 == g_stSensorCfg[i].u32SensorBusId)
            {
                i2c0_pin_mux();
            }
            else if (1 == g_stSensorCfg[i].u32SensorBusId)
            {
                i2c1_pin_mux();
            }
            else
            {
                return;
            }
        }

        else if (g_stSensorCfg[i].enSensortype == PDT_INIT_SENSOR_TYPE_SPI)
        {
            if (0 == g_stSensorCfg[i].u32SensorBusId)
            {
                spi0_pin_mux();
            }
            else if (1 == g_stSensorCfg[i].u32SensorBusId)
            {
                spi1_pin_mux();
            }
            else
            {
                return;
            }
        }
        else
        {
        }
    }
}

static void peripheral_pinmux_cfg(void)
{
    /**<bus_pin_mux for lcd*/
    Lcd_Pin_Mux();

    /**<bus_pin_mux for 9022 */
    //i2c0_pin_mux();

    vi_mipi_rx_mux();
    vi_slave_mode_mux();

    /**<bus_pin_mux for hmdi*/
    hmdi_pin_mode();

    /**<bus_pin_mux for audio dev */
    //i2s_pin_mux();
}

void set_vi_workmode(int online_flag)
{
    if (1 == online_flag) /* vi online */
    {
        // write
        himm(0x12030080, 0x57566776);
        himm(0x12030084, 0x30000071);
        himm(0x12030088, 0x00444455);
        himm(0x1203008c, 0x00000003);

        // read
        himm(0x12030090, 0x57566776);
        himm(0x12030094, 0x30000071);
        himm(0x12030098, 0x00445455);
        himm(0x1203009c, 0x00000003);
    }
    else /* offline */
    {
        // write
        himm(0x12030080, 0x46565667);
        himm(0x12030084, 0x30000071);
        himm(0x12030088, 0x00444445);
        himm(0x1203008c, 0x00000003);

        // read
        himm(0x12030090, 0x56575677);
        himm(0x12030094, 0x30000071);
        himm(0x12030098, 0x00445555);
        himm(0x1203009c, 0x00000003);
    }
}

void ddrc_config(void)
{
    himm(0x12060200, 0x00110000);
    himm(0x12060210, 0x00110000);
    himm(0x12060220, 0x00110000);
    himm(0x12060230, 0x00110000);
    himm(0x12060240, 0x00110000);
    himm(0x12060250, 0x00110000);
    himm(0x12060260, 0x00110000);
    himm(0x12060270, 0x00110000);
    himm(0x12060280, 0x00110000);
    himm(0x12060290, 0x00110000);
    himm(0x120602a0, 0x00110000);
    himm(0x120602b0, 0x00110000);

    himm(0x12060204, 0x01234567);
    himm(0x12060214, 0x01234567);
    himm(0x12060224, 0x01234567);
    himm(0x12060234, 0x01234567);
    himm(0x12060244, 0x01234567);
    himm(0x12060254, 0x01234567);
    himm(0x12060264, 0x01234567);
    himm(0x12060274, 0x01234567);
    himm(0x12060284, 0x01234567);
    himm(0x12060294, 0x01234567);
    himm(0x120602a4, 0x01234567);
    himm(0x120602b4, 0x01234567);

    himm(0x12060208, 0x01234567);
    himm(0x12060218, 0x01234567);
    himm(0x12060228, 0x01234567);
    himm(0x12060238, 0x01234567);
    himm(0x12060248, 0x01234567);
    himm(0x12060258, 0x01234567);
    himm(0x12060268, 0x01234567);
    himm(0x12060278, 0x01234567);
    himm(0x12060288, 0x01234567);
    himm(0x12060298, 0x01234567);
    himm(0x120602a8, 0x01234567);
    himm(0x120602b8, 0x01234567);

    himm(0x12064000, 0x00000002);
    himm(0x1206410c, 0x0000000b);
    himm(0x12064110, 0x0000000b);
    himm(0x1206408c, 0x90b20906);
    himm(0x12064090, 0x90620906);
    himm(0x120640f4, 0x00000033);
    himm(0x120640ec, 0x00000011);
    himm(0x120640f0, 0x00001111);
    himm(0x120641f4, 0x00000000);

    himm(0x120641f0, 0x1);
    himm(0x120640ac, 0x00000080);
    himm(0x120641f8, 0x800002);
    himm(0x12064068, 0x51);
    himm(0x1206406c, 0x51);

    himm(0x12064300, 0x20040);
}


static int ampunmute(void)
{
    himm(0x111F0034, 0x431);
    himm(0x120DA3FC, 0x000000ff);
    himm(0x120DA400, 0x000000ff);
    himm(0x120DA3FC, 0x000000ff);

    return 0;
}


void sys_ctl(int online_flag)
{
    set_vi_workmode(online_flag);
    ddrc_config();
}

#if 0
/**<Adjust priority*/
static void system_qosbuf(void)
{
    himm(0x12064000, 0x2);
    himm(0x1206410c, 0x17);
    himm(0x12064110, 0x17);
    himm(0x12064088, 0x5);
    himm(0x1206408c, 0x92821e14);
    himm(0x12064090, 0x92821e14);
    himm(0x12064068, 0x51);
    himm(0x1206406c, 0x51);
    himm(0x120640ac, 0x80);
    himm(0x120640ec, 0x11);
    himm(0x120640f0, 0x1111);
    himm(0x120640f4, 0x33);
    himm(0x120641f0, 0x1);
    himm(0x120641f4, 0x0);
    himm(0x120641f8, 0x00800002);
}
#endif

/**< CLK config **/
static void system_clk_cfg(const char* pchip)
{
    MLOGI("\n==========chip: %s==========\n", pchip);

    himm(0x120101B8, 0x0007f81f);
    himm(0x120100F0, 0x00000861);
    himm(0x120100F8, 0x0000000f);

    if (0 == strncmp("hi3559v200", pchip, MAX_CHIP_NAME_LEN))
    {
#ifdef CONFIG_SNS0_OV12870
        writereg(0x1 << 21,0x7 << 21,PERI_CRG40); /**<[23,21] vpss clk, 001 396M*/
        writereg(0x2 << 15,0x7 << 15,PERI_CRG40); /**<[17,15] ive  clk, 010 300M*/
        writereg(0x0 << 6, 0x7 << 6, PERI_CRG40); /**<[8,6]   vgs  clk, 000 475M*/
        writereg(0x0 << 4, 0x3 << 4, PERI_CRG40); /**<[5,4]   gdc  clk,  00 475M*/
        himm(0x120100F4, 0x24903ff);
        himm(0x12010100, 0x00f00492);
        himm(0x120100FC, 0x10);
#else
        writereg(0x1 << 21,0x7 << 21,PERI_CRG40); /**<[23,21] vpss clk, 001 396M*/
        writereg(0x2 << 15,0x7 << 15,PERI_CRG40); /**<[17,15] ive  clk, 010 300M*/
        writereg(0x0 << 6, 0x7 << 6, PERI_CRG40); /**<[8,6]   vgs  clk, 000 475M*/
        writereg(0x0 << 4, 0x3 << 4, PERI_CRG40); /**<[5,4]   gdc  clk,  00 475M*/
        himm(0x120100F4, 0x027183FF);/**<PER1_CRG61 VICAP Port0 clk 396M, VICAP PPC clk 396M*/
        himm(0x12010100, 0x6db);     /**<PER1_CRG64 isp0~isp3 clk 396M*/
        himm(0x120100FC, 0x11);      /**<PER1_CRG63 VIPROC offline mode clk 396M*/
#endif
    }
    else if (0 == strncmp("hi3556v200", pchip, MAX_CHIP_NAME_LEN))
    {
        writereg(0x2 << 21,0x7 << 21,PERI_CRG40); /**<[23,21] vpss clk, 010 300M*/
        writereg(0x2 << 15,0x7 << 15,PERI_CRG40); /**<[17,15] ive  clk, 010 300M*/
        writereg(0x3 << 6, 0x7 << 6, PERI_CRG40); /**<[8,6]   vgs  clk, 011 300M*/
        writereg(0x3 << 4, 0x3 << 4, PERI_CRG40); /**<[5,4]   gdc  clk,  11 300M*/
        himm(0x120100F4, 0x29203ff);
        himm(0x12010100, 0x00f00924);
        himm(0x120100FC, 0x12);
    }
    else
    {
        writereg(0x1 << 21,0x7 << 21,PERI_CRG40); /**<[23,21] vpss clk, 001 396M*/
        writereg(0x2 << 15,0x7 << 15,PERI_CRG40); /**<[17,15] ive  clk, 010 300M*/
        writereg(0x2 << 6, 0x7 << 6, PERI_CRG40); /**<[8,6]   vgs  clk, 000 396M*/
        writereg(0x0 << 4, 0x3 << 4, PERI_CRG40); /**<[5,4]   gdc  clk,  00 475M*/
        himm(0x120100F4, 0x026d83ff);
        himm(0x12010100, 0x00f006dB);
        himm(0x120100FC, 0x11);
    }

    himm(0x12030018, 0x540);
    himm(0x12030034, 0x1);
    himm(0x12030000, 0x0);
    himm(0x12010018, 0x12000000);
    himm(0x1201001C, 0x1002063);
}

#if 0
static void system_pwr_en(void)
{
    /**< set MISC_CTRL44, AVSP VDH VEDU2 VPSS1 VIPROC1 power up*/
    himm(0x120300ac, 0x33333332);
    himm(0x120300b0, 0x00033333);
    HI_usleep(10000);
    himm(0x120300ac, 0x22222222);
    himm(0x120300b0, 0x00022222);
}
#endif

/**< MMZ init for media **/
static HI_S32 MMZ_init(void)
{
    extern int media_mem_init(void * pArgs);
    MMZ_MODULE_PARAMS_S stMMZ_Param;

    snprintf(stMMZ_Param.mmz, MMZ_SETUP_CMDLINE_LEN, "anonymous,0,0x%lx,%uK", (HI_UL)MMZ_START_ADDRESS, MMZ_SIZE);
    snprintf(stMMZ_Param.map_mmz, MMZ_SETUP_CMDLINE_LEN, "0x%lx,%uK", (HI_UL)CFG_LINUX_MMZ_START, CFG_LINUX_MMZ_LEN);
    stMMZ_Param.anony = 1;

    MLOGI("mmz_start=0x%lx, mmz_size=0x%x\n", (HI_UL)MMZ_START_ADDRESS, MMZ_SIZE << 10);
    MLOGI("mapmmz_start=0x%lx, mmz_size=0x%x\n", (HI_UL)CFG_LINUX_MMZ_START, CFG_LINUX_MMZ_LEN << 10);
    MLOGI("mmz param=%s\n", stMMZ_Param.mmz);
    return media_mem_init(&stMMZ_Param);
}

/**< GDC config and init  **/
static HI_S32 GDC_init(void)
{
    extern int gdc_mod_init(void * pArgs);

    GDC_MODULE_PARAMS_S stGdcModuleParam = {0};
    stGdcModuleParam.u32MaxGdcJob   = 32;
    stGdcModuleParam.u32MaxGdcTask  = 64;
    stGdcModuleParam.u32MaxGdcNode  = 64;
    stGdcModuleParam.au32GdcEn[0]   = 1;

    return gdc_mod_init(&stGdcModuleParam);
}

/**< VGS config and init  **/
static HI_S32 VGS_init(void)
{
    extern int vgs_mod_init(void * pArgs);

    VGS_MODULE_PARAMS_S  stVgsModuleParam = {0};
    stVgsModuleParam.u32MaxVgsJob  = 64;
    stVgsModuleParam.u32MaxVgsTask = 100;
    stVgsModuleParam.u32MaxVgsNode = 100;
    stVgsModuleParam.au32VgsEn[0]  = 1;
    return vgs_mod_init(&stVgsModuleParam);
}

static HI_VOID sensor_driver_load(void)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_U32  i = 0;
    HI_BOOL bI2cDevInitFlg = HI_FALSE;
    HI_BOOL bSpiDevInitFlg = HI_FALSE;

    MLOGD("-->spi bus init ...\n");
    extern int spi_dev_init(void);
    spi_dev_init();
    MLOGD("-->i2c bus init ...\n");
    extern int i2c_dev_init(void);
    i2c_dev_init();
    for (i = 0; i < MAX_SENSOR_NUM; ++i)
    {
        if ((PDT_INIT_SENSOR_TYPE_I2C == g_stSensorCfg[i].enSensortype) && (!bI2cDevInitFlg))
        {
            /**< hi_sensor_i2c_init **/
            extern int hi_dev_init(void);
            s32Ret = hi_dev_init();
            HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "Hi_sensor_i2c_init");
            MLOGI("sensor[%s] set hi_sensor_i2c_init success!\n", g_stSensorCfg[i].szSensorName);
            bI2cDevInitFlg = HI_TRUE;
        }

        else if ((PDT_INIT_SENSOR_TYPE_SPI == g_stSensorCfg[i].enSensortype) && (!bSpiDevInitFlg))
        {
            /**< hi_sensor_spi_init **/
            extern int sensor_spi_dev_init(void * pArgs);
            s32Ret = sensor_spi_dev_init(NULL);
            HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "Hi_sensor_spi_init");
            MLOGI("sensor[%s] set hi_sensor_spi_init success!\n", g_stSensorCfg[i].szSensorName);
            bSpiDevInitFlg = 1;
        }
    }
}

static inline void reg_write32(unsigned long value, unsigned long mask, unsigned long addr)
{
    unsigned long t;

    t = SYS_READ((const volatile void *)addr);
    t &= ~mask;
    t |= value & mask;
    SYS_WRITEL((volatile void *)addr, t);
}

/**< 0x0: 74.25MHz 0x1: 72MHz 0x2: 54MHz 0x3: 50MHz 0x4: 24MHz 0x6: 32.4MHz */
/**< 0x8: 37.125MHz 0x9: 36MHz 0xA: 27MHz 0xB: 25MHz 0xC: 12MHz*/
static HI_VOID sensor_clock_config(void)
{
    HI_U32 u32SensorClk = 0x0;
    HI_U32 i = 0;

    for (i = 0; i < MAX_SENSOR_NUM; ++i)
    {
        reg_write32(((g_stSensorCfg[i].enSensorClk << 2) + 0x1) << (6 * i),
            0x3F << (6 * i), (HI_UL)0x120100F0);
    }

    MLOGI("Set SensorClk[%x] success ! \n", u32SensorClk);
}

static HI_S32 PM_init(void)
{
    extern int pm_mod_init(void* pArgs);
    PM_MODULE_PARAMS_S stPmModuleParam;
    stPmModuleParam.bAvspOn   = HI_FALSE;
    stPmModuleParam.bSvpAcceleratorOn  = HI_FALSE;

    return pm_mod_init(&stPmModuleParam);
}

static HI_VOID load_mpp(void)
{
    HI_S32 s32Ret = HI_SUCCESS;

    extern int base_mod_init(void);
    extern HI_U32 vb_force_exit;
    vb_force_exit = 0;
    s32Ret = base_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "BASE_init");

    extern int sys_mod_init(void);
    s32Ret = sys_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "SYS_init");

    extern int rgn_mod_init(void);
    s32Ret = rgn_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "RGN_init");

    s32Ret = GDC_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "GDC_init");

    s32Ret = VGS_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "VGS_init");

    extern int dis_mod_init(void);
    s32Ret = dis_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "DIS_init");

    extern int vi_mod_init(void);
    s32Ret = vi_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "VI_init");

    extern int isp_mod_init(void);
    s32Ret = isp_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "ISP_init");

    extern int vpss_mod_init(void * pArgs);
    s32Ret = vpss_mod_init(NULL);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "VPSS_init");
    VPSS_MOD_PARAM_S stVpssModParam;
    stVpssModParam.bOneBufForLowDelay = HI_FALSE;
    stVpssModParam.u32VpssVbSource = 0;
    stVpssModParam.u32VpssSplitNodeNum = 1;
    stVpssModParam.bHdrSupport = HI_FALSE;
    s32Ret = HI_MPI_VPSS_SetModParam(&stVpssModParam);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "HI_MPI_VPSS_SetModParam");

    extern int vou_mod_init(void);
    s32Ret = vou_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "VO_init");

    VO_MOD_PARAM_S stModParam;
    s32Ret = HI_MPI_VO_GetModParam(&stModParam);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "HI_MPI_VO_GetModParam");
    stModParam.bExitDev = HI_FALSE;
    stModParam.bTransparentTransmit = HI_FALSE;
    s32Ret = HI_MPI_VO_SetModParam(&stModParam);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "HI_MPI_VO_SetModParam");

    extern int  rc_mod_init(void);
    s32Ret = rc_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "RC_init");

    VENC_MODULE_PARAMS_S stVencModuleParam;
    stVencModuleParam.u32VencMaxChnNum = HI_PDT_MEDIA_VENC_MAX_CNT;
    extern int  venc_mod_init(void * pArgs);
    s32Ret = venc_mod_init(&stVencModuleParam);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "VENC_init");

    VENC_PARAM_MOD_S stVencModParam;
    stVencModParam.enVencModType = MODTYPE_VENC;
    s32Ret = HI_MPI_VENC_GetModParam(&stVencModParam);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "HI_MPI_VENC_GetModParam");

    stVencModParam.stVencModParam.u32VencBufferCache = 0;
    stVencModParam.stVencModParam.u32FrameBufRecycle=1;
    s32Ret = HI_MPI_VENC_SetModParam(&stVencModParam);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "HI_MPI_VENC_SetModParam");

    extern int chnl_mod_init(void);
    s32Ret = chnl_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "CHNL_init");

    extern int vedu_mod_init(void);
    s32Ret = vedu_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "VEDU_init");

    extern int h264e_mod_init(void * pArgs);
    s32Ret = h264e_mod_init(NULL);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "H264e_init");

    extern int h265e_mod_init(void * pArgs);
    s32Ret = h265e_mod_init(NULL);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "H265e_init");

    extern int jpege_mod_init(void * pArgs);
    s32Ret = jpege_mod_init(NULL);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "JPEGE_init");

    extern int jpegd_mod_init(void);
    s32Ret = jpegd_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "JPEGD_init");

    stVencModParam.enVencModType = MODTYPE_JPEGE;
    stVencModParam.stJpegeModParam.u32OneStreamBuffer = 0;
    stVencModParam.stJpegeModParam.u32JpegeMiniBufMode = 1;
    stVencModParam.stJpegeModParam.u32JpegClearStreamBuf = 1;
    s32Ret = HI_MPI_VENC_SetModParam(&stVencModParam);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "HI_MPI_VENC_SetModParam");

    extern int vfmw_mod_init(void * pArgs);
    s32Ret = vfmw_mod_init(NULL);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "VFMW_init");

    VDEC_MODULE_PARAMS_S  stVdecModuleParam;
    stVdecModuleParam.u32VdecMaxChnNum = 1;
    extern int vdec_mod_init(void * pArgs);
    s32Ret = vdec_mod_init(&stVdecModuleParam);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "VDEC_init");

    /**<Init Audio */
    extern int aiao_mod_init(void);
    s32Ret = aiao_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "AcodecMod_init");

    extern int  ai_mod_init(void);
    s32Ret = ai_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "AiMod_init");

    extern int ao_mod_init(void);
    s32Ret = ao_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "AoMod_init");

    extern int  aenc_mod_init(void);
    s32Ret = aenc_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "AencMod_init");

    extern int adec_mod_init(void);
    s32Ret = adec_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "AdecMod_init");
    HI_TIME_STAMP;
    extern int acodec_mod_init(void * pArgs);
    s32Ret = acodec_mod_init(NULL);
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "AcodecMod_init");
    HI_TIME_STAMP;
    extern int pwm_init(void);
    s32Ret = pwm_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "PWM_init");

    extern int hdmi_mod_init(void);
    s32Ret = hdmi_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "HDMI_init");

    extern int hi_user_init(void);
    s32Ret = hi_user_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "HI_USER_init");

    extern int mipi_rx_mod_init(void);
    s32Ret = mipi_rx_mod_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "MIPI_RX_init");

    extern int rtc_init(void);
    s32Ret = rtc_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "rtc_init");
}

static void lowpower_cfg(void)
{
    HI_S32 s32Value = 0;

    himm(0x12010190, 0x0);        /** close GZIP CLK after fastboot */

    himd(0x120101A0, &s32Value);
    s32Value &= ~(1<<9);          /** close SPACC CLK(use to encryption) */
    s32Value &= ~(1<<7);          /** close RSA1  CLK(use to encryption) */
    s32Value &= ~(1<<5);          /** close RSA0  CLK(use to encryption) */
    himm(0x120101A0, s32Value);

    himd(0x1201010C, &s32Value);
    s32Value &= ~(1<<0);          /** close MIPI_TX CTRL CLK */
    himm(0x1201010C, s32Value);
}

extern void osal_proc_init(void);

HI_S32 HI_PDT_INIT_CHIP_PreInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    memset(g_stSensorCfg, 0x00, sizeof(g_stSensorCfg));

#ifdef CONFIG_SNS0_IMX377
    g_stSensorCfg[0].enSensortype = PDT_INIT_SENSOR_TYPE_I2C;
    g_stSensorCfg[0].u32SensorBusId = 0;
    g_stSensorCfg[0].enSensorClk = PDT_INIT_SENSOR_CLK_24MHz;
    snprintf(g_stSensorCfg[0].szSensorName, MAX_SENSOR_NAME_LEN, "%s", "imx377");
#endif

#ifdef CONFIG_SNS0_IMX458
    g_stSensorCfg[0].enSensortype = PDT_INIT_SENSOR_TYPE_I2C;
    g_stSensorCfg[0].u32SensorBusId = 0;
    g_stSensorCfg[0].enSensorClk = PDT_INIT_SENSOR_CLK_24MHz;
    snprintf(g_stSensorCfg[0].szSensorName, MAX_SENSOR_NAME_LEN, "%s", "imx458");
#endif

#ifdef CONFIG_SNS0_OV12870
    g_stSensorCfg[0].enSensortype = PDT_INIT_SENSOR_TYPE_I2C;
    g_stSensorCfg[0].u32SensorBusId = 0;
    g_stSensorCfg[0].enSensorClk = PDT_INIT_SENSOR_CLK_24MHz;
    snprintf(g_stSensorCfg[0].szSensorName, MAX_SENSOR_NAME_LEN, "%s", "ov12870");
#endif

    sensor_pinmux_config();
    system_clk_cfg("hi3559v200");
    //system_pwr_en();
    sys_ctl(1);
    ampunmute();
    lowpower_cfg();
    peripheral_pinmux_cfg();
    osal_proc_init();
    s32Ret = MMZ_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "MMZ_init");
    load_mpp();

    sensor_driver_load();
    sensor_clock_config();

    s32Ret = PM_init();
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "PM_init");

    return s32Ret;
}

HI_S32 HI_PDT_INIT_CHIP_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    return s32Ret;
}

HI_S32 HI_PDT_INIT_CHIP_PostInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

