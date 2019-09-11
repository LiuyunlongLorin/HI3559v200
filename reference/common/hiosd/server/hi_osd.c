/**
 * @file    hi_osd.c
 * @brief   osd module interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 * @version   1.0

 */

#include <string.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <errno.h>
#include "hi_osd.h"
#include "hi_math.h"
#include "mpi_vi.h"
#include "mpi_vpss.h"
#ifdef SUPPORT_STITCH
#include "mpi_avs.h"
#endif
#include "mpi_venc.h"
#include "mpi_vo.h"
#include "mpi_region.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "OSD"

#define IsASCII(a) (((a)>=0x00&&(a)<=0x7F)?1:0)
#define BYTE_BITS (8)
#define NOASCII_CHARACTER_BYTES  (2) /* Number of bytes occupied by each Chinese character */

/* OSD Font Step In Lib, in bytes */
#define OSD_LIB_FONT_W (s_stOSDFonts.u32FontWidth)
#define OSD_LIB_FONT_H (s_stOSDFonts.u32FontHeight)
#define OSD_LIB_FONT_STEP (OSD_LIB_FONT_W * OSD_LIB_FONT_H / BYTE_BITS)

/* OSD Parameter */
typedef struct tagOSD_PARAM_S
{
    HI_OSD_ATTR_S stAttr;
    SIZE_S stMaxSize;
    pthread_mutex_t  mutexLock;
    HI_BOOL bInit; /* OSD Attribute Set or not, Canbe modified only HI_OSD_SetAttr */
    HI_BOOL bOn; /* OSD On/Off Flag, Canbe modified only by HI_OSD_Start/HI_OSD_Stop */
} OSD_PARAM_S;

/** OSD Module Init Flag
      Canbe modified only by HI_PDT_OSD_Init/HI_PDT_OSD_DeInit */
static HI_BOOL s_bOSDInitFlg = HI_FALSE;

/** OSD Time Update Runing Flag
      Canbe modified only by HI_PDT_OSD_Init/HI_PDT_OSD_DeInit */
static HI_BOOL s_bOSDTimeRun = HI_FALSE;

/* OSD Fonts Lib, inited by HI_PDT_OSD_Init */
static HI_OSD_FONTS_S s_stOSDFonts;

/* OSD Parameter Array */
static OSD_PARAM_S s_stOSDParam[HI_OSD_MAX_CNT];

/* Time OSD Update Task Thread ID, created by HI_PDT_OSD_Init, destroyed by HI_OSD_DeInit */
static pthread_t s_OSDTimeTskId = 0;

/**
 * @brief   get time string with given format
 * @param[in]pstTime : time struct, get current system time if null
 * @param[in]enFmt : time string format, eg. YMD24H
 * @param[out]pazStr : time string buffer
 * @param[in]s32Len : time string buffer length
 */
static HI_VOID OSD_GetTimeStr(const struct tm* pstTime, HI_OSD_TIMEFMT_E enFmt, HI_CHAR* pazStr, HI_S32 s32Len)
{
    /* Get Time */
    time_t nowTime;
    struct tm stTime = {0,};

    if (!pstTime)
    {
        time(&nowTime);
        localtime_r(&nowTime, &stTime);
        pstTime = &stTime;
    }

    /* Generate Time String */
    switch (enFmt)
    {
        case HI_OSD_TIMEFMT_YMD24H:
        default:
            snprintf(pazStr, s32Len, "%04d-%02d-%02d %02d:%02d:%02d",
                     pstTime->tm_year + 1900, pstTime->tm_mon + 1, pstTime->tm_mday,
                     pstTime->tm_hour, pstTime->tm_min, pstTime->tm_sec);
            break;
    }

    return;
}

static HI_S32 OSD_Ratio2Absolute(MPP_CHN_S stChn, const POINT_S* pstRatioCoor, POINT_S* pstAbsCoor)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SIZE_S stImageSize;

    if (pstRatioCoor->s32X < 0 || pstRatioCoor->s32X > 100 || pstRatioCoor->s32Y < 0 || pstRatioCoor->s32Y > 100)
    {
        MLOGE("invalide Ratio coordinate(%d,%d)\n", pstRatioCoor->s32X, pstRatioCoor->s32Y);
        return HI_EINVAL;
    }

    switch (stChn.enModId)
    {
        case HI_ID_VI:
        {
            VI_CHN_ATTR_S stChnAttr;
            s32Ret = HI_MPI_VI_GetChnAttr(stChn.s32DevId, stChn.s32ChnId, &stChnAttr);

            if (s32Ret != HI_SUCCESS)
            {
                MLOGE("HI_MPI_VI_GetChnAttr(%d,%d) fail,Error Code: [0x%08X]\n", stChn.s32DevId, stChn.s32ChnId, s32Ret);
                return s32Ret;
            }

            stImageSize = stChnAttr.stSize;
            break;
        }

        case HI_ID_VPSS:
        {
            VPSS_CHN_ATTR_S stChnAttr;
            s32Ret = HI_MPI_VPSS_GetChnAttr(stChn.s32DevId, stChn.s32ChnId, &stChnAttr);

            if (s32Ret != HI_SUCCESS)
            {
                MLOGE("HI_MPI_VPSS_GetChnAttr(%d,%d) fail,Error Code: [0x%08X]\n", stChn.s32DevId, stChn.s32ChnId, s32Ret);
                return s32Ret;
            }

            stImageSize.u32Width = stChnAttr.u32Width;
            stImageSize.u32Height = stChnAttr.u32Height;
            break;
        }

        case HI_ID_AVS:
        {
#ifdef SUPPORT_STITCH
            AVS_CHN_ATTR_S stChnAttr;
            s32Ret = HI_MPI_AVS_GetChnAttr(stChn.s32DevId, stChn.s32ChnId, &stChnAttr);

            if (s32Ret != HI_SUCCESS)
            {
                MLOGE("HI_MPI_AVS_GetChnAttr(%d,%d) fail,Error Code: [0x%08X]\n", stChn.s32DevId, stChn.s32ChnId, s32Ret);
                return s32Ret;
            }

            stImageSize.u32Width = stChnAttr.u32Width;
            stImageSize.u32Height = stChnAttr.u32Height;
#else
            MLOGW("stitch is not support.\n");
            return HI_EPAERM;
#endif
            break;
        }

        case HI_ID_VENC:
        {
            VENC_CHN_ATTR_S stChnAttr;
            s32Ret = HI_MPI_VENC_GetChnAttr(stChn.s32ChnId, &stChnAttr);

            if (s32Ret != HI_SUCCESS)
            {
                MLOGE("HI_MPI_VENC_GetChnAttr(%d) fail,Error Code: [0x%08X]\n", stChn.s32ChnId, s32Ret);
                return s32Ret;
            }

            stImageSize.u32Width = stChnAttr.stVencAttr.u32PicWidth;
            stImageSize.u32Height = stChnAttr.stVencAttr.u32PicHeight;
            break;
        }

        case HI_ID_VO:
        {
            VO_CHN_ATTR_S stChnAttr;
            s32Ret = HI_MPI_VO_GetChnAttr(stChn.s32DevId, stChn.s32ChnId, &stChnAttr);

            if (s32Ret != HI_SUCCESS)
            {
                MLOGE("HI_MPI_VO_GetChnAttr(%d,%d) fail,Error Code: [0x%08X]\n", stChn.s32DevId, stChn.s32ChnId, s32Ret);
                return s32Ret;
            }

            stImageSize.u32Width = stChnAttr.stRect.u32Width;
            stImageSize.u32Height = stChnAttr.stRect.u32Height;
            break;
        }

        default:
            MLOGE("invalide mode id [%d]\n", stChn.enModId);
            return HI_EINVAL;
    }

    pstAbsCoor->s32X = HI_APPCOMM_ALIGN(stImageSize.u32Width * pstRatioCoor->s32X / 100, 2);
    pstAbsCoor->s32Y = HI_APPCOMM_ALIGN(stImageSize.u32Height * pstRatioCoor->s32Y / 100, 2);
    return HI_SUCCESS;
}

static HI_S32 OSD_GetNonASCNum(HI_CHAR* string,HI_S32 len)
{
    HI_S32 i,n=0;
    for(i=0;i<len;i++)
    {
        if (string[i]=='\0')
        {
            break;
        }
        if(!IsASCII(string[i]))
        {
            i++;
            n++;
        }
    }
    return n;
}

static HI_S32 OSD_UpdateTextBitmap(RGN_HANDLE RgnHdl, HI_OSD_CONTENT_S* pstContent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32StrLen = strnlen(pstContent->szStr, HI_OSD_MAX_STR_LEN);
    HI_S32 NonASCNum = OSD_GetNonASCNum(pstContent->szStr,s32StrLen);

    RGN_CANVAS_INFO_S stCanvasInfo;
    s32Ret = HI_MPI_RGN_GetCanvasInfo(RgnHdl, &stCanvasInfo);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_MPI_RGN_GetCanvasInfo fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }

    /* Generate Bitmap */
    pstContent->stBitmap.u32Width = pstContent->stFontSize.u32Width * (s32StrLen - NonASCNum*(NOASCII_CHARACTER_BYTES-1));
    pstContent->stBitmap.u32Height = pstContent->stFontSize.u32Height;
    HI_U16* puBmData = (HI_U16*)(HI_UL)stCanvasInfo.u64VirtAddr;
    HI_S32 s32BmRow, s32BmCol; /* Bitmap Row/Col Index */

    for (s32BmRow = 0; s32BmRow < pstContent->stBitmap.u32Height; ++s32BmRow)
    {
        HI_S32 NonASCShow = 0;
        for (s32BmCol = 0; s32BmCol < pstContent->stBitmap.u32Width; ++s32BmCol)
        {
            /* Bitmap Data Offset for the point */
            HI_S32 s32BmDataIdx = s32BmRow * stCanvasInfo.u32Stride / 2 + s32BmCol;
            /* Character Index in Text String */
            HI_S32 s32CharIdx = s32BmCol / pstContent->stFontSize.u32Width;
            HI_S32 s32StringIdx = s32CharIdx+NonASCShow*(NOASCII_CHARACTER_BYTES-1);
            if(NonASCNum>0&&s32CharIdx>0)
            {
                NonASCShow = OSD_GetNonASCNum(pstContent->szStr, s32StringIdx);
                s32StringIdx = s32CharIdx+NonASCShow*(NOASCII_CHARACTER_BYTES-1);
            }
            /* Point Row/Col Index in Character */
            HI_S32 s32CharCol = (s32BmCol - (pstContent->stFontSize.u32Width * s32CharIdx)) * OSD_LIB_FONT_W / pstContent->stFontSize.u32Width;
            HI_S32 s32CharRow = s32BmRow * OSD_LIB_FONT_H / pstContent->stFontSize.u32Height;
            HI_S32 s32HexOffset = s32CharRow * OSD_LIB_FONT_W / BYTE_BITS + s32CharCol / BYTE_BITS;
            HI_S32 s32BitOffset = s32CharCol % BYTE_BITS;
            HI_U8* FontMod = NULL;
            HI_S32 FontModLen = 0;
            if (HI_SUCCESS==s_stOSDFonts.pfnGetFontMod(&pstContent->szStr[s32StringIdx],&FontMod,&FontModLen))
            {
                if(FontMod!=NULL&&s32HexOffset<FontModLen)
                {
                    HI_U8 temp=FontMod[s32HexOffset];
                    if (temp >> ((BYTE_BITS - 1) - s32BitOffset) & 0x1)
                    {
                        puBmData[s32BmDataIdx] = (HI_U16)pstContent->u32Color;
                    }
                    else
                    {
                        puBmData[s32BmDataIdx] = (HI_U16)pstContent->u32BgColor;
                    }
                    continue;
                }
            }
            MLOGE("GetFontMod Fail\n");
            return HI_FAILURE;
        }

        for (s32BmCol = pstContent->stBitmap.u32Width; s32BmCol < s_stOSDParam[RgnHdl].stMaxSize.u32Width; ++s32BmCol)
        {
            HI_S32 s32BmDataIdx = s32BmRow * stCanvasInfo.u32Stride / 2 + s32BmCol;
            puBmData[s32BmDataIdx] = 0;
        }
    }

    for (s32BmRow = pstContent->stBitmap.u32Height; s32BmRow < s_stOSDParam[RgnHdl].stMaxSize.u32Height; ++s32BmRow)
    {
        for (s32BmCol = 0; s32BmCol < s_stOSDParam[RgnHdl].stMaxSize.u32Width; ++s32BmCol)
        {
            HI_S32 s32BmDataIdx = s32BmRow * stCanvasInfo.u32Stride / 2 + s32BmCol;
            puBmData[s32BmDataIdx] = 0;
        }
    }

    stCanvasInfo.enPixelFmt = PIXEL_FORMAT_ARGB_1555;
    stCanvasInfo.stSize.u32Width = pstContent->stBitmap.u32Width;
    stCanvasInfo.stSize.u32Height = pstContent->stBitmap.u32Height;

    s32Ret = HI_MPI_RGN_UpdateCanvas(RgnHdl);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_MPI_RGN_UpdateCanvas fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

static HI_S32 OSD_RGNAttach(RGN_HANDLE RgnHdl, const HI_OSD_DISP_ATTR_S* pstDispAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    RGN_CHN_ATTR_S stRgnChnAttr;
    MPP_CHN_S stChn;

    stChn.s32DevId = pstDispAttr->ModHdl;
    stChn.s32ChnId = pstDispAttr->ChnHdl;
    memset(&stRgnChnAttr, 0x0, sizeof(RGN_CHN_ATTR_S));
    stRgnChnAttr.bShow = pstDispAttr->bShow;
    stRgnChnAttr.enType = OVERLAYEX_RGN;

    switch (pstDispAttr->enBindedMod)
    {
        case HI_OSD_BINDMOD_VI:
            stChn.enModId = HI_ID_VI;
            break;

        case HI_OSD_BINDMOD_VPSS:
            stChn.enModId = HI_ID_VPSS;
            break;

        case HI_OSD_BINDMOD_AVS:
#ifdef SUPPORT_STITCH
            stChn.enModId = HI_ID_AVS;
            break;
#else
            MLOGW("stitch is not support.\n");
            return HI_EPAERM;
#endif

        case HI_OSD_BINDMOD_VENC:
            stChn.enModId = HI_ID_VENC;
            stChn.s32DevId = 0;
            stRgnChnAttr.enType = OVERLAY_RGN;
            stRgnChnAttr.unChnAttr.stOverlayChn.enAttachDest = pstDispAttr->enAttachDest;
            break;

        case HI_OSD_BINDMOD_VO:
            stChn.enModId = HI_ID_VO;
            break;

        default:
            MLOGE("RgnHdl[%d] invalide bind mode [%d]\n", RgnHdl, pstDispAttr->enBindedMod);
            return HI_EINVAL;
    }

    POINT_S stStartPos;

    if (pstDispAttr->enCoordinate == HI_OSD_COORDINATE_RATIO_COOR)
    {
        OSD_Ratio2Absolute(stChn, &pstDispAttr->stStartPos, &stStartPos);

        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("OSD_Ratio2Absolute fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
            return s32Ret;
        }
    }
    else
    {
        stStartPos = pstDispAttr->stStartPos;
    }

    if (stRgnChnAttr.enType == OVERLAYEX_RGN)
    {
        stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = stStartPos.s32X;
        stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = stStartPos.s32Y;
        stRgnChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha = pstDispAttr->u32BgAlpha;
        stRgnChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha = pstDispAttr->u32FgAlpha;
        stRgnChnAttr.unChnAttr.stOverlayExChn.u32Layer = 0;
    }
    else
    {
        stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = stStartPos.s32X;
        stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = stStartPos.s32Y;
        stRgnChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = pstDispAttr->u32BgAlpha;
        stRgnChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = pstDispAttr->u32FgAlpha;
        stRgnChnAttr.unChnAttr.stOverlayChn.u32Layer = 0;
    }

    s32Ret = HI_MPI_RGN_AttachToChn(RgnHdl, &stChn, &stRgnChnAttr);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_MPI_RGN_AttachToChn fail,RgnHdl[%d] stChn[%d,%d,%d] Error Code: [0x%08X]\n",
              RgnHdl, stChn.enModId, stChn.s32DevId, stChn.s32ChnId, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 OSD_CreateRGN(RGN_HANDLE RgnHdl, const HI_OSD_ATTR_S* pstAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32DispIdx = 0;
    RGN_ATTR_S stRgnAttr;
    stRgnAttr.enType = OVERLAY_RGN;

    for (s32DispIdx = 0; s32DispIdx < pstAttr->u32DispNum ; ++s32DispIdx)
    {
        if (pstAttr->astDispAttr[s32DispIdx].enBindedMod != HI_OSD_BINDMOD_VENC)
        {
            stRgnAttr.enType = OVERLAYEX_RGN;
            break;
        }
    }

    if (stRgnAttr.enType == OVERLAYEX_RGN)
    {
        stRgnAttr.unAttr.stOverlayEx.enPixelFmt = PIXEL_FORMAT_ARGB_1555;
        stRgnAttr.unAttr.stOverlayEx.u32BgColor = pstAttr->stContent.u32Color;
        stRgnAttr.unAttr.stOverlayEx.stSize.u32Width = pstAttr->stContent.stBitmap.u32Width;
        stRgnAttr.unAttr.stOverlayEx.stSize.u32Height = pstAttr->stContent.stBitmap.u32Height;
        stRgnAttr.unAttr.stOverlayEx.u32CanvasNum = (HI_OSD_TYPE_BITMAP == pstAttr->stContent.enType) ? 1 : 2;
    }
    else
    {
        stRgnAttr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_ARGB_1555;
        stRgnAttr.unAttr.stOverlay.u32BgColor = pstAttr->stContent.u32Color;
        stRgnAttr.unAttr.stOverlay.stSize.u32Width = pstAttr->stContent.stBitmap.u32Width;
        stRgnAttr.unAttr.stOverlay.stSize.u32Height = pstAttr->stContent.stBitmap.u32Height;
        stRgnAttr.unAttr.stOverlay.u32CanvasNum = (HI_OSD_TYPE_BITMAP == pstAttr->stContent.enType) ? 1 : 2;
    }

    s32Ret = HI_MPI_RGN_Create(RgnHdl, &stRgnAttr);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_MPI_RGN_Create fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }

    if (pstAttr->stContent.enType == HI_OSD_TYPE_BITMAP)
    {
        BITMAP_S stBitmap;
        stBitmap.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
        stBitmap.u32Width = pstAttr->stContent.stBitmap.u32Width;
        stBitmap.u32Height = pstAttr->stContent.stBitmap.u32Height;
        stBitmap.pData = pstAttr->stContent.stBitmap.pvData;
        s32Ret = HI_MPI_RGN_SetBitMap(RgnHdl, &stBitmap);

        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("HI_MPI_RGN_SetBitMap fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
            return s32Ret;
        }
    }
    else
    {
        s32Ret = OSD_UpdateTextBitmap(RgnHdl, (HI_OSD_CONTENT_S*)&pstAttr->stContent);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("OSD_UpdateTextBitmap failed\n");
            return s32Ret;
        }
    }

    for (s32DispIdx = 0; s32DispIdx < pstAttr->u32DispNum ; ++s32DispIdx)
    {
        if (pstAttr->astDispAttr[s32DispIdx].bShow)
        {
            OSD_RGNAttach(RgnHdl, &pstAttr->astDispAttr[s32DispIdx]);
        }
    }

    return HI_SUCCESS;
}

static HI_S32 OSD_RGNDetach(RGN_HANDLE RgnHdl, const HI_OSD_DISP_ATTR_S* pstDispAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stChn;

    stChn.s32DevId = pstDispAttr->ModHdl;
    stChn.s32ChnId = pstDispAttr->ChnHdl;
    switch (pstDispAttr->enBindedMod)
    {
        case HI_OSD_BINDMOD_VI:
            stChn.enModId = HI_ID_VI;
            break;

        case HI_OSD_BINDMOD_VPSS:
            stChn.enModId = HI_ID_VPSS;
            break;

        case HI_OSD_BINDMOD_AVS:
            stChn.enModId = HI_ID_AVS;
            break;

        case HI_OSD_BINDMOD_VENC:
            stChn.s32DevId = 0;
            stChn.enModId = HI_ID_VENC;
            break;

        case HI_OSD_BINDMOD_VO:
            stChn.enModId = HI_ID_VO;
            break;

        default:
            MLOGE("RgnHdl[%d] invalide bind mode [%d]\n", RgnHdl, pstDispAttr->enBindedMod);
            return HI_EINVAL;
    }

    s32Ret = HI_MPI_RGN_DetachFromChn(RgnHdl, &stChn);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_MPI_RGN_DetachFromChn fail,RgnHdl[%d] stChn[%d,%d,%d] Error Code: [0x%08X]\n",
              RgnHdl, stChn.enModId, stChn.s32DevId, stChn.s32ChnId, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 OSD_DestroyRGN(RGN_HANDLE RgnHdl, const HI_OSD_ATTR_S* pstAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32DispIdx = 0;

    for (s32DispIdx = 0; s32DispIdx < pstAttr->u32DispNum ; ++s32DispIdx)
    {
        if (pstAttr->astDispAttr[s32DispIdx].bShow)
        {
            OSD_RGNDetach(RgnHdl, &pstAttr->astDispAttr[s32DispIdx]);
        }
    }

    s32Ret = HI_MPI_RGN_Destroy(RgnHdl);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_MPI_RGN_Destroy fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 OSD_Update(RGN_HANDLE RgnHdl, const HI_OSD_ATTR_S* pstAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32DispIdx = 0;
    RGN_CHN_ATTR_S stRgnChnAttr;
    MPP_CHN_S stChn;

    for (s32DispIdx = 0; s32DispIdx < pstAttr->u32DispNum ; ++s32DispIdx)
    {
        if (!pstAttr->astDispAttr[s32DispIdx].bShow)
        {
            /*not no show,no need to update*/
            continue;
        }

        stChn.s32DevId = pstAttr->astDispAttr[s32DispIdx].ModHdl;
        stChn.s32ChnId = pstAttr->astDispAttr[s32DispIdx].ChnHdl;
        switch (pstAttr->astDispAttr[s32DispIdx].enBindedMod)
        {
            case HI_OSD_BINDMOD_VI:
                stChn.enModId = HI_ID_VI;
                break;

            case HI_OSD_BINDMOD_VPSS:
                stChn.enModId = HI_ID_VPSS;
                break;

            case HI_OSD_BINDMOD_AVS:
                stChn.enModId = HI_ID_AVS;
                break;

            case HI_OSD_BINDMOD_VENC:
                stChn.s32DevId = 0;
                stChn.enModId = HI_ID_VENC;
                break;

            case HI_OSD_BINDMOD_VO:
                stChn.enModId = HI_ID_VO;
                break;

            default:
                MLOGE("RgnHdl[%d] invalide bind mode [%d]\n", RgnHdl, pstAttr->astDispAttr[s32DispIdx].enBindedMod);
                return HI_EINVAL;
        }

        s32Ret = HI_MPI_RGN_GetDisplayAttr(RgnHdl, &stChn, &stRgnChnAttr);

        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("HI_MPI_RGN_GetDisplayAttr fail,RgnHdl[%d] stChn[%d,%d,%d] Error Code: [0x%08X]\n",
                  RgnHdl, stChn.enModId, stChn.s32DevId, stChn.s32ChnId, s32Ret);
            return s32Ret;
        }

        stRgnChnAttr.bShow = pstAttr->astDispAttr[s32DispIdx].bShow;

        POINT_S stStartPos;

        if (pstAttr->astDispAttr[s32DispIdx].enCoordinate == HI_OSD_COORDINATE_RATIO_COOR)
        {
            s32Ret = OSD_Ratio2Absolute(stChn, &pstAttr->astDispAttr[s32DispIdx].stStartPos, &stStartPos);

            if (s32Ret != HI_SUCCESS)
            {
                MLOGE("OSD_Ratio2Absolute fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
                return s32Ret;
            }
        }
        else
        {
            stStartPos = pstAttr->astDispAttr[s32DispIdx].stStartPos;
        }

        if (stRgnChnAttr.enType == OVERLAYEX_RGN)
        {
            stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = stStartPos.s32X;
            stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = stStartPos.s32Y;
            stRgnChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha = pstAttr->astDispAttr[s32DispIdx].u32BgAlpha;
            stRgnChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha = pstAttr->astDispAttr[s32DispIdx].u32FgAlpha;
        }
        else
        {
            stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = stStartPos.s32X;
            stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = stStartPos.s32Y;
            stRgnChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = pstAttr->astDispAttr[s32DispIdx].u32BgAlpha;
            stRgnChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = pstAttr->astDispAttr[s32DispIdx].u32FgAlpha;
        }

        s32Ret = HI_MPI_RGN_SetDisplayAttr(RgnHdl, &stChn, &stRgnChnAttr);

        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("HI_MPI_RGN_SetDisplayAttr fail,RgnHdl[%d] stChn[%d,%d,%d] Error Code: [0x%08X]\n",
                  RgnHdl, stChn.enModId, stChn.s32DevId, stChn.s32ChnId, s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

/**
 * @brief   time osd update task
 * @param[in]pvParam : nonuse
 * @return 0 success,non-zero error code.
 */
static HI_VOID* OSD_TimeUpdate(HI_VOID* pvParam)
{
    HI_S32 s32Ret = 0;
    HI_S32 s32OsdIdx = 0;
    time_t nowTime = 0;
    time_t lastTime = 0;
    struct tm stTime = {0,};
    prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);

    while (s_bOSDTimeRun)
    {
        time(&nowTime);

        if (nowTime == lastTime)
        {
            HI_usleep(10 * 1000);
            continue;
        }
        else
        {
            localtime_r(&nowTime, &stTime);

            for (s32OsdIdx = 0; s32OsdIdx < HI_OSD_MAX_CNT; ++s32OsdIdx)
            {
                HI_MUTEX_LOCK(s_stOSDParam[s32OsdIdx].mutexLock);

                if (s_stOSDParam[s32OsdIdx].stAttr.stContent.enType == HI_OSD_TYPE_TIME && s_stOSDParam[s32OsdIdx].bOn)
                {
                    /* Update OSD Time String */
                    OSD_GetTimeStr(&stTime, s_stOSDParam[s32OsdIdx].stAttr.stContent.enTimeFmt, s_stOSDParam[s32OsdIdx].stAttr.stContent.szStr, HI_OSD_MAX_STR_LEN);

                    /* Update OSD Text Bitmap */
                    s32Ret = OSD_UpdateTextBitmap(s32OsdIdx, &s_stOSDParam[s32OsdIdx].stAttr.stContent);

                    if (HI_SUCCESS != s32Ret)
                    {
                        HI_MUTEX_UNLOCK(s_stOSDParam[s32OsdIdx].mutexLock);
                        MLOGE("Update Text Bitmap failed\n");
                        continue;
                    }

                    /* Update OSD Attribute */
                    s32Ret = OSD_Update(s32OsdIdx, &s_stOSDParam[s32OsdIdx].stAttr);

                    if (HI_SUCCESS != s32Ret)
                    {
                        MLOGE("Update Attribute failed\n");
                    }
                }

                HI_MUTEX_UNLOCK(s_stOSDParam[s32OsdIdx].mutexLock);
            }

            lastTime = nowTime; /* update time */
        }

        HI_usleep(500 * 1000);
    }

    return NULL;
}

static HI_S32 OSD_Start(HI_S32 s32OsdIdx)
{
    HI_S32 s32Ret = HI_SUCCESS;
    OSD_PARAM_S* pstOsdParam = &s_stOSDParam[s32OsdIdx];

    /* Time OSD: Update time string and bitmap */
    if (HI_OSD_TYPE_TIME == pstOsdParam->stAttr.stContent.enType)
    {
        OSD_GetTimeStr(NULL, pstOsdParam->stAttr.stContent.enTimeFmt, pstOsdParam->stAttr.stContent.szStr, HI_OSD_MAX_STR_LEN);
    }

    s32Ret = OSD_CreateRGN(s32OsdIdx, &pstOsdParam->stAttr);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("OSD_CreateRGN s32OsdIdx[%d] failed:[0x%08X]\n", s32OsdIdx, s32Ret);
        return s32Ret;
    }

    pstOsdParam->bOn = HI_TRUE;
    return HI_SUCCESS;
}

static HI_S32 OSD_Stop(HI_S32 s32OsdIdx)
{
    HI_S32 s32Ret = HI_SUCCESS;
    OSD_PARAM_S* pstOsdParam = &s_stOSDParam[s32OsdIdx];

    s32Ret = OSD_DestroyRGN(s32OsdIdx, &pstOsdParam->stAttr);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("OSD_DestroyRGN s32OsdIdx[%d] failed:[0x%08X]\n", s32OsdIdx, s32Ret);
        return s32Ret;
    }

    pstOsdParam->bOn = HI_FALSE;
    return HI_SUCCESS;
}

/**
 * @brief    osd module initialization, eg. create time osd update task.
 * @param[in] pstFonts:osd fonts lib
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Init(const HI_OSD_FONTS_S* pstFonts)
{
    HI_APPCOMM_CHECK_EXPR(HI_TRUE != s_bOSDInitFlg, HI_EINITIALIZED);
    HI_APPCOMM_CHECK_POINTER(pstFonts, HI_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstFonts->pfnGetFontMod, HI_EINVAL);

    if (pstFonts->u32FontWidth % BYTE_BITS)
    {
        MLOGE("FontWidth must be a multiple of %d.", BYTE_BITS);
        return HI_EINVAL;
    }

    HI_S32 s32Idx = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    /* Init OSD Param */
    for (s32Idx = 0; s32Idx < HI_OSD_MAX_CNT; ++s32Idx)
    {
        HI_MUTEX_INIT_LOCK(s_stOSDParam[s32Idx].mutexLock);
        HI_MUTEX_LOCK(s_stOSDParam[s32Idx].mutexLock);
        memset(&s_stOSDParam[s32Idx], 0, sizeof(OSD_PARAM_S));
        HI_MUTEX_UNLOCK(s_stOSDParam[s32Idx].mutexLock);
    }

    memcpy(&s_stOSDFonts, pstFonts, sizeof(HI_OSD_FONTS_S));

    /* Create Time OSD Update Thread */
    s_bOSDTimeRun = HI_TRUE;
    s32Ret = pthread_create(&s_OSDTimeTskId, NULL, OSD_TimeUpdate, NULL);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("create OSD_TimeUpdate failed:%s\n", strerror(errno));
        return HI_ENORES;
    }
#if defined(AMP_LINUX_HUAWEILITE)
    extern HI_S32 OSD_MSG_Register(HI_VOID);
    OSD_MSG_Register();
#endif
    s_bOSDInitFlg = HI_TRUE;
    return HI_SUCCESS;
}

/**
 * @brief    osd module de-initialization, eg. destroy time osd update task, release resource.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Deinit(HI_VOID)
{
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bOSDInitFlg, HI_ENOINIT);
    /* Destroy Time OSD Update Task */
    s_bOSDTimeRun = HI_FALSE;
    pthread_join(s_OSDTimeTskId, NULL);

    HI_S32 s32Idx = 0;

    for (s32Idx = 0; s32Idx < HI_OSD_MAX_CNT; ++s32Idx)
    {
        HI_MUTEX_LOCK(s_stOSDParam[s32Idx].mutexLock);
        /* Stop Osd */
        OSD_Stop(s32Idx);

        /* Release Time/String Bitmap Memory */
        if ((HI_OSD_TYPE_TIME == s_stOSDParam[s32Idx].stAttr.stContent.enType
             || HI_OSD_TYPE_STRING == s_stOSDParam[s32Idx].stAttr.stContent.enType)
            && s_stOSDParam[s32Idx].stAttr.stContent.stBitmap.pvData)
        {
            s_stOSDParam[s32Idx].stAttr.stContent.stBitmap.u64PhyAddr = 0;
        }

        HI_MUTEX_UNLOCK(s_stOSDParam[s32Idx].mutexLock);
        HI_MUTEX_DESTROY(s_stOSDParam[s32Idx].mutexLock);
    }

    s_bOSDInitFlg = HI_FALSE;

    return HI_SUCCESS;
}

/**
 * @brief    set osd attribute.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @param[in] pstAttr:osd configure attribute
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_SetAttr(HI_S32 s32OsdIdx, const HI_OSD_ATTR_S* pstAttr)
{
    /* Check Module Init or not */
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bOSDInitFlg, HI_ENOINIT);
    /* Check Input Param */
    HI_APPCOMM_CHECK_EXPR(0 <= s32OsdIdx, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_OSD_MAX_CNT > s32OsdIdx, HI_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_OSD_MAX_DISP_CNT >= pstAttr->u32DispNum, HI_EINVAL);

    if (HI_OSD_TYPE_BITMAP == pstAttr->stContent.enType)
    {
        HI_APPCOMM_CHECK_POINTER(pstAttr->stContent.stBitmap.pvData, HI_EINVAL);
    }

    HI_S32 s32Ret = HI_SUCCESS;
    OSD_PARAM_S* pstOsdParam = &s_stOSDParam[s32OsdIdx];
    HI_MUTEX_LOCK(pstOsdParam->mutexLock);
    HI_BOOL bOn = pstOsdParam->bOn;

    /* Update Attribute */
    pstOsdParam->stAttr.stContent.u32Color = pstAttr->stContent.u32Color;
    pstOsdParam->stAttr.stContent.u32BgColor = pstAttr->stContent.u32BgColor;

    if (HI_OSD_TYPE_BITMAP != pstAttr->stContent.enType)
    {
        if (HI_OSD_TYPE_TIME == pstAttr->stContent.enType)
        {
            /* Time Type: Update time string */
            pstOsdParam->stAttr.stContent.enTimeFmt = pstAttr->stContent.enTimeFmt;
            OSD_GetTimeStr(NULL, pstAttr->stContent.enTimeFmt, ((HI_OSD_ATTR_S*)pstAttr)->stContent.szStr, HI_OSD_MAX_STR_LEN);
        }

        /* Update string */
        snprintf(pstOsdParam->stAttr.stContent.szStr, HI_OSD_MAX_STR_LEN, "%s", pstAttr->stContent.szStr);
        pstOsdParam->stAttr.stContent.stBitmap.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
        ((HI_OSD_ATTR_S*)pstAttr)->stContent.stBitmap.u32Width = pstAttr->stContent.stFontSize.u32Width * strnlen(pstOsdParam->stAttr.stContent.szStr, HI_OSD_MAX_STR_LEN);
        ((HI_OSD_ATTR_S*)pstAttr)->stContent.stBitmap.u32Height= pstAttr->stContent.stFontSize.u32Height;
        pstOsdParam->stAttr.stContent.stFontSize = pstAttr->stContent.stFontSize;
    }
    else
    {
        pstOsdParam->stAttr.stContent.stBitmap.enPixelFormat = pstAttr->stContent.stBitmap.enPixelFormat;
        pstOsdParam->stAttr.stContent.stBitmap.u64PhyAddr = pstAttr->stContent.stBitmap.u64PhyAddr;
        pstOsdParam->stAttr.stContent.stBitmap.pvData = pstAttr->stContent.stBitmap.pvData;
    }
    pstOsdParam->stAttr.stContent.stBitmap.u32Width = pstAttr->stContent.stBitmap.u32Width;
    pstOsdParam->stAttr.stContent.stBitmap.u32Height = pstAttr->stContent.stBitmap.u32Height;
    pstOsdParam->stAttr.stContent.enType = pstAttr->stContent.enType;

    if (bOn)
    {
        if (pstOsdParam->stMaxSize.u32Width < pstOsdParam->stAttr.stContent.stBitmap.u32Width
            || pstOsdParam->stMaxSize.u32Height < pstOsdParam->stAttr.stContent.stBitmap.u32Height)
        {
            MLOGI("RGN(%d) size increase[%d,%d->%d,%d], rebuild\n", s32OsdIdx,
                pstOsdParam->stMaxSize.u32Width,pstOsdParam->stMaxSize.u32Height,
                pstAttr->stContent.stBitmap.u32Width,pstAttr->stContent.stBitmap.u32Height);
            /*rebuild RGN*/
            s32Ret = OSD_DestroyRGN(s32OsdIdx, &pstOsdParam->stAttr);

            if (HI_SUCCESS != s32Ret)
            {
                HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
                return s32Ret;
            }

            s32Ret = OSD_CreateRGN(s32OsdIdx, pstAttr);

            if (HI_SUCCESS != s32Ret)
            {
                HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
                return s32Ret;
            }
        }
        else
        {
            /*Update RGN Content*/
            if (pstAttr->stContent.enType == HI_OSD_TYPE_BITMAP)
            {
                BITMAP_S stBitmap;
                stBitmap.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
                stBitmap.u32Width = pstAttr->stContent.stBitmap.u32Width;
                stBitmap.u32Height = pstAttr->stContent.stBitmap.u32Height;
                stBitmap.pData = pstAttr->stContent.stBitmap.pvData;
                s32Ret = HI_MPI_RGN_SetBitMap(s32OsdIdx, &stBitmap);

                if (s32Ret != HI_SUCCESS)
                {
                    HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
                    MLOGE("HI_MPI_RGN_SetBitMap fail,RgnHdl[%d] Error Code: [0x%08X]\n", s32OsdIdx, s32Ret);
                    return s32Ret;
                }
            }
            else
            {
                /* Time/String Type: Update text bitmap */
                s32Ret = OSD_UpdateTextBitmap(s32OsdIdx, (HI_OSD_CONTENT_S*)&pstOsdParam->stAttr.stContent);

                if (HI_SUCCESS != s32Ret)
                {
                    HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
                    return s32Ret;
                }
            }

            HI_S32 s32DispIdx = 0;
            for (s32DispIdx = 0; s32DispIdx < pstAttr->u32DispNum ; ++s32DispIdx)
            {
                if (HI_TRUE==pstAttr->astDispAttr[s32DispIdx].bShow&&HI_FALSE==pstOsdParam->stAttr.astDispAttr[s32DispIdx].bShow)
                {
                    OSD_RGNAttach(s32OsdIdx, &pstAttr->astDispAttr[s32DispIdx]);
                }
                else if(HI_FALSE==pstAttr->astDispAttr[s32DispIdx].bShow&&HI_TRUE==pstOsdParam->stAttr.astDispAttr[s32DispIdx].bShow)
                {
                    OSD_RGNDetach(s32OsdIdx, &pstOsdParam->stAttr.astDispAttr[s32DispIdx]);
                }
            }

            s32Ret = OSD_Update(s32OsdIdx, pstAttr);

            if (HI_SUCCESS != s32Ret)
            {
                HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
                return s32Ret;
            }
        }
    }

    memcpy(pstOsdParam->stAttr.astDispAttr, pstAttr->astDispAttr, sizeof(HI_OSD_DISP_ATTR_S) * HI_OSD_MAX_DISP_CNT);
    pstOsdParam->stAttr.u32DispNum = pstAttr->u32DispNum;

#if 0
    MLOGD("Idx: %d\n", s32OsdIdx);
    MLOGD("Content: type[%d], timeFmt[%d], Color[%08x], BgColor[%08x], Str[%s] FontSize[%u,%u]\n",
          pstOsdParam->stAttr.stContent.enType, pstOsdParam->stAttr.stContent.enTimeFmt,
          pstOsdParam->stAttr.stContent.u32Color, pstOsdParam->stAttr.stContent.u32BgColor,
          pstOsdParam->stAttr.stContent.szStr, pstOsdParam->stAttr.stContent.stFontSize.u32Width,
          pstOsdParam->stAttr.stContent.stFontSize.u32Height);
    MLOGD("Bitmap: PixelFmt[%d], Size[%u,%u]\n", pstOsdParam->stAttr.stContent.stBitmap.enPixelFormat,
          pstOsdParam->stAttr.stContent.stBitmap.u32Width, pstOsdParam->stAttr.stContent.stBitmap.u32Height);
#endif
    pstOsdParam->stMaxSize.u32Width = MAX(pstOsdParam->stMaxSize.u32Width,pstOsdParam->stAttr.stContent.stBitmap.u32Width);
    pstOsdParam->stMaxSize.u32Height= MAX(pstOsdParam->stMaxSize.u32Height,pstOsdParam->stAttr.stContent.stBitmap.u32Height);
    pstOsdParam->bInit = HI_TRUE;
    HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
    return HI_SUCCESS;
}

/**
 * @brief    get osd attribute.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @param[out] pstAttr:osd configure attribute
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_GetAttr(HI_S32 s32OsdIdx, HI_OSD_ATTR_S* pstAttr)
{
    /* Check Module Init or not */
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bOSDInitFlg, HI_ENOINIT);
    /* Check Input Param */
    HI_APPCOMM_CHECK_EXPR(0 <= s32OsdIdx, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_OSD_MAX_CNT > s32OsdIdx, HI_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_EINVAL);

    OSD_PARAM_S* pstOsdParam = &s_stOSDParam[s32OsdIdx];
    HI_MUTEX_LOCK(pstOsdParam->mutexLock);

    /* Check OSD Attrbute init or not */
    if (!pstOsdParam->bInit)
    {
        HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
        MLOGE("OSD[%d] not init yet!\n", s32OsdIdx);
        return HI_EINVAL;
    }

    memcpy(pstAttr, &pstOsdParam->stAttr, sizeof(HI_OSD_ATTR_S));
    HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
    return HI_SUCCESS;
}

/**
 * @brief    start osd by index.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Start(HI_S32 s32OsdIdx)
{
    /* Check Module Init or not */
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bOSDInitFlg, HI_ENOINIT);
    /* Check Input Param */
    HI_APPCOMM_CHECK_EXPR(0 <= s32OsdIdx, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_OSD_MAX_CNT > s32OsdIdx, HI_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    OSD_PARAM_S* pstOsdParam = &s_stOSDParam[s32OsdIdx];

    HI_MUTEX_LOCK(pstOsdParam->mutexLock);

    /* Check OSD Attrbute init or not */
    if (!pstOsdParam->bInit)
    {
        HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
        MLOGE("OSD[%d] not init yet!\n", s32OsdIdx);
        return HI_EINVAL;
    }

    /* Check OSD stop or not */
    if (pstOsdParam->bOn)
    {
        HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
        MLOGW("OSD[%d] has already started!\n", s32OsdIdx);
        return HI_SUCCESS;
    }

    s32Ret = OSD_Start(s32OsdIdx);
    HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
    return s32Ret;
}

/**
 * @brief    stop osd by index.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Stop(HI_S32 s32OsdIdx)
{
    /* Check Module Init or not */
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bOSDInitFlg, HI_ENOINIT);
    /* Check Input Param */
    HI_APPCOMM_CHECK_EXPR(0 <= s32OsdIdx, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_OSD_MAX_CNT > s32OsdIdx, HI_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    OSD_PARAM_S* pstOsdParam = &s_stOSDParam[s32OsdIdx];

    HI_MUTEX_LOCK(pstOsdParam->mutexLock);

    /* Check OSD Attrbute init or not */
    if (!pstOsdParam->bInit)
    {
        HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
        return HI_SUCCESS;
    }

    /* Check OSD stop or not */
    if (!pstOsdParam->bOn)
    {
        HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
        return HI_SUCCESS;
    }

    s32Ret = OSD_Stop(s32OsdIdx);
    pstOsdParam->stMaxSize.u32Width = 0;
    pstOsdParam->stMaxSize.u32Height= 0;
    HI_MUTEX_UNLOCK(pstOsdParam->mutexLock);
    return s32Ret;
}

HI_S32 HI_OSD_Batch(HI_S32 s32Batch, HI_BOOL bShow)
{
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bOSDInitFlg, HI_ENOINIT);
    HI_S32 s32OsdIdx = 0;
    HI_S32 s32DispIdx = 0;

    for (s32OsdIdx = 0; s32OsdIdx < HI_OSD_MAX_CNT; ++s32OsdIdx)
    {
        HI_MUTEX_LOCK(s_stOSDParam[s32OsdIdx].mutexLock);

        if (s_stOSDParam[s32OsdIdx].bOn)
        {
            for (s32DispIdx = 0; s32DispIdx < s_stOSDParam[s32OsdIdx].stAttr.u32DispNum ; ++s32DispIdx)
            {
                if (s_stOSDParam[s32OsdIdx].stAttr.astDispAttr[s32DispIdx].s32Batch == s32Batch)
                {
                    if (s_stOSDParam[s32OsdIdx].stAttr.astDispAttr[s32DispIdx].bShow == HI_FALSE && bShow == HI_TRUE)
                    {
                        OSD_RGNAttach(s32OsdIdx, &s_stOSDParam[s32OsdIdx].stAttr.astDispAttr[s32DispIdx]);
                    }
                    else if (s_stOSDParam[s32OsdIdx].stAttr.astDispAttr[s32DispIdx].bShow == HI_TRUE && bShow == HI_FALSE)
                    {
                        OSD_RGNDetach(s32OsdIdx, &s_stOSDParam[s32OsdIdx].stAttr.astDispAttr[s32DispIdx]);
                    }
                    else
                    {
                        /*on show not change,do nothing*/
                        continue;
                    }

                    s_stOSDParam[s32OsdIdx].stAttr.astDispAttr[s32DispIdx].bShow = bShow;
                }
            }

            OSD_Update(s32OsdIdx, &s_stOSDParam[s32OsdIdx].stAttr);
        }

        HI_MUTEX_UNLOCK(s_stOSDParam[s32OsdIdx].mutexLock);
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

