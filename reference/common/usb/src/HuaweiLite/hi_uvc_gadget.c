/*
 * UVC gadget test application
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "core/usb_endian.h"
#include "core/usb.h"
#include <controller/usb_device/usb_gadget.h>
#include "gadget/f_uvc.h"
#include "gadget/usbd_video.h"
#include "gadget/hicamera_control.h"

#include "hi_uvc_gadget.h"
#include "uvc_framebuffer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct tagUVC_DEVICE_CTX_S {
    uvc_t fd;
    HI_U32 bitrate;
    HI_U32 fcc;
    HI_U32 width;
    HI_U32 height;
    HI_U32 stream_used;
} UVC_DEVICE_CTX_S;

extern HI_BOOL g_bPushVencData;
EVENT_CB_S g_frame_event;
static UVC_DEVICE_CTX_S s_stUVCDevCtx;

/* ---------------------------------------------------------------------------*/

static HI_UVC_STREAM_FORMAT_E UVC_FCC2StreamFormat(HI_U32 u32Fcc)
{
    switch (u32Fcc) {
        case V4L2_PIX_FMT_YUYV:
            return HI_UVC_STREAM_FORMAT_YUV420;
        case V4L2_PIX_FMT_MJPEG:
            return HI_UVC_STREAM_FORMAT_MJPEG;
        case V4L2_PIX_FMT_H264:
        default:
            return HI_UVC_STREAM_FORMAT_H264;
    }
}

static HI_U32 UVC_StreamFormat2FCC(HI_UVC_STREAM_FORMAT_E enFormat)
{
    switch (enFormat) {
        case HI_UVC_STREAM_FORMAT_YUV420:
            return V4L2_PIX_FMT_YUYV;
        case HI_UVC_STREAM_FORMAT_MJPEG:
            return V4L2_PIX_FMT_MJPEG;
        case HI_UVC_STREAM_FORMAT_H264:
        default:
            return V4L2_PIX_FMT_H264;
    }
}

static HI_VOID UVC_Clear_Queue(HI_VOID)
{
    UVC_FRAME_NODE_S *node = 0;
    UVC_FRAMEBUF_QUEUES_S *uvc_cache = UVC_FRAMEBUF_GetQueues();
    while (0 == UVC_FRAMEBUF_GetNodeFromQueue(&uvc_cache->stReady, &node)) {
        node->u32used = 0;
        UVC_FRAMEBUF_PutNodeToQueue(&uvc_cache->stIdle, node);
    }
}

static HI_VOID UVC_VideoDisable(UVC_DEVICE_CTX_S *dev)
{
    if (dev->stream_used == 1) {
        UVC_GetStreamOps()->pfnClose();
        dev->stream_used = 0;
    }
}

static HI_VOID UVC_VideoEnable(UVC_DEVICE_CTX_S *dev)
{
    UVC_VideoDisable(dev);
    UVC_Clear_Queue();
    if (dev->stream_used == 0) {
        UVC_STREAM_ATTR_S stStreamAttr;
        stStreamAttr.enFormat = UVC_FCC2StreamFormat(dev->fcc);
        stStreamAttr.u32Width = dev->width;
        stStreamAttr.u32Height = dev->height;
        stStreamAttr.u32Fps = 30;
        stStreamAttr.u32BitRate = dev->bitrate;
        UVC_GetStreamOps()->pfnSetAttr(&stStreamAttr);
        UVC_GetStreamOps()->pfnClose();
        sleep(5);
        UVC_GetStreamOps()->pfnOpen();
        dev->stream_used = 1;
    }
}

static HI_S32 UVC_Continue_Copy(uvc_t hdl, struct uvc_transfer_data *tran, HI_VOID *_stream)
{
    HI_U32 len, offset;
    UVC_FRAME_NODE_S *stream = NULL;
    struct uvc_handle *uvc = (struct uvc_handle *)hdl;
    stream = (UVC_FRAME_NODE_S *)_stream;
    if ((stream == NULL) || (stream->pu8Mem == NULL)) {
        return UVC_ERROR_PTR;
    }
    len = tran->length;
    if (uvc->retransmission == 1) {
        stream->offset = uvc->offset;
        uvc->retransmission = 0;
    }
    uvc->offset = stream->offset;
    offset = stream->offset;
    if (offset >= stream->u32used) {
        tran->length = 0;
        return UVC_OK;
    }
    if ((len + offset) >= stream->u32used) {
        tran->last = 1;
        tran->length = len = stream->u32used - offset;
        stream->offset = stream->u32used;
    } else {
        tran->last = 0;
        stream->offset += len;
    }
    memcpy(tran->data, &(stream->pu8Mem[offset]), len);
    return UVC_OK;
}

static HI_VOID UVC_Video_Fill_Buffer(UVC_DEVICE_CTX_S *dev)
{
    switch (dev->fcc) {
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_YUYV: {
            UVC_FRAMEBUF_QUEUES_S *uvc_cache = UVC_FRAMEBUF_GetQueues();
            UVC_FRAME_NODE_S *node = NULL;
            UVC_FRAME_QUEUE_S *q = 0, *fq = 0;
            if (uvc_cache) {
                q  = &uvc_cache->stReady;
                fq = &uvc_cache->stIdle;
                UVC_FRAMEBUF_GetNodeFromQueue(q, &node);
            }
            if (node != NULL) {
                HI_S32 ret = uvc_video_tran_copy(dev->fd, UVC_Continue_Copy, node);
                if (ret != UVC_OK) {
                    MLOGE("uvc_video_tran_copy faile\n");
                }
                uvc_video_stop(dev->fd);
                node->u32used = 0;
                UVC_FRAMEBUF_PutNodeToQueue(fq, node);
            } else {
                LOS_EventRead(&g_frame_event, 0x01, LOS_WAITMODE_OR | LOS_WAITMODE_CLR, LOS_WAIT_FOREVER);
            }
        }
        break;
        default:
            break;
    }
}

static HI_VOID UVC_EventProcess(UVC_DEVICE_CTX_S *dev)
{
    enum format_switch_status format_state = FORMAT_SWITCH_FINISH;
    format_state = uvc_format_status_check();
    if (format_state == FORMAT_SWITCH_PENDING) {
        HI_S32 ret = 0;
        struct uvc_format_info info;
        ret = uvc_format_info_get(&info);
        format_state = uvc_format_status_check();
        if (ret == UVC_OK) {
            UVC_VideoDisable(dev);
            dev->fcc = info.format;
            dev->height = info.height;
            dev->width = info.width;
            dev->stream_used = 0;
            dev->bitrate = 0;
            MLOGI("handle streamon event,%d w=%d,h=%d\n", UVC_FCC2StreamFormat(dev->fcc), dev->width, dev->height);
            UVC_VideoEnable(dev);
        }
    }
}

static HI_S32 UVC_VideoProcess(UVC_DEVICE_CTX_S *dev)
{
    HI_S32 ret = 0;
    HI_S32 con = 0;
    ret = uvc_wait_host(dev->fd, UVC_WAIT_HOST_NOP, &con);
    if ((ret == UVC_OK) && (con == 0)) {
        MLOGD("USB disconnection, wait connect...\n");
        return 0;
    } else if (ret != UVC_OK) {
        return -1;
    }
    UVC_Video_Fill_Buffer(dev);
    return 1;
}

static HI_S32 UVC_Process(HI_VOID)
{
    UVC_EventProcess(&s_stUVCDevCtx);
    return UVC_VideoProcess(&s_stUVCDevCtx);
}

/* ---------------------------------------------------------------------------*/
HI_VOID UVC_SetDeviceCap(const HI_UVC_DEVICE_CAP_S *pstCap)
{
    HI_S32 i = 0;
    HI_S32 j = 0;
    struct fuvc_format_info format_info;
    struct fuvc_frame_info frames_info[] = {
        {  640,  480, {333333,       0 }, },
        { 1280,  720, {333333,       0 }, },
        { 1920, 1080, {333333,       0 }, },
        {    0,    0, {     0,         }, },
    };
    for (i = 0; i < HI_UVC_STREAM_FORMAT_BUTT; ++i) {
        format_info.fcc = UVC_StreamFormat2FCC(i);
        for (j = 0; j < MIN(pstCap->astFmtCaps[i].u32Cnt, ARRAY_SIZE(frames_info) - 1); ++j) {
            switch (j) {
                case HI_UVC_VIDEOMODE_VGA30: /**<640x480*/
                    frames_info[j].width = 640;
                    frames_info[j].height = 480;
                    break;
                case HI_UVC_VIDEOMODE_720P30:/**<1280x720*/
                    frames_info[j].width = 1280;
                    frames_info[j].height = 720;
                    break;
                case HI_UVC_VIDEOMODE_1080P30:/**<1920x1080*/
                    frames_info[j].width = 1920;
                    frames_info[j].height = 1080;
                    break;
                case HI_UVC_VIDEOMODE_4K30: /**<3840x2160*/
                    frames_info[j].width = 3840;
                    frames_info[j].height = 2160;
                    break;
                default:
                    frames_info[j].width = 0;
                    frames_info[j].height = 0;
            }
        }
        format_info.frames = frames_info;
        fuvc_frame_descriptors_get((struct fuvc_format_info *)&format_info);
    }
    return;
}

HI_BOOL UVC_Is_Connect(VOID)
{
    HI_S32 ret = 0;
    HI_S32 con = 0;
    ret = uvc_wait_host(s_stUVCDevCtx.fd, UVC_WAIT_HOST_NOP, &con);
    if ((ret == UVC_OK) && (con == 1)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

HI_S32 UVC_GADGET_Init(const HI_UVC_DEVICE_CAP_S *pstDevCaps, HI_U32 u32MaxFrameSize)
{
    LOS_EventInit(&g_frame_event);
    return 0;
}

HI_S32 UVC_GADGET_DeviceOpen(const HI_CHAR *pazDevPath)
{
    struct uvc_open_param param;
    uvc_t uvc_hdl;
    HI_S32 ret = 0;
    param.vid_width  = 1920;
    param.vid_height = 1080;
    param.vid_format = UVC_VFF_H264;
    ret = uvc_open_device(&uvc_hdl, &param);
    if (ret != UVC_OK) {
        MLOGE("uvc_open_device fail !!\n");
        return -1;
    }
    s_stUVCDevCtx.fd = uvc_hdl;
    return HI_SUCCESS;
}

HI_S32 UVC_GADGET_DeviceClose(HI_VOID)
{
    UVC_VideoDisable(&s_stUVCDevCtx);
    UVC_Clear_Queue();
    uvc_close_device(s_stUVCDevCtx.fd);
    return HI_SUCCESS;
}

HI_S32 UVC_GADGET_DeviceCheck(HI_VOID)
{
    if (UVC_Is_Connect()) {
        g_bPushVencData = HI_TRUE;
        if (0 >= UVC_Process()) {
            MLOGE("UVC_Process Err ...\n");
            return 0;
        }
        return 1;
    }
    UVC_VideoDisable(&s_stUVCDevCtx);
    sleep(1);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
