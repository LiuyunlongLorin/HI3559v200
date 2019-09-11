#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#include "hi_comm_vb.h"
#include "mpi_vb.h"
#include "hi_comm_vo.h"
#include "mpi_vo.h"
#include "mpi_vgs.h"
#include "hi_math.h"

#define MAX_FRM_WIDTH   8192

typedef struct hiDUMP_MEMBUF_S
{
    VB_BLK  hBlock;
    VB_POOL hPool;
    HI_U32  u32PoolId;

    HI_U64  u64PhyAddr;
    HI_U64   u64VirAddr;
    HI_S32  s32Mdev;
} DUMP_MEMBUF_S;

static VIDEO_FRAME_INFO_S g_stFrame;
static char* g_pVBufVirt_Y = NULL;
static char* g_pVBufVirt_C = NULL;
static HI_U32 g_Ysize, g_Csize;
static FILE* g_pfd = NULL;
static VB_POOL g_hPool  = VB_INVALID_POOLID;
static DUMP_MEMBUF_S g_stMem;

static HI_CHAR* pUserPageAddr[2] = {HI_NULL, HI_NULL};
static HI_U32 u32Size = 0;


static void usage(void)
{
    printf(
        "\n"
        "*************************************************\n"
#ifndef __HuaweiLite__
        "Usage: ./vo_screen_dump [VoLayer] [Frmcnt].\n"
#else
        "Usage: vo_screen_dump [VoLayer] [Frmcnt].\n"
#endif
        "1)VoLayer: \n"
        "   Which layer to be dumped\n"
        "   Default: 0\n"
        "2)FrmCnt: \n"
        "   The count of frame to be dumped\n"
        "   Default: 1\n"
        "*)Example:\n"
#ifndef __HuaweiLite__
        "   e.g : ./vo_screen_dump 0 1 (dump one YUV)\n"
#else
        "   e.g : vo_screen_dump 0 1 (dump one YUV)\n"
#endif
        "*************************************************\n"
        "\n");
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
#ifndef __HuaweiLite__
void VOU_TOOL_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        if (0 != g_stFrame.stVFrame.u64PhyAddr[0])
        {
            HI_MPI_VO_ReleaseScreenFrame(0, &g_stFrame);
            memset(&g_stFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
        }

        if (NULL != g_pVBufVirt_Y)
        {
            HI_MPI_SYS_Munmap(g_pVBufVirt_Y, g_Ysize);
            g_pVBufVirt_Y = NULL;
        }

        if (NULL != g_pVBufVirt_C)
        {
            HI_MPI_SYS_Munmap(g_pVBufVirt_C, g_Csize);
            g_pVBufVirt_C = NULL;
        }

        if (NULL != g_pfd)
        {
            fclose(g_pfd);
            g_pfd = NULL;
        }

        if (VB_INVALID_HANDLE != g_stMem.hBlock)
        {
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
            g_stMem.hBlock = VB_INVALID_HANDLE;
        }

        if (VB_INVALID_POOLID != g_hPool)
        {
            HI_MPI_VB_DestroyPool( g_hPool );
        }

        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}
#endif


/* sp420 to p420 ; sp422 to p422  */
static void sample_yuv_dump(VIDEO_FRAME_S* pVBuf)
{
    unsigned int w, h;
    char* pMemContent;
    static unsigned char TmpBuff[MAX_FRM_WIDTH];
    HI_U64 phy_addr;
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight;/*u32UvHeight, if planar */

    g_Ysize = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);
    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == enPixelFormat)
    {
        g_Csize = (pVBuf->u32Stride[1]) * (pVBuf->u32Height) / 2;
        u32UvHeight = pVBuf->u32Height / 2;
    }
    else if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == enPixelFormat)
    {
        g_Csize = (pVBuf->u32Stride[1]) * (pVBuf->u32Height);
        u32UvHeight = pVBuf->u32Height;
    }
    else
    {
        g_Csize = 0;
        u32UvHeight = 0;
    }

    phy_addr = pVBuf->u64PhyAddr[0];

    g_pVBufVirt_Y = (HI_CHAR*) HI_MPI_SYS_Mmap(phy_addr, g_Ysize);
    if (NULL == g_pVBufVirt_Y)
    {
        return;
    }

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......Y......");
    fflush(stderr);
    for (h = 0; h < pVBuf->u32Height; h++)
    {
        pMemContent = g_pVBufVirt_Y + h * pVBuf->u32Stride[0];
        fwrite(pMemContent, pVBuf->u32Width, 1, g_pfd);
    }
    fflush(g_pfd);

    if (PIXEL_FORMAT_YUV_400 != enPixelFormat)
    {
        g_pVBufVirt_C = (HI_CHAR*) HI_MPI_SYS_Mmap(pVBuf->u64PhyAddr[1], g_Csize);
        if (NULL == g_pVBufVirt_C)
        {
            HI_MPI_SYS_Munmap(g_pVBufVirt_Y, g_Ysize);
            g_pVBufVirt_Y = NULL;
            return;
        }

        /* save U ----------------------------------------------------------------*/
        fprintf(stderr, "U......");
        fflush(stderr);
        for (h = 0; h < u32UvHeight; h++)
        {
            pMemContent = g_pVBufVirt_C + h * pVBuf->u32Stride[1];

            pMemContent += 1;

            for (w = 0; w < pVBuf->u32Width / 2; w++)
            {
                TmpBuff[w] = *pMemContent;
                pMemContent += 2;
            }
            fwrite(TmpBuff, pVBuf->u32Width / 2, 1, g_pfd);
        }
        fflush(g_pfd);

        /* save V ----------------------------------------------------------------*/
        fprintf(stderr, "V......");
        fflush(stderr);
        for (h = 0; h < u32UvHeight; h++)
        {
            pMemContent = g_pVBufVirt_C + h * pVBuf->u32Stride[1];

            for (w = 0; w < pVBuf->u32Width / 2; w++)
            {
                TmpBuff[w] = *pMemContent;
                pMemContent += 2;
            }
            fwrite(TmpBuff, pVBuf->u32Width / 2, 1, g_pfd);
        }
        fflush(g_pfd);
        HI_MPI_SYS_Munmap(g_pVBufVirt_C, g_Csize);
        g_pVBufVirt_C = NULL;
    }
    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(g_pVBufVirt_Y, g_Ysize);
    g_pVBufVirt_Y = NULL;

}

/* sp420 to p420 ; sp422 to p422  for 10bit*/
static void sample_yuv_dump_10bit(VIDEO_FRAME_S* pVBuf)
{
    unsigned int w, h, k, wy, wuv;
    char* pVBufVirt_Y;
    char* pVBufVirt_C;
    char* pMemContent;
    //unsigned char TmpBuff[MAX_FRM_WIDTH];                //If this value is too small and the image is big, this memory may not be enough

    HI_U64 phy_addr;
    HI_U16  src[MAX_FRM_WIDTH];
    HI_U8  dest[MAX_FRM_WIDTH];

    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight = 0;/*When the storage format is a planar format, this variable is used to keep the height of the UV component */
    //HI_U32 u32UvWidth;
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

        fwrite(src, pVBuf->u32Width * 2, 1, g_pfd);
    }

    if (PIXEL_FORMAT_YUV_400 != enPixelFormat)
    {
        fflush(g_pfd);
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

            fwrite(src, pVBuf->u32Width , 1, g_pfd);
        }

        fflush(g_pfd);

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

            fwrite(src, pVBuf->u32Width, 1, g_pfd);
        }
    }

    fflush(g_pfd);

    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], u32Size);
    pUserPageAddr[0] = HI_NULL;


}


static HI_CHAR * sample_get_dyrg(DYNAMIC_RANGE_E enDyRg,HI_CHAR * pszYuvDyRg)
{
    switch(enDyRg)
    {
        case DYNAMIC_RANGE_SDR8:
            snprintf(pszYuvDyRg, 10, "SDR8");
            break;
        case DYNAMIC_RANGE_SDR10:
            snprintf(pszYuvDyRg, 10, "SDR10");
            break;
        case DYNAMIC_RANGE_HDR10:
            snprintf(pszYuvDyRg, 10, "HDR10");
            break;
        case DYNAMIC_RANGE_HLG:
            snprintf(pszYuvDyRg, 10, "HLG");
            break;
        case DYNAMIC_RANGE_SLF:
            snprintf(pszYuvDyRg, 10, "SLF");
            break;
        default:
            snprintf(pszYuvDyRg, 10, "SDR8");
            break;
    }
    return pszYuvDyRg;
}

/*
* Name : SAMPLE_MISC_VoDump
* Note : MISC,miscellaneous
*/
static HI_S32 SAMPLE_MISC_VoDump(VO_LAYER VoLayer, HI_S32 s32Cnt)
{
    HI_S32 i, s32Ret;
    //VIDEO_FRAME_INFO_S astFrame[256];
    HI_CHAR szYuvName[128];
    HI_CHAR szPixFrm[10];
    HI_U32  u32BlkSize = 0;
    HI_CHAR szYuvDyRg[10];

    VIDEO_FRAME_INFO_S stFrmInfo;
    VGS_HANDLE hHandle = -1;
    VGS_TASK_ATTR_S stTask;
    HI_U32 u32Width                 = 0;
    HI_U32 u32Height                = 0;
    HI_BOOL bSendToVgs              = HI_FALSE;
    HI_U32             u32OutStride;
    HI_U32             u32OutWidth,u32OutHeight;
    VB_POOL_CONFIG_S stVbPoolCfg;

    /* Get Frame to make file name*/
    s32Ret = HI_MPI_VO_GetScreenFrame(VoLayer, &g_stFrame, 0);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO(%d)_GetScreenFrame errno %#x\n", VoLayer, s32Ret);
        return -1;
    }

    /* make file name */
    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == g_stFrame.stVFrame.enPixelFormat)
    {
        snprintf(szPixFrm, 10, "p420");
    }
    else if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == g_stFrame.stVFrame.enPixelFormat)
    {
        snprintf(szPixFrm, 10, "p422");
    }
    else if (PIXEL_FORMAT_YUV_400 == g_stFrame.stVFrame.enPixelFormat)
    {
        snprintf(szPixFrm, 10, "p400");
    }
    else
    {
        return -1;
    }

    snprintf(szYuvName, 128, "./vo_layer%d_%ux%u_%s_%s_%d.yuv", VoLayer,
            g_stFrame.stVFrame.u32Width, g_stFrame.stVFrame.u32Height, szPixFrm, sample_get_dyrg(g_stFrame.stVFrame.enDynamicRange,szYuvDyRg),s32Cnt);
    printf("Dump YUV frame of vo(%d) to file: \"%s\"\n", VoLayer, szYuvName);

    bSendToVgs = (g_stFrame.stVFrame.enCompressMode > 0) || (g_stFrame.stVFrame.enVideoFormat > 0);
    HI_MPI_VO_ReleaseScreenFrame(VoLayer, &g_stFrame);

    /* open file */
    g_pfd = fopen(szYuvName, "wb");

    if (NULL == g_pfd)
    {
        printf("open file failed:%s!\n", strerror(errno));
        return -1;
    }

    u32Width      = g_stFrame.stVFrame.u32Width;
    u32Height     = g_stFrame.stVFrame.u32Height;
    u32OutWidth   = u32Width;
    u32OutHeight  = u32Height;
    u32OutStride  = ALIGN_UP((u32OutHeight * 10 + 7) >> 3,16/*COMMON_ALIGN*/);

    if (bSendToVgs)
    {
        if (PIXEL_FORMAT_YUV_400 == g_stFrame.stVFrame.enPixelFormat)
        {
            u32BlkSize = g_stFrame.stVFrame.u32Stride[0] * g_stFrame.stVFrame.u32Height;
        }
        else
        {
            u32BlkSize = (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == g_stFrame.stVFrame.enPixelFormat)
                         ? (g_stFrame.stVFrame.u32Stride[0] * g_stFrame.stVFrame.u32Height * 3 >> 1) : (g_stFrame.stVFrame.u32Stride[0] * g_stFrame.stVFrame.u32Height * 2);
        }
        if (DYNAMIC_RANGE_SDR8 != g_stFrame.stVFrame.enDynamicRange)
        {
            u32OutStride        = ALIGN_UP((u32OutWidth * 10 + 7) >> 3,16/*COMMON_ALIGN*/);
        }else
        {
            u32OutStride        = ALIGN_UP(u32OutWidth,16/*COMMON_ALIGN*/);
        }
        u32BlkSize          = u32OutStride * u32OutHeight * 3 / 2;

        memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
        stVbPoolCfg.u64BlkSize  = u32BlkSize;
        stVbPoolCfg.u32BlkCnt   = 1;
        stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;
        g_hPool   = HI_MPI_VB_CreatePool(&stVbPoolCfg);
        if (g_hPool == VB_INVALID_POOLID)
        {
            printf("HI_MPI_VB_CreatePool BlkSize = %d failed! \n",u32BlkSize);
            goto END1;
        }

        g_stMem.hPool = g_hPool;
    }

    /* get VO frame  */
    for (i = 0; i < s32Cnt; i++)
    {
        s32Ret = HI_MPI_VO_GetScreenFrame(VoLayer, &g_stFrame, 0);
        if (HI_SUCCESS != s32Ret)
        {
            printf("get vo(%d) frame err\n", VoLayer);
            printf("only get %d frame\n", i);
            break;
        }

        if (bSendToVgs)
        {
            while ((g_stMem.hBlock = HI_MPI_VB_GetBlock(g_stMem.hPool, u32BlkSize, NULL)) == VB_INVALID_HANDLE)
            {
                ;
            }

            g_stMem.u64PhyAddr = HI_MPI_VB_Handle2PhysAddr(g_stMem.hBlock);


            g_stMem.u64VirAddr = (HI_UL) HI_MPI_SYS_Mmap( g_stMem.u64PhyAddr, u32BlkSize );
            if (g_stMem.u64VirAddr == 0)
            {
                printf("Mem dev may not open\n");
                HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
                goto END2;
            }

            memset(&stFrmInfo.stVFrame, 0, sizeof(VIDEO_FRAME_S));
            stFrmInfo.stVFrame.u32Width       = u32Width;
            stFrmInfo.stVFrame.u32Height      = u32Height;
            stFrmInfo.stVFrame.enDynamicRange = g_stFrame.stVFrame.enDynamicRange;
            stFrmInfo.stVFrame.enField        = VIDEO_FIELD_FRAME;
            stFrmInfo.stVFrame.u64PhyAddr[0]  = g_stMem.u64PhyAddr;
            stFrmInfo.stVFrame.u64PhyAddr[1]  = stFrmInfo.stVFrame.u64PhyAddr[0] + u32OutStride * u32OutHeight;
            stFrmInfo.stVFrame.u64PhyAddr[2]  = stFrmInfo.stVFrame.u64PhyAddr[1] + u32OutStride * u32OutHeight;

            stFrmInfo.stVFrame.u64VirAddr[0]  = g_stMem.u64VirAddr;
            stFrmInfo.stVFrame.u64VirAddr[1]  = (HI_UL)stFrmInfo.stVFrame.u64VirAddr[0] + u32OutStride * u32OutHeight;
            stFrmInfo.stVFrame.u64VirAddr[2]  = (HI_UL)stFrmInfo.stVFrame.u64VirAddr[1] + u32OutStride * u32OutHeight;

            stFrmInfo.stVFrame.u32Stride[0]   = u32OutStride;
            stFrmInfo.stVFrame.u32Stride[1]   = u32OutStride;
            stFrmInfo.stVFrame.u32Stride[2]   = u32OutStride;

            stFrmInfo.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
            stFrmInfo.stVFrame.enPixelFormat  = g_stFrame.stVFrame.enPixelFormat;
            stFrmInfo.stVFrame.enVideoFormat  = VIDEO_FORMAT_LINEAR;

            stFrmInfo.stVFrame.u64PTS         = (i * 40);
            stFrmInfo.stVFrame.u32TimeRef     = (i * 2);

            stFrmInfo.enModId   = HI_ID_VGS;
            stFrmInfo.u32PoolId = g_hPool;

            s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_BeginJob failed\n");
                HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
                g_stMem.hBlock = VB_INVALID_HANDLE;
                HI_MPI_VO_ReleaseScreenFrame(VoLayer, &g_stFrame);
                goto END2;
            }

            memcpy(&stTask.stImgIn, &g_stFrame, sizeof(VIDEO_FRAME_INFO_S));
            memcpy(&stTask.stImgOut , &stFrmInfo, sizeof(VIDEO_FRAME_INFO_S));
            s32Ret = HI_MPI_VGS_AddScaleTask(hHandle, &stTask, VGS_SCLCOEF_NORMAL);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_AddScaleTask failed\n");
                HI_MPI_VGS_CancelJob(hHandle);
                HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
                g_stMem.hBlock = VB_INVALID_HANDLE;
                HI_MPI_VO_ReleaseScreenFrame(VoLayer, &g_stFrame);
                goto END2;
            }

            s32Ret = HI_MPI_VGS_EndJob(hHandle);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_EndJob failed\n");
                HI_MPI_VGS_CancelJob(hHandle);
                HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
                g_stMem.hBlock = VB_INVALID_HANDLE;
                HI_MPI_VO_ReleaseScreenFrame(VoLayer, &g_stFrame);
                goto END2;
            }

            /* save VO frame to file */
            //sample_yuv_dump(&stFrmInfo.stVFrame);
            if (DYNAMIC_RANGE_SDR8 == stFrmInfo.stVFrame.enDynamicRange)
            {
                sample_yuv_dump(&stFrmInfo.stVFrame);
            }
            else
            {
                sample_yuv_dump_10bit(&stFrmInfo.stVFrame);
            }

            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
            g_stMem.hBlock = VB_INVALID_HANDLE;
        }
        else
        {
            /* save VO frame to file */
            if (DYNAMIC_RANGE_SDR8 == g_stFrame.stVFrame.enDynamicRange)
            {
                sample_yuv_dump(&g_stFrame.stVFrame);
            }
            else
            {
                sample_yuv_dump_10bit(&g_stFrame.stVFrame);
            }
        }

        /* release frame after using */
        s32Ret = HI_MPI_VO_ReleaseScreenFrame(VoLayer, &g_stFrame);
        memset(&g_stFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
        if (HI_SUCCESS != s32Ret)
        {
            printf("Release vo(%d) frame err\n", VoLayer);
            printf("only get %d frame\n", i);
            break;
        }
    }


END2:
    if (VB_INVALID_POOLID != g_hPool)
    {
        HI_MPI_VB_DestroyPool(g_hPool);
        g_hPool = VB_INVALID_POOLID;
    }
    memset(&g_stFrame, 0, sizeof(VIDEO_FRAME_INFO_S));

END1:

    fclose(g_pfd);

    return 0;
}
/*
* Name : vo_screen_dump
* Desc : Dump screen frame.
*/
#ifdef __HuaweiLite__
HI_S32 vo_screen_dump(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    VO_LAYER VoLayer = 0;
    HI_S32 s32FrmCnt = 1;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    #ifndef __HuaweiLite__
    printf("\tTo see more usage, please enter: ./vo_screen_dump -h\n\n");
    #else
    printf("\tTo see more usage, please enter: vo_screen_dump -h\n\n");
    #endif

    if (argc > 3)
    {
        printf("Too many parameters!\n");
        return HI_FAILURE;
    }

    /* VO video layer ID*/
    if (argc > 1)
    {
        if (!strncmp(argv[1], "-h", 2))
        {
            usage();
            exit(HI_SUCCESS);
        }
        VoLayer = atoi(argv[1]);
    }

    /* need Frmcnt*/
    if (argc > 2)
    {
        s32FrmCnt = atoi(argv[2]);
        if (s32FrmCnt <= 0)
        {
            printf("The Frmcnt(%d) is wrong!\n", s32FrmCnt);
            return HI_FAILURE;
        }
    }

    #ifndef __HuaweiLite__
    signal(SIGINT, VOU_TOOL_HandleSig);
    signal(SIGTERM, VOU_TOOL_HandleSig);
    #endif

    SAMPLE_MISC_VoDump(VoLayer, s32FrmCnt);

    return HI_SUCCESS;
}

