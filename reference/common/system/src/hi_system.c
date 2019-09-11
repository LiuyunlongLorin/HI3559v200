/**
 * @file      hi_system.c
 * @brief     system module interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/4/10
 * @version   1.0

 */
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#ifdef __LINUX__
#include <sys/reboot.h>
#include "linux/rtc.h"
#endif
#ifdef __HuaweiLite__
#include "hi_rtc.h"
#endif
#include "hi_math.h"
#include "mpi_vb.h"
#include "mpi_audio.h"
#include "hi_mapi_disp.h"
#include "hi_mapi_ao.h"

#include "hi_appcomm_util.h"
#ifdef AMP_LINUX_HUAWEILITE
#ifdef __LINUX__
#include "hi_appcomm_msg_client.h"
#else
#include "hi_appcomm_msg_server.h"
#endif
#endif
#include "hi_system.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "SYSTEM"

#if defined(__LINUX__)
#define DEFAULT_RTC_DEVICE "/dev/rtc0"
#endif
#if defined(__HuaweiLite__)
#define DEFAULT_RTC_DEVICE "/dev/hi_rtc"
#endif

#define MSG_SYSTEM_SET_DATETIME                HI_APPCOMM_MSG_ID(HI_APP_MOD_SYSTEM, 0)
#define MSG_SYSTEM_GET_SHAREFS_CONNECTED_STATE HI_APPCOMM_MSG_ID(HI_APP_MOD_SYSTEM, 1)


#if (defined(AMP_LINUX_HUAWEILITE)&&defined(__HuaweiLite__))||defined(AMP_HUAWEILITE)

#ifdef CFG_BOOT_SOUND_ON
typedef struct tagSYSTEM_SOUND_S
{
    HI_HANDLE AoHdl;
    HI_U8* pu8PCMData;
    HI_U32 u32DataLen;
} SYSTEM_SOUND_S;


static HI_VOID SYSTEM_PlayAudio(HI_HANDLE AoHdl, HI_U8* pu8PCMData, HI_U32 u32DataLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32FrameCnt = 0;
    AIO_ATTR_S stAttr;
    s32Ret = HI_MPI_AO_GetPubAttr(AoHdl, &stAttr);

    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_MPI_AO_GetPubAttr, s32Ret);
        return;
    }

    HI_U32 u32FrameLen = stAttr.u32PtNumPerFrm * sizeof(HI_U16);//1024 PoiNum
    AUDIO_FRAME_S stAoFrame;
    memset(&stAoFrame, 0, sizeof(AUDIO_FRAME_S));
    stAoFrame.enBitwidth = stAttr.enBitwidth;//AUDIO_BIT_WIDTH_16;
    stAoFrame.enSoundmode = stAttr.enSoundmode;//AUDIO_SOUND_MODE_STEREO;
    stAoFrame.u32Len = u32FrameLen;

    while (u32FrameCnt < (u32DataLen / u32FrameLen))
    {
        stAoFrame.u32Seq++;
        stAoFrame.u64VirAddr[0] = pu8PCMData;
        stAoFrame.u64PhyAddr[0] = (HI_U64)(HI_UL)pu8PCMData;

        if (AUDIO_SOUND_MODE_STEREO == stAoFrame.enSoundmode)
        {
            stAoFrame.u64VirAddr[1] = pu8PCMData;
            stAoFrame.u64PhyAddr[1] = (HI_U64)(HI_UL)pu8PCMData;
        }

        s32Ret = HI_MPI_AO_SendFrame(AoHdl, 0, (const AUDIO_FRAME_S*)&stAoFrame, 50);

        if (HI_SUCCESS != s32Ret)
        {
            HI_LOG_PrintFuncErr(HI_MPI_AO_SendFrame, s32Ret);

            if (HI_ERR_AO_NOT_ENABLED == s32Ret)
            {
                break;
            }

            HI_usleep(10 * 1000);
            continue;
        }

        u32FrameCnt++;
        pu8PCMData += u32FrameLen;
        HI_usleep(10 * 1000);
    }
}

static HI_VOID* SYSTEM_BootSound(HI_VOID* pvarg)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, __func__, 0, 0, 0);
    SYSTEM_SOUND_S* pstSound = (SYSTEM_SOUND_S*)pvarg;
    SYSTEM_PlayAudio(pstSound->AoHdl, pstSound->pu8PCMData, pstSound->u32DataLen);
    return NULL;
}

HI_S32 HI_SYSTEM_BootSound(HI_HANDLE AoHdl, const HI_U8* pu8PCMData, HI_U32 u32DataLen)
{
    pthread_t pid;
    static SYSTEM_SOUND_S stSound;
    stSound.AoHdl = AoHdl;
    stSound.pu8PCMData = (HI_U8*)pu8PCMData;
    stSound.u32DataLen = u32DataLen;
    return pthread_create(&pid, NULL, SYSTEM_BootSound, &stSound);
}
#endif

#ifdef CFG_BOOT_LOGO_ON
static HI_S32 SYSTEM_ReadYUVFrame( HI_U8* pSrc,
                                   HI_U8* pY,
                                   HI_U8* pU,
                                   HI_U8* pV,
                                   HI_U32 width,
                                   HI_U32 height,
                                   HI_U32 stride,
                                   HI_U32 stride2,
                                   PIXEL_FORMAT_E enPixFrm)
{
    HI_U8* pDst = HI_NULL;
    HI_U8* pTmpSrc = HI_NULL;
    HI_U32 u32UVHeight;
    HI_U32 u32Row;

    pTmpSrc = pSrc;

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == enPixFrm)
    {
        u32UVHeight = height;
    }
    else
    {
        u32UVHeight = height / 2;
    }

    pDst = pY;

    for ( u32Row = 0; u32Row < height; u32Row++ )
    {
        memcpy(pDst, pTmpSrc, width);
        pTmpSrc += width;
        pDst += stride;
    }

    pDst = pU;

    for ( u32Row = 0; u32Row < u32UVHeight; u32Row++ )
    {
        memcpy(pDst, pTmpSrc, width / 2);
        pTmpSrc += (width / 2);
        pDst += stride2;
    }

    pDst = pV;

    for ( u32Row = 0; u32Row < u32UVHeight; u32Row++ )
    {
        memcpy(pDst, pTmpSrc, width / 2);
        pTmpSrc += (width / 2);
        pDst += stride2;
    }

    return HI_SUCCESS;
}

HI_S32 HI_SYSTEM_BootLogo(HI_HANDLE DispHdl, HI_HANDLE WndHdl, SIZE_S stRes, const HI_U8* pu8YUVData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32LumaSize = 0;
    HI_U32 u32ChromaSize = 0;
    HI_U32 u32SrcWidth = stRes.u32Width, u32SrcHeight = stRes.u32Height;
    VB_BLK hBlkHdl = VB_INVALID_HANDLE;
    HI_U64 u64BlkSize = u32SrcWidth * u32SrcHeight * 2;

    hBlkHdl = HI_MPI_VB_GetBlock(0, u64BlkSize, NULL);

    if (VB_INVALID_HANDLE == hBlkHdl)
    {
        HI_LOG_PrintFuncErr(HI_MPI_VB_GetBlock, hBlkHdl);
        return HI_FAILURE;
    }

    HI_MAPI_FRAME_DATA_S stFramedata;
    stFramedata.enFrameDataType = HI_FRAME_DATA_TYPE_YUV;
    stFramedata.u32Width = u32SrcWidth;
    stFramedata.u32Height = u32SrcHeight;
    stFramedata.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stFramedata.enCompressMode = COMPRESS_MODE_NONE;
    stFramedata.u32Stride[0]   = ALIGN_DOWN(u32SrcWidth, 16);
    stFramedata.u32Stride[1]   = ALIGN_DOWN(u32SrcWidth, 16);
    stFramedata.u32Stride[2]   = ALIGN_DOWN(u32SrcWidth, 16);
    u32LumaSize =  stFramedata.u32Stride[0] * u32SrcHeight;
    u32ChromaSize =  stFramedata.u32Stride[0] * u32SrcHeight >> 2;
    stFramedata.u64PhyAddr[0] = HI_MPI_VB_Handle2PhysAddr(hBlkHdl);
    stFramedata.u64PhyAddr[1] = stFramedata.u64PhyAddr[0] + u32LumaSize;
    stFramedata.u64PhyAddr[2] = stFramedata.u64PhyAddr[1] + u32ChromaSize;
    stFramedata.u64VirAddr[0] = stFramedata.u64PhyAddr[0];
    stFramedata.u64VirAddr[1] = stFramedata.u64PhyAddr[1];
    stFramedata.u64VirAddr[2] = stFramedata.u64PhyAddr[2];
    stFramedata.u64pts = 0;
    stFramedata.u32PoolId = HI_MPI_VB_Handle2PoolId(hBlkHdl);

    /*read vo display picture, nv21 format*/
    SYSTEM_ReadYUVFrame((HI_U8*)pu8YUVData,
                        (HI_U8*)(HI_UL)stFramedata.u64VirAddr[0],
                        (HI_U8*)(HI_UL)stFramedata.u64VirAddr[1],
                        (HI_U8*)(HI_UL)stFramedata.u64VirAddr[2],
                        stFramedata.u32Width,
                        stFramedata.u32Height,
                        stFramedata.u32Stride[0],
                        stFramedata.u32Stride[0] >> 1,
                        stFramedata.enPixelFormat);

    s32Ret = HI_MAPI_DISP_SendFrame(DispHdl, WndHdl, &stFramedata);

    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_MAPI_DISP_SendFrame, s32Ret);
    }

    s32Ret = HI_MPI_VB_ReleaseBlock(hBlkHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}
#endif
#endif

#if (defined(AMP_LINUX_HUAWEILITE)&&defined(__HuaweiLite__))
static HI_S32 MSG_SYSTEM_SetDateTime(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_SYSTEM_TM_S* pstDateTime = (HI_SYSTEM_TM_S*) pvRequest;
    return HI_SYSTEM_SetDateTime(pstDateTime);
}

static HI_S32 MSG_SYSTEM_GetSharefsConnectedState(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32* ps32ConnectedState = (HI_S32*)malloc(sizeof(HI_S32));
    if (HI_NULL == ps32ConnectedState)
    {
        MLOGE("malloc failed.\n");
        return HI_FAILURE;
    }
    *ppvResponse = ps32ConnectedState;
    *pu32RespLen = sizeof(HI_S32);
    extern int sfs_ipc_connected(void);
    *ps32ConnectedState = sfs_ipc_connected();
    return HI_SUCCESS;
}


HI_S32 HI_SYSTEM_ServiceInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret |= HI_MSG_RegisterMsgProc(MSG_SYSTEM_SET_DATETIME, MSG_SYSTEM_SetDateTime, NULL);
    s32Ret |= HI_MSG_RegisterMsgProc(MSG_SYSTEM_GET_SHAREFS_CONNECTED_STATE, MSG_SYSTEM_GetSharefsConnectedState, NULL);
    return s32Ret;
}
#endif

#if (defined(AMP_LINUX_HUAWEILITE)&& !defined(__HuaweiLite__))

#define SYSTEM_INTERVAL_TIME_US  (10*1000)  /**< unit:us */
HI_BOOL HI_SYSTEM_WaitSharefsReady(HI_S32 s32Timeout_s)
{
    HI_APPCOMM_CHECK_EXPR(s32Timeout_s > 0, HI_EINVAL);
    HI_S32 s32LastTime = 0;
    HI_S32 s32ConnectedState = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    while(1)
    {
        s32Ret = HI_MSG_SendSync(MSG_SYSTEM_GET_SHAREFS_CONNECTED_STATE, NULL, 0,
            HI_NULL, &s32ConnectedState, sizeof(HI_S32));
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MSG_SendSync error.%d\n", s32Ret);
            return HI_FALSE;
        }

        if(s32ConnectedState == 1)
        {
            break;
        }
        else
        {
            HI_usleep(SYSTEM_INTERVAL_TIME_US);
            s32LastTime += SYSTEM_INTERVAL_TIME_US;
        }

        if(s32LastTime >= s32Timeout_s*1000*1000)
        {
            MLOGE("Sharefs Connect TimeOut!\n");
            return HI_FALSE;
        }

    }
    return HI_TRUE;
}
#endif


HI_S32 HI_SYSTEM_SetDateTime(const HI_SYSTEM_TM_S* pstDateTime)
{
    HI_APPCOMM_CHECK_POINTER(pstDateTime, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(1970 < pstDateTime->s32year, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(pstDateTime->s32mon, 1, 12), HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(pstDateTime->s32mday, 1, 31), HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(pstDateTime->s32hour, 0, 23), HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(pstDateTime->s32min, 0, 59), HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(pstDateTime->s32sec, 0, 59), HI_EINVAL);
    /*1.set systime*/
    time_t t = 0L;
    struct tm stTime;
    memset(&stTime, 0, sizeof(struct tm));
    stTime.tm_year = pstDateTime->s32year - 1900;
    stTime.tm_mon = pstDateTime->s32mon - 1;
    stTime.tm_mday = pstDateTime->s32mday;
    stTime.tm_hour = pstDateTime->s32hour;
    stTime.tm_min = pstDateTime->s32min;
    stTime.tm_sec = pstDateTime->s32sec;
    t = mktime(&stTime);
    struct timeval stNowTime = {t, 0};
    settimeofday(&stNowTime, NULL);

    /*2.set rtc time*/
    HI_S32 s32Ret = -1;
    HI_S32 s32fd_rtc = -1;
    s32fd_rtc = open(DEFAULT_RTC_DEVICE, O_RDWR);

    if (s32fd_rtc < 0)
    {
        MLOGE("open %s error:%s\n", DEFAULT_RTC_DEVICE, strerror(errno));
    }
    else
    {
        localtime_r(&t, &stTime);
#if defined(__LINUX__)
        struct rtc_time rtctm;
        rtctm.tm_year = stTime.tm_year;
        rtctm.tm_mon = stTime.tm_mon;
        rtctm.tm_mday = stTime.tm_mday;
        rtctm.tm_hour = stTime.tm_hour;
        rtctm.tm_min = stTime.tm_min;
        rtctm.tm_sec = stTime.tm_sec;
        rtctm.tm_wday = stTime.tm_wday;
        s32Ret = ioctl(s32fd_rtc, RTC_SET_TIME, &rtctm);
#elif  defined(__HuaweiLite__)
        rtc_time_t rtctm;
        rtctm.year = pstDateTime->s32year;
        rtctm.month = pstDateTime->s32mon;
        rtctm.date = pstDateTime->s32mday;
        rtctm.hour = pstDateTime->s32hour;
        rtctm.minute = pstDateTime->s32min;
        rtctm.second = pstDateTime->s32sec;
        rtctm.weekday = stTime.tm_wday;
        s32Ret = ioctl(s32fd_rtc, HI_RTC_SET_TIME, &rtctm);
#endif

        if (s32Ret < 0)
        {
            MLOGE("ioctl set rtc time error:%s\n", strerror(errno));
        }

        close(s32fd_rtc);
    }

#if defined(AMP_LINUX_HUAWEILITE)&&defined(__LINUX__)
    HI_MSG_SendASync(MSG_SYSTEM_SET_DATETIME, pstDateTime, sizeof(HI_SYSTEM_TM_S), NULL);
#endif

    gettimeofday(&stNowTime, NULL);
    MLOGI("%s\n", ctime(&(stNowTime.tv_sec)));
    return HI_SUCCESS;
}

HI_S32 HI_SYSTEM_GetRTCDateTime(HI_SYSTEM_TM_S* pstDateTime)
{
    HI_APPCOMM_CHECK_POINTER(pstDateTime, HI_EINVAL);
    HI_S32 s32fd_rtc = -1;
    s32fd_rtc = open(DEFAULT_RTC_DEVICE, O_RDWR);

    if (s32fd_rtc < 0)
    {
        MLOGE("open %s error:%s\n", DEFAULT_RTC_DEVICE, strerror(errno));
    }
    else
    {
#if defined(__LINUX__)
        struct rtc_time rtctm;

        if (ioctl(s32fd_rtc, RTC_RD_TIME, &rtctm) < 0)
        {
            MLOGE("ioctl get rtc time error:%s\n", strerror(errno));
        }
        else
        {
            pstDateTime->s32year = rtctm.tm_year + 1900;
            pstDateTime->s32mon = rtctm.tm_mon + 1;
            pstDateTime->s32mday = rtctm.tm_mday;
            pstDateTime->s32hour = rtctm.tm_hour;
            pstDateTime->s32min = rtctm.tm_min;
            pstDateTime->s32sec = rtctm.tm_sec;
        }

#elif  defined(__HuaweiLite__)
        rtc_time_t rtctm;

        if (ioctl(s32fd_rtc, HI_RTC_RD_TIME, &rtctm) < 0)
        {
            MLOGE("ioctl get rtc time error:%s\n", strerror(errno));
        }
        else
        {
            pstDateTime->s32year = rtctm.year;
            pstDateTime->s32mon = rtctm.month;
            pstDateTime->s32mday = rtctm.date;
            pstDateTime->s32hour = rtctm.hour;
            pstDateTime->s32min = rtctm.minute;
            pstDateTime->s32sec = rtctm.second;
        }

#endif
        close(s32fd_rtc);
    }

    return HI_SUCCESS;
}

HI_S32 HI_SYSTEM_Sound(HI_HANDLE AoHdl, const HI_MAPI_AO_ATTR_S* pstAoAttr, HI_U64 u64PhyAddr, HI_U32 u32DataLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32FrameCnt = 0;
    HI_U32 u32FrameLen = pstAoAttr->u32PtNumPerFrm * sizeof(HI_U16);//1024 PoiNum
    AUDIO_FRAME_S stAudioFrame;
    memset(&stAudioFrame, 0, sizeof(AUDIO_FRAME_S));
    stAudioFrame.enBitwidth = pstAoAttr->enBitwidth;//AUDIO_BIT_WIDTH_16;
    stAudioFrame.enSoundmode = AUDIO_SOUND_MODE_MONO;
    stAudioFrame.u32Len = u32FrameLen;

    while (u32FrameCnt < (u32DataLen / u32FrameLen))
    {
        stAudioFrame.u64VirAddr[0] = (HI_U8*)(HI_UL)u64PhyAddr;
        stAudioFrame.u64PhyAddr[0] = u64PhyAddr;
        s32Ret = HI_MAPI_AO_SendSysFrame(AoHdl, &stAudioFrame, 50);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        u32FrameCnt++;
        u64PhyAddr += u32FrameLen;
    }

    return HI_SUCCESS;
}

HI_VOID HI_SYSTEM_Reboot(HI_VOID)
{
    sync();
#if defined(__LINUX__)
    reboot(RB_AUTOBOOT);
#else
    void cmd_reset(void);
    cmd_reset();
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

