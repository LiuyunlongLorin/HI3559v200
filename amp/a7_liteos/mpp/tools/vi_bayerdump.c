#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifndef __HuaweiLite__
#include <sys/poll.h>
#endif
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vi.h"
#include "hi_comm_isp.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_isp.h"


#define MAX_FRM_CNT     25
#define MAX_FRM_WIDTH   8192

#define ALIGN_BACK(x, a)              ((a) * (((x) / (a))))

void Usage(void)
{
    printf(
        "\n"
        "*************************************************\n"
        "Usage: ./vi_bayerdump [ViPipe] [nbit] [Compress] [FrmCnt] [ByteAlign] [RatioShow]\n"
        "ViPipe: \n"
        "   0:ViPipe0 ~ 3:ViPipe 3\n"
        "nbit: \n"
        "   The bit num to be dump\n"
        "Compress: \n"
        "   Whether to  dump compressed raw, default is 0\n"
        "FrmCnt: \n"
        "   the count of frame to be dump\n"
        "ByteAlign: \n"
        "   Whether convert to Byte align, default is 1\n"
        "RatioShow: \n"
        "   Whether the file name of raw show ratio info, default is 0\n"
        "e.g : ./vi_bayerdump  0 16  0 2 1 0\n"
        "*************************************************\n"
        "\n");
    exit(1);
}

HI_S32 GetDumpPipe(VI_PIPE ViPipe, VI_DEV_BIND_PIPE_S *pstDevBindPipe)
{
    HI_S32 s32Ret;
    VI_DEV ViDev = 0;
    HI_S32 i,j;
    VI_DEV_ATTR_S      stDevAttr;
    VI_DEV_BIND_PIPE_S stTmpDevBindPipe;

    for(i = 0; i < VI_MAX_DEV_NUM; i++)
    {
        s32Ret = HI_MPI_VI_GetDevBindPipe(i, &stTmpDevBindPipe);
        if(HI_SUCCESS == s32Ret)
        {
            for(j = 0; j < stTmpDevBindPipe.u32Num; j++)
            {
                if(ViPipe == stTmpDevBindPipe.PipeId[j])
                {
                    ViDev = i;
                    break;
                }
            }
        }
    }

    memset(pstDevBindPipe, 0, sizeof(VI_DEV_BIND_PIPE_S));

    s32Ret = HI_MPI_VI_GetDevBindPipe(ViDev, pstDevBindPipe);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VI_GetDevBindPipe error 0x%x !\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_GetDevAttr(ViDev, &stDevAttr);

    if (HI_SUCCESS != s32Ret)
    {
        printf("Get dev %d attr failed!\n", ViDev);
        return HI_FAILURE;
    }

    if(WDR_MODE_NONE == stDevAttr.stWDRAttr.enWDRMode || WDR_MODE_BUILT_IN == stDevAttr.stWDRAttr.enWDRMode)
    {
        pstDevBindPipe->u32Num = 1;
        pstDevBindPipe->PipeId[0] = ViPipe;
    }

    return HI_SUCCESS;
}

static inline HI_S32 BitWidth2PixelFormat(HI_U32 u32Nbit, PIXEL_FORMAT_E *penPixelFormat)
{
    PIXEL_FORMAT_E enPixelFormat;

    if (8 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_8BPP;
    }
    else if (10 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_10BPP;
    }
    else if (12 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
    }
    else if (14 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_14BPP;
    }
    else if (16 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_16BPP;
    }
    else
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_16BPP;
    }

    *penPixelFormat = enPixelFormat;
    return HI_SUCCESS;
}

HI_S32 ConvertBitPixel(HI_U8 *pu8Data, HI_U32 u32DataNum, HI_U32 u32BitWidth, HI_U16 *pu16OutData)
{
    HI_BOOL bReverse = HI_TRUE;
    HI_S32  i, u32Tmp, s32OutCnt;
    HI_U32  u32Val;
    HI_U64  u64Val;
    HI_U8   *pu8Tmp = pu8Data;

    s32OutCnt = 0;
    switch(u32BitWidth)
    {
    case 10:
        {
            /* 4 pixels consist of 5 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++)
            {
                /* byte4 byte3 byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 5 * i;
                u64Val = pu8Tmp[0] + ((HI_U32)pu8Tmp[1] << 8) + ((HI_U32)pu8Tmp[2] << 16) +
                         ((HI_U32)pu8Tmp[3] << 24) + ((HI_U64)pu8Tmp[4] << 32);

                pu16OutData[s32OutCnt++] = u64Val & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 10) & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 20) & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 30) & 0x3ff;
            }
        }
        break;

    case 12:
        {
            /* 2 pixels consist of 3 bytes  */
            u32Tmp = u32DataNum / 2;

            for (i = 0; i < u32Tmp; i++)
            {
                /* byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 3 * i;

                if (HI_FALSE == bReverse)
                {
                    u32Val = pu8Tmp[0] + (pu8Tmp[1] << 8) + (pu8Tmp[2] << 16);
                    pu16OutData[s32OutCnt++] = u32Val & 0xfff;
                    pu16OutData[s32OutCnt++] = (u32Val >> 12) & 0xfff;
                }
                else
                {
                    pu16OutData[s32OutCnt++] = (((pu8Tmp[1] & 0xf) << 8) + pu8Tmp[0]) << 4;
                    pu16OutData[s32OutCnt++] = ((pu8Tmp[2] << 4) + ((pu8Tmp[1] & 0xf0) >> 4)) << 4;
                }
            }
        }
        break;

    case 14:
        {
            /* 4 pixels consist of 7 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++)
            {
                pu8Tmp = pu8Data + 7 * i;
                u64Val = pu8Tmp[0] + ((HI_U32)pu8Tmp[1] << 8) + ((HI_U32)pu8Tmp[2] << 16) +
                         ((HI_U32)pu8Tmp[3] << 24) + ((HI_U64)pu8Tmp[4] << 32) +
                         ((HI_U64)pu8Tmp[5] << 40) + ((HI_U64)pu8Tmp[6] << 48);

                pu16OutData[s32OutCnt++] = u64Val & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 14) & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 28) & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 42) & 0x3fff;
            }
        }
        break;
    default:
        fprintf(stderr, "unsuport bitWidth: %d\n", u32BitWidth);
        return -1;
        break;
    }

    return s32OutCnt;
}

int SampleSaveCompressParam(HI_VOID* pCmpParam, HI_U32 u32Size, FILE* pfd)
{
    fwrite(pCmpParam, u32Size, 1, pfd);
    fflush(pfd);
    return HI_SUCCESS;
}

int SampleSaveCompressedRaw(VIDEO_FRAME_S* pVBuf, HI_U32 u32Nbit, FILE* pfd)
{
    HI_U32 u32H;
    HI_U32 u32DataSize;
    HI_U16 u16HeadData = 0x0;
    HI_U64 phy_addr, size;
    HI_U8* pUserPageAddr[2];
    HI_U8  *pu8Data;
    PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_BUTT;

    BitWidth2PixelFormat(u32Nbit, &enPixelFormat);
    if (enPixelFormat != pVBuf->enPixelFormat)
    {
        fprintf(stderr, "Cmp: invalid pixel format:%d, u32Nbit: %d\n", pVBuf->enPixelFormat, u32Nbit);
        return HI_FAILURE;
    }

    size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);
    phy_addr = pVBuf->u64PhyAddr[0];


    pUserPageAddr[0] = (HI_U8*) HI_MPI_SYS_Mmap(phy_addr, size);
    if (NULL == pUserPageAddr[0])
    {
        return HI_FAILURE;
    }

    pu8Data = pUserPageAddr[0];

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......dump data......u32Stride[0]: %d, width: %d\n", pVBuf->u32Stride[0], pVBuf->u32Width);
    fflush(stderr);

    for (u32H = 0; u32H < pVBuf->u32Height; u32H++)
    {
        u16HeadData = *(HI_U16*)pu8Data;

        u32DataSize =  (u16HeadData + 1) * 16;

        fwrite(pu8Data, u32DataSize, 1, pfd);

        pu8Data += pVBuf->u32Stride[0];
    }
    fflush(pfd);

    fprintf(stderr, "done u32TimeRef: %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
    pUserPageAddr[0] = NULL;

    return HI_SUCCESS;
}


int SampleSaveUncompressRaw(VIDEO_FRAME_S* pVBuf, HI_U32 u32Nbit, FILE* pfd, HI_U32 u32ByteAlign)
{
    HI_U32 u32H;
    HI_U16 *pu16Data = NULL;
    HI_U64 phy_addr, size;
    HI_U32 u32TmpStride;
    HI_U8* pUserPageAddr[2];
    HI_U8  *pu8Data;
    PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_BUTT;

    BitWidth2PixelFormat(u32Nbit, &enPixelFormat);
    if (enPixelFormat != pVBuf->enPixelFormat)
    {
        fprintf(stderr, "NoCmp: invalid pixel format:%d, u32Nbit: %d\n", pVBuf->enPixelFormat, u32Nbit);
        return HI_FAILURE;
    }

    if (1 == u32ByteAlign)
    {
        u32TmpStride = 2 * pVBuf->u32Width;
    }
    else
    {
        u32TmpStride = pVBuf->u32Stride[0];
    }

    size = u32TmpStride * (pVBuf->u32Height);
    phy_addr = pVBuf->u64PhyAddr[0];

    pUserPageAddr[0] = (HI_U8*) HI_MPI_SYS_Mmap(phy_addr, size);
    if (NULL == pUserPageAddr[0])
    {
        return HI_FAILURE;
    }

    pu8Data = pUserPageAddr[0];
    if ((8 != u32Nbit) && (16 != u32Nbit))
    {
        pu16Data = (HI_U16*)malloc(pVBuf->u32Width * 2U);
        if (NULL == pu16Data)
        {
            fprintf(stderr, "alloc memory failed\n");
            HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
            pUserPageAddr[0] = NULL;
            return HI_FAILURE;
        }
    }

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......dump data......u32Stride[0]: %d, width: %d\n", pVBuf->u32Stride[0], pVBuf->u32Width);
    fflush(stderr);

    for (u32H = 0; u32H < pVBuf->u32Height; u32H++)
    {
        if (8 == u32Nbit)
        {
            fwrite(pu8Data, pVBuf->u32Width, 1, pfd);
        }
        else if (16 == u32Nbit)
        {
            fwrite(pu8Data, pVBuf->u32Width, 2, pfd);
            fflush(pfd);
        }
        else
        {
            if (1 == u32ByteAlign)
            {
                ConvertBitPixel(pu8Data, pVBuf->u32Width, u32Nbit, pu16Data);
                fwrite(pu16Data, pVBuf->u32Width, 2, pfd);
            }
            else
            {
                if (0 == ((pVBuf->u32Width * u32Nbit) % 8))
                {
                    fwrite(pu8Data, pVBuf->u32Width * u32Nbit / 8, 1, pfd); //-- pVBuf->u32Width * u32Nbit / 8
                }
                else
                {
                    fwrite(pu8Data, ((pVBuf->u32Width * u32Nbit) / 8 + 8), 1, pfd);
                }

            }
        }
        pu8Data += pVBuf->u32Stride[0];
    }
    fflush(pfd);

    fprintf(stderr, "u32Nbit_%d done u32TimeRef: %d, u32ByteAlign: %d!\n", u32Nbit, pVBuf->u32TimeRef, u32ByteAlign);
    fflush(stderr);

    if (NULL != pu16Data)
    {
        free(pu16Data);
    }

    HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
    pUserPageAddr[0] = NULL;

    return HI_SUCCESS;
}

int SampleBayerDump(VIDEO_FRAME_S* pVBuf, HI_U32 u32Nbit, FILE* pfd, HI_U32 u32ByteAlign)
{
    if(COMPRESS_MODE_NONE == pVBuf->enCompressMode)
    {
        return SampleSaveUncompressRaw(pVBuf, u32Nbit, pfd, u32ByteAlign);
    }
    else
    {
        return SampleSaveCompressedRaw(pVBuf, u32Nbit, pfd);
    }
}

char* CompressModeToString(COMPRESS_MODE_E enCompressMode)
{
    if(COMPRESS_MODE_NONE == enCompressMode)
    {
        return "CMP_NONE";
    }
    else if(COMPRESS_MODE_LINE == enCompressMode)
    {
        return "CMP_LINE";
    }
    else if(COMPRESS_MODE_SEG == enCompressMode)
    {
        return "CMP_SEG";
    }
    else
    {
        return "CMP_XXX";
    }
}

char* IspBayerFormatToString(ISP_BAYER_FORMAT_E  enBayer)
{
    if(BAYER_RGGB == enBayer)
    {
        return "RGGB";
    }
    else if(BAYER_GRBG == enBayer)
    {
        return "GRBG";
    }
    else if(BAYER_GBRG == enBayer)
    {
        return "GBRG";
    }
    else if(BAYER_BGGR == enBayer)
    {
        return "BGGR";
    }
    else
    {
        return "RGGB";
    }
}

HI_S32 DumpLinearBayer(VI_PIPE ViPipe, HI_U32 u32Nbit, COMPRESS_MODE_E enCompressMode, HI_U32 u32Cnt, HI_U32 u32ByteAlign, HI_U32 u32RatioShow)
{
    HI_S32                 s32Ret        = HI_SUCCESS;
    int                    i, j;
    HI_CHAR                szYuvName[256] = {0};
    FILE*                  pfd = NULL;
    HI_S32                 s32MilliSec = 4000;
    HI_U32                 u32CapCnt   = 0;
    HI_U64                 u64IspInfoPhyAddr = 0;
    VI_CMP_PARAM_S         stCmpPara;
    ISP_FRAME_INFO_S*      pstIspFrameInfo;
    ISP_PUB_ATTR_S         stPubAttr;
    VIDEO_FRAME_INFO_S     astFrame[MAX_FRM_CNT];
    VB_SUPPLEMENT_CONFIG_S stSupplementConfig;
    PIXEL_FORMAT_E         enPixelFormat;

    BitWidth2PixelFormat(u32Nbit, &enPixelFormat);

    while (1)
    {
        if (HI_SUCCESS != HI_MPI_VI_GetPipeFrame(ViPipe, &astFrame[0], s32MilliSec))
        {
            printf("Linear: get vi Pipe %d frame err\n", ViPipe);
            break;
        }

        if ((astFrame[0].stVFrame.enCompressMode == enCompressMode)
            && (astFrame[0].stVFrame.enPixelFormat == enPixelFormat))
        {
            HI_MPI_VI_ReleasePipeFrame(ViPipe, &astFrame[0]);
            break;
        }

        HI_MPI_VI_ReleasePipeFrame(ViPipe, &astFrame[0]);
    }

    /* get VI frame  */
    for (i = 0; i < u32Cnt; i++)
    {
        if (HI_SUCCESS != HI_MPI_VI_GetPipeFrame(ViPipe, &astFrame[i], s32MilliSec))
        {
            printf("Linear: get vi Pipe %d frame err\n", ViPipe);
            printf("only get %d frame\n", i);
            break;
        }

        printf("Linear: get vi Pipe %d frame num %d ok\n",ViPipe,  i);
    }

    u32CapCnt = i;

    if (0 == u32CapCnt)
    {
        return -1;
    }

    /* make file name */
    if (0 == u32RatioShow)
    {
        snprintf(szYuvName, 256, "./vi_Pipe_%d_%d_%d_%d_%dbits_%s_%d_%d.raw", ViPipe,
                astFrame[0].stVFrame.u32Width,  astFrame[0].stVFrame.u32Height,
                u32CapCnt, u32Nbit, CompressModeToString(astFrame[0].stVFrame.enCompressMode), u32ByteAlign, u32RatioShow);
    }
    else
    {

        s32Ret = HI_MPI_VB_GetSupplementConfig(&stSupplementConfig);
        if (HI_SUCCESS != s32Ret)
        {
            printf("\nHI_MPI_VB_GetSupplementConfig err res %x \n", s32Ret);
            goto end;
        }

        if (0 == (VB_SUPPLEMENT_ISPINFO_MASK & stSupplementConfig.u32SupplementConfig))
        {
            printf("\nNot set VB_SUPPLEMENT_ISPINFO_MASK,u32SupplementConfig:0x%x !\n",stSupplementConfig.u32SupplementConfig);
            goto end;
        }

        s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe,&stPubAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("\nHI_MPI_ISP_GetPubAttr err res %x \n", s32Ret);
            goto end;
        }

        u64IspInfoPhyAddr = astFrame[0].stVFrame.stSupplement.u64IspInfoPhyAddr;
        pstIspFrameInfo = HI_MPI_SYS_Mmap(u64IspInfoPhyAddr, sizeof(ISP_FRAME_INFO_S));
        if (HI_NULL == pstIspFrameInfo)
        {
            printf("\nHI_MPI_SYS_Mmap err ,u64IspInfoPhyAddr:0x%llx\n", u64IspInfoPhyAddr);
            goto end;
        }

        snprintf(szYuvName, 256, "./HisiRAW_%lld_%d_%d_%d_%dbits_%s_Linear_Ratio%d_ISO%d_AG%d_DG%d_IspDG%d_Exp%dus_Cnt%d.raw",
                astFrame[u32Cnt-1].stVFrame.u64PTS, pstIspFrameInfo->u32SensorID, astFrame[0].stVFrame.u32Width,  astFrame[0].stVFrame.u32Height, u32Nbit,
                IspBayerFormatToString(stPubAttr.enBayer), pstIspFrameInfo->au32Ratio[0], pstIspFrameInfo->u32ISO, pstIspFrameInfo->u32Again,
                pstIspFrameInfo->u32Dgain, pstIspFrameInfo->u32IspDgain, pstIspFrameInfo->u32ExposureTime, u32Cnt);

        if (256 < sizeof(szYuvName))
        {
            printf("The Length of your RAW name %u is too large , need smaller than 256.\n", sizeof(szYuvName));
        }

        HI_MPI_SYS_Munmap(pstIspFrameInfo, sizeof(ISP_FRAME_INFO_S));
    }

    /* open file */
    pfd = fopen(szYuvName, "wb");

    if (NULL == pfd)
    {
        printf("open file failed:%s!\n", strerror(errno));

        goto end;
    }

    if(COMPRESS_MODE_NONE != astFrame[0].stVFrame.enCompressMode)
    {
        if(HI_SUCCESS != HI_MPI_VI_GetPipeCmpParam(ViPipe, &stCmpPara))
        {
            printf("HI_MPI_VI_GetPipeCmpParam failed!\n");

            for (j = 0; j < u32CapCnt; j++)
            {
                HI_MPI_VI_ReleasePipeFrame(ViPipe, &astFrame[j]);
            }
        }

        SampleSaveCompressParam(&stCmpPara, sizeof(VI_CMP_PARAM_S), pfd);
    }

    for (j = 0; j < u32CapCnt; j++)
    {
        /* save VI frame to file */
        SampleBayerDump(&astFrame[j].stVFrame, u32Nbit, pfd, u32ByteAlign);

        /* release frame after using */
        HI_MPI_VI_ReleasePipeFrame(ViPipe, &astFrame[j]);
    }

    fclose(pfd);

    return HI_SUCCESS;

end:

    for (j = 0; j < u32CapCnt; j++)
    {
        HI_MPI_VI_ReleasePipeFrame(ViPipe, &astFrame[j]);
    }

    return HI_FAILURE;
}

HI_S32 DumpWDRBayer(HI_U32 u32PipeNum, VI_PIPE ViPipeId[], COMPRESS_MODE_E enCompressMode, HI_U32 u32Nbit, HI_U32 u32Cnt, HI_U32 u32ByteAlign, HI_U32 u32RatioShow)
{
    HI_S32                 s32Ret        = HI_SUCCESS;
    int                    i, j, k;
    HI_U32                 u32GetFrameCnt;
    FILE*                  pfd = NULL;
    HI_S32                 s32MilliSec = 4000;
    HI_U64                 u64IspInfoPhyAddr = 0;
    HI_CHAR                szYuvName[256] = {0};
    VI_CMP_PARAM_S         stCmpPara;
    ISP_PUB_ATTR_S         stPubAttr;
    ISP_FRAME_INFO_S*      pstIspFrameInfo;
    VIDEO_FRAME_INFO_S*    pastFrame[MAX_FRM_CNT] =  {0};
    VIDEO_FRAME_INFO_S     astFrame[MAX_FRM_CNT];
    VB_SUPPLEMENT_CONFIG_S stSupplementConfig;
    PIXEL_FORMAT_E         enPixelFormat;

    if (0 == u32PipeNum)
    {
        printf("u32PipeNum is %d, fail.\n", u32PipeNum);
        return HI_FAILURE;
    }

    BitWidth2PixelFormat(u32Nbit, &enPixelFormat);

    for (i = 0; i < u32PipeNum; i++)
    {
        while (1)
        {
            if (HI_SUCCESS != HI_MPI_VI_GetPipeFrame(i, &astFrame[0], s32MilliSec))
            {
                printf("Linear: get vi Pipe %d frame err\n", i);
                break;
            }

            if ((astFrame[0].stVFrame.enCompressMode == enCompressMode)
                && (astFrame[0].stVFrame.enPixelFormat == enPixelFormat))
            {
                HI_MPI_VI_ReleasePipeFrame(i, &astFrame[0]);
                break;
            }

            HI_MPI_VI_ReleasePipeFrame(i, &astFrame[0]);
        }
    }


    for (i = 0; i < u32PipeNum; i++)
    {
        pastFrame[i] = (VIDEO_FRAME_INFO_S*)malloc(u32Cnt * sizeof(VIDEO_FRAME_INFO_S));

        if (HI_NULL == pastFrame[i])
        {
            printf("malloc %u fail\n", (u32Cnt * sizeof(VIDEO_FRAME_INFO_S)));

            for (j = i - 1; j >= 0; j--)
            {
                free(pastFrame[j]);
            }

            return HI_FAILURE;
        }
    }

    for (i = 0; i < u32Cnt; i++)
    {
        for (j = 0; j <= u32PipeNum - 1; j++)
        {
            if (HI_SUCCESS != HI_MPI_VI_GetPipeFrame(ViPipeId[j], &pastFrame[j][i], s32MilliSec))
            {
                printf("WDR: get vi Pipe %d frame err\n", ViPipeId[j]);
                printf("only get %d frame\n", i);
                for (k = j + 1; k < u32PipeNum; k++)
                {
                    HI_MPI_VI_ReleasePipeFrame(ViPipeId[k], &pastFrame[k][i]);
                }
                goto getframeok;
            }
            else
            {

                printf("get vi Pipe %d frame OK\n", ViPipeId[j]);
            }
        }

    }

getframeok:

    u32GetFrameCnt = i;

    if (0 == u32GetFrameCnt)
    {
        goto end;
    }

    /* make file name */
    if (0 == u32RatioShow)
    {
        snprintf(szYuvName, 256, "./vi_Pipe_%d_%d_%d_%d_%dbits_%s_%d_lineWDR.raw", ViPipeId[0],
                pastFrame[0][0].stVFrame.u32Width,  pastFrame[0][0].stVFrame.u32Height,
                u32GetFrameCnt*u32PipeNum, u32Nbit, CompressModeToString(pastFrame[0][0].stVFrame.enCompressMode), u32ByteAlign);
    }
    else
    {
        s32Ret = HI_MPI_VB_GetSupplementConfig(&stSupplementConfig);
        if (HI_SUCCESS != s32Ret)
        {
            printf("\nHI_MPI_VB_GetSupplementConfig err res %x \n", s32Ret);
            goto end;
        }

        if (0 == (VB_SUPPLEMENT_ISPINFO_MASK & stSupplementConfig.u32SupplementConfig))
        {
            printf("\nNot set VB_SUPPLEMENT_ISPINFO_MASK,u32SupplementConfig:0x%x !\n",stSupplementConfig.u32SupplementConfig);
            goto end;
        }

        s32Ret = HI_MPI_ISP_GetPubAttr(ViPipeId[0],&stPubAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("\nHI_MPI_ISP_GetPubAttr err res %x \n", s32Ret);
            goto end;
        }

        u64IspInfoPhyAddr = pastFrame[0][0].stVFrame.stSupplement.u64IspInfoPhyAddr;
        pstIspFrameInfo = HI_MPI_SYS_Mmap(u64IspInfoPhyAddr, sizeof(ISP_FRAME_INFO_S));

        if(HI_NULL == pstIspFrameInfo)
        {
            printf("\nHI_MPI_SYS_Mmap err ,u64IspInfoPhyAddr:0x%llx\n", u64IspInfoPhyAddr);
            goto end;
        }

        /* pstSaveFrame[0],, maybe not be exactly, eg. au32Ratio[1] */
        if (4 == u32PipeNum)
        {
            snprintf(szYuvName, 256, "./HisiRAW_%lld_%d_%d_%d_%dbits_%s_WDR_Ratio(0)%d_Ratio(1)%d_Ratio(2)%d_ISO%d_AG%d_DG%d_IspDG%d_Exp%dus_Cnt%d.raw",
                    pastFrame[0][0].stVFrame.u64PTS, pstIspFrameInfo->u32SensorID, pastFrame[0][0].stVFrame.u32Width,  pastFrame[0][0].stVFrame.u32Height, u32Nbit,
                    IspBayerFormatToString(stPubAttr.enBayer), pstIspFrameInfo->au32Ratio[0], pstIspFrameInfo->au32Ratio[1], pstIspFrameInfo->au32Ratio[2],
                    pstIspFrameInfo->u32ISO, pstIspFrameInfo->u32Again, pstIspFrameInfo->u32Dgain, pstIspFrameInfo->u32IspDgain,
                    pstIspFrameInfo->u32ExposureTime, u32GetFrameCnt*u32PipeNum);
        }
        else if (3 == u32PipeNum)
        {
            snprintf(szYuvName, 256, "./HisiRAW_%lld_%d_%d_%d_%dbits_%s_WDR_Ratio(0)%d_Ratio(1)%d_ISO%d_AG%d_DG%d_IspDG%d_Exp%dus_Cnt%d.raw",
                    pastFrame[0][0].stVFrame.u64PTS, pstIspFrameInfo->u32SensorID, pastFrame[0][0].stVFrame.u32Width,  pastFrame[0][0].stVFrame.u32Height, u32Nbit,
                    IspBayerFormatToString(stPubAttr.enBayer), pstIspFrameInfo->au32Ratio[0], pstIspFrameInfo->au32Ratio[1], pstIspFrameInfo->u32ISO,
                    pstIspFrameInfo->u32Again, pstIspFrameInfo->u32Dgain, pstIspFrameInfo->u32IspDgain, pstIspFrameInfo->u32ExposureTime, u32GetFrameCnt*u32PipeNum);
        }
        else
        {
            snprintf(szYuvName, 256, "./HisiRAW_%lld_%d_%d_%d_%dbits_%s_WDR_Ratio(0)%d_ISO%d_AG%d_DG%d_IspDG%d_Exp%dus_Cnt%d.raw",
                    pastFrame[0][0].stVFrame.u64PTS, pstIspFrameInfo->u32SensorID, pastFrame[0][0].stVFrame.u32Width,  pastFrame[0][0].stVFrame.u32Height, u32Nbit,
                    IspBayerFormatToString(stPubAttr.enBayer), pstIspFrameInfo->au32Ratio[0], pstIspFrameInfo->u32ISO, pstIspFrameInfo->u32Again,
                    pstIspFrameInfo->u32Dgain, pstIspFrameInfo->u32IspDgain, pstIspFrameInfo->u32ExposureTime, u32GetFrameCnt*u32PipeNum);
        }


        if (256 < sizeof(szYuvName))
        {
            printf("The Length of your RAW name %u is too large , need smaller than 256.\n", sizeof(szYuvName));
        }

        HI_MPI_SYS_Munmap(pstIspFrameInfo, sizeof(ISP_FRAME_INFO_S));
    }

    /* open file */
    pfd = fopen(szYuvName, "wb");

    if (NULL == pfd)
    {
        printf("Open file failed!\n");
        goto end;
    }

    /* dump file */
    for (i = 0; i < u32GetFrameCnt; i++)
    {
        for (j = 0; j <= u32PipeNum - 1; j++)
        {
            if (COMPRESS_MODE_NONE != pastFrame[j][i].stVFrame.enCompressMode)
            {
                if (HI_SUCCESS != HI_MPI_VI_GetPipeCmpParam(ViPipeId[i], &stCmpPara))
                {
                    printf("HI_MPI_VI_GetPipeCmpParam failed cmp:%d %d %d !\n",pastFrame[j][i].stVFrame.enCompressMode,i ,j);

                    goto end;
                }

                SampleSaveCompressParam(&stCmpPara, sizeof(VI_CMP_PARAM_S), pfd);
            }
            else
            {
                /* save VI frame to file */
                SampleBayerDump(&pastFrame[j][i].stVFrame, u32Nbit, pfd, u32ByteAlign);
            }
        }
    }

end:

    if (NULL != pfd)
    {
        fclose(pfd);
    }

    for (i = 0; i < u32GetFrameCnt; i++)
    {
        for (j = 0; j <= u32PipeNum - 1; j++)
        {
            HI_MPI_VI_ReleasePipeFrame(ViPipeId[j], &pastFrame[j][i]);
        }
    }

    for (i = 0; i < u32PipeNum; i++)
    {
        free(pastFrame[i]);
    }

    return HI_SUCCESS;
}


#ifdef __HuaweiLite__
HI_S32 vi_bayerdump(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    VI_PIPE           ViPipe               = 0;
    HI_S32            s32Ret               = 0;
    HI_U32            i                    = 0;
    HI_U32            u32Nbit              = 16;
    HI_U32            u32FrmCnt            = 1;
    HI_U32            u32RawDepth          = 2;
    HI_U32            u32ByteAlign         = 1;
    HI_U32            u32RatioShow         = 0;
    COMPRESS_MODE_E   enCompressMode       = COMPRESS_MODE_NONE;
    PIXEL_FORMAT_E    enPixFmt;
    VI_DEV_BIND_PIPE_S stDevBindPipe;
    VI_DUMP_ATTR_S    astBackUpDumpAttr[4];
    VI_DUMP_ATTR_S    stDumpAttr;
    VI_PIPE_ATTR_S    astBackUpPipeAttr[4];
    VI_PIPE_ATTR_S    stPipeAttr;


    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\t To see more usage, please enter: ./vi_bayerdump -h\n\n");

    if (argc > 1)
    {
        if (!strncmp(argv[1], "-h", 2))
        {
            Usage();
            exit(HI_SUCCESS);
        }
        else
        {
            ViPipe = atoi(argv[1]); /* pipe*/
        }
    }

    if (argc > 2)
    {
        u32Nbit = atoi(argv[2]);    /* bit width of raw data:8/10/12/14/16bit */

        if (8 != u32Nbit &&  10 != u32Nbit &&  12 != u32Nbit &&  14 != u32Nbit &&  16 != u32Nbit)
        {
            printf("Can't not support %d bits, only support 8bits,10bits,12bits,14bits,16bits\n", u32Nbit);
            exit(HI_FAILURE);
        }

    }

    if (argc > 3)
    {
        if (atoi(argv[3]) > 0)
        {
            enCompressMode = COMPRESS_MODE_LINE ;
        }
        else
        {
            enCompressMode = COMPRESS_MODE_NONE;
        }
    }

    if (argc > 4)
    {
        u32FrmCnt = atoi(argv[4]);  /* the frame number */
    }

    if (argc > 5)
    {
        if (atoi(argv[5]) > 0)
        {
            u32ByteAlign = 1 ;
        }
        else
        {
            u32ByteAlign = 0;
        }
    }

    if (argc > 6)
    {
        if (atoi(argv[6]) > 0)
        {
            u32RatioShow = 1 ;
        }
        else
        {
            u32RatioShow = 0;
        }
    }

    if (1 > u32FrmCnt || MAX_FRM_CNT < u32FrmCnt)
    {
        printf("invalid FrmCnt %d, FrmCnt range from 1 to %d\n", u32FrmCnt, MAX_FRM_CNT);
        exit(HI_FAILURE);
    }


    s32Ret = GetDumpPipe(ViPipe, &stDevBindPipe);

    if (HI_SUCCESS != s32Ret)
    {
        printf("getDumpPipe failed 0x%x!\n", s32Ret);
        return HI_ERR_VI_INVALID_PARA;
    }

    BitWidth2PixelFormat(u32Nbit, &enPixFmt);

    for (i = 0; i < stDevBindPipe.u32Num; i++)
    {
        s32Ret = HI_MPI_VI_GetPipeDumpAttr(stDevBindPipe.PipeId[i], &astBackUpDumpAttr[i]);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Get Pipe %d dump attr failed!\n", ViPipe);
            return s32Ret;
        }

        memcpy(&stDumpAttr, &astBackUpDumpAttr[i], sizeof(VI_DUMP_ATTR_S));
        stDumpAttr.bEnable  = HI_TRUE;
        stDumpAttr.u32Depth = u32RawDepth;

        s32Ret = HI_MPI_VI_SetPipeDumpAttr(stDevBindPipe.PipeId[i], &stDumpAttr);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Set Pipe %d dump attr failed!\n", stDevBindPipe.PipeId[i]);
            return s32Ret;
        }

        s32Ret = HI_MPI_VI_GetPipeAttr(stDevBindPipe.PipeId[i], &astBackUpPipeAttr[i]);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Get Pipe %d attr failed!\n", ViPipe);
            return s32Ret;
        }

        memcpy(&stPipeAttr, &astBackUpPipeAttr[i], sizeof(VI_PIPE_ATTR_S));
        stPipeAttr.enPixFmt = enPixFmt;
        stPipeAttr.enCompressMode = enCompressMode;

        s32Ret = HI_MPI_VI_SetPipeAttr(stDevBindPipe.PipeId[i], &stPipeAttr);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Set Pipe %d attr failed!\n", ViPipe);
            return s32Ret;
        }
    }


    if (1 == stDevBindPipe.u32Num)
    {
        DumpLinearBayer(ViPipe, u32Nbit, enCompressMode, u32FrmCnt, u32ByteAlign, u32RatioShow);
    }
    else
    {
        DumpWDRBayer(stDevBindPipe.u32Num, stDevBindPipe.PipeId, enCompressMode, u32Nbit, u32FrmCnt, u32ByteAlign, u32RatioShow);
    }

    for (i = 0; i < stDevBindPipe.u32Num; i++)
    {
        s32Ret = HI_MPI_VI_SetPipeAttr(stDevBindPipe.PipeId[i], &astBackUpPipeAttr[i]);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Set Pipe %d attr failed!\n", ViPipe);
            return s32Ret;
        }

        s32Ret = HI_MPI_VI_SetPipeDumpAttr(stDevBindPipe.PipeId[i], &astBackUpDumpAttr[i]);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Set Pipe %d dump attr failed!\n", ViPipe);
            return s32Ret;
        }
    }

    return s32Ret;
}
