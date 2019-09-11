/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : vou_ext.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2011/06/28
  Description   :
  History       :
  1.Date        : 2011/06/28
    Author      : n00168968/z00185248/l00181524
    Modification: Created file

******************************************************************************/

#ifndef __HI_VOU_EXT_H__
#define __HI_VOU_EXT_H__

#include <hi_common.h>
#include "hi_comm_vo.h"

/*
* Name : VO_BITEXT_MODE_E
* Desc : BITEXT mode
*/
typedef enum hiVO_BITEXT_MODE_E {
    VO_BITEXT_LOW_ZERO = 0x0,
    VO_BITEXT_LOW_HIGHBIT = 0x2,
    VO_BITEXT_LOW_HIGHBITS = 0x3,
    VO_BITEXT_BUTT
} VO_BITEXT_MODE_E;

/*
* Name : VO_COLORKEY_MODE_E
* Desc : Color key mode
*/
typedef enum hiVO_COLORKEY_MODE_E {
    VO_COLORKEY_IN = 0x0,
    VO_COLORKEY_OUT = 0x1,
    VO_COLORKEY_BUTT
} VO_COLORKEY_MODE_E;

typedef enum hiVO_SCAN_MODE_E {
    VO_SCAN_MODE_INTERLACE = 0x0,
    VO_SCAN_MODE_PROGRESSIVE = 0x1,
    VO_SCAN_MODE_BUTT
} VO_SCAN_MODE_E;

typedef enum hiVO_DITHER_OUT_BITWIDTH_E {
    DITHER_OUT_BITWIDTH_8 = 0x0, /* dither output 8bit */
    DITHER_OUT_BITWIDTH_10 = 0x1, /* dither output 10bit */

    DITHER_OUT_BITWIDTH_BUTT
} VO_DITHER_OUT_BITWIDTH_E;

typedef enum {
    VO_HIFB_INTTYPE_VO,
    VO_HIFB_INTTYPE_VO_DISP,
    VO_HIFB_INTTYPE_WBC,
    VO_HIFB_INTTYPE_BUTT,
} VO_FB_INT_TYPE_E;

typedef struct {
    HI_U32 u32StartPhyAddr;
    HI_U32 u32EndPhyAddr;
} VO_MDDRC_ZONE_ADDR_S;

typedef enum tagVO_DISP_PIXEL_FORMAT_E {
    VO_INPUTFMT_ARGB_4444 = 0x48,
    VO_INPUTFMT_ARGB_1555 = 0x49,
    VO_INPUTFMT_ARGB_8888 = 0x68,

    VO_DISP_PIXELFORMAT_BUTT
} VO_DISP_PIXEL_FORMAT_E;

typedef struct tagVO_GFX_KEY_MAX_S {
    HI_U8 u8KeyMax_R;
    HI_U8 u8KeyMax_G;
    HI_U8 u8KeyMax_B;
} VO_GFX_KEY_MAX_S;

typedef struct tagVO_GFX_KEY_MIN_S {
    HI_U8 u8KeyMin_R;
    HI_U8 u8KeyMin_G;
    HI_U8 u8KeyMin_B;
} VO_GFX_KEY_MIN_S;

typedef enum {
    VO_GFX_BITEXTEND_1ST = 0,
    VO_GFX_BITEXTEND_2ND = 0x2,
    VO_GFX_BITEXTEND_3RD = 0x3,

    VO_GFX_BITEXTEND_BUTT
} VO_GFX_BITEXTEND_E;

typedef struct tagVO_GFX_MASK_S {
    HI_U8 u8Mask_r;
    HI_U8 u8Mask_g;
    HI_U8 u8Mask_b;
} VO_GFX_MASK_S;

typedef struct {
    VO_DISP_PIXEL_FORMAT_E enPixelFmt;  // new
    HI_U64 u64AR_PhyAddr;
    HI_U64 u64GB_PhyAddr;  // new
    HI_U32 u32Frame_Size0;  // new
    HI_U32 u32Frame_Size1;  // new
    HI_U32 u32Width;  // new
    HI_U32 u32Height;  // new
    HI_BOOL IsLosslessA;  // new
    HI_BOOL IsLossless;  // new
    HI_U32 u32Offset;
    HI_U32 u32Stride;
} VO_GRAPHIC_DCMP_INFO_S;

typedef struct hiVO_RECT_S {
    HI_S32 x;
    HI_S32 y;
    HI_S32 w;
    HI_S32 h;
} VO_RECT_S;

typedef struct {
    HI_U64 u64AR_PhyAddr;
    HI_U64 u64GB_PhyAddr;
    HI_U32 u32Offset;
    HI_U32 u32Stride;
    VO_RECT_S stInRect;
    VO_RECT_S stOutRect;
} VO_GRAPHIC_WBC_INFO_S;

typedef HI_S32 (*VO_FB_IntCallBack)(HI_VOID *pParaml, HI_VOID *pParamr);

/* initial and exit service of VO module,called by SYS module */
typedef HI_S32 FN_VOU_Init(HI_VOID);
typedef HI_VOID FN_VOU_Exit(HI_VOID);
/* send pic to VO ,called by internal modules */
typedef HI_S32 FN_VOU_ChnSendPic(VO_DEV VoDev, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pVFrame, HI_VOID *pAppendArg);
/* clear buffer of VO channle */
typedef HI_S32 FN_VOU_ClearChnBuf(VO_DEV VoDev, VO_CHN VoChn, HI_BOOL bClear);
/* set vou channel buffer deep */
typedef HI_S32 FN_VOU_ChnSetBufLen(VO_DEV VoDev, VO_CHN VoChn, HI_U32 u32BufLen);
typedef HI_BOOL FN_VOU_CheckHdmiEn(HI_VOID);
/* set vo dither output bitwidth */
typedef HI_S32 FN_VOU_SetDitherOutBitWidth(MOD_ID_E enModId, VO_DEV VoDev, VO_DITHER_OUT_BITWIDTH_E enOutBitWidth);

typedef HI_S32 FN_VOU_GraphicsGetDevMode(GRAPHIC_LAYER gfxLayer, VO_SCAN_MODE_E *pScanMode, HI_BOOL *pbFeildUpdate);
typedef HI_S32 FN_VOU_GraphicsOpenLayer(GRAPHIC_LAYER gfxLayer);
typedef HI_S32 FN_VOU_GraphicsCloseLayer(GRAPHIC_LAYER gfxLayer);
typedef HI_S32 FN_VOU_GraphicsEnableLayer(GRAPHIC_LAYER gfxLayer, HI_BOOL bEnable);
typedef HI_S32 FN_VOU_GraphicsSetCallback(GRAPHIC_LAYER gfxLayer, VO_FB_INT_TYPE_E enIntType,
                                          VO_FB_IntCallBack pCallBack, HI_VOID *pArg);
typedef HI_S32 FN_VOU_GraphicsGetIntfSize(GRAPHIC_LAYER gfxLayer, HI_U32 *pu32Width, HI_U32 *pu32Height);
typedef HI_S32 FN_VOU_GraphicsGetIntfType(GRAPHIC_LAYER gfxLayer, VO_INTF_TYPE_E *penIntfType);
typedef HI_S32 FN_VOU_GraphicsSetCscCoef(GRAPHIC_LAYER gfxLayer);
typedef HI_BOOL FN_VOU_GraphicsSetGfxKeyMode(GRAPHIC_LAYER enLayer, HI_U32 u32KeyOut);
typedef HI_BOOL FN_VOU_GraphicsSetGfxPalpha(GRAPHIC_LAYER enLayer, HI_U32 bAlphaEn, HI_U32 bArange, HI_U8 u8Alpha0,
                                            HI_U8 u8Alpha1);
typedef HI_BOOL FN_VOU_GraphicsSetLayerGalpha(GRAPHIC_LAYER enLayer, HI_U8 u8Alpha0);
typedef HI_BOOL FN_VOU_GraphicsSetCscEn(GRAPHIC_LAYER enLayer, HI_BOOL bCscEn);
typedef HI_BOOL FN_VOU_GraphicsSetGfxAddr(GRAPHIC_LAYER enLayer, HI_U64 u64LAddr);
typedef HI_BOOL FN_VOU_GraphicsSetGfxStride(GRAPHIC_LAYER enLayer, HI_U16 u16pitch);
typedef HI_BOOL FN_VOU_GraphicsGetGfxPreMult(GRAPHIC_LAYER enLayer, HI_U32 *pbEnable);
typedef HI_BOOL FN_VOU_GraphicsSetGfxPreMult(GRAPHIC_LAYER enLayer, HI_U32 bEnable);
typedef HI_BOOL FN_VOU_GraphicsSetLayerDataFmt(GRAPHIC_LAYER enLayer, VO_DISP_PIXEL_FORMAT_E enDataFmt);
typedef HI_BOOL FN_VOU_GraphicsSetLayerInRect(GRAPHIC_LAYER enLayer, RECT_S *pstRect);
typedef HI_BOOL FN_VOU_GraphicsSetLayerSrcImageReso(GRAPHIC_LAYER enLayer, RECT_S *pstRect);
typedef HI_BOOL FN_VOU_GraphicsSetLayerOutRect(GRAPHIC_LAYER enLayer, RECT_S *pstRect);
typedef HI_BOOL FN_VOU_GraphicsSetColorKeyValue(GRAPHIC_LAYER enLayer, VO_GFX_KEY_MAX_S stKeyMax,
                                                VO_GFX_KEY_MIN_S stKeyMin);
typedef HI_BOOL FN_VOU_GraphicsSetColorKeyMask(GRAPHIC_LAYER enLayer, VO_GFX_MASK_S stMsk);
typedef HI_BOOL FN_VOU_GraphicsSetGfxKeyEn(GRAPHIC_LAYER enLayer, HI_U32 u32KeyEnable);
typedef HI_BOOL FN_VOU_GraphicsSetRegUp(GRAPHIC_LAYER enLayer);
typedef HI_BOOL FN_VOU_GraphicsGetLayerGalpha(GRAPHIC_LAYER enLayer, HI_U8 *pu8Alpha0);
typedef HI_BOOL FN_VOU_GraphicsGetLayerDataFmt(GRAPHIC_LAYER enLayer, HI_U32 *pu32Fmt);
typedef HI_BOOL FN_VOU_GraphicsSetGfxExt(GRAPHIC_LAYER enLayer, VO_GFX_BITEXTEND_E enMode);
typedef HI_BOOL FN_VOU_GraphicsGetGfxAddr(GRAPHIC_LAYER enLayer, HI_U64 *pu64GfxAddr);
typedef HI_BOOL FN_VOU_GraphicsGetGfxStride(GRAPHIC_LAYER enLayer, HI_U32 *pu32GfxStride);
typedef HI_VOID FN_VOU_GraphicsGetDDRZoneCfg(GRAPHIC_LAYER gfxLayer, HI_U32 *pu32StartID, HI_U32 *pu32Cnt);
typedef HI_VOID FN_VOU_GraphicsClearDDRDectectZone(HI_U32 u32StartID, HI_U32 u32Cnt);
typedef HI_S32 FN_VOU_GraphicsCheckDDRDectectZone(GRAPHIC_LAYER gfxLayer, HI_U32 u32NewStartID, HI_U32 u32NewCnt,
                                                  HI_U32 u32PreStartID, HI_U32 u32PreCnt);
typedef HI_VOID FN_VOU_GraphicsGetMDDRStatus(HI_U32 u32StartID, HI_U32 u32Cnt, HI_U32 *pu32Status);
typedef HI_VOID FN_VOU_GraphicsSetMDDRDetectZone(HI_U32 u32StartID, HI_U32 u32Cnt, VO_MDDRC_ZONE_ADDR_S *pstZoneAddr);

// For graphics compress
typedef HI_BOOL FN_VOU_GraphicsEnableDcmp(GRAPHIC_LAYER gfxLayer, HI_BOOL bEnable);
typedef HI_BOOL FN_VOU_GraphicsGetDcmpEnableState(GRAPHIC_LAYER gfxLayer, HI_BOOL *pbEnable);
typedef HI_BOOL FN_VOU_GraphicsSetDcmpInfo(GRAPHIC_LAYER enLayer, VO_GRAPHIC_DCMP_INFO_S *pstDcmpInfo);

// For graphics hdr(GHDR)
typedef HI_BOOL FN_VOU_GraphicsEnableGHDR(GRAPHIC_LAYER enLayer, HI_BOOL bEnable);
// For graphics ZME
typedef HI_BOOL FN_VOU_GraphicsEnableZME(GRAPHIC_LAYER enLayer, RECT_S *stInRect, RECT_S *stOutRect, HI_BOOL bEnable);

typedef struct hiVOU_EXPORT_FUNC_S {
    FN_VOU_Init *pfnInit;
    FN_VOU_Exit *pfnExit;
    FN_VOU_ChnSendPic *pfnVouChnSendPic;
    FN_VOU_ClearChnBuf *pfnVouClearChnBuf;
    FN_VOU_ChnSetBufLen *pfnVouChnSetBufLen;
    FN_VOU_CheckHdmiEn *pfnVouCheckHdmiEn;
    FN_VOU_SetDitherOutBitWidth *pfnVouSetDitherOutBitWidth;

    FN_VOU_GraphicsGetDevMode *pfnVOU_GraphicsGetDevMode;
    FN_VOU_GraphicsOpenLayer *pfnVOU_GraphicsOpenLayer;
    FN_VOU_GraphicsCloseLayer *pfnVOU_GraphicsCloseLayer;
    FN_VOU_GraphicsEnableLayer *pfnVOU_GraphicsEnableLayer;
    FN_VOU_GraphicsSetCallback *pfnVOU_GraphicsSetCallback;
    FN_VOU_GraphicsGetIntfSize *pfnVOU_GraphicsGetIntfSize;
    FN_VOU_GraphicsGetIntfType *pfnVOU_GraphicsGetIntfType;
    FN_VOU_GraphicsSetCscCoef *pfnVOU_GraphicsSetCscCoef;
    FN_VOU_GraphicsSetGfxKeyMode *pfnVOU_GraphicsSetGfxKeyMode;
    FN_VOU_GraphicsSetGfxPalpha *pfnVOU_GraphicsSetGfxPalpha;
    FN_VOU_GraphicsSetLayerGalpha *pfnVOU_GraphicsSetLayerGalpha;
    FN_VOU_GraphicsSetCscEn *pfnVOU_GraphicsSetCscEn;
    FN_VOU_GraphicsSetGfxAddr *pfnVOU_GraphicsSetGfxAddr;
    FN_VOU_GraphicsSetGfxStride *pfnVOU_GraphicsSetGfxStride;
    FN_VOU_GraphicsGetGfxPreMult *pfnVOU_GraphicsGetGfxPreMult;
    FN_VOU_GraphicsSetGfxPreMult *pfnVOU_GraphicsSetGfxPreMult;
    FN_VOU_GraphicsSetLayerDataFmt *pfnVOU_GraphicsSetLayerDataFmt;
    FN_VOU_GraphicsSetLayerInRect *pfnVOU_GraphicsSetLayerInRect;
    FN_VOU_GraphicsSetLayerSrcImageReso *pfnVOU_GraphicsSetLayerSrcImageReso;
    FN_VOU_GraphicsSetLayerOutRect *pfnVOU_GraphicsSetLayerOutRect;
    FN_VOU_GraphicsSetColorKeyValue *pfnVOU_GraphicsSetColorKeyValue;
    FN_VOU_GraphicsSetColorKeyMask *pfnVOU_GraphicsSetColorKeyMask;
    FN_VOU_GraphicsSetGfxKeyEn *pfnVOU_GraphicsSetGfxKeyEn;
    FN_VOU_GraphicsSetRegUp *pfnVOU_GraphicsSetRegUp;
    FN_VOU_GraphicsGetLayerGalpha *pfnVOU_GraphicsGetLayerGalpha;
    FN_VOU_GraphicsGetLayerDataFmt *pfnVOU_GraphicsGetLayerDataFmt;
    FN_VOU_GraphicsSetGfxExt *pfnVOU_GraphicsSetGfxExt;
    FN_VOU_GraphicsGetGfxAddr *pfnVOU_GraphicsGetGfxAddr;
    FN_VOU_GraphicsGetGfxStride *pfnVOU_GraphicsGetGfxStride;
    FN_VOU_GraphicsGetDDRZoneCfg *pfnVOU_GraphicsGetDDRZoneCfg;
    FN_VOU_GraphicsClearDDRDectectZone *pfnVOU_GraphicsClearDDRDectectZone;
    FN_VOU_GraphicsCheckDDRDectectZone *pfnVOU_GraphicsCheckDDRDectectZone;
    FN_VOU_GraphicsGetMDDRStatus *pfnVOU_GraphicsGetMDDRStatus;
    FN_VOU_GraphicsSetMDDRDetectZone *pfnVOU_GraphicsSetMDDRDetectZone;
    // For compress
    FN_VOU_GraphicsEnableDcmp *pfnVOU_GraphicsEnableDcmp;
    FN_VOU_GraphicsGetDcmpEnableState *pfnVOU_GraphicsGetDcmpEnableState;
    FN_VOU_GraphicsSetDcmpInfo *pfnVOU_GraphicsSetDcmpInfo;
    // For GHDR
    FN_VOU_GraphicsEnableGHDR *pfnVOU_GraphicsEnableGHDR;
    // For ZME
    FN_VOU_GraphicsEnableZME *pfnVOU_GraphicsEnableZME;
} VOU_EXPORT_FUNC_S;

#define CKFN_VOU_ENTRY() CHECK_FUNC_ENTRY(HI_ID_VO)

#define CKFN_VOU_CheckHdmiEn() \
    (FUNC_ENTRY(VOU_EXPORT_FUNC_S, HI_ID_VO)->pfnVouCheckHdmiEn != NULL)
#define CALL_VOU_CheckHdmiEn() \
    FUNC_ENTRY(VOU_EXPORT_FUNC_S, HI_ID_VO)->pfnVouCheckHdmiEn()

#endif /* __HI_VOU_EXT_H__ */


