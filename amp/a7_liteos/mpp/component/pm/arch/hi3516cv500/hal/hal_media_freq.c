/******************************************************************************
 Copyright (C), 2018, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : hal_media_freq.c
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2018/10/18
Last Modified :
Description   :
Function List :
******************************************************************************/
#include "hi_osal.h"
#include "hi_math.h"
#include "hi_debug.h"

#include "hal_media_frq.h"
#include "pm_drv_comm.h"

//==========================


#define CRG_ADDR_MEDIA1 0x1201004c
#define CRG_ADDR_MEDIA2 0x12010054
#define CRG_ADDR_MEDIA3 0x12010044
#define CRG_ADDR_MEDIA4 0x12010080

const HI_MEDIA_FREQ_S media_profile[3] = { //max freq
    {
        .enVI[0]  = VI_F300M,
        .enPipeBE = PIPEBE_F300M,
        .enVPSS = VPSS_F300M,
        .enVEDU = VEDU_F396M,
        .enJPGE = JPGE_F400M,
        .enVDP = VDP_F198M,
        .enVGS = VGS_F500M,
        .enGDC = GDC_F475M,
        .enTDE = TDE_F150M,
        //       .enAIAO = AIAO_F1188M,
        //      .enDDR = DDR_F1600M,
    },
    {
        .enVI[0]  = VI_F396M,
        .enPipeBE = PIPEBE_F300M,
        .enVPSS = VPSS_F300M,
        .enVEDU = VEDU_F396M,
        .enJPGE = JPGE_F400M,
        .enVDP = VDP_F198M,
        .enVGS = VGS_F500M,
        .enGDC = GDC_F475M,
        .enTDE = TDE_F150M,
        //       .enAIAO = AIAO_F1188M,
        //      .enDDR = DDR_F1600M,
    },
    {
        .enVI[0]  = VI_F600M,
        .enPipeBE = PIPEBE_F340M,
        .enVPSS = VPSS_F300M,
        .enVEDU = VEDU_F396M,
        .enJPGE = JPGE_F400M,
        .enVDP = VDP_F198M,
        .enVGS = VGS_F500M,
        .enGDC = GDC_F475M,
        .enTDE = TDE_F150M,
        //       .enAIAO = AIAO_F1188M,
        //      .enDDR = DDR_F1600M,
    },
};


/*
HI_S32 MediaPowerManage_init(HI_VOID)
{

    return HI_SUCCESS;
}
*/

#define pm_readl(x) (*((volatile int *)(x)))
#define pm_writel(v, x) (*((volatile int *)(x)) = (v))

static inline void HI_RegWrite32(HI_U32 value, HI_U32 mask,
                                 HI_U32 addr)
{
    HI_U32 t;
    t = pm_readl((const volatile void*)addr);
    t &= ~mask;
    t |= value & mask;
    pm_writel(t, (volatile void*)addr);
}

static inline void HI_RegRead(HI_U32* pvalue, HI_U32 addr)
{
    *pvalue = pm_readl((const volatile void*)addr);
}


HI_S32 PM_HAL_GetMiscPolicy(void)
{
    return  (HI_S32)HI_PM_MISC_POLICY_NONE;
}

/*----------------media freq config--------------------------------*/

static HI_S32 PM_HAL_SetVicapFreq(const HI_VI_FREQ_E enViFreq)
{
    switch (enViFreq) {
        case VI_F214M:
            HI_RegWrite32(0, 0x7, CRG_ADDR_MEDIA1);
            break;
        case VI_F300M:
            HI_RegWrite32(1, 0x7, CRG_ADDR_MEDIA1);
            break;
        case VI_F340M:
            HI_RegWrite32(4, 0x7, CRG_ADDR_MEDIA1);
            break;
        case VI_F396M:
            HI_RegWrite32(5, 0x7, CRG_ADDR_MEDIA1);
            break;
        case VI_F500M:
            HI_RegWrite32(2, 0x7, CRG_ADDR_MEDIA1);
            break;
        case VI_F600M:
            HI_RegWrite32(3, 0x7, CRG_ADDR_MEDIA1);
            break;
        default:
            osal_printk( "Set VI0 freq %d error.\n", enViFreq);
            return MPP_EN_PM_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetVicapFreq(HI_VI_FREQ_E* penViFreq)
{
    HI_U32 u32VI0Freq;
    HI_RegRead(&u32VI0Freq, CRG_ADDR_MEDIA1);
    u32VI0Freq &= 0x7;
    switch (u32VI0Freq) {
        case 0:
            *penViFreq = VI_F214M;
            break;
        case 1:
            *penViFreq = VI_F300M;
            break;
        case 4:
            *penViFreq = VI_F340M;
            break;
        case 5:
            *penViFreq = VI_F396M;
            break;
        case 2:
            *penViFreq = VI_F500M;
            break;
        case 3:
            *penViFreq = VI_F600M;
            break;
        default:
            osal_printk( "Get VI0 freq %d error.\n", u32VI0Freq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}


static HI_S32 PM_HAL_SetViPipeBeFreq(HI_PIPEBE_FREQ_E enPipeBEFreq)
{
    switch (enPipeBEFreq) {
        case PIPEBE_F214M:
            HI_RegWrite32(0 << 14, 0x1f << 14, CRG_ADDR_MEDIA2);
            break;
        case PIPEBE_F300M:
            HI_RegWrite32(1 << 14, 0x1f << 14, CRG_ADDR_MEDIA2);
            break;
        case PIPEBE_F340M:
            HI_RegWrite32(2 << 14, 0x1f << 14, CRG_ADDR_MEDIA2);
            break;
        case PIPEBE_FVI:
            HI_RegWrite32(8 << 14, 0x1f << 14, CRG_ADDR_MEDIA2);
            break;
        /*    case PIPEBE_FVI_DIV2:
                 HI_RegWrite32(16 << 14, 0x1f << 14, CRG_ADDR_MEDIA2);
                 break;*/
        default:
            osal_printk( "Set PIPEBE0 freq %d error.\n", enPipeBEFreq);
            return MPP_EN_PM_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetViPipeBeFreq(HI_PIPEBE_FREQ_E* penPipeBEFreq)
{
    HI_U32 u32PIPEBE0Freq;
    HI_RegRead(&u32PIPEBE0Freq, CRG_ADDR_MEDIA2);
    u32PIPEBE0Freq &= (0x1f << 14);
    u32PIPEBE0Freq >>= 14;
    switch (u32PIPEBE0Freq) {
        case 0:
            *penPipeBEFreq = PIPEBE_F214M;
            break;
        case 1:
            *penPipeBEFreq = PIPEBE_F300M;
            break;
        case 2:
            *penPipeBEFreq = PIPEBE_F340M;
            break;
        case 8:
            *penPipeBEFreq = PIPEBE_FVI;
            break;
        /*    case 16:
                 *penPipeBEFreq = PIPEBE_FVI_DIV2;
                 break;*/
        default:
            osal_printk( "Get PIPEBE0 freq %d error.\n", u32PIPEBE0Freq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_SetVpssFreq(HI_VPSS_FREQ_E enVpssFreq)
{
    switch (enVpssFreq) {
        case VPSS_F214M:
            HI_RegWrite32(0 << 5, 0x7 << 5, CRG_ADDR_MEDIA1);
            break;
        case VPSS_F300M:
            HI_RegWrite32(1 << 5, 0x7 << 5, CRG_ADDR_MEDIA1);
            break;
        case VPSS_F396M:
            HI_RegWrite32(2 << 5, 0x7 << 5, CRG_ADDR_MEDIA1);
            break;
        default:
            osal_printk( "Set VPSS freq %d error.\n", enVpssFreq);
            return MPP_EN_PM_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetVpssFreq(HI_VPSS_FREQ_E* penVpssFreq)
{
    HI_U32 u32VpssFreq;
    HI_RegRead(&u32VpssFreq, CRG_ADDR_MEDIA1);
    u32VpssFreq &= (0x7 << 5);
    u32VpssFreq >>= 5;
    switch (u32VpssFreq) {
        case 0:
            *penVpssFreq = VPSS_F214M;
            break;
        case 1:
            *penVpssFreq = VPSS_F300M;
            break;
        case 2:
            *penVpssFreq = VPSS_F396M;
            break;
        default:
            osal_printk( "Get VPSS freq %d error.\n", u32VpssFreq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}


static HI_S32 PM_HAL_SetVeduFreq(HI_VEDU_FREQ_E enVeduFreq)
{
    switch (enVeduFreq) {
        case VEDU_F396M:
            HI_RegWrite32(1 << 10, 0x7 << 10, CRG_ADDR_MEDIA1);
            break;
        case VEDU_F500M:
            HI_RegWrite32(2 << 10, 0x7 << 10, CRG_ADDR_MEDIA1);
            break;
        case VEDU_F594M:
            HI_RegWrite32(3 << 10, 0x7 << 10, CRG_ADDR_MEDIA1);
            break;
        default:
            osal_printk( "Set VEDU freq %d error.\n", enVeduFreq);
            return MPP_EN_PM_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetVeduFreq(HI_VEDU_FREQ_E* penVeduFreq)
{
    HI_U32 u32VeduFreq;
    HI_RegRead(&u32VeduFreq, CRG_ADDR_MEDIA1);
    u32VeduFreq &= (0x7 << 10);
    u32VeduFreq >>= 10;
    switch (u32VeduFreq) {
        case 1:
            *penVeduFreq = VEDU_F396M;
            break;
        case 2:
            *penVeduFreq = VEDU_F500M;
            break;
        case 3:
            *penVeduFreq = VEDU_F594M;
            break;
        default:
            osal_printk( "Get VEDU freq %d error.\n", u32VeduFreq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_SetJpgeFreq(HI_JPGE_FREQ_E enJpgeFreq)
{
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetJpgeFreq(HI_JPGE_FREQ_E* penJpgeFreq)
{
    *penJpgeFreq = JPGE_F400M;
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetVgsFreq(HI_VGS_FREQ_E* penVgsFreq)
{
    HI_U32 u32VgsFreq;
    HI_RegRead(&u32VgsFreq, CRG_ADDR_MEDIA1);
    u32VgsFreq &= (0x7 << 13);
    u32VgsFreq >>= 13;
    switch (u32VgsFreq) {
        case 0:
            *penVgsFreq = VGS_F300M;
            break;
        case 1:
            *penVgsFreq = VGS_F396M;
            break;
        case 2:
            *penVgsFreq = VGS_F500M;
            break;
        default:
            osal_printk( "Get VGS freq %d error.\n", u32VgsFreq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}


static HI_S32 PM_HAL_SetVgsFreq(HI_VGS_FREQ_E enVgsFreq)
{
    switch (enVgsFreq) {
        case VGS_F300M:
            HI_RegWrite32(0 << 13, 0x7 << 13, CRG_ADDR_MEDIA1);
            break;
        case VGS_F396M:
            HI_RegWrite32(1 << 13, 0x7 << 13, CRG_ADDR_MEDIA1);
            break;
        case VGS_F500M:
            HI_RegWrite32(2 << 13, 0x7 << 13, CRG_ADDR_MEDIA1);
            break;
        default:
            osal_printk( "Set VGS freq %d error.\n", enVgsFreq);
            return MPP_EN_PM_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}


static HI_S32 PM_HAL_GetVdecFreq(HI_VDH_FREQ_E* penVdhFreq)
{
    HI_U32 u32VdhFreq;
    HI_RegRead(&u32VdhFreq, CRG_ADDR_MEDIA2);
    u32VdhFreq &= (0x1f << 6);
    u32VdhFreq >>= 6;
    switch (u32VdhFreq) {
        case 0:
            *penVdhFreq = VDH_F396M;
            break;
        case 1:
            *penVdhFreq = VDH_F500M;
            break;
        case 8:
            *penVdhFreq = VDH_F594M;
            break;
        default:
            osal_printk( "Get PIPEBE1 freq %d error.\n", u32VdhFreq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_SetVdecFreq(HI_VDH_FREQ_E enVdhFreq)
{
    switch (enVdhFreq) {
        case VDH_F396M:
            HI_RegWrite32(0 << 13, 0x7 << 13, CRG_ADDR_MEDIA1);
            break;
        case VDH_F500M:
            HI_RegWrite32(1 << 13, 0x7 << 13, CRG_ADDR_MEDIA1);
            break;
        case VDH_F594M:
            HI_RegWrite32(2 << 13, 0x7 << 13, CRG_ADDR_MEDIA1);
            break;
        default:
            osal_printk( "Set VDH freq %d error.\n", enVdhFreq);
            return MPP_EN_PM_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_SetVdpFreq(HI_VDP_FREQ_E enVdpFreq)
{
    switch (enVdpFreq) {
        case VDP_F198M:
            HI_RegWrite32(1 << 17, 0x1 << 17, CRG_ADDR_MEDIA3);
            break;
        case VDP_F300M:
            HI_RegWrite32(0 << 17, 0x1 << 17, CRG_ADDR_MEDIA3);
            break;
        default:
            osal_printk( "Set VDP freq %d error.\n", enVdpFreq);
            return MPP_EN_PM_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetVdpFreq(HI_VDP_FREQ_E* penVdpFreq)
{
    HI_U32 u32VdpFreq;
    HI_RegRead(&u32VdpFreq, CRG_ADDR_MEDIA3);
    u32VdpFreq &= (0x1 << 17);
    u32VdpFreq >>= 17;
    switch (u32VdpFreq) {
        case 1:
            *penVdpFreq = VDP_F198M;
            break;
        case 0:
            *penVdpFreq = VDP_F300M;
            break;
        default:
            osal_printk( "Get VDP freq %d error.\n", u32VdpFreq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetAvspFreq(HI_AVSP_FREQ_E* penAvspFreq)
{
    HI_U32 u32AvspFreq;
    HI_RegRead(&u32AvspFreq, CRG_ADDR_MEDIA1);
    u32AvspFreq &= (0x7 << 13);
    u32AvspFreq >>= 13;
    switch (u32AvspFreq) {
        case 0:
            *penAvspFreq = AVSP_F300M;
            break;
        default:
            osal_printk( "Get VGS freq %d error.\n", u32AvspFreq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_SetAvspFreq(HI_AVSP_FREQ_E enAvspFreq)
{
    switch (enAvspFreq) {
        case AVSP_F300M:
            HI_RegWrite32(0 << 16, 0x7 << 16, CRG_ADDR_MEDIA1);
            break;
        default:
            osal_printk( "Set AVSP freq %d error.\n", enAvspFreq);
            return MPP_EN_PM_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_SetGdcFreq(HI_GDC_FREQ_E enGdcFreq)
{
    switch (enGdcFreq) {
        case GDC_F340M:
            HI_RegWrite32(0 << 16, 0x7 << 16, CRG_ADDR_MEDIA1);
            break;
        case GDC_F400M:
            HI_RegWrite32(2 << 16, 0x7 << 16, CRG_ADDR_MEDIA1);
            break;
        case GDC_F475M:
            HI_RegWrite32(1 << 16, 0x7 << 16, CRG_ADDR_MEDIA1);
            break;
        default:
            osal_printk( "Set GDC freq %d error.\n", enGdcFreq);
            return MPP_EN_PM_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetGdcFreq(HI_GDC_FREQ_E* penGdcFreq)
{
    HI_U32 u32GdcFreq;
    HI_RegRead(&u32GdcFreq, CRG_ADDR_MEDIA1);
    u32GdcFreq &= (0x7 << 16);
    u32GdcFreq >>= 16;
    switch (u32GdcFreq) {
        case 0:
            *penGdcFreq = GDC_F340M;
            break;
        case 2:
            *penGdcFreq = GDC_F400M;
            break;
        case 1:
            *penGdcFreq = GDC_F475M;
            break;
        default:
            osal_printk( "Get GDC freq %d error.\n", u32GdcFreq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_SetTdeFreq(HI_TDE_FREQ_E enTdeFreq)
{
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetTdeFreq(HI_TDE_FREQ_E* penTdeFreq)
{
    *penTdeFreq = TDE_F150M;
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_SetAiAoFreq(HI_AIAO_FREQ_E enAiaoFreq)
{
    switch (enAiaoFreq) {
        case AIAO_F1188M:
            HI_RegWrite32(0 << 2, 0x3 << 2, CRG_ADDR_MEDIA4);
            break;
        case AIAO_F1500M:
            HI_RegWrite32(1 << 2, 0x3 << 2, CRG_ADDR_MEDIA4);
            break;
        default:
            osal_printk( "Set AIAO freq %d error.\n", enAiaoFreq);
            return MPP_EN_PM_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

static HI_S32 PM_HAL_GetAiAoFreq(HI_AIAO_FREQ_E* penAiaoFreq)
{
    HI_U32 u32AiaoFreq;
    HI_RegRead(&u32AiaoFreq, CRG_ADDR_MEDIA4);
    u32AiaoFreq &= (0x3 << 2);
    u32AiaoFreq >>= 2;
    switch (u32AiaoFreq) {
        case 0:
            *penAiaoFreq = AIAO_F1188M;
            break;
        case 1:
            *penAiaoFreq = AIAO_F1500M;
            break;
        default:
            osal_printk( "Get AIAO freq %d error.\n", u32AiaoFreq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/*----------------DDR freq config----------------------------*/

#if 0
static HI_S32 PM_HAL_GetDdrFreq(HI_DDR_FREQ_E* penDdrFreq)
{
    HI_U32 u32DdrFreq;
    HI_RegRead(&u32DdrFreq, 0x12010014);    //This is not entirely proper.
    u32DdrFreq &= 0xFFF;
    switch (u32DdrFreq) {
        case 0x42:
            *penDdrFreq = DDR_F1600M;
            break;
        case 0x4d:
            *penDdrFreq = DDR_F1866M;
            break;
        default:
            osal_printk( "Get DDR freq %d maybe is error.\n", u32DdrFreq);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}
#endif


HI_S32 SetMediaFreqReg(const HI_MEDIA_FREQ_S* pstMediaFreq)
{
    HI_S32 i = 0;
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        PM_HAL_SetVicapFreq(pstMediaFreq->enVI[0]);
    }
    PM_HAL_SetViPipeBeFreq(pstMediaFreq->enPipeBE);
    PM_HAL_SetVgsFreq(pstMediaFreq->enVGS);
    PM_HAL_SetGdcFreq(pstMediaFreq->enGDC);
    PM_HAL_SetVpssFreq(pstMediaFreq->enVPSS);
    PM_HAL_SetVeduFreq(pstMediaFreq->enVEDU);
    PM_HAL_SetVdecFreq(pstMediaFreq->enVDH);
    PM_HAL_SetJpgeFreq(pstMediaFreq->enJPGE);
    PM_HAL_SetVdpFreq(pstMediaFreq->enVDP);
    PM_HAL_SetAvspFreq(pstMediaFreq->enAVSP);
    PM_HAL_SetTdeFreq(pstMediaFreq->enTDE);
    PM_HAL_SetAiAoFreq(pstMediaFreq->enAIAO);
    return HI_SUCCESS;
}

HI_S32 GetMediaFreqReg(HI_MEDIA_FREQ_S*  pstMediaFreq)
{
    HI_S32 i = 0;
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        PM_HAL_GetVicapFreq(&pstMediaFreq->enVI[i]);
    }
    PM_HAL_GetViPipeBeFreq(&pstMediaFreq->enPipeBE);
    PM_HAL_GetVpssFreq(&pstMediaFreq->enVPSS);
    PM_HAL_GetVeduFreq(&pstMediaFreq->enVEDU);
    PM_HAL_GetVdecFreq(&pstMediaFreq->enVDH);
    PM_HAL_GetVdecFreq(&pstMediaFreq->enVDH);
    PM_HAL_GetJpgeFreq(&pstMediaFreq->enJPGE);
    PM_HAL_GetVgsFreq(&pstMediaFreq->enVGS);
    PM_HAL_GetGdcFreq(&pstMediaFreq->enGDC);
    PM_HAL_GetAvspFreq(&pstMediaFreq->enAVSP);
    PM_HAL_GetVdpFreq(&pstMediaFreq->enVDP);
    PM_HAL_GetTdeFreq(&pstMediaFreq->enTDE);
    PM_HAL_GetAiAoFreq(&pstMediaFreq->enAIAO);
    return HI_SUCCESS;
}


HI_S32 PM_HAL_GetMediaFreqByUsrCfg(const HI_MPI_PM_MEDIA_CFG_S* pstPmParam, HI_MEDIA_FREQ_S* pstMediaFreq)
{
    int i = 0;
    HI_U32 au32VipipeSumPixel = 0;
    if (pstMediaFreq == NULL) {
        osal_printk( "PM_GetMediaFreq, pstMediaFreq is null.\n");
        return HI_FAILURE;
    }
    if (pstPmParam == NULL) {
        osal_printk( "PM_GetMediaFreq, pstPmParam is null.\n");
        return HI_FAILURE;
    }
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pstPmParam->stViCfg.au32VicapFreq[i] <= 300) {
            /*----according to the sensor ctrl.c cmos.c info ,to fill the vicap freq value-*/
            pstMediaFreq->enVI[i] = VI_F300M;
        } else if (pstPmParam->stViCfg.au32VicapFreq[i] <= 340) {
            /*----according to the sensor ctrl.c cmos.c info ,to fill the vicap freq value-*/
            pstMediaFreq->enVI[i] = VI_F340M;
        } else if (pstPmParam->stViCfg.au32VicapFreq[i] <= 396) {
            /*----according to the sensor ctrl.c cmos.c info ,to fill the vicap freq value-*/
            pstMediaFreq->enVI[i] = VI_F396M;
        } else if ( pstPmParam->stViCfg.au32VicapFreq[i] <= 500) {
            /*----according to the sensor ctrl.c cmos.c info ,to fill the vicap freq value-*/
            pstMediaFreq->enVI[i] = VI_F500M;
        } else if (pstPmParam->stViCfg.au32VicapFreq[i] <= 600) {
            /*----according to the sensor ctrl.c cmos.c info ,to fill the vicap freq value-*/
            pstMediaFreq->enVI[i] = VI_F600M;
        }
    }
    /*-------PIPE BE freq----*/
    for (i = 0; i < VI_MAX_PHY_PIPE_NUM; i++) {
        au32VipipeSumPixel += pstPmParam->stViCfg.au32VipipeSumPixel[i];
    }
    if ( au32VipipeSumPixel <= 300) {
        pstMediaFreq->enPipeBE = PIPEBE_F300M;
    } else if ( au32VipipeSumPixel <= 340) {
        pstMediaFreq->enPipeBE = PIPEBE_F340M;
    }
    /*----vpss freq---*/
    if (pstPmParam->u32VpssSumPix <= 300) {
        pstMediaFreq->enVPSS = VPSS_F300M;
    } else if (pstPmParam->u32VpssSumPix <= 396) {
        pstMediaFreq->enVPSS = VPSS_F396M;
    }
    /*----venc freq---*/
    if (pstPmParam->u32VencSumPix <= 396) {
        pstMediaFreq->enVEDU = VEDU_F396M;
    }
    /*----vdec freq---*/
    if ( pstPmParam->u32VdecSumPix <= 396) {
        //pstMediaFreq->enVEDU= VDH_F396M;
    }
    /*----avsp freq---*/
    if ( pstPmParam->u32AvspSumPix <= 300) {
        pstMediaFreq->enAVSP = AVSP_F300M;
    }
    return HI_SUCCESS;
}


HI_S32 PM_HAL_SetMediaFreqByUsrCfg(const HI_MPI_PM_MEDIA_CFG_S* pstUsrParam)
{
    HI_MEDIA_FREQ_S stMediaFreq = {};
    PM_HAL_GetMediaFreqByUsrCfg(pstUsrParam, &stMediaFreq);
    SetMediaFreqReg(&stMediaFreq);
    return HI_SUCCESS;
}


void PM_HAL_GetMediaProfile(HI_MEDIA_FREQ_S* pstMediaFreq, HI_U32*  pu32Profile)
{
    HI_S32 i = 0;
    HI_U32 u32Profile = 0;
    HI_MEDIA_FREQ_S stMediaFreq = {};
    osal_memcpy(&stMediaFreq, pstMediaFreq, sizeof(HI_MEDIA_FREQ_S));
    for (i = 0; i < 3; i++) {
#if 0
        if (stMediaFreq.enVI0 > media_profile[i].enVI0) {
            continue;
        }
        /*
                if(PIPEBEFreqCmp(stMediaFreq.enPipeBE, media_profile[i].enPipeBE, stMediaFreq.enVI0, media_profile[i].enVI0) > 0)
                {
                    continue;
                }
        */
        if (stMediaFreq.enVI1 > media_profile[i].enVI1) {
            continue;
        }
        /*
                if(PIPEBEFreqCmp(stMediaFreq.enPIPEBE1, media_profile[i].enPIPEBE1, stMediaFreq.enVI1, media_profile[i].enVI1) > 0)
                {
                    continue;
                }
        */
#endif
        if (stMediaFreq.enVGS > media_profile[i].enVGS) {
            continue;
        }
        if (stMediaFreq.enGDC > media_profile[i].enGDC) {
            continue;
        }
        if (stMediaFreq.enVPSS > media_profile[i].enVPSS) {
            continue;
        }
        if (stMediaFreq.enVEDU > media_profile[i].enVEDU) {
            continue;
        }
        if (stMediaFreq.enJPGE > media_profile[i].enJPGE) {
            continue;
        }
        if (stMediaFreq.enVDP > media_profile[i].enVDP) {
            continue;
        }
        if (stMediaFreq.enTDE > media_profile[i].enTDE) {
            continue;
        }
        /*
            if (stMediaFreq.enAIAO > media_profile[i].enAIAO)
            {
                continue;
            }


         if (enDdrFreq > media_profile[i].enDDR)
            {
                continue;
            }

            */
        u32Profile = i + 1;
        *pu32Profile = u32Profile;
        break;
    }
    return ;
}

HI_S32 PM_HAL_GetMediaProfileByUsrCfg(HI_MPI_PM_MEDIA_CFG_S* pstUsrParam, HI_U32*  pu32Profile)
{
    HI_S32 i = 0;
    HI_U32 u32Profile = 0;
    HI_MEDIA_FREQ_S stMediaFreq = {};
    PM_HAL_GetMediaFreqByUsrCfg(pstUsrParam, &stMediaFreq);
    for (i = 0; i < 3; i++) {
#if 0
        if (stMediaFreq.enVI0 > media_profile[i].enVI0) {
            continue;
        }
        /*
                if(PIPEBEFreqCmp(stMediaFreq.enPipeBE, media_profile[i].enPipeBE, stMediaFreq.enVI0, media_profile[i].enVI0) > 0)
                {
                    continue;
                }
        */
        if (stMediaFreq.enVI1 > media_profile[i].enVI1) {
            continue;
        }
        /*
                if(PIPEBEFreqCmp(stMediaFreq.enPIPEBE1, media_profile[i].enPIPEBE1, stMediaFreq.enVI1, media_profile[i].enVI1) > 0)
                {
                    continue;
                }
        */
#endif
        if (stMediaFreq.enVGS > media_profile[i].enVGS) {
            continue;
        }
        if (stMediaFreq.enGDC > media_profile[i].enGDC) {
            continue;
        }
        if (stMediaFreq.enVPSS > media_profile[i].enVPSS) {
            continue;
        }
        if (stMediaFreq.enVEDU > media_profile[i].enVEDU) {
            continue;
        }
        if (stMediaFreq.enJPGE > media_profile[i].enJPGE) {
            continue;
        }
        if (stMediaFreq.enVDP > media_profile[i].enVDP) {
            continue;
        }
        if (stMediaFreq.enTDE > media_profile[i].enTDE) {
            continue;
        }
        /*
            if (stMediaFreq.enAIAO > media_profile[i].enAIAO)
            {
                continue;
            }


         if (enDdrFreq > media_profile[i].enDDR)
            {
                continue;
            }

            */
        u32Profile = i + 1;
        break;
    }
    *pu32Profile = u32Profile;
    return HI_SUCCESS;
}






