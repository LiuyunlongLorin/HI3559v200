#include <errno.h>
#include "loadbmp.h"
#include "hi_osd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define COMMAND_PARAM_CNT (3)

static inline HI_VOID usage()
{
    printf("usage: ./loadbmp xxx.bmp xxx.bin\n");
}

HI_S32 LoadBmpData(const HI_CHAR* filename, BITMAP_S* pstBitmap)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;
    HI_S32 s32BytesPerPix = 2;

    if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0)
    {
        MLOGE("GetBmpInfo err!\n");
        return HI_FAILURE;
    }

    HI_U32 datasize = s32BytesPerPix * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight);
    pstBitmap->pData = malloc(datasize);

    if (NULL == pstBitmap->pData)
    {
        MLOGE("malloc osd memroy (%u Bytes) err!\n", datasize);
        return HI_FAILURE;
    }

    Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;

    if (CreateSurfaceByBitMap(filename, &Surface, (HI_U8*)(pstBitmap->pData)) < 0)
    {
        MLOGE("CreateSurfaceByBitMap err!\n");
        free(pstBitmap->pData);
        return HI_FAILURE;
    }

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    return datasize;
}

static HI_S32 SaveBmp2Bin(const BITMAP_S* pstBitmap, HI_U32 datasize, const HI_CHAR* pszFilePath)
{
    FILE* fp = NULL;
    HI_S32 ret = 0;
    fp = fopen(pszFilePath, "w+b");

    if (!fp)
    {
        MLOGE("open [%s] failed %s\n", pszFilePath, strerror(errno));
        return HI_FAILURE;
    }

    ret = fwrite(&pstBitmap->enPixelFormat, 1, sizeof(PIXEL_FORMAT_E), fp);

    if (ret != sizeof(PIXEL_FORMAT_E))
    {
        MLOGE("fwrite : total %lu, write %u\n", (HI_UL)sizeof(PIXEL_FORMAT_E), ret);
        goto end;
    }

    ret = fwrite(&pstBitmap->u32Width, 1, sizeof(HI_U32), fp);

    if (ret != sizeof(HI_U32))
    {
        MLOGE("fwrite : total %lu, write %u\n", (HI_UL)sizeof(HI_U32), ret);
        goto end;
    }

    ret = fwrite(&pstBitmap->u32Height, 1, sizeof(HI_U32), fp);

    if (ret != sizeof(HI_U32))
    {
        MLOGE("fwrite : total %lu, write %u\n", (HI_UL)sizeof(HI_U32), ret);
        goto end;
    }

    ret = fwrite(pstBitmap->pData, 1, datasize, fp);

    if (ret != datasize)
    {
        MLOGE("fwrite : total %lu, write %u\n", (HI_UL)datasize, ret);
        goto end;
    }

    fflush(fp);
end:
    fclose(fp);
    return HI_SUCCESS;
}

HI_S32 main(HI_S32 argc, HI_CHAR** argv)
{
    HI_S32 s32Idx = 0;

    /* check command line param */
    if (COMMAND_PARAM_CNT != argc)
    {
        MLOGE("invalid input param number(%d)!\n", argc);

        for (s32Idx = 0; s32Idx < argc; ++s32Idx)
        {
            MLOGE("param[%d] %s\n", s32Idx, argv[s32Idx]);
        }

        usage();
        exit(1);
    }

    for (s32Idx = 0; s32Idx < argc; ++s32Idx)
    {
        MLOGD("param[%d] %s\n", s32Idx, argv[s32Idx]);
    }

    HI_S32 s32Ret = 0;
    HI_CHAR* pszInPut = argv[1];
    HI_CHAR* pszOutPut = argv[2];
    BITMAP_S stBitmap =
    {
        .enPixelFormat = PIXEL_FORMAT_ARGB_1555,
        .u32Width = 0,
        .u32Height = 0,
        .pData = NULL,
    };
    s32Ret = LoadBmpData(pszInPut, &stBitmap);

    if (s32Ret > 0 && stBitmap.pData != NULL)
    {
        SaveBmp2Bin(&stBitmap, (HI_U32)s32Ret, pszOutPut);
    }

    HI_APPCOMM_SAFE_FREE(stBitmap.pData);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

