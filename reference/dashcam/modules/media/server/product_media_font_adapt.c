/**
 * @file      product_media_font_adapt.c
 * @brief     product media font adapt
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 * @version   1.0

 */
#include "hi_osd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define BYTE_BITS (8)
#define IsASCII(a) (((a)>=0x00&&(a)<=0x7F)?1:0)

#define OSD_USE_HZK

#ifdef OSD_USE_HZK
#define OSD_FONT_MOD_W (16)
#define OSD_FONT_MOD_H (16)
/** OSD Fonts Lib */
static const HI_U8 s_fontlib[]  __attribute__ ((aligned (4))) =
{
#include "HZK16.dat"
};

static HI_S32 PDT_MEDIA_GetFontMod(HI_CHAR* Character,HI_U8** FontMod,HI_S32* FontModLen)
{
    /* Get Font Mod in GB2312 Fontlib*/
    HI_APPCOMM_CHECK_POINTER(Character, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(FontMod, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(FontModLen, HI_FAILURE);
    HI_U32 areacode = 0;
    HI_U32 bitcode = 0;
    if(IsASCII(Character[0]))
    {
        areacode = 3;
        bitcode = (HI_U32)((HI_U8)Character[0]-0x20);
    }
    else
    {
        areacode = (HI_U32)((HI_U8)Character[0]-0xA0);
        bitcode = (HI_U32)((HI_U8)Character[1]-0xA0);
    }

    HI_U32 offset = (94*(areacode-1)+(bitcode-1))*(OSD_FONT_MOD_H*OSD_FONT_MOD_W/BYTE_BITS);
    *FontMod = (HI_U8*)s_fontlib+offset;
    *FontModLen = OSD_FONT_MOD_H*OSD_FONT_MOD_W/BYTE_BITS;
    return HI_SUCCESS;
}
#else
#define OSD_FONT_MOD_W (16)
#define OSD_FONT_MOD_H (32)
/** OSD Fonts Lib */
static const HI_U8 s_fontlib[]  __attribute__ ((aligned (4))) =
{
#include "simsunb_16x32.dat"
};

static HI_S32 PDT_MEDIA_GetFontMod(HI_CHAR* Character,HI_U8** FontMod,HI_S32* FontModLen)
{
    /* Get Font Mod in ASCII Fontlib*/
    HI_APPCOMM_CHECK_POINTER(Character, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(FontMod, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(FontModLen, HI_FAILURE);
    if(!IsASCII(Character[0]))
    {
        return HI_FAILURE;
    }
    HI_U32 offset = (Character[0]-0x20)*(OSD_FONT_MOD_H*OSD_FONT_MOD_W/BYTE_BITS);
    *FontMod = (HI_U8*)s_fontlib+offset;
    *FontModLen = OSD_FONT_MOD_H*OSD_FONT_MOD_W/BYTE_BITS;
    return HI_SUCCESS;
}
#endif

HI_S32 PDT_MEDIA_InitOsdFontLib(HI_VOID)
{
    /* Init OSD */
    HI_OSD_FONTS_S stOsdFonts;
    stOsdFonts.u32FontWidth = OSD_FONT_MOD_W;
    stOsdFonts.u32FontHeight = OSD_FONT_MOD_H;
    stOsdFonts.pfnGetFontMod = PDT_MEDIA_GetFontMod;
    return HI_OSD_Init(&stOsdFonts);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

