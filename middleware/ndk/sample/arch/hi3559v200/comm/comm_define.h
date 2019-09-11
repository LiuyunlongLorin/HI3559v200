/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    comm_define.c
 * @brief   sample define function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#ifndef __COMM_DEFINE_H__
#define __COMM_DEFINE_H__

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
#include "hi_comm_video.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "hi_comm_vo.h"
#include "hi_mapi_disp_define.h"
#include "hi_mapi_disp.h"

#ifdef SUPPORT_HDMI
#include "hi_mapi_hdmi.h"
#include "hi_mapi_hdmi_define.h"
#endif

#include "hi_mapi_acap_define.h"
#include "hi_mapi_acap.h"
#include "hi_mapi_ao_define.h"
#include "hi_mapi_ao.h"
#include "hi_mapi_aenc_adpt.h"
#include "hi_mapi_aenc_define.h"
#include "hi_mapi_aenc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define SAMPLE_U32MAX_NUM 0xFFFFFFFF
#define SAMPLE_U64MAX_NUM 0xFFFFFFFFFFFFFFFF

typedef enum SAMPLE_SENSOR_SEQ_E {
    SAMPLE_SENSOR_1080P30,
    SAMPLE_SENSOR_4M30,
    SAMPLE_SENSOR_4K30,
    SAMPLE_SENSOR_4K60,
    SAMPLE_SENSOR_12M,
    SAMPLE_SENSOR_DEFAULT, /* it is the typical resolution in every chip */
} SAMPLE_SENSOR_SEQ_E;

#ifndef CHECK_RET
#define CHECK_RET(express)                                                                      \
    do {                                                                                        \
        s32Ret = express;                                                                       \
        if (HI_SUCCESS != s32Ret) {                                                             \
            printf("\nFailed at %s: LINE: %d  (ret:0x%#x!)\n", __FUNCTION__, __LINE__, s32Ret); \
            goto exit;                                                                          \
        }                                                                                       \
    } while (0)
#endif

#ifndef SAMPLE_CHECK_RET
#define SAMPLE_CHECK_RET(express)                                                               \
    do {                                                                                        \
        s32Ret = express;                                                                       \
        if (HI_SUCCESS != s32Ret) {                                                             \
            printf("\nFailed at %s: LINE: %d  (ret:0x%#x!)\n", __FUNCTION__, __LINE__, s32Ret); \
            return s32Ret;                                                                      \
        }                                                                                       \
    } while (0)
#endif

#define SAMPLE_CHECK_U32PRODUCT_OVERFLOW(value1, value2)             \
    do {                                                             \
        if ((HI_U32)SAMPLE_U32MAX_NUM / (value1) < (value2)) {       \
            printf("\n%u * %u product owerflow!\n", value1, value2); \
            return HI_FAILURE;                                       \
        }                                                            \
    } while (0)

#define SAMPLE_CHECK_U64SUM_OVERFLOW(value1, value2)                 \
    do {                                                             \
        if ((HI_U64)SAMPLE_U64MAX_NUM - (value1) < (value2)) {       \
            printf("\n%llu + %llu sum owerflow!\n", value1, value2); \
            return HI_FAILURE;                                       \
        }                                                            \
    } while (0)


#define SENSORNUM  (8)
#define VCAP_DEV_0 (0)
#define VCAP_DEV_1 (1)

#define VCAP_DEV_3  (3)
#define VCAP_DEV_4  (4)
#define VCAP_DEV_5  (5)
#define VCAP_DEV_6  (6)
#define VCAP_PIPE_0 (0)
#define VCAP_PIPE_1 (1)
#define VCAP_PIPE_2 (2)
#define VCAP_PIPE_3 (3)
#define VCAP_PIPE_4 (4)
#define VCAP_PIPE_5 (5)
#define VCAP_PIPE_5 (5)
#define VCAP_PIPE_6 (6)
#define VCAP_PIPE_7 (7)

#define PIPE_CHN_0 (0)
#define PIPE_CHN_1 (1)

#define VPORT_16BPP (0)
#define VCAP_DEV_2  (2)

#define VB_SUPPLEMENT_JPEG_MASK 0x1

typedef struct tagSAMPLE_COMM_INITATTR_S {
    VI_VPSS_MODE_S stViVpssMode;
    SIZE_S stResolution;
    HI_U8 u8SnsCnt;
} SAMPLE_COMM_INITATTR_S;

typedef struct tagSAMPLE_LUT_BUFFER_S {
    HI_U64 u64PhyAddr;
    HI_VOID *VirAddr;
    HI_U32 u32BufLen;
} SAMPLE_LUT_BUFFER_S;

typedef enum tagSAMPLE_VB_CFG_MODE_E {
    VB_MODE_1 = 0, /* not stitch */
    VB_MODE_2,     /* stitch */
} SAMPLE_VB_CFG_MODE_E;

typedef struct hiSAMPLE_MODE_COMM_CONFIG_S {
    const HI_MAPI_SENSOR_ATTR_S *pstVcapSensorAttr;
    const HI_MAPI_VCAP_DEV_ATTR_S *pstVcapDevAttr;
    const HI_MAPI_VCAP_PIPE_ATTR_S *pstVcapPipeAttr;
    const HI_MAPI_PIPE_CHN_ATTR_S *pstPipeChnAttr;
    const HI_MAPI_VPSS_ATTR_S *pstVpssAttr;
    const HI_MAPI_VPORT_ATTR_S *pstVPortAttr;
    const HI_MAPI_VENC_ATTR_S *pstRecVencAttr;
    const HI_MAPI_VENC_ATTR_S *pstSnapVencAttr;
} SAMPLE_MODE_COMM_CONFIG_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

