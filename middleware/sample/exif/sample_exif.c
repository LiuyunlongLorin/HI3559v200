#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <errno.h>
#include "hi_exif.h"
#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif

#define SAMPLE_FILENAME_LEN_MAX   (256)
#define SMAPLE_ROOT_DIR "/tmp/"

#ifdef __HuaweiLite__
#define SAMPLE_EXIF_EDITOR_OUTPUT_FILE           SMAPLE_ROOT_DIR"buf_pic.jpeg"
#define SAMPLE_EXIF_DEMUXER_OUTPUT_FILE_PREFIX   SMAPLE_ROOT_DIR"pic_"
#else
#define SAMPLE_EXIF_EDITOR_OUTPUT_FILE           "buf_pic.jpeg"
#define SAMPLE_EXIF_DEMUXER_OUTPUT_FILE_PREFIX   "./pic_"
#endif

static HI_S32 SAMPLE_mount_tmp_dir()
{
#ifdef __HuaweiLite__
    if (HI_SUCCESS != mount("/dev/mmcblk0p0", "/tmp", "vfat", 0, 0))
    {
        printf("mount tmp failed, errno:%d \n", errno);
        return HI_FAILURE;
    }
#endif
    return HI_SUCCESS;
}

static HI_VOID SAMPLE_umount_tmp_dir()
{
#ifdef __HuaweiLite__
    HI_S32 s32Ret = umount("/tmp");
    if (HI_SUCCESS != s32Ret)
    {
        printf("umount tmp failed, errno:%d \n", errno);
    }
#endif
    return;
}

int EXIF_EDITOR(char* fileName)
{
    char* inFilename = fileName;
    FILE* fp_pic = fopen(inFilename, "r");

    if (NULL == fp_pic)
    {
        printf("open the file:%s failed \n", inFilename);
        return  -1;
    }

    fseek(fp_pic, 0, SEEK_END);
    HI_U32 u32TotalDataLen = ftell(fp_pic);
    printf("the filename:%s, the len:%d\n", inFilename, u32TotalDataLen);

    HI_EXIF_BUFFER_SOURCE_S stBufSource;
    memset(&stBufSource, 0x00, sizeof(stBufSource));

    int i = 0;

    fseek(fp_pic, 0, SEEK_SET);

    /*seperate file into multi segment buffer to simulate sdk(mpp) image buffer input */
    for(; i< HI_EXIF_DATA_SEG_MAX_CNT; i++)
    {
        if(i == HI_EXIF_DATA_SEG_MAX_CNT -1)
        {
            stBufSource.u32BufSize[i] =  (u32TotalDataLen/HI_EXIF_DATA_SEG_MAX_CNT) + (u32TotalDataLen%HI_EXIF_DATA_SEG_MAX_CNT);
        }
        else
        {
            stBufSource.u32BufSize[i] = (u32TotalDataLen/HI_EXIF_DATA_SEG_MAX_CNT);
        }
        stBufSource.pImgBuf[i] = malloc(stBufSource.u32BufSize[i]);
        if(!stBufSource.pImgBuf[i])
        {
            printf("malloc failed\n");
            break;
        }
        memset(stBufSource.pImgBuf[i], 0, stBufSource.u32BufSize[i]);
        fread(stBufSource.pImgBuf[i], 1, stBufSource.u32BufSize[i], fp_pic);
        printf("buf idx: %d len: %d\n", i, stBufSource.u32BufSize[i]);
    }
    fclose(fp_pic);


    HI_S32 s32Ret =  HI_SUCCESS;

    HI_MW_PTR hExifHandle = NULL;
    HI_EXIF_ATTR_S stExifAttr;
    memset(&stExifAttr, 0, sizeof(HI_EXIF_ATTR_S));
    stExifAttr.enExifConfigType = HI_EXIF_CONFIG_EDITOR;
    stExifAttr.stEditorConfig.pszFileName = SAMPLE_EXIF_EDITOR_OUTPUT_FILE;
    stExifAttr.stEditorConfig.bIsOverWrite = HI_TRUE;
    stExifAttr.stEditorConfig.u32FixedFileSize = 0;
    stExifAttr.stEditorConfig.u32MaxExifHeaderSize = 0;
    s32Ret = HI_EXIF_Open(&hExifHandle, &stExifAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_OpenFile exec failed,ret:%x \n", s32Ret);
        goto FREE_BUF;
    }

    s32Ret = HI_EXIF_SetBufferSource(hExifHandle, &stBufSource);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_SetBufferSource exec failed,ret:%x \n", s32Ret);
        goto EXIF_CLOSE;
    }

    HI_EXIF_TAG_ENTRY_S stExifTagEntry;
    stExifTagEntry.u16TagNum = 0x02BC;
    stExifTagEntry.enDataType = HI_EXIF_DATA_TYPE_BYTE;
    char* pValue = "<?xpacket begin=\"w\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n<x:xmpmeta xmlns:x='adobe:ns:meta/' x:xmptk='Image::ExifTool 10.29'>\n<rdf:RDF xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>\n <rdf:Description rdf:about=''  xmlns:crs='http://ns.adobe.com/camera-raw-settings/1.0/'>\n<crs:AlreadyApplied>False</crs:AlreadyApplied>\n<crs:HasCrop>False</crs:HasCrop>\n<crs:HasSettings>False</crs:HasSettings>\n<crs:Version>7.0</crs:Version>\n</rdf:Description>\n<rdf:Description rdf:about=''  xmlns:dc='http://purl.org/dc/elements/1.1/'>\n<dc:format>image/jpeg</dc:format>\n</rdf:Description>\n<rdf:Description rdf:about=''  xmlns:tiff='http://ns.adobe.com/tiff/1.0/'>\n<tiff:Make>HISI</tiff:Make>\n<tiff:Model>FC300S</tiff:Model>\n</rdf:Description>\n<rdf:Description rdf:about=''  xmlns:xmp='http://ns.adobe.com/xap/1.0/'>\n<xmp:CreateDate>2017-08-23</xmp:CreateDate>\n<xmp:ModifyDate>2017-08-23</xmp:ModifyDate>\n</rdf:Description>\n</rdf:RDF>\n</x:xmpmeta>\n<?xpacket end='w'?>";
    stExifTagEntry.u32Cnt =  strlen(pValue) + 1;
    stExifTagEntry.pValue = pValue;

    HI_EXIF_TAG_LOCATION_E enTagLocation = HI_EXIF_APP1_IFD0;

    s32Ret = HI_EXIF_SetTag(hExifHandle, &stExifTagEntry, enTagLocation);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_SetTag exec failed,ret:%x \n", s32Ret);
        goto EXIF_CLOSE;
    }


    HI_EXIF_TAG_ENTRY_S stExifModelTagEntry;
    stExifModelTagEntry.u16TagNum = 0xC614;
    stExifModelTagEntry.enDataType = HI_EXIF_DATA_TYPE_ASCII;
    char* pModelValue = "Hisilicon ActionCam Demo";
    stExifModelTagEntry.u32Cnt =  strlen(pModelValue) + 1;
    stExifModelTagEntry.pValue = pModelValue;

    enTagLocation = HI_EXIF_APP1_IFD0;

    s32Ret = HI_EXIF_SetTag(hExifHandle, &stExifModelTagEntry, enTagLocation);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_SetTag for unique camera model exec failed,ret:%x \n", s32Ret);
        goto EXIF_CLOSE;
    }


    HI_EXIF_TAG_ENTRY_S stExifMakerNodeTagEntry;
    stExifMakerNodeTagEntry.u16TagNum = 0x927C;
    stExifMakerNodeTagEntry.enDataType = HI_EXIF_DATA_TYPE_UNDEFINED;
    char value[256] = { 0x0b, 0x00,
        0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x48, 0x49, 0x53, 0x49,
        0x02, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x00, 0xcd, 0xcc, 0xcc, 0x3d,
        0x04, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x66, 0x66, 0x06, 0x40,
        0x07, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x9a, 0x99, 0xbc, 0x42,
        0x08, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x33, 0x33, 0xf3, 0x3f,
        0x09, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb4, 0xc2,
        0x0a, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x42,
        0x0b, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
          };
    stExifMakerNodeTagEntry.u32Cnt =  256;
    stExifMakerNodeTagEntry.pValue = value;

    enTagLocation = HI_EXIF_APP1_EXIFIFD;

    s32Ret = HI_EXIF_SetTag(hExifHandle, &stExifMakerNodeTagEntry, enTagLocation);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_SetTag for maker node exec failed,ret:%x \n", s32Ret);
        goto EXIF_CLOSE;
    }

EXIF_CLOSE:
    s32Ret = HI_EXIF_Close(hExifHandle);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_Close exec failed,ret:%x \n", s32Ret);
        return s32Ret;
    }

FREE_BUF:
    i = 0;
    for(; i< HI_EXIF_DATA_SEG_MAX_CNT; i++)
    {
        if(stBufSource.pImgBuf[i])
            free(stBufSource.pImgBuf[i]);
    }

    return HI_SUCCESS;
}


int EXIF_DEMUXER(char* fileName)
{
    char* inFilename = fileName;
    HI_S32  s32Ret = HI_SUCCESS;
    struct timeval stBeginTime, stEndTime;

    gettimeofday(&stBeginTime, 0);
    HI_MW_PTR pExif;
    HI_EXIF_ATTR_S stExifAttr;
    memset(&stExifAttr, 0, sizeof(HI_EXIF_ATTR_S));
    stExifAttr.enExifConfigType = HI_EXIF_CONFIG_DEMUXER;
    stExifAttr.stDemuxerConfig.pszFileName = inFilename;
    s32Ret = HI_EXIF_Open(&pExif, &stExifAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_Open exec failed \n");
        return -1;
    }
    gettimeofday(&stEndTime, 0);
    printf("\n HI_EXIF_Open cost time:%ld ms \n", stEndTime.tv_sec*1000 + stEndTime.tv_usec/1000 - stBeginTime.tv_sec*1000 - stBeginTime.tv_usec/1000);

    HI_EXIF_INFO_S stPicDemuxerInfo;
    s32Ret = HI_EXIF_GetInfo(pExif, &stPicDemuxerInfo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_GetInfo exec failed \n");
        goto EXIF_CLOSE;
    }


    HI_U32 u32Index = 0;
    for(; u32Index < HI_EXIF_SUBIMG_MAX_CNT; u32Index++)
    {
       printf("index:%u,  width:%u, height:%u, data len:%u, bthumbnail:%d  \n",
        u32Index, stPicDemuxerInfo.stSubImg[u32Index].u32Width, stPicDemuxerInfo.stSubImg[u32Index].u32Height,
        stPicDemuxerInfo.stSubImg[u32Index].u32DataLen, stPicDemuxerInfo.stSubImg[u32Index].bThumbnail);

       if (stPicDemuxerInfo.stSubImg[u32Index].u32DataLen != 0)
       {
           char fileName[SAMPLE_FILENAME_LEN_MAX] = {0};
           snprintf(fileName, SAMPLE_FILENAME_LEN_MAX, "%s%d.jpeg", SAMPLE_EXIF_DEMUXER_OUTPUT_FILE_PREFIX,u32Index);
           printf("filename:%s \n", fileName);
           FILE* out_pic = fopen(fileName, "w+");
           if(out_pic == NULL)
           {
               printf("create file failed, error:%d \n", errno);
               goto EXIF_CLOSE;
           }

           unsigned char* pData = (unsigned char*) malloc(stPicDemuxerInfo.stSubImg[u32Index].u32DataLen);
           printf("read len: %u\n", stPicDemuxerInfo.stSubImg[u32Index].u32DataLen);
           memset(pData, 0, stPicDemuxerInfo.stSubImg[u32Index].u32DataLen);

           HI_EXIF_DATA_S stPicDemuxerData;
           stPicDemuxerData.u32PicIndex = u32Index;
           stPicDemuxerData.u32DataLen = stPicDemuxerInfo.stSubImg[u32Index].u32DataLen;
           stPicDemuxerData.pu8Data =  pData;
           s32Ret = HI_EXIF_ReadPicData(pExif, &stPicDemuxerData);
           if (HI_SUCCESS != s32Ret)
           {
               printf("HI_EXIF_ReadPicData exec failed \n");
               fclose(out_pic);
               free(pData);
               goto EXIF_CLOSE;
           }


           fwrite(pData, 1, stPicDemuxerInfo.stSubImg[u32Index].u32DataLen, out_pic);
           fclose(out_pic);
           free(pData);

       }

    }
    printf("file len is %lld\n", stPicDemuxerInfo.s64FileSize);

EXIF_CLOSE:
    s32Ret = HI_EXIF_Close(pExif);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_Close exec failed \n");
        return -1;
    }

    return HI_SUCCESS;
}

void PRINT_HELP()
{
    printf("usage: ./sample_exif  editor/demuxer  filename \n");
    printf("we use 0 replace editor, 1 replace demuxer \n");
    printf("eg: add tag for a picture: ./sample_exif 0 exif.jpeg \n");
    printf("eg: demuxer for a picture: ./sample_exif 1 mpf.jpeg \n");
}

#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef __HuaweiLite__
    /*init cpp running context on liteos begin*/
    extern char __init_array_start__, __init_array_end__;

    LOS_CppSystemInit((unsigned long)&__init_array_start__, (unsigned long)&__init_array_end__, NO_SCATTER);
    /*init cpp running context on liteos end*/
#endif
    if (argc < 3)
    {
        PRINT_HELP();
        return -1;
    }

    HI_EXIF_CONFIG_TYPE_E enExifConfig;
    if (0 == atoi(argv[1]))
    {
        enExifConfig = HI_EXIF_CONFIG_EDITOR;
    }
    else if (1 == atoi(argv[1]))
    {
        enExifConfig = HI_EXIF_CONFIG_DEMUXER;
    }
    else
    {
        printf("illegal param argv[1]:%s \n", argv[1]);
        PRINT_HELP();
        return -1;
    }

    SAMPLE_mount_tmp_dir();

    char* filename = argv[2];
    HI_S32  s32Ret = HI_SUCCESS;
    s32Ret = HI_EXIF_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_Init exec failed,ret:%x \n", s32Ret);
        goto EXIT;
    }

    if (HI_EXIF_CONFIG_DEMUXER == enExifConfig)
    {
        s32Ret = EXIF_DEMUXER(filename);
    }
    else
    {
        s32Ret =EXIF_EDITOR(filename);
    }

    s32Ret = HI_EXIF_DeInit();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_EXIF_DeInit exec failed \n");
        goto EXIT;
    }

EXIT:
    SAMPLE_umount_tmp_dir();
    return s32Ret;
}

#ifdef __HuaweiLite__
void __pthread_key_create()
{
    printf("__pthread_key_create not support\n");
}

void __register_atfork()
{
    printf("__register_atfork not support\n");
}

FILE * tmpfile()
{
    printf("tmpfile not support\n");
}
#endif
