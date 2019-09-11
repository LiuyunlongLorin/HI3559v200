/**
 * @file      hi_uvc_gadget.c
 * @brief     uvc gadget application
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 * @version   1.0

 */

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <linux/usb/ch9.h>
#include <linux/videodev2.h>
#include "video.h"
#include "uvc.h"

#include "hi_uvc_gadget.h"
#include "uvc_framebuffer.h"
#include "hi_uvc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


/** CTRL ID */
#define HI_XUID_SET_RESET      (0x01)
#define HI_XUID_SET_STREAM     (0x02)
#define HI_XUID_SET_RESOLUTION (0x03)
#define HI_XUID_SET_IFRAME     (0x04)
#define HI_XUID_SET_BITRATE    (0x05)
#define HI_XUID_UPDATE_SYSTEM  (0x06)

/** H264 extension unit id, must same as to driver */
#define UNIT_XU_H264 (10)

/** USB_Video_Payload_H_264_1.0 3.3 */
#define UVCX_PICTURE_TYPE_CONTROL (0x09)

/** HICAMERA */
#define UNIT_XU_HICAMERA (0x11)

/** waited stream buffer node list */
#define UVC_WAITED_NODE_MAX_CNT (4)
static UVC_FRAME_NODE_S *s_apstUVCWaitedNode[UVC_WAITED_NODE_MAX_CNT];

extern HI_BOOL g_bPushVencData;

/** uvc probe status */
typedef struct tagUVC_PROBE_STATUS_S
{
    HI_BOOL bSetCur;
    HI_BOOL bGetCur;
    HI_BOOL bGetMax;
    HI_BOOL bGetMin;
} UVC_PROBE_STATUS_S;

/** uvc device context */
typedef struct tagUVC_DEVICE_CTX_S
{
    HI_S32  s32Fd;
    struct uvc_streaming_control stProbe;
    struct uvc_streaming_control stCommit;
    HI_S32  s32Ctrl;
    HI_S32  s32UnitId;
    HI_S32  s32InterfaceId;
    HI_U32  u32Fcc;
    HI_U32  u32Width;
    HI_U32  u32Height;
    HI_U32  u32Fps;
    HI_BOOL bBulk;
    HI_U32  u32BulkSize;
    HI_U32  u32ImgSize;
    HI_U8   u8BufCnt;
    HI_U8   u8Color;
    UVC_PROBE_STATUS_S stProbeStatus;
    HI_U32 u32Streaming;
} UVC_DEVICE_CTX_S;
static UVC_DEVICE_CTX_S s_stUVCDevCtx;

/** uvc frame attribute */
typedef struct tagUVC_FRAME_ATTR_S
{
    HI_U32 u32FrmIdx;   /**<frame index in uvc driver define */
    HI_U32 u32Width;    /**<frame width */
    HI_U32 u32Height;   /**<frame height */
    HI_U32 u32Interval; /**<frame interval in 100ns, eg. 30fps=333333 */
} UVC_FRAME_ATTR_S;
static const UVC_FRAME_ATTR_S s_astUVCFrames[HI_UVC_VIDEOMODE_BUTT] =
{
    {1, 640,  360,  333333},
    {2, 1280, 720,  333333},
    {3, 1920, 1080, 333333},
    {4, 3840, 2160, 333333}
};

/** uvc format attribute */
typedef struct tagUVC_FORMAT_ATTR_S
{
    HI_U32 u32Fcc;          /**<defined by V4L2 protocol, eg. V4L2_PIX_FMT_NV21 */
    HI_U8  u8FrameTypeCnt;  /**<frame type count in this uvc format, [1, UVC_VIDEOMODE_BUTT] */
    HI_U8  u8MinFrmIdx;     /**<minimum frame index, --> UVC_FRAME_ATTR_S.u32FrmIdx */
    HI_U8  u8MaxFrmIdx;     /**<maximum frame index, --> UVC_FRAME_ATTR_S.u32FrmIdx  */
    HI_U8  u8DefFrmIdx;     /**<default frame index, --> UVC_FRAME_ATTR_S.u32FrmIdx  */
    const UVC_FRAME_ATTR_S *pastFrames[HI_UVC_VIDEOMODE_BUTT];
} UVC_FORMAT_ATTR_S;
static UVC_FORMAT_ATTR_S s_astUVCFormats[HI_UVC_STREAM_FORMAT_BUTT] =
{
    { V4L2_PIX_FMT_YUV420, 0,0,0,0, {NULL,}},
    { V4L2_PIX_FMT_MJPEG,  0,0,0,0, {NULL,}},
    { V4L2_PIX_FMT_H264,   0,0,0,0, {NULL,}}
};


static HI_VOID UVC_ClearWaitedNode(HI_VOID)
{
    HI_S32 s32Idx = 0;
    for (s32Idx = 0; s32Idx < UVC_WAITED_NODE_MAX_CNT; ++s32Idx)
    {
        if (s_apstUVCWaitedNode[s32Idx])
        {
            MLOGD("Idx=%d Node=%p\n", s32Idx, s_apstUVCWaitedNode[s32Idx]);
            UVC_FRAMEBUF_PutNodeToQueue(&UVC_FRAMEBUF_GetQueues()->stIdle, s_apstUVCWaitedNode[s32Idx]);
            s_apstUVCWaitedNode[s32Idx] = NULL;
        }
    }
}

static HI_VOID UVC_ClearReadyQueue(HI_VOID)
{
    UVC_FRAME_NODE_S* pstNode = NULL;
    while (HI_SUCCESS == UVC_FRAMEBUF_GetNodeFromQueue(&UVC_FRAMEBUF_GetQueues()->stReady, &pstNode))
    {
        pstNode->u32used = 0;
        UVC_FRAMEBUF_PutNodeToQueue(&UVC_FRAMEBUF_GetQueues()->stIdle, pstNode);
        pstNode = NULL;
    }
}

#ifdef CFG_DEBUG_LOG_ON
static const HI_CHAR* UVC_GetFormatStr(HI_U32 u32Format)
{
    switch (u32Format)
    {
        case V4L2_PIX_FMT_H264:
            return "H264";
        case V4L2_PIX_FMT_MJPEG:
            return "MJPEG";
        case V4L2_PIX_FMT_YUV420:
            return "YUV420";
        case V4L2_PIX_FMT_YUYV:
            return "YUV422";
        default:
            return "unknown format";
    }
}
#endif

static HI_S32 UVC_DeviceOpen(const HI_CHAR* pszDevName, UVC_DEVICE_CTX_S *pstCtx)
{
    struct v4l2_capability stCap;
    HI_S32 s32Ret;
    HI_S32 s32Fd;
    /* Check device */
    struct stat buf;
    s32Ret =stat(pszDevName, &buf);
    if(s32Ret!=0)
    {
        sleep(1); //TimeOut 1s for USB device Recognition
    }

    /* open device */
    s32Fd = open(pszDevName, O_RDWR | O_NONBLOCK);
    if (s32Fd == -1)
    {
        MLOGE("v4l2 open failed(%s): %s (%d)\n",pszDevName, strerror(errno), errno);
        return HI_FAILURE;
    }

    /* query device capability */
    s32Ret = ioctl(s32Fd, VIDIOC_QUERYCAP, &stCap);
    if (s32Ret < 0)
    {
        MLOGE("unable to query device: %s (%d)\n", strerror(errno), errno);
        close(s32Fd);
        return HI_FAILURE;
    }
    MLOGD("V4l2 Capability[%s:caps=0x%04x]\n", pszDevName, stCap.capabilities);

    /* check capability */
    if (!(stCap.capabilities & V4L2_CAP_VIDEO_OUTPUT))
    {
        close(s32Fd);
        return HI_FAILURE;
    }
    MLOGD("device is %s on bus %s\n", stCap.card, stCap.bus_info);

    /* record device fd */
    pstCtx->s32Fd = s32Fd;
    MLOGD("DevFd=%d\n", pstCtx->s32Fd);

    memset(&(s_apstUVCWaitedNode), 0, sizeof(s_apstUVCWaitedNode));
    return HI_SUCCESS;
}

static HI_VOID UVC_DeviceClose(UVC_DEVICE_CTX_S *pstCtx)
{
    close(pstCtx->s32Fd);
    pstCtx->s32Fd = HI_APPCOMM_FD_INVALID_VAL;
}

static HI_VOID UVC_FillStreamUserPtrBuffer(struct v4l2_buffer* pstBuf)
{
    UVC_FRAME_NODE_S*  pstNode = NULL;
    UVC_FRAME_QUEUE_S* pstIdleQ = &UVC_FRAMEBUF_GetQueues()->stIdle;
    UVC_FRAME_QUEUE_S* pstReadyQ = &UVC_FRAMEBUF_GetQueues()->stReady;

    /* release node if process complete */
    if (s_apstUVCWaitedNode[pstBuf->index])
    {
        UVC_FRAMEBUF_PutNodeToQueue(pstIdleQ, s_apstUVCWaitedNode[pstBuf->index]);
        s_apstUVCWaitedNode[pstBuf->index] = NULL;
    }

    /* wait util frame data ready */
    UVC_FRAMEBUF_GetNodeFromQueue(pstReadyQ, &pstNode);
    if (!pstNode)
    {
        //MLOGD("Wait Ready Queue...\n");
        HI_MUTEX_LOCK(pstReadyQ->MutexLock);
        HI_COND_WAIT(pstReadyQ->CondWait, pstReadyQ->MutexLock);
        HI_MUTEX_UNLOCK(pstReadyQ->MutexLock);
        //MLOGD("Ready Queue OK\n");
        UVC_FRAMEBUF_GetNodeFromQueue(pstReadyQ, &pstNode);
    }
    pstBuf->bytesused = pstNode->u32used;
    pstBuf->m.userptr = (unsigned long)pstNode->pu8Mem;
    pstBuf->length = pstNode->u32BufLen;
    s_apstUVCWaitedNode[pstBuf->index] = pstNode;
}

static HI_S32 UVC_VideoProcess(UVC_DEVICE_CTX_S* pstCtx)
{
    struct v4l2_buffer stBuf;
    HI_S32 s32Ret;

    memset(&stBuf, 0, sizeof(stBuf));
    stBuf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    stBuf.memory = V4L2_MEMORY_USERPTR;

    /* release buffer */
    if ((s32Ret = ioctl(pstCtx->s32Fd, VIDIOC_DQBUF, &stBuf)) < 0)
    {
        return s32Ret;
    }
    //MLOGD(YELLOW"buf index = %u\n"NONE, stBuf.index);

    UVC_FillStreamUserPtrBuffer(&stBuf);

    /* get buffer */
    if ((s32Ret = ioctl(pstCtx->s32Fd, VIDIOC_QBUF, &stBuf)) < 0)
    {
        MLOGD("Unable to requeue buffer: %s (%d).\n", strerror(errno), errno);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 UVC_VideoReqUserPtrBufs(UVC_DEVICE_CTX_S* pstCtx, HI_U32 u32BufNum)
{
    struct v4l2_requestbuffers stReqBufs;
    HI_S32 s32Ret;
    pstCtx->u8BufCnt = 0;

    memset(&stReqBufs, 0, sizeof(stReqBufs));
    stReqBufs.count  = u32BufNum;
    stReqBufs.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    stReqBufs.memory = V4L2_MEMORY_USERPTR;

    /* request buffers */
    MLOGD("repect %u buffers allocated.\n", stReqBufs.count);
    s32Ret = ioctl(pstCtx->s32Fd, VIDIOC_REQBUFS, &stReqBufs);
    if (s32Ret < 0)
    {
        MLOGE("Unable to allocate buffers: %s (%d).\n", strerror(errno), errno);
        return HI_FAILURE;
    }
    MLOGD("%u buffers allocated.\n", stReqBufs.count);

    /* update context buffer number */
    pstCtx->u8BufCnt = stReqBufs.count;
    return HI_SUCCESS;
}

static HI_S32 UVC_StreamOn(UVC_DEVICE_CTX_S* pstCtx)
{
    struct v4l2_buffer stBuf;
    HI_U32 u32Idx;
    HI_S32 s32Ret = 0;

    UVC_GetStreamOps()->pfnReqIDR();

    for (u32Idx = 0; u32Idx < pstCtx->u8BufCnt; ++u32Idx)
    {
        memset(&stBuf, 0, sizeof(stBuf));
        stBuf.index = u32Idx;
        stBuf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        stBuf.memory = V4L2_MEMORY_USERPTR;
        UVC_FillStreamUserPtrBuffer(&stBuf);

        if ((s32Ret = ioctl(pstCtx->s32Fd, VIDIOC_QBUF, &stBuf)) < 0)
        {
            MLOGE("Unable to queue buffer(%u): %s (%d)\n", u32Idx, strerror(errno), errno);
            break;
        }
    }

    /* stream on */
    HI_S32 s32Type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ioctl(pstCtx->s32Fd, VIDIOC_STREAMON, &s32Type);
    pstCtx->u32Streaming = 1;
    MLOGD("stream on\n");

    return s32Ret;
}

static HI_S32 UVC_SetVideoFormat(UVC_DEVICE_CTX_S* pstCtx)
{
    /* fill format */
    struct v4l2_format stFmt;
    memset(&stFmt, 0, sizeof(stFmt));
    stFmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    stFmt.fmt.pix.width  = pstCtx->u32Width;
    stFmt.fmt.pix.height = pstCtx->u32Height;
    stFmt.fmt.pix.pixelformat = pstCtx->u32Fcc;
    stFmt.fmt.pix.field = V4L2_FIELD_NONE;
    if ((V4L2_PIX_FMT_MJPEG == pstCtx->u32Fcc) || (V4L2_PIX_FMT_H264 == pstCtx->u32Fcc))
    {
        stFmt.fmt.pix.sizeimage = pstCtx->u32ImgSize;
    }
    else
    {
        stFmt.fmt.pix.sizeimage = pstCtx->u32Width * pstCtx->u32Height * 3 / 2;
    }
    MLOGD("Format[%s] Res[%ux%u] FrameRate[%u] SizeImage[%u]\n", UVC_GetFormatStr(pstCtx->u32Fcc),
            pstCtx->u32Width, pstCtx->u32Height, pstCtx->u32Fps, stFmt.fmt.pix.sizeimage);

    /* set video format */
    HI_S32 s32Ret;
    if ((s32Ret = ioctl(pstCtx->s32Fd, VIDIOC_S_FMT, &stFmt)) < 0)
    {
        MLOGE("Unable to set format: %s (%d).\n", strerror(errno), errno);
    }

    return s32Ret;
}

static HI_VOID UVC_StreamOff(UVC_DEVICE_CTX_S* pstCtx)
{
    HI_S32 s32Type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ioctl(pstCtx->s32Fd, VIDIOC_STREAMOFF, &s32Type);
    UVC_GetStreamOps()->pfnClose();
    pstCtx->u32Streaming = 0;
    MLOGD("Stopping video stream.\n");
}

static HI_VOID UVC_VideoDisable(UVC_DEVICE_CTX_S* pstCtx)
{
    UVC_StreamOff(pstCtx);
    UVC_ClearWaitedNode();
}

static HI_UVC_STREAM_FORMAT_E UVC_FCC2StreamFormat(HI_U32 u32Fcc)
{
    switch(u32Fcc)
    {
        case V4L2_PIX_FMT_YUV420:
            return HI_UVC_STREAM_FORMAT_YUV420;
        case V4L2_PIX_FMT_YUYV:
            return HI_UVC_STREAM_FORMAT_YUV420;
        case V4L2_PIX_FMT_MJPEG:
            return HI_UVC_STREAM_FORMAT_MJPEG;
        case V4L2_PIX_FMT_H264:
        default:
            return HI_UVC_STREAM_FORMAT_H264;
    }
}

static HI_VOID UVC_VideoEnable(UVC_DEVICE_CTX_S* pstCtx)
{
    UVC_ClearReadyQueue();
    UVC_VideoDisable(pstCtx);

    UVC_STREAM_ATTR_S stStreamAttr;
    stStreamAttr.enFormat = UVC_FCC2StreamFormat(pstCtx->u32Fcc);
    stStreamAttr.u32Width = pstCtx->u32Width;
    stStreamAttr.u32Height = pstCtx->u32Height;
    stStreamAttr.u32Fps = pstCtx->u32Fps;

    UVC_GetStreamOps()->pfnSetAttr(&stStreamAttr);
    UVC_GetStreamOps()->pfnClose();
    UVC_GetStreamOps()->pfnOpen();

    UVC_VideoReqUserPtrBufs(pstCtx, UVC_WAITED_NODE_MAX_CNT);
    UVC_StreamOn(pstCtx);
}

static HI_VOID UVC_FillStreamingControl(UVC_DEVICE_CTX_S* pstCtx,
                        struct uvc_streaming_control* pstCtrl,
                        HI_UVC_STREAM_FORMAT_E enFormat, HI_U32 u32FrameIdx)
{
    UVC_FORMAT_ATTR_S *pstFormatInfo = NULL;
    const UVC_FRAME_ATTR_S* pstFrameInfo = NULL;
    MLOGD("format[%d] frameIdx[%u]\n", enFormat, u32FrameIdx);

    /* check format and frame support or not */
    if (enFormat >= HI_UVC_STREAM_FORMAT_BUTT)
    {
        MLOGW("unsupport format\n");
        return;
    }
    pstFormatInfo = &s_astUVCFormats[enFormat];
    MLOGD("fcc[%08x], framecnt[%u]\n", pstFormatInfo->u32Fcc, pstFormatInfo->u8FrameTypeCnt);

    HI_U32 u32Idx = 0;
    for (u32Idx = 0; u32Idx < pstFormatInfo->u8FrameTypeCnt; ++u32Idx)
    {
        if (u32FrameIdx == pstFormatInfo->pastFrames[u32Idx]->u32FrmIdx)
        {
            pstFrameInfo = pstFormatInfo->pastFrames[u32Idx];
            break;
        }
    }
    if (!pstFrameInfo)
    {
        MLOGW("unsupport frame !! \n");
        return;
    }
    MLOGD("FrameInfo: Resolution[%ux%u] Interval[%u]\n", pstFrameInfo->u32Width,
        pstFrameInfo->u32Height, pstFrameInfo->u32Interval);

    /* fill streaming control info */
    memset(pstCtrl, 0, sizeof(*pstCtrl));
    pstCtrl->bmHint = 1;
    pstCtrl->bFormatIndex = enFormat + 1;
    pstCtrl->bFrameIndex = pstFrameInfo->u32FrmIdx;
    pstCtrl->dwFrameInterval = pstFrameInfo->u32Interval;
    switch (pstFormatInfo->u32Fcc)
    {
        case V4L2_PIX_FMT_NV21:
            pstCtrl->dwMaxVideoFrameSize = pstFrameInfo->u32Width * pstFrameInfo->u32Height * 2;
            break;
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
            pstCtrl->dwMaxVideoFrameSize = pstCtx->u32ImgSize;
            break;
    }
    /* TODO this should be filled by the driver. */
    pstCtrl->dwMaxPayloadTransferSize = ((pstCtx->bBulk) ? pstCtx->u32BulkSize : 3072);
    pstCtrl->bmFramingInfo = 3;
    pstCtrl->bPreferedVersion = 1;
    pstCtrl->bMaxVersion = 1;

    MLOGI("FormatIdx[%u] Frame[%u] Interval[%u] MaxVideoFrameSize[%u] MaxPayloadTansferSize[%u]\n",
         pstCtrl->bFormatIndex, pstCtrl->bFrameIndex, pstCtrl->dwFrameInterval,
         pstCtrl->dwMaxVideoFrameSize, pstCtrl->dwMaxPayloadTransferSize);
}

static HI_VOID UVC_HandleH265PicTypeControl(UVC_DEVICE_CTX_S* pstCtx,
                        HI_U8 u8Req, struct uvc_request_data *pstReqData)
{
    switch (u8Req)
    {
        case UVC_SET_CUR:
            pstReqData->length = 4;
            MLOGI("SET_CUR\n");
            break;
        case UVC_GET_LEN:
            pstReqData->data[0] = 0x04;
            pstReqData->data[1] = 0x00;
            pstReqData->length = 2;
            MLOGI("GET_LEN\n");
            break;
        case UVC_GET_INFO:
            pstReqData->data[0] = 0x03;
            pstReqData->length = 1;
            MLOGI("GET_INFO\n");
            break;
        default:
            break;
    }
}

static HI_VOID UVC_HandleHiCamIFrameControl(UVC_DEVICE_CTX_S* pstCtx,
                        HI_U8 u8Req, struct uvc_request_data *pstReqData)
{
    switch (u8Req)
    {
        case UVC_SET_CUR:
            pstReqData->length = 4;
            MLOGI("SET_CUR");
            break;
        case UVC_GET_LEN:
            pstReqData->data[0] = 0x04;
            pstReqData->data[1] = 0x00;
            pstReqData->length = 2;
            MLOGI("GET_LEN for UVCX_PICRURE_TYPE_CONTROL\n");
            break;
        case UVC_GET_INFO:
            pstReqData->data[0] = 0x03;
            pstReqData->length = 1;
            MLOGI("GET_INFO\n");
            break;
        default:
            break;
    }
}

static HI_VOID UVC_HandleHiCamStreamControl(UVC_DEVICE_CTX_S* pstCtx,
                        HI_U8 u8Req, struct uvc_request_data *pstReqData)
{
    switch (u8Req)
    {
        case UVC_SET_CUR:
            pstReqData->length = 4;
            MLOGI("SET_CUR");
            break;
        case UVC_GET_LEN:
            pstReqData->data[0] = 0x04;
            pstReqData->data[1] = 0x00;
            pstReqData->length = 2;
            MLOGI("GET_LEN\n");
            break;
        case UVC_GET_INFO:
            pstReqData->data[0] = 0x03;
            pstReqData->length = 1;
            MLOGI("GET_INFO\n");
            break;
        default:
            break;
    }
}

static HI_VOID UVC_HandleHiCamResetControl(UVC_DEVICE_CTX_S* pstCtx,
                        HI_U8 u8Req, struct uvc_request_data *pstReqData)
{
    switch (u8Req)
    {
        case UVC_SET_CUR:
            pstReqData->length = 4;
            MLOGI("SET_CUR");
            break;
        case UVC_GET_LEN:
            pstReqData->data[0] = 0x04;
            pstReqData->data[1] = 0x00;
            pstReqData->length = 2;
            MLOGI("GET_LEN\n");
            break;
        case UVC_GET_INFO:
            pstReqData->data[0] = 0x03;
            pstReqData->length = 1;
            MLOGI("GET_INFO\n");
            break;
        default:
            break;
    }
}

static HI_VOID UVC_HandleHiCamUpdateSys(UVC_DEVICE_CTX_S* pstCtx,
                        HI_U8 u8Req, struct uvc_request_data *pstReqData)
{
    switch (u8Req)
    {
        case UVC_SET_CUR:
            pstReqData->length = 4;
            MLOGI("SET_CUR\n");
            break;
        case UVC_GET_LEN:
            pstReqData->data[0] = 0x04;
            pstReqData->data[1] = 0x00;
            pstReqData->length = 2;
            MLOGI("GET_LEN\n");
            break;
        case UVC_GET_INFO:
            pstReqData->data[0] = 0x03;
            pstReqData->length = 1;
            MLOGI("GET_INFO\n");
            break;
        default:
            break;
    }
}

static HI_VOID UVC_HandleDefControl(UVC_DEVICE_CTX_S* pstCtx,
                        HI_U8 u8Req, struct uvc_request_data* pstReqData)
{
    switch (u8Req)
    {
        case UVC_GET_MIN:
            pstReqData->length = 4;
            break;
        case UVC_GET_LEN:
            pstReqData->data[0] = 0x04;
            pstReqData->data[1] = 0x00;
            pstReqData->length = 2;
            break;
        case UVC_GET_INFO:
            pstReqData->data[0] = 0x03;
            pstReqData->length = 1;
            break;
        default:
            break;
    }
}

static HI_VOID UVC_EventProcIntfControl(UVC_DEVICE_CTX_S *pstCtx,
                        HI_U8 u8Req, struct uvc_request_data* pstReqData)
{
    if (UNIT_XU_H264 == pstCtx->s32UnitId)
    {
        switch (pstCtx->s32Ctrl)
        {
            case UVCX_PICTURE_TYPE_CONTROL:
                MLOGD("\n");
                UVC_HandleH265PicTypeControl(pstCtx, u8Req, pstReqData);
                break;
            default:
                UVC_HandleDefControl(pstCtx, u8Req, pstReqData);
                break;
        }
    }
    else if (UNIT_XU_HICAMERA == pstCtx->s32UnitId)
    {
        switch (pstCtx->s32Ctrl)
        {
            case HI_XUID_SET_RESET:
                MLOGD("\n");
                UVC_HandleHiCamResetControl(pstCtx, u8Req, pstReqData);
                break;

            case HI_XUID_SET_STREAM:
                MLOGD("\n");
                UVC_HandleHiCamStreamControl(pstCtx, u8Req, pstReqData);
                break;

            case HI_XUID_SET_IFRAME:
                MLOGD("\n");
                UVC_HandleHiCamIFrameControl(pstCtx, u8Req, pstReqData);
                break;

            case HI_XUID_UPDATE_SYSTEM:
                MLOGD("\n");
                UVC_HandleHiCamUpdateSys(pstCtx, u8Req, pstReqData);
                break;

            default:
                UVC_HandleDefControl(pstCtx, u8Req, pstReqData);
                break;
        }
    }
}

static HI_VOID UVC_EventProcIntfStreaming(UVC_DEVICE_CTX_S *pstCtx, HI_U8 u8Req,
                                         struct uvc_request_data* pstReqData)
{
    struct uvc_streaming_control* pstStreamCtrl;

    if ((pstCtx->s32Ctrl != UVC_VS_PROBE_CONTROL) && (pstCtx->s32Ctrl != UVC_VS_COMMIT_CONTROL))
    {
        return;
    }

    pstReqData->length = sizeof(struct uvc_streaming_control);
    pstStreamCtrl = (struct uvc_streaming_control*)&pstReqData->data;

    switch (u8Req)
    {
        case UVC_SET_CUR:
            break;

        case UVC_GET_CUR:
            if (pstCtx->s32Ctrl == UVC_VS_PROBE_CONTROL)
            {
                memcpy(pstStreamCtrl, &pstCtx->stProbe, sizeof(*pstStreamCtrl));
            }
            else
            {
                memcpy(pstStreamCtrl, &pstCtx->stCommit, sizeof(*pstStreamCtrl));
            }

            break;

        case UVC_GET_MIN:
        {
            HI_UVC_STREAM_FORMAT_E enFormat = UVC_FCC2StreamFormat(pstCtx->u32Fcc);
            HI_U32 u32FrameIdx = s_astUVCFormats[enFormat].u8MinFrmIdx;
            UVC_FillStreamingControl(pstCtx, pstStreamCtrl, enFormat, u32FrameIdx);
        }
            break;

        case UVC_GET_MAX:
        {
            HI_UVC_STREAM_FORMAT_E enFormat = UVC_FCC2StreamFormat(pstCtx->u32Fcc);
            HI_U32 u32FrameIdx = s_astUVCFormats[enFormat].u8MaxFrmIdx;
            UVC_FillStreamingControl(pstCtx, pstStreamCtrl, enFormat, u32FrameIdx);
        }
            break;

        case UVC_GET_DEF:
        {
            HI_UVC_STREAM_FORMAT_E enFormat = UVC_FCC2StreamFormat(pstCtx->u32Fcc);
            HI_U32 u32FrameIdx = s_astUVCFormats[enFormat].u8DefFrmIdx;
            UVC_FillStreamingControl(pstCtx, pstStreamCtrl, enFormat, u32FrameIdx);
        }
            break;

        case UVC_GET_RES:
            memset(pstStreamCtrl, 0, sizeof(struct uvc_streaming_control));
            break;

        case UVC_GET_LEN:
            pstReqData->data[0] = 0x00;
            pstReqData->data[1] = 0x22;/* sizeof(struct uvc_streaming_control) */
            pstReqData->length = 2;
            break;

        case UVC_GET_INFO:
            pstReqData->data[0] = 0x03;
            pstReqData->length = 1;
            break;
    }
}
#ifdef CFG_DEBUG_LOG_ON

static const HI_CHAR* UVC_GetRequestStr(HI_U8 u8Req)
{
    switch(u8Req)
    {
        case UVC_SET_CUR:
            return "SET_CUR";
        case UVC_GET_CUR:
            return "GET_CUR ";
        case UVC_GET_MIN:
            return "GET_MIN";
        case UVC_GET_MAX:
            return "GET_MAX";
        case UVC_GET_RES:
            return "GET_RES";
        case UVC_GET_LEN:
            return "GET_LEN";
        case UVC_GET_INFO:
            return "GET_INFO";
        case UVC_GET_DEF:
            return "GET_DEF";
        default:
            return "UNKNOW";
    }
}

static const HI_CHAR* UVC_GetInterfaceCtrlStr(HI_S32 s32CtrlCode)
{
    switch(s32CtrlCode)
    {
        case UVC_VS_PROBE_CONTROL:
            return "PROB_CONTROL";
        case UVC_VS_COMMIT_CONTROL:
            return "COMMIT_CONTROL";
        default:
            return "UNKOWN";
    }
}
#endif
static HI_VOID UVC_SetProbeStatus(UVC_PROBE_STATUS_S* pstProbeStatus, HI_S32 s32Ctrl, HI_U8 u8Req)
{
    if (UVC_VS_PROBE_CONTROL == s32Ctrl)
    {
        switch (u8Req)
        {
            case UVC_SET_CUR:
                pstProbeStatus->bSetCur = HI_TRUE;
                break;
            case UVC_GET_CUR:
                pstProbeStatus->bGetCur = HI_TRUE;
                break;
            case UVC_GET_MIN:
                pstProbeStatus->bGetMin = HI_TRUE;
                break;
            case UVC_GET_MAX:
                pstProbeStatus->bGetMax = HI_TRUE;
                break;
            default:
                break;
        }
    }
}

static HI_BOOL UVC_CheckProbeStatus(const UVC_PROBE_STATUS_S* pstProbeStatus)
{
    if ((pstProbeStatus->bGetCur) && (pstProbeStatus->bSetCur)
        && (pstProbeStatus->bGetMin) && (pstProbeStatus->bGetMax))
    {
        return HI_TRUE;
    }
    MLOGW("the probe status is not correct...\n");
    return HI_FALSE;
}

static HI_VOID UVC_EventProcStandard(UVC_DEVICE_CTX_S *pstCtx, struct usb_ctrlrequest* stCtrl,
                                        struct uvc_request_data* pstReqData)
{
    MLOGD("standard request\n");
}


static HI_VOID UVC_EventProcInterface(UVC_DEVICE_CTX_S *pstCtx, struct usb_ctrlrequest* pstCtrl,
                                    struct uvc_request_data* pstReqData)
{
    UVC_SetProbeStatus(&pstCtx->stProbeStatus, pstCtx->s32Ctrl, pstCtrl->bRequest);
    switch (pstCtx->s32InterfaceId)
    {
    case UVC_INTF_CONTROL:
        /* PC Connected */
        if (!UVC_GetCtx()->bPCConnect)
        {
            MLOGD(YELLOW"PC Connected"NONE"\n");
            UVC_GetCtx()->bPCConnect = HI_TRUE;
        }
        UVC_EventProcIntfControl(pstCtx, pstCtrl->bRequest, pstReqData);
        break;

    case UVC_INTF_STREAMING:
        UVC_EventProcIntfStreaming(pstCtx, pstCtrl->bRequest, pstReqData);
        break;

    default:
        break;
    }
}

static HI_VOID UVC_EventProcClass(UVC_DEVICE_CTX_S* pstCtx, struct usb_ctrlrequest* stCtrl,
                                     struct uvc_request_data* pstReqData)
{
    switch (stCtrl->bRequestType & USB_RECIP_MASK)
    {
        case USB_RECIP_INTERFACE:
            /** Save Control Selector/UnitId/InterfaceId */
            pstCtx->s32Ctrl = (stCtrl->wValue >> 8);
            pstCtx->s32UnitId = (stCtrl->wIndex >> 8);
            pstCtx->s32InterfaceId = (stCtrl->wIndex & 0xff);
            if (stCtrl->wIndex & 0xff)
            {
                MLOGD("USB_RECIP_INTERFACE\n");
                MLOGD("InterfaceId : %d\n", pstCtx->s32InterfaceId);
                MLOGD("UnitId  : %d\n", pstCtx->s32UnitId);
                MLOGD("CS Code : 0x%02x(%s)\n", pstCtx->s32Ctrl, (HI_CHAR*)UVC_GetInterfaceCtrlStr(pstCtx->s32Ctrl));
                MLOGD("Request : 0x%02x(%s)\n", stCtrl->bRequest, (HI_CHAR*)UVC_GetRequestStr(stCtrl->bRequest));
            }
            UVC_EventProcInterface(pstCtx, stCtrl, pstReqData);
            break;
        case USB_RECIP_DEVICE:
            MLOGD("USB_RECIP_DEVICE\n");
            break;
        case USB_RECIP_ENDPOINT:
            MLOGD("USB_RECIP_ENDPOINT\n");
            break;
        case USB_RECIP_OTHER:
            MLOGD("USB_RECIP_OTHER\n");
            break;
    }
}

static HI_VOID UVC_EventProcSetup(UVC_DEVICE_CTX_S* pstCtx, struct usb_ctrlrequest* stCtrl,
                                     struct uvc_request_data* pstReqData)
{
    pstCtx->s32Ctrl = 0;
    pstCtx->s32UnitId = 0;
    pstCtx->s32InterfaceId = 0;

    switch (stCtrl->bRequestType & USB_TYPE_MASK)
    {
        case USB_TYPE_STANDARD:
            UVC_EventProcStandard(pstCtx, stCtrl, pstReqData);
            break;
        case USB_TYPE_CLASS:
            UVC_EventProcClass(pstCtx, stCtrl, pstReqData);
            break;
        default:
            break;
    }
}

static HI_VOID UVC_HandleH264Data(UVC_DEVICE_CTX_S* pstCtx, struct uvc_request_data* pstReqData)
{
    switch (pstCtx->s32Ctrl)
    {
        case UVCX_PICTURE_TYPE_CONTROL:
            MLOGI("DO UVCX_PICTURE_TYPE_CONTROL");
            break;
        default:
            break;
    }
}

static HI_VOID UVC_HandleHiCamData(UVC_DEVICE_CTX_S* pstCtx, struct uvc_request_data* pstReqData)
{
    switch (pstCtx->s32Ctrl)
    {
        case HI_XUID_SET_RESET:
            MLOGI("DO HI_XUID_SET_RESET");
            break;
        default:
            break;
    }
}

static HI_VOID UVC_EventProcIntfData(UVC_DEVICE_CTX_S* pstCtx, struct uvc_request_data* pstReqData)
{
    if (pstCtx->s32UnitId == UNIT_XU_H264)
    {
        return UVC_HandleH264Data(pstCtx, pstReqData);
    }
    else if (pstCtx->s32UnitId == UNIT_XU_HICAMERA)
    {
        return UVC_HandleHiCamData(pstCtx, pstReqData);
    }
}

static HI_VOID UVC_EventProcData(UVC_DEVICE_CTX_S* pstCtx, struct uvc_request_data* pstReqData)
{
    /* check unitId and interfaceId */
    if ((pstCtx->s32UnitId != 0) && (pstCtx->s32InterfaceId == UVC_INTF_CONTROL))
    {
        return UVC_EventProcIntfData(pstCtx, pstReqData);
    }

    /* check control selector */
    struct uvc_streaming_control* stTargetCtrl = NULL;
    switch (pstCtx->s32Ctrl)
    {
        case UVC_VS_PROBE_CONTROL:
            MLOGD("setting probe control, length = %d\n", pstReqData->length);
            stTargetCtrl = &pstCtx->stProbe;
            break;
        case UVC_VS_COMMIT_CONTROL:
            MLOGD("setting commit control, length = %d\n", pstReqData->length);
            stTargetCtrl = &pstCtx->stCommit;
            break;
        default:
            MLOGW("setting unknown control, length = %d\n", pstReqData->length);
            return;
    }

    HI_U32 u32FormatIdx, u32FrameIdx;
    struct uvc_streaming_control* pstControl = (struct uvc_streaming_control*)&pstReqData->data;
    UVC_FORMAT_ATTR_S* pstFormatInfo = NULL;
    const UVC_FRAME_ATTR_S* pstFrameInfo = NULL;

    u32FormatIdx = pstControl->bFormatIndex;
    u32FrameIdx = pstControl->bFrameIndex;
    MLOGD("FormatIdx[%u] FrameIdx[%u] FrameInterval[%u] MaxVideoFrameSize[%u]\n",
        u32FormatIdx, u32FrameIdx, pstControl->dwFrameInterval, pstControl->dwMaxVideoFrameSize);

    /* check format and frame support or not */
    if (u32FormatIdx > HI_UVC_STREAM_FORMAT_BUTT)
    {
        MLOGW("unsupport format u32FormatIdx:%d \n",u32FormatIdx);
        return;
    }
    pstFormatInfo = &s_astUVCFormats[u32FormatIdx - 1];

    HI_U32 u32Idx = 0;
    for (u32Idx = 0; u32Idx < pstFormatInfo->u8FrameTypeCnt; ++u32Idx)
    {
        if (u32FrameIdx == pstFormatInfo->pastFrames[u32Idx]->u32FrmIdx)
        {
            pstFrameInfo = pstFormatInfo->pastFrames[u32Idx];
            break;
        }
    }
    if (!pstFrameInfo)
    {
        MLOGW("unsupport frame !\n");
        return;
    }
    MLOGD("FrameInfo: Resolution[%ux%u] Interval[%u]\n", pstFrameInfo->u32Width,
        pstFrameInfo->u32Height, pstFrameInfo->u32Interval);

    /* update streaming control information */
    stTargetCtrl->bFormatIndex = u32FormatIdx;
    stTargetCtrl->bFrameIndex = u32FrameIdx;
    switch (pstFormatInfo->u32Fcc)
    {
        case V4L2_PIX_FMT_YUV420:
            stTargetCtrl->dwMaxVideoFrameSize = pstFrameInfo->u32Width * pstFrameInfo->u32Height * 2;
            break;
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
            stTargetCtrl->dwMaxVideoFrameSize = pstCtx->u32ImgSize;
            break;
    }
    stTargetCtrl->dwFrameInterval = pstFrameInfo->u32Interval;

    /* prepare video if probe finished in commit control */
    if ((UVC_VS_COMMIT_CONTROL == pstCtx->s32Ctrl) && UVC_CheckProbeStatus(&pstCtx->stProbeStatus))
    {
        pstCtx->u32Fcc    = pstFormatInfo->u32Fcc;
        pstCtx->u32Width  = pstFrameInfo->u32Width;
        pstCtx->u32Height = pstFrameInfo->u32Height;
        pstCtx->u32Fps = 10000000 / pstFrameInfo->u32Interval;
        UVC_SetVideoFormat(pstCtx);
        if (pstCtx->bBulk != 0)
        {
            UVC_VideoDisable(pstCtx);
            UVC_VideoEnable(pstCtx);
        }
    }

    /* clear probe status in commit control */
    if (pstCtx->s32Ctrl == UVC_VS_COMMIT_CONTROL)
    {
        memset(&pstCtx->stProbeStatus, 0, sizeof(pstCtx->stProbeStatus));
    }
}

static HI_VOID UVC_EventProcess(UVC_DEVICE_CTX_S* pstCtx)
{
    struct v4l2_event stV4l2Event;
    struct uvc_event* pstUVCEvent = (struct uvc_event*)&stV4l2Event.u.data;
    struct uvc_request_data stReqData;
    HI_S32 s32Ret = HI_SUCCESS;

    /* get event */
    s32Ret = ioctl(pstCtx->s32Fd, VIDIOC_DQEVENT, &stV4l2Event);
    if (s32Ret < 0)
    {
        MLOGE("VIDIOC_DQEVENT failed: %s (%d)\n", strerror(errno), errno);
        return;
    }

    memset(&stReqData, 0, sizeof(stReqData));
    stReqData.length = 32;

    switch (stV4l2Event.type)
    {
        case UVC_EVENT_CONNECT:
            MLOGD(YELLOW"UVC_EVENT_CONNECT"NONE"\n");
            return;
        case UVC_EVENT_DISCONNECT:
            MLOGD(YELLOW"UVC_EVENT_DISCONNECT"NONE"\n");
            return;
        case UVC_EVENT_SETUP:
            MLOGD(YELLOW"UVC_EVENT_SETUP"NONE"\n");
            UVC_EventProcSetup(pstCtx, &pstUVCEvent->req, &stReqData);
            break;
        case UVC_EVENT_DATA:
            MLOGD("UVC_EVENT_DATA\n");
            UVC_EventProcData(pstCtx, &pstUVCEvent->data);
            return;
        case UVC_EVENT_STREAMON:
            MLOGD("UVC_EVENT_STREAMON\n");
            if (!pstCtx->bBulk)
            {
                UVC_VideoEnable(pstCtx);
            }
            return;
        case UVC_EVENT_STREAMOFF:
            MLOGD("UVC_EVENT_STREAMOFF\n");
            if (!pstCtx->bBulk)
            {
                UVC_VideoDisable(pstCtx);
            }
            return;
          default:
           MLOGW("Setting unknown UVC_EVENT, type = %d !!!\n", stV4l2Event.type);
           return;
    }

    /* send process response */
    s32Ret = ioctl(pstCtx->s32Fd, UVCIOC_SEND_RESPONSE, &stReqData);
    if (s32Ret < 0)
    {
        MLOGE("UVCIOC_SEND_RESPONSE failed: %s (%d)\n", strerror(errno), errno);
        return;
    }
}

static HI_VOID UVC_EventInit(UVC_DEVICE_CTX_S* pstCtx)
{
    struct v4l2_event_subscription stEventSub;

    UVC_FillStreamingControl(pstCtx, &pstCtx->stProbe, HI_UVC_STREAM_FORMAT_H264,
        s_astUVCFormats[HI_UVC_STREAM_FORMAT_H264].u8DefFrmIdx);
    UVC_FillStreamingControl(pstCtx, &pstCtx->stCommit, HI_UVC_STREAM_FORMAT_H264,
        s_astUVCFormats[HI_UVC_STREAM_FORMAT_H264].u8DefFrmIdx);

    if (pstCtx->bBulk)
    {
        /* FIXME Crude hack, must be negotiated with the driver. */
        pstCtx->stProbe.dwMaxPayloadTransferSize  = pstCtx->u32BulkSize;
        pstCtx->stCommit.dwMaxPayloadTransferSize = pstCtx->u32BulkSize;
    }

    memset(&stEventSub, 0, sizeof stEventSub);
    stEventSub.type = UVC_EVENT_SETUP;
    ioctl(pstCtx->s32Fd, VIDIOC_SUBSCRIBE_EVENT, &stEventSub);
    stEventSub.type = UVC_EVENT_DATA;
    ioctl(pstCtx->s32Fd, VIDIOC_SUBSCRIBE_EVENT, &stEventSub);
    stEventSub.type = UVC_EVENT_STREAMON;
    ioctl(pstCtx->s32Fd, VIDIOC_SUBSCRIBE_EVENT, &stEventSub);
    stEventSub.type = UVC_EVENT_STREAMOFF;
    ioctl(pstCtx->s32Fd, VIDIOC_SUBSCRIBE_EVENT, &stEventSub);
}

static HI_VOID UVC_VideoInit(UVC_DEVICE_CTX_S* pstCtx __attribute__ ((__unused__)))
{
}

HI_S32 UVC_GADGET_Init(const HI_UVC_DEVICE_CAP_S *pstDevCaps, HI_U32 u32MaxFrameSize)
{
    HI_APPCOMM_CHECK_POINTER(pstDevCaps, HI_FAILURE);

    HI_S32 i = 0, j = 0;
    HI_UVC_VIDEOMODE_E enMode;
    for (i = 0; i < HI_UVC_STREAM_FORMAT_BUTT; ++i)
    {
        MLOGD("Format[%d]: support %u videomodes\n", i, pstDevCaps->astFmtCaps[i].u32Cnt);
        s_astUVCFormats[i].u8FrameTypeCnt = (HI_U8)pstDevCaps->astFmtCaps[i].u32Cnt;
        for (j = 0; j < pstDevCaps->astFmtCaps[i].u32Cnt; ++j)
        {
            MLOGD("------videomode[%d] %d\n", j, pstDevCaps->astFmtCaps[i].astModes[j].enMode);
            s_astUVCFormats[i].pastFrames[j] = &s_astUVCFrames[pstDevCaps->astFmtCaps[i].astModes[j].enMode];
        }
        enMode = pstDevCaps->astFmtCaps[i].astModes[0].enMode;
        s_astUVCFormats[i].u8MinFrmIdx = s_astUVCFrames[enMode].u32FrmIdx;
        if(j < 1)
        {
            return HI_FAILURE;
        }
        enMode = pstDevCaps->astFmtCaps[i].astModes[j-1].enMode;
        s_astUVCFormats[i].u8MaxFrmIdx = s_astUVCFrames[enMode].u32FrmIdx;
        enMode = pstDevCaps->astFmtCaps[i].enDefMode;
        s_astUVCFormats[i].u8DefFrmIdx = s_astUVCFrames[enMode].u32FrmIdx;
        MLOGD("FrameTypeCnt[%u] MinFrmIdx[%u] MaxFrmIdx[%u] DefFrmIdx[%u]\n",
            s_astUVCFormats[i].u8FrameTypeCnt, s_astUVCFormats[i].u8MinFrmIdx,
            s_astUVCFormats[i].u8MaxFrmIdx,    s_astUVCFormats[i].u8DefFrmIdx);
    }

    s_stUVCDevCtx.u32ImgSize  = u32MaxFrameSize;
    s_stUVCDevCtx.bBulk = 0;
    s_stUVCDevCtx.u32BulkSize = u32MaxFrameSize;
    s_stUVCDevCtx.s32Fd = HI_APPCOMM_FD_INVALID_VAL;
    MLOGD("ImageSize[%u] BulkMode[%d] BulkSize[%u]\n", s_stUVCDevCtx.u32ImgSize,
        s_stUVCDevCtx.bBulk, s_stUVCDevCtx.u32BulkSize);
    return HI_SUCCESS;
}

HI_S32 UVC_GADGET_DeviceOpen(const HI_CHAR *pazDevPath)
{
    HI_APPCOMM_CHECK_POINTER(pazDevPath, HI_FAILURE);

    if(s_stUVCDevCtx.s32Fd == HI_APPCOMM_FD_INVALID_VAL)
    {
        HI_S32 s32Ret;
        s32Ret = UVC_DeviceOpen(pazDevPath, &s_stUVCDevCtx);
        if (s32Ret != 0)
        {
            return HI_FAILURE;
        }
    }

    UVC_EventInit(&s_stUVCDevCtx);
    UVC_VideoInit(&s_stUVCDevCtx);

    return HI_SUCCESS;
}

HI_S32 UVC_GADGET_DeviceClose(HI_VOID)
{
    UVC_VideoDisable(&s_stUVCDevCtx);
    UVC_DeviceClose(&s_stUVCDevCtx);

    return HI_SUCCESS;
}

HI_S32 UVC_GADGET_DeviceCheck(HI_VOID)
{
    fd_set efds, wfds;
    struct timeval stTv;
    HI_S32 s32Ret;

    stTv.tv_sec  = 1;
    stTv.tv_usec = 0;

    FD_ZERO(&efds);
    FD_ZERO(&wfds);
    FD_SET(s_stUVCDevCtx.s32Fd, &efds);

    if(s_stUVCDevCtx.u32Streaming == 1)
        FD_SET(s_stUVCDevCtx.s32Fd, &wfds);

    s32Ret = select(s_stUVCDevCtx.s32Fd + 1, NULL, &wfds, &efds, &stTv);
    if (s32Ret > 0)
    {
        g_bPushVencData = HI_TRUE;
        if (FD_ISSET(s_stUVCDevCtx.s32Fd, &efds))
        {
            UVC_EventProcess(&s_stUVCDevCtx);
        }

        if (FD_ISSET(s_stUVCDevCtx.s32Fd, &wfds))
        {
            UVC_VideoProcess(&s_stUVCDevCtx);
        }
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

