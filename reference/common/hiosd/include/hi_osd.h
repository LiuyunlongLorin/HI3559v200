/**
 * @file    hi_osd.h
 * @brief   osd module interface and struct define.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 * @version   1.0

 */
#include "hi_comm_region.h"
#include "hi_appcomm.h"

#ifndef __HI_OSD_H
#define __HI_OSD_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     OSD */
/** @{ */  /** <!-- [OSD] */
#define OSD_CMD_SET_ATTR (1)
#define OSD_CMD_GET_ATTR (2)
#define OSD_CMD_START (3)
#define OSD_CMD_STOP (4)
#define OSD_CMD_BATCH (5)

/** Maximum OSD Count */
#define HI_OSD_MAX_CNT  (16)

/** Maximum Display Count for Each OSD */
#define HI_OSD_MAX_DISP_CNT (2)

/** String OSD Maximum Length */
#define HI_OSD_MAX_STR_LEN  (64)

/** struct define */
/** OSD Type Enum */
typedef enum hiOSD_TYPE_E
{
    HI_OSD_TYPE_TIME = 0,
    HI_OSD_TYPE_STRING,
    HI_OSD_TYPE_BITMAP,
    HI_OSD_TYPE_BUTT
} HI_OSD_TYPE_E;

/** OSD Time Format Enum */
typedef enum hiOSD_TIMEFMT_E
{
    HI_OSD_TIMEFMT_YMD24H = 0, /**<eg. 2017-03-10 23:00:59 */
    HI_OSD_TIMEFMT_BUTT
} HI_OSD_TIMEFMT_E;

/** OSD Binded Module enum */
typedef enum hiOSD_BIND_MOD_E
{
    HI_OSD_BINDMOD_VI = 0,
    HI_OSD_BINDMOD_VPSS,
    HI_OSD_BINDMOD_AVS,
    HI_OSD_BINDMOD_VENC,
    HI_OSD_BINDMOD_VO,
    HI_OSD_BINDMOD_BUTT
} HI_OSD_BIND_MOD_E;

typedef enum hiOSD_COORDINATE_E
{
    HI_OSD_COORDINATE_RATIO_COOR = 0,   /* Ratio coordinate. */
    HI_OSD_COORDINATE_ABS_COOR          /* Absolute coordinate. */
} HI_OSD_COORDINATE_E;

/** OSD Display Attribute */
typedef struct hiOSD_DISP_ATTR_S
{
    HI_BOOL bShow;
    HI_OSD_BIND_MOD_E enBindedMod; /**<OSD Binded Module: Static  */
    HI_HANDLE ModHdl; /**<Binded Module  Handle: Static eg.VcapPipeHdl VpssHdl  StitchHdl     DispHdl */
    HI_HANDLE ChnHdl; /**<Binded Channel Handle: Static eg.PipeChnHdl  VPortHdl StitchPortHdl WndHdl  VencHdl */
    HI_U32  u32FgAlpha;
    HI_U32  u32BgAlpha;
    HI_OSD_COORDINATE_E enCoordinate;   /**< Coordinate mode of the osd start Position. */
    POINT_S stStartPos; /**<OSD Start Position */
    ATTACH_DEST_E enAttachDest; /**<only for venc*/
    HI_S32 s32Batch;
} HI_OSD_DISP_ATTR_S;

/** osd pixel format enum */
typedef enum hiOSD_PIXEL_FMT_E
{
    HI_OSD_PIXEL_FMT_RGB1555 = 0,
    HI_OSD_PIXEL_FMT_BUTT
} HI_OSD_PIXEL_FMT_E;

/** OSD Bitmap Attribute */
typedef struct hiOSD_BITMAP_ATTR_S
{
    HI_OSD_PIXEL_FMT_E enPixelFormat;
    HI_U32             u32Width;
    HI_U32             u32Height;
    HI_U64             u64PhyAddr;
    HI_VOID*           pvData;
} HI_OSD_BITMAP_ATTR_S;

/* OSD Content */
typedef struct hiOSD_CONTENT_S
{
    HI_OSD_TYPE_E enType;
    HI_OSD_TIMEFMT_E enTimeFmt;
    HI_U32  u32Color;
    HI_U32  u32BgColor;
    HI_CHAR szStr[HI_OSD_MAX_STR_LEN];
    SIZE_S  stFontSize; /**<String/Time Attribute: Font Size, unit in pixel, align 2 */
    HI_OSD_BITMAP_ATTR_S stBitmap; /**<Pixel Format: Only Support RGB1555 for now */
} HI_OSD_CONTENT_S;

/* OSD Parameter */
typedef struct hiOSD_ATTR_S
{
    HI_U32 u32DispNum; /* Binded Display Number for this OSD */
    HI_OSD_DISP_ATTR_S astDispAttr[HI_OSD_MAX_DISP_CNT];
    HI_OSD_CONTENT_S stContent;
} HI_OSD_ATTR_S;

typedef HI_S32 (*HI_OSD_GETFONTMOD_CALLBACK_FN_PTR)(HI_CHAR* Character,HI_U8** FontMod,HI_S32* FontModLen);

/* OSD Fonts Lib */
typedef struct hiOSD_FONTS_S
{
    /* OSD Lib Font Size, in pixel */
    HI_U32 u32FontWidth;
    HI_U32 u32FontHeight;
    HI_OSD_GETFONTMOD_CALLBACK_FN_PTR pfnGetFontMod;
} HI_OSD_FONTS_S;

/** function interface */
/**
 * @brief    osd module initialization, eg. create time osd update task.
 * @param[in] pstFonts:osd fonts lib
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Init(const HI_OSD_FONTS_S* pstFonts);

/**
 * @brief    osd module de-initialization, eg. destroy time osd update task, release resource.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Deinit(HI_VOID);

/**
 * @brief    set osd attribute.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @param[in] pstAttr:osd configure attribute
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_SetAttr(HI_S32 s32OsdIdx, const HI_OSD_ATTR_S* pstAttr);

/**
 * @brief    get osd attribute.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @param[out] pstAttr:osd configure attribute
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_GetAttr(HI_S32 s32OsdIdx, HI_OSD_ATTR_S* pstAttr);

/**
 * @brief    start osd by index.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Start(HI_S32 s32OsdIdx);

/**
 * @brief    stop osd by index.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Stop(HI_S32 s32OsdIdx);

/**
 * @brief    set osd show by batch.
 * @param[in] s32Batch:user define
 * @param[in] bShow:1-show 0-not show
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Batch(HI_S32 s32Batch, HI_BOOL bShow);

/** @}*/  /** <!-- ==== OSD End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif
