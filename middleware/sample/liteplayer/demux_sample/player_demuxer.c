#include <stdio.h>
#include <string.h>
#include "hi_demuxer.h"

extern HI_DEMUX_S  g_stFormat_entry ;
#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{

    printf("go to sample demuxer \n");
    const char* in_filename;
    HI_VOID* pFormat;
    char* out_filename_video = "test_video.h264";
    char* out_filename_audio = "test_audio.aac";
    int ret = 0;

    if (argc < 2)
    {
        printf("usage: %s filename \n", argv[0]);
        return -1;
    }

    in_filename  = argv[1];
    FILE* fp_audio = fopen(out_filename_audio, "wb+");

    if (fp_audio == NULL)
    {
        printf("create file  %s failed\n", out_filename_audio);
        return  -1;
    }

    FILE* fp_video = fopen(out_filename_video, "wb+");

    if (fp_video == NULL)
    {
        printf("create file  %s failed\n", out_filename_video);
        goto file_close;
    }

    ret = g_stFormat_entry.stFmtFun.fmt_open(&pFormat, in_filename);

    if ( ret != 0)
    {
        printf(" exec fmt_open failed \n");
        goto file_close;
    }

    ret = g_stFormat_entry.stFmtFun.fmt_probe(pFormat);

    if ( ret != 0)
    {
        printf(" exec fmt_probe failed \n");
        goto Release;
    }

    HI_FORMAT_FILE_INFO_S stFmtFileInfo;
    memset(&stFmtFileInfo,  0, sizeof(HI_FORMAT_FILE_INFO_S));
    ret = g_stFormat_entry.stFmtFun.fmt_getinfo(pFormat, &stFmtFileInfo);

    if ( ret != 0)
    {
        printf(" exec fmt_getinfo failed \n");
        goto Release;
    }

    printf(" used audiostream index %d \n", stFmtFileInfo.s32UsedAudioStreamIndex);
    printf(" used videostream index %d \n", stFmtFileInfo.s32UsedVideoStreamIndex);

    HI_FORMAT_PACKET_S fmtPacket;
    memset(&fmtPacket, 0,  sizeof(HI_FORMAT_PACKET_S));
    int video_cnt = 0;
    HI_BOOL  bNeedFree = HI_FALSE;

    while (1)
    {
        ret = g_stFormat_entry.stFmtFun.fmt_read(pFormat, &fmtPacket);

        if (ret < 0 )
        {
            printf(" exec fmt_read failed , ret:%d\n", ret);
            break;
        }

        if (ret == HI_RET_FILE_EOF)
        {
            printf("read file end\n");
            break;
        }

        if (fmtPacket.u32StreamIndex == (HI_U32)stFmtFileInfo.s32UsedAudioStreamIndex)
        {
            ret = fwrite(fmtPacket.pu8Data, 1, fmtPacket.u32Size, fp_audio);
            if(ret < 0)
            {
                bNeedFree = HI_TRUE;
                printf("fwrite audio file failed ret: %d\n", ret);
                break;
            }
        }
        else if (fmtPacket.u32StreamIndex == (HI_U32)stFmtFileInfo.s32UsedVideoStreamIndex)
        {

            if (video_cnt == 0)
            {
                if (fmtPacket.pu8Header == NULL)
                {
                    bNeedFree = HI_TRUE;
                    printf("err frame header is null");
                    break;
                }

                ret = fwrite(fmtPacket.pu8Header, 1, fmtPacket.u32HeaderLen, fp_video);
                if(ret < 0)
                {
                    bNeedFree = HI_TRUE;
                    printf("fwrite video file packet header failed ret: %d\n", ret);
                    break;
                }
            }

            printf("pts:%d, len:%d\n", (signed int)fmtPacket.s64Pts, fmtPacket.u32Size);
            ret = fwrite(fmtPacket.pu8Data, 1, fmtPacket.u32Size, fp_video);
            if(ret < 0)
            {
                bNeedFree = HI_TRUE;
                printf("fwrite video file failed ret: %d\n", ret);
                break;
            }
            video_cnt++;

        }
        else
        {
            printf("get packet's stream index is %d\n", fmtPacket.u32StreamIndex);
        }

        ret = g_stFormat_entry.stFmtFun.fmt_free(pFormat, &fmtPacket);

        if (ret < 0 )
        {
            printf(" exec fmt_free failed , ret:%d\n", ret);
            break;
        }
    }

    if(bNeedFree)
    {
        ret = g_stFormat_entry.stFmtFun.fmt_free(pFormat, &fmtPacket);
        if (ret < 0 )
        {
            printf(" exec fmt_free failed , ret:%d\n", ret);
        }
    }

Release:
    ret = g_stFormat_entry.stFmtFun.fmt_close(pFormat);

    if (ret != 0 )
    {
        printf(" exec fmt_close failed , ret:%d\n", ret);
    }

file_close:
    if (fp_video != NULL)
    {
        ret = fclose(fp_video);

        if (ret != 0)
        {
            printf("close video file failed \n");
        }
    }

    if (fp_audio != NULL)
    {
        ret = fclose(fp_audio);

        if (ret != 0)
        {
            printf("close audio file failed \n");
        }
    }

    return ret;
}
#ifdef __HuaweiLite__
HI_DEMUX_S  g_stFormat_entry ;
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