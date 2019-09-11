/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    sample_comm.c
 * @brief   sample comm function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#include <stdio.h>
#include "comm_define.h"
#include "sample_comm.h"
#include "sensor_comm.h"
#include "sensor_interface_cfg_params.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define DISP_HANDLE 0
#define WIND_HANDLE 0
#define HDMI_HANDLE 0

#define DATA_BITWIDTH DATA_BITWIDTH_8
#define TEST_AI_DEV   0 /** dev for inner acodec */
#define TEST_AENC_CHN 0

const HI_MAPI_SENSOR_ATTR_S g_stSample_Sensor_1080P_Mode0 = {
    .u8SnsMode = 0,
    .enWdrMode = WDR_MODE_NONE,
    .stSize = {
        .u32Width = 1920,
        .u32Height = 1080,
    }
};

const HI_MAPI_SENSOR_ATTR_S g_stSensor_4M_Mode0 = {
    .u8SnsMode = 0,
    .enWdrMode = WDR_MODE_NONE,
    .stSize = {
        .u32Width = 2592,
        .u32Height = 1536,
    }
};

const HI_MAPI_SENSOR_ATTR_S g_stSample_Sensor_4K_Mode1 = {
    .u8SnsMode = 1,
    .enWdrMode = WDR_MODE_NONE,
    .stSize = {
        .u32Width = 3840,
        .u32Height = 2160,
    }
};

const HI_MAPI_SENSOR_ATTR_S g_stSample_Sensor_4K_Mode0 = {
    .u8SnsMode = 0,
    .enWdrMode = WDR_MODE_NONE,
    .stSize = {
        .u32Width = 3840,
        .u32Height = 2160,
    }
};

const HI_MAPI_SENSOR_ATTR_S g_stSample_Sensor_12M_Mode1 = {
    .u8SnsMode = 1,
    .enWdrMode = WDR_MODE_NONE,
    .stSize = {
        .u32Width = 4000,
        .u32Height = 3000,
    }
};

const HI_MAPI_SENSOR_ATTR_S g_stSample_Sensor_12M_Mode0 = {
    .u8SnsMode = 0,
    .enWdrMode = WDR_MODE_NONE,
    .stSize = {
        .u32Width = 4000,
        .u32Height = 3000,
    }
};

const HI_MAPI_VCAP_PIPE_ATTR_S g_stPipe_1080P30FPS = {
    .stFrameRate = {
        .s32SrcFrameRate = -1,
        .s32DstFrameRate = -1,
    },
    .enCompressMode = COMPRESS_MODE_NONE,
    .bIspBypass = HI_FALSE,
    .enPipeBypassMode = VI_PIPE_BYPASS_NONE,
    .stIspPubAttr = {
        .f32FrameRate = 30.0f,
        .stSize = {
            .u32Width = 1920,
            .u32Height = 1080,
        },
        .enBayer = BAYER_RGGB,
    }
};

const HI_MAPI_VCAP_PIPE_ATTR_S g_stPipe_4M30FPS = {
    .stFrameRate = {
        .s32SrcFrameRate = -1,
        .s32DstFrameRate = -1,
    },
    .enCompressMode = COMPRESS_MODE_NONE,
    .bIspBypass = HI_FALSE,
    .enPipeBypassMode = VI_PIPE_BYPASS_NONE,
    .stIspPubAttr = {
        .f32FrameRate = 30.0f,
        .stSize = {
            .u32Width = 2592,
            .u32Height = 1520,
        },
        .enBayer = BAYER_RGGB,
    }
};

const HI_MAPI_VCAP_PIPE_ATTR_S g_stPipe_4K30FPS = {
    .stFrameRate = {
        .s32SrcFrameRate = -1,
        .s32DstFrameRate = -1,
    },
    .enCompressMode = COMPRESS_MODE_NONE,
    .bIspBypass = HI_FALSE,
    .enPipeBypassMode = VI_PIPE_BYPASS_NONE,
    .stIspPubAttr = {
        .f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE,
        .stSize = {
            .u32Width = 3840,
            .u32Height = 2160,
        },
        .enBayer = BAYER_RGGB,
    }
};

const HI_MAPI_VCAP_DEV_ATTR_S g_stDev_1080P = {
    .stBasSize = {
        .u32Width = 1920,
        .u32Height = 1080,
    },
    .enWdrMode = WDR_MODE_NONE,
};

const HI_MAPI_VCAP_DEV_ATTR_S g_stDev_4M = {
    .stBasSize = {
        .u32Width = 2592,
        .u32Height = 1520,
    },
    .enWdrMode = WDR_MODE_NONE,
};

const HI_MAPI_VCAP_DEV_ATTR_S g_stDev_4K = {
    .stBasSize = {
        .u32Width = 3840,
        .u32Height = 2160,
    },
    .enWdrMode = WDR_MODE_NONE,
};

const HI_MAPI_PIPE_CHN_ATTR_S g_stChn_1080P30FPS = {
    .stDestSize = {
        .u32Width = 1920,
        .u32Height = 1080,
    },
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enCompressMode = COMPRESS_MODE_NONE,
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
};

const HI_MAPI_PIPE_CHN_ATTR_S g_stChn_4M30FPS = {
    .stDestSize = {
        .u32Width = 2592,
        .u32Height = 1520,
    },
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enCompressMode = COMPRESS_MODE_NONE,
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
};

const HI_MAPI_PIPE_CHN_ATTR_S g_stChn_4K30FPS = {
    .stDestSize = {
        .u32Width = 3840,
        .u32Height = 2160,
    },
    .stFrameRate = {
        .s32SrcFrameRate = 29,
        .s32DstFrameRate = 29,
    },
    .enCompressMode = COMPRESS_MODE_NONE,
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
};

const HI_MAPI_VPSS_ATTR_S g_stVpss_1080P30FPS = {
    .u32MaxW = 1920,
    .u32MaxH = 1080,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .bNrEn = HI_TRUE,
    .stNrAttr = {
        .enCompressMode = COMPRESS_MODE_NONE,
        .enNrMotionMode = NR_MOTION_MODE_NORMAL,
    },
};

const HI_MAPI_VPSS_ATTR_S g_stVpss_4M30FPS = {
    .u32MaxW = 2592,
    .u32MaxH = 1520,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .bNrEn = HI_TRUE,
    .stNrAttr = {
        .enCompressMode = COMPRESS_MODE_NONE,
        .enNrMotionMode = NR_MOTION_MODE_NORMAL,
    },
};

const HI_MAPI_VPSS_ATTR_S g_stVpss_4K30FPS = {
    .u32MaxW = 3840,
    .u32MaxH = 2160,
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .bNrEn = HI_TRUE,
    .stNrAttr = {
        .enCompressMode = COMPRESS_MODE_NONE,
        .enNrMotionMode = NR_MOTION_MODE_NORMAL,
    },
};

const HI_MAPI_VPORT_ATTR_S g_stVport_1080P30FPS = {
    .u32Width = 1920,
    .u32Height = 1080,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .enVideoFormat = VIDEO_FORMAT_LINEAR,
    .enCompressMode = COMPRESS_MODE_NONE,
};

const HI_MAPI_VPORT_ATTR_S g_stVport_4M30FPS = {
    .u32Width = 2592,
    .u32Height = 1520,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .enVideoFormat = VIDEO_FORMAT_LINEAR,
    .enCompressMode = COMPRESS_MODE_NONE,
};

const HI_MAPI_VPORT_ATTR_S g_stVport_4K30FPS = {
    .u32Width = 3840,
    .u32Height = 2160,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .enVideoFormat = VIDEO_FORMAT_LINEAR,
    .enCompressMode = COMPRESS_MODE_NONE,
};

const HI_MAPI_VPORT_ATTR_S g_stVport_1080P30FPS_16BPP = {
    .u32Width = 1920,
    .u32Height = 1080,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .enVideoFormat = VIDEO_FORMAT_TILE_16x8,
    .enCompressMode = COMPRESS_MODE_NONE,
};

const HI_MAPI_VPORT_ATTR_S g_stVport_4K30FPS_16BPP = {
    .u32Width = 3840,
    .u32Height = 2160,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .enVideoFormat = VIDEO_FORMAT_TILE_16x8,
    .enCompressMode = COMPRESS_MODE_NONE,
};

const HI_MAPI_VENC_ATTR_S g_stVenc_1080P30FPS_JPEG = {
    .stVencPloadTypeAttr = {
        .enType = HI_MAPI_PAYLOAD_TYPE_JPEG,
        .u32Width = 1920,
        .u32Height = 1080,
        .u32BufSize = 1920 * 1080 * 3 / 2,
        .stAttrJpege = {
            .bEnableDCF = HI_TRUE,
            .u32Qfactor = 90,
            .enJpegEncodeMode = JPEG_ENCODE_SNAP,
            .stAttrMPF = {
                .u8LargeThumbNailNum = 0,
            },
        },
    },
};

const HI_MAPI_VENC_ATTR_S g_stVenc_4K30FPS_JPEG = {
    .stVencPloadTypeAttr = {
        .enType = HI_MAPI_PAYLOAD_TYPE_JPEG,
        .u32Width = 3840,
        .u32Height = 2160,
        .u32BufSize = 3840 * 2160 * 2,
        .u32Profile = 0,
        .stAttrJpege = {
            .bEnableDCF = HI_TRUE,
            .u32Qfactor = 90,
            .enJpegEncodeMode = JPEG_ENCODE_SNAP,
            .stAttrMPF = {
                .u8LargeThumbNailNum = 0,
            },
        },
    },
};

const HI_MAPI_VENC_ATTR_S g_stVenc_4K30FPS_VIDEO_H264 = {
    .stVencPloadTypeAttr = {
        .enType = HI_MAPI_PAYLOAD_TYPE_H264,
        .u32Width = 3840,
        .u32Height = 2160,
        .u32BufSize = 3840 * 2160 * 2,
        .u32Profile = 0,
        .enSceneMode = HI_MAPI_VENC_SCENE_MODE_DV,
    },
    .stRcAttr = {
        .stAttrCbr = {
            .u32Gop = 30,
            .u32StatTime = 2,
            .u32SrcFrameRate = 30,
            .fr32DstFrameRate = 30,
            .u32BitRate = 10000,
        }
    }
};

const HI_MAPI_VENC_ATTR_S g_stVenc_4K30FPS_VIDEO_H265 = {
    .stVencPloadTypeAttr = {
        .enType = HI_MAPI_PAYLOAD_TYPE_H265,
        .u32Width = 3840,
        .u32Height = 2160,
        .u32BufSize = 3840 * 2160 * 2,
        .u32Profile = 0,
        .enSceneMode = HI_MAPI_VENC_SCENE_MODE_DV,
    },
    .stRcAttr = {
        .stAttrCbr = {
            .u32Gop = 30,
            .u32StatTime = 2,
            .u32SrcFrameRate = 30,
            .fr32DstFrameRate = 30,
            .u32BitRate = 10000,
        }
    }
};

const HI_MAPI_VENC_ATTR_S g_stVenc_1080P30FPS_VIDEO_H265 = {
    .stVencPloadTypeAttr = {
        .enType = HI_MAPI_PAYLOAD_TYPE_H265,
        .u32Width = 1920,
        .u32Height = 1080,
        .u32BufSize = 1920 * 1080 * 2,
        .u32Profile = 0,
        .enSceneMode = HI_MAPI_VENC_SCENE_MODE_DV,
    },
    .stRcAttr = {
        .stAttrCbr = {
            .u32Gop = 30,
            .u32StatTime = 2,
            .u32SrcFrameRate = 30,
            .fr32DstFrameRate = 30,
            .u32BitRate = 4000,
        }
    }
};

const HI_MAPI_VENC_ATTR_S g_stVenc_720P30FPS_H264 = {
    .stVencPloadTypeAttr = {
        .enType = HI_MAPI_PAYLOAD_TYPE_H264,
        .u32Width = 1280,
        .u32Height = 720,
        .u32BufSize = 1280 * 720 * 2,
        .u32Profile = 0,
        .enSceneMode = HI_MAPI_VENC_SCENE_MODE_DV,
    },
    .stRcAttr = {
        .stAttrCbr = {
            .u32Gop = 30,
            .u32StatTime = 2,
            .u32SrcFrameRate = 30,
            .fr32DstFrameRate = 30,
            .u32BitRate = 4096,
        }
    }
};

const VO_PUB_ATTR_S g_stDisp_HDMI_1080P30 = {
    .u32BgColor = 0xFF,
    .enIntfType = VO_INTF_HDMI,
    .enIntfSync = VO_OUTPUT_1080P30,
    .stSyncInfo = {
        .bSynm = 0,
        .bIop = 0,
        .u8Intfb = 0,

        .u16Vact = 0,
        .u16Vbb = 0,
        .u16Vfb = 0,

        .u16Hact = 0,
        .u16Hbb = 0,
        .u16Hfb = 0,
        .u16Hmid = 0,

        .u16Bvact = 0,
        .u16Bvbb = 0,
        .u16Bvfb = 0,

        .u16Hpw = 0,
        .u16Vpw = 0,

        .bIdv = 0,
        .bIhs = 0,
        .bIvs = 0,
    },
};

const HI_MAPI_DISP_WINDOW_ATTR_S g_stWnd_1920x1080 = {
    .u32Priority = 0,
    .stRect = {
        .s32X = 0,
        .s32Y = 0,
        .u32Width = 1920,
        .u32Height = 1080,

    },

};

#ifdef SUPPORT_HDMI
const HI_MAPI_HDMI_ATTR_S g_sthdmi_1080P30 = {
    .bEnableHdmi = HI_TRUE,
    .bEnableVideo = HI_TRUE,
    .enHdmiSync = VO_OUTPUT_1080P30,
    .enSampleRate = AUDIO_SAMPLE_RATE_48000,
    .bEnableAudio = HI_FALSE,
    .u32PixClk = 0,

};
#endif

HI_CHAR g_aszDumpFrameName[HI_MAPI_VCAP_MAX_PIPE_NUM][64] = {0};
HI_S32 g_as32RawFrameCnt[HI_MAPI_VCAP_MAX_PIPE_NUM];

HI_S32 COMM_SnapTrigger(HI_S32 VCapPipeHdl, HI_S32 VEncHdl, HI_S32 s32Framecnt)
{
    HI_S32 s32Ret;
    HI_S32 i = 0;

    s32Ret = HI_MAPI_VENC_Stop(VEncHdl);

    if (s32Ret != HI_SUCCESS) {
        printf("HI_MAPI_VEnc_Stop fail s32Ret:%d\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MAPI_VENC_Start(VEncHdl, s32Framecnt);

    if (s32Ret != HI_SUCCESS) {
        printf("HI_MAPI_VEnc_Start fail s32Ret:%d\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MAPI_VCAP_StartTrigger(VCapPipeHdl);

    if (s32Ret != HI_SUCCESS) {
        printf("HI_MAPI_VCap_Trigger fail s32Ret:%d\n", s32Ret);
        return s32Ret;
    }

    sleep(1);
    s32Ret = HI_MAPI_VCAP_StopTrigger(VCapPipeHdl);

    if (s32Ret != HI_SUCCESS) {
        printf("HI_MAPI_VCap_Trigger fail s32Ret:%d\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_SaveJpegFile(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pVStreamData, HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aFileName[128] = {0};
    HI_CHAR aPayLoadType[8] = {0};
    FILE *pVencFile = NULL;
    static int u32count = 0;

    switch (pVStreamData->astPack[0].stDataType.enPayloadType) {
        case HI_MAPI_PAYLOAD_TYPE_JPEG:
            snprintf(aPayLoadType, 8, "%s", "jpg");
            break;

        default:
            printf("enPayloadType is error.\n");
            return -1;
    }

    if (pPrivateData != NULL) {
        snprintf(aFileName, 128, "%s_chn%d_%d.%s", (char *)pPrivateData, VencHdl, u32count, aPayLoadType);
    } else {
        snprintf(aFileName, 128, "%s_chn%d_%d.%s", "null", VencHdl, u32count, aPayLoadType);
    }

    pVencFile = fopen(aFileName, "w");

    if (pVencFile == NULL) {
        printf("fopen %s error.\n", aFileName);
        return -1;
    }

    HI_S32 i = 0;

    for (i = 0; i < pVStreamData->u32PackCount; i++) {
        HI_U64 u64PackPhyAddr = pVStreamData->astPack[i].au64PhyAddr[0];
        HI_U32 u32PackLen = pVStreamData->astPack[i].au32Len[0];
        HI_U32 u32PackOffset = pVStreamData->astPack[i].u32Offset;

        HI_U64 s_pDataVirtAddr = (HI_U64)(HI_UL)pVStreamData->astPack[i].apu8Addr[1];
        HI_U64 s_u64DataPhyAddr = pVStreamData->astPack[i].au64PhyAddr[1];
        HI_U32 s_u32DataLen = pVStreamData->astPack[i].au32Len[1];
        HI_VOID *pPackVirtAddr = pVStreamData->astPack[i].apu8Addr[0];

        if (u64PackPhyAddr + u32PackLen >= s_u64DataPhyAddr + s_u32DataLen) {
            /* physical address retrace in offset segment */
            if (u64PackPhyAddr + u32PackOffset >= s_u64DataPhyAddr + s_u32DataLen) {
                HI_VOID *pSrcVirtAddr = (HI_VOID *)(HI_UL)(s_pDataVirtAddr +
                    ((u64PackPhyAddr + u32PackOffset) - (s_u64DataPhyAddr + s_u32DataLen)));
                s32Ret = fwrite(pSrcVirtAddr, u32PackLen - u32PackOffset, 1, pVencFile);

                if (s32Ret < 0) {
                    printf("fwrite error %d\n", s32Ret);
                }
            } else {
                /* physical address retrace in data segment */
                HI_U32 u32Left = (s_u64DataPhyAddr + s_u32DataLen) - u64PackPhyAddr;
                s32Ret = fwrite((HI_VOID *)((HI_UL)pPackVirtAddr + u32PackOffset), u32Left - u32PackOffset, 1,
                    pVencFile);

                if (s32Ret < 0) {
                    printf("fwrite error %d\n", s32Ret);
                }

                s32Ret = fwrite((HI_VOID *)(HI_UL)s_pDataVirtAddr, u32PackLen - u32Left, 1, pVencFile);

                if (s32Ret < 0) {
                    printf("fwrite error %d\n", s32Ret);
                }
            }
        } else {
            /* physical address retrace does not happen */
            s32Ret = fwrite((HI_VOID *)((HI_UL)pPackVirtAddr + u32PackOffset), u32PackLen - u32PackOffset, 1,
                pVencFile);

            if (s32Ret < 0) {
                printf("fwrite error %d\n", s32Ret);
            }
        }

        if (fflush(pVencFile)) {
            perror("fflush file\n");
        }
    }

    if (fclose(pVencFile)) {
        printf("fclose error\n");
    }

    printf("save jpeg %d\n", u32count);
    u32count++;

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_SaveStreamFile(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pVStreamData,
    HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aFileName[128] = {0};
    HI_CHAR aPayLoadType[8] = {0};
    FILE *pVencFile = NULL;
    static int u32count = 0;

    switch (pVStreamData->astPack[0].stDataType.enPayloadType) {
        case HI_MAPI_PAYLOAD_TYPE_H264:
            snprintf(aPayLoadType, 8, "%s", "h264");
            break;

        case HI_MAPI_PAYLOAD_TYPE_H265:
            snprintf(aPayLoadType, 8, "%s", "h265");
            break;

        default:
            printf("enPayloadType is error.\n");
            return -1;
    }

    if (pPrivateData != NULL) {
        snprintf(aFileName, 128, "%s_chn%d.%s", (char *)pPrivateData, VencHdl, aPayLoadType);
    } else {
        snprintf(aFileName, 128, "%s_chn%d.%s", "null", VencHdl, aPayLoadType);
    }

    if (pVStreamData->u32Seq == 0) {
        pVencFile = fopen(aFileName, "w");
    } else {
        pVencFile = fopen(aFileName, "a");
    }

    if (pVencFile == NULL) {
        printf("fopen %s error.\n", aFileName);
        return HI_FAILURE;
    }

    HI_S32 i = 0;

    for (i = 0; i < pVStreamData->u32PackCount; i++) {
        HI_U64 u64PackPhyAddr = pVStreamData->astPack[i].au64PhyAddr[0];
        HI_U32 u32PackLen = pVStreamData->astPack[i].au32Len[0];
        HI_U32 u32PackOffset = pVStreamData->astPack[i].u32Offset;

        HI_U64 s_pDataVirtAddr = (HI_U64)(HI_UL)pVStreamData->astPack[i].apu8Addr[1];
        HI_U64 s_u64DataPhyAddr = pVStreamData->astPack[i].au64PhyAddr[1];
        HI_U32 s_u32DataLen = pVStreamData->astPack[i].au32Len[1];
        HI_VOID *pPackVirtAddr = pVStreamData->astPack[i].apu8Addr[0];

        if (u64PackPhyAddr + u32PackLen >= s_u64DataPhyAddr + s_u32DataLen) {
            /* physical address retrace in offset segment */
            if (u64PackPhyAddr + u32PackOffset >= s_u64DataPhyAddr + s_u32DataLen) {
                HI_VOID *pSrcVirtAddr = (HI_VOID *)(HI_UL)(s_pDataVirtAddr +
                    ((u64PackPhyAddr + u32PackOffset) - (s_u64DataPhyAddr + s_u32DataLen)));
                s32Ret = fwrite(pSrcVirtAddr, u32PackLen - u32PackOffset, 1, pVencFile);

                if (s32Ret < 0) {
                    printf("fwrite error %d\n", s32Ret);
                }
            } else {
                /* physical address retrace in data segment */
                HI_U32 u32Left = (s_u64DataPhyAddr + s_u32DataLen) - u64PackPhyAddr;
                s32Ret = fwrite((HI_VOID *)((HI_UL)pPackVirtAddr + u32PackOffset), u32Left - u32PackOffset, 1,
                    pVencFile);

                if (s32Ret < 0) {
                    printf("fwrite error %d\n", s32Ret);
                }

                s32Ret = fwrite((HI_VOID *)(HI_UL)s_pDataVirtAddr, u32PackLen - u32Left, 1, pVencFile);

                if (s32Ret < 0) {
                    printf("fwrite error %d\n", s32Ret);
                }
            }
        } else {
            /* physical address retrace does not happen */
            s32Ret = fwrite((HI_VOID *)((HI_UL)pPackVirtAddr + u32PackOffset), u32PackLen - u32PackOffset, 1,
                pVencFile);

            if (s32Ret < 0) {
                printf("fwrite error %d\n", s32Ret);
            }
        }

        if (fflush(pVencFile)) {
            perror("fflush file\n");
        }
    }

    if (fclose(pVencFile)) {
        printf("fclose error\n");
    }

    return HI_SUCCESS;
}

HI_S32 VENC_DataProc(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pVStreamData, HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (pVStreamData->astPack[0].stDataType.enPayloadType == HI_MAPI_PAYLOAD_TYPE_JPEG) {
        s32Ret = SAMPLE_COMM_SaveJpegFile(VencHdl, pVStreamData, pPrivateData);
    } else {
        s32Ret = SAMPLE_COMM_SaveStreamFile(VencHdl, pVStreamData, pPrivateData);
    }

    if (s32Ret != HI_SUCCESS) {
        printf("VENC_SaveFile error s32Ret:%d\n", s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 SMAPLE_COMM_VENC_DataProc(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pVStreamData, HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (pVStreamData->astPack[0].stDataType.enPayloadType == HI_MAPI_PAYLOAD_TYPE_JPEG) {
        s32Ret = SAMPLE_COMM_SaveJpegFile(VencHdl, pVStreamData, pPrivateData);
    } else {
        s32Ret = SAMPLE_COMM_SaveStreamFile(VencHdl, pVStreamData, pPrivateData);
    }

    if (s32Ret != HI_SUCCESS) {
        printf("VENC_SaveFile error s32Ret:%d\n", s32Ret);
    }

    return HI_SUCCESS;
}
HI_S32 SAMPLE_COMM_Init(SAMPLE_COMM_INITATTR_S stSampleCommAttr, SAMPLE_VB_CFG_MODE_E enVbMode)
{
    HI_S32 s32Ret;

    /* start media */
    HI_MAPI_MEDIA_ATTR_S stMediaAttr;
    HI_U32 u32BlkSize;

    memset(&stMediaAttr, 0, sizeof(HI_MAPI_MEDIA_ATTR_S));
    memset(&stMediaAttr.stMediaConfig.stVIVPSSMode, 0, sizeof(VI_VPSS_MODE_S));

    memcpy(&stMediaAttr.stMediaConfig.stVIVPSSMode, &stSampleCommAttr.stViVpssMode, sizeof(VI_VPSS_MODE_S));
    stMediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt = 16;
    if (VB_MODE_1 == enVbMode) {
        u32BlkSize = COMMON_GetPicBufferSize(stSampleCommAttr.stResolution.u32Width,
                                             stSampleCommAttr.stResolution.u32Height,PIXEL_FORMAT_YVU_SEMIPLANAR_420,
                                             DATA_BITWIDTH, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u64BlkSize = u32BlkSize;
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u32BlkCnt = 16;

        u32BlkSize = COMMON_GetPicBufferSize(1920, 1080, PIXEL_FORMAT_YVU_SEMIPLANAR_420,
                                             DATA_BITWIDTH, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u64BlkSize = u32BlkSize;
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u32BlkCnt = 6;

        u32BlkSize = VI_GetRawBufferSize(stSampleCommAttr.stResolution.u32Width,
            stSampleCommAttr.stResolution.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[2].u64BlkSize = u32BlkSize;
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[2].u32BlkCnt = 2;

    } else if (VB_MODE_2 == enVbMode) {
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u64BlkSize = 18481152;  // VPSS
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u32BlkCnt = 15;

        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u64BlkSize = 3750016;  // vi
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u32BlkCnt = 22;

        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[2].u64BlkSize = 6220800;  // avs
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[2].u32BlkCnt = 4;

        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[2].u64BlkSize = 3110400;  // avs_chn1
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[2].u32BlkCnt = 4;
    }
    stMediaAttr.stVencModPara.u32H264eLowPowerMode = 1;
    stMediaAttr.stVencModPara.u32H265eLowPowerMode = 1;
    s32Ret = HI_MAPI_Media_Init(&stMediaAttr);

    if (s32Ret != HI_SUCCESS) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_Deinit(HI_VOID)
{
    HI_S32 s32Ret;
    s32Ret = HI_MAPI_Media_Deinit();

    if (s32Ret != HI_SUCCESS) {
        printf("HI_MAPI_Media_DeInit fail\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

#ifdef SUPPORT_HDMI
HI_VOID SAMPLE_COMM_EventPro(HI_HDMI_EVENT_TYPE_E event, HI_VOID *pPrivateData)
{
    switch (event) {
        case HI_HDMI_EVENT_HOTPLUG:
            printf("\033[0;32mhdmi HOTPLUG event! \033[0;39m\n");
            break;

        case HI_HDMI_EVENT_NO_PLUG:
            printf("\033[0;31mhdmi NO_PLUG event! \033[0;39m\n");
            break;

        default:
            printf("\033[0;31minvalid hdmi event! \033[0;39m\n");
            break;
    }
}
#endif

HI_S32 SAMPLE_COMM_StartHdmi(HI_VOID)
{
#ifdef SUPPORT_HDMI
    HI_S32 s32Ret = HI_SUCCESS;
    /* starts HDMI */
    HI_MAPI_HDMI_ATTR_S sthdmiattr;
    HI_HANDLE HdmiHdl = HDMI_HANDLE;
    HI_HDMI_CALLBACK_FUNC_S stHdmiCallBackFunc;

    stHdmiCallBackFunc.pfnHdmiEventCallback = (HI_HDMI_CallBack)SAMPLE_COMM_EventPro;
    stHdmiCallBackFunc.pPrivateData = HI_NULL;

    memset(&sthdmiattr, 0, sizeof(HI_MAPI_HDMI_ATTR_S));
    sthdmiattr.bEnableHdmi = HI_TRUE;
    sthdmiattr.bEnableVideo = HI_TRUE;
    sthdmiattr.enHdmiSync = VO_OUTPUT_1080P30;
    sthdmiattr.bEnableAudio = HI_FALSE;
    sthdmiattr.enSampleRate = AUDIO_SAMPLE_RATE_48000;

    CHECK_RET(HI_MAPI_HDMI_Init(HdmiHdl, &stHdmiCallBackFunc));
    CHECK_RET(HI_MAPI_HDMI_SetAttr(HdmiHdl, &sthdmiattr));
    CHECK_RET(HI_MAPI_HDMI_Start(HdmiHdl));
exit:
    return s32Ret;
#endif
    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_FastBootStartHdmi(HI_HANDLE HdmiHdl, const HI_MAPI_DISP_ATTR_S *pstDispAttr)
{
#ifdef SUPPORT_HDMI
    HI_S32 s32Ret = HI_SUCCESS;

    /* starts HDMI */
    HI_MAPI_HDMI_ATTR_S sthdmiattr;
    HI_HDMI_CALLBACK_FUNC_S stHdmiCallBackFunc;
    memcpy(&sthdmiattr, &g_sthdmi_1080P30, sizeof(HI_MAPI_HDMI_ATTR_S));
    stHdmiCallBackFunc.pfnHdmiEventCallback = NULL;
    stHdmiCallBackFunc.pPrivateData = HI_NULL;
    sthdmiattr.enHdmiSync = pstDispAttr->stPubAttr.enIntfSync;
    SAMPLE_CHECK_RET(HI_MAPI_HDMI_Init(HdmiHdl, &stHdmiCallBackFunc));
    SAMPLE_CHECK_RET(HI_MAPI_HDMI_SetAttr(HdmiHdl, &sthdmiattr));
    SAMPLE_CHECK_RET(HI_MAPI_HDMI_Start(HdmiHdl));

#endif
    return HI_SUCCESS;
}

HI_VOID SAMPLE_COMM_StopHdmi(HI_HANDLE HdmiHdl)
{
#ifdef SUPPORT_HDMI
    HI_MAPI_HDMI_Stop(HdmiHdl);
    HI_MAPI_HDMI_Deinit(HdmiHdl);
#endif
}

HI_S32 SAMPLE_COMM_StartPreview(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch)
{
    /* start disp */
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE DispHdl = DISP_HANDLE;
    HI_MAPI_DISP_ATTR_S stDispAttr = {0};
    HI_MAPI_DISP_VIDEOLAYER_ATTR_S stVideoLayerAttr = {0};

    stDispAttr.stPubAttr.u32BgColor = 0xFF;
    stDispAttr.stPubAttr.enIntfType = VO_INTF_HDMI;
    stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_1080P30;
    memset(&stDispAttr.stPubAttr.stSyncInfo, 0, sizeof(VO_SYNC_INFO_S));
    memset(&stVideoLayerAttr, 0, sizeof(HI_MAPI_DISP_VIDEOLAYER_ATTR_S));
    stVideoLayerAttr.u32BufLen = 4;
    CHECK_RET(HI_MAPI_DISP_Init(DispHdl, &stDispAttr));
    CHECK_RET(HI_MAPI_DISP_Start(DispHdl, &stVideoLayerAttr));

    CHECK_RET(SAMPLE_COMM_StartHdmi());

    /* start window */
    HI_MAPI_DISP_WINDOW_ATTR_S stWndAttr;
    HI_HANDLE WndHdl = WIND_HANDLE;

    stWndAttr.stRect.s32X = 0;
    stWndAttr.stRect.s32Y = 0;
    stWndAttr.stRect.u32Width = 1920;
    stWndAttr.stRect.u32Height = 1080;
    stWndAttr.u32Priority = 0;
    CHECK_RET(HI_MAPI_DISP_SetWindowAttr(DispHdl, WndHdl, &stWndAttr));
    CHECK_RET(HI_MAPI_DISP_StartWindow(DispHdl, WndHdl));
    CHECK_RET(HI_MAPI_DISP_Bind_VProc(GrpHdl, PortHdl, DispHdl, WndHdl, bStitch));
exit:
    return s32Ret;
}

HI_VOID SAMPLE_COMM_StopPreview(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch)
{
    /* start disp */
    HI_HANDLE DispHdl = DISP_HANDLE;
    HI_HANDLE HdmiHdl = HDMI_HANDLE;
    HI_HANDLE WndHdl = WIND_HANDLE;

    HI_MAPI_DISP_UnBind_VProc(GrpHdl, PortHdl, DispHdl, WndHdl, bStitch);
    HI_MAPI_DISP_StopWindow(DispHdl, WndHdl);
    SAMPLE_COMM_StopHdmi(HdmiHdl);
    HI_MAPI_DISP_Stop(DispHdl);
    HI_MAPI_DISP_Deinit(DispHdl);
}

HI_S32 SAMPLE_COMM_SaveYuvFile(HI_HANDLE VProcHdl, HI_HANDLE VPortHdl, HI_MAPI_FRAME_DATA_S *pVPortYUV,
                               HI_VOID *pPrivateData)
{
    unsigned int w, h;
    char *pVBufVirt_Y;
    char *pVBufVirt_C;
    char *pMemContent;

    // If this value is too small and the image is big, this memory may not be enough
    unsigned char TmpBuff[4096];
    HI_U8 *pUserPageAddr = NULL;
    PIXEL_FORMAT_E enPixelFormat = pVPortYUV->enPixelFormat;

    /* When the storage format is a planar format, this variable is used to keep the height of the UV component */
    HI_U32 u32UvHeight;
    HI_CHAR aFileName[128] = {0};
    FILE *pYuvFile = NULL;
    static HI_U64 u64OldPts = 0;

    if (pVPortYUV->enCompressMode != COMPRESS_MODE_NONE) {
        printf("Frame is compressed:%d\n", pVPortYUV->enCompressMode);
        return -1;
    }

    snprintf(aFileName, 128, "%s%d_port%d_w%d_h%d_sp%d.yuv", (char *)pPrivateData, VProcHdl, VPortHdl,
        pVPortYUV->u32Width, pVPortYUV->u32Height, ((enPixelFormat - PIXEL_FORMAT_YVU_SEMIPLANAR_422) ? 420 : 422));

    if ((pVPortYUV->u64pts - u64OldPts) / 20000000 > 0) {
        pYuvFile = fopen(aFileName, "w");
    } else {
        pYuvFile = fopen(aFileName, "a");
    }

    if (pYuvFile == NULL) {
        printf("fopen %s error.\n", aFileName);
        return -1;
    }

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == enPixelFormat) {
        u32UvHeight = pVPortYUV->u32Height / 2;
    } else if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == enPixelFormat) {
        u32UvHeight = pVPortYUV->u32Height;
    } else {
        printf("enPixelFormat error.\n");
        fclose(pYuvFile);
        return -1;
    }

    pUserPageAddr = (HI_U8 *)(HI_UL)pVPortYUV->u64VirAddr[0];
    if (pUserPageAddr == HI_NULL) {
        printf("mmap error.\n");
        fclose(pYuvFile);
        return -1;
    }

    pVBufVirt_Y = (HI_CHAR *)pUserPageAddr;
    pVBufVirt_C = pVBufVirt_Y + (pVPortYUV->u32Stride[0]) * (pVPortYUV->u32Height);

    /* save Y ---------------------------------------------------------------- */
    fprintf(stderr, "saving......Y......");
    for (h = 0; h < pVPortYUV->u32Height; h++) {
        pMemContent = pVBufVirt_Y + h * pVPortYUV->u32Stride[0];
        fwrite(pMemContent, pVPortYUV->u32Width, 1, pYuvFile);
    }
    fflush(pYuvFile);

    /* save U ---------------------------------------------------------------- */
    fprintf(stderr, "U......");
    for (h = 0; h < u32UvHeight; h++) {
        pMemContent = pVBufVirt_C + h * pVPortYUV->u32Stride[1];

        pMemContent += 1;

        for (w = 0; w < pVPortYUV->u32Width / 2; w++) {
            TmpBuff[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(TmpBuff, pVPortYUV->u32Width / 2, 1, pYuvFile);
    }
    fflush(pYuvFile);

    /* save V ---------------------------------------------------------------- */
    fprintf(stderr, "V......");
    for (h = 0; h < u32UvHeight; h++) {
        pMemContent = pVBufVirt_C + h * pVPortYUV->u32Stride[1];

        for (w = 0; w < pVPortYUV->u32Width / 2; w++) {
            TmpBuff[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(TmpBuff, pVPortYUV->u32Width / 2, 1, pYuvFile);
    }
    fflush(pYuvFile);

    if (fclose(pYuvFile)) {
        printf("fclose error\n");
    }

    fprintf(stderr, "done %llu!\n", pVPortYUV->u64pts);
    u64OldPts = pVPortYUV->u64pts;

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_SEQ_E enSensorSeq,
    HI_MAPI_SENSOR_ATTR_S *pstSensorAttr)
{
    if (pstSensorAttr == NULL) {
        printf("Sample Get Sensor Cfg is null pointer.\n");
        return HI_FAILURE;
    }

    if (SAMPLE_SENSOR_4K30 == enSensorSeq) {
        if (IMX377 == CFG_SENSOR_TYPE0) {
            memcpy(pstSensorAttr, &g_stSample_Sensor_4K_Mode1, sizeof(HI_MAPI_SENSOR_ATTR_S));
        } else if (IMX458 == CFG_SENSOR_TYPE0) {
            memcpy(pstSensorAttr, &g_stSample_Sensor_4K_Mode0, sizeof(HI_MAPI_SENSOR_ATTR_S));
        } else {
            printf("sensor type :%d is not adapt,default use mode 0\n", CFG_SENSOR_TYPE0);
            memcpy(pstSensorAttr, &g_stSample_Sensor_4K_Mode0, sizeof(HI_MAPI_SENSOR_ATTR_S));
        }
    } else if (SAMPLE_SENSOR_12M == enSensorSeq) {
        if (IMX377 == CFG_SENSOR_TYPE0) {
            memcpy(pstSensorAttr, &g_stSample_Sensor_12M_Mode1, sizeof(HI_MAPI_SENSOR_ATTR_S));
        } else if (IMX458 == CFG_SENSOR_TYPE0) {
            memcpy(pstSensorAttr, &g_stSample_Sensor_12M_Mode0, sizeof(HI_MAPI_SENSOR_ATTR_S));
        } else {
            printf("sensor type :%d is not adapt,default use mode 0\n", CFG_SENSOR_TYPE0);
            memcpy(pstSensorAttr, &g_stSample_Sensor_12M_Mode0, sizeof(HI_MAPI_SENSOR_ATTR_S));
        }
    } else if (SAMPLE_SENSOR_1080P30 == enSensorSeq) {
        memcpy(pstSensorAttr, &g_stSample_Sensor_1080P_Mode0, sizeof(HI_MAPI_SENSOR_ATTR_S));
    } else if (SAMPLE_SENSOR_4M30 == enSensorSeq) {
        memcpy(pstSensorAttr, &g_stSensor_4M_Mode0, sizeof(HI_MAPI_SENSOR_ATTR_S));
    } else {
        memcpy(pstSensorAttr, &g_stSample_Sensor_4K_Mode0, sizeof(HI_MAPI_SENSOR_ATTR_S));
        printf("other sensor seq to be add later, defalut use  g_stSensor_4K30FPS_Mode0\n");
    }

    return HI_SUCCESS;
}

HI_S32 MAPI_SAMPLE_COMM_GetMediaCFG(SAMPLE_SENSOR_SEQ_E enSensorSeq,
    SAMPLE_MODE_COMM_CONFIG_S *pstCommConfig)
{
    if (pstCommConfig == NULL) {
        printf("Input is null!\n");
        return HI_FAILURE;
    }

    if (enSensorSeq == SAMPLE_SENSOR_4K30) {
        pstCommConfig->pstVcapDevAttr = &g_stDev_4K;
        pstCommConfig->pstVcapPipeAttr = &g_stPipe_4K30FPS;
        pstCommConfig->pstPipeChnAttr = &g_stChn_4K30FPS;
        pstCommConfig->pstVpssAttr = &g_stVpss_4K30FPS;
        pstCommConfig->pstVPortAttr = &g_stVport_4K30FPS;
        pstCommConfig->pstRecVencAttr = &g_stVenc_4K30FPS_VIDEO_H264;
        pstCommConfig->pstSnapVencAttr = &g_stVenc_4K30FPS_JPEG;
    }

    else if (enSensorSeq == SAMPLE_SENSOR_4M30) {
        pstCommConfig->pstVcapDevAttr = &g_stDev_4M;
        pstCommConfig->pstVcapPipeAttr = &g_stPipe_4M30FPS;
        pstCommConfig->pstPipeChnAttr = &g_stChn_4M30FPS;
        pstCommConfig->pstVpssAttr = &g_stVpss_4M30FPS;
        pstCommConfig->pstVPortAttr = &g_stVport_4M30FPS;
        pstCommConfig->pstRecVencAttr = &g_stVenc_1080P30FPS_VIDEO_H265;
        pstCommConfig->pstSnapVencAttr = &g_stVenc_1080P30FPS_JPEG;
    }

    else if (enSensorSeq == SAMPLE_SENSOR_1080P30) {
        pstCommConfig->pstVcapDevAttr = &g_stDev_1080P;
        pstCommConfig->pstVcapPipeAttr = &g_stPipe_1080P30FPS;
        pstCommConfig->pstPipeChnAttr = &g_stChn_1080P30FPS;
        pstCommConfig->pstVpssAttr = &g_stVpss_1080P30FPS;
        pstCommConfig->pstVPortAttr = &g_stVport_1080P30FPS;
        pstCommConfig->pstRecVencAttr = &g_stVenc_1080P30FPS_VIDEO_H265;
        pstCommConfig->pstSnapVencAttr = &g_stVenc_1080P30FPS_JPEG;
    } else {
        pstCommConfig->pstVcapDevAttr = &g_stDev_1080P;
        pstCommConfig->pstVcapPipeAttr = &g_stPipe_1080P30FPS;
        pstCommConfig->pstPipeChnAttr = &g_stChn_1080P30FPS;
        pstCommConfig->pstVpssAttr = &g_stVpss_1080P30FPS;
        pstCommConfig->pstVPortAttr = &g_stVport_1080P30FPS;
        pstCommConfig->pstRecVencAttr = &g_stVenc_1080P30FPS_VIDEO_H265;
        pstCommConfig->pstSnapVencAttr = &g_stVenc_1080P30FPS_JPEG;
        printf("other MediaCFG to be add later, default use  1080P30 Mode\n");
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_VENC_Get_Media_Mode(SAMPLE_SENSOR_SEQ_E *penSensorSeq,
    HI_MAPI_SENSOR_ATTR_S *pstVcapSensorAttr)
{
    char ch;
    HI_S32 s32Ret = HI_SUCCESS;

    // IMX307 only support 1080P
    if (CFG_SENSOR_TYPE0 == IMX307) {
        ch = '1';
    }
    // IMX335 only support 4M
    else if (CFG_SENSOR_TYPE0 == IMX335) {
        ch = '2';
    } else {
        printf("Usage : \n");
        printf("\t 0) media cfg :4K@30fps  + 1080p HDMI Preview.\n");
        printf("\t 1) media cfg :1080p@30fps + 1080p HDMI Preview.\n");
        ch = (char)getchar();
        (void)getchar();
    }

    switch (ch) {
        case '0':
            *penSensorSeq = SAMPLE_SENSOR_4K30;
            s32Ret = SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_4K30, pstVcapSensorAttr);
            if (s32Ret != HI_SUCCESS) {
                printf("SAMPLE_COMM_GetSensorCfg error!\n");
                return HI_FAILURE;
            }
            break;

        case '1':
            *penSensorSeq = SAMPLE_SENSOR_1080P30;
            s32Ret = SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_1080P30, pstVcapSensorAttr);
            if (s32Ret != HI_SUCCESS) {
                printf("SAMPLE_COMM_GetSensorCfg error!\n");
                return HI_FAILURE;
            }
            break;

        case '2':
            *penSensorSeq = SAMPLE_SENSOR_4M30;
            s32Ret = SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_4M30, pstVcapSensorAttr);
            if (s32Ret != HI_SUCCESS) {
                printf("SAMPLE_COMM_GetSensorCfg error!\n");
                return HI_FAILURE;
            }
            break;

        default:
            printf("the index is invaild!\n");
            printf("Usage : \n");
            printf("\t 0) media cfg :4K@30fps + 1080p HDMI Preview.\n");
            printf("\t 1) media cfg :1080p@30fps + 1080p HDMI Preview.\n");
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 YUV_DataProc(HI_HANDLE VProcHdl, HI_HANDLE VPortHdl, HI_MAPI_FRAME_DATA_S *pVPortYUV,
    HI_VOID *pPrivateData)
{
    HI_S32 s32Ret;
    s32Ret = SAMPLE_COMM_SaveYuvFile(VProcHdl, VPortHdl, pVPortYUV, pPrivateData);
    if (s32Ret != HI_SUCCESS) {
        printf("SAMPLE_COMM_SaveYuvFile error s32Ret:%d\n", s32Ret);
    }

    return HI_SUCCESS;
}

static HI_S32 Snap_ConvertBitPixel(HI_U8 *pu8Data, HI_U32 u32DataNum, PIXEL_FORMAT_E enPixelFormat,
                                   HI_U16 *pu16OutData)
{
    HI_S32 i, u32Tmp, s32OutCnt;
    HI_U32 u32Val;
    HI_U64 u64Val;
    HI_U8 *pu8Tmp = pu8Data;

    s32OutCnt = 0;

    switch (enPixelFormat) {
        case PIXEL_FORMAT_RGB_BAYER_10BPP: {
            /* 4 pixels consist of 5 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++) {
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

        case PIXEL_FORMAT_RGB_BAYER_12BPP: {
            /* 2 pixels consist of 3 bytes  */
            u32Tmp = u32DataNum / 2;

            for (i = 0; i < u32Tmp; i++) {
                /* byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 3 * i;
                u32Val = pu8Tmp[0] + (pu8Tmp[1] << 8) + (pu8Tmp[2] << 16);
                pu16OutData[s32OutCnt++] = u32Val & 0xfff;
                pu16OutData[s32OutCnt++] = (u32Val >> 12) & 0xfff;
            }
        }
        break;

        case PIXEL_FORMAT_RGB_BAYER_14BPP: {
            /* 4 pixels consist of 7 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++) {
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
            fprintf(stderr, "unsuport enPixelFormat: %d\n", enPixelFormat);
            return -1;
            break;
    }

    return s32OutCnt;
}

static HI_S32 PixelFormatToNumber(PIXEL_FORMAT_E enPixelFormat)
{
    switch (enPixelFormat) {
        case PIXEL_FORMAT_RGB_BAYER_8BPP:
            return 8;

        case PIXEL_FORMAT_RGB_BAYER_10BPP:
            return 10;

        case PIXEL_FORMAT_RGB_BAYER_12BPP:
            return 12;

        case PIXEL_FORMAT_RGB_BAYER_14BPP:
            return 14;

        case PIXEL_FORMAT_RGB_BAYER_16BPP:
            return 16;

        default:
            return 0;
    }
}

static int RawData_Save(HI_MAPI_FRAME_DATA_S *pVBuf, FILE *pfd, HI_U32 u32FrameCnt)
{
    HI_U32 u32H = 0;
    HI_S32 i = 0;
    HI_U16 *apu16Data[HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM] = {
        NULL,
    };
    HI_U64 u64PhyAddr = 0;
    HI_U32 au32size[HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM] = {
        0,
    };
    HI_U8 *apUserPageAddr[HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM] = {
        HI_NULL,
    };
    HI_U8 *pu8Data = HI_NULL;
    HI_S32 s32Ret = HI_SUCCESS;

    if (u32FrameCnt > HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM) {
        fprintf(stderr, "u32FrameCnt max is :%d\n", HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM);
        return HI_FAILURE;
    }

    for (i = 0; i < u32FrameCnt; i++) {
        apUserPageAddr[i] = (HI_U8 *)(HI_UL)(pVBuf + i)->u64VirAddr[0];

        if (apUserPageAddr[i] == NULL) {
            fprintf(stderr, "alloc memory failed\n");
            s32Ret = HI_FAILURE;
            goto exit;
        }

        pu8Data = apUserPageAddr[i];
        if ((PIXEL_FORMAT_RGB_BAYER_8BPP != (pVBuf + i)->enPixelFormat) &&
            (PIXEL_FORMAT_RGB_BAYER_16BPP != (pVBuf + i)->enPixelFormat)) {
            if ((pVBuf + i)->u32Width > 0) {
                SAMPLE_CHECK_U32PRODUCT_OVERFLOW((pVBuf + i)->u32Width, 2);
                apu16Data[i] = (HI_U16 *)malloc((pVBuf + i)->u32Width * 2);
                if (apu16Data[i] == NULL) {
                    fprintf(stderr, "alloc memory failed\n");
                    s32Ret = HI_FAILURE;
                    goto exit;
                }
                memset(apu16Data[i], 0, (pVBuf + i)->u32Width * 2);
            } else {
                printf("error u32Width :0!\n");
                s32Ret = HI_FAILURE;
                goto exit;
            }
        }

        fprintf(stderr, "saving(UNCMP)......dump data......u32Stride[0]: %d, width: %d\n", (pVBuf + i)->u32Stride[0],
                (pVBuf + i)->u32Width);

        for (u32H = 0; u32H < (pVBuf + i)->u32Height; u32H++) {
            if (PIXEL_FORMAT_RGB_BAYER_8BPP == (pVBuf + i)->enPixelFormat) {
                fwrite(pu8Data, (pVBuf + i)->u32Width, 1, pfd);
                fflush(pfd);
            } else if (PIXEL_FORMAT_RGB_BAYER_16BPP == (pVBuf + i)->enPixelFormat) {
                fwrite(pu8Data, (pVBuf + i)->u32Width, 2, pfd);
                fflush(pfd);
            } else {
                Snap_ConvertBitPixel(pu8Data, (pVBuf + i)->u32Width, (pVBuf + i)->enPixelFormat, apu16Data[i]);
                fwrite(apu16Data[i], (pVBuf + i)->u32Width, 2, pfd);
            }
            pu8Data += (pVBuf + i)->u32Stride[0];
        }

        fflush(pfd);
        printf("\n************saving(UNCMP)...No%d...done!****************************\n", i);
    }
exit:
    for (i = 0; i < u32FrameCnt; i++) {
        if (apu16Data[i] != HI_NULL) {
            free(apu16Data[i]);
            apu16Data[i] = HI_NULL;
        }
    }

    return s32Ret;
}

static int SaveCompressParam(HI_HANDLE VcapPipeHdl, FILE *pfd)
{
    VI_CMP_PARAM_S stCmpParam = {
        0,
    };
    HI_S32 s32Ret;

    s32Ret = HI_MAPI_VCAP_GetAttrEx(VcapPipeHdl, 0, HI_MAPI_VCAP_CMD_PIPE_GetCmpParam, &stCmpParam,
                                    sizeof(VI_CMP_PARAM_S));

    fwrite(&stCmpParam, sizeof(VI_CMP_PARAM_S), 1, pfd);
    fflush(pfd);
    return s32Ret;
}

static int RawData_Save_Compress(HI_MAPI_FRAME_DATA_S *pVBuf, FILE *pfd, HI_U32 u32FrameCnt)
{
    HI_U32 u32H = 0;
    HI_S32 i = 0;
    HI_U16 *apu16Data[HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM] = { NULL };
    HI_U64 u64PhyAddr = 0;
    HI_U32 au32size[HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM] = {0};
    HI_U8 *apUserPageAddr[HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM] = { HI_NULL };
    HI_U8 *pu8Data = HI_NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32DataSize;
    HI_U16 u16HeadData = 0x0;

    if (u32FrameCnt > HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM) {
        fprintf(stderr, "u32FrameCnt max is :%d\n", HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM);
        return HI_FAILURE;
    }

    for (i = 0; i < u32FrameCnt; i++) {
        fprintf(stderr, "saving(CMP)......dump data......u32Stride[0]: %d, width: %d\n", (pVBuf + i)->u32Stride[0],
                (pVBuf + i)->u32Width);
        fflush(stderr);

        apUserPageAddr[i] = (HI_U8 *)(HI_UL)(pVBuf + i)->u64VirAddr[0];
        pu8Data = apUserPageAddr[i];

        for (u32H = 0; u32H < (pVBuf + i)->u32Height; u32H++) {
            u16HeadData = *(HI_U16 *)pu8Data;
            u32DataSize = (u16HeadData + 1) * 16;

            fwrite(pu8Data, u32DataSize, 1, pfd);
            pu8Data += (pVBuf + i)->u32Stride[0];
        }

        fflush(pfd);
        printf("\n************saving(CMP)...No%d...done!****************************\n", i);
        fflush(stderr);

        apUserPageAddr[i] = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 RAW_DataProc(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pVCapRawData, HI_S32 s32DataNum,
                    HI_VOID *pPrivateData)
{
    FILE *pfd = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szRawDataName[128] = {0};
    JPEG_DCF_S *pstJpegDCFInfo;
    DNG_IMAGE_DYNAMIC_INFO_S *pstDngDynamicInfo;
    HI_S32 s32PixBit;

    s32PixBit = PixelFormatToNumber(pVCapRawData->enPixelFormat);

    printf("++nbit:%d\n", s32PixBit);
    printf("begin dump...\n");

    snprintf(szRawDataName, 128, "./%s_vcap%d_%d_%d_%dbit_%d.raw", g_aszDumpFrameName[VcapPipeHdl], VcapPipeHdl,
             pVCapRawData->u32Width, pVCapRawData->u32Height, s32PixBit, g_as32RawFrameCnt[VcapPipeHdl]++);

    pfd = fopen(szRawDataName, "wb");

    if (pfd != NULL) {
        if (pVCapRawData->enCompressMode == COMPRESS_MODE_NONE) {
            s32Ret = RawData_Save(pVCapRawData, pfd, s32DataNum);

            if (s32Ret != HI_SUCCESS) {
                fclose(pfd);
                printf("RawData_Save fail\n");
                return HI_FAILURE;
            }
        } else {
            s32Ret = SaveCompressParam(VcapPipeHdl, pfd);
            if (s32Ret != HI_SUCCESS) {
                fclose(pfd);
                printf("SaveCompressParam fail\n");
                return HI_FAILURE;
            }

            s32Ret = RawData_Save_Compress(pVCapRawData, pfd, s32DataNum);

            if (s32Ret != HI_SUCCESS) {
                fclose(pfd);
                printf("RawData_Save_Compress fail\n");
                return HI_FAILURE;
            }
        }

        fclose(pfd);
    } else {
        printf("open file fail\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 Fastboot_Start_Audio_Aenc(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    /* start hdmi ao */
    HI_HANDLE hAcapHdl = TEST_AI_DEV;
    HI_HANDLE hAcapChnHdl = SAMPLE_MAPI_AI_CHN;
    HI_HANDLE hAencHdl = TEST_AENC_CHN;
    HI_MAPI_ACAP_ATTR_S stACapAttr;
    HI_S32 s32InGain;

    /* init and start acap */
    stACapAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    stACapAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    stACapAttr.enSampleRate = AUDIO_SAMPLE_RATE_48000;
    stACapAttr.enSoundMode = HI_MAPI_AUDIO_SOUND_MODE_STEREO;
    stACapAttr.u32PtNumPerFrm = 1024;
    stACapAttr.enMixerMicMode = ACODEC_MIXER_IN0;
    stACapAttr.enResampleRate = AUDIO_SAMPLE_RATE_48000;
    stACapAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
    s32InGain = 20;

    SAMPLE_CHECK_RET(HI_MAPI_ACAP_Init(hAcapHdl, &stACapAttr));
    SAMPLE_CHECK_RET(HI_MAPI_ACAP_Start(hAcapHdl, hAcapChnHdl));
    SAMPLE_CHECK_RET(HI_MAPI_ACAP_SetVolume(hAcapHdl, s32InGain));
    /* init and start aenc */
    SAMPLE_CHECK_RET(HI_MAPI_AENC_RegisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_AACLC));

    HI_MAPI_AENC_ATTR_S stAencAttr = {0};
    HI_MAPI_AENC_ATTR_AAC_S stAacAencAttr = {0};

    stAacAencAttr.enAACType = AAC_TYPE_AACLC;
    stAacAencAttr.enBitRate = AAC_BPS_48K;
    stAacAencAttr.enBitWidth = AUDIO_BIT_WIDTH_16;
    stAacAencAttr.enSmpRate = AUDIO_SAMPLE_RATE_48000;
    stAacAencAttr.enSoundMode = AUDIO_SOUND_MODE_STEREO;
    stAacAencAttr.enTransType = AAC_TRANS_TYPE_ADTS;
    stAacAencAttr.s16BandWidth = 0;

    stAencAttr.enAencFormat = HI_MAPI_AUDIO_FORMAT_AACLC;
    stAencAttr.u32PtNumPerFrm = 1024;
    stAencAttr.pValue = &stAacAencAttr;
    stAencAttr.u32Len = sizeof(HI_MAPI_AENC_ATTR_AAC_S);

    SAMPLE_CHECK_RET(HI_MAPI_AENC_Init(hAencHdl, &stAencAttr));
    SAMPLE_CHECK_RET(HI_MAPI_AENC_Start(hAencHdl));
    printf("init and start aenc complete\n");

    /* aenc bind acap */
    SAMPLE_CHECK_RET(HI_MAPI_AENC_BindACap(hAcapHdl, hAcapChnHdl, hAencHdl));
    return HI_SUCCESS;
}

HI_S32 Fastboot_Start_Vcap(HI_HANDLE VcapDev, HI_HANDLE VcapPipe, HI_HANDLE PipeChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    HI_U8 u8SnsCnt = 1;

    SAMPLE_CHECK_RET(SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_4K30, &stVcapSensorAttr));

    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = VcapPipe;

    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[VcapPipe].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe].astPipeChnAttr[PipeChn], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[VcapPipe] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = u8SnsCnt;
    SAMPLE_CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));

    SAMPLE_CHECK_RET(HI_MAPI_VCAP_InitSensor(VcapDev, &stVcapSensorAttr));
    SAMPLE_CHECK_RET(HI_MAPI_VCAP_SetAttr(VcapDev, &stVcapAttr));
    SAMPLE_CHECK_RET(HI_MAPI_VCAP_StartDev(VcapDev));
    SAMPLE_CHECK_RET(HI_MAPI_VCAP_StartChn(VcapPipe, PipeChn));
    SAMPLE_CHECK_RET(HI_MAPI_VCAP_StartISP(VcapPipe));

    return HI_SUCCESS;
}

HI_S32 Fastboot_Start_DispAndHdmi(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE HdmiHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_DISP_ATTR_S stDispAttr;
    HI_MAPI_DISP_VIDEOLAYER_ATTR_S stVideoLayerAttr;
    memcpy(&stDispAttr.stPubAttr, &g_stDisp_HDMI_1080P30, sizeof(VO_PUB_ATTR_S));
    stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_720P60;
    memset(&stVideoLayerAttr, 0, sizeof(HI_MAPI_DISP_VIDEOLAYER_ATTR_S));
    SAMPLE_CHECK_RET(HI_MAPI_DISP_Init(DispHdl, &stDispAttr));
    SAMPLE_CHECK_RET(HI_MAPI_DISP_Start(DispHdl, &stVideoLayerAttr));

    HI_MAPI_DISP_WINDOW_ATTR_S stWndAttr;
    memcpy(&stWndAttr, &g_stWnd_1920x1080, sizeof(HI_MAPI_DISP_WINDOW_ATTR_S));
    stWndAttr.stRect.u32Width = 1280;
    stWndAttr.stRect.u32Height = 720;
    SAMPLE_CHECK_RET(HI_MAPI_DISP_SetWindowAttr(DispHdl, WndHdl, &stWndAttr));
    SAMPLE_CHECK_RET(HI_MAPI_DISP_StartWindow(DispHdl, WndHdl));
    SAMPLE_CHECK_RET(SAMPLE_COMM_FastBootStartHdmi(HdmiHdl, &stDispAttr));

    return HI_SUCCESS;
}

// audio + vcap + vproc + venc + disp + hdmi
HI_S32 SAMPLE_COMM_FASTBOOT_ALL(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE hVcapDev0 = 0;
    HI_HANDLE hVcapPipe0 = 0;
    HI_HANDLE hPipeChn0 = 0;

    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVportHdl0 = 0;
    HI_HANDLE hVportHdl1 = 1;
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    HI_HANDLE hVencHdl0 = 0;
    HI_HANDLE hVencHdl1 = 1;
    HI_S32 s32FrameCnt = -1;
    HI_MAPI_VENC_ATTR_S stVencAttr;

    HI_HANDLE hDispHdl0 = 0;
    HI_HANDLE hWndHdl0 = 0;
    HI_HANDLE hHdmiHdl0 = 0;

    printf("\n----4K@30fps H.265 record + 720p@30fps H.264 record----\n");

    /**************************start vcap *******************************/
    SAMPLE_CHECK_RET(Fastboot_Start_Vcap(hVcapDev0, hVcapPipe0, hPipeChn0));

    /**************************start audio****************************/
    SAMPLE_CHECK_RET(Fastboot_Start_Audio_Aenc());

    /**************************start vproc *******************************/
    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));

    SAMPLE_CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    SAMPLE_CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    SAMPLE_CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    SAMPLE_CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));
    SAMPLE_CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl1, &stVPortAttr));
    SAMPLE_CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl1));

    /**************************start venc *******************************/
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H265, sizeof(HI_MAPI_VENC_ATTR_S));
    SAMPLE_CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    SAMPLE_CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, bStitch));
    SAMPLE_CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, s32FrameCnt));

    memcpy(&stVencAttr, &g_stVenc_720P30FPS_H264, sizeof(HI_MAPI_VENC_ATTR_S));
    SAMPLE_CHECK_RET(HI_MAPI_VENC_Init(hVencHdl1, &stVencAttr));
    SAMPLE_CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl1, hVencHdl1, bStitch));
    SAMPLE_CHECK_RET(HI_MAPI_VENC_Start(hVencHdl1, s32FrameCnt));

    /**************************start disp *******************************/
    SAMPLE_CHECK_RET(Fastboot_Start_DispAndHdmi(hDispHdl0, hWndHdl0, hHdmiHdl0));
    SAMPLE_CHECK_RET(HI_MAPI_DISP_Bind_VProc(hVpssHdl0, hVportHdl0, hDispHdl0, hWndHdl0, bStitch));

    printf("media start ok.\n");
    return HI_SUCCESS;
}

// audio + vcap + venc + disp + hdmi
HI_S32 SAMPLE_COMM_FASTBOOT_NO_VPROC(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE hVcapDev0 = 0;
    HI_HANDLE hVcapPipe0 = 0;
    HI_HANDLE hPipeChn0 = 0;

    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVportHdl0 = 0;
    HI_HANDLE hVportHdl1 = 1;
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    HI_HANDLE hVencHdl0 = 0;
    HI_HANDLE hVencHdl1 = 1;
    HI_S32 s32FrameCnt = -1;
    HI_MAPI_VENC_ATTR_S stVencAttr;

    HI_HANDLE hDispHdl0 = 0;
    HI_HANDLE hWndHdl0 = 0;
    HI_HANDLE hHdmiHdl0 = 0;

    printf("\n----4K@30fps H.265 record \n");

    /**************************start vcap *******************************/
    SAMPLE_CHECK_RET(Fastboot_Start_Vcap(hVcapDev0, hVcapPipe0, hPipeChn0));

    /**************************start audio****************************/
    SAMPLE_CHECK_RET(Fastboot_Start_Audio_Aenc());

    /**************************start venc *******************************/
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H264, sizeof(HI_MAPI_VENC_ATTR_S));
    SAMPLE_CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    SAMPLE_CHECK_RET(HI_MAPI_VENC_BindVCap(hVcapPipe0, hPipeChn0, hVencHdl0));
    SAMPLE_CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, s32FrameCnt));

    /**************************start disp *******************************/
    SAMPLE_CHECK_RET(Fastboot_Start_DispAndHdmi(hDispHdl0, hWndHdl0, hHdmiHdl0));
    SAMPLE_CHECK_RET(HI_MAPI_DISP_Bind_VCap(hVcapPipe0, hPipeChn0, hDispHdl0, hWndHdl0));

    printf("media start ok.\n");
    return HI_SUCCESS;
}

HI_VOID Fastboot_Test_Usage(HI_VOID)
{
    printf("\n----1.audio + vcap + vproc + venc + disp + hdmi .\n");
    printf("\n----2.audio + vcap + venc + disp + hdmi.\n");
    printf("\n----pleale input:\n");
    return;
}

HI_VOID Fastboot_Sample(HI_VOID)
{
    HI_CHAR cmd[10] = {0};
    Fastboot_Test_Usage();
    fgets(cmd, 10, stdin);
    cmd[9] = '\0';

    if (strncmp(cmd, "1", 1) == 0) {
        (void)SAMPLE_COMM_FASTBOOT_ALL();  // audio + vcap + vproc + venc + disp + hdmi
    } else if (strncmp(cmd, "2", 1) == 0) {
        (void)SAMPLE_COMM_FASTBOOT_NO_VPROC();  // audio + vcap + venc + disp + hdmi
    } else {
        printf("the index is invaild!\n");
        Fastboot_Test_Usage();
    }
}

HI_VOID Liteos_Test(HI_VOID)
{
    printf("\n----sample not supprot  Liteos_Test, please goto test directory.\n");
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

