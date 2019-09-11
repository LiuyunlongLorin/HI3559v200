#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "hi_math.h"
#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#include "hi_comm_vb.h"
#include "mpi_vb.h"
#include "hi_comm_vi.h"
#include "mpi_vi.h"
#include "mpi_vgs.h"

#define MAX_FRM_WIDTH   8192

#define VALUE_BETWEEN(x,min,max) (((x)>=(min)) && ((x) <= (max)))

typedef struct hiDUMP_MEMBUF_S
{
    VB_BLK  hBlock;
    VB_POOL hPool;
    HI_U32  u32PoolId;

    HI_U32  u64PhyAddr;
    HI_U8*  pVirAddr;
    HI_S32  s32Mdev;
} DUMP_MEMBUF_S;

static HI_U32 u32SignalFlag = 0;

static VI_PIPE ViPipe = 0;
static HI_U32 u32OrigDepth = 0;
static VIDEO_FRAME_INFO_S stFrame;

static VI_DUMP_ATTR_S astBackUpDumpAttr;

static VB_POOL hPool  = VB_INVALID_POOLID;
static DUMP_MEMBUF_S stMem = {0};
static VGS_HANDLE hHandle = -1;
static HI_U32  u32BlkSize = 0;

static HI_CHAR* pUserPageAddr[2] = {HI_NULL,HI_NULL};
static HI_U32 u32Size = 0;

static FILE* pfd = HI_NULL;

/*When saving a file,sp420 will be denoted by p420 and sp422 will be denoted by p422 in the name of the file */
static void sample_yuv_8bit_dump(VIDEO_FRAME_S* pVBuf, FILE* pfd)
{
    unsigned int w, h;
    char* pVBufVirt_Y;
    char* pVBufVirt_C;
    char* pMemContent;
    unsigned char TmpBuff[MAX_FRM_WIDTH];

    HI_U32 phy_addr;
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    VIDEO_FORMAT_E  enVideoFormat = stFrame.stVFrame.enVideoFormat;
    HI_U32 u32UvHeight = 0;

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == enPixelFormat)
    {
        if (VIDEO_FORMAT_TILE_16x8 != enVideoFormat)
        {
            u32Size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height) * 3 / 2;
            u32UvHeight = pVBuf->u32Height / 2;
        }
        else
        {
            u32Size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);
            u32UvHeight = 0;
        }
    }
    else if(PIXEL_FORMAT_YVU_SEMIPLANAR_422 == enPixelFormat)
    {
        u32Size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height) * 2;
        u32UvHeight = pVBuf->u32Height;
    }
    else if(PIXEL_FORMAT_YUV_400 == enPixelFormat)
    {
        u32Size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);
        u32UvHeight = pVBuf->u32Height;
    }

    phy_addr = pVBuf->u64PhyAddr[0];

    //printf("phy_addr:%x, size:%d\n", phy_addr, size);
    pUserPageAddr[0] = (HI_CHAR*) HI_MPI_SYS_Mmap(phy_addr, u32Size);
    if (HI_NULL == pUserPageAddr[0])
    {
        return;
    }
    //printf("stride: %d,%d\n",pVBuf->u32Stride[0],pVBuf->u32Stride[1] );

    pVBufVirt_Y = pUserPageAddr[0];
    pVBufVirt_C = pVBufVirt_Y + (pVBuf->u32Stride[0]) * (pVBuf->u32Height);

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......Y......");
    fflush(stderr);
    if(VIDEO_FORMAT_TILE_16x8 == enVideoFormat)
    {
        for (h = 0; h < pVBuf->u32Height; h++)
        {
            pMemContent = pVBufVirt_Y + h * pVBuf->u32Stride[0];
            fwrite(pMemContent, pVBuf->u32Stride[0], 1, pfd);
        }
    }
    else
    {
        for (h = 0; h < pVBuf->u32Height; h++)
        {
            pMemContent = pVBufVirt_Y + h * pVBuf->u32Stride[0];
            fwrite(pMemContent, pVBuf->u32Width, 1, pfd);
        }
    }

    if(PIXEL_FORMAT_YUV_400 != enPixelFormat && VIDEO_FORMAT_TILE_16x8 != enVideoFormat)
    {
        fflush(pfd);
        /* save U ----------------------------------------------------------------*/
        fprintf(stderr, "U......");
        fflush(stderr);

        for (h = 0; h < u32UvHeight; h++)
        {
            pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

            pMemContent += 1;

            for (w = 0; w < pVBuf->u32Width / 2; w++)
            {
                TmpBuff[w] = *pMemContent;
                pMemContent += 2;
            }
            fwrite(TmpBuff, pVBuf->u32Width / 2, 1, pfd);
        }
        fflush(pfd);

        /* save V ----------------------------------------------------------------*/
        fprintf(stderr, "V......");
        fflush(stderr);
        for (h = 0; h < u32UvHeight; h++)
        {
            pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

            for (w = 0; w < pVBuf->u32Width / 2; w++)
            {
                TmpBuff[w] = *pMemContent;
                pMemContent += 2;
            }
            fwrite(TmpBuff, pVBuf->u32Width / 2, 1, pfd);
        }
    }
    fflush(pfd);

    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], u32Size);
    pUserPageAddr[0] = HI_NULL;
}


void sample_yuv_10bit_dump(VIDEO_FRAME_S* pVBuf, FILE* pfd, HI_U32 u32ByteAlign)
{
    unsigned int w, h, k, wy, wuv;
    char* pVBufVirt_Y;
    char* pVBufVirt_C;
    char* pMemContent;
    char* pMem;
    HI_U32 phy_addr;
    HI_U16  src[MAX_FRM_WIDTH];
    HI_U8  dest[MAX_FRM_WIDTH];
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    VIDEO_FORMAT_E  enVideoFormat = stFrame.stVFrame.enVideoFormat;
    HI_U32 u32UvHeight = 0;
    HI_U32 u32YWidth;

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == enPixelFormat)
    {
        u32Size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height) * 3 / 2;
        u32UvHeight = pVBuf->u32Height / 2;
    }
    else if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == enPixelFormat)
    {
        u32Size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height) * 2;
        u32UvHeight = pVBuf->u32Height;
    }
    else if (PIXEL_FORMAT_YUV_400 == enPixelFormat)
    {
        u32Size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);
        u32UvHeight = pVBuf->u32Height;
    }

    u32YWidth = (pVBuf->u32Width * 10 + 7) / 8;

    phy_addr = pVBuf->u64PhyAddr[0];
    pUserPageAddr[0] = (HI_CHAR*) HI_MPI_SYS_Mmap(phy_addr, u32Size);

    if (HI_NULL == pUserPageAddr[0])
    {
        return;
    }

    pVBufVirt_Y = pUserPageAddr[0];
    pVBufVirt_C = pVBufVirt_Y + (pVBuf->u32Stride[0]) * (pVBuf->u32Height);

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......Y......");
    fflush(stderr);

    if (VIDEO_FORMAT_TILE_16x8 == enVideoFormat)
    {
        for (h = 0; h < pVBuf->u32Height; h++)
        {
            pMemContent = pVBufVirt_Y + h * pVBuf->u32Stride[0];
            fwrite(pMemContent, pVBuf->u32Stride[0], 1, pfd);
        }
    }

    else
    {
        /* 16_byte align */
        for (h = 0; h < pVBuf->u32Height; h++)
        {
            pMemContent = pVBufVirt_Y + h * pVBuf->u32Stride[0];

            if (1 == u32ByteAlign)
            {
                wy = 0;

                for (w = 0; w < u32YWidth - 1; w++)
                {
                    dest[w] = *pMemContent;
                    dest[w + 1] = *(pMemContent + 1);
                    k = wy % 4;

                    switch (k)
                    {
                        case 0:
                            src[wy] = (((HI_U16)(dest[w]))) + (((dest[w + 1]) & 0x3) << 8);
                            break;

                        case 1:
                            src[wy] = ((((HI_U16)(dest[w])) & 0xfc) >> 2) + (((HI_U16)(dest[w + 1]) & 0xf) << 6);
                            break;

                        case 2:
                            src[wy] = ((((HI_U16)(dest[w])) & 0xf0) >> 4) + (((HI_U16)(dest[w + 1]) & 0x3f) << 4);
                            break;

                        case 3:
                            src[wy] = ((((HI_U16)(dest[w])) & 0xc0) >> 6) + ((HI_U16)(dest[w + 1]) << 2);
                            w++;
                            pMemContent += 1;
                            break;
                    }

                    pMemContent += 1;
                    wy++;
                }

                fwrite(src, pVBuf->u32Width * 2, 1, pfd);
            }
            else
            {
                fwrite(pMemContent, pVBuf->u32Stride[0], 1, pfd);
            }
        }

    }


    if (PIXEL_FORMAT_YUV_400 != enPixelFormat && VIDEO_FORMAT_TILE_16x8 != enVideoFormat)
    {
        fflush(pfd);
        /* save U ----------------------------------------------------------------*/
        fprintf(stderr, "U......");
        fflush(stderr);

        if (1 == u32ByteAlign)
        {
            for (h = 0; h < u32UvHeight; h++)
            {
                pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1]; // pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

                //pMemContent += 1;
                wy = 0;
                wuv = 0;

                for (w = 0; w < u32YWidth - 1; w++)
                {
                    dest[w] = *pMemContent;
                    dest[w + 1] = *(pMemContent + 1);
                    k = wuv % 4;

                    switch (k)
                    {
                        case 0:
                            // src[wy] = (((HI_U16)(dest[w]))) + (((dest[w+1])&0x3)<<8);
                            break;

                        case 1:
                            src[wy] = ((((HI_U16)(dest[w])) & 0xfc) >> 2) + (((HI_U16)(dest[w + 1]) & 0xf) << 6);
                            wy++;
                            break;

                        case 2:
                            //src[wy] = ((((HI_U16)(dest[w]))&0xf0)>>4) + (((HI_U16)(dest[w+1])&0x3f)<<4);
                            break;

                        case 3:
                            src[wy] = ((((HI_U16)(dest[w])) & 0xc0) >> 6) + ((HI_U16)(dest[w + 1]) << 2);
                            wy++;
                            w++;
                            pMemContent += 1;
                            break;
                    }

                    wuv++;
                    pMemContent += 1;

                }

                fwrite(src, pVBuf->u32Width , 1, pfd);
            }

            fflush(pfd);

            /* save V ----------------------------------------------------------------*/
            fprintf(stderr, "V......");
            fflush(stderr);

            for (h = 0; h < u32UvHeight; h++)
            {
                pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];
                wy = 0;
                wuv = 0;

                for (w = 0; w < u32YWidth - 1; w++)
                {
                    dest[w] = *pMemContent;
                    dest[w + 1] = *(pMemContent + 1);
                    k = wuv % 4;

                    switch (k)
                    {
                        case 0:
                            src[wy] = (((HI_U16)(dest[w]))) + (((dest[w + 1]) & 0x3) << 8);
                            wy++;
                            break;

                        case 1:
                            //src[wy] = ((((HI_U16)(dest[w]))&0xfc)>>2) + (((HI_U16)(dest[w+1])&0xf)<<6);
                            break;

                        case 2:
                            src[wy] = ((((HI_U16)(dest[w])) & 0xf0) >> 4) + (((HI_U16)(dest[w + 1]) & 0x3f) << 4);
                            wy++;
                            break;

                        case 3:
                            //src[wy] = ((((HI_U16)(dest[w]))&0xc0)>>6) + ((HI_U16)(dest[w+1])<<2);
                            w++;
                            pMemContent += 1;
                            break;
                    }

                    pMemContent += 1;
                    wuv ++;
                }

                fwrite(src, pVBuf->u32Width, 1, pfd);
            }
        }
        else
        {
            for (h = 0; h < u32UvHeight; h++)
            {
                pMem = pVBufVirt_C + h * pVBuf->u32Stride[1]; // pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

                if (0 == h % 2)
                {
                    wy = 0;
                }

                for (w = 0; w < (u32YWidth - 19); w += 20)
                {
                    src[wy] = (((pMem[w + 1]) & 0xfc) >> 2) + (((pMem[w + 2]) & 0xf) << 6) + (((pMem[w + 3]) & 0xc0) << 4) + (((pMem[w + 4]) & 0xf) << 12);
                    src[wy + 1] = (((pMem[w + 4]) & 0xf0) >> 4) + ((pMem[w + 6] & 0xfc) << 2) + (((pMem[w + 7]) & 0xf) << 10) + (((pMem[w + 8]) & 0xc0) << 8);
                    src[wy + 2] = ((pMem[w + 9]) & 0xff) + (((pMem[w + 11]) & 0xfc) << 6) + ((pMem[w + 12] & 0x3) << 14);
                    src[wy + 3] = (((pMem[w + 12]) & 0xc) >> 2) + (((pMem[w + 13]) & 0xc0) >> 4) + (((pMem[w + 14]) & 0xff) << 4) + (((pMem[w + 16]) & 0x3c) << 10);
                    src[wy + 4] = (((pMem[w + 16]) & 0xc0) >> 6) + (((pMem[w + 17]) & 0xf) << 2) + ((pMem[w + 18]) & 0xc0) + (((pMem[w + 19]) & 0xff) << 8);

                    pMem += 20;
                    wy += 5;
                }

                fwrite(src, (pVBuf->u32Stride[1] / 2), 1, pfd);
            }

            fflush(pfd);

            /* save V ----------------------------------------------------------------*/
            fprintf(stderr, "V......");
            fflush(stderr);

            for (h = 0; h < u32UvHeight; h++)
            {
                pMem = pVBufVirt_C + h * pVBuf->u32Stride[1]; // pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];
                wy = 0;

                for (w = 0; w < u32YWidth - 19; w += 20)
                {
                    src[wy] = (pMem[w] & 0xff) + (((pMem[w + 1]) & 0x3) << 8) + (((pMem[w + 2]) & 0xf0) << 6) + (((pMem[w + 3]) & 0x3) << 14);
                    src[wy + 1] = (((pMem[w + 3]) & 0x3c) >> 2) + (pMem[w + 5] & 0xff) + (((pMem[w + 6]) & 0x3) << 12) + (((pMem[w + 7]) & 0x30) << 10);
                    src[wy + 2] = (((pMem[w + 7]) & 0xc0) >> 6) + (((pMem[w + 8]) & 0x3f) << 2) + (pMem[w + 10] & 0xff);
                    src[wy + 3] = ((pMem[w + 11]) & 0x3) + (((pMem[w + 12]) & 0xf0) >> 2) + (((pMem[w + 13]) & 0x3f) << 6) + (((pMem[w + 15]) & 0xf) << 12);
                    src[wy + 4] = (((pMem[w + 15]) & 0xf0) >> 4) + (((pMem[w + 16]) & 0x3) << 4) + (((pMem[w + 17]) & 0xf0) << 2) + (((pMem[w + 18]) & 0x3f) << 10);

                    pMem += 20;
                    wy += 5;
                }

                fwrite(src, (pVBuf->u32Stride[1] / 2), 1, pfd);
            }
        }
    }

    fflush(pfd);

    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], u32Size);
    pUserPageAddr[0] = HI_NULL;
}

static HI_S32 VI_Restore(VI_PIPE Pipe)
{
    HI_S32 s32Ret= HI_FAILURE;

    if(VB_INVALID_POOLID != stFrame.u32PoolId)
    {
        s32Ret = HI_MPI_VI_ReleasePipeFrame(Pipe, &stFrame);
        if(HI_SUCCESS != s32Ret)
        {
             printf("Release Chn Frame error!!!\n");
        }
        stFrame.u32PoolId = VB_INVALID_POOLID;
    }
    if(-1 != hHandle)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        hHandle = -1;
    }
    if(HI_NULL != stMem.pVirAddr)
    {
        HI_MPI_SYS_Munmap((HI_VOID*)stMem.pVirAddr, u32BlkSize );
        stMem.u64PhyAddr = HI_NULL;
    }
    if(VB_INVALID_POOLID != stMem.hPool)
    {
        HI_MPI_VB_ReleaseBlock(stMem.hBlock);
        stMem.hPool = VB_INVALID_POOLID;
    }

    if (VB_INVALID_POOLID != hPool)
    {
        HI_MPI_VB_DestroyPool( hPool );
        hPool = VB_INVALID_POOLID;
    }

    if(HI_NULL != pUserPageAddr[0])
    {
        HI_MPI_SYS_Munmap(pUserPageAddr[0], u32Size);
        pUserPageAddr[0] = HI_NULL;
    }

    if(pfd)
    {
        fclose(pfd);
        pfd = HI_NULL;
    }

    s32Ret = HI_MPI_VI_SetPipeDumpAttr(Pipe, &astBackUpDumpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Set Pipe %d dump attr failed!\n",Pipe);
        return s32Ret;
    }

    return HI_SUCCESS;
}

void VI_Pipe_YuvDump_HandleSig(HI_S32 signo)
{
    if(u32SignalFlag)
    {
        exit(-1);
    }

    if (SIGINT == signo || SIGTERM == signo)
    {
        u32SignalFlag++;
        VI_Restore(ViPipe);
        u32SignalFlag--;
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}


HI_VOID SAMPLE_MISC_ViDump(VI_PIPE Pipe, HI_U32 u32FrameCnt, HI_U32 u32ByteAlign)
{
    HI_CHAR szYuvName[128];
    HI_CHAR szPixFrm[10];
    HI_CHAR szDynamicRange[10];
    HI_CHAR szVideoFrm[10];
    HI_U32 u32Cnt = u32FrameCnt;
    HI_U32 u32Depth = 2;
    HI_S32 s32MilliSec = -1;
    HI_S32 s32Times = 10;
    HI_BOOL bSendToVgs = HI_FALSE;
    VIDEO_FRAME_INFO_S stFrmInfo;
    VGS_TASK_ATTR_S stTask;
    HI_U32 u32LumaSize = 0;
    HI_U32 u32PicLStride = 0;
    HI_U32 u32PicCStride = 0;
    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;
    HI_S32 i = 0;
    HI_S32 s32Ret;
    HI_U32 u32BitWidth;
    VI_DUMP_ATTR_S    stDumpAttr;
    VB_POOL_CONFIG_S stVbPoolCfg;

    s32Ret = HI_MPI_VI_GetPipeDumpAttr(Pipe, &astBackUpDumpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get Pipe %d dump attr failed!\n", Pipe);
        return;
    }

    memcpy(&stDumpAttr, &astBackUpDumpAttr, sizeof(VI_DUMP_ATTR_S));
    stDumpAttr.bEnable  = HI_TRUE;
    stDumpAttr.u32Depth = u32Depth;

    s32Ret = HI_MPI_VI_SetPipeDumpAttr(Pipe, &stDumpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Set Pipe %d dump attr failed!\n",Pipe);
        return;
    }

    memset(&stFrame, 0, sizeof(stFrame));
    stFrame.u32PoolId = VB_INVALID_POOLID;
    while (HI_MPI_VI_GetPipeFrame(Pipe, &stFrame, s32MilliSec) != HI_SUCCESS)
    {
        s32Times--;
        if(0 >= s32Times)
        {
            printf("get frame error for 10 times,now exit !!!\n");
            VI_Restore(Pipe);
            return;
        }
        usleep(40000);
    }

    switch (stFrame.stVFrame.enPixelFormat)
    {
        case PIXEL_FORMAT_YVU_SEMIPLANAR_420:
            snprintf(szPixFrm, 10, "P420");
            break;
        case PIXEL_FORMAT_YVU_SEMIPLANAR_422:
            snprintf(szPixFrm, 10, "P422");
            break;
        case PIXEL_FORMAT_YUV_400:
            snprintf(szPixFrm, 10, "P400");
            break;
        default:
            printf("pixel format err, pipe_yuvdump is dump YUV, not RAW.\n");
            snprintf(szPixFrm, 10, "--");
            break;
    }

    switch (stFrame.stVFrame.enVideoFormat)
    {
        case VIDEO_FORMAT_LINEAR:
            snprintf(szVideoFrm, 10, "linear");
            break;
        case VIDEO_FORMAT_TILE_64x16:
            snprintf(szVideoFrm, 10, "tile_64X16");
            break;
        case VIDEO_FORMAT_TILE_16x8:
            snprintf(szVideoFrm, 10, "tile_16X8");
            break;
        default:
            snprintf(szVideoFrm, 10, "--");
            break;
    }

    switch (stFrame.stVFrame.enDynamicRange)
    {
        case DYNAMIC_RANGE_SDR8:
            snprintf(szDynamicRange, 10, "SDR8");
            break;

        case DYNAMIC_RANGE_SDR10:
            snprintf(szDynamicRange, 10, "SDR10");
            break;

        case DYNAMIC_RANGE_HDR10:
            snprintf(szDynamicRange, 10, "HDR10");
            break;

        case DYNAMIC_RANGE_HLG:
            snprintf(szDynamicRange, 10, "HLG");
            break;

        case DYNAMIC_RANGE_SLF:
            snprintf(szDynamicRange, 10, "SLF");
            break;

        default:
            snprintf(szDynamicRange, 10, "--");
            break;
    }


    /* make file name */
    snprintf(szYuvName, 128, "./vi_pipe%d_w%d_h%d_%s_%s_%s_%d_%d.yuv", Pipe,
            stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height, szPixFrm, szVideoFrm, szDynamicRange, u32Cnt, u32ByteAlign);
    printf("Dump YUV frame of vi pipe %d  to file: \"%s\"\n", Pipe, szYuvName);
    fflush(stdout);

    s32Ret = HI_MPI_VI_ReleasePipeFrame(Pipe, &stFrame);
    if(HI_SUCCESS != s32Ret)
    {
        printf("Release frame error ,now exit !!!\n");
        VI_Restore(Pipe);
        return;
    }
    stFrame.u32PoolId = VB_INVALID_POOLID;
     /* open file */
    pfd = fopen(szYuvName, "wb");

    if (HI_NULL == pfd)
    {
        printf("open file failed:%s!\n", strerror(errno));
        VI_Restore(Pipe);
        return;
    }

    /* get frame  */
    while (u32Cnt--)
    {
        if (HI_MPI_VI_GetPipeFrame(Pipe, &stFrame, s32MilliSec) != HI_SUCCESS)
        {
            printf("Get frame fail \n");
            usleep(1000);
            continue;
        }

        bSendToVgs = ((COMPRESS_MODE_NONE != stFrame.stVFrame.enCompressMode)
                                    || (VIDEO_FORMAT_LINEAR != stFrame.stVFrame.enVideoFormat));

        if (bSendToVgs)
        {

            u32Width    = stFrame.stVFrame.u32Width;
            u32Height   = stFrame.stVFrame.u32Height;

            u32BitWidth = (DYNAMIC_RANGE_SDR8 == stFrame.stVFrame.enDynamicRange) ? 8 : 10;

            u32PicLStride = ALIGN_UP((u32Width * u32BitWidth + 7) >> 3, 16);
            u32PicCStride = u32PicLStride;
            u32LumaSize = u32PicLStride * u32Height;

            if(PIXEL_FORMAT_YVU_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat)
            {
                u32BlkSize = u32PicLStride * u32Height * 3 >> 1;
            }
            else if(PIXEL_FORMAT_YVU_SEMIPLANAR_422 == stFrame.stVFrame.enPixelFormat)
            {
                u32BlkSize = u32PicLStride * u32Height * 2;
            }
            else if(PIXEL_FORMAT_YUV_400 == stFrame.stVFrame.enPixelFormat)
            {
                u32BlkSize = u32PicLStride * u32Height;
            }
            else
            {
                printf("Unsupported pixelformat %d\n",stFrame.stVFrame.enPixelFormat);
                VI_Restore(Pipe);
                return;
            }

            memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
            stVbPoolCfg.u64BlkSize  = u32BlkSize;
            stVbPoolCfg.u32BlkCnt   = 1;
            stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;
            hPool   = HI_MPI_VB_CreatePool(&stVbPoolCfg);
            if (hPool == VB_INVALID_POOLID)
            {
                printf("HI_MPI_VB_CreatePool failed! \n");
                VI_Restore(Pipe);
                return;
            }
            stMem.hPool = hPool;

            while ((stMem.hBlock = HI_MPI_VB_GetBlock(stMem.hPool, u32BlkSize, HI_NULL)) == VB_INVALID_HANDLE)
            {
                ;
            }
            stMem.u64PhyAddr = HI_MPI_VB_Handle2PhysAddr(stMem.hBlock);

            stMem.pVirAddr = (HI_U8*) HI_MPI_SYS_Mmap(stMem.u64PhyAddr, u32BlkSize );
            if (stMem.pVirAddr == HI_NULL)
            {
                printf("Mem dev may not open\n");
                VI_Restore(Pipe);
                return;
            }

            memset(&stFrmInfo.stVFrame, 0, sizeof(VIDEO_FRAME_S));
            stFrmInfo.stVFrame.u64PhyAddr[0] = stMem.u64PhyAddr;
            stFrmInfo.stVFrame.u64PhyAddr[1] = stFrmInfo.stVFrame.u64PhyAddr[0] + u32LumaSize;

            stFrmInfo.stVFrame.u64VirAddr[0] = (HI_U64)(HI_UL)stMem.pVirAddr;
            stFrmInfo.stVFrame.u64VirAddr[1] = stFrmInfo.stVFrame.u64VirAddr[0] + u32LumaSize;

            stFrmInfo.stVFrame.u32Width     = u32Width;
            stFrmInfo.stVFrame.u32Height    = u32Height;
            stFrmInfo.stVFrame.u32Stride[0] = u32PicLStride;
            stFrmInfo.stVFrame.u32Stride[1] = u32PicCStride;

            stFrmInfo.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
            stFrmInfo.stVFrame.enPixelFormat  = stFrame.stVFrame.enPixelFormat;
            stFrmInfo.stVFrame.enVideoFormat  = VIDEO_FORMAT_LINEAR;
            stFrmInfo.stVFrame.enDynamicRange =  stFrame.stVFrame.enDynamicRange;

            stFrmInfo.stVFrame.u64PTS     = (i * 40);
            stFrmInfo.stVFrame.u32TimeRef = (i * 2);

            stFrmInfo.u32PoolId = hPool;
            stFrmInfo.enModId = HI_ID_VGS;

            s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_BeginJob failed\n");
                hHandle = -1;
                VI_Restore(Pipe);
                return;
            }
            memcpy(&stTask.stImgIn, &stFrame, sizeof(VIDEO_FRAME_INFO_S));
            memcpy(&stTask.stImgOut , &stFrmInfo, sizeof(VIDEO_FRAME_INFO_S));
            s32Ret = HI_MPI_VGS_AddScaleTask(hHandle, &stTask, VGS_SCLCOEF_NORMAL);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_AddScaleTask failed\n");
                VI_Restore(Pipe);
                return;
            }

            s32Ret = HI_MPI_VGS_EndJob(hHandle);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_EndJob failed\n");
                VI_Restore(Pipe);
                return;
            }
            hHandle = -1;
            /* save VO frame to file */

            if(DYNAMIC_RANGE_SDR8 == stFrame.stVFrame.enDynamicRange)
            {
                sample_yuv_8bit_dump(&stFrmInfo.stVFrame, pfd);
            }
            else
            {
                sample_yuv_10bit_dump(&stFrmInfo.stVFrame, pfd, u32ByteAlign);
            }

            HI_MPI_VB_ReleaseBlock(stMem.hBlock);

            stMem.hPool =  VB_INVALID_POOLID;
            hHandle = -1;
            if(HI_NULL != stMem.pVirAddr)
            {
                HI_MPI_SYS_Munmap((HI_VOID*)stMem.pVirAddr, u32BlkSize );
                stMem.u64PhyAddr = HI_NULL;
            }
            if (hPool != VB_INVALID_POOLID)
            {
                HI_MPI_VB_DestroyPool( hPool );
                hPool = VB_INVALID_POOLID;
            }

        }
        else
        {
            if(DYNAMIC_RANGE_SDR8 == stFrame.stVFrame.enDynamicRange)
            {
                sample_yuv_8bit_dump(&stFrame.stVFrame, pfd);
            }
            else
            {
                sample_yuv_10bit_dump(&stFrame.stVFrame, pfd, u32ByteAlign);
            }
        }

        printf("Get ViPipe %d frame %d!!\n", Pipe, u32Cnt);
        /* release frame after using */
        s32Ret = HI_MPI_VI_ReleasePipeFrame(Pipe, &stFrame);
        if(HI_SUCCESS != s32Ret)
        {
            printf("Release frame error ,now exit !!!\n");
            VI_Restore(Pipe);
            return;
        }

        stFrame.u32PoolId = VB_INVALID_POOLID;
    }
    VI_Restore(Pipe);
    return;
}
static void usage(void)
{
    printf(
        "\n"
        "**********************************************************\n"
        "Usage: ./vi_chn_dump [ViPipe] [FrmCnt] [ByteAlign]\n"
        "1)ViPipe: \n"
        "   Vi pipe id\n"
        "2)FrmCnt: \n"
        "   the count of frame to be dump\n"
        "3)ByteAlign: \n"
        "   Whether convert to Byte align , default is 1\n"
        "*)Example:\n"
        "   e.g : ./vi_pipe_yuvdump 0 1 1\n"
        "   e.g : ./vi_pipe_yuvdump 1 2 0\n"
        "**********************************************************\n"
        "\n");
}

#ifdef __HuaweiLite__
HI_S32 vi_pipe_yuvdump(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    HI_U32 u32FrmCnt = 1;
    HI_U32 u32ByteAlign = 1;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\tTo see more usage, please enter: ./vi_chn_dump -h\n\n");
    if (argc > 1)
    {
        if (!strncmp(argv[1], "-h", 2))
        {
            usage();
            exit(HI_SUCCESS);
        }
    }

    if (argc < 4)
    {
        usage();
        exit(HI_SUCCESS);
    }

    ViPipe = atoi(argv[1]);
    if (!VALUE_BETWEEN(ViPipe, 0, VI_MAX_PIPE_NUM - 1))
    {
        printf("pipe id must be [0,%d]!!!!\n\n",VI_MAX_PIPE_NUM - 1);
        return -1;
    }

    u32SignalFlag = 0;
    pUserPageAddr[0] = HI_NULL;
    stFrame.u32PoolId = VB_INVALID_POOLID;
    u32OrigDepth = 0;
    hPool  = VB_INVALID_POOLID;
    hHandle = -1;
    u32BlkSize = 0;
    u32Size = 0;
    pfd = HI_NULL;

    #ifndef __HuaweiLite__
    signal(SIGINT, VI_Pipe_YuvDump_HandleSig);
    signal(SIGTERM, VI_Pipe_YuvDump_HandleSig);
    #endif

    u32FrmCnt = atoi(argv[2]);/* frame count*/
    u32ByteAlign = atoi(argv[3]);/* Byte align type*/

    SAMPLE_MISC_ViDump(ViPipe, u32FrmCnt, u32ByteAlign);

    return HI_SUCCESS;
}
