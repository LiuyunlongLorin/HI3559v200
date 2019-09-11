/******************************************************************************
  Some simple Hisilicon Hi3516A system functions.

  Copyright (C), 2017-2018, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2018-1 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include "hi_type.h"

#include "osal_mmz.h"

static HI_VOID BASE_exit(void)
{
    base_mod_exit();
}

static HI_VOID MMZ_exit(void)
{
    extern void media_mem_exit(void);
    media_mem_exit();
#ifdef TEE_VERSION
    extern void tzasc_mod_exit(void);
    tzasc_mod_exit();
#endif
}
static HI_VOID SYS_exit(void)
{
    sys_mod_exit();
}

static HI_S32 ISP_exit(void)
{
    return isp_mod_exit();
}

static HI_S32 VI_exit(void)
{
    return vi_mod_exit();
}

static HI_S32 RGN_exit(void)
{
    return rgn_mod_exit();
}

static HI_S32 GDC_exit(void)
{
    return gdc_mod_exit();
}

static HI_S32 DIS_exit(void)
{
    return vi_mod_exit();
}

static HI_S32 VGS_exit(void)
{
    return vgs_mod_exit();
}

static HI_S32 VPSS_exit(void)
{
    return vpss_mod_exit();
}

static HI_S32 VO_exit(void)
{
    return vou_mod_exit();
}

//NO tde&hifb on LiteOS
#if 0
static HI_S32 TDE_exit(void)
{
    return tde_mod_exit();
}

static HI_VOID HIFB_exit(void)
{
    extern HI_VOID hifb_cleanup(HI_VOID);

    hifb_cleanup();
}
#endif

static HI_S32 HDMI_exit(void)
{
    return HDMI_DRV_ModExit();
}

static HI_VOID MIPIRX_exit(void)
{
    mipi_rx_mod_exit();
}

static HI_VOID MIPITX_exit(void)
{
    mipi_tx_module_exit();
}

static HI_S32 RC_exit(void)
{
    return rc_mod_exit();
}

static HI_S32 VENC_exit(void)
{
    return venc_mod_exit(NULL);
}

static HI_S32 CHNL_exit(void)
{
    return chnl_mod_exit();
}

static HI_S32 VEDU_exit(void)
{
    return vedu_mod_exit();
}

static HI_S32 H264e_exit(void)
{
    return h264e_mod_exit();
}

static HI_S32 H265e_exit(void)
{
    return h265e_mod_exit();
}

static HI_S32 JPEGE_exit(void)
{
    return jpege_mod_exit();
}

static HI_S32 PWM_exit(void)
{
    return pwm_exit();
}

static HI_S32 hi_sensor_spi_exit(void)
{
    return sensor_spi_dev_exit();
}

static HI_S32 hi_sensor_i2c_exit(void)
{
    return hi_dev_exit();
}

static HI_S32 JPEGD_exit(void)
{
    return jpegd_mod_exit();
}

static HI_S32 VFMW_exit(void)
{
    return vfmw_mod_exit();
}

static HI_S32 SSP_st7789_exit(void)
{
    extern hi_ssp_lcd_st7789_exit(void);
    return hi_ssp_lcd_st7789_exit();
}

static HI_S32 VDEC_exit(void)
{
    return vdec_mod_exit();
}

static void Cipher_exit(void)
{
    extern void CIPHER_DRV_ModExit(void);
    CIPHER_DRV_ModExit();
}

extern void osal_proc_exit(void);

HI_VOID SDK_exit(void)
{
#if CONFIG_HI_SUBCHIP_TYPE==HI3516C_V500
    Cipher_exit();
#endif
//    MIPITX_exit();
    MIPIRX_exit();
    SSP_st7789_exit();

    JPEGD_exit();
    VFMW_exit();

    VDEC_exit();

    RC_exit();
    JPEGE_exit();
    H264e_exit();
    H265e_exit();
    VEDU_exit();
    CHNL_exit();
    VENC_exit();

    VO_exit();
    VPSS_exit();
    ISP_exit();
    VI_exit();
    GDC_exit();
    DIS_exit();
    VGS_exit();
    RGN_exit();

    HDMI_exit();
    hi_sensor_i2c_exit();
//    hi_sensor_spi_exit();
    PWM_exit();

    SYS_exit();
    BASE_exit();
    MMZ_exit();
    osal_proc_exit();

    printf("SDK exit ok...\n");
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


