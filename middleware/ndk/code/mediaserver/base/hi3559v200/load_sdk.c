/******************************************************************************
  Some simple Hisilicon Hi3559A system functions.

  Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2017-11 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include "hi_type.h"
#include "osal_mmz.h"
#include "asm/io.h"
#include "hi_module_param.h"

#include "stdlib.h"
#include "fcntl.h"
#include "string.h"

//#define TEE_VERSION
#include "sensor_interface_cfg_params.h"

#ifdef CONFIG_HI_TZASC_SUPPORT
#include "tzasc_user.h"
#endif

#define SENSOR_LIST_CMDLINE_LEN     256
#define SENSOR_NAME_LEN             64
#define CHIP_NAME_LEN               64

#define MAX_SENSOR_TYPE_NUM 2

static int g_online_flag = 1;
static unsigned long long mmz_start = 0x88000000;
static unsigned int mmz_size = 336;   //M Byte

static char* sensor_type0 = "imx307"; /* imx335 imx327  imx290 imx377 imx458 */
static char* sensor_type1 = "unused"; /* imx307 bt656 */
static int  g_cmos_yuv_flag = 0;      /* vi: 0--RAW, 1--BT1120, 2--BT656 */
static char chip_list[CHIP_NAME_LEN] = "hi3559v200";  /* hi3559v200  hi3556v200 */

/******************************************************************/
#define himm(address, value)        writel(value, address)


void i2c0_pin_mux(void)
{
    himm(0x114F0074, 0x412);
    himm(0x114F0078, 0x412);
}

void i2c1_pin_mux(void)
{
    himm(0x114F007C, 0x412);
    himm(0x114F0080, 0x512);
}

void i2c2_pin_mux(void)
{
    himm(0x111F0018, 0x511);
    himm(0x111F001C, 0x511);
}

void i2c3_pin_mux(void)
{
    himm(0x112F0090, 0x511);
    himm(0x112F008C, 0x511);
}

void spi0_pin_mux(void)
{
    himm(0x114F0074, 0x411);
    himm(0x114F0078, 0x411);
    himm(0x114F007C, 0x411);
    himm(0x114F0080, 0x411);
}

void spi1_pin_mux(void)
{

    himm(0x112F0020, 0x4f1);//SCLK
    himm(0x112F0024, 0x4f1);//
    himm(0x112F002c, 0x14f1);//
    himm(0x112F0028, 0x4f1);//
}

/*spi 2 pin mux for motion sensor*/
void motionsensor_cfg_mux(void)
{
#ifdef SUPPORT_GYRO
    extern HI_U32 u32MotionSensorSpiNum;
    u32MotionSensorSpiNum = 2;

    himm(0x120101bc, 0x01807882);
    himm(0x114f0050, 0x474);
    himm(0x114f0054, 0x474);
    himm(0x114f0058, 0x474);
    himm(0x114f005C, 0x474);
#endif
}

void sensor_cfg_mux(void)
{
    //sensor  0
    himm(0x114F006C, 0x461);
    himm(0x114F0070, 0x671);

   //sensor 1
    himm (0x114F0060, 0x662);
    himm (0x114F0064, 0x672);
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

void vi_bt1120_mode_mux(void)
{
    himm(0x114F0010 ,0x401);
    himm(0x114F0014 ,0x401);
    himm(0x114F0018 ,0x401);
    himm(0x114F001C ,0x401);
    himm(0x114F0020 ,0x401);
    himm(0x114F0024 ,0x401);
    himm(0x114F0028 ,0x401);
    himm(0x114F002C ,0x401);
    himm(0x114F0040, 0x471);
    himm(0x114F0044, 0x471);
    himm(0x114F0048, 0x471);
    himm(0x114F004C, 0x471);
    himm(0x114F0050, 0x471);
    himm(0x114F0054, 0x471);
    himm(0x114F0058, 0x471);
    himm(0x114F005C, 0x471);
    himm(0x114F0060, 0x471);
    himm(0x114F0064, 0x471);
    himm(0x114F0068, 0x461);
}

void vi_bt656_mode_mux(void)
{
    himm(0x114F0040, 0x471);
    himm(0x114F0044, 0x471);
    himm(0x114F0048, 0x471);
    himm(0x114F004C, 0x471);
    himm(0x114F0050, 0x471);
    himm(0x114F0054, 0x471);
    himm(0x114F0058, 0x471);
    himm(0x114F005C, 0x471);
    himm(0x114F0068, 0x461);
    himm(0x120D4400, 0xff);
    himm(0x120D43fc, 0xff);
}

void vi_slave_mode_mux(void)
{
    himm(0x114F0060, 0x662);
    himm(0x114F0064, 0x672);
}

void vo_bt656_mode_mux(void)
{
    himm(0x114F0068, 0x462);
    himm(0x114F0040, 0x472);
    himm(0x114F0044, 0x472);
    himm(0x114F0048, 0x472);
    himm(0x114F004C, 0x472);
    himm(0x114F0050, 0x472);
    himm(0x114F0054, 0x472);
    himm(0x114F0058, 0x472);
    himm(0x114F005C, 0x472);
}

void mipi_tx_lcd_mux(void)
{
    himm(0x112F0074, 0x460);
    himm(0x112F0078, 0x460);
    himm(0x112F0088, 0x670);
    himm(0x112F0084, 0x670);
    himm(0x112F007C, 0x470);
    himm(0x112F0080, 0x470);
    himm(0x112F006C, 0x470);
    himm(0x112F0070, 0x470);
    himm(0x112F0068, 0x670);
    himm(0x112F0064, 0x670);
}

void mipi_tx_set_rest(void)
{
    himm(0x120D0080,0x20);
    himm(0x120D0400,0x20);
    himm(0x120D0080,0x00);
    himm(0x120D0080,0x20);
}

void hdmi_pin_mux(void)
{
    himm(0x114F0000, 0x431);
    himm(0x114F0004, 0x431);
    himm(0x114F0008, 0x631);
    himm(0x114F000C, 0x621);
}


void vo_6bit_lcd_mux(void)
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
}

void i2s0_pin_mux(void)
{
    himm(0x112F0020, 0x663);
    himm(0x112F0024, 0x673);
    himm(0x112F0028, 0x573);
    himm(0x112F002C, 0x473);
    himm(0x112F0030, 0x573);
}

/*Get sensor type from comm_sensor_intf_cfg_params.c*/
static void Load_Sensor_Type(void)
{
    HI_S32 as32SensorType[MAX_SENSOR_TYPE_NUM];

    MAPI_SENSOR_LoadSnsType(as32SensorType);
    if(as32SensorType[0] == IMX477)
    {
        sensor_type0 = "imx477";
    }
    else if(as32SensorType[0] == IMX377)
    {
        sensor_type0 = "imx377";
    }
    else if(as32SensorType[0] == IMX290)
    {
        sensor_type0 = "imx290";
    }
    else if(as32SensorType[0] == IMX458)
    {
        sensor_type0 = "imx458";
    }
    else
    {
        sensor_type0 = "unused";
    }


    if(as32SensorType[1] == IMX307)
    {
        sensor_type1 = "imx307";
    }
    else if(as32SensorType[1] == BT656)
    {
        sensor_type1 = "BT656";
        g_cmos_yuv_flag = 1;
    }
    else
    {
        sensor_type1 = "unused";
    }

}


int pinmux(void)
{
    Load_Sensor_Type(); // change g_cmos_yuv_flag value

    i2c0_pin_mux();
    i2c1_pin_mux();
    //i2c2_pin_mux();
    i2c3_pin_mux();
    //spi0_pin_mux();
    sensor_cfg_mux();
    motionsensor_cfg_mux();
    vi_mipi_rx_mux();
    vi_slave_mode_mux();
    //vo_bt656_mode_mux();
    mipi_tx_lcd_mux();
    mipi_tx_set_rest();

    if (1 == g_cmos_yuv_flag)
    {
        vi_bt1120_mode_mux();
    }
    else if (2 == g_cmos_yuv_flag)
    {
        vi_bt656_mode_mux();
    }
    else
    {}

    hdmi_pin_mux();
    //spi1_pin_mux();
    //vo_6bit_lcd_mux();
    //i2s0_pin_mux();
    return 0;
}

int clk_cfg(char *s)
{
    printf("\n==========chip: %s==========\n", s);

    himm(0x120101B8, 0x0007f81f);
    himm(0x120100F0, 0x00000861);
    himm(0x120100F8, 0x0000000f);

    if (   (0 == strncmp("hi3559v200", s, CHIP_NAME_LEN))
        || (0 == strncmp("hi3516dv300", s, CHIP_NAME_LEN)))
    {
        himm(0x120100A0, 0x00582c01);
        himm(0x120100F4, 0x24903ff);
        himm(0x12010100, 0x00f00492);
        himm(0x120100FC, 0x10);
    }
    else if ((0 == strncmp("hi3556v200", s, CHIP_NAME_LEN))
          || (0 == strncmp("hi3516cv500", s, CHIP_NAME_LEN)))
    {
        himm(0x120100A0, 0x00782cf1);
        himm(0x120100F4, 0x29203ff);
        himm(0x12010100, 0x00f00924);
        himm(0x120100FC, 0x12);
    }
    else
    {
        himm(0x120100A0, 0x00482c01);
        himm(0x120100F4, 0x026d83ff);
        himm(0x12010100, 0x00f006dB);
        himm(0x120100FC, 0x11);
    }

    himm(0x12030018, 0x540);
    himm(0x12030034, 0x1);
    himm(0x12030000, 0x0);
    himm(0x12010018, 0x12000000);
    himm(0x1201001C, 0x1002063);
    //himm(0x12010000+0x01BC,0x01802882);

    if (g_cmos_yuv_flag > 0)
    {
        himm(0x12030018, 0x560);
        himm(0x120100F4, 0x025d03ff);
        himm(0x120100FC, 0x30);
        himm(0x12010100, 0x00f00492);
    }

    return 0;
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

void sys_ctl(int online_flag)
{
    set_vi_workmode(online_flag);
    ddrc_config();
}

static int ampunmute(void)
{
    himm(0x111F0034, 0x431);
    himm(0x120DA3FC, 0x000000ff);
    himm(0x120DA400, 0x000000ff);
    himm(0x120DA3FC, 0x000000ff);

    return 0;
}


void sensor_config(void)
{
    Load_Sensor_Type();
    if (   (!strcmp(sensor_type0, "imx327"))
        || (!strcmp(sensor_type0, "imx335"))
        || (!strcmp(sensor_type0, "imx290")))
    {
        himm(0x120100F0, 0x861);
    }
    else if ((!strcmp(sensor_type0, "imx377"))
             || (!strcmp(sensor_type0, "imx458"))
             || (!strcmp(sensor_type0, "ov12870"))
             || (!strcmp(sensor_type0, "os05a"))
             || (!strcmp(sensor_type0, "os04b10"))
             || (!strcmp(sensor_type0, "os08a10")))
    {
        /*bt656 n3 use 27M*/
        himm(0x120100F0, 0xa51);
    }
    else if (!strcmp(sensor_type0, "gc2053"))
    {
        himm(0x120100F0, 0xa69);
    }
    else
    {
         himm(0x120100F0, 0x8a1);
    }
}

static HI_VOID SYS_cfg(void)
{
    pinmux();
    clk_cfg(chip_list);
    sys_ctl(g_online_flag);
    ampunmute();
    sensor_config();
}

#ifdef CONFIG_HI_TZASC_SUPPORT
static HI_S32 tzasc_init(char mmzparam[])
{
    tzasc_region_attr anonymous_region, svp_region, share_region;
    tzasc_handle hdl_annoymous = tzasc_invalid_handle;
    tzasc_handle hdl_svp = tzasc_invalid_handle;
    tzasc_handle hdl_share = tzasc_invalid_handle;
    int ret;

    ret = tzasc_mod_init();
    if (ret)
    {
        printf("tzasc mod init fail. \n");
        goto tzasc_init_err;
    }

    strncpy(anonymous_region.name, "anonymous", TZASC_REGION_NAME_LENGTH);
    anonymous_region.phystart = 0x52000000;
        anonymous_region.length = 512;
    anonymous_region.type = tzasc_anonymous_type;
    hdl_annoymous = tzasc_init_one_region(&anonymous_region);
    if (hdl_annoymous == tzasc_invalid_handle)
    {
        dprintf("init anonymous region fail. \n");
        ret = -1;
        goto tzasc_init_err;
    }
    mmzparam = tzasc_concatenate_mmz_string(hdl_annoymous, mmzparam);

    strncpy(svp_region.name, "svp", TZASC_REGION_NAME_LENGTH);
    svp_region.phystart = 0x72000000;
        svp_region.length = 256;
    svp_region.type = tzasc_svp_type;
    hdl_svp = tzasc_init_one_region(&svp_region);
    if (hdl_svp == tzasc_invalid_handle)
    {
        dprintf("init svp region fail. \n");
        ret = -1;
        goto tzasc_init_err;
    }
    mmzparam = tzasc_concatenate_mmz_string(hdl_svp, mmzparam);

    strncpy(share_region.name, "share", TZASC_REGION_NAME_LENGTH);
    share_region.phystart = 0x82000000;
    share_region.length = 224;
    share_region.type = tzasc_share_type;
    hdl_share = tzasc_init_one_region(&share_region);
    if (hdl_share == tzasc_invalid_handle)
    {
        dprintf("init share region fail. \n");
        ret = -1;
        goto tzasc_init_err;
    }
    mmzparam = tzasc_concatenate_mmz_string(hdl_share, mmzparam);

    return 0;

tzasc_init_err:
    if (hdl_annoymous)
    {
        tzasc_exit_one_region(hdl_annoymous);
    }
    if (hdl_svp)
    {
        tzasc_exit_one_region(hdl_svp);
    }
    if (hdl_share)
    {
        tzasc_exit_one_region(hdl_share);
    }
    return ret;
}
#endif

static HI_S32 MMZ_init(void)
{
    extern int media_mem_init(void * pArgs);
    MMZ_MODULE_PARAMS_S stMMZ_Param = {0};

#ifdef CONFIG_HI_TZASC_SUPPORT
    int ret;

    ret = tzasc_init(stMMZ_Param.mmz);
    if (ret)
    {
        dprintf("tzasc init err. \n");
        return ret;
    }
#else
    snprintf(stMMZ_Param.mmz, MMZ_SETUP_CMDLINE_LEN, "anonymous,0,0x%llx,%dM", mmz_start, mmz_size);
#endif

    snprintf(stMMZ_Param.map_mmz, MMZ_SETUP_CMDLINE_LEN, "0x9d000000,48M");

    stMMZ_Param.anony = 1;

    dprintf("mmz_start=0x%llx, mmz_size=0x%x\n", mmz_start, mmz_size);
    dprintf("mmz param= %s\n", stMMZ_Param.mmz);

    return media_mem_init(&stMMZ_Param);
}

static HI_S32 BASE_init(void)
{
    HI_U32 vb_force_exit;
    vb_force_exit = 0;

    return base_mod_init();
}

static HI_S32 SYS_init(void)
{
    return sys_mod_init();
}

static HI_S32 RGN_init(void)
{
    return rgn_mod_init();
}

static HI_S32 ISP_init(void)
{
    return isp_mod_init();
}

static HI_S32 VI_init(void)
{
    return vi_mod_init();
}

static HI_S32 DIS_init(void)
{
    return dis_mod_init();
}

#ifdef SUPPORT_GYRO
static HI_S32 Gyro_Dis_init(void)
{
    extern int gyrodis_mod_init(void);
    return gyrodis_mod_init();
}

static HI_S32 MOTIONFUSION_init(void)
{
    extern int motionfusion_mod_init(void);
    return motionfusion_mod_init();
}

static HI_S32 MotionSensorMngInit(void)
{
    extern HI_S32 motionsensor_init(HI_VOID);
    return motionsensor_init();
}

static HI_S32 MotionSensorChipInit(void)
{
    extern HI_S32 MotionSensorInit(void);
    return MotionSensorInit();
}
#endif

static HI_S32 GDC_init(void)
{
    GDC_MODULE_PARAMS_S stGdcModuleParam;
    stGdcModuleParam.u32MaxGdcJob   = 32;
    stGdcModuleParam.u32MaxGdcTask  = 64;
    stGdcModuleParam.u32MaxGdcNode  = 64;
    stGdcModuleParam.au32GdcEn[0]    = 1;

    return gdc_mod_init(&stGdcModuleParam);
}

static HI_S32 VGS_init(void)
{
    VGS_MODULE_PARAMS_S  stVgsModuleParam;
    stVgsModuleParam.u32MaxVgsJob  = 64;
    stVgsModuleParam.u32MaxVgsTask = 100;
    stVgsModuleParam.u32MaxVgsNode = 100;
    stVgsModuleParam.au32VgsEn[0]  = 1;
    stVgsModuleParam.bVgsHdrSupport = 0;

    return vgs_mod_init(&stVgsModuleParam);
}

static HI_S32 VPSS_init(void)
{
    return vpss_mod_init(NULL);
}

static HI_S32 VO_init(void)
{
    return vou_mod_init();
}

static HI_S32 RC_init(void)
{
    return rc_mod_init();
}

static HI_S32 VENC_init(void)
{
    return venc_mod_init(NULL);
}

static HI_S32 CHNL_init(void)
{
    return chnl_mod_init();
}

static HI_S32 VEDU_init(void)
{
    return vedu_mod_init();
}

static HI_S32 H264e_init(void)
{
    return h264e_mod_init();
}

static HI_S32 H265e_init(void)
{
    return h265e_mod_init();
}

static HI_S32 JPEGE_init(void)
{
    return jpege_mod_init();
}

static HI_S32 JPEGD_init(void)
{
    return jpegd_mod_init();
}

static HI_S32 VFMW_init(void)
{
    return vfmw_mod_init();
}

static HI_S32 VDEC_init(void)
{
    return vdec_mod_init();
}

static HI_S32 hi_sensor_spi_init(void)
{
    return sensor_spi_dev_init();
}

static HI_S32 hi_sensor_i2c_init(void)
{
    return hi_dev_init();
}

static HI_S32 PWM_init(void)
{
    return pwm_init();
}

static HI_S32 sil9136_init(void)
{
    return sil902x_init();
}

static HI_S32 SSP_st7789_init(void)
{
    extern hi_ssp_lcd_st7789_init(void);
    return hi_ssp_lcd_st7789_init();
}

static HI_S32 HDMI_init(void)
{
    return HDMI_DRV_ModInit();
}

static HI_S32 MIPI_RX_init(void)
{
    return mipi_rx_mod_init();
}

static HI_S32 MIPI_TX_init(void)
{
    return mipi_tx_module_init();
}

static HI_S32 AcodecMod_init(void)
{
    return acodec_mod_init(NULL);
}

static HI_S32 AiaoMod_init(void)
{
    return aiao_mod_init();
}

static HI_S32 AiMod_init(void)
{
    return ai_mod_init();
}

static HI_S32 AoMod_init(void)
{
    return ao_mod_init();
}

static HI_S32 AencMod_init(void)
{
    return aenc_mod_init();
}

static HI_S32 AdecMod_init(void)
{
    return adec_mod_init();
}

static HI_S32 Cipher_init(void)
{
    extern int CIPHER_DRV_ModInit(void);
    return CIPHER_DRV_ModInit();
}

static void insert_audio(void)
{
    HI_S32 ret = 0;

    ret = AiaoMod_init();
    if (ret != 0)
    {
        printf("aiao init error.\n");
    }
    ret = AiMod_init();
    if (ret != 0)
    {
        printf("ai init error.\n");
    }
    ret = AoMod_init();
    if (ret != 0)
    {
        printf("ao init error.\n");
    }
    ret = AencMod_init();
    if (ret != 0)
    {
        printf("aenc init error.\n");
    }
    ret = AdecMod_init();
    if (ret != 0)
    {
        printf("adec init error.\n");
    }

    ret = AcodecMod_init();
    if (ret != 0)
    {
        printf("acodec init error.\n");
    }
}
extern void osal_proc_init(void);

HI_VOID Load_sdk(void)
{
    HI_S32        ret = 0;

    SYS_cfg();

    osal_proc_init();
    ret = MMZ_init();
    if (ret != 0)
    {
        printf("MMZ init error.\n");
    }

    ret = BASE_init();
    if (ret != 0)
    {
        printf("base init error.\n");
    }

    ret = SYS_init();
    if (ret != 0)
    {
        printf("sys init error.\n");
    }

    ret = RGN_init();
    if (ret != 0)
    {
        printf("rgn init error.\n");
    }

    ret = GDC_init();
    if (ret != 0)
    {
        printf("gdc init error.\n");
    }

    ret = VGS_init();
    if (ret != 0)
    {
        printf("vgs init error.\n");
    }

    ret = DIS_init();
    if (ret != 0)
    {
        printf("vgs init error.\n");
    }

#ifdef SUPPORT_GYRO
    ret = Gyro_Dis_init();
    if(ret != 0)
    {
        printf("gyro dis init error");
    }

    ret = MOTIONFUSION_init();
    if(ret != 0)
    {
        printf("motionfusion init error");
    }

    ret = MotionSensorMngInit();
    if(ret != 0)
    {
        printf("motion sensor mng init error.\n");
    }

    ret = MotionSensorChipInit();
    if(ret != 0)
    {
        printf("motion sensor chip init error.\n");
    }
#endif

    ret = VI_init();
    if (ret != 0)
    {
        printf("vi init error.\n");
    }

    ret = ISP_init();
    if (ret != 0)
    {
        printf("isp init error.\n");
    }

    ret = VPSS_init();
    if(ret != 0)
    {
        printf("vpss init error.\n");
    }

    ret = VO_init();
    if (ret != 0)
    {
        printf("vo init error.\n");
    }

    ret = CHNL_init();
    if (ret != 0)
    {
        printf("chnl init error.\n");
    }

    ret = VEDU_init();
    if (ret != 0)
    {
        printf("vedu init error.\n");
    }

    ret = RC_init();
    if (ret != 0)
    {
        printf("rc init error.\n");
    }

    ret = VENC_init();
    if (ret != 0)
    {
        printf("venc init error.\n");
    }

    ret = H264e_init();
    if (ret != 0)
    {
        printf("H264e init error.\n");
    }

    ret = H265e_init();
    if (ret != 0)
    {
        printf("H265e init error.\n");
    }

    ret = JPEGE_init();
    if (ret != 0)
    {
        printf("jpege init error.\n");
    }

    ret = JPEGD_init();
    if (ret != 0)
    {
        printf("jpegd init error.\n");
    }

    ret = VFMW_init();
    if (ret != 0)
    {
        printf("vfmw init error.\n");
    }
    ret = VDEC_init();
    if (ret != 0)
    {
        printf("vdec init error.\n");
    }

    insert_audio();

    ret = PWM_init();
    if(ret != 0)
    {
        printf("pwm init error.\n");
    }


#ifdef LOSCFG_DRIVERS_SPI
    dprintf("spi bus init ...\n");
    extern int spi_dev_init(void);
    spi_dev_init();
#endif

#ifdef LOSCFG_DRIVERS_I2C
    dprintf("i2c bus init ...\n");
    extern int i2c_dev_init(void);
    i2c_dev_init();
#endif

    ret = hi_sensor_i2c_init();
    if(ret != 0)
    {
        printf("sensor i2c init error.\n");
    }

#if 0
    ret = hi_sensor_spi_init();
    if(ret != 0)
    {
        printf("sensor spi init error.\n");
    }

    ret = sil9136_init();
    if(ret != 0)
    {
        printf("sil9136 init error.\n");
    }
#endif

    ret = HDMI_init();
    if(ret != 0)
    {
        printf("hdmi init error.\n");
    }

    ret = SSP_st7789_init();
    if(ret != 0)
    {
        printf("SSP_st7789 init error.\n");
    }

    ret = MIPI_RX_init();
    if (ret != 0)
    {
        printf("mipi_rx init error.\n");
    }

#if 1
    ret = MIPI_TX_init();
    if (ret != 0)
    {
        printf("mipi_tx init error.\n");
    }
#endif

#if CONFIG_HI_SUBCHIP_TYPE==HI3516C_V500
    ret = Cipher_init();
    if (ret != 0)
    {
        printf("cipher init error.\n");
    }
#endif

    printf("SDK init ok...\n");
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
