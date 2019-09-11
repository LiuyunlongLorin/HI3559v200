#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "venc_wrapper.h"
#include "hi_recorder_pro.h"
#include "rtsp_wrapper.h"
#include "hi_mw_media_intf.h"
#include "hi_rtsp_server.h"
#include "rtsp_wrapper.h"
const HI_U32 MAX_THM_SIZE = 50 * 1024;
extern HI_MW_PTR g_hRecHandle;
extern HI_MW_PTR s_hRtspServerHandle;
FILE* g_pFile = NULL;
HI_BOOL bWrite = HI_FALSE;
//sensor type define in makefile.
#if defined(SENSOR_OV4689)
static HI_MW_SensorAttr_S s_stSensorAttr = {0, 2688, 1512, 30};
static HI_MW_VCapAttr_S s_stVCapAttr = {0, 2688, 1512, 30};
static HI_MW_VProcAttr_S s_stVProcAttr = {1, 2688, 1512};
static HI_MW_VPortAttr s_stVPort4MainAttr = {1, 2560, 1440, 30},
                       s_stVPort4SlaveAttr = {2, 640, 360, 30},
                       s_stVPort4ThmAttr = {3, 320, 240, 30};
static HI_MW_VEncAttr s_stVEnc4MainAttr = {0, HI_MW_PAYLOAD_TYPE_H264, 2560, 1440, 2764800, 0, 1, 0, HI_FALSE, 20480, 30, 30},
                      s_stVEnc4SlaveAttr = {1, HI_MW_PAYLOAD_TYPE_H264, 640, 360, 172800, 0, 1, 0, HI_FALSE, 2048, 30, 30},
                      s_stVEnc4ThmAttr = {2, HI_MW_PAYLOAD_TYPE_JPEG, 320, 240, 320 * 240 * 2, 0, 1, 0, HI_FALSE, 1000, 30, 30};
#elif defined(SENSOR_MN34120)
static HI_MW_SensorAttr_S s_stSensorAttr = {0, 3840, 2160, 30};
static HI_MW_VCapAttr_S s_stVCapAttr = {0, 3840, 2160, 30};
static HI_MW_VProcAttr_S s_stVProcAttr = {1, 3840, 2160};
static HI_MW_VPortAttr s_stVPort4MainAttr = {0, 3840, 2160, 30},
                       s_stVPort4SlaveAttr = {1, 640, 360, 30},
                       s_stVPort4ThmAttr = {2, 640, 360, 30};
static HI_MW_VEncAttr s_stVEnc4MainAttr = {0, HI_MW_PAYLOAD_TYPE_H264, 3840, 2160, 3840 * 2160 * 2, 0, 1, 0, HI_FALSE, 40000, 30, 30},
                      s_stVEnc4SlaveAttr = {1, HI_MW_PAYLOAD_TYPE_H264, 640, 360, 640 * 360 * 2, 0, 1, 0, HI_FALSE, 2000, 30, 30},
                      s_stVEnc4ThmAttr = {2, HI_MW_PAYLOAD_TYPE_JPEG, 320, 240, 320 * 240 * 2, 0, 1, 0, HI_FALSE, 1000, 30, 30};
#elif defined(SENSOR_IMX477)
static HI_MW_SensorAttr_S s_stSensorAttr = {0, 3840, 2160, 30};
static HI_MW_VCapAttr_S s_stVCapAttr = {0, 3840, 2160, 30};
static HI_MW_VProcAttr_S s_stVProcAttr = {0, 3840, 2160};
static HI_MW_VPortAttr s_stVPort4MainAttr = {0, 3840, 2160, 30},
                       s_stVPort4SlaveAttr = {1, 640, 360, 30},
                       s_stVPort4ThmAttr = {2, 640, 360, 30};
static HI_MW_VEncAttr s_stVEnc4MainAttr = {0, HI_MW_PAYLOAD_TYPE_H264, 3840, 2160, 3840 * 2160 * 2, 0, 1, 0, HI_TRUE, 40000, 30, 30},
                      s_stVEnc4SlaveAttr = {1, HI_MW_PAYLOAD_TYPE_H264, 640, 360, 640 * 360 * 2, 1, 1, 0, HI_TRUE, 2000, 30, 30},
                      s_stVEnc4ThmAttr = {2, HI_MW_PAYLOAD_TYPE_JPEG, 320, 240, 320 * 240 * 2, 0, 1, 0, HI_FALSE, 1000, 30, 30};
#elif defined(SENSOR_IMX117)
static HI_MW_SensorAttr_S s_stSensorAttr = {0, 3840, 2160, 30};
static HI_MW_VCapAttr_S s_stVCapAttr = {0, 3840, 2160, 30};
static HI_MW_VProcAttr_S s_stVProcAttr = {1, 3840, 2160};
static HI_MW_VPortAttr s_stVPort4MainAttr = {0, 3840, 2160, 30},
                       s_stVPort4SlaveAttr = {1, 640, 360, 30},
                       s_stVPort4ThmAttr = {2, 640, 360, 30};
static HI_MW_VEncAttr s_stVEnc4MainAttr = {0, HI_MW_PAYLOAD_TYPE_H264, 3840, 2160, 3840 * 2160 * 2, 0, 1, 0, HI_FALSE, 40000, 30, 30},
                      s_stVEnc4SlaveAttr = {1, HI_MW_PAYLOAD_TYPE_H264, 640, 360, 640 * 360 * 2, 0, 1, 0, HI_FALSE, 2000, 30, 30},
                      s_stVEnc4ThmAttr = {2, HI_MW_PAYLOAD_TYPE_JPEG, 320, 240, 320 * 240 * 2, 0, 1, 0, HI_FALSE, 1000, 30, 30};
#elif defined(SENSOR_OV2718)
static HI_MW_SensorAttr_S s_stSensorAttr = {0, 1920, 1080, 30};
static HI_MW_VCapAttr_S s_stVCapAttr = {0, 1920, 1080, 30};
static HI_MW_VProcAttr_S s_stVProcAttr = {0, 1920, 1080};
static HI_MW_VPortAttr s_stVPort4MainAttr = {0, 1920, 1080, 30},
                       s_stVPort4SlaveAttr = {1, 640, 360, 30},
                       s_stVPort4ThmAttr = {2, 320, 240, 30};
static HI_MW_VEncAttr s_stVEnc4MainAttr = {0, HI_MW_PAYLOAD_TYPE_H264, 1920, 1080, 1920 * 1080 * 2, 0, 1, 0, HI_TRUE, 10000, 30, 30},
                      s_stVEnc4SlaveAttr = {1, HI_MW_PAYLOAD_TYPE_H264, 640, 360, 640 * 360 * 2, 1, 1, 0, HI_TRUE, 2000, 30, 30},
                      s_stVEnc4ThmAttr = {2, HI_MW_PAYLOAD_TYPE_JPEG, 320, 240, 320 * 240 * 2, 0, 1, 0, HI_FALSE, 1000, 30, 30};
#elif defined(SENSOR_IMX307)
static HI_MW_SensorAttr_S s_stSensorAttr = {0, 1920, 1080, 30};
static HI_MW_VCapAttr_S s_stVCapAttr = {0, 1920, 1080, 30};
static HI_MW_VProcAttr_S s_stVProcAttr = {0, 1920, 1080};
static HI_MW_VPortAttr s_stVPort4MainAttr = {0, 1920, 1080, 30},
                       s_stVPort4SlaveAttr = {1, 640, 360, 30},
                       s_stVPort4ThmAttr = {2, 320, 240, 30};
static HI_MW_VEncAttr s_stVEnc4MainAttr = {0, HI_MW_PAYLOAD_TYPE_H264, 1920, 1080, 1920 * 1080 * 2, 0, 1, 0, HI_TRUE, 10000, 30, 30},
                      s_stVEnc4SlaveAttr = {1, HI_MW_PAYLOAD_TYPE_H264, 640, 360, 640 * 360 * 2, 1, 1, 0, HI_TRUE, 2000, 30, 30},
                      s_stVEnc4ThmAttr = {2, HI_MW_PAYLOAD_TYPE_JPEG, 320, 240, 320 * 240 * 2, 0, 1, 0, HI_FALSE, 1000, 30, 30};

#endif


static HI_MW_VideoBindMap_S s_astMediaAttr[] =
{
    {&s_stVProcAttr, &s_stVPort4MainAttr, &s_stVEnc4MainAttr},
    {&s_stVProcAttr, &s_stVPort4SlaveAttr, &s_stVEnc4SlaveAttr},
    {&s_stVProcAttr, &s_stVPort4ThmAttr, &s_stVEnc4ThmAttr},
};

static HI_U32 s_u32LostPacket = 0;

extern HI_BOOL IsMainVideoTrackSource(HI_Track_Source_S* pstTrackSource);

static OnDataReceived_FN s_pfnOnThmReceiveCB = NULL;
static HI_U8* s_pu8ThmData = NULL;
static HI_S32 VEncThmDataProc(__attribute__((unused))HI_HANDLE hVEncHdl, HI_MW_MEDIA_VIDEO_S* pVStreamData, __attribute__((unused))HI_VOID* pPrivateData)
{
    printf("thm received\n");
    HI_U32 i = 0;
    HI_CHAR* pWriteAddr = HI_NULL;
    HI_U32 u32WriteLen = 0;
    HI_CHAR* pWritePos = s_pu8ThmData;
    HI_U32 u32TotalLen = 0;
    if (pWritePos == NULL)
    {
        printf("write addr is null\n");
        return HI_FAILURE;
    }
    for (i = 0; i < pVStreamData->u32PackCount; i++)
    {
        pWriteAddr = pVStreamData->astPack[i].pu8Addr[0];
        u32WriteLen = pVStreamData->astPack[i].au32Len[0];
        if (u32TotalLen + u32WriteLen > MAX_THM_SIZE)
        {
            printf("Thm u32TotalLen %d writeLen %d\n", u32TotalLen, u32WriteLen);
            return HI_FAILURE;
        }
        memcpy(pWritePos, pWriteAddr, u32WriteLen);
        pWritePos += u32WriteLen;
        u32TotalLen += u32WriteLen;
        //retrace happened,only on liteos,one packet devide two segment.
        if (2 == pVStreamData->astPack[i].u32SegCnt)
        {
            pWriteAddr = pVStreamData->astPack[i].pu8Addr[1];
            u32WriteLen = pVStreamData->astPack[i].au32Len[1];
            if (u32TotalLen + u32WriteLen > MAX_THM_SIZE)
            {
                printf("thm u32TotalLen %d writeLen %d\n", u32TotalLen, u32WriteLen);
                return HI_FAILURE;
            }
            memcpy(pWritePos, pWriteAddr, u32WriteLen);
            pWritePos += u32WriteLen;
            u32TotalLen += u32WriteLen;
        }
    }
    printf("thm len: %d\n", u32TotalLen);
    return s_pfnOnThmReceiveCB(s_pu8ThmData, u32TotalLen);
}

HI_S32 RequestThumbnail(OnDataReceived_FN pfnOnDataReceived)
{
    printf("in %s\n", __FUNCTION__);
    s_pfnOnThmReceiveCB = pfnOnDataReceived;
    s_pu8ThmData = (HI_U8*)malloc(MAX_THM_SIZE);
    if (s_pu8ThmData == NULL)
    {
        printf("%s malloc failed.\n", __FUNCTION__);
        return HI_FAILURE;
    }
    return HI_MW_VEnc_Start(s_stVEnc4ThmAttr.HANDLE, 1, VEncThmDataProc, NULL);

}

HI_S32 RequestThumbnailEnd(__attribute__((unused))OnDataReceived_FN pfnOnDataReceived)
{
    printf("in %s\n", __FUNCTION__);
    if (s_pu8ThmData != NULL)
    {
        free(s_pu8ThmData);
        s_pu8ThmData = NULL;
    }
    return HI_MW_VEnc_Stop(s_stVEnc4ThmAttr.HANDLE, VEncThmDataProc);
}
FILE* pRtspFile = NULL;

static HI_S32 VEncDataProc2(__attribute__((unused))HI_HANDLE hVEncHdl, HI_MW_MEDIA_VIDEO_S* pVStreamData, HI_VOID* pPrivateData)
{
    HI_Track_Source_Handle pTrackSource = (HI_Track_Source_Handle)pPrivateData;
    HI_RTSP_DATA_S stFrame;
    stFrame.u32BlockCnt = 0;
    HI_S32 s32Ret;
    HI_U32 i;
    for (i = 0; i < pVStreamData->u32PackCount; i++)
    {
        stFrame.apu8DataPtr[stFrame.u32BlockCnt] = pVStreamData->astPack[i].pu8Addr[0] + pVStreamData->astPack[i].u32Offset;
        stFrame.au32DataLen[stFrame.u32BlockCnt] = pVStreamData->astPack[i].au32Len[0] - pVStreamData->astPack[i].u32Offset;
        stFrame.u32BlockCnt++;
        //retrace happened,only on liteos,one packet devide two segment.
        if (2 == pVStreamData->astPack[i].u32SegCnt)
        {
            stFrame.apu8DataPtr[stFrame.u32BlockCnt] = pVStreamData->astPack[i].pu8Addr[1] + pVStreamData->astPack[i].u32Offset;
            stFrame.au32DataLen[stFrame.u32BlockCnt] = pVStreamData->astPack[i].au32Len[1] - pVStreamData->astPack[i].u32Offset;
            stFrame.u32BlockCnt++;
        }
        stFrame.u64Pts = pVStreamData->astPack[i].u64PTS;
        if (HI_MW_PAYLOAD_TYPE_H264 == pVStreamData->astPack[i].stDataType.enPayloadType)
        {
            if (HI_MW_H264E_NALU_ISLICE == pVStreamData->astPack[i].stDataType.enH264EType
                || HI_MW_H264E_NALU_IDRSLICE == pVStreamData->astPack[i].stDataType.enH264EType
                || HI_MW_H264E_NALU_SPS == pVStreamData->astPack[0].stDataType.enH264EType)
            {
                stFrame.bIsKeyFrame = HI_TRUE;
            }
            else
            {
                stFrame.bIsKeyFrame = HI_FALSE;
            }
        }
        else if (HI_MW_PAYLOAD_TYPE_H265 == pVStreamData->astPack[i].stDataType.enPayloadType)
        {
            if (HI_MW_H265E_NALU_ISLICE == pVStreamData->astPack[i].stDataType.enH265EType
                || HI_MW_H265E_NALU_IDRSLICE == pVStreamData->astPack[i].stDataType.enH265EType
                || HI_MW_H265E_NALU_SPS == pVStreamData->astPack[0].stDataType.enH265EType)
            {
                stFrame.bIsKeyFrame = HI_TRUE;
            }
            else
            {
                stFrame.bIsKeyFrame = HI_FALSE;
            }
        }
        else
        {
            stFrame.bIsKeyFrame = HI_FALSE;
        }
    }
    s32Ret = HI_RTSPSVR_WriteFrame(s_hRtspServerHandle,  pTrackSource, &stFrame);
    return s32Ret;

}


HI_S32 SAMPLE_COMM_SaveStreamFile(HI_HANDLE VencHdl, HI_MW_MEDIA_VIDEO_S* pVStreamData, HI_VOID* pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aFileName[128] = {0};
    HI_CHAR aPayLoadType[8] = {0};
    FILE* pVencFile = NULL;

    switch (pVStreamData->astPack[0].stDataType.enPayloadType)
    {
        case HI_MW_PAYLOAD_TYPE_H264:
            snprintf(aPayLoadType, 8, "%s", "h264");
            break;
        case HI_MW_PAYLOAD_TYPE_H265:
            snprintf(aPayLoadType, 8, "%s", "h265");
            break;
        default:
            printf("enPayloadType is error.\n");
            return -1;
    }

    if (pPrivateData != NULL)
    {
        snprintf(aFileName, 128, "%s_chn%d.%s", (char*)pPrivateData, VencHdl, aPayLoadType);
    }
    else
    {
        snprintf(aFileName, 128, "%s_chn%d.%s", "null", VencHdl, aPayLoadType);
    }

    if (pVStreamData->u32Seq == 0)
    {
        pVencFile = fopen(aFileName, "w");
    }
    else
    {
        pVencFile = fopen(aFileName, "a");
    }

    if (pVencFile == NULL)
    {
        printf("fopen %s error.\n", aFileName);
        return HI_FAILURE;
    }

    HI_U32 i = 0;
    HI_U8* pPackVirtAddr;
    HI_U32 u32PackLen;
    HI_U32 u32PackOffset;
    for ( i = 0 ; i < pVStreamData->u32PackCount; i++ )
    {
        pPackVirtAddr = pVStreamData->astPack[i].pu8Addr[0];
        u32PackLen = pVStreamData->astPack[i].au32Len[0];
        u32PackOffset = pVStreamData->astPack[i].u32Offset;
        s32Ret = fwrite(pPackVirtAddr + u32PackOffset, u32PackLen - u32PackOffset, 1, pVencFile);
        printf("[%s] fwrite u64PTS:%lld pPackVirtAddr:%p u32PackLen-u32PackOffset %d\n", __FUNCTION__, pVStreamData->astPack[0].u64PTS, pPackVirtAddr, u32PackLen - u32PackOffset);
        if (s32Ret < 0)
        {
            printf("fwrite error %d\n", s32Ret);
        }
        if (fflush(pVencFile))
        {
            perror("fflush file\n");
        }
    }

    if (fclose(pVencFile))
    {
        printf("fclose error\n");
    }

    return HI_SUCCESS;
}

static HI_S32 VEncDataProc(__attribute__((unused))HI_HANDLE hVEncHdl, HI_MW_MEDIA_VIDEO_S* pVStreamData, HI_VOID* pPrivateData)
{
    HI_Track_Source_Handle pTrackSource = (HI_Track_Source_Handle)pPrivateData;
    HI_REC_FRAME_DATA_S stRecFrame;
    stRecFrame.u32SliceCnt = 0;
    HI_S32 s32Ret;
    HI_U32 i;
    for (i = 0; i < pVStreamData->u32PackCount; i++)
    {
        stRecFrame.apu8SliceAddr[stRecFrame.u32SliceCnt] = pVStreamData->astPack[i].pu8Addr[0] + pVStreamData->astPack[i].u32Offset;
        stRecFrame.au32SliceLen[stRecFrame.u32SliceCnt] = pVStreamData->astPack[i].au32Len[0] - pVStreamData->astPack[i].u32Offset;
        stRecFrame.u32SliceCnt++;
        //retrace happened,only on liteos,one packet devide two segment.
        if (2 == pVStreamData->astPack[i].u32SegCnt)
        {
            stRecFrame.apu8SliceAddr[stRecFrame.u32SliceCnt] = pVStreamData->astPack[i].pu8Addr[1] + pVStreamData->astPack[i].u32Offset;
            stRecFrame.au32SliceLen[stRecFrame.u32SliceCnt] = pVStreamData->astPack[i].au32Len[1] - pVStreamData->astPack[i].u32Offset;
            stRecFrame.u32SliceCnt++;
        }
        stRecFrame.u64TimeStamp = pVStreamData->astPack[i].u64PTS;
        if (HI_MW_PAYLOAD_TYPE_H264 == pVStreamData->astPack[i].stDataType.enPayloadType)
        {
            if (HI_MW_H264E_NALU_ISLICE == pVStreamData->astPack[i].stDataType.enH264EType
                || HI_MW_H264E_NALU_IDRSLICE == pVStreamData->astPack[i].stDataType.enH264EType
                || HI_MW_H264E_NALU_SPS == pVStreamData->astPack[0].stDataType.enH264EType)
            {
                stRecFrame.bKeyFrameFlag = HI_TRUE;
            }
            else
            {
                stRecFrame.bKeyFrameFlag = HI_FALSE;
            }
        }
        else if (HI_MW_PAYLOAD_TYPE_H265 == pVStreamData->astPack[i].stDataType.enPayloadType)
        {
            if (HI_MW_H265E_NALU_ISLICE == pVStreamData->astPack[i].stDataType.enH265EType
                || HI_MW_H265E_NALU_IDRSLICE == pVStreamData->astPack[i].stDataType.enH265EType
                || HI_MW_H265E_NALU_SPS == pVStreamData->astPack[0].stDataType.enH265EType)
            {
                stRecFrame.bKeyFrameFlag = HI_TRUE;
            }
            else
            {
                stRecFrame.bKeyFrameFlag = HI_FALSE;
            }
        }
        else
        {
            stRecFrame.bKeyFrameFlag = HI_FALSE;
        }
    }
    s32Ret = HI_REC_WriteData(g_hRecHandle, pTrackSource, &stRecFrame);
    if (HI_SUCCESS != s32Ret && IsMainVideoTrackSource(pTrackSource))
    {
        s_u32LostPacket++;
    }
    return s32Ret;
}

HI_S32 Venc4RtspStart(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    //HI_CHAR* aFileName = "/sd/rtspserver2.h264";
    //pRtspFile = fopen(aFileName, "a+");
    return HI_MW_VEnc_Start(pTrackSource->s32PrivateHandle, -1, VEncDataProc2, pTrackSource);
}

HI_S32 LapseVencStart(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    if (IsMainVideoTrackSource(pTrackSource))
    {
        s_u32LostPacket = 0;
    }
    return HI_MW_VEnc_Start(pTrackSource->s32PrivateHandle, 1, VEncDataProc, pTrackSource);
}

HI_S32 VencStart(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    if (IsMainVideoTrackSource(pTrackSource))
    {
        s_u32LostPacket = 0;
    }
    return HI_MW_VEnc_Start(pTrackSource->s32PrivateHandle, -1, VEncDataProc, pTrackSource);
}

HI_S32 VencStop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    if (IsMainVideoTrackSource(pTrackSource))
    {
        printf("main stream lost %d packet\n", s_u32LostPacket);
    }
    return HI_MW_VEnc_Stop(pTrackSource->s32PrivateHandle, VEncDataProc);
}

HI_S32 Venc4RtspStop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    HI_S32 s32Ret = HI_MW_VEnc_Stop(pTrackSource->s32PrivateHandle, VEncDataProc2);
    //fclose(pRtspFile);
    return s32Ret;
}


#ifdef ENABLE_AUDIO

static HI_S32 AEncDataProc2(__attribute__((unused))HI_HANDLE hAEncHdl, HI_MW_MEDIA_AUDIO_S* pAStreamData, HI_VOID* pPrivateData)
{
    HI_Track_Source_Handle pTrackSource =  (HI_Track_Source_Handle)pPrivateData;
    HI_RTSP_DATA_S stFrame;
    stFrame.au32DataLen[0] = pAStreamData->u32Len;
    stFrame.u32Seq = 1;
    stFrame.apu8DataPtr[0] = pAStreamData->pu8Addr;
    stFrame.u32BlockCnt = 1;
    stFrame.bIsKeyFrame = HI_FALSE;

    stFrame.u64Pts = pAStreamData->u64TimeStamp;


    HI_RTSPSVR_WriteFrame(s_hRtspServerHandle,  pTrackSource, &stFrame);

}

static HI_S32 AEncDataProc(__attribute__((unused))HI_HANDLE hAEncHdl, HI_MW_MEDIA_AUDIO_S* pAStreamData, HI_VOID* pPrivateData)
{
    HI_Track_Source_Handle pTrackSource = (HI_Track_Source_Handle)pPrivateData;
    HI_REC_FRAME_DATA_S stRecFrame;
    stRecFrame.u32SliceCnt = 1;
    stRecFrame.u64TimeStamp = pAStreamData->u64TimeStamp;
    stRecFrame.apu8SliceAddr[0] = pAStreamData->pu8Addr;
    stRecFrame.au32SliceLen[0] = pAStreamData->u32Len;
    stRecFrame.bKeyFrameFlag = HI_FALSE;
    return HI_REC_WriteData(g_hRecHandle, pTrackSource, &stRecFrame);
}

HI_S32 Aenc4RtspStart(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return HI_MW_AEnc_Start(pTrackSource->s32PrivateHandle, AEncDataProc2, pTrackSource );
}

HI_S32 AencStart(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return HI_MW_AEnc_Start(pTrackSource->s32PrivateHandle, AEncDataProc, pTrackSource);
}

HI_S32 Aenc4RtspStop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    HI_S32 s32Ret = HI_MW_AEnc_Stop(pTrackSource->s32PrivateHandle, AEncDataProc2);
    return s32Ret;
}

HI_S32 AencStop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return HI_MW_AEnc_Stop(pTrackSource->s32PrivateHandle, AEncDataProc);
}
#else
HI_S32 Aenc4RtspStart(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return HI_SUCCESS;
}

HI_S32 AencStart(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return HI_SUCCESS;
}

HI_S32 Aenc4RtspStop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return HI_SUCCESS;
}

HI_S32 AencStop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return HI_SUCCESS;
}
#endif

HI_S32 GetMainVEncAttr(HI_MW_VEncAttr* pstVEncAttr)
{
    memcpy(pstVEncAttr, &s_stVEnc4MainAttr, sizeof(HI_MW_VEncAttr));
    return HI_SUCCESS;
}

HI_S32 GetSlaveVEncAttr(HI_MW_VEncAttr* pstVEncAttr)
{
    memcpy(pstVEncAttr, &s_stVEnc4SlaveAttr, sizeof(HI_MW_VEncAttr));
    return HI_SUCCESS;
}

HI_S32 GetLiveVEncAttr(HI_MW_VEncAttr* pstVEncAttr)
{
    memcpy(pstVEncAttr, &s_stVEnc4SlaveAttr, sizeof(HI_MW_VEncAttr));
    return HI_SUCCESS;
}

HI_S32 MediaInit()
{
    return HI_MW_Media_Init(&s_stSensorAttr, &s_stVCapAttr, s_astMediaAttr, 3, 0, 0);
}

HI_S32 MediaDeInit()
{
    return HI_MW_Media_DeInit(&s_stSensorAttr, &s_stVCapAttr, s_astMediaAttr, 3, 0, 0);
}

HI_S32 GetAEncAttr(HI_HANDLE* pstAEncAttr)
{
    *pstAEncAttr = 0;
    return HI_SUCCESS;
}
