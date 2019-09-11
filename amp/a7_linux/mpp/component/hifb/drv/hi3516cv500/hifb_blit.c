/*
* Copyright (c) 2018 HiSilicon Technologies Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "hifb_main.h"
#include "hi_tde_ext.h"
#include "mod_ext.h"

#include <linux/hardirq.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#ifdef __HuaweiLite__
#define in_atomic()	(HI_FALSE)
#endif

static IntCallBack s_pTdeCallBack;
static IntCallBack s_pTdeRotateCallBack;

/* tde support which color format */
static HI_BOOL s_bTdeColFmt[HIFB_FMT_BUTT] = {
    HI_FALSE,    /* HIFB_FMT_RGB565 */
    HI_FALSE,    /* HIFB_FMT_RGB888 */
    HI_FALSE,    /* HIFB_FMT_KRGB444 */
    HI_FALSE,    /* HIFB_FMT_KRGB555 */
    HI_FALSE,   /* HIFB_FMT_KRGB888 */
    HI_TRUE,    /* HIFB_FMT_ARGB4444 */
    HI_TRUE,    /* HIFB_FMT_ARGB1555 */
    HI_TRUE,    /* HIFB_FMT_ARGB8888 */
    HI_FALSE,    /* HIFB_FMT_ARGB8565 */
    HI_FALSE,   /* HIFB_FMT_RGBA4444 */
    HI_FALSE,   /* HIFB_FMT_RGBA5551 */
    HI_FALSE,   /* HIFB_FMT_RGBA5658 */
    HI_FALSE,   /* HIFB_FMT_RGBA8888 */

    HI_FALSE,   /**< BGR565 */
    HI_FALSE,   /**< BGR888 */
    HI_FALSE,   /**< ABGR4444 */
    HI_FALSE,   /**< ABGR1555 */
    HI_FALSE,   /**< ABGR8888 */
    HI_FALSE,   /**< ABGR8565 */
    HI_FALSE,   /**< BGR444 16bpp */
    HI_FALSE,   /**< BGR555 16bpp */
    HI_FALSE,   /**< BGR888 32bpp */

    HI_FALSE,    /* HIFB_FMT_1BPP */
    HI_FALSE,    /* HIFB_FMT_2BPP */
    HI_FALSE,    /* HIFB_FMT_4BPP */
    HI_FALSE,    /* HIFB_FMT_8BPP */
    HI_FALSE,    /* HIFB_FMT_ACLUT44 */
    HI_FALSE,    /* HIFB_FMT_ACLUT88 */
    HI_FALSE,   /* HIFB_FMT_PUYVY */
    HI_FALSE,   /* HIFB_FMT_PYUYV */
    HI_FALSE,   /* HIFB_FMT_PYVYU */
    HI_FALSE,   /* HIFB_FMT_YUV888*/
    HI_FALSE,   /* HIFB_FMT_AYUV8888 */
    HI_FALSE,   /* HIFB_FMT_YUVA8888 */
};

HI_BOOL hifb_TdeIsSupportFmt(HIFB_COLOR_FMT_E fmt)
{
	if(HIFB_FMT_BUTT <= fmt)
	{
		return HI_FALSE;
	}
	return s_bTdeColFmt[fmt];
}
#if 1
TDE2_COLOR_FMT_E HIFB_DRV_ConvFmt(HIFB_COLOR_FMT_E Fmt)
{
    switch (Fmt)
    {
        case HIFB_FMT_RGB565:
            return TDE2_COLOR_FMT_RGB565;
        case HIFB_FMT_RGB888:
            return TDE2_COLOR_FMT_RGB888;
        case HIFB_FMT_KRGB444:
            return TDE2_COLOR_FMT_RGB444;
        case HIFB_FMT_KRGB555:
            return TDE2_COLOR_FMT_RGB555;
        /*HIFB_FMT_KRGB888 32bpp;TDE2_COLOR_FMT_RGB888:24bpp.so transform HIFB_FMT_KRGB888 to
        TDE2_COLOR_FMT_ARGB8888 ,but ignore alpha data in process*/
        case HIFB_FMT_KRGB888:
            return TDE2_COLOR_FMT_ARGB8888;
        case HIFB_FMT_ARGB4444:
            return TDE2_COLOR_FMT_ARGB4444;
        case HIFB_FMT_ARGB1555:
            return TDE2_COLOR_FMT_ARGB1555;
        case HIFB_FMT_ARGB8888:
            return TDE2_COLOR_FMT_ARGB8888;
        case HIFB_FMT_ARGB8565:
            return TDE2_COLOR_FMT_ARGB8565;

        case HIFB_FMT_BGR565:
            return TDE2_COLOR_FMT_BGR565;
        case HIFB_FMT_ABGR1555:
            return TDE2_COLOR_FMT_ABGR1555;
        case HIFB_FMT_ABGR4444:
            return TDE2_COLOR_FMT_ABGR4444;
        case HIFB_FMT_KBGR444:
            return TDE2_COLOR_FMT_BGR444;
        case HIFB_FMT_KBGR555:
            return TDE2_COLOR_FMT_BGR555;
        case HIFB_FMT_BGR888:
            return TDE2_COLOR_FMT_BGR888;
        case HIFB_FMT_ABGR8888:
            return TDE2_COLOR_FMT_ABGR8888;
        case HIFB_FMT_ABGR8565:
            return TDE2_COLOR_FMT_ABGR8565;
        case HIFB_FMT_KBGR888:
            return TDE2_COLOR_FMT_ABGR8888;

        case HIFB_FMT_1BPP:
            return TDE2_COLOR_FMT_CLUT1;
        case HIFB_FMT_2BPP:
            return TDE2_COLOR_FMT_CLUT2;
        case HIFB_FMT_4BPP:
            return TDE2_COLOR_FMT_CLUT4;
        case HIFB_FMT_8BPP:
            return TDE2_COLOR_FMT_CLUT8;
        case HIFB_FMT_ACLUT44:
            return TDE2_COLOR_FMT_ACLUT44;
        case HIFB_FMT_ACLUT88:
            return TDE2_COLOR_FMT_ACLUT88;

        default:
            return TDE2_COLOR_FMT_BUTT;

    }
}

HI_S32 HIFB_DRV_Fill(HIFB_BUFFER_S *pDstImg, HI_U32 u32FillData)
{
    HI_S32 s32Ret;
    TDE2_SURFACE_S stDstSur = {0};
    TDE2_RECT_S stDstRect;
    TDE_HANDLE handle;
    TDE_EXPORT_FUNC_S  *pTdeExportFunc = NULL;

    pTdeExportFunc = FUNC_ENTRY(TDE_EXPORT_FUNC_S, HI_ID_TDE);
    if ((NULL == pTdeExportFunc) || (NULL == pTdeExportFunc->pfnTdeBeginJob)
        || (NULL == pTdeExportFunc->pfnTdeEndJob) || (NULL == pTdeExportFunc->pfnTdeCancelJob)
        || (NULL == pTdeExportFunc->pfnTdeQuickFill)
        )
    {
        HI_TRACE_FB(HI_DBG_ERR, "TDE pTdeExportFunc is NULL!\n");
        return HI_FAILURE;
    }

    /** config dst*/
    stDstSur.PhyAddr      = pDstImg->stCanvas.u64PhyAddr;
    stDstSur.u32Width     = pDstImg->stCanvas.u32Width;
    stDstSur.u32Height    = pDstImg->stCanvas.u32Height;
    stDstSur.u32Stride    = pDstImg->stCanvas.u32Pitch;
    stDstSur.bAlphaMax255 = HI_TRUE;
    stDstSur.bYCbCrClut   = HI_FALSE;
    stDstSur.enColorFmt   = HIFB_DRV_ConvFmt(pDstImg->stCanvas.enFmt);

    stDstRect.s32Xpos     = 0;
    stDstRect.s32Ypos     = 0;
    stDstRect.u32Height   = stDstSur.u32Height;
    stDstRect.u32Width    = stDstSur.u32Width;

    s32Ret = pTdeExportFunc->pfnTdeBeginJob(&handle);
    if(s32Ret != HI_SUCCESS)
    {
        osal_printk("HiFB Warning: TDE begin job failed\n");
        return s32Ret;
    }

    s32Ret = pTdeExportFunc->pfnTdeQuickFill(handle, &stDstSur, &stDstRect, u32FillData);
    if(s32Ret != HI_SUCCESS)
    {
        osal_printk("HiFB Warning: TDE blit failed. ret = 0x%x handle %d \n",s32Ret,handle);
        pTdeExportFunc->pfnTdeCancelJob(handle);
        return s32Ret;
    }

    s32Ret = pTdeExportFunc->pfnTdeEndJob(handle, HI_TRUE, 1000, HI_FALSE, HI_NULL, HI_NULL);
    if(s32Ret != HI_SUCCESS)
    {
        osal_printk("HiFB Warning: FILL job submitted to TDE failed!!! Ret = 0x%x\n", s32Ret);
 //       TdeOsiCancelJob(handle);
        return s32Ret;
    }

    return HI_SUCCESS;
}


static HI_U32 HIFB_DRV_GetDcmpStride(HIFB_SURFACE_S *pstCanvas)
{
    HI_U32 comp_ratio_int = 1000;
    HI_U32 u32BytePerPix = 2;
    HI_U32 extend_width;
    HI_U32 exp_num_temp = 0;
    HI_U32 u32Stride = pstCanvas->u32Pitch;

    if(HIFB_FMT_ARGB8888 == pstCanvas->enFmt)
    {
        if(pstCanvas->u32Width <= 320)
        {
            exp_num_temp = 2;
            comp_ratio_int = 1000;
        }
        else if(pstCanvas->u32Width <= 720)
        {
            exp_num_temp = 10;
            comp_ratio_int = 2000;
        }
        else if(pstCanvas->u32Width <= 3840)
        {
            exp_num_temp = 10;
            comp_ratio_int = 2000;
        }

        u32BytePerPix  = 4;

        u32Stride = ((pstCanvas->u32Width * u32BytePerPix * 8 * 1000 / comp_ratio_int + 127) / 128 + exp_num_temp) * 16;
    }
    else if(HIFB_FMT_ARGB1555 == pstCanvas->enFmt || HIFB_FMT_ARGB4444 == pstCanvas->enFmt)
    {
        if(pstCanvas->u32Width <= 720)
        {
            exp_num_temp = 2;
            comp_ratio_int = 1000;
        }
        else if(pstCanvas->u32Width <= 3840)
        {
            exp_num_temp = 0;
            comp_ratio_int = 1000;
        }
        u32BytePerPix = 2;

        extend_width = ((pstCanvas->u32Width + 31) / 32) * 32;
        u32Stride = ((extend_width * u32BytePerPix * 8 * 1000 / comp_ratio_int + 127) / 128 + exp_num_temp) * 16;
    }

    u32Stride = (u32Stride + 0xf) & 0xfffffff0;

    return u32Stride;
}


//static int s_counter = 0;
HI_S32 HIFB_DRV_Blit(HIFB_BUFFER_S *pSrcImg, HIFB_BUFFER_S *pDstImg, HIFB_BLIT_OPT_S *pstOpt, HI_BOOL bRefreshScreen)
{
    HI_S32                    s32Ret;
    TDE2_SURFACE_S            stSrcSur       = {0};
    TDE2_SURFACE_S            stDstSur       = {0};
    TDE2_RECT_S               stSrcRect;
    TDE2_RECT_S               stDstRect;
    TDE_HANDLE                handle;
    TDE2_OPT_S                stOpt          = {0};
    HIFB_TDECALLBACK_PARAM *  pParam         = HI_NULL;
    TDE_EXPORT_FUNC_S  *      pTdeExportFunc = NULL;
    HI_U32                    u32timeout     = 100; // 100ms

    pTdeExportFunc = FUNC_ENTRY(TDE_EXPORT_FUNC_S, HI_ID_TDE);
    if ((NULL == pTdeExportFunc) || (NULL == pTdeExportFunc->pfnTdeBeginJob)
        || (NULL == pTdeExportFunc->pfnTdeEndJob) || (NULL == pTdeExportFunc->pfnTdeCancelJob)
        || (NULL == pTdeExportFunc->pfnTdeEnableRegionDeflicker) || (NULL == pTdeExportFunc->pfnTdeBlit)
        )
    {
        HI_TRACE_FB(HI_DBG_ERR, "TDE pTdeExportFunc is NULL!\n");
        return HI_FAILURE;
    }

    /** config src*/
    stSrcSur.PhyAddr   = pSrcImg->stCanvas.u64PhyAddr;
    stSrcSur.u32Width  = pSrcImg->stCanvas.u32Width;
    stSrcSur.u32Height = pSrcImg->stCanvas.u32Height;
    stSrcSur.u32Stride = pSrcImg->stCanvas.u32Pitch;
    stSrcSur.bAlphaMax255 = HI_TRUE;
    stSrcSur.bYCbCrClut = HI_FALSE;
    stSrcSur.enColorFmt = HIFB_DRV_ConvFmt(pSrcImg->stCanvas.enFmt);
    stSrcSur.u8Alpha0 = pstOpt->stAlpha.u8Alpha0;
    stSrcSur.u8Alpha1 = pstOpt->stAlpha.u8Alpha1;

    stSrcRect.s32Xpos = pSrcImg->UpdateRect.x;
    stSrcRect.s32Ypos = pSrcImg->UpdateRect.y;
    stSrcRect.u32Width = pSrcImg->UpdateRect.w;
    stSrcRect.u32Height = pSrcImg->UpdateRect.h;

    /** config dst*/
    stDstSur.PhyAddr   = pDstImg->stCanvas.u64PhyAddr;
    stDstSur.u32Width  = pDstImg->stCanvas.u32Width;
    stDstSur.u32Height = pDstImg->stCanvas.u32Height;
    stDstSur.u32Stride = pDstImg->stCanvas.u32Pitch;
    stDstSur.bAlphaMax255 = HI_TRUE;
    stDstSur.bYCbCrClut = HI_FALSE;
    stDstSur.enColorFmt = HIFB_DRV_ConvFmt(pDstImg->stCanvas.enFmt);
    stDstSur.u8Alpha0 = pstOpt->stAlpha.u8Alpha0;
    stDstSur.u8Alpha1 = pstOpt->stAlpha.u8Alpha1;

    if(HI_TRUE == pstOpt->bCompress)
    {
        stDstSur.CbCrPhyAddr   = pDstImg->stCanvas.u64GBPhyAddr;
        stDstSur.u32CbCrStride = pDstImg->stCanvas.u32Pitch;

        stDstSur.u32Stride = HIFB_DRV_GetDcmpStride(&pDstImg->stCanvas);
    }

    if (HI_TRUE == bRefreshScreen)
    {
        stDstRect.s32Xpos   =  stSrcRect.s32Xpos * stDstSur.u32Width / stSrcSur.u32Width;
        stDstRect.s32Ypos   =  stSrcRect.s32Ypos * stDstSur.u32Height / stSrcSur.u32Height;
        stDstRect.u32Width  =  stSrcRect.u32Width * stDstSur.u32Width / stSrcSur.u32Width;
        stDstRect.u32Height =  stSrcRect.u32Height * stDstSur.u32Height / stSrcSur.u32Height;
    }
    else
    {
        stDstRect.s32Xpos = pDstImg->UpdateRect.x;
        stDstRect.s32Ypos = pDstImg->UpdateRect.y;
        stDstRect.u32Width = pDstImg->UpdateRect.w;
        stDstRect.u32Height = pDstImg->UpdateRect.h;
    }

    stOpt.bResize = pstOpt->bScale;
    if ((stSrcSur.enColorFmt >= HIFB_DRV_ConvFmt(HIFB_FMT_1BPP)) && (stSrcSur.enColorFmt <= HIFB_DRV_ConvFmt(HIFB_FMT_ACLUT88)))
    {
        stOpt.bClutReload = HI_TRUE;
        stSrcSur.ClutPhyAddr = pstOpt->u64CmapAddr;
        stDstSur.ClutPhyAddr = pstOpt->u64CmapAddr;
    }

    //stOpt.bDeflicker = HI_TRUE;
    switch(pstOpt->enAntiflickerLevel)
    {
        case HIFB_LAYER_ANTIFLICKER_NONE:
        {
            stOpt.enFilterMode = TDE2_FILTER_MODE_COLOR;
            stOpt.enDeflickerMode= TDE2_DEFLICKER_MODE_NONE;
            break;
        }
        case HIFB_LAYER_ANTIFLICKER_LOW:
        {
            stOpt.enFilterMode = TDE2_FILTER_MODE_COLOR;
            //TdeOsiSetDeflickerLevel(TDE_DEFLICKER_LOW);
            break;
        }
        case HIFB_LAYER_ANTIFLICKER_MIDDLE:
       {
            stOpt.enFilterMode = TDE2_FILTER_MODE_COLOR;
            //TdeOsiSetDeflickerLevel(TDE_DEFLICKER_MIDDLE);
            break;
        }
        case HIFB_LAYER_ANTIFLICKER_HIGH:
        {
            stOpt.enFilterMode = TDE2_FILTER_MODE_COLOR;
            //TdeOsiSetDeflickerLevel(TDE_DEFLICKER_HIGH);
            break;
        }
        case HIFB_LAYER_ANTIFLICKER_AUTO:
        {
            stOpt.enFilterMode = TDE2_FILTER_MODE_COLOR;
            //TdeOsiSetDeflickerLevel(TDE_DEFLICKER_AUTO);
            break;
        }
        default:
            stOpt.enDeflickerMode = TDE2_DEFLICKER_MODE_NONE;
            break;
    }

    if (pstOpt->stCKey.bKeyEnable)
    {
        if ((stSrcSur.enColorFmt >= HIFB_DRV_ConvFmt(HIFB_FMT_1BPP)) && (stSrcSur.enColorFmt <= HIFB_DRV_ConvFmt(HIFB_FMT_ACLUT88)))
        {
            stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_FOREGROUND;
            stOpt.unColorKeyValue.struCkClut.stAlpha.bCompIgnore = HI_TRUE;
            stOpt.unColorKeyValue.struCkClut.stClut.bCompOut = pstOpt->stCKey.u32KeyMode;
            stOpt.unColorKeyValue.struCkClut.stClut.u8CompMax = pstOpt->stCKey.u8BlueMax;
            stOpt.unColorKeyValue.struCkClut.stClut.u8CompMin = pstOpt->stCKey.u8BlueMin;
        }
        else
        {
            stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_FOREGROUND;
            stOpt.unColorKeyValue.struCkARGB.stAlpha.bCompIgnore = HI_TRUE;
            stOpt.unColorKeyValue.struCkARGB.stRed.u8CompMax = pstOpt->stCKey.u8RedMax;
            stOpt.unColorKeyValue.struCkARGB.stRed.u8CompMin = pstOpt->stCKey.u8RedMin;
            stOpt.unColorKeyValue.struCkARGB.stRed.bCompOut = pstOpt->stCKey.u32KeyMode;
            stOpt.unColorKeyValue.struCkARGB.stRed.u8CompMask = 0xff;
            stOpt.unColorKeyValue.struCkARGB.stGreen.u8CompMax = pstOpt->stCKey.u8GreenMax;
            stOpt.unColorKeyValue.struCkARGB.stGreen.u8CompMin = pstOpt->stCKey.u8GreenMin;
            stOpt.unColorKeyValue.struCkARGB.stGreen.bCompOut = pstOpt->stCKey.u32KeyMode;
            stOpt.unColorKeyValue.struCkARGB.stGreen.u8CompMask = 0xff;
            stOpt.unColorKeyValue.struCkARGB.stBlue.u8CompMax = pstOpt->stCKey.u8BlueMax;
            stOpt.unColorKeyValue.struCkARGB.stBlue.u8CompMin = pstOpt->stCKey.u8BlueMin;
            stOpt.unColorKeyValue.struCkARGB.stBlue.bCompOut = pstOpt->stCKey.u32KeyMode;
            stOpt.unColorKeyValue.struCkARGB.stBlue.u8CompMask = 0xff;
        }
    }

    if (pstOpt->stAlpha.bAlphaEnable)
    {
        stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
        stOpt.u8GlobalAlpha = pstOpt->stAlpha.u8GlobalAlpha;
        stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_NORM;
    }
    else
    {
        stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_FOREGROUND;
    }

    switch (pstOpt->enMirrorMode)
    {
        case HIFB_MIRROR_NONE:
            stOpt.enMirror = TDE2_MIRROR_NONE;
            break;
        case HIFB_MIRROR_HORIZONTAL:
            stOpt.enMirror = TDE2_MIRROR_HORIZONTAL;
            break;
        case HIFB_MIRROR_VERTICAL:
            stOpt.enMirror = TDE2_MIRROR_VERTICAL;
            break;
        case HIFB_MIRROR_BOTH:
            stOpt.enMirror = TDE2_MIRROR_BOTH;
            break;
        default:
            stOpt.enMirror = TDE2_MIRROR_NONE;
            break;
    }

#if 1
    if (pstOpt->bRegionDeflicker)
    {
        s32Ret = pTdeExportFunc->pfnTdeEnableRegionDeflicker(HI_TRUE);
        if (s32Ret != HI_SUCCESS)
        {
            HI_TRACE_FB(HI_DBG_ERR, "enable region deflicker failed!\n");
            return s32Ret;
        }
    }
#endif

    s32Ret = pTdeExportFunc->pfnTdeBeginJob(&handle);
    if(s32Ret != HI_SUCCESS)
    {
        HI_TRACE_FB(HI_DBG_ERR, "begin job failed\n");
        return s32Ret;
    }

    #if 0
    printk("%d, sw:%d sh:%d sstride %d dw:%d dh:%d dstride %d \n" ,__LINE__, stSrcSur.u32Width, stSrcSur.u32Height, stSrcSur.u32Stride,stDstSur.u32Width, stDstSur.u32Height,stDstSur.u32Stride);
    printk("%d, srcx %d srcy %d srcw %d srch %d dstx %d dsty %d dstw %d dsth %d\n",__LINE__,stSrcRect.s32Xpos,stSrcRect.s32Ypos,stSrcRect.u32Width,stSrcRect.u32Height,stDstRect.s32Xpos,stDstRect.s32Ypos,stDstRect.u32Width,stDstRect.u32Height);
    printk("+++++++srcAddr 0x%x DstAddr 0x%x \n",stSrcSur.ClutPhyAddr,stDstSur.ClutPhyAddr);
    #endif

    // Set bCompress to blit opt.
    stOpt.bCompress = pstOpt->bCompress;
    //printk("++ stOpt.bResize %d stOpt.bDeflicker %d stOpt.enFilterMode %d\n",stOpt.bResize,stOpt.bDeflicker,stOpt.enFilterMode);
    // See    TdeOsiBlit
    s32Ret = pTdeExportFunc->pfnTdeBlit(handle, &stDstSur, &stDstRect, &stSrcSur, &stSrcRect, &stDstSur, \
        &stDstRect, &stOpt);
    if(s32Ret != HI_SUCCESS)
    {
        HI_TRACE_FB(HI_DBG_ERR, "tde blit failed ret = 0x%x handle %d \n", s32Ret, handle);
        pTdeExportFunc->pfnTdeCancelJob(handle);
        return s32Ret;
    }
    #ifdef HI_FPGA
    // ARGB1555 : widthxheight/8/period s, period is ASIC or FPGA CORE clock of tde.
    // eg. FPGA core 48MHz, 3840 * 2160 / 8 / (48 * 10 ^ 6) = 0.0216(s)
    u32timeout = 432; // 0.216 * 2
    #endif
    if (pstOpt->bCallBack)
    {
        if (in_atomic())
        {
            pParam = kmalloc(sizeof(HIFB_TDECALLBACK_PARAM), GFP_ATOMIC);
        }

        else
        {
            pParam = kmalloc(sizeof(HIFB_TDECALLBACK_PARAM), GFP_KERNEL);
        }
        if (HI_NULL == pParam)
        {
            pTdeExportFunc->pfnTdeCancelJob(handle);
            return HI_FAILURE;
        }

        pParam->u32LayerId = *(HI_U32 *)pstOpt->pParam;
        pParam->bSoftCursorUpdate = pstOpt->bSoftCursorUpdate;
        pParam->bCompress = pstOpt->bCompress;

        s32Ret = pTdeExportFunc->pfnTdeEndJob(handle, pstOpt->bBlock, u32timeout, HI_FALSE, \
            (TDE_FUNC_CB)s_pTdeCallBack, pParam);
    }
    else
    {
        s32Ret = pTdeExportFunc->pfnTdeEndJob(handle, pstOpt->bBlock, u32timeout, HI_FALSE, \
            HI_NULL, HI_NULL);
    }

    if(s32Ret != HI_SUCCESS)
    {
        HI_TRACE_FB(HI_DBG_ERR, "end job failed!s32Ret = %x\n", s32Ret);
        // TdeOsiListCancelJob
        s32Ret = pTdeExportFunc->pfnTdeCancelJob(handle);
        // if pfnTdeCancelJob returns HI_FAILURE, it means : job has be submitted, so we do not free pParam.
        if(HI_SUCCESS == s32Ret)
        {
            if (pParam)
            {
                kfree(pParam);
            }
        }
        else
        {
            HI_TRACE_FB(HI_DBG_ERR, "cancel job failed!s32Ret = %x\n", s32Ret);
        }
        return s32Ret;
    }

#if 1
    if (pstOpt->bRegionDeflicker)
    {
        s32Ret = pTdeExportFunc->pfnTdeEnableRegionDeflicker(HI_FALSE);
        if (s32Ret != HI_SUCCESS)
        {
            HI_TRACE_FB(HI_DBG_ERR, "disable region deflicker failed!\n");
            return s32Ret;
        }
    }
#endif
    return handle;
}
#endif

// for graphic rotation.
HI_S32 HIFB_DRV_Rotate(HIFB_BUFFER_S *pSrcImg, HIFB_BUFFER_S *pDstImg, HIFB_ROTATE_OPT_S *pstRotOpt, HI_BOOL bRefreshScreen)
{
    HI_S32                   s32Ret;
    TDE2_SURFACE_S           stSrcSur         = {0};
    TDE2_SURFACE_S           stDstSur         = {0};
    TDE2_RECT_S              stSrcRect;
    TDE2_RECT_S              stDstRect;
    TDE_HANDLE               handle;
    HIFB_TDECALLBACK_PARAM*  pParam           = HI_NULL;
    TDE_EXPORT_FUNC_S*       pTdeExportFunc   = NULL;
    TDE_ROTATE_ANGLE_E       enTdeRotateAngle = TDE_ROTATE_BUTT;
    HI_U32                   u32timeout       = 2000; // 100ms

    pTdeExportFunc = FUNC_ENTRY(TDE_EXPORT_FUNC_S, HI_ID_TDE);
    if (   (NULL == pTdeExportFunc)
        || (NULL == pTdeExportFunc->pfnTdeBeginJob)
        || (NULL == pTdeExportFunc->pfnTdeRotate)
        || (NULL == pTdeExportFunc->pfnTdeCancelJob)
        || (NULL == pTdeExportFunc->pfnTdeEndJob)
        )
    {
        HI_TRACE_FB(HI_DBG_ERR, "TDE pTdeExportFunc is NULL!\n");
        return HI_FAILURE;
    }

    /** config src*/
    stSrcSur.PhyAddr   = pSrcImg->stCanvas.u64PhyAddr;
    stSrcSur.u32Width  = pSrcImg->stCanvas.u32Width;
    stSrcSur.u32Height = pSrcImg->stCanvas.u32Height;
    stSrcSur.u32Stride = pSrcImg->stCanvas.u32Pitch;
    stSrcSur.bAlphaMax255 = HI_TRUE;
    stSrcSur.bYCbCrClut = HI_FALSE;
    stSrcSur.enColorFmt = HIFB_DRV_ConvFmt(pSrcImg->stCanvas.enFmt);

    stSrcRect.s32Xpos   = pSrcImg->UpdateRect.x;
    stSrcRect.s32Ypos   = pSrcImg->UpdateRect.y;
    stSrcRect.u32Width  = pSrcImg->UpdateRect.w;
    stSrcRect.u32Height = pSrcImg->UpdateRect.h;

    /** config dst*/
    stDstSur.PhyAddr      = pDstImg->stCanvas.u64PhyAddr;
    stDstSur.u32Width     = pDstImg->stCanvas.u32Width;
    stDstSur.u32Height    = pDstImg->stCanvas.u32Height;
    stDstSur.u32Stride    = pDstImg->stCanvas.u32Pitch;
    stDstSur.bAlphaMax255 = HI_TRUE;
    stDstSur.bYCbCrClut   = HI_FALSE;
    stDstSur.enColorFmt   = HIFB_DRV_ConvFmt(pDstImg->stCanvas.enFmt);


    stDstRect.s32Xpos   = pDstImg->UpdateRect.x;
    stDstRect.s32Ypos   = pDstImg->UpdateRect.y;
    stDstRect.u32Width  = pSrcImg->UpdateRect.h;
    stDstRect.u32Height = pSrcImg->UpdateRect.w;


#if 0
    printk("%d, sw:%d sh:%d sstride %d dw:%d dh:%d dstride %d \n" ,__LINE__, stSrcSur.u32Width, stSrcSur.u32Height, stSrcSur.u32Stride,stDstSur.u32Width, stDstSur.u32Height,stDstSur.u32Stride);
    printk("%d, srcx %d srcy %d srcw %d srch %d dstx %d dsty %d dstw %d dsth %d\n",__LINE__,stSrcRect.s32Xpos,stSrcRect.s32Ypos,stSrcRect.u32Width,stSrcRect.u32Height,stDstRect.s32Xpos,stDstRect.s32Ypos,stDstRect.u32Width,stDstRect.u32Height);
    printk("+++++++srcAddr 0x%x DstAddr 0x%x \n",stSrcSur.ClutPhyAddr,stDstSur.ClutPhyAddr);
#endif

#ifdef HI_FPGA
    // ARGB1555 : widthxheight/8/period s, period is ASIC or FPGA CORE clock of tde.
    // eg. FPGA core 48MHz, 3840 * 2160 / 8 / (48 * 10 ^ 6) = 0.0216(s)
    u32timeout = 432; // 0.216 * 2
#endif


    s32Ret = pTdeExportFunc->pfnTdeBeginJob(&handle);
    if(s32Ret != HI_SUCCESS)
    {
        HI_TRACE_FB(HI_DBG_ERR, "begin job for rotate failed\n");
        return s32Ret;
    }

    switch(pstRotOpt->enRotateMode)
    {
        case HIFB_ROTATE_90:
            enTdeRotateAngle = TDE_ROTATE_CLOCKWISE_90;
            break;
        case HIFB_ROTATE_180:
            enTdeRotateAngle = TDE_ROTATE_CLOCKWISE_180;
            break;
        case HIFB_ROTATE_270:
            enTdeRotateAngle = TDE_ROTATE_CLOCKWISE_270;
            break;
        default:
            break;
    }
    // See    TdeOsiQuickRotate
    s32Ret = pTdeExportFunc->pfnTdeRotate(handle, &stSrcSur, &stSrcRect, &stDstSur, &stDstRect, enTdeRotateAngle);
    if(s32Ret != HI_SUCCESS)
    {
        HI_TRACE_FB(HI_DBG_ERR, "tde rotate failed ret = 0x%x handle %d \n", s32Ret, handle);
        pTdeExportFunc->pfnTdeCancelJob(handle);
        return s32Ret;
    }

    if (pstRotOpt->bCallBack)
    {
        if (in_atomic())
        {
            pParam = kmalloc(sizeof(HIFB_TDECALLBACK_PARAM), GFP_ATOMIC);
        }

        else
        {
            pParam = kmalloc(sizeof(HIFB_TDECALLBACK_PARAM), GFP_KERNEL);
        }
        if (HI_NULL == pParam)
        {
            pTdeExportFunc->pfnTdeCancelJob(handle);
            return HI_FAILURE;
        }

        pParam->u32LayerId = *(HI_U32 *)pstRotOpt->pParam;

        s32Ret = pTdeExportFunc->pfnTdeEndJob(handle, pstRotOpt->bBlock, u32timeout, HI_FALSE, \
            (TDE_FUNC_CB)s_pTdeRotateCallBack, pParam);
    }
    else
    {
        s32Ret = pTdeExportFunc->pfnTdeEndJob(handle, pstRotOpt->bBlock, u32timeout, HI_FALSE, \
            HI_NULL, HI_NULL);
    }

    if(s32Ret != HI_SUCCESS)
    {
        HI_TRACE_FB(HI_DBG_ERR, "end job failed!s32Ret = %x\n", s32Ret);
        s32Ret = pTdeExportFunc->pfnTdeCancelJob(handle);
        if(HI_SUCCESS == s32Ret)
        {
            if (pParam)
            {
                kfree(pParam);
            }
        }
        else
        {
            HI_TRACE_FB(HI_DBG_ERR, "cancel job failed!s32Ret = %x\n", s32Ret);
        }
        return s32Ret;
    }

    return handle;
}

HI_S32 HIFB_DRV_SetTdeCallBack(IntCallBack pTdeCallBack)
{
   s_pTdeCallBack = pTdeCallBack;
   return HI_SUCCESS;
}
HI_S32 HIFB_DRV_SetTdeRotateCallBack(IntCallBack pTdeRotCallBack)
{
   s_pTdeRotateCallBack = pTdeRotCallBack;
   return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

