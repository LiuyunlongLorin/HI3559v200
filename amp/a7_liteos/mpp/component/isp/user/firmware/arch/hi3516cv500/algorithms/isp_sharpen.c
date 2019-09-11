/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_sharpen.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/16
  Description   :
  History       :
  1.Date        : 2014/01/16
    Author      :
    Modification: Created file

******************************************************************************/
#include "isp_config.h"
#include "hi_isp_debug.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_proc.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define     SHRP_DIR_RANGE_MUL_PRECS            (4)
#define     SHRP_HF_EPS_MUL_PRECS               (4)
#define     SHRP_SHT_VAR_MUL_PRECS              (4)
#define     SHRP_SKIN_EDGE_MUL_PRECS            (4)
#define     SHRP_SKIN_ACCUM_MUL_PRECS           (3)
#define     SHRP_CHR_MUL_SFT                    (4)
#define     SHRP_DETAIL_SHT_MUL_PRECS           (4)
#define     SHRP_DETAIL_CTRL_THR_DELTA          (16)

#define SHARPEN_CLIP3(min,max,x) ( (x)<= (min) ? (min) : ((x)>(max)?(max):(x)) )

typedef struct hiISP_SHARPEN_S
{
    HI_BOOL bSharpenEn;
    HI_BOOL bSharpenMpiUpdateEn;
    HI_U32  u32IsoLast;
    /* Sharpening Yuv */
    //tmp registers
    HI_U8   u8ManualSharpenYuvEnabled;
    HI_U8   u8mfThdSftD;
    HI_U8   u8dirVarSft;
    HI_U8   u8rangeThd0;       //U8.0
    HI_U8   u8rangeDirWgt0;    //U0.7

    HI_U8   u8selPixWgt;
    HI_U8   u8RmfGainScale;
    HI_U8   u8BmfGainScale;
    //HI_U16  u16chrGGain;
    //HI_U16  u16chrGmfGain;

    HI_U8   u8mfThdSelUD;
    HI_U8   u8mfThdSftUD;
    HI_U8   u8hfEpsVarThr0;
    HI_U8   u8hfEpsVarThr1;
    HI_U8   u8hfEpsVal1;

    HI_U8   u8oshtVarWgt0;
    HI_U8   u8ushtVarWgt0;
    HI_U8   u8oshtVarDiffThd0;
    HI_U8   u8oshtVarDiffThd1;
    HI_U8   u8oshtVarDiffWgt1;
    HI_U8   u8ushtVarDiffWgt1;
    //MPI
    HI_U8   u8SkinUmin;
    HI_U8   u8SkinVmin;
    HI_U8   u8SkinUmax;
    HI_U8   u8SkinVmax;
    HI_U16  au16TextureStr[ISP_SHARPEN_GAIN_NUM];        //Undirectional sharpen strength for texture and detail enhancement. U7.5  [0, 4095]
    HI_U16  au16EdgeStr[ISP_SHARPEN_GAIN_NUM];           //Directional sharpen strength for edge enhancement.     U7.5  [0, 4095]
    HI_U8   au8LumaWgt[ISP_SHARPEN_LUMA_NUM];            //Adjust the sharpen strength according to luma. Sharpen strength will be weaker when it decrease. U7.0  [0, 127]
    HI_U16  u16TextureFreq;                              //Texture frequency adjustment. Texture and detail will be finer when it increase.    U6.6  [0, 4095]
    HI_U16  u16EdgeFreq;                                 //Edge frequency adjustment. Edge will be narrower and thiner when it increase.      U6.6  [0, 4095]
    HI_U8   u8OverShoot;                                 //u8OvershootAmt       U7.0  [0, 127]
    HI_U8   u8UnderShoot;                                //u8UndershootAmt      U7.0  [0, 127]
    HI_U8   u8ShootSupStr;                               //overshoot and undershoot suppression strength, the amplitude and width of shoot will be decrease when shootSupSt increase.  U8.0 [0, 255]
    HI_U8   u8ShootSupAdj;
    HI_U8   u8DetailCtrl;                                //Different sharpen strength for detail and edge. When it is bigger than 128, detail sharpen strength will be stronger than edge.  //[0, 255]
    HI_U8   u8DetailCtrlThr;
    HI_U8   u8EdgeFiltStr;                               //u8EdgeFiltStr    U6.0  [0, 63]
    HI_U8   u8RGain;
    HI_U8   u8GGain;
    HI_U8   u8BGain;
    HI_U8   u8SkinGain;
    HI_U16  u16MaxSharpGain;

    HI_U16 au16AutoTextureStr[ISP_SHARPEN_GAIN_NUM][ISP_AUTO_ISO_STRENGTH_NUM];     //Undirectional sharpen strength for texture and detail enhancement. U7.5  [0, 4095]
    HI_U16 au16AutoEdgeStr[ISP_SHARPEN_GAIN_NUM][ISP_AUTO_ISO_STRENGTH_NUM];        //Directional sharpen strength for edge enhancement.  U7.5  [0, 4095]
    HI_U8  au8AutoLumaWgt[ISP_SHARPEN_LUMA_NUM][ISP_AUTO_ISO_STRENGTH_NUM];         //Adjust the sharpen strength according to luma. Sharpen strength will be weaker when it decrease. U7.0  [0, 127]
    HI_U16 au16TextureFreq[ISP_AUTO_ISO_STRENGTH_NUM];                              //Texture frequency adjustment. Texture and detail will be finer when it increase.    U6.6  [0, 4095]
    HI_U16 au16EdgeFreq[ISP_AUTO_ISO_STRENGTH_NUM];                                 //Edge frequency adjustment. Edge will be narrower and thiner when it increase.      U6.6  [0, 4095]
    HI_U8  au8OverShoot[ISP_AUTO_ISO_STRENGTH_NUM];                                 //u8OvershootAmt       U7.0  [0, 127]
    HI_U8  au8UnderShoot[ISP_AUTO_ISO_STRENGTH_NUM];                                //u8UndershootAmt      U7.0  [0, 127]
    HI_U8  au8ShootSupStr[ISP_AUTO_ISO_STRENGTH_NUM];                               //overshoot and undershoot suppression strength, the amplitude and width of shoot will be decrease when shootSupSt increase.  U8.0 [0, 255]
    HI_U8  au8ShootSupAdj[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8DetailCtrl[ISP_AUTO_ISO_STRENGTH_NUM];                                //Different sharpen strength for detail and edge. When it is bigger than 128, detail sharpen strength will be stronger than edge.  //[0, 255]
    HI_U8  au8DetailCtrlThr[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8EdgeFiltStr[ISP_AUTO_ISO_STRENGTH_NUM];                               //u8EdgeFiltStr    U6.0  [0, 63]
    HI_U8  au8RGain[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8GGain[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8BGain[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8SkinGain[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16MaxSharpGain[ISP_AUTO_ISO_STRENGTH_NUM];
} ISP_SHARPEN_S;

ISP_SHARPEN_S *g_pastSharpenCtx[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define SHARPEN_GET_CTX(dev, pstCtx)   (pstCtx = g_pastSharpenCtx[dev])
#define SHARPEN_SET_CTX(dev, pstCtx)   (g_pastSharpenCtx[dev] = pstCtx)
#define SHARPEN_RESET_CTX(dev)         (g_pastSharpenCtx[dev] = HI_NULL)

HI_S32 shrp_blend( HI_U8 sft, HI_S32 wgt1, HI_S32 v1, HI_S32 wgt2, HI_S32 v2)
{
    HI_S32 res;
    res = SignedRightShift(((HI_S64)v1*wgt1) + ((HI_S64)v2*wgt2), sft);
    return res;
}

HI_S32 SharpenCtxInit(VI_PIPE ViPipe)
{
    ISP_SHARPEN_S *pastSharpenCtx = HI_NULL;

    SHARPEN_GET_CTX(ViPipe, pastSharpenCtx);

    if (HI_NULL == pastSharpenCtx)
    {
        pastSharpenCtx = (ISP_SHARPEN_S *)ISP_MALLOC(sizeof(ISP_SHARPEN_S));
        if (HI_NULL == pastSharpenCtx)
        {
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] SharpenCtx malloc memory failed!\n", ViPipe);
            return HI_ERR_ISP_NOMEM;
        }
    }

    memset(pastSharpenCtx, 0, sizeof(ISP_SHARPEN_S));

    SHARPEN_SET_CTX(ViPipe, pastSharpenCtx);

    return HI_SUCCESS;
}

HI_VOID SharpenCtxExit(VI_PIPE ViPipe)
{
    ISP_SHARPEN_S *pastSharpenCtx = HI_NULL;

    SHARPEN_GET_CTX(ViPipe, pastSharpenCtx);
    ISP_FREE(pastSharpenCtx);
    SHARPEN_RESET_CTX(ViPipe);
}

static HI_S32 SharpenCheckCmosParam(VI_PIPE ViPipe, const ISP_CMOS_SHARPEN_S *pstSharpen)
{
    HI_U8 i, j;

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        for (j = 0; j < ISP_SHARPEN_GAIN_NUM; j++)
        {
            if (pstSharpen->stAuto.au16TextureStr[j][i] > 4095)
            {
                ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au16TextureStr[%d][%d]! Value range:[0, 4095]\n", j, i);
                return HI_ERR_ISP_ILLEGAL_PARAM;
            }
            if (pstSharpen->stAuto.au16EdgeStr[j][i] > 4095 )
            {
                ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au16EdgeStr[%d][%d]! Value range:[0, 4095]\n", j, i);
                return HI_ERR_ISP_ILLEGAL_PARAM;
            }
        }
        for (j = 0; j < ISP_SHARPEN_LUMA_NUM; j++)
        {
            if (pstSharpen->stAuto.au8LumaWgt[j][i] > 127 )
            {
                ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au8LumaWgt[%d][%d]! Value range:[0, 127]\n", j, i);
                return HI_ERR_ISP_ILLEGAL_PARAM;
            }
        }
        if (pstSharpen->stAuto.au16TextureFreq[i] > 4095)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au16TextureFreq[%d]! Value range:[0, 4095]\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstSharpen->stAuto.au16EdgeFreq[i] > 4095 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au16EdgeFreq[%d]! Value range:[0, 4095]\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstSharpen->stAuto.au8OverShoot[i] > 127 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au8OverShoot[%d]! Value range:[0, 127]\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstSharpen->stAuto.au8UnderShoot[i] > 127 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au8UnderShoot[%d]! Value range:[0, 127]\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstSharpen->stAuto.au8ShootSupAdj[i] > 15 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au8ShootSupAdj[%d]! Value range:[0, 15]\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstSharpen->stAuto.au8EdgeFiltStr[i] > 63 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au8EdgeFiltStr[%d]! Value range:[0, 63]\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstSharpen->stAuto.au8RGain[i] > HI_ISP_SHARPEN_RGAIN_MAX )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au8RGain[%d]! Value range:[0, %d]\n", i, HI_ISP_SHARPEN_RGAIN_MAX);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstSharpen->stAuto.au8GGain[i] > HI_ISP_SHARPEN_GGAIN_MAX )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au8GGain[%d]! Value range:[0, %d]\n", i, HI_ISP_SHARPEN_GGAIN_MAX);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstSharpen->stAuto.au8BGain[i] > HI_ISP_SHARPEN_BGAIN_MAX )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au8BGain[%d]! Value range:[0, %d]\n", i, HI_ISP_SHARPEN_BGAIN_MAX);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstSharpen->stAuto.au8SkinGain[i] > 31 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au8SkinGain[%d]! Value range:[0, 31]\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstSharpen->stAuto.au16MaxSharpGain[i] > 0x7FF )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stAuto.au16MaxSharpGain[%d]! Value range:[0, 0x7FF]\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

    }
    for (j = 0; j < ISP_SHARPEN_GAIN_NUM; j++)
    {
        if (pstSharpen->stManual.au16TextureStr[j] > 4095)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stManual.au16TextureStr[%d]! Value range:[0, 4095]\n", j);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
        if (pstSharpen->stManual.au16EdgeStr[j] > 4095 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stManual.au16EdgeStr[%d]! Value range:[0, 4095]\n", j);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    for (j = 0; j < ISP_SHARPEN_LUMA_NUM; j++)
    {
        if (pstSharpen->stManual.au8LumaWgt[j] > 127 )
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid stManual.au8LumaWgt[%d]! Value range:[0, 127]\n", j);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
    }
    if (pstSharpen->stManual.u16TextureFreq > 4095 )
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u16TextureFreq! Value range:[0, 4095]\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstSharpen->stManual.u16EdgeFreq > 4095 )
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u16EdgeFreq! Value range:[0, 4095]\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstSharpen->stManual.u8OverShoot > 127)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u8OverShoot! Value range:[0, 127]\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstSharpen->stManual.u8UnderShoot > 127)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u8UnderShoot! Value range:[0, 127]\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstSharpen->stManual.u8ShootSupAdj > 15)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u8ShootSupAdj! Value range:[0, 15]\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstSharpen->stManual.u8EdgeFiltStr > 63)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u8EdgeFiltStr! Value range:[0, 63]\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstSharpen->stManual.u8RGain > HI_ISP_SHARPEN_RGAIN_MAX )
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u8RGain! Value range:[0, %d]\n", HI_ISP_SHARPEN_RGAIN_MAX);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstSharpen->stManual.u8GGain > HI_ISP_SHARPEN_GGAIN_MAX )
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u8GGain! Value range:[0, %d]\n", HI_ISP_SHARPEN_GGAIN_MAX);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstSharpen->stManual.u8BGain > HI_ISP_SHARPEN_BGAIN_MAX)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u8BGain! Value range:[0, %d]\n", HI_ISP_SHARPEN_BGAIN_MAX);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstSharpen->stManual.u8SkinGain > 31)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u8SkinGain! Value range:[0, 31]\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstSharpen->stManual.u16MaxSharpGain > 0x7FF)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stManual.u16MaxSharpGain! Value range:[0, 0x7FF]\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static HI_S32 SharpenExtRegsInitialize(VI_PIPE ViPipe)
{
    HI_U16 i, j;
    HI_S32 s32Ret;
    ISP_CMOS_DEFAULT_S       *pstSnsDft      = HI_NULL;
    const ISP_CMOS_SHARPEN_S *pstCmosSharpen = HI_NULL;

    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    hi_ext_system_isp_sharpen_manu_mode_write(ViPipe, OP_TYPE_AUTO);
    hi_ext_system_manual_isp_sharpen_en_write(ViPipe, HI_TRUE);
    hi_ext_system_sharpen_mpi_update_en_write(ViPipe, HI_TRUE);

    //auto ExtRegs initial
    if (pstSnsDft->unKey.bit1Sharpen)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstSharpen);

        s32Ret = SharpenCheckCmosParam(ViPipe, pstSnsDft->pstSharpen);
        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        pstCmosSharpen = pstSnsDft->pstSharpen;

        for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
        {
            for (j = 0; j < ISP_SHARPEN_GAIN_NUM ; j++)
            {
                hi_ext_system_Isp_sharpen_TextureStr_write (ViPipe, i + j * ISP_AUTO_ISO_STRENGTH_NUM, pstCmosSharpen->stAuto.au16TextureStr[j][i]);
                hi_ext_system_Isp_sharpen_EdgeStr_write    (ViPipe, i + j * ISP_AUTO_ISO_STRENGTH_NUM, pstCmosSharpen->stAuto.au16EdgeStr[j][i]);
            }
            for (j = 0; j < ISP_SHARPEN_LUMA_NUM; j++)
            {
                hi_ext_system_Isp_sharpen_LumaWgt_write    (ViPipe, i + j * ISP_AUTO_ISO_STRENGTH_NUM, pstCmosSharpen->stAuto.au8LumaWgt[j][i]);
            }
            hi_ext_system_Isp_sharpen_TextureFreq_write    (ViPipe, i, pstCmosSharpen->stAuto.au16TextureFreq[i]);
            hi_ext_system_Isp_sharpen_EdgeFreq_write       (ViPipe, i, pstCmosSharpen->stAuto.au16EdgeFreq[i]);
            hi_ext_system_Isp_sharpen_OverShoot_write      (ViPipe, i, pstCmosSharpen->stAuto.au8OverShoot[i]);
            hi_ext_system_Isp_sharpen_UnderShoot_write     (ViPipe, i, pstCmosSharpen->stAuto.au8UnderShoot[i]);
            hi_ext_system_Isp_sharpen_shootSupStr_write    (ViPipe, i, pstCmosSharpen->stAuto.au8ShootSupStr[i]);
            hi_ext_system_Isp_sharpen_detailctrl_write     (ViPipe, i, pstCmosSharpen->stAuto.au8DetailCtrl[i]);
            hi_ext_system_Isp_sharpen_EdgeFiltStr_write    (ViPipe, i, pstCmosSharpen->stAuto.au8EdgeFiltStr[i]);
            hi_ext_system_Isp_sharpen_RGain_write          (ViPipe, i, pstCmosSharpen->stAuto.au8RGain[i]);
            hi_ext_system_Isp_sharpen_GGain_write          (ViPipe, i, pstCmosSharpen->stAuto.au8GGain[i]);
            hi_ext_system_Isp_sharpen_BGain_write          (ViPipe, i, pstCmosSharpen->stAuto.au8BGain[i]);
            hi_ext_system_Isp_sharpen_SkinGain_write       (ViPipe, i, pstCmosSharpen->stAuto.au8SkinGain[i]);
            hi_ext_system_Isp_sharpen_ShootSupAdj_write    (ViPipe, i, pstCmosSharpen->stAuto.au8ShootSupAdj[i]);
            hi_ext_system_Isp_sharpen_MaxSharpGain_write   (ViPipe, i, pstCmosSharpen->stAuto.au16MaxSharpGain[i]);
            hi_ext_system_Isp_sharpen_detailctrlThr_write  (ViPipe, i, pstCmosSharpen->stAuto.au8DetailCtrlThr[i]);
        }

        //manual ExtRegs initial
        for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
        {
            hi_ext_system_manual_Isp_sharpen_TextureStr_write(ViPipe, i, pstCmosSharpen->stManual.au16TextureStr[i]);
            hi_ext_system_manual_Isp_sharpen_EdgeStr_write   (ViPipe, i, pstCmosSharpen->stManual.au16EdgeStr[i]);
        }

        for (i = 0; i < ISP_SHARPEN_LUMA_NUM; i++)
        {
            hi_ext_system_manual_Isp_sharpen_LumaWgt_write   (ViPipe, i, pstCmosSharpen->stManual.au8LumaWgt[i]);
        }

        hi_ext_system_manual_Isp_sharpen_TextureFreq_write   (ViPipe, pstCmosSharpen->stManual.u16TextureFreq);
        hi_ext_system_manual_Isp_sharpen_EdgeFreq_write      (ViPipe, pstCmosSharpen->stManual.u16EdgeFreq);
        hi_ext_system_manual_Isp_sharpen_OverShoot_write     (ViPipe, pstCmosSharpen->stManual.u8OverShoot);
        hi_ext_system_manual_Isp_sharpen_UnderShoot_write    (ViPipe, pstCmosSharpen->stManual.u8UnderShoot);
        hi_ext_system_manual_Isp_sharpen_shootSupStr_write   (ViPipe, pstCmosSharpen->stManual.u8ShootSupStr);
        hi_ext_system_manual_Isp_sharpen_detailctrl_write    (ViPipe, pstCmosSharpen->stManual.u8DetailCtrl);
        hi_ext_system_manual_Isp_sharpen_EdgeFiltStr_write   (ViPipe, pstCmosSharpen->stManual.u8EdgeFiltStr);
        hi_ext_system_manual_Isp_sharpen_RGain_write         (ViPipe, pstCmosSharpen->stManual.u8RGain);
        hi_ext_system_manual_Isp_sharpen_GGain_write         (ViPipe, pstCmosSharpen->stManual.u8GGain);
        hi_ext_system_manual_Isp_sharpen_BGain_write         (ViPipe, pstCmosSharpen->stManual.u8BGain);
        hi_ext_system_manual_Isp_sharpen_SkinGain_write      (ViPipe, pstCmosSharpen->stManual.u8SkinGain);
        hi_ext_system_manual_Isp_sharpen_ShootSupAdj_write   (ViPipe, pstCmosSharpen->stManual.u8ShootSupAdj);
        hi_ext_system_manual_Isp_sharpen_MaxSharpGain_write  (ViPipe, pstCmosSharpen->stManual.u16MaxSharpGain);
        hi_ext_system_manual_Isp_sharpen_detailctrlThr_write (ViPipe, pstCmosSharpen->stManual.u8DetailCtrlThr);
        hi_ext_system_manual_Isp_sharpen_SkinUmax_write      (ViPipe, pstCmosSharpen->u8SkinUmax);
        hi_ext_system_manual_Isp_sharpen_SkinUmin_write      (ViPipe, pstCmosSharpen->u8SkinUmin);
        hi_ext_system_manual_Isp_sharpen_SkinVmax_write      (ViPipe, pstCmosSharpen->u8SkinVmax);
        hi_ext_system_manual_Isp_sharpen_SkinVmin_write      (ViPipe, pstCmosSharpen->u8SkinVmin);

        {
            hi_ext_system_actual_sharpen_overshootAmt_write     (ViPipe, pstCmosSharpen->stManual.u8OverShoot);
            hi_ext_system_actual_sharpen_undershootAmt_write    (ViPipe, pstCmosSharpen->stManual.u8UnderShoot);
            hi_ext_system_actual_sharpen_shootSupSt_write       (ViPipe, pstCmosSharpen->stManual.u8ShootSupStr);
            hi_ext_system_actual_sharpen_edge_frequence_write   (ViPipe, pstCmosSharpen->stManual.u16EdgeFreq);
            hi_ext_system_actual_sharpen_texture_frequence_write(ViPipe, pstCmosSharpen->stManual.u16TextureFreq);

            for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
            {
                hi_ext_system_actual_sharpen_edge_str_write     (ViPipe, i, pstCmosSharpen->stManual.au16EdgeStr[i]);
                hi_ext_system_actual_sharpen_texture_str_write  (ViPipe, i, pstCmosSharpen->stManual.au16TextureStr[i]);
            }
        }
    }
    else
    {
        //auto ExtRegs initial
        for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
        {
            for (j = 0; j < ISP_SHARPEN_GAIN_NUM ; j++)
            {
                hi_ext_system_Isp_sharpen_TextureStr_write (ViPipe, i + j * ISP_AUTO_ISO_STRENGTH_NUM, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_TEXTURESTR_DEFAULT);
                hi_ext_system_Isp_sharpen_EdgeStr_write    (ViPipe, i + j * ISP_AUTO_ISO_STRENGTH_NUM, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGESTR_DEFAULT);
            }
            for (j = 0; j < ISP_SHARPEN_LUMA_NUM; j++)
            {
                hi_ext_system_Isp_sharpen_LumaWgt_write    (ViPipe, i + j * ISP_AUTO_ISO_STRENGTH_NUM, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_LUMAWGT_DEFAULT);
            }
            hi_ext_system_Isp_sharpen_TextureFreq_write    (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_TEXTUREFREQ_DEFAULT);
            hi_ext_system_Isp_sharpen_EdgeFreq_write       (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGEFREQ_DEFAULT);
            hi_ext_system_Isp_sharpen_OverShoot_write      (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_OVERSHOOT_DEFAULT);
            hi_ext_system_Isp_sharpen_UnderShoot_write     (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_UNDERSHOOT_DEFAULT);
            hi_ext_system_Isp_sharpen_shootSupStr_write    (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SHOOTSUPSTR_DEFAULT);
            hi_ext_system_Isp_sharpen_detailctrl_write     (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_DETAILCTRL_DEFAULT);
            hi_ext_system_Isp_sharpen_EdgeFiltStr_write    (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGEFILTSTR_DEFAULT);
            hi_ext_system_Isp_sharpen_RGain_write          (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_RGAIN_DEFAULT);
            hi_ext_system_Isp_sharpen_GGain_write          (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_GGAIN_DEFAULT);
            hi_ext_system_Isp_sharpen_BGain_write          (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_BGAIN_DEFAULT);
            hi_ext_system_Isp_sharpen_SkinGain_write       (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINGAIN_DEFAULT);
            hi_ext_system_Isp_sharpen_ShootSupAdj_write    (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SHOOTSUPADJ_DEFAULT);
            hi_ext_system_Isp_sharpen_MaxSharpGain_write   (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_MAXSHARPGAIN_DEFAULT);
            hi_ext_system_Isp_sharpen_detailctrlThr_write  (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_DETAILCTRLTHR_DEFAULT);
        }
        //manual ExtRegs initial
        for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
        {
            hi_ext_system_manual_Isp_sharpen_TextureStr_write(ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_TEXTURESTR_DEFAULT);
            hi_ext_system_manual_Isp_sharpen_EdgeStr_write   (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGESTR_DEFAULT);
        }
        for (i = 0; i < ISP_SHARPEN_LUMA_NUM; i++)
        {
            hi_ext_system_manual_Isp_sharpen_LumaWgt_write   (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_LUMAWGT_DEFAULT);
        }

        hi_ext_system_manual_Isp_sharpen_TextureFreq_write  (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_TEXTUREFREQ_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_EdgeFreq_write     (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGEFREQ_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_OverShoot_write    (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_OVERSHOOT_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_UnderShoot_write   (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_UNDERSHOOT_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_shootSupStr_write  (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SHOOTSUPSTR_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_detailctrl_write   (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_DETAILCTRL_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_EdgeFiltStr_write  (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGEFILTSTR_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_RGain_write        (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_RGAIN_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_GGain_write        (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_GGAIN_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_BGain_write        (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_BGAIN_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_SkinGain_write     (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINGAIN_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_ShootSupAdj_write  (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SHOOTSUPADJ_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_MaxSharpGain_write (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_MAXSHARPGAIN_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_detailctrlThr_write(ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_DETAILCTRLTHR_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_SkinUmax_write(ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINUMAX_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_SkinUmin_write(ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINUMIN_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_SkinVmax_write(ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINVMAX_DEFAULT);
        hi_ext_system_manual_Isp_sharpen_SkinVmin_write(ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINVMIN_DEFAULT);
        {
            hi_ext_system_actual_sharpen_overshootAmt_write      (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_OVERSHOOT_DEFAULT);
            hi_ext_system_actual_sharpen_undershootAmt_write     (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_UNDERSHOOT_DEFAULT);
            hi_ext_system_actual_sharpen_shootSupSt_write        (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SHOOTSUPSTR_DEFAULT);
            hi_ext_system_actual_sharpen_edge_frequence_write    (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGEFREQ_DEFAULT);
            hi_ext_system_actual_sharpen_texture_frequence_write (ViPipe, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_TEXTUREFREQ_DEFAULT);

            for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
            {
                hi_ext_system_actual_sharpen_edge_str_write      (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGESTR_DEFAULT);
                hi_ext_system_actual_sharpen_texture_str_write   (ViPipe, i, HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_TEXTURESTR_DEFAULT);
            }
        }
    }

    return HI_SUCCESS;
}

//****Sharpen hardware Regs that will not change****//
static HI_VOID SharpenStaticRegInit(VI_PIPE ViPipe, ISP_SHARPEN_STATIC_REG_CFG_S *pstStaticRegCfg)
{
    HI_U8 i;
    pstStaticRegCfg->bStaticResh     = HI_TRUE;
    pstStaticRegCfg->u8hfThdSelD     = 1;
    pstStaticRegCfg->u8mfThdSelD     = 1;
    pstStaticRegCfg->u8dirVarScale   = 0;
    pstStaticRegCfg->u8dirRly[0]     = 127;
    pstStaticRegCfg->u8dirRly[1]     = 0;
    pstStaticRegCfg->u8rangeThd1     = 55 ;
    pstStaticRegCfg->u8rangeDirWgt1  = 127 ;
    pstStaticRegCfg->u8bEnHfEps      = 1 ;
    pstStaticRegCfg->u8hfEpsVal0     = 127 ;
    pstStaticRegCfg->u8MaxVarClipMin = 3;
    pstStaticRegCfg->u16oMaxChg      = 180;
    pstStaticRegCfg->u16uMaxChg      = 200;
    pstStaticRegCfg->u8shtVarSft     = 0;

    for ( i = 0; i < ISP_SHARPEN_FREQ_CORING_LENGTH; i++ )
    {
        pstStaticRegCfg->u8lmtMF[i]     = 0;
        pstStaticRegCfg->u8lmtHF[i]     = 0;
    }

    pstStaticRegCfg->u8skinSrcSel       = 0;
    pstStaticRegCfg->u8skinCntThd[0]    = 5;
    pstStaticRegCfg->u8skinEdgeThd[0]   = 10;
    pstStaticRegCfg->u16skinAccumThd[0] = 0;
    pstStaticRegCfg->u8skinAccumWgt[0]  = 0;
    pstStaticRegCfg->u8skinCntThd[1]    = 8;
    pstStaticRegCfg->u8skinEdgeThd[1]   = 30;
    pstStaticRegCfg->u16skinAccumThd[1] = 10;
    pstStaticRegCfg->u8skinAccumWgt[1]  = 20;
    pstStaticRegCfg->u8skinEdgeSft      = 1;

    //Chr
    pstStaticRegCfg->u8chrRVarSft    = 7;
    pstStaticRegCfg->u8RcBmin        = 0;
    pstStaticRegCfg->u8RcBmax        = 255;
    pstStaticRegCfg->u8RcRmin        = 0;
    pstStaticRegCfg->u8RcRmax        = 255;
    pstStaticRegCfg->u8chrROriCb     = 120;
    pstStaticRegCfg->u8chrROriCr     = 220;
    pstStaticRegCfg->u8chrRSft[0]    = 7;
    pstStaticRegCfg->u8chrRSft[1]    = 7;
    pstStaticRegCfg->u8chrRSft[2]    = 7;
    pstStaticRegCfg->u8chrRSft[3]    = 6;
    pstStaticRegCfg->u8chrRThd[0]    = 40;
    pstStaticRegCfg->u8chrRThd[1]    = 60;

    pstStaticRegCfg->u8chrBVarSft    = 2;
    pstStaticRegCfg->u8BcBmin        = 0;
    pstStaticRegCfg->u8BcBmax        = 255;
    pstStaticRegCfg->u8BcRmin        = 0;
    pstStaticRegCfg->u8BcRmax        = 255;
    pstStaticRegCfg->u8chrBOriCb     = 200;
    pstStaticRegCfg->u8chrBOriCr     = 64;
    pstStaticRegCfg->u8chrBSft[0]    = 7;
    pstStaticRegCfg->u8chrBSft[1]    = 7;
    pstStaticRegCfg->u8chrBSft[2]    = 7;
    pstStaticRegCfg->u8chrBSft[3]    = 7;
    pstStaticRegCfg->u8chrBThd[0]    = 40;
    pstStaticRegCfg->u8chrBThd[1]    = 90;

    pstStaticRegCfg->u8GcBmin        = 60;
    pstStaticRegCfg->u8GcBmax        = 120;
    pstStaticRegCfg->u8GcRmin        = 90;
    pstStaticRegCfg->u8GcRmax        = 130;
    pstStaticRegCfg->u8chrGOriCb     = 90;
    pstStaticRegCfg->u8chrGOriCr     = 110;
    pstStaticRegCfg->u8chrGSft[0]    = 4;
    pstStaticRegCfg->u8chrGSft[1]    = 7;
    pstStaticRegCfg->u8chrGSft[2]    = 4;
    pstStaticRegCfg->u8chrGSft[3]    = 7;
    pstStaticRegCfg->u8chrGThd[0]    = 20;
    pstStaticRegCfg->u8chrGThd[1]    = 40;

    pstStaticRegCfg->u8oshtVarWgt1     = 127;
    pstStaticRegCfg->u8ushtVarWgt1     = 127;
    pstStaticRegCfg->u8ushtVarDiffWgt0 = 127;
    pstStaticRegCfg->u8oshtVarDiffWgt0 = 127;
    pstStaticRegCfg->u8oshtVarThd0     = 0;
    pstStaticRegCfg->u8ushtVarThd0     = 0;
    pstStaticRegCfg->u8lumaSrcSel      = 0;
    pstStaticRegCfg->u8dirRt[0]        = 6;
    pstStaticRegCfg->u8dirRt[1]        = 18;
    pstStaticRegCfg->u8shtNoiseMin     = 0;
    pstStaticRegCfg->u8shtNoiseMax     = 0;

    // filter
    pstStaticRegCfg->s8lpfCoefUD[0]   = 5;
    pstStaticRegCfg->s8lpfCoefUD[1]   = 7;
    pstStaticRegCfg->s8lpfCoefUD[2]   = 8;

    pstStaticRegCfg->s8lpfCoefD[0]    = 4;
    pstStaticRegCfg->s8lpfCoefD[1]    = 7;
    pstStaticRegCfg->s8lpfCoefD[2]    = 10;

    pstStaticRegCfg->s8hsfCoefUD[0]   = -2;
    pstStaticRegCfg->s8hsfCoefUD[1]   = 9;
    pstStaticRegCfg->s8hsfCoefUD[2]   = 18;

    pstStaticRegCfg->s8hsfCoefD0[0]   = -1;
    pstStaticRegCfg->s8hsfCoefD0[1]   = -2;
    pstStaticRegCfg->s8hsfCoefD0[2]   = -3;
    pstStaticRegCfg->s8hsfCoefD0[3]   = -2;
    pstStaticRegCfg->s8hsfCoefD0[4]   = -1;
    pstStaticRegCfg->s8hsfCoefD0[5]   = 12;
    pstStaticRegCfg->s8hsfCoefD0[6]   = 27;
    pstStaticRegCfg->s8hsfCoefD0[7]   = 36;
    pstStaticRegCfg->s8hsfCoefD0[8]   = 27;
    pstStaticRegCfg->s8hsfCoefD0[9]   = 12;
    pstStaticRegCfg->s8hsfCoefD0[10]  = 31;
    pstStaticRegCfg->s8hsfCoefD0[11]  = 72;
    pstStaticRegCfg->s8hsfCoefD0[12]  = 96;

    pstStaticRegCfg->s8hsfCoefD1[0]   = -2;
    pstStaticRegCfg->s8hsfCoefD1[1]   = -3;
    pstStaticRegCfg->s8hsfCoefD1[2]   = -1;
    pstStaticRegCfg->s8hsfCoefD1[3]   = 6;
    pstStaticRegCfg->s8hsfCoefD1[4]   = 6;
    pstStaticRegCfg->s8hsfCoefD1[5]   = 0;
    pstStaticRegCfg->s8hsfCoefD1[6]   = 13;
    pstStaticRegCfg->s8hsfCoefD1[7]   = 41;
    pstStaticRegCfg->s8hsfCoefD1[8]   = 47;
    pstStaticRegCfg->s8hsfCoefD1[9]   = 23;
    pstStaticRegCfg->s8hsfCoefD1[10]  = 18;
    pstStaticRegCfg->s8hsfCoefD1[11]  = 62;
    pstStaticRegCfg->s8hsfCoefD1[12]  = 92;

    pstStaticRegCfg->s8hsfCoefD2[0]   = -1;
    pstStaticRegCfg->s8hsfCoefD2[1]   = -3;
    pstStaticRegCfg->s8hsfCoefD2[2]   = 6;
    pstStaticRegCfg->s8hsfCoefD2[3]   = 16;
    pstStaticRegCfg->s8hsfCoefD2[4]   = 10;
    pstStaticRegCfg->s8hsfCoefD2[5]   = -3;
    pstStaticRegCfg->s8hsfCoefD2[6]   = 10;
    pstStaticRegCfg->s8hsfCoefD2[7]   = 50;
    pstStaticRegCfg->s8hsfCoefD2[8]   = 53;
    pstStaticRegCfg->s8hsfCoefD2[9]   = 16;
    pstStaticRegCfg->s8hsfCoefD2[10]  = 6;
    pstStaticRegCfg->s8hsfCoefD2[11]  = 50;
    pstStaticRegCfg->s8hsfCoefD2[12]  = 92;

    pstStaticRegCfg->s8hsfCoefD3[0]   = -2;
    pstStaticRegCfg->s8hsfCoefD3[1]   = 0;
    pstStaticRegCfg->s8hsfCoefD3[2]   = 18;
    pstStaticRegCfg->s8hsfCoefD3[3]   = 23;
    pstStaticRegCfg->s8hsfCoefD3[4]   = 6;
    pstStaticRegCfg->s8hsfCoefD3[5]   = -3;
    pstStaticRegCfg->s8hsfCoefD3[6]   = 13;
    pstStaticRegCfg->s8hsfCoefD3[7]   = 62;
    pstStaticRegCfg->s8hsfCoefD3[8]   = 47;
    pstStaticRegCfg->s8hsfCoefD3[9]   = 6;
    pstStaticRegCfg->s8hsfCoefD3[10]  = -1;
    pstStaticRegCfg->s8hsfCoefD3[11]  = 41;
    pstStaticRegCfg->s8hsfCoefD3[12]  = 92;

    pstStaticRegCfg->u8lpfSftUD        = 5;
    pstStaticRegCfg->u8lpfSftD         = 5;
    pstStaticRegCfg->u8hsfSftUD        = 5;
    pstStaticRegCfg->u8hsfSftD         = 9;

    pstStaticRegCfg->bEnShp8Dir        = 1;

    pstStaticRegCfg->u8hfGainSft       = 5;
    pstStaticRegCfg->u8mfGainSft       = 5;
    pstStaticRegCfg->u8shtVarSel       = 1;
    pstStaticRegCfg->u8shtVar5x5Sft    = 1;
    pstStaticRegCfg->u8detailThdSel    = 0;
    pstStaticRegCfg->u8detailThdSft    = 0;

    //SharpenCheckStaticReg(pstStaticRegCfg);

    // Skin detection
    pstStaticRegCfg->u8skinCntMul    = CalcMulCoef( pstStaticRegCfg->u8skinCntThd[0], 0, pstStaticRegCfg->u8skinCntThd[1], 31, 0);
    pstStaticRegCfg->s16skinAccumMul = CalcMulCoef( pstStaticRegCfg->u16skinAccumThd[0], pstStaticRegCfg->u8skinAccumWgt[0],
                                                    pstStaticRegCfg->u16skinAccumThd[1], pstStaticRegCfg->u8skinAccumWgt[1],
                                                    SHRP_SKIN_ACCUM_MUL_PRECS );
}

//****Sharpen hardware Regs that will change with MPI and ISO****//
static HI_VOID SharpenMpiDynaRegInit(ISP_SHARPEN_MPI_DYNA_REG_CFG_S *pstMpiDynaRegCfg)
{
    HI_U8 i;

    for (i = 0; i < SHRP_GAIN_LUT_SIZE; i++)
    {
        pstMpiDynaRegCfg->u16mfGainD[i]  = 300;
        pstMpiDynaRegCfg->u16mfGainUD[i] = 200;
        pstMpiDynaRegCfg->u16hfGainD[i]  = 450;
        pstMpiDynaRegCfg->u16hfGainUD[i] = 400;
    }

    pstMpiDynaRegCfg->u8oshtAmt          = 100;
    pstMpiDynaRegCfg->u8ushtAmt          = 127;
    pstMpiDynaRegCfg->u8bEnShtCtrlByVar  = 1;
    pstMpiDynaRegCfg->u8shtBldRt         = 9;
    pstMpiDynaRegCfg->u8oshtVarThd1      = 5;
    pstMpiDynaRegCfg->u8ushtVarThd1      = 5;

    pstMpiDynaRegCfg->u8bEnChrCtrl       = 1;
    pstMpiDynaRegCfg->u8chrRGain         = 6;
    pstMpiDynaRegCfg->u16chrGGain        = 32;
    pstMpiDynaRegCfg->u16chrGmfGain      = 32;
    pstMpiDynaRegCfg->u8chrBGain         = 14;
    pstMpiDynaRegCfg->u8bEnSkinCtrl      = 0;
    pstMpiDynaRegCfg->u8skinEdgeWgt[1]   = 31;
    pstMpiDynaRegCfg->u8skinEdgeWgt[0]   = 31;

    pstMpiDynaRegCfg->u8bEnLumaCtrl      = 0;
    pstMpiDynaRegCfg->bEnDetailCtrl      = 0;
    pstMpiDynaRegCfg->u8detailOshtAmt    = 100;
    pstMpiDynaRegCfg->u8detailUshtAmt    = 127;
    pstMpiDynaRegCfg->u8dirDiffSft = 10;
    pstMpiDynaRegCfg->u8skinMaxU         = 127;
    pstMpiDynaRegCfg->u8skinMinU         = 95;
    pstMpiDynaRegCfg->u8skinMaxV         = 155;
    pstMpiDynaRegCfg->u8skinMinV         = 135;
    pstMpiDynaRegCfg->u16oMaxGain        = 160;
    pstMpiDynaRegCfg->u16uMaxGain        = 160;
    pstMpiDynaRegCfg->u8detailOshtThr[0] = 65;
    pstMpiDynaRegCfg->u8detailOshtThr[1] = 90;
    pstMpiDynaRegCfg->u8detailUshtThr[0] = 65;
    pstMpiDynaRegCfg->u8detailUshtThr[1] = 90;

    for ( i = 0; i < ISP_SHARPEN_LUMA_NUM; i++ )
    {
        pstMpiDynaRegCfg->au8LumaWgt[i]  = 127;
    }

    pstMpiDynaRegCfg->u32UpdateIndex     = 1;
    pstMpiDynaRegCfg->u8BufId            = 0;
    pstMpiDynaRegCfg->bResh              = HI_TRUE;
    pstMpiDynaRegCfg->bPreRegNewEn       = HI_FALSE;

}

//****Sharpen hardware Regs that will change only with ISO****//
static HI_VOID SharpenDefaultDynaRegInit(ISP_SHARPEN_DEFAULT_DYNA_REG_CFG_S *pstDefaultDynaRegCfg)
{
    pstDefaultDynaRegCfg->bResh               = HI_TRUE;
    pstDefaultDynaRegCfg->u8mfThdSftD         = 0;
    pstDefaultDynaRegCfg->u8mfThdSelUD        = 2;
    pstDefaultDynaRegCfg->u8mfThdSftUD        = 0;
    pstDefaultDynaRegCfg->u8hfThdSftD         = 0;
    pstDefaultDynaRegCfg->u8hfThdSelUD        = 2;
    pstDefaultDynaRegCfg->u8hfThdSftUD        = 0;
    pstDefaultDynaRegCfg->u8dirVarSft         = 10;
    pstDefaultDynaRegCfg->u8rangeThd0         = 25;
    pstDefaultDynaRegCfg->u8rangeDirWgt0      = 0;
    pstDefaultDynaRegCfg->u8hfEpsVarThr0      = 20;
    pstDefaultDynaRegCfg->u8hfEpsVarThr1      = 30;
    pstDefaultDynaRegCfg->u8hfEpsVal1         = 20;
    pstDefaultDynaRegCfg->u8selPixWgt         = 31;
    pstDefaultDynaRegCfg->u8oshtVarDiffThd[0] = 20;
    pstDefaultDynaRegCfg->u8ushtVarDiffThd[0] = 20;
    pstDefaultDynaRegCfg->u8oshtVarWgt0       = 0;
    pstDefaultDynaRegCfg->u8ushtVarWgt0       = 0;
    pstDefaultDynaRegCfg->u8oshtVarDiffThd[1] = 35;
    pstDefaultDynaRegCfg->u8oshtVarDiffWgt1   = 5;
    pstDefaultDynaRegCfg->u8ushtVarDiffThd[1] = 35;
    pstDefaultDynaRegCfg->u8ushtVarDiffWgt1   = 10;
    pstDefaultDynaRegCfg->u8RmfGainScale      = 16;
    pstDefaultDynaRegCfg->u8BmfGainScale      = 10;
    //pstDefaultDynaRegCfg->u16chrGGain         = 32;
    //pstDefaultDynaRegCfg->u16chrGmfGain       = 32;

}

static HI_VOID SharpenDynaRegInit(ISP_SHARPEN_REG_CFG_S *pstSharpenRegCfg)
{
    ISP_SHARPEN_DEFAULT_DYNA_REG_CFG_S *pstDefaultDynaRegCfg = HI_NULL;
    ISP_SHARPEN_MPI_DYNA_REG_CFG_S     *pstMpiDynaRegCfg = HI_NULL;
    ISP_SHARPEN_STATIC_REG_CFG_S       *pstStaticRegCfg = HI_NULL;

    pstDefaultDynaRegCfg = &(pstSharpenRegCfg->stDynaRegCfg.stDefaultDynaRegCfg);
    pstMpiDynaRegCfg     = &pstSharpenRegCfg->stDynaRegCfg.stMpiDynaRegCfg;
    pstStaticRegCfg      = &pstSharpenRegCfg->stStaticRegCfg;

    SharpenDefaultDynaRegInit(pstDefaultDynaRegCfg);
    SharpenMpiDynaRegInit(pstMpiDynaRegCfg);

    /* Calc all MulCoef */
    // Mpi
    pstMpiDynaRegCfg->u16oshtVarMul  = CalcMulCoef( pstStaticRegCfg->u8oshtVarThd0,  pstDefaultDynaRegCfg->u8oshtVarWgt0,
                                                    pstMpiDynaRegCfg->u8oshtVarThd1, pstStaticRegCfg->u8oshtVarWgt1,
                                                    SHRP_SHT_VAR_MUL_PRECS );

    pstMpiDynaRegCfg->u16ushtVarMul  = CalcMulCoef( pstStaticRegCfg->u8ushtVarThd0,  pstDefaultDynaRegCfg->u8ushtVarWgt0,
                                                    pstMpiDynaRegCfg->u8ushtVarThd1, pstStaticRegCfg->u8ushtVarWgt1,
                                                    SHRP_SHT_VAR_MUL_PRECS );
    pstMpiDynaRegCfg->s16chrRMul     = CalcMulCoef( pstStaticRegCfg->u8chrRThd[0], pstMpiDynaRegCfg->u8chrRGain,
                                                    pstStaticRegCfg->u8chrRThd[1], 32,
                                                    SHRP_CHR_MUL_SFT );
    pstMpiDynaRegCfg->s16chrGMul     = CalcMulCoef( pstStaticRegCfg->u8chrGThd[0], pstMpiDynaRegCfg->u16chrGGain,
                                                    pstStaticRegCfg->u8chrGThd[1], 32,
                                                    SHRP_CHR_MUL_SFT );
    pstMpiDynaRegCfg->s16chrGmfMul   = CalcMulCoef( pstStaticRegCfg->u8chrGThd[0], pstMpiDynaRegCfg->u16chrGmfGain,
                                                    pstStaticRegCfg->u8chrGThd[1], 32,
                                                    SHRP_CHR_MUL_SFT );
    pstMpiDynaRegCfg->s16chrBMul     = CalcMulCoef( pstStaticRegCfg->u8chrBThd[0], pstMpiDynaRegCfg->u8chrBGain,
                                                    pstStaticRegCfg->u8chrBThd[1], 32,
                                                    SHRP_CHR_MUL_SFT );

    pstMpiDynaRegCfg->s16skinEdgeMul = CalcMulCoef( pstStaticRegCfg->u8skinEdgeThd[0], pstMpiDynaRegCfg->u8skinEdgeWgt[0],
                                                    pstStaticRegCfg->u8skinEdgeThd[1], pstMpiDynaRegCfg->u8skinEdgeWgt[1],
                                                    SHRP_SKIN_EDGE_MUL_PRECS );

   pstMpiDynaRegCfg->s16detailOshtMul = CalcMulCoef( pstMpiDynaRegCfg->u8detailOshtThr[0], pstMpiDynaRegCfg->u8detailOshtAmt,
                                                      pstMpiDynaRegCfg->u8detailOshtThr[1], pstMpiDynaRegCfg->u8oshtAmt,
                                                      SHRP_DETAIL_SHT_MUL_PRECS );

    pstMpiDynaRegCfg->s16detailUshtMul = CalcMulCoef( pstMpiDynaRegCfg->u8detailUshtThr[0], pstMpiDynaRegCfg->u8detailUshtAmt,
                                                      pstMpiDynaRegCfg->u8detailUshtThr[1], pstMpiDynaRegCfg->u8ushtAmt,
                                                      SHRP_DETAIL_SHT_MUL_PRECS );

    // Defalut
    pstDefaultDynaRegCfg->s16hfEpsMul       = CalcMulCoef( pstDefaultDynaRegCfg->u8hfEpsVarThr0, pstStaticRegCfg->u8hfEpsVal0,
                                                           pstDefaultDynaRegCfg->u8hfEpsVarThr1, pstDefaultDynaRegCfg->u8hfEpsVal1,
                                                           SHRP_HF_EPS_MUL_PRECS);
    pstDefaultDynaRegCfg->s16oshtVarDiffMul = CalcMulCoef( pstDefaultDynaRegCfg->u8oshtVarDiffThd[0], pstStaticRegCfg->u8oshtVarDiffWgt0,
                                                           pstDefaultDynaRegCfg->u8oshtVarDiffThd[1], pstDefaultDynaRegCfg->u8oshtVarDiffWgt1,
                                                           SHRP_SHT_VAR_MUL_PRECS );

    pstDefaultDynaRegCfg->s16ushtVarDiffMul = CalcMulCoef( pstDefaultDynaRegCfg->u8ushtVarDiffThd[0], pstStaticRegCfg->u8ushtVarDiffWgt0,
                                                           pstDefaultDynaRegCfg->u8ushtVarDiffThd[1], pstDefaultDynaRegCfg->u8ushtVarDiffWgt1,
                                                           SHRP_SHT_VAR_MUL_PRECS );
    pstDefaultDynaRegCfg->s16rangeDirMul    = CalcMulCoef( pstDefaultDynaRegCfg->u8rangeThd0, pstDefaultDynaRegCfg->u8rangeDirWgt0,
                                                           pstStaticRegCfg->u8rangeThd1, pstStaticRegCfg->u8rangeDirWgt1,
                                                           SHRP_DIR_RANGE_MUL_PRECS );
}

static HI_VOID SharpenRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pRegCfg)
{
    HI_U32 i;

    for (i = 0; i < pRegCfg->u8CfgNum; i++)
    {
        pRegCfg->stAlgRegCfg[i].stSharpenRegCfg.bEnable    = HI_TRUE;
        pRegCfg->stAlgRegCfg[i].stSharpenRegCfg.bLut2SttEn = HI_TRUE;
        SharpenStaticRegInit(ViPipe, &(pRegCfg->stAlgRegCfg[i].stSharpenRegCfg.stStaticRegCfg));
        SharpenDynaRegInit(&(pRegCfg->stAlgRegCfg[i].stSharpenRegCfg));
    }

    pRegCfg->unKey.bit1SharpenCfg = 1;
}

static HI_VOID SharpenReadExtregs(VI_PIPE ViPipe)
{
    HI_U8 i, j;
    ISP_SHARPEN_S *pstSharpen = HI_NULL;

    SHARPEN_GET_CTX(ViPipe, pstSharpen);

    pstSharpen->bSharpenMpiUpdateEn = hi_ext_system_sharpen_mpi_update_en_read(ViPipe);
    pstSharpen->u8SkinUmax   = hi_ext_system_manual_Isp_sharpen_SkinUmax_read(ViPipe);
    pstSharpen->u8SkinUmin   = hi_ext_system_manual_Isp_sharpen_SkinUmin_read(ViPipe);
    pstSharpen->u8SkinVmax   = hi_ext_system_manual_Isp_sharpen_SkinVmax_read(ViPipe);
    pstSharpen->u8SkinVmin   = hi_ext_system_manual_Isp_sharpen_SkinVmin_read(ViPipe);

    hi_ext_system_sharpen_mpi_update_en_write(ViPipe, HI_FALSE);

    if (pstSharpen->bSharpenMpiUpdateEn)
    {
        pstSharpen->u8ManualSharpenYuvEnabled = hi_ext_system_isp_sharpen_manu_mode_read(ViPipe);

        if (pstSharpen->u8ManualSharpenYuvEnabled == OP_TYPE_MANUAL)
        {
            for (j = 0; j < ISP_SHARPEN_GAIN_NUM; j++)
            {
                pstSharpen->au16TextureStr[j] = hi_ext_system_manual_Isp_sharpen_TextureStr_read(ViPipe, j);
                pstSharpen->au16EdgeStr[j]    = hi_ext_system_manual_Isp_sharpen_EdgeStr_read(ViPipe, j);
            }
            for (j = 0; j < ISP_SHARPEN_LUMA_NUM; j++)
            {
                pstSharpen->au8LumaWgt[j]     = hi_ext_system_manual_Isp_sharpen_LumaWgt_read(ViPipe, j);
            }
            pstSharpen->u16TextureFreq    = hi_ext_system_manual_Isp_sharpen_TextureFreq_read(ViPipe);
            pstSharpen->u16EdgeFreq       = hi_ext_system_manual_Isp_sharpen_EdgeFreq_read(ViPipe);
            pstSharpen->u8OverShoot       = hi_ext_system_manual_Isp_sharpen_OverShoot_read(ViPipe);
            pstSharpen->u8UnderShoot      = hi_ext_system_manual_Isp_sharpen_UnderShoot_read(ViPipe);
            pstSharpen->u8ShootSupStr     = hi_ext_system_manual_Isp_sharpen_shootSupStr_read(ViPipe);
            pstSharpen->u8DetailCtrl      = hi_ext_system_manual_Isp_sharpen_detailctrl_read(ViPipe);
            pstSharpen->u8EdgeFiltStr     = hi_ext_system_manual_Isp_sharpen_EdgeFiltStr_read(ViPipe);
            pstSharpen->u8RGain           = hi_ext_system_manual_Isp_sharpen_RGain_read(ViPipe);
            pstSharpen->u8GGain           = hi_ext_system_manual_Isp_sharpen_GGain_read(ViPipe);
            pstSharpen->u8BGain           = hi_ext_system_manual_Isp_sharpen_BGain_read(ViPipe);
            pstSharpen->u8SkinGain        = hi_ext_system_manual_Isp_sharpen_SkinGain_read(ViPipe);
            pstSharpen->u8ShootSupAdj     = hi_ext_system_manual_Isp_sharpen_ShootSupAdj_read(ViPipe);
            pstSharpen->u8DetailCtrlThr   = hi_ext_system_manual_Isp_sharpen_detailctrlThr_read(ViPipe);
            pstSharpen->u16MaxSharpGain   = hi_ext_system_manual_Isp_sharpen_MaxSharpGain_read(ViPipe);
        }
        else
        {
            for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
            {
                for (j = 0; j < ISP_SHARPEN_GAIN_NUM; j++)
                {
                    pstSharpen->au16AutoTextureStr[j][i] = hi_ext_system_Isp_sharpen_TextureStr_read(ViPipe, i + j * ISP_AUTO_ISO_STRENGTH_NUM);
                    pstSharpen->au16AutoEdgeStr[j][i]    = hi_ext_system_Isp_sharpen_EdgeStr_read(ViPipe, i + j * ISP_AUTO_ISO_STRENGTH_NUM);
                }
                for (j = 0; j < ISP_SHARPEN_LUMA_NUM; j++)
                {
                    pstSharpen->au8AutoLumaWgt[j][i]     = hi_ext_system_Isp_sharpen_LumaWgt_read(ViPipe, i + j * ISP_AUTO_ISO_STRENGTH_NUM);
                }
                pstSharpen->au16TextureFreq[i]  = hi_ext_system_Isp_sharpen_TextureFreq_read(ViPipe, i);
                pstSharpen->au16EdgeFreq[i]     = hi_ext_system_Isp_sharpen_EdgeFreq_read(ViPipe, i);
                pstSharpen->au8OverShoot[i]     = hi_ext_system_Isp_sharpen_OverShoot_read(ViPipe, i);
                pstSharpen->au8UnderShoot[i]    = hi_ext_system_Isp_sharpen_UnderShoot_read(ViPipe, i);
                pstSharpen->au8ShootSupStr[i]   = hi_ext_system_Isp_sharpen_shootSupStr_read(ViPipe, i);
                pstSharpen->au8DetailCtrl[i]    = hi_ext_system_Isp_sharpen_detailctrl_read(ViPipe, i);
                pstSharpen->au8EdgeFiltStr[i]   = hi_ext_system_Isp_sharpen_EdgeFiltStr_read(ViPipe, i);
                pstSharpen->au8RGain[i]         = hi_ext_system_Isp_sharpen_RGain_read(ViPipe, i);
                pstSharpen->au8GGain[i]         = hi_ext_system_Isp_sharpen_GGain_read(ViPipe, i);
                pstSharpen->au8BGain[i]         = hi_ext_system_Isp_sharpen_BGain_read(ViPipe, i);
                pstSharpen->au8SkinGain[i]      = hi_ext_system_Isp_sharpen_SkinGain_read(ViPipe, i);
                pstSharpen->au8ShootSupAdj[i]   = hi_ext_system_Isp_sharpen_ShootSupAdj_read(ViPipe, i);
                pstSharpen->au16MaxSharpGain[i] = hi_ext_system_Isp_sharpen_MaxSharpGain_read(ViPipe, i);
                pstSharpen->au8DetailCtrlThr[i] = hi_ext_system_Isp_sharpen_detailctrlThr_read(ViPipe, i);
            }
        }
    }
}

static HI_S32 SharpenReadProMode(VI_PIPE ViPipe)
{
    HI_U8 i, j;
    ISP_CTX_S     *pstIspCtx  = HI_NULL;
    ISP_SHARPEN_S *pstSharpen = HI_NULL;
    HI_U8 u8Index = 0;
    HI_U8 u8IndexMaxValue = 0;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    SHARPEN_GET_CTX(ViPipe, pstSharpen);

    if (HI_TRUE == pstIspCtx->stProShpParamCtrl.pstProShpParam->bEnable)
    {
        u8Index = pstIspCtx->stLinkage.u8ProIndex;
        u8IndexMaxValue = MIN2(pstIspCtx->stProShpParamCtrl.pstProShpParam->u32ParamNum, PRO_MAX_FRAME_NUM);
        if (u8Index > u8IndexMaxValue)
        {
            u8Index =  u8IndexMaxValue;
        }
        if (u8Index < 1)
        {
            return HI_SUCCESS;
        }
        u8Index -= 1;

    }
    else
    {
        return HI_SUCCESS;
    }
    pstSharpen->u8ManualSharpenYuvEnabled = OP_TYPE_AUTO;
    pstSharpen->bSharpenMpiUpdateEn       = HI_TRUE;
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        for (j = 0; j < ISP_SHARPEN_GAIN_NUM; j++)
        {
            pstSharpen->au16AutoTextureStr[j][i] = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au16TextureStr[j][i];
            pstSharpen->au16AutoEdgeStr[j][i]    = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au16EdgeStr[j][i];
        }
        for (j = 0; j < ISP_SHARPEN_LUMA_NUM; j++)
        {
            pstSharpen->au8AutoLumaWgt[j][i]     = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8LumaWgt[j][i];
        }
        pstSharpen->au16TextureFreq[i]  = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au16TextureFreq[i];
        pstSharpen->au16EdgeFreq[i]     = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au16EdgeFreq[i];
        pstSharpen->au8OverShoot[i]     = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8OverShoot[i];
        pstSharpen->au8UnderShoot[i]    = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8UnderShoot[i];
        pstSharpen->au8ShootSupStr[i]   = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8ShootSupStr[i];
        pstSharpen->au8DetailCtrl[i]    = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8DetailCtrl[i];
        pstSharpen->au8EdgeFiltStr[i]   = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8EdgeFiltStr[i];
        pstSharpen->au8RGain[i]         = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8RGain[i];
        pstSharpen->au8GGain[i]         = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8GGain[i];
        pstSharpen->au8BGain[i]         = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8BGain[i];
        pstSharpen->au8SkinGain[i]      = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8SkinGain[i];
        pstSharpen->au8ShootSupAdj[i]   = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8ShootSupAdj[i];
        pstSharpen->au16MaxSharpGain[i] = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au16MaxSharpGain[i];
        pstSharpen->au8DetailCtrlThr[i] = pstIspCtx->stProShpParamCtrl.pstProShpParam->astShpAttr[u8Index].au8DetailCtrlThr[i];
    }
    return HI_SUCCESS;
}
static HI_S32 ISP_SharpenCtxInit(ISP_SHARPEN_S *pstSharpen)
{
    HI_U8 i, j;
    pstSharpen->bSharpenEn          = 1;
    pstSharpen->bSharpenMpiUpdateEn = 1;
    pstSharpen->u32IsoLast          = 0;
    /* Sharpening Yuv */
    //tmp registers
    pstSharpen->u8ManualSharpenYuvEnabled = 1;
    pstSharpen->u8mfThdSftD               = 0;
    pstSharpen->u8dirVarSft               = 10;
    pstSharpen->u8rangeThd0               = 25;
    pstSharpen->u8rangeDirWgt0            = 0;
    pstSharpen->u8selPixWgt               = 31;

    pstSharpen->u8RmfGainScale    = 2;
    pstSharpen->u8BmfGainScale    = 4;
    //pstSharpen->u16chrGGain       = 32;
    //pstSharpen->u16chrGmfGain     = 32;

    pstSharpen->u8mfThdSelUD      = 2;
    pstSharpen->u8mfThdSftUD      = 0;
    pstSharpen->u8hfEpsVarThr0    = 20;
    pstSharpen->u8hfEpsVarThr1    = 30;
    pstSharpen->u8hfEpsVal1       = 20;

    pstSharpen->u8oshtVarWgt0     = 10;
    pstSharpen->u8ushtVarWgt0     = 20;
    pstSharpen->u8oshtVarDiffThd0 = 20;
    pstSharpen->u8oshtVarDiffThd1 = 35;
    pstSharpen->u8oshtVarDiffWgt1 = 20;
    pstSharpen->u8ushtVarDiffWgt1 = 35;
    //MPI
    pstSharpen->u8SkinUmax      = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINUMAX_DEFAULT;
    pstSharpen->u8SkinUmin      = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINUMIN_DEFAULT;
    pstSharpen->u8SkinVmin      = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINVMIN_DEFAULT;
    pstSharpen->u8SkinVmax      = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINVMAX_DEFAULT;
    for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
    {
        pstSharpen->au16TextureStr[i] = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_TEXTURESTR_DEFAULT;
        pstSharpen->au16EdgeStr[i]    = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGESTR_DEFAULT;
    }
    for (i = 0; i < ISP_SHARPEN_LUMA_NUM; i++)
    {
        pstSharpen->au8LumaWgt[i]     = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_LUMAWGT_DEFAULT;
    }
    pstSharpen->u16TextureFreq  = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_TEXTUREFREQ_DEFAULT;         //Texture frequency adjustment. Texture and detail will be finer when it increase.    U6.6  [0, 4095]
    pstSharpen->u16EdgeFreq     = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGEFREQ_DEFAULT;            //Edge frequency adjustment. Edge will be narrower and thiner when it increase.      U6.6  [0, 4095]
    pstSharpen->u8OverShoot     = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_OVERSHOOT_DEFAULT;          //u8OvershootAmt     U7.0  [0, 127]
    pstSharpen->u8UnderShoot    = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_UNDERSHOOT_DEFAULT;         //u8UndershootAmt       U7.0  [0, 127]
    pstSharpen->u8ShootSupStr   = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SHOOTSUPSTR_DEFAULT;        //overshoot and undershoot suppression strength, the amplitude and width of shoot will be decrease when shootSupSt increase.  U10.0    [0, 255]
    pstSharpen->u8DetailCtrl    = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_DETAILCTRL_DEFAULT;
    pstSharpen->u8EdgeFiltStr   = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGEFILTSTR_DEFAULT;
    pstSharpen->u8RGain         = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_RGAIN_DEFAULT;
    pstSharpen->u8GGain         = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_GGAIN_DEFAULT;
    pstSharpen->u8BGain         = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_BGAIN_DEFAULT;
    pstSharpen->u8SkinGain      = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINGAIN_DEFAULT;
    pstSharpen->u8ShootSupAdj   = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SHOOTSUPADJ_DEFAULT;
    pstSharpen->u16MaxSharpGain = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_MAXSHARPGAIN_DEFAULT;
    pstSharpen->u8DetailCtrlThr = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_DETAILCTRLTHR_DEFAULT;

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        for (j = 0; j < ISP_SHARPEN_GAIN_NUM ; j++)
        {
            pstSharpen->au16AutoTextureStr[j][i] = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_TEXTURESTR_DEFAULT;
            pstSharpen->au16AutoEdgeStr[j][i]    = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGESTR_DEFAULT;
        }
        for (j = 0; j < ISP_SHARPEN_LUMA_NUM; j++)
        {
            pstSharpen->au8AutoLumaWgt[j][i]     = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_LUMAWGT_DEFAULT;
        }
        pstSharpen->au16TextureFreq[i]  = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_TEXTUREFREQ_DEFAULT;         //Texture frequency adjustment. Texture and detail will be finer when it increase.    U6.6  [0, 4095]
        pstSharpen->au16EdgeFreq[i]     = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGEFREQ_DEFAULT;        //Edge frequency adjustment. Edge will be narrower and thiner when it increase.      U6.6  [0, 4095]
        pstSharpen->au8OverShoot[i]     = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_OVERSHOOT_DEFAULT;          //u8OvershootAmt     U7.0  [0, 127]
        pstSharpen->au8UnderShoot[i]    = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_UNDERSHOOT_DEFAULT;         //u8UndershootAmt       U7.0  [0, 127]
        pstSharpen->au8ShootSupStr[i]   = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SHOOTSUPSTR_DEFAULT;        //overshoot and undershoot suppression strength, the amplitude and width of shoot will be decrease when shootSupSt increase.  U10.0    [0, 1023]
        pstSharpen->au8DetailCtrl[i]    = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_DETAILCTRL_DEFAULT;
        pstSharpen->au8EdgeFiltStr[i]   = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_EDGEFILTSTR_DEFAULT;
        pstSharpen->au8RGain[i]         = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_RGAIN_DEFAULT;
        pstSharpen->au8GGain[i]         = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_GGAIN_DEFAULT;
        pstSharpen->au8BGain[i]         = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_BGAIN_DEFAULT;
        pstSharpen->au8SkinGain[i]      = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SKINGAIN_DEFAULT;
        pstSharpen->au8ShootSupAdj[i]   = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_SHOOTSUPADJ_DEFAULT;
        pstSharpen->au16MaxSharpGain[i] = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_MAXSHARPGAIN_DEFAULT;
        pstSharpen->au8DetailCtrlThr[i] = HI_EXT_SYSTEM_MANUAL_ISP_SHARPEN_DETAILCTRLTHR_DEFAULT;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SharpenInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_SHARPEN_S *pstSharpen = HI_NULL;

    s32Ret = SharpenCtxInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    SHARPEN_GET_CTX(ViPipe, pstSharpen);
    ISP_CHECK_POINTER(pstSharpen);

    pstSharpen->u8ManualSharpenYuvEnabled = 1;
    pstSharpen->bSharpenEn                = HI_TRUE;
    pstSharpen->u32IsoLast                = 0;

    ISP_SharpenCtxInit(pstSharpen);
    SharpenRegsInitialize(ViPipe, (ISP_REG_CFG_S *)pRegCfg);
    s32Ret = SharpenExtRegsInitialize(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_VOID ISP_SharpenWdrModeSet(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_U8  i;
    HI_U32 au32UpdateIdx[ISP_STRIPING_MAX_NUM] = {0};
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        au32UpdateIdx[i] = pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.u32UpdateIndex;
    }

    ISP_SharpenInit(ViPipe, pRegCfg);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.u32UpdateIndex = au32UpdateIdx[i] + 1;
        pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.bSwitchMode    = HI_TRUE;
    }
}

static HI_VOID ISP_Sharpen_GetLinearDefaultRegCfg(ISP_SHARPEN_S *pstSharpenPara, HI_U32 u32ISO, HI_U32  idxCur, HI_U32 idxPre, HI_U32  isoLvlCur, HI_U32 isoLvlPre)
{
    //Linear mode defalt regs
    const HI_U8  u8mfThdSelUDLinear[ISP_AUTO_ISO_STRENGTH_NUM]      = {2 ,    2,    2,     2,    2,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1};
    const HI_U8  u8mfThdSftUDLinear[ISP_AUTO_ISO_STRENGTH_NUM]      = {0,     0,    0,     0,    0,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1};
    const HI_U8  u8hfEpsVarThr0Linear[ISP_AUTO_ISO_STRENGTH_NUM]    = {20,   20,   22,    25,   28,   30,   30,   40,   50,   60,   60,   60,   60,   60,   60,   60};
    const HI_U8  u8hfEpsVarThr1Linear[ISP_AUTO_ISO_STRENGTH_NUM]    = {30,   30,   32,    35,   38,   40,   40,   50,   60,   70,   70,   70,   70,   70,   70,   70};

    const HI_U8  u8oshtVarWgt0Linear[ISP_AUTO_ISO_STRENGTH_NUM]     = {20   ,   20   ,   20   ,    20   ,   20    ,   20    ,   20    ,   20     ,   20    ,   20    ,   20    ,   20    ,   20    ,   20    ,   20    ,   20};
    const HI_U8  u8ushtVarWgt0Linear[ISP_AUTO_ISO_STRENGTH_NUM]     = {30   ,   30   ,   30   ,    30   ,   30    ,   30    ,   30    ,   30     ,   30    ,   30    ,   30    ,   30    ,   30    ,   30    ,   30    ,   30};

    const HI_U8  u8oshtVarDiffThd0Linear[ISP_AUTO_ISO_STRENGTH_NUM] = {20,   22,   23,    25,   28,   30,   32,   36,   37,   38,   39,   40,   40,   40,   40,   40};
    const HI_U8  u8oshtVarDiffThd1Linear[ISP_AUTO_ISO_STRENGTH_NUM] = {35,   37,   38,    40,   40,   43,   43,   46,   47,   48,   49,   50,   50,   50,   50,   50};
    const HI_U8  u8oshtVarDiffWgt1Linear[ISP_AUTO_ISO_STRENGTH_NUM] = {0 ,    0,    0,     0,    5,   10,   15,   18,   20,   20,   20,   20,   20,   20,   20,   20};
    const HI_U8  u8ushtVarDiffWgt1Linear[ISP_AUTO_ISO_STRENGTH_NUM] = {30,   30,   30,    30,   30,   30,   30,   30,   30,   30,   20,   10,   10,   10,   10,   10};

    HI_U16 WgtPre = 0;
    HI_U16 WgtCur = 0;   //U0.8
    HI_U8  sft = 8;

    //pstSharpenPara->u8mfThdSelUD    =  u8mfThdSelUDLinear[idxCur];

    if (u32ISO <= isoLvlPre)
    {
	    pstSharpenPara->u8mfThdSelUD   =  u8mfThdSelUDLinear[idxPre];
        pstSharpenPara->u8mfThdSftUD   =  u8mfThdSftUDLinear[idxPre];
        pstSharpenPara->u8hfEpsVarThr0 =  u8hfEpsVarThr0Linear[idxPre];
        pstSharpenPara->u8hfEpsVarThr1 =  u8hfEpsVarThr1Linear[idxPre];
        pstSharpenPara->u8oshtVarWgt0  =  u8oshtVarWgt0Linear[idxPre];
        pstSharpenPara->u8ushtVarWgt0  =  u8ushtVarWgt0Linear[idxPre];
        pstSharpenPara->u8oshtVarDiffThd0 =  u8oshtVarDiffThd0Linear[idxPre];
        pstSharpenPara->u8oshtVarDiffThd1 =  u8oshtVarDiffThd1Linear[idxPre];
        pstSharpenPara->u8oshtVarDiffWgt1 =  u8oshtVarDiffWgt1Linear[idxPre];
        pstSharpenPara->u8ushtVarDiffWgt1 =  u8ushtVarDiffWgt1Linear[idxPre];
    }
    else if (u32ISO >= isoLvlCur)
    {
	    pstSharpenPara->u8mfThdSelUD   =  u8mfThdSelUDLinear[idxCur];
        pstSharpenPara->u8mfThdSftUD   =  u8mfThdSftUDLinear[idxCur];
        pstSharpenPara->u8hfEpsVarThr0 =  u8hfEpsVarThr0Linear[idxCur];
        pstSharpenPara->u8hfEpsVarThr1 =  u8hfEpsVarThr1Linear[idxCur];
        pstSharpenPara->u8oshtVarWgt0  =  u8oshtVarWgt0Linear[idxCur];
        pstSharpenPara->u8ushtVarWgt0  =  u8ushtVarWgt0Linear[idxCur];
        pstSharpenPara->u8oshtVarDiffThd0 =  u8oshtVarDiffThd0Linear[idxCur];
        pstSharpenPara->u8oshtVarDiffThd1 =  u8oshtVarDiffThd1Linear[idxCur];
        pstSharpenPara->u8oshtVarDiffWgt1 =  u8oshtVarDiffWgt1Linear[idxCur];
        pstSharpenPara->u8ushtVarDiffWgt1 =  u8ushtVarDiffWgt1Linear[idxCur];

    }
    else
    {
        WgtPre = SignedLeftShift((isoLvlCur - u32ISO), sft)/(isoLvlCur - isoLvlPre);
        WgtCur = SignedLeftShift(1, sft) - WgtPre;

		pstSharpenPara->u8mfThdSelUD      =  shrp_blend(sft, WgtPre, u8mfThdSelUDLinear[idxPre],      WgtCur, u8mfThdSelUDLinear[idxCur]);
        pstSharpenPara->u8mfThdSftUD      =  shrp_blend(sft, WgtPre, u8mfThdSftUDLinear[idxPre],      WgtCur, u8mfThdSftUDLinear[idxCur]);
        pstSharpenPara->u8hfEpsVarThr0    =  shrp_blend(sft, WgtPre, u8hfEpsVarThr0Linear[idxPre],    WgtCur, u8hfEpsVarThr0Linear[idxCur]);
        pstSharpenPara->u8hfEpsVarThr1    =  shrp_blend(sft, WgtPre, u8hfEpsVarThr1Linear[idxPre],    WgtCur, u8hfEpsVarThr1Linear[idxCur]);
        pstSharpenPara->u8oshtVarWgt0     =  shrp_blend(sft, WgtPre, u8oshtVarWgt0Linear[idxPre],     WgtCur, u8oshtVarWgt0Linear[idxCur]);
        pstSharpenPara->u8ushtVarWgt0     =  shrp_blend(sft, WgtPre, u8ushtVarWgt0Linear[idxPre],     WgtCur, u8ushtVarWgt0Linear[idxCur]);
        pstSharpenPara->u8oshtVarDiffThd0 =  shrp_blend(sft, WgtPre, u8oshtVarDiffThd0Linear[idxPre], WgtCur, u8oshtVarDiffThd0Linear[idxCur]);
        pstSharpenPara->u8oshtVarDiffThd1 =  shrp_blend(sft, WgtPre, u8oshtVarDiffThd1Linear[idxPre], WgtCur, u8oshtVarDiffThd1Linear[idxCur]);
        pstSharpenPara->u8oshtVarDiffWgt1 =  shrp_blend(sft, WgtPre, u8oshtVarDiffWgt1Linear[idxPre], WgtCur, u8oshtVarDiffWgt1Linear[idxCur]);
        pstSharpenPara->u8ushtVarDiffWgt1 =  shrp_blend(sft, WgtPre, u8ushtVarDiffWgt1Linear[idxPre], WgtCur, u8ushtVarDiffWgt1Linear[idxCur]);

    }

}

static HI_VOID ISP_Sharpen_GetWdrDefaultRegCfg(ISP_SHARPEN_S *pstSharpenPara, HI_U32 u32ISO, HI_U32  idxCur, HI_U32 idxPre, HI_U32  isoLvlCur, HI_U32 isoLvlPre)
{
    //WDR mode defalt regs
    const HI_U8  u8mfThdSelUDWdr[ISP_AUTO_ISO_STRENGTH_NUM]      = { 2,    2,    2,     2,    2,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1};
    const HI_U8  u8mfThdSftUDWdr[ISP_AUTO_ISO_STRENGTH_NUM]      = { 0,    0,    0,     0,    0,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1};
    const HI_U8  u8hfEpsVarThr0Wdr[ISP_AUTO_ISO_STRENGTH_NUM]    = {20,   20,   22,    25,   28,   30,   30,   40,   50,   60,   60,   60,   60,   60,   60,   60};
    const HI_U8  u8hfEpsVarThr1Wdr[ISP_AUTO_ISO_STRENGTH_NUM]    = {30,   30,   32,    35,   38,   40,   40,   50,   60,   70,   70,   70,   70,   70,   70,   70};

    const HI_U8  u8oshtVarWgt0Wdr[ISP_AUTO_ISO_STRENGTH_NUM]     = {60,   60,   60,    60,   60,   50,   20,   20,   20,   20,   20,   20,   20,   20,   20,   20};
    const HI_U8  u8ushtVarWgt0Wdr[ISP_AUTO_ISO_STRENGTH_NUM]     = {70,   70,   70,    70,   70,   60,   30,   30,   30,   30,   30,   30,   30,   30,   30,   30};
    const HI_U8  u8oshtVarDiffThd0Wdr[ISP_AUTO_ISO_STRENGTH_NUM] = {20,   22,   23,    25,   28,   30,   32,   36,   37,   38,   39,   40,   40,   40,   40,   40};
    const HI_U8  u8oshtVarDiffThd1Wdr[ISP_AUTO_ISO_STRENGTH_NUM] = {35,   37,   38,    40,   40,   43,   43,   46,   47,   48,   49,   50,   50,   50,   50,   50};
    const HI_U8  u8oshtVarDiffWgt1Wdr[ISP_AUTO_ISO_STRENGTH_NUM] = { 5,    5,    5,     5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5};
    const HI_U8  u8ushtVarDiffWgt1Wdr[ISP_AUTO_ISO_STRENGTH_NUM] = {10,   10,   10,    10,   10,   10,   10,   10,   10,   10,   10,   10,   10,   10,   10,   10};

    HI_U16 WgtPre = 0;
    HI_U16 WgtCur = 0;   //U0.8
    HI_U8  sft = 8;

    //pstSharpenPara->u8mfThdSelUD    =  u8mfThdSelUDWdr[idxCur];

    if (u32ISO <= isoLvlPre)
    {
	    pstSharpenPara->u8mfThdSelUD    =  u8mfThdSelUDWdr[idxPre];
        pstSharpenPara->u8mfThdSftUD   =  u8mfThdSftUDWdr[idxPre];
        pstSharpenPara->u8hfEpsVarThr0 =  u8hfEpsVarThr0Wdr[idxPre];
        pstSharpenPara->u8hfEpsVarThr1 =  u8hfEpsVarThr1Wdr[idxPre];
        pstSharpenPara->u8oshtVarWgt0  =  u8oshtVarWgt0Wdr[idxPre];
        pstSharpenPara->u8ushtVarWgt0  =  u8ushtVarWgt0Wdr[idxPre];
        pstSharpenPara->u8oshtVarDiffThd0 =  u8oshtVarDiffThd0Wdr[idxPre];
        pstSharpenPara->u8oshtVarDiffThd1 =  u8oshtVarDiffThd1Wdr[idxPre];
        pstSharpenPara->u8oshtVarDiffWgt1 =  u8oshtVarDiffWgt1Wdr[idxPre];
        pstSharpenPara->u8ushtVarDiffWgt1 =  u8ushtVarDiffWgt1Wdr[idxPre];
    }
    else if (u32ISO >= isoLvlCur)
    {
	    pstSharpenPara->u8mfThdSelUD    =  u8mfThdSelUDWdr[idxCur];
        pstSharpenPara->u8mfThdSftUD   =  u8mfThdSftUDWdr[idxCur];
        pstSharpenPara->u8hfEpsVarThr0 =  u8hfEpsVarThr0Wdr[idxCur];
        pstSharpenPara->u8hfEpsVarThr1 =  u8hfEpsVarThr1Wdr[idxCur];
        pstSharpenPara->u8oshtVarWgt0  =  u8oshtVarWgt0Wdr[idxCur];
        pstSharpenPara->u8ushtVarWgt0  =  u8ushtVarWgt0Wdr[idxCur];
        pstSharpenPara->u8oshtVarDiffThd0 =  u8oshtVarDiffThd0Wdr[idxCur];
        pstSharpenPara->u8oshtVarDiffThd1 =  u8oshtVarDiffThd1Wdr[idxCur];
        pstSharpenPara->u8oshtVarDiffWgt1 =  u8oshtVarDiffWgt1Wdr[idxCur];
        pstSharpenPara->u8ushtVarDiffWgt1 =  u8ushtVarDiffWgt1Wdr[idxCur];

    }
    else
    {
        WgtPre = SignedLeftShift((isoLvlCur - u32ISO), sft)/(isoLvlCur - isoLvlPre);
        WgtCur = SignedLeftShift(1, sft) - WgtPre;

        pstSharpenPara->u8mfThdSelUD      =  shrp_blend(sft, WgtPre, u8mfThdSelUDWdr[idxPre],      WgtCur, u8mfThdSelUDWdr[idxCur]);
        pstSharpenPara->u8mfThdSftUD      =  shrp_blend(sft, WgtPre, u8mfThdSftUDWdr[idxPre],      WgtCur, u8mfThdSftUDWdr[idxCur]);
        pstSharpenPara->u8hfEpsVarThr0    =  shrp_blend(sft, WgtPre, u8hfEpsVarThr0Wdr[idxPre],    WgtCur, u8hfEpsVarThr0Wdr[idxCur]);
        pstSharpenPara->u8hfEpsVarThr1    =  shrp_blend(sft, WgtPre, u8hfEpsVarThr1Wdr[idxPre],    WgtCur, u8hfEpsVarThr1Wdr[idxCur]);
        pstSharpenPara->u8oshtVarWgt0     =  shrp_blend(sft, WgtPre, u8oshtVarWgt0Wdr[idxPre],     WgtCur, u8oshtVarWgt0Wdr[idxCur]);
        pstSharpenPara->u8ushtVarWgt0     =  shrp_blend(sft, WgtPre, u8ushtVarWgt0Wdr[idxPre],     WgtCur, u8ushtVarWgt0Wdr[idxCur]);
        pstSharpenPara->u8oshtVarDiffThd0 =  shrp_blend(sft, WgtPre, u8oshtVarDiffThd0Wdr[idxPre], WgtCur, u8oshtVarDiffThd0Wdr[idxCur]);
        pstSharpenPara->u8oshtVarDiffThd1 =  shrp_blend(sft, WgtPre, u8oshtVarDiffThd1Wdr[idxPre], WgtCur, u8oshtVarDiffThd1Wdr[idxCur]);
        pstSharpenPara->u8oshtVarDiffWgt1 =  shrp_blend(sft, WgtPre, u8oshtVarDiffWgt1Wdr[idxPre], WgtCur, u8oshtVarDiffWgt1Wdr[idxCur]);
        pstSharpenPara->u8ushtVarDiffWgt1 =  shrp_blend(sft, WgtPre, u8ushtVarDiffWgt1Wdr[idxPre], WgtCur, u8ushtVarDiffWgt1Wdr[idxCur]);

    }

}

static HI_VOID ISP_Sharpen_GetDefaultRegCfg(VI_PIPE ViPipe, HI_U32 u32ISO)
{
    //Common Regs
    const HI_U8  u8mfThdSftD[ISP_AUTO_ISO_STRENGTH_NUM]    = {  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1};
    const HI_U8  u8dirVarSft[ISP_AUTO_ISO_STRENGTH_NUM]    = { 10,  10,  10,  10,  10,  10,  10,   8,   7,   6,   5,   4,   3,   3,   3,   3};
    const HI_U8  u8rangeThd0[ISP_AUTO_ISO_STRENGTH_NUM]    = { 10,  10,  15,  22,  25,  25,  25,  25,  25,  25,  25,  25,  25,  25,  25,  25};
    const HI_U8  u8rangeDirWgt0[ISP_AUTO_ISO_STRENGTH_NUM] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127};
    const HI_U8  u8hfEpsVal1[ISP_AUTO_ISO_STRENGTH_NUM]    = { 20,  20,  20,  20,  20,  20,  20,  50,  90, 120, 120, 120, 120, 120, 120, 120};

    const HI_U8  u8selPixWgt[ISP_AUTO_ISO_STRENGTH_NUM]    = { 31,  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,  31};
    const HI_U16 u8RmfGainScale[ISP_AUTO_ISO_STRENGTH_NUM] = { 24,  24,  24,  24,  24,  20,  14,  10,   6,   4,   2,   2,   2,   2,   2,   2};
    const HI_U16 u8BmfGainScale[ISP_AUTO_ISO_STRENGTH_NUM] = { 16,  12,  10,   6,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4};
    //const HI_U16 u16chrGGain[ISP_AUTO_ISO_STRENGTH_NUM]    = { 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32};
    //const HI_U16 u16chrGmfGain[ISP_AUTO_ISO_STRENGTH_NUM]  = { 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32};

    HI_U32 idxCur, idxPre;
    HI_U32 isoLvlCur, isoLvlPre;
    HI_U8  u8WDRMode;

    HI_U16 WgtPre = 0;   //linerinter
    HI_U16 WgtCur = 0;   //linerinter
    HI_U8  sft = 8;      //linerinter

    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_SHARPEN_S *pstSharpenPara = HI_NULL;

    SHARPEN_GET_CTX(ViPipe, pstSharpenPara);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    u8WDRMode = pstIspCtx->u8SnsWDRMode;

    // Get ISO category index
    // idxCur : current index
    // idxPre : previous level index
    idxCur = GetIsoIndex(u32ISO);
    idxPre = (idxCur == 0) ? 0 : MAX2( idxCur - 1, 0 );

    isoLvlCur   =  g_au32IsoLut[idxCur];
    isoLvlPre   =  g_au32IsoLut[idxPre];
    /* Common default regs */
    if (u32ISO <= isoLvlPre)
    {
        pstSharpenPara->u8mfThdSftD    =  u8mfThdSftD[idxPre];
        pstSharpenPara->u8dirVarSft    =  u8dirVarSft[idxPre];
        pstSharpenPara->u8rangeThd0    =  u8rangeThd0[idxPre];
        pstSharpenPara->u8rangeDirWgt0 =  u8rangeDirWgt0[idxPre];
        pstSharpenPara->u8hfEpsVal1    =  u8hfEpsVal1[idxPre];
        pstSharpenPara->u8selPixWgt    =  u8selPixWgt[idxPre];
        pstSharpenPara->u8RmfGainScale =  u8RmfGainScale[idxPre];
        pstSharpenPara->u8BmfGainScale =  u8BmfGainScale[idxPre];
        //pstSharpenPara->u16chrGGain    =  u16chrGGain[idxPre];
        //pstSharpenPara->u16chrGmfGain  =  u16chrGmfGain[idxPre];
    }
    else if (u32ISO >= isoLvlCur)
    {
        pstSharpenPara->u8mfThdSftD    =  u8mfThdSftD[idxCur];
        pstSharpenPara->u8dirVarSft    =  u8dirVarSft[idxCur];
        pstSharpenPara->u8rangeThd0    =  u8rangeThd0[idxCur];
        pstSharpenPara->u8rangeDirWgt0 =  u8rangeDirWgt0[idxCur];
        pstSharpenPara->u8hfEpsVal1    =  u8hfEpsVal1[idxCur];
        pstSharpenPara->u8selPixWgt    =  u8selPixWgt[idxCur];
        pstSharpenPara->u8RmfGainScale =  u8RmfGainScale[idxCur];
        pstSharpenPara->u8BmfGainScale =  u8BmfGainScale[idxCur];
        //pstSharpenPara->u16chrGGain    =  u16chrGGain[idxCur];
        //pstSharpenPara->u16chrGmfGain  =  u16chrGmfGain[idxCur];
    }
    else
    {
        WgtPre = SignedLeftShift((isoLvlCur - u32ISO), sft)/(isoLvlCur - isoLvlPre);
        WgtCur = SignedLeftShift(1, sft) - WgtPre;

        pstSharpenPara->u8mfThdSftD    =  shrp_blend(sft, WgtPre, u8mfThdSftD[idxPre],    WgtCur, u8mfThdSftD[idxCur]);
        pstSharpenPara->u8dirVarSft    =  shrp_blend(sft, WgtPre, u8dirVarSft[idxPre],    WgtCur, u8dirVarSft[idxCur]);
        pstSharpenPara->u8rangeThd0    =  shrp_blend(sft, WgtPre, u8rangeThd0[idxPre],    WgtCur, u8rangeThd0[idxCur]);
        pstSharpenPara->u8rangeDirWgt0 =  shrp_blend(sft, WgtPre, u8rangeDirWgt0[idxPre], WgtCur, u8rangeDirWgt0[idxCur]);
        pstSharpenPara->u8hfEpsVal1    =  shrp_blend(sft, WgtPre, u8hfEpsVal1[idxPre],    WgtCur, u8hfEpsVal1[idxCur]);
        pstSharpenPara->u8selPixWgt    =  shrp_blend(sft, WgtPre, u8selPixWgt[idxPre],    WgtCur, u8selPixWgt[idxCur]);
        pstSharpenPara->u8RmfGainScale =  shrp_blend(sft, WgtPre, u8RmfGainScale[idxPre], WgtCur, u8RmfGainScale[idxCur]);
        pstSharpenPara->u8BmfGainScale =  shrp_blend(sft, WgtPre, u8BmfGainScale[idxPre], WgtCur, u8BmfGainScale[idxCur]);
        //pstSharpenPara->u16chrGGain    =  shrp_blend(sft, WgtPre, u16chrGGain[idxPre],    WgtCur, u16chrGGain[idxCur]);
        //pstSharpenPara->u16chrGmfGain  =  shrp_blend(sft, WgtPre, u16chrGmfGain[idxPre],  WgtCur, u16chrGmfGain[idxCur]);
    }


    /* Linear mode default regs */
    if (IS_LINEAR_MODE(u8WDRMode))
    {
        ISP_Sharpen_GetLinearDefaultRegCfg(pstSharpenPara, u32ISO, idxCur, idxPre, isoLvlCur, isoLvlPre);
    }
    /* WDR mode default regs */
    else //if (IS_WDR_MODE(u8WDRMode))
    {
        ISP_Sharpen_GetWdrDefaultRegCfg(pstSharpenPara, u32ISO, idxCur, idxPre, isoLvlCur, isoLvlPre);
    }
}

static HI_VOID ISP_Sharpen_GetMpiRegCfg(VI_PIPE ViPipe, HI_U32 u32ISO)
{
    HI_U32  i;
    HI_S32  idxCur, idxPre;
    HI_S32  isoLvlCur, isoLvlPre;
    ISP_SHARPEN_S *pstSharpenPara = HI_NULL;

    HI_U16 WgtPre = 0;   //linerinter
    HI_U16 WgtCur = 0;   //linerinter
    HI_U8  sft = 8;      //linerinter prec

    SHARPEN_GET_CTX(ViPipe, pstSharpenPara);

    // Get ISO category index
    // idxCur : current index
    // idxPre : previous level index
    idxCur    = GetIsoIndex(u32ISO);
    idxPre    = MAX2( idxCur - 1, 0 );
    isoLvlCur = g_au32IsoLut[idxCur];
    isoLvlPre = g_au32IsoLut[idxPre];


    // linerinter begain
    if (u32ISO <= isoLvlPre)
    {
        for ( i = 0 ; i < ISP_SHARPEN_GAIN_NUM; i++ )
        {
            pstSharpenPara->au16TextureStr[i] =  pstSharpenPara->au16AutoTextureStr[i][idxPre];
            pstSharpenPara->au16EdgeStr[i]    =  pstSharpenPara->au16AutoEdgeStr[i][idxPre];
        }
        for ( i = 0 ; i < ISP_SHARPEN_LUMA_NUM; i++ )
        {
            pstSharpenPara->au8LumaWgt[i]     =  pstSharpenPara->au8AutoLumaWgt[i][idxPre];
        }
        pstSharpenPara->u16TextureFreq        =  pstSharpenPara->au16TextureFreq[idxPre];
        pstSharpenPara->u16EdgeFreq           =  pstSharpenPara->au16EdgeFreq[idxPre];
        pstSharpenPara->u8OverShoot           =  pstSharpenPara->au8OverShoot[idxPre];
        pstSharpenPara->u8UnderShoot          =  pstSharpenPara->au8UnderShoot[idxPre];
        pstSharpenPara->u8ShootSupStr         =  pstSharpenPara->au8ShootSupStr[idxPre];
        pstSharpenPara->u8DetailCtrl          =  pstSharpenPara->au8DetailCtrl[idxPre];
        pstSharpenPara->u8EdgeFiltStr         =  pstSharpenPara->au8EdgeFiltStr[idxPre];
        pstSharpenPara->u8RGain               =  pstSharpenPara->au8RGain[idxPre];
        pstSharpenPara->u8GGain               =  pstSharpenPara->au8GGain[idxPre];
        pstSharpenPara->u8BGain               =  pstSharpenPara->au8BGain[idxPre];
        pstSharpenPara->u8SkinGain            =  pstSharpenPara->au8SkinGain[idxPre];
        pstSharpenPara->u8ShootSupAdj         =  pstSharpenPara->au8ShootSupAdj[idxPre];
        pstSharpenPara->u8DetailCtrlThr       =  pstSharpenPara->au8DetailCtrlThr[idxPre];
        pstSharpenPara->u16MaxSharpGain       =  pstSharpenPara->au16MaxSharpGain[idxPre];

    }
    else if (u32ISO >= isoLvlCur)
    {
        for ( i = 0 ; i < ISP_SHARPEN_GAIN_NUM; i++ )
        {
            pstSharpenPara->au16TextureStr[i] =  pstSharpenPara->au16AutoTextureStr[i][idxCur];
            pstSharpenPara->au16EdgeStr[i]    =  pstSharpenPara->au16AutoEdgeStr[i][idxCur];
        }
        for ( i = 0 ; i < ISP_SHARPEN_LUMA_NUM; i++ )
        {
            pstSharpenPara->au8LumaWgt[i]     =  pstSharpenPara->au8AutoLumaWgt[i][idxCur];
        }
        pstSharpenPara->u16TextureFreq        =  pstSharpenPara->au16TextureFreq[idxCur];
        pstSharpenPara->u16EdgeFreq           =  pstSharpenPara->au16EdgeFreq[idxCur];
        pstSharpenPara->u8OverShoot           =  pstSharpenPara->au8OverShoot[idxCur];
        pstSharpenPara->u8UnderShoot          =  pstSharpenPara->au8UnderShoot[idxCur];
        pstSharpenPara->u8ShootSupStr         =  pstSharpenPara->au8ShootSupStr[idxCur];
        pstSharpenPara->u8DetailCtrl          =  pstSharpenPara->au8DetailCtrl[idxCur];
        pstSharpenPara->u8EdgeFiltStr         =  pstSharpenPara->au8EdgeFiltStr[idxCur];
        pstSharpenPara->u8RGain               =  pstSharpenPara->au8RGain[idxCur];
        pstSharpenPara->u8GGain               =  pstSharpenPara->au8GGain[idxCur];
        pstSharpenPara->u8BGain               =  pstSharpenPara->au8BGain[idxCur];
        pstSharpenPara->u8SkinGain            =  pstSharpenPara->au8SkinGain[idxCur];
        pstSharpenPara->u8ShootSupAdj         =  pstSharpenPara->au8ShootSupAdj[idxCur];
        pstSharpenPara->u8DetailCtrlThr       =  pstSharpenPara->au8DetailCtrlThr[idxCur];
        pstSharpenPara->u16MaxSharpGain       =  pstSharpenPara->au16MaxSharpGain[idxCur];

    }
    else
    {
        WgtPre = SignedLeftShift((isoLvlCur - u32ISO), sft)/(isoLvlCur - isoLvlPre);
        WgtCur = SignedLeftShift(1, sft) - WgtPre;

        for ( i = 0 ; i < ISP_SHARPEN_GAIN_NUM; i++ )
        {
            pstSharpenPara->au16TextureStr[i] =  shrp_blend(sft, WgtPre, pstSharpenPara->au16AutoTextureStr[i][idxPre],WgtCur, pstSharpenPara->au16AutoTextureStr[i][idxCur]);
            pstSharpenPara->au16EdgeStr[i]    =  shrp_blend(sft, WgtPre, pstSharpenPara->au16AutoEdgeStr[i][idxPre],WgtCur, pstSharpenPara->au16AutoEdgeStr[i][idxCur]);
        }
        for ( i = 0 ; i < ISP_SHARPEN_LUMA_NUM; i++ )
        {
            pstSharpenPara->au8LumaWgt[i]     =  shrp_blend(sft, WgtPre, pstSharpenPara->au8AutoLumaWgt[i][idxPre],WgtCur, pstSharpenPara->au8AutoLumaWgt[i][idxCur]);
        }
        pstSharpenPara->u16TextureFreq        =  shrp_blend(sft, WgtPre, pstSharpenPara->au16TextureFreq[idxPre],WgtCur, pstSharpenPara->au16TextureFreq[idxCur]);
        pstSharpenPara->u16EdgeFreq           =  shrp_blend(sft, WgtPre, pstSharpenPara->au16EdgeFreq[idxPre],WgtCur, pstSharpenPara->au16EdgeFreq[idxCur]);
        pstSharpenPara->u8OverShoot           =  shrp_blend(sft, WgtPre, pstSharpenPara->au8OverShoot[idxPre],WgtCur, pstSharpenPara->au8OverShoot[idxCur]);
        pstSharpenPara->u8UnderShoot          =  shrp_blend(sft, WgtPre, pstSharpenPara->au8UnderShoot[idxPre],WgtCur, pstSharpenPara->au8UnderShoot[idxCur]);
        pstSharpenPara->u8ShootSupStr         =  shrp_blend(sft, WgtPre, pstSharpenPara->au8ShootSupStr[idxPre],WgtCur, pstSharpenPara->au8ShootSupStr[idxCur]);
        pstSharpenPara->u8DetailCtrl          =  shrp_blend(sft, WgtPre, pstSharpenPara->au8DetailCtrl[idxPre],WgtCur, pstSharpenPara->au8DetailCtrl[idxCur]);
        pstSharpenPara->u8EdgeFiltStr         =  shrp_blend(sft, WgtPre, pstSharpenPara->au8EdgeFiltStr[idxPre],WgtCur, pstSharpenPara->au8EdgeFiltStr[idxCur]);
        pstSharpenPara->u8RGain               =  shrp_blend(sft, WgtPre, pstSharpenPara->au8RGain[idxPre],WgtCur, pstSharpenPara->au8RGain[idxCur]);
        pstSharpenPara->u8GGain               =  shrp_blend(sft, WgtPre, pstSharpenPara->au8GGain[idxPre],WgtCur, pstSharpenPara->au8GGain[idxCur]);
        pstSharpenPara->u8BGain               =  shrp_blend(sft, WgtPre, pstSharpenPara->au8BGain[idxPre],WgtCur, pstSharpenPara->au8BGain[idxCur]);
        pstSharpenPara->u8SkinGain            =  shrp_blend(sft, WgtPre, pstSharpenPara->au8SkinGain[idxPre],WgtCur, pstSharpenPara->au8SkinGain[idxCur]);
        pstSharpenPara->u8ShootSupAdj         =  shrp_blend(sft, WgtPre, pstSharpenPara->au8ShootSupAdj[idxPre],WgtCur, pstSharpenPara->au8ShootSupAdj[idxCur]);
        pstSharpenPara->u8DetailCtrlThr       =  shrp_blend(sft, WgtPre, pstSharpenPara->au8DetailCtrlThr[idxPre],WgtCur, pstSharpenPara->au8DetailCtrlThr[idxCur]);
        pstSharpenPara->u16MaxSharpGain       =  shrp_blend(sft, WgtPre, pstSharpenPara->au16MaxSharpGain[idxPre],WgtCur, pstSharpenPara->au16MaxSharpGain[idxCur]);
    }

    // linerinter end

}

static HI_VOID SharpenMPI2Reg(ISP_SHARPEN_REG_CFG_S *pstSharpenRegCfg, ISP_SHARPEN_S *pstSharpen, HI_U32 u32ISO)
{
    HI_U8    i, j ;
    ISP_SHARPEN_DEFAULT_DYNA_REG_CFG_S *pstDefaultDynaRegCfg = HI_NULL;
    ISP_SHARPEN_MPI_DYNA_REG_CFG_S     *pstMpiDynaRegCfg = HI_NULL;
    ISP_SHARPEN_STATIC_REG_CFG_S       *pstStaticRegCfg = HI_NULL;
    ISP_SHARPEN_DYNA_REG_CFG_S         *pstDynaRegCfg = HI_NULL;

    pstDynaRegCfg        = &(pstSharpenRegCfg->stDynaRegCfg);
    pstDefaultDynaRegCfg = &(pstDynaRegCfg->stDefaultDynaRegCfg);
    pstMpiDynaRegCfg     = &(pstDynaRegCfg->stMpiDynaRegCfg);
    pstStaticRegCfg      = &(pstSharpenRegCfg->stStaticRegCfg);

    if (pstDefaultDynaRegCfg->bResh)
    {
        pstDefaultDynaRegCfg->u8mfThdSftD         = pstSharpen->u8mfThdSftD;
        pstDefaultDynaRegCfg->u8mfThdSelUD        = pstSharpen->u8mfThdSelUD;
        pstDefaultDynaRegCfg->u8mfThdSftUD        = pstSharpen->u8mfThdSftUD;
        pstDefaultDynaRegCfg->u8hfThdSftD         = pstDefaultDynaRegCfg->u8mfThdSftD;
        pstDefaultDynaRegCfg->u8hfThdSelUD        = pstDefaultDynaRegCfg->u8mfThdSelUD;
        pstDefaultDynaRegCfg->u8hfThdSftUD        = pstDefaultDynaRegCfg->u8mfThdSftUD;
        pstDefaultDynaRegCfg->u8dirVarSft         = pstSharpen->u8dirVarSft;
        pstDefaultDynaRegCfg->u8rangeThd0         = pstSharpen->u8rangeThd0;
        pstDefaultDynaRegCfg->u8rangeDirWgt0      = pstSharpen->u8rangeDirWgt0;
        pstDefaultDynaRegCfg->u8selPixWgt         = pstSharpen->u8selPixWgt;
        pstDefaultDynaRegCfg->u8oshtVarDiffThd[0] = pstSharpen->u8oshtVarDiffThd0;
        pstDefaultDynaRegCfg->u8ushtVarDiffThd[0] = pstDefaultDynaRegCfg->u8oshtVarDiffThd[0];
        pstDefaultDynaRegCfg->u8oshtVarWgt0       = pstSharpen->u8oshtVarWgt0;
        pstDefaultDynaRegCfg->u8ushtVarWgt0       = pstSharpen->u8ushtVarWgt0;
        pstDefaultDynaRegCfg->u8oshtVarDiffThd[1] = pstSharpen->u8oshtVarDiffThd1;
        pstDefaultDynaRegCfg->u8oshtVarDiffWgt1   = pstSharpen->u8oshtVarDiffWgt1;
        pstDefaultDynaRegCfg->u8ushtVarDiffThd[1] = pstDefaultDynaRegCfg->u8oshtVarDiffThd[1];
        pstDefaultDynaRegCfg->u8ushtVarDiffWgt1   = pstSharpen->u8ushtVarDiffWgt1;
        pstDefaultDynaRegCfg->u8RmfGainScale      = pstSharpen->u8RmfGainScale;
        pstDefaultDynaRegCfg->u8BmfGainScale      = pstSharpen->u8BmfGainScale;
        //pstDefaultDynaRegCfg->u16chrGGain         = pstSharpen->u16chrGGain;
        //pstDefaultDynaRegCfg->u16chrGmfGain       = pstSharpen->u16chrGmfGain;
    }

    // GainD
    if (pstMpiDynaRegCfg->bResh)
    {
        if (pstStaticRegCfg->u8mfThdSelD == 1)////1
        {
            j = 0;
            for (i = 0; i < 16; i++)
            {
                pstMpiDynaRegCfg->u16mfGainD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16EdgeStr[i]));
            }
            for (i = 16; i < 24; i++)
            {
                pstMpiDynaRegCfg->u16mfGainD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16EdgeStr[i]));
                pstMpiDynaRegCfg->u16mfGainD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16EdgeStr[i] + pstSharpen->au16EdgeStr[i + 1]) >> 1)));
            }
            for (i = 24; i < 28; i++)
            {
                pstMpiDynaRegCfg->u16mfGainD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16EdgeStr[i]));
                pstMpiDynaRegCfg->u16mfGainD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16EdgeStr[i]) * 2 + (pstSharpen->au16EdgeStr[i + 1])   ) / 3));
                pstMpiDynaRegCfg->u16mfGainD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16EdgeStr[i])   + (pstSharpen->au16EdgeStr[i + 1]) * 2 ) / 3));
            }
            for (i = 28; i < 31; i++)
            {
                pstMpiDynaRegCfg->u16mfGainD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16EdgeStr[i]));
                pstMpiDynaRegCfg->u16mfGainD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16EdgeStr[i]) * 3 + (pstSharpen->au16EdgeStr[i + 1]) * 1 ) / 5));
                pstMpiDynaRegCfg->u16mfGainD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16EdgeStr[i]) * 2 + (pstSharpen->au16EdgeStr[i + 1]) * 2 ) / 5));
                pstMpiDynaRegCfg->u16mfGainD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16EdgeStr[i]) * 1 + (pstSharpen->au16EdgeStr[i + 1]) * 3 ) / 5));
            }

            i = 31;
            for (; j < 64; j++)
            {
                pstMpiDynaRegCfg->u16mfGainD[j] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16EdgeStr[i]));
            }

        }
        else////  2
        {
            for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
            {
              pstMpiDynaRegCfg->u16mfGainD[i] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16EdgeStr[i]));
            }
            for (i = ISP_SHARPEN_GAIN_NUM; i < SHRP_GAIN_LUT_SIZE; i++)
            {
              pstMpiDynaRegCfg->u16mfGainD[i] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16EdgeStr[ISP_SHARPEN_GAIN_NUM-1]));
            }
        }

      // GainUD
        if (pstDefaultDynaRegCfg->u8mfThdSelUD == 1)
        {
            j = 0;

            for (i = 0; i < 16; i++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
            }
            for (i = 16; i < 24; i++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i] + pstSharpen->au16TextureStr[i + 1]) >> 1)));
            }
            for (i = 24; i < 28; i++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 2 + (pstSharpen->au16TextureStr[i + 1])   ) / 3));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i])   + (pstSharpen->au16TextureStr[i + 1]) * 2 ) / 3));
            }
            for (i = 28; i < 31; i++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 4 + (pstSharpen->au16TextureStr[i + 1]) * 1 ) / 5));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 3 + (pstSharpen->au16TextureStr[i + 1]) * 2 ) / 5));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 2 + (pstSharpen->au16TextureStr[i + 1]) * 3 ) / 5));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 1 + (pstSharpen->au16TextureStr[i + 1]) * 4 ) / 5));
            }
            i = 31;
            for (; j < 64; j++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
            }
        }
        else
        {
            j = 0;
            for (i = 0; i < 16; i=i+2)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (64 + pstSharpen->au16TextureStr[(i)] + pstSharpen->au16TextureStr[(i)+1])>>1);
            }
            for (i = 16; i < 20; i++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
            }
            for (i = 20; i < 24; i++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i] + pstSharpen->au16TextureStr[i + 1]) >> 1)));
            }
            for (i = 24; i < 26; i++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 2 + (pstSharpen->au16TextureStr[i + 1])   ) / 3));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i])   + (pstSharpen->au16TextureStr[i + 1]) * 2 ) / 3));
            }
            for (i = 26; i < 28; i++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 3 + (pstSharpen->au16TextureStr[i + 1]) * 1 ) / 4));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 2 + (pstSharpen->au16TextureStr[i + 1]) * 2 ) / 4));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 1 + (pstSharpen->au16TextureStr[i + 1]) * 3 ) / 4));
            }
            for (i = 28; i < 31; i++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 4 + (pstSharpen->au16TextureStr[i + 1]) * 1 ) / 5));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 3 + (pstSharpen->au16TextureStr[i + 1]) * 2 ) / 5));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 2 + (pstSharpen->au16TextureStr[i + 1]) * 3 ) / 5));
                pstMpiDynaRegCfg->u16mfGainUD[j++] = SHARPEN_CLIP3(0, 0xFFF, (32 + ((pstSharpen->au16TextureStr[i]) * 1 + (pstSharpen->au16TextureStr[i + 1]) * 4 ) / 5));
            }
            i = 31;
            for (; j < 64; j++)
            {
                pstMpiDynaRegCfg->u16mfGainUD[j] = SHARPEN_CLIP3(0, 0xFFF, (32 + pstSharpen->au16TextureStr[i]));
            }

        }

        for (i = 0; i < SHRP_GAIN_LUT_SIZE; i++)
        {
            pstMpiDynaRegCfg->u16hfGainUD[i]  = (HI_U16)SHARPEN_CLIP3(0, 0xFFF, ((((HI_U32)(pstMpiDynaRegCfg->u16mfGainUD[i])) * (pstSharpen->u16TextureFreq)) >> 6));
            pstMpiDynaRegCfg->u16hfGainD[i]   = (HI_U16)SHARPEN_CLIP3(0, 0xFFF, ((((HI_U32)(pstMpiDynaRegCfg->u16mfGainD[i]))  * (pstSharpen->u16EdgeFreq)) >> 6));
        }

        pstMpiDynaRegCfg->u8oshtAmt              = pstSharpen->u8OverShoot;
        pstMpiDynaRegCfg->u8ushtAmt              = pstSharpen->u8UnderShoot;
        // skin Ctrl
        if (31 == pstSharpen->u8SkinGain)
        {
            pstMpiDynaRegCfg->u8bEnSkinCtrl      = 0;
        }
        else
        {
            pstMpiDynaRegCfg->u8bEnSkinCtrl      = 1;
            pstMpiDynaRegCfg->u8skinEdgeWgt[1]   = SHARPEN_CLIP3(0, 0x1F, (31 - pstSharpen->u8SkinGain));
            pstMpiDynaRegCfg->u8skinEdgeWgt[0]   = SHARPEN_CLIP3(0, 0x1F, (pstMpiDynaRegCfg->u8skinEdgeWgt[1] << 1));

        }
        // Chr Ctrl
        if ((32 == pstSharpen->u8GGain) & (31 == pstSharpen->u8RGain) & (31 == pstSharpen->u8BGain))
        {
            pstMpiDynaRegCfg->u8bEnChrCtrl       = 0;
        }
        else
        {
            pstMpiDynaRegCfg->u8bEnChrCtrl       = 1;
            pstMpiDynaRegCfg->u8chrRGain         = pstSharpen->u8RGain;
            pstMpiDynaRegCfg->u16chrGGain        = pstSharpen->u8GGain;
            pstMpiDynaRegCfg->u16chrGmfGain      = pstSharpen->u8GGain;
            pstMpiDynaRegCfg->u8chrBGain         = pstSharpen->u8BGain;
        }

        if ( 128 == pstSharpen->u8DetailCtrl )
        {
            pstMpiDynaRegCfg->bEnDetailCtrl = 0;
        }
        else
        {
            pstMpiDynaRegCfg->bEnDetailCtrl = 1;
        }

        pstMpiDynaRegCfg->u8detailOshtAmt = SHARPEN_CLIP3(0, 127, (pstMpiDynaRegCfg->u8oshtAmt) + (pstSharpen->u8DetailCtrl) - 128);
        pstMpiDynaRegCfg->u8detailUshtAmt = SHARPEN_CLIP3(0, 127, (pstMpiDynaRegCfg->u8ushtAmt) + (pstSharpen->u8DetailCtrl) - 128);

        pstMpiDynaRegCfg->u8dirDiffSft    = 63 - pstSharpen->u8EdgeFiltStr;

        pstMpiDynaRegCfg->u8bEnShtCtrlByVar = 1;
        pstMpiDynaRegCfg->u8shtBldRt        = pstSharpen->u8ShootSupAdj;
        pstMpiDynaRegCfg->u8oshtVarThd1     = pstSharpen->u8ShootSupStr;

        pstMpiDynaRegCfg->u8ushtVarThd1 = pstMpiDynaRegCfg->u8oshtVarThd1;

        pstMpiDynaRegCfg->u16oMaxGain = pstSharpen->u16MaxSharpGain;
        pstMpiDynaRegCfg->u16uMaxGain = pstSharpen->u16MaxSharpGain;
        pstMpiDynaRegCfg->u8skinMaxU  = pstSharpen->u8SkinUmax;
        pstMpiDynaRegCfg->u8skinMinU  = pstSharpen->u8SkinUmin;
        pstMpiDynaRegCfg->u8skinMaxV  = pstSharpen->u8SkinVmax;
        pstMpiDynaRegCfg->u8skinMinV  = pstSharpen->u8SkinVmin;
        pstMpiDynaRegCfg->u8detailOshtThr[0]  = pstSharpen->u8DetailCtrlThr;
        pstMpiDynaRegCfg->u8detailOshtThr[1]  = SHARPEN_CLIP3(0, 255,  (pstSharpen->u8DetailCtrlThr + SHRP_DETAIL_CTRL_THR_DELTA));
        pstMpiDynaRegCfg->u8detailUshtThr[0]  = pstSharpen->u8DetailCtrlThr;
        pstMpiDynaRegCfg->u8detailUshtThr[1]  = SHARPEN_CLIP3(0, 255,  (pstSharpen->u8DetailCtrlThr + SHRP_DETAIL_CTRL_THR_DELTA));
        pstMpiDynaRegCfg->u8bEnLumaCtrl = 0;
        for ( i = 0; i < ISP_SHARPEN_LUMA_NUM; i++ )
        {
            pstMpiDynaRegCfg->au8LumaWgt[i] = pstSharpen->au8LumaWgt[i];

            if (pstMpiDynaRegCfg->au8LumaWgt[i] < 127)
            {
                pstMpiDynaRegCfg->u8bEnLumaCtrl = 1;
            }
        }
    }

    /* Calc all MulCoef */
    // Defalut
    pstDefaultDynaRegCfg->s16hfEpsMul       = CalcMulCoef( pstDefaultDynaRegCfg->u8hfEpsVarThr0, pstStaticRegCfg->u8hfEpsVal0,
                                                           pstDefaultDynaRegCfg->u8hfEpsVarThr1, pstDefaultDynaRegCfg->u8hfEpsVal1,
                                                           SHRP_HF_EPS_MUL_PRECS);
    pstDefaultDynaRegCfg->s16oshtVarDiffMul = CalcMulCoef( pstDefaultDynaRegCfg->u8oshtVarDiffThd[0], pstStaticRegCfg->u8oshtVarDiffWgt0,
                                                           pstDefaultDynaRegCfg->u8oshtVarDiffThd[1], pstDefaultDynaRegCfg->u8oshtVarDiffWgt1,
                                                           SHRP_SHT_VAR_MUL_PRECS );

    pstDefaultDynaRegCfg->s16ushtVarDiffMul = CalcMulCoef( pstDefaultDynaRegCfg->u8ushtVarDiffThd[0], pstStaticRegCfg->u8ushtVarDiffWgt0,
                                                           pstDefaultDynaRegCfg->u8ushtVarDiffThd[1], pstDefaultDynaRegCfg->u8ushtVarDiffWgt1,
                                                           SHRP_SHT_VAR_MUL_PRECS );
    pstDefaultDynaRegCfg->s16rangeDirMul    = CalcMulCoef( pstDefaultDynaRegCfg->u8rangeThd0, pstDefaultDynaRegCfg->u8rangeDirWgt0,
                                                           pstStaticRegCfg->u8rangeThd1, pstStaticRegCfg->u8rangeDirWgt1,
                                                           SHRP_DIR_RANGE_MUL_PRECS );

    // Mpi
    pstMpiDynaRegCfg->u16oshtVarMul    = CalcMulCoef( pstStaticRegCfg->u8oshtVarThd0, pstDefaultDynaRegCfg->u8oshtVarWgt0,
                                                      pstMpiDynaRegCfg->u8oshtVarThd1, pstStaticRegCfg->u8oshtVarWgt1,
                                                      SHRP_SHT_VAR_MUL_PRECS );

    pstMpiDynaRegCfg->u16ushtVarMul    = CalcMulCoef( pstStaticRegCfg->u8ushtVarThd0, pstDefaultDynaRegCfg->u8ushtVarWgt0,
                                                      pstMpiDynaRegCfg->u8ushtVarThd1, pstStaticRegCfg->u8ushtVarWgt1,
                                                      SHRP_SHT_VAR_MUL_PRECS );
    pstMpiDynaRegCfg->s16detailOshtMul = CalcMulCoef( pstMpiDynaRegCfg->u8detailOshtThr[0], pstMpiDynaRegCfg->u8detailOshtAmt,
                                                      pstMpiDynaRegCfg->u8detailOshtThr[1], pstMpiDynaRegCfg->u8oshtAmt,
                                                      SHRP_DETAIL_SHT_MUL_PRECS );

    pstMpiDynaRegCfg->s16detailUshtMul = CalcMulCoef( pstMpiDynaRegCfg->u8detailUshtThr[0], pstMpiDynaRegCfg->u8detailUshtAmt,
                                                      pstMpiDynaRegCfg->u8detailUshtThr[1], pstMpiDynaRegCfg->u8ushtAmt,
                                                      SHRP_DETAIL_SHT_MUL_PRECS );
    pstMpiDynaRegCfg->s16chrRMul       = CalcMulCoef( pstStaticRegCfg->u8chrRThd[0], pstMpiDynaRegCfg->u8chrRGain,
                                                      pstStaticRegCfg->u8chrRThd[1], 32,
                                                      SHRP_CHR_MUL_SFT );
   pstMpiDynaRegCfg->s16chrGMul        = CalcMulCoef( pstStaticRegCfg->u8chrGThd[0], pstMpiDynaRegCfg->u16chrGGain,
                                                      pstStaticRegCfg->u8chrGThd[1], 32,
                                                      SHRP_CHR_MUL_SFT );
    pstMpiDynaRegCfg->s16chrGmfMul     = CalcMulCoef( pstStaticRegCfg->u8chrGThd[0], pstMpiDynaRegCfg->u16chrGmfGain,
                                                      pstStaticRegCfg->u8chrGThd[1], 32,
                                                      SHRP_CHR_MUL_SFT );
    pstMpiDynaRegCfg->s16chrBMul       = CalcMulCoef( pstStaticRegCfg->u8chrBThd[0], pstMpiDynaRegCfg->u8chrBGain,
                                                      pstStaticRegCfg->u8chrBThd[1], 32,
                                                      SHRP_CHR_MUL_SFT );
    pstMpiDynaRegCfg->s16skinEdgeMul   = CalcMulCoef( pstStaticRegCfg->u8skinEdgeThd[0], pstMpiDynaRegCfg->u8skinEdgeWgt[0],
                                                      pstStaticRegCfg->u8skinEdgeThd[1], pstMpiDynaRegCfg->u8skinEdgeWgt[1],
                                                      SHRP_SKIN_EDGE_MUL_PRECS );
}

static HI_BOOL __inline CheckSharpenOpen(ISP_SHARPEN_S *pstSharpen)
{
    return (HI_TRUE == pstSharpen->bSharpenEn);
}

static HI_VOID SharpenActualUpdate(VI_PIPE ViPipe, ISP_SHARPEN_S *pstSharpen)
{
    HI_U8 i;

    hi_ext_system_actual_sharpen_overshootAmt_write(ViPipe, pstSharpen->u8OverShoot);
    hi_ext_system_actual_sharpen_undershootAmt_write(ViPipe, pstSharpen->u8UnderShoot);
    hi_ext_system_actual_sharpen_shootSupSt_write(ViPipe, pstSharpen->u8ShootSupStr);
    hi_ext_system_actual_sharpen_edge_frequence_write(ViPipe, pstSharpen->u16EdgeFreq);
    hi_ext_system_actual_sharpen_texture_frequence_write(ViPipe, pstSharpen->u16TextureFreq);

    for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
    {
        hi_ext_system_actual_sharpen_edge_str_write(ViPipe, i, pstSharpen->au16EdgeStr[i]);
        hi_ext_system_actual_sharpen_texture_str_write(ViPipe, i, pstSharpen->au16TextureStr[i]);
    }
}

HI_S32 SharpenProcWrite(VI_PIPE ViPipe, ISP_CTRL_PROC_WRITE_S *pstProc)
{
    HI_U8 i, u8Index;
    ISP_CTRL_PROC_WRITE_S stProcTmp;

    ISP_SHARPEN_S *pstSharpen = HI_NULL;

    SHARPEN_GET_CTX(ViPipe, pstSharpen);
    ISP_CHECK_POINTER(pstSharpen);

    if ((HI_NULL == pstProc->pcProcBuff) || (0 == pstProc->u32BuffLen))
    {
        return HI_FAILURE;
    }

    stProcTmp.pcProcBuff = pstProc->pcProcBuff;
    stProcTmp.u32BuffLen = pstProc->u32BuffLen;

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "-----SHARPEN INFO--------------------------------------------------------------\n");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%16s\n",
                    "bSharpenEn");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%16u\n",
                    (HI_U16)pstSharpen->bSharpenEn);

    for (i = 0; i < 4; i++)
    {
        u8Index = i * 8;

        ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                        "%s"  "%d"  "--"  "%d:\n",
                        "LumaWgt ", u8Index, u8Index + 7);

        ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                        "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u\n\n",
                        (HI_U16)pstSharpen->au8LumaWgt[u8Index + 0],
                        (HI_U16)pstSharpen->au8LumaWgt[u8Index + 1],
                        (HI_U16)pstSharpen->au8LumaWgt[u8Index + 2],
                        (HI_U16)pstSharpen->au8LumaWgt[u8Index + 3],
                        (HI_U16)pstSharpen->au8LumaWgt[u8Index + 4],
                        (HI_U16)pstSharpen->au8LumaWgt[u8Index + 5],
                        (HI_U16)pstSharpen->au8LumaWgt[u8Index + 6],
                        (HI_U16)pstSharpen->au8LumaWgt[u8Index + 7]
                       );
    }

    for (i = 0; i < 4; i++)
    {
        u8Index = i * 8;

        ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                        "%s"  "%d"  "--"  "%d:\n",
                        "TextureStr ", u8Index, u8Index + 7);

        ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                        "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u\n\n",
                        (HI_U16)pstSharpen->au16TextureStr[u8Index + 0],
                        (HI_U16)pstSharpen->au16TextureStr[u8Index + 1],
                        (HI_U16)pstSharpen->au16TextureStr[u8Index + 2],
                        (HI_U16)pstSharpen->au16TextureStr[u8Index + 3],
                        (HI_U16)pstSharpen->au16TextureStr[u8Index + 4],
                        (HI_U16)pstSharpen->au16TextureStr[u8Index + 5],
                        (HI_U16)pstSharpen->au16TextureStr[u8Index + 6],
                        (HI_U16)pstSharpen->au16TextureStr[u8Index + 7]
                       );
    }

    for (i = 0; i < 4; i++)
    {
        u8Index = i * 8;
        ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                        "%s"  "%d"  "--"  "%d:\n",
                        "EdgeStr ", u8Index, u8Index + 7);

        ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                        "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u\n\n",
                        (HI_U16)pstSharpen->au16EdgeStr[u8Index + 0],
                        (HI_U16)pstSharpen->au16EdgeStr[u8Index + 1],
                        (HI_U16)pstSharpen->au16EdgeStr[u8Index + 2],
                        (HI_U16)pstSharpen->au16EdgeStr[u8Index + 3],
                        (HI_U16)pstSharpen->au16EdgeStr[u8Index + 4],
                        (HI_U16)pstSharpen->au16EdgeStr[u8Index + 5],
                        (HI_U16)pstSharpen->au16EdgeStr[u8Index + 6],
                        (HI_U16)pstSharpen->au16EdgeStr[u8Index + 7]
                       );
    }

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%12s" "%12s" "%12s" "%12s" "%12s" "%12s" "%12s" "%12s" "%12s" "%12s" "%12s \n",
                    "TextureFreq", "EdgeFreq", "OverShoot", "UnderShoot", "ShootSupStr", "DetailCtrl", "EdgeFiltStr", "RGain", "GGain", "BGain", "SkinGain");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%12u"  "%12u"  "%12u"  "%12u"  "%12u"  "%12u"  "%12u"  "%12u"  "%12u"  "%12u"   "%12u\n\n",
                    (HI_U16)pstSharpen->u16TextureFreq,
                    (HI_U16)pstSharpen->u16EdgeFreq,
                    (HI_U16)pstSharpen->u8OverShoot,
                    (HI_U16)pstSharpen->u8UnderShoot,
                    (HI_U16)pstSharpen->u8ShootSupStr,
                    (HI_U16)pstSharpen->u8DetailCtrl,
                    (HI_U16)pstSharpen->u8EdgeFiltStr,
                    (HI_U16)pstSharpen->u8RGain,
                    (HI_U16)pstSharpen->u8GGain,
                    (HI_U16)pstSharpen->u8BGain,
                    (HI_U16)pstSharpen->u8SkinGain
                   );

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%12s" "%14s" "%14s" "%12s" "%12s" "%12s"  "%12s\n",
                    "ShootSupAdj", "DetailCtrlThr", "MaxSharpGain", "SkinUmax", "SkinUmin", "SkinVmax", "SkinVmin");


    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%12u"  "%14u"  "%14u"  "%12u"  "%12u"  "%12u"   "%12u\n\n",
                    (HI_U16)pstSharpen->u8ShootSupAdj,
                    (HI_U16)pstSharpen->u8DetailCtrlThr,
                    (HI_U16)pstSharpen->u16MaxSharpGain,
                    (HI_U16)pstSharpen->u8SkinUmax,
                    (HI_U16)pstSharpen->u8SkinUmin,
                    (HI_U16)pstSharpen->u8SkinVmax,
                    (HI_U16)pstSharpen->u8SkinVmin
                   );

    pstProc->u32WriteLen += 1;

    return HI_SUCCESS;
}

HI_S32 ISP_SharpenRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo, HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_U8  i;
    HI_U32 u32Iso = 0;

    ISP_CTX_S     *pstIspCtx  = HI_NULL;
    ISP_SHARPEN_S *pstSharpen = HI_NULL;
    ISP_REG_CFG_S *pstRegCfg  = (ISP_REG_CFG_S *)pRegCfg;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    SHARPEN_GET_CTX(ViPipe, pstSharpen);
    ISP_CHECK_POINTER(pstSharpen);

    if (pstIspCtx->stLinkage.bDefectPixel)
    {
        return HI_SUCCESS;
    }

    /* calculate every two interrupts */
    //if ((0 != pstIspCtx->u32FrameCnt % 5) && (HI_TRUE != pstIspCtx->stLinkage.bSnapState) && (pstIspCtx->stLinkage.u8ProIndex < 1))
    //{
    //    return HI_SUCCESS;
    //}

    pstSharpen->bSharpenEn = hi_ext_system_manual_isp_sharpen_en_read(ViPipe);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg.bEnable = pstSharpen->bSharpenEn;
    }

    pstRegCfg->unKey.bit1SharpenCfg = 1;

    /*check hardware setting*/
    if (!CheckSharpenOpen(pstSharpen))
    {
        return HI_SUCCESS;
    }

    /* sharpen strength linkage with the u32ISO calculated by ae */
    u32Iso = pstIspCtx->stLinkage.u32Iso;

    SharpenReadExtregs(ViPipe);
    SharpenReadProMode(ViPipe);
    if (u32Iso != pstSharpen->u32IsoLast)       //will not work if ISO is the same
    {
        ISP_Sharpen_GetDefaultRegCfg(ViPipe, u32Iso);
        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stDefaultDynaRegCfg.bResh = HI_TRUE;
        }
    }

    if (pstSharpen->bSharpenMpiUpdateEn)
    {
        if (pstSharpen->u8ManualSharpenYuvEnabled == OP_TYPE_AUTO)  //auto mode
        {
            ISP_Sharpen_GetMpiRegCfg(ViPipe, u32Iso);
        }
        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.bResh            = HI_TRUE;
            pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.u32UpdateIndex  += 1;
            SharpenMPI2Reg(&(pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg), pstSharpen, u32Iso);
        }
    }
    else
    {
        if (u32Iso != pstSharpen->u32IsoLast)
        {
            if (pstSharpen->u8ManualSharpenYuvEnabled == OP_TYPE_AUTO)  //auto mode
            {
                ISP_Sharpen_GetMpiRegCfg(ViPipe, u32Iso);
                for (i = 0; i < pstRegCfg->u8CfgNum; i++)
                {
                    pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.bResh           = HI_TRUE;
                    pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.u32UpdateIndex += 1;
                    SharpenMPI2Reg(&(pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg), pstSharpen, u32Iso);
                }
            }
            else
            {
                for (i = 0; i < pstRegCfg->u8CfgNum; i++)
                {
                    //pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg.stDynaRegCfg.stMpiDynaRegCfg.bResh = HI_FALSE;
                    SharpenMPI2Reg(&(pstRegCfg->stAlgRegCfg[i].stSharpenRegCfg), pstSharpen, u32Iso);
                }
            }
        }
        else
        {
            //pstRegCfg->unKey.bit1SharpenCfg = 0;
        }
    }

    SharpenActualUpdate(ViPipe, pstSharpen);

    pstSharpen->u32IsoLast = u32Iso;    //will not work if ISO is the same

    return HI_SUCCESS;
}

HI_S32 ISP_SharpenCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;

    switch (u32Cmd)
    {
        case ISP_WDR_MODE_SET :
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            ISP_SharpenWdrModeSet(ViPipe, (HI_VOID *)&pRegCfg->stRegCfg);
            break;
        case ISP_PROC_WRITE:
            SharpenProcWrite(ViPipe, (ISP_CTRL_PROC_WRITE_S *)pValue);
            break;
        default :
            break;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SharpenExit(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_REGCFG_S *pRegCfg   = HI_NULL;
    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);

    for (i = 0; i < pRegCfg->stRegCfg.u8CfgNum; i++)
    {
        pRegCfg->stRegCfg.stAlgRegCfg[i].stSharpenRegCfg.bEnable = HI_FALSE;
    }

    pRegCfg->stRegCfg.unKey.bit1SharpenCfg = 1;

    hi_ext_system_isp_sharpen_manu_mode_write(ViPipe, HI_FALSE);

    SharpenCtxExit(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterSharpen(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Sharpen);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_SHARPEN;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_SharpenInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_SharpenRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_SharpenCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_SharpenExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
