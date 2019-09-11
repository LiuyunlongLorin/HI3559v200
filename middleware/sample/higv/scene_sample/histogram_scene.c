#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "higv_cextfile.h"
#include "higv_language.h"
#include "hi_gv_graphiccontext.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define INVALID_HANDLE             (0)

#define  IMG_PATH   "./res/pic/histogram/SING0003.JPG"

#define HIGV_CHECK(Function, Ret)  \
    if (Ret != HI_SUCCESS) {printf("[Func: %s, Line: %d] %s fail Ret(x):%x, Ret(d): %d\n", __FUNCTION__, __LINE__, Function, Ret, Ret);}

#define TIME_STAMP \
    do \
    { \
        struct timespec Nowtime = {0}; \
        clock_gettime(CLOCK_MONOTONIC, &Nowtime); \
        printf("[Func: %s, Line: %d] tv_sec: %ld, tv_usec: %ld\n\n", __FUNCTION__, __LINE__, Nowtime.tv_sec, (Nowtime.tv_nsec / 1000)); \
    } \
    while(0);

static HIGV_HANDLE s_WinGC = INVALID_HANDLE;
static HI_S32 s_ArraryY[256] = {0};

/**************************************************************************************/
/***                                                              Test Line Draw                                                                          ***/
/**************************************************************************************/
static HI_S32 TestLineDraw(HI_VOID)
{
    HI_S32 Ret = HI_SUCCESS;
    HI_RECT Rect;
    HI_S32 k = 0;

    Rect.x = 0;
    Rect.y = 0;
    Rect.w = 320;
    Rect.h = 240;

    {
        Ret = HI_GV_GraphicContext_Create(HISTOGRAM_SCENE_WIN_GROUPBOX, &s_WinGC);
        HIGV_CHECK("HI_GV_GraphicContext_Create", Ret);

        Ret = HI_GV_GraphicContext_Begin(s_WinGC);
        HIGV_CHECK("HI_GV_GraphicContext_Begin", Ret);

        Ret = HI_GV_GraphicContext_AddClipRect(s_WinGC, &Rect);
        HIGV_CHECK("HI_GV_GraphicContext_AddClipRect", Ret);

        Ret = HI_GV_GraphicContext_SetFgColor(s_WinGC, 0xFF000000);
        HIGV_CHECK("HI_GV_GraphicContext_SetFgColor", Ret);

        Ret = HI_GV_GraphicContext_SetFont(s_WinGC, simhei_font_text_20);
        HIGV_CHECK("HI_GV_GraphicContext_SetFgColor", Ret);

        HI_S32 j = 0;

        for (int i = 20; i < 128 + 20; i++)
        {
            k = s_ArraryY[j] + s_ArraryY[j + 1];
            j = j + 2;

            k = k / 20;

            if (k == 0)
            {
                k = 0;
            }

            if (k >= 100)
            {
                k = 99;
            }


            if (k != 0)
            {
                Ret = HI_GV_GraphicContext_DrawLine(s_WinGC, i, 100 - k, i, 100);
                HIGV_CHECK("HI_GV_GraphicContext_DrawLine", Ret);
            }
        }

        Ret = HI_GV_GraphicContext_End(s_WinGC);
        HIGV_CHECK("HI_GV_GraphicContext_End", Ret);

        if (s_WinGC)
        {
            Ret = HI_GV_GraphicContext_Destroy(s_WinGC);
            HIGV_CHECK("HI_GV_GraphicContext_Destroy", Ret);
            s_WinGC = INVALID_HANDLE;
        }
    }

    Ret = HI_GV_Widget_Refresh(HISTOGRAM_SCENE_WIN, HI_NULL);
    HIGV_CHECK("HI_GV_Widget_Refresh", Ret);

    return HI_SUCCESS;
}

static HI_S32 ParseImg(const HI_CHAR* pFileName)
{
    HI_S32 Ret = HI_SUCCESS;
    HIGO_DEC_ATTR_S SrcDesc;
    HIGV_HANDLE hDecoder;
    HIGV_HANDLE hSurface;
    HIGO_DEC_SURINFO_S SurfInfo;
    HIGO_DEC_IMGATTR_S ImgAttr;
    HI_U32 ImgDecIndex = 1;
    HI_PIXELDATA pData;
    HIGO_DEC_IMGINFO_S ImgInfo;
    HIGO_PIXELDATA_S PixelData;
    HIGV_DEC_SUFINFO_S ImgDecSufInfo = {HIGO_PF_8888, HIGO_MEMTYPE_MMZ, HI_FALSE};
    HI_CHAR ARGB[4] = {0};

    SrcDesc.SrcType = HIGO_DEC_SRCTYPE_FILE;
    SrcDesc.SrcInfo.pFileName = pFileName;

    TIME_STAMP;

    Ret = HI_GO_CreateDecoder(&SrcDesc, &hDecoder);
    HIGV_CHECK("HI_GO_CreateDecoder", Ret);

    SurfInfo.MemType = ImgDecSufInfo.MemType;
    SurfInfo.IsPubPalette = ImgDecSufInfo.IsPubPalette;
    Ret = HI_GO_SetDecSurfInfo(hDecoder, &SurfInfo);
    HIGV_CHECK("HI_GO_SetDecSurfInfo", Ret);

    Ret = HI_GO_DecImgInfo(hDecoder, ImgDecIndex, &ImgInfo);
    HIGV_CHECK("HI_GO_SetDecSurfInfo", Ret);

    ImgAttr.Width  = ImgInfo.Width;
    ImgAttr.Height = ImgInfo.Height;
    ImgAttr.Format = ImgDecSufInfo.PixFormat;

    Ret = HI_GO_DecImgData(hDecoder, ImgDecIndex, &ImgAttr, &hSurface);
    HIGV_CHECK("HI_GO_DecImgData", Ret);

    Ret = HI_GO_LockSurface(hSurface, pData, HI_TRUE);
    HIGV_CHECK("HI_GO_LockSurface", Ret);

    Ret = HI_GO_UnlockSurface(hSurface);
    HIGV_CHECK("HI_GO_UnlockSurface", Ret);

    PixelData.pData = (HI_CHAR*)pData[0].pData;

    HI_S32 Y;

    for (HI_U32 i = 0; i < ImgInfo.Width * ImgInfo.Height; i++)
    {
        ARGB[0] = *(HI_CHAR*)PixelData.pData;

        PixelData.pData = (char *)(PixelData.pData) + 1;

        ARGB[1] = *(HI_CHAR*)PixelData.pData;

        PixelData.pData = (char *)(PixelData.pData) + 1;

        ARGB[2] = *(HI_CHAR*)PixelData.pData;

        PixelData.pData = (char *)(PixelData.pData) + 2;

        Y = (HI_S32)(0.299 * ARGB[2] + 0.587 * ARGB[1] + 0.114 * ARGB[0]);

        s_ArraryY[Y] += 1;
    }

    Ret = HI_GO_FreeSurface(hSurface);
    HIGV_CHECK("HI_GO_FreeSurface", Ret);

    Ret = HI_GO_DestroyDecoder(hDecoder);
    HIGV_CHECK("HI_GO_DestroyDecoder", Ret);

    (HI_VOID)TestLineDraw();

    TIME_STAMP;

    return HI_SUCCESS;
}

HI_S32 HISTOGRAM_SCENE_WIN_onrefresh(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    memset(s_ArraryY, 0x0, sizeof(s_ArraryY));
    (HI_VOID)ParseImg(IMG_PATH);

    return HIGV_PROC_GOON;
}

HI_S32 HISTOGRAM_SCENE_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(HISTOGRAM_SCENE_WIN);
    return HIGV_PROC_GOON;
}

HI_S32 HISTOGRAM_SCENE_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
