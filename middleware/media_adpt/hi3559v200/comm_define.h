/**
* Copyright (C), 2017-2018, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file    comm_define.h
* @brief   NDK mapi functions declaration
* @author  HiMobileCam NDK develop team
* @date    2018/3/8
*/
#ifndef __COMM_DEFINE_H__
#define __COMM_DEFINE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "hi_mapi_vcap_define.h"
#include "hi_mapi_vproc_define.h"
#include "hi_mapi_venc_define.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"
#include "hi_buffer.h"
#include "hi_comm_vb.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "hi_comm_vo.h"
#include "hi_mapi_disp_define.h"
#include "hi_mapi_disp.h"
#include "hi_mapi_hdmi.h"
#include "hi_mapi_hdmi_define.h"
#include "hi_mapi_acap_define.h"
#include "hi_mapi_acap.h"
#include "hi_mapi_ao_define.h"
#include "hi_mapi_ao.h"
#include "hi_mapi_aenc_define.h"
#include "hi_mapi_aenc.h"



#ifndef CHECK_RET
#define CHECK_RET(express)\
    do{\
        if (HI_SUCCESS != express)\
        {\
            s32Ret = express;\
            printf("\nFailed at %s: LINE: %d  (ret:0x%#x!)\n", __FUNCTION__, __LINE__, s32Ret);\
            goto exit;\
        }\
    }while(0)
#endif


#define ALIGN_UP(x, a)           ( ( ((x) + ((a) - 1) ) / a ) * a )

#define SENSORNUM     (8)
#define VCAP_DEV_0    (0)
#define VCAP_DEV_1    (1)
#define VCAP_DEV_2    (2)
#define VCAP_DEV_3    (3)
#define VCAP_DEV_4    (4)
#define VCAP_DEV_5    (5)
#define VCAP_DEV_6    (6)
#define VCAP_PIPE_0   (0)
#define VCAP_PIPE_1   (1)
#define VCAP_PIPE_2   (2)
#define VCAP_PIPE_3   (3)
#define VCAP_PIPE_4   (4)
#define VCAP_PIPE_5   (5)
#define VCAP_PIPE_5   (5)
#define VCAP_PIPE_6   (6)
#define PIPE_CHN_0    (0)
#define PIPE_CHN_1    (1)

#define VB_SUPPLEMENT_JPEG_MASK         0x1


typedef struct tagSAMPLE_COMM_INITATTR_S
{
    VI_VPSS_MODE_S    stViVpssMode;
    SIZE_S            stResolution;
    HI_U8             u8SnsCnt;
} SAMPLE_COMM_INITATTR_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

