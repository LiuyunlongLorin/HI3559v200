/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2017>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#ifndef LITEOS_USB_DEVICE_VIDEO_H
#define LITEOS_USB_DEVICE_VIDEO_H

#include <los_typedef.h>

/* UVC function error codes */
#define UVC_OK                   0x00     /* No error, function has successfully completed */
#define UVC_ERROR_NOMATCH        0x01     /* The operation can not be complete, due to mismatch of UVC state */
#define UVC_ERROR_HANDLE         0x02     /* Mismatch of UVC handle, UVC can be obtained only once before calling`close */
#define UVC_ERROR_PTR            0x03     /* Invalid pointer */
#define UVC_ERROR_MEMORY         0x04     /* Memory error */
#define UVC_ERROR_VALUE          0x05     /* Incorrect value */
#define UVC_ERROR_FATAL          0x06     /* Fatal internal error, please contact the one who wrote the code */

/* UVC video frame format */
#define UVC_VFF_YUY2             0x00
#define UVC_VFF_H264             0x01
#define UVC_VFF_MJPG             0x02

#define UVC_INVALID_HANDLE          0xFFFFFFFF

/*
 * parameter structure used to open a Video handle (UVC device)
 */
struct uvc_open_param {
#define UVC_VIDEO_WIDTH_DEF      1920
#define UVC_VIDEO_HEIGHT_DEF     1080
#define UVC_VIDEO_WIDTH_MAX      4096
#define UVC_VIDEO_HEIGHT_MAX     4096
    uint32_t vid_width;          /* width of the video frame, in pixels */
    uint32_t vid_height;         /* height of the video frame, in pixels */
    uint32_t vid_format;         /* format of video frame */

    uint32_t bulk_size;
    uint32_t imgsize;
};

enum format_switch_status
{
    FORMAT_SWITCH_FINISH = 0,
    FORMAT_SWITCH_PENDING
};

/*
 * format info
 */
struct uvc_format_info
{
    uint32_t width;
    uint32_t height;
    uint32_t format;

    uint32_t status;
} __attribute__((packed));

/* type-define UVC handle as pointer type */
typedef void * uvc_t;

/* open the UVC device, and returns a handle */
extern int uvc_open_device(uvc_t * hdl, struct uvc_open_param * param);

/* closes a UVC handle */
extern int uvc_close_device(uvc_t uvc);

/* check the frame switch status */
extern enum format_switch_status uvc_format_status_check(void);

extern int uvc_format_info_get(struct uvc_format_info *info);

#define UVC_WAIT_HOST_NOP      0x0
#define UVC_WAIT_HOST_FOREVER  0x1
/*
 * Wait for UVC USB host connection
 */
extern int uvc_wait_host(uvc_t uvc, int wait_option, int * connected);

/* UVC device state definitions */
enum uvc_state {
    UVC_STATE_IDLE          = 0x01,     /* UVC device has been opened, but not connected to USB host */
    UVC_STATE_CONN          = 0x02,     /* UVC device has connected to USB host, but not trasmitting video data */
    UVC_STATE_TRAN_COPY     = 0x04,     /* UVC device has connected to USB host, and transmitting video data */
    UVC_STATE_TRAN_NOCP     = 0x08,
    UVC_STATE_ERR           = 0x10,     /* indicates that UVC device has encounterred an error */
    UVC_STATE_NONE          = 0x20      /* Not used */
};

/*
 * the function returns the operating state of UVC device
 */
extern /* enum uvc_state */ int uvc_get_state(uvc_t uvc, uint32_t * state);

/* structure used for callback furnction, (partial) frame data;
 * structure used to describe next complete frame data.
 * quite simple, is it not ?
*/
struct uvc_transfer_data {
    unsigned char * data;      /* UVC_STATE_TRAN_COPY: copy target address; UVC_STATE_TRAN_NOCP: NULL */
    uint32_t length;           /* length of data */
    uint32_t last;             /* last piece of data in the frame? (ignored when UVC_STATE_TRAN_NOCP) */
};

/*
 * callback function proto-type, copy (partial) video frame data
 * callback function proto-type, used while transmitting single
 * frame of video data one by one
 */
typedef int (* uvc_continue_func)(uvc_t uvc, struct uvc_transfer_data * td, void * priv);

/* start video transmission, copy method */
extern int uvc_video_tran_copy(uvc_t uvc, uvc_continue_func copy_func, void * priv);

/* start video transmission, no-copy method */
extern int uvc_video_tran_nocp(uvc_t uvc, uvc_continue_func next_func, void * priv);

/* stop stransmission */
extern int uvc_video_stop(uvc_t uvc);
void usb_format_yuv_semiplanar_420(char *BufVirt_Y, char *BufVirt_C,
    unsigned char *frame_mem, uint32_t frame_len, uint32_t frame_height,
    uint32_t frame_width);

#endif

