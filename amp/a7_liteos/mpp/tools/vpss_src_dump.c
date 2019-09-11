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
#include "hi_comm_vpss.h"
#include "mpi_vpss.h"
#include "mpi_vgs.h"

#define MAX_FRM_WIDTH   20480

#define VALUE_BETWEEN(x,min,max) (((x)>=(min)) && ((x) <= (max)))


typedef struct hiDUMP_MEMBUF_S
{
    VB_BLK  hBlock;
    VB_POOL hPool;
    HI_U32  u32PoolId;

    HI_U64  u64PhyAddr;
    HI_U8*   pVirAddr;
    HI_S32  s32Mdev;
} DUMP_MEMBUF_S;



VIDEO_FRAME_INFO_S stFrame;
DUMP_MEMBUF_S stMem = {0};
static VPSS_GRP VpssGrp = 0;
VPSS_GRP_PIPE  VpssPipe = 0;
VB_POOL hPool  = VB_INVALID_POOLID;
VGS_HANDLE hHandle = -1;
FILE* pfd = HI_NULL;
HI_U32  u32BlkSize = 0;
static HI_CHAR* pUserPageAddr[2] = {HI_NULL, HI_NULL};
static HI_U32 size = 0;
static HI_U32 u32SignalFlag = 0;
static HI_U32 u32Size = 0;


/*When saving a file,sp420 will be denoted by p420 and sp422 will be denoted by p422 in the name of the file */
static void sample_yuv_8bit_dump(VIDEO_FRAME_S* pVBuf, FILE* pfd)
{
    unsigned int w, h;
    char* pVBufVirt_Y;
    char* pVBufVirt_C;
    char* pMemContent;
    unsigned char TmpBuff[MAX_FRM_WIDTH]; //If this value is too small and the image is big, this memory may not be enough
    HI_U64 phy_addr;
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight = 0;/*When the storage format is a planar format, this variable is used to keep the height of the UV component */

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

    for (h = 0; h < pVBuf->u32Height; h++)
    {
        pMemContent = pVBufVirt_Y + h * pVBuf->u32Stride[0];
        fwrite(pMemContent, pVBuf->u32Width, 1, pfd);
    }

    if (PIXEL_FORMAT_YUV_400 != enPixelFormat)
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


static void sample_yuv_10bit_dump(VIDEO_FRAME_S* pVBuf, FILE* pfd)
{
    unsigned int w, h, k, wy, wuv;
    char* pVBufVirt_Y;
    char* pVBufVirt_C;
    char* pMemContent;
    HI_U64 phy_addr;
    HI_U16  src[MAX_FRM_WIDTH];
    HI_U8  dest[MAX_FRM_WIDTH];
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight = 0;/*When the storage format is a planar format, this variable is used to keep the height of the UV component */
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

    for (h = 0; h < pVBuf->u32Height; h++)
    {
        pMemContent = pVBufVirt_Y + h * pVBuf->u32Stride[0];
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

    if (PIXEL_FORMAT_YUV_400 != enPixelFormat)
    {
        fflush(pfd);
        /* save U ----------------------------------------------------------------*/
        fprintf(stderr, "U......");
        fflush(stderr);

        for (h = 0; h < u32UvHeight; h++)
        {
            pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

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

    fflush(pfd);

    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], u32Size);
    pUserPageAddr[0] = HI_NULL;
}



static HI_S32 VPSS_Restore(VPSS_GRP VpssGrp, VPSS_GRP_PIPE VpssPipe)
{
    if (VB_INVALID_POOLID != stFrame.u32PoolId)
    {
        HI_MPI_VPSS_ReleaseGrpFrame(VpssGrp, VpssPipe, &stFrame);
        stFrame.u32PoolId = VB_INVALID_POOLID;
    }

    if (-1 != hHandle)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        hHandle = -1;
    }

    if (HI_NULL != pUserPageAddr[0])
    {
        HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
        pUserPageAddr[0] = HI_NULL;
    }

    if (HI_NULL != stMem.pVirAddr)
    {
        HI_MPI_SYS_Munmap((HI_VOID*)stMem.pVirAddr, u32BlkSize );
        stMem.u64PhyAddr = HI_NULL;
    }

    if (VB_INVALID_POOLID != stMem.hPool)
    {
        HI_MPI_VB_ReleaseBlock(stMem.hBlock);
        stMem.hPool = VB_INVALID_POOLID;
    }

    if (VB_INVALID_POOLID != hPool)
    {
        HI_MPI_VB_DestroyPool( hPool );
        hPool = VB_INVALID_POOLID;
    }

    if (pfd)
    {
        fclose(pfd);
        pfd = HI_NULL;
    }

    return HI_SUCCESS;
}

void VPSS_Src_Dump_HandleSig(HI_S32 signo)
{
    if (u32SignalFlag)
    {
        exit(-1);
    }

    if (SIGINT == signo || SIGTERM == signo)
    {
        u32SignalFlag++;
        VPSS_Restore(VpssGrp, VpssPipe);
        u32SignalFlag--;
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }

    exit(-1);
}


HI_S32 SAMPLE_MISC_VpssDumpSrcImage(VPSS_GRP Grp, VPSS_GRP_PIPE Pipe)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;
    HI_CHAR szYuvName[128];
    HI_CHAR szPixFrm[10];
    HI_U32  u32BlkSize = 0;
    HI_S32 s32Times = 10;

    VIDEO_FRAME_INFO_S stFrmInfo;
    VGS_TASK_ATTR_S stTask;
    HI_U32 u32LumaSize              = 0;
    HI_U32 u32PicLStride            = 0;
    HI_U32 u32PicCStride            = 0;
    HI_U32 u32Width                 = 0;
    HI_U32 u32Height                = 0;
    HI_BOOL bSendToVgs              = HI_FALSE;
    VPSS_GRP VpssGrp = Grp;
    VPSS_GRP_PIPE VpssPipe = Pipe;
    HI_U32 u32BitWidth;
    VB_POOL_CONFIG_S stVbPoolCfg;

    /* get frame  */
    while ((HI_MPI_VPSS_GetGrpFrame(VpssGrp, VpssPipe, &stFrame) != HI_SUCCESS))
    {
        s32Times--;

        if (0 >= s32Times)
        {
            printf("get frame error for 10 times,now exit !!!\n");
            return -1;
        }

        sleep(2);
    }

    if (VIDEO_FORMAT_LINEAR != stFrame.stVFrame.enVideoFormat)
    {
        printf("only support linear frame dump!\n");
        HI_MPI_VPSS_ReleaseGrpFrame(VpssGrp, VpssPipe, &stFrame);
        stFrame.u32PoolId = VB_INVALID_POOLID;
        return -1;
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
            snprintf(szPixFrm, 10, "--");
            break;
    }

    /* make file name */
    snprintf(szYuvName, 128, "./vpss%d_pipe%d_%dx%d_%s.yuv", VpssGrp, VpssPipe,
             stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height, szPixFrm);

    printf("Dump YUV frame of vpss%d pipe%d to file: \"%s\"\n", VpssGrp, VpssPipe, szYuvName);

    bSendToVgs = ((COMPRESS_MODE_NONE != stFrame.stVFrame.enCompressMode)
                                    || (VIDEO_FORMAT_LINEAR != stFrame.stVFrame.enVideoFormat));

    /* open file */
    pfd = fopen(szYuvName, "wb");

    if (HI_NULL == pfd)
    {
        printf("open file failed:%s!\n", strerror(errno));
        HI_MPI_VPSS_ReleaseGrpFrame(VpssGrp, VpssPipe, &stFrame);
        stFrame.u32PoolId = VB_INVALID_POOLID;
        return -1;
    }

    if (bSendToVgs)
    {
        u32Width    = stFrame.stVFrame.u32Width;
        u32Height   = stFrame.stVFrame.u32Height;

        u32BitWidth = (DYNAMIC_RANGE_SDR8 == stFrame.stVFrame.enDynamicRange) ? 8 : 10;

        u32PicLStride = ALIGN_UP((u32Width * u32BitWidth + 7) >> 3, 16);
        u32PicCStride = u32PicLStride;
        u32LumaSize = u32PicLStride * u32Height;

        if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat)
        {
            u32BlkSize = u32PicLStride * u32Height * 3 >> 1;
        }
        else if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == stFrame.stVFrame.enPixelFormat)
        {
            u32BlkSize = u32PicLStride * u32Height * 2;
        }
        else if (PIXEL_FORMAT_YUV_400 == stFrame.stVFrame.enPixelFormat)
        {
            u32BlkSize = u32PicLStride * u32Height;
        }
        else
        {
            printf("Unsupported pixelformat %d\n", stFrame.stVFrame.enPixelFormat);
            VPSS_Restore(Grp, Pipe);
            return - 1;
        }

        memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
        stVbPoolCfg.u64BlkSize  = u32BlkSize;
        stVbPoolCfg.u32BlkCnt   = 1;
        stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;
        hPool   = HI_MPI_VB_CreatePool(&stVbPoolCfg);

        if (hPool == VB_INVALID_POOLID)
        {
            printf("HI_MPI_VB_CreatePool failed! \n");
            VPSS_Restore(Grp, Pipe);
            return - 1;
        }

        stMem.hPool = hPool;

        while ((stMem.hBlock = HI_MPI_VB_GetBlock(stMem.hPool, u32BlkSize, HI_NULL)) == VB_INVALID_HANDLE)
        {
            ;
        }

        stMem.u64PhyAddr = HI_MPI_VB_Handle2PhysAddr(stMem.hBlock);

        stMem.pVirAddr = (HI_U8*) HI_MPI_SYS_Mmap( stMem.u64PhyAddr, u32BlkSize );

        if (stMem.pVirAddr == HI_NULL)
        {
            printf("Mem dev may not open\n");
            VPSS_Restore(Grp, Pipe);
            return - 1;
        }

        memset(&stFrmInfo.stVFrame, 0, sizeof(VIDEO_FRAME_S));
        stFrmInfo.stVFrame.u64PhyAddr[0] = stMem.u64PhyAddr;
        stFrmInfo.stVFrame.u64PhyAddr[1] = stFrmInfo.stVFrame.u64PhyAddr[0] + u32LumaSize;

        stFrmInfo.stVFrame.u64VirAddr[0] = (HI_U64)(HI_UL)stMem.pVirAddr;
        stFrmInfo.stVFrame.u64VirAddr[1] = (stFrmInfo.stVFrame.u64VirAddr[0] + u32LumaSize);

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
            VPSS_Restore(Grp, Pipe);
            return -1;
        }

        memcpy(&stTask.stImgIn, &stFrame, sizeof(VIDEO_FRAME_INFO_S));
        memcpy(&stTask.stImgOut , &stFrmInfo, sizeof(VIDEO_FRAME_INFO_S));
        s32Ret = HI_MPI_VGS_AddScaleTask(hHandle, &stTask, VGS_SCLCOEF_NORMAL);

        if (s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VGS_AddScaleTask failed\n");
            VPSS_Restore(Grp, Pipe);
            return -1;
        }

        s32Ret = HI_MPI_VGS_EndJob(hHandle);

        if (s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VGS_EndJob failed\n");
            VPSS_Restore(Grp, Pipe);
            return -1;
        }

        hHandle = -1;

        if (DYNAMIC_RANGE_SDR8 == stFrame.stVFrame.enDynamicRange)
        {
            sample_yuv_8bit_dump(&stFrmInfo.stVFrame, pfd);
        }
        else
        {
            sample_yuv_10bit_dump(&stFrmInfo.stVFrame, pfd);
        }

        HI_MPI_VB_ReleaseBlock(stMem.hBlock);

        stMem.hPool =  VB_INVALID_POOLID;
        hHandle = -1;

        if (HI_NULL != stMem.pVirAddr)
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
        if (DYNAMIC_RANGE_SDR8 == stFrame.stVFrame.enDynamicRange)
        {
            sample_yuv_8bit_dump(&stFrame.stVFrame, pfd);
        }
        else
        {
            sample_yuv_10bit_dump(&stFrame.stVFrame, pfd);
        }
    }


    VPSS_Restore(Grp, Pipe);

    return 0;
}

static void usage(void)
{
    printf(
        "\n"
        "*************************************************\n"
        "Usage: ./vpss_src_dump [Grp] [Pipe]\n"
        "1)VpssGrp: \n"
        "   Vpss group id\n"
        "2)Pipe:\n"
        "   Vpss Grp pipe\n"
        "*)Example:\n"
        "e.g : ./vpss_src_dump 0 0 \n"
        "*************************************************\n"
        "\n");
}

#ifdef __HuaweiLite__
HI_S32 vpss_src_dump(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    VpssGrp = 0;
    VpssPipe = 0;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\tTo see more usage, please enter: ./vpss_src_dump -h\n\n");

    if (argc > 1)
    {
        if (!strncmp(argv[1], "-h", 2))
        {
            usage();
            exit(HI_SUCCESS);
        }

        VpssGrp = atoi(argv[1]);
    }

    if (argc > 2)
    {
        VpssPipe = atoi(argv[2]);
    }

    if (!VALUE_BETWEEN(VpssGrp, 0, VPSS_MAX_GRP_NUM - 1))
    {
        printf("grp id must be [0,%d]!!!!\n\n", VPSS_MAX_GRP_NUM - 1);
        return -1;
    }

    if (!VALUE_BETWEEN(VpssPipe, 0, VPSS_MAX_GRP_PIPE_NUM - 1))
    {
        printf("VpssPipe must be [0,%d]!!!!\n\n", VPSS_MAX_GRP_PIPE_NUM - 1);
        return -1;
    }

    stFrame.u32PoolId = VB_INVALID_POOLID;
    hHandle = -1;
    pUserPageAddr[0] = HI_NULL;
    stMem.u64PhyAddr = HI_NULL;
    stMem.hPool = VB_INVALID_POOLID;
    hPool = VB_INVALID_POOLID;
    pfd = HI_NULL;

#ifndef __HuaweiLite__
    signal(SIGINT, VPSS_Src_Dump_HandleSig);
    signal(SIGTERM, VPSS_Src_Dump_HandleSig);
#endif

    SAMPLE_MISC_VpssDumpSrcImage(VpssGrp, VpssPipe);

    return HI_SUCCESS;
}

