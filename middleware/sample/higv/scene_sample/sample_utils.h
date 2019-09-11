#ifndef __SAMPLE_UTILS_H__
#define __SAMPLE_UTILS_H__
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <hi_gv.h>

#define RES_PATH  "./res/"

#define SBFONT_FILE  RES_PATH"font/ttf/simhei.ttf"
#define MBFONT_FILE  RES_PATH"font/ttf/simhei.ttf"

#if defined(__HI3559AV100__)
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 480
#else if (defined(__HI3556__) || defined(__HI3559__) || defined(__HI3556AV100__) || defined(__HI3559V200__))
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#endif

#define INVALID_HANDLE 0x0
#define WIDGET_CHECK_RET(p, val)   if(!(p)) { printf("Error==%s,%d==\n", __FUNCTION__, __LINE__);return (val);}

#define HI_GV_Widget_SetMsgProcX(x1,x2,x3)  HI_GV_Widget_SetMsgProc(x1,x2,x3,HIGV_PROCORDER_BEFORE)

#if 0
static HI_S32 Widget_DecImg(const HI_CHAR *pFileName, HIGV_HANDLE *pSurface)
{
    HI_S32 Ret;
    HIGO_DEC_ATTR_S SrcDesc;
    HIGV_HANDLE Decoder;

    SrcDesc.SrcType = HIGO_DEC_SRCTYPE_FILE;
    SrcDesc.SrcInfo.pFileName = pFileName;

    Ret = HI_GO_CreateDecoder(&SrcDesc, &Decoder);
    if (Ret != HI_SUCCESS)
    {
        printf("failed to create decoder!");
        return Ret;
    }

    Ret = HI_GO_DecImgData(Decoder, 0, NULL,pSurface);
    Ret |= HI_GO_DestroyDecoder(Decoder);

    return Ret;
}

static HI_S32 Widget_SetWinSkin(HIGV_HANDLE Handle, HI_U32 SkinIndex, HI_COLOR color)
{
    HI_S32 ret;
    HIGV_STYLE_S WinStyle;
    HI_RESID hNormalSkin;

    memset(&WinStyle, 0x00, sizeof(HIGV_STYLE_S));
    WinStyle.StyleType = HIGV_STYLETYPE_COLOR;
    WinStyle.BackGround.Color = color;
    ret = HI_GV_Res_CreateStyle(&WinStyle, &hNormalSkin);
    WIDGET_CHECK_RET(ret == HI_SUCCESS, HI_FAILURE);

    ret = HI_GV_Widget_SetSkin(Handle, SkinIndex,  hNormalSkin);
    WIDGET_CHECK_RET(ret == HI_SUCCESS, HI_FAILURE);

    return ret;
}
#endif

static HI_S32 Widget_AppCreateSysFont(HIGV_HANDLE *pFont)
{
    HI_S32 Ret;
    HI_RESID SbFont, MbFont;
    HIGV_HANDLE hFont;
    HIGV_FONT_S FontInfo;
    Ret = HI_GV_Res_CreateID(SBFONT_FILE, HIGV_RESTYPE_FONT, &SbFont);
    if (HI_SUCCESS != Ret)
    {
        return Ret;
    }

    Ret = HI_GV_Res_CreateID(MBFONT_FILE, HIGV_RESTYPE_FONT, &MbFont);
    if (HI_SUCCESS != Ret)
    {
        HI_GV_Res_DestroyID(SbFont);
        return Ret;
    }

    FontInfo.MbFontID = MbFont;
    FontInfo.SbFontID = SbFont;
    FontInfo.Size = 20;
    FontInfo.bBold = HI_FALSE;
    FontInfo.bItalic = HI_FALSE;
    Ret = HI_GV_Font_Create((const HIGV_FONT_S *)&FontInfo, &hFont);
    if (HI_SUCCESS != Ret)
    {
        HI_GV_Res_DestroyID(SbFont);
        HI_GV_Res_DestroyID(MbFont);
        return Ret;
    }

    *pFont = hFont;
    return HI_SUCCESS;
}

#endif
