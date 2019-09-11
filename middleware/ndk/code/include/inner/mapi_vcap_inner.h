/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_acap_inner.h
 * @brief   server vcap module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MAPI_VCAP_INNER_H__
#define __MAPI_VCAP_INNER_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <pthread.h>
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_log.h"
#include "hi_mapi_vcap_define.h"
#include "hi_mapi_hal_ahd_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/* VCAP DEV ID */
#define VCAP_DEV_0 (0)
#define VCAP_DEV_1 (1)
#define VCAP_DEV_2 (2)
#define VCAP_DEV_3 (3)
#define VCAP_DEV_4 (4)

/* VCAP PIPE ID */
#define VCAP_PIPE_0 (0)
#define VCAP_PIPE_1 (1)
#define VCAP_PIPE_2 (2)
#define VCAP_PIPE_3 (3)
#define VCAP_PIPE_4 (4)
#define VCAP_PIPE_5 (5)

/* PIPE CHN ID */
#define PIPE_CHN_0 (0)
#define PIPE_CHN_1 (1)

/* clock and reset lins max */
#define SENSOR_MAX_CLOCK_DEV (3)
#define SENSOR_MAX_RESET_DEV (3)

/**VCAP PARAM CHECK */
#define CHECK_MAPI_VCAP_NULL_PTR(ptr)                                       \
    do {                                                                    \
        if (NULL == ptr) {                                                  \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "%s is NULL pointer\n", #ptr); \
            return HI_MAPI_VCAP_ENULL_PTR;                                  \
        }                                                                   \
    } while (0)
#define CHECK_MAPI_VCAP_ZERO_VAL(paraname, value)                       \
    do {                                                                \
        if (0 == value) {                                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "%s is zero\n", paraname); \
            return HI_MAPI_VCAP_EILLEGAL_PARA;                          \
        }                                                               \
    } while (0)

#define CHECK_MAPI_VCAP_MAX_VAL(paraname, value, max)                                                         \
    do {                                                                                                      \
        if (value > max) {                                                                                    \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "%s:%d can not larger than max val %d\n", paraname, value, max); \
            return HI_MAPI_VCAP_EILLEGAL_PARA;                                                                \
        }                                                                                                     \
    } while (0)
#define CHECK_MAPI_VCAP_MIN_VAL(paraname, value, min)                                                       \
    do {                                                                                                    \
        if (value < min) {                                                                                  \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "%s:%d can not less than min val %d\n", paraname, value, min); \
            return HI_MAPI_VCAP_EILLEGAL_PARA;                                                              \
        }                                                                                                   \
    } while (0)
#define CHECK_MAPI_VCAP_HANDLE(hdlname, value, max)                                             \
    do {                                                                                        \
        if ((HI_HANDLE)value > max) {                                                           \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "%s [%d][%d]out of range\n", hdlname, value, max); \
            return HI_MAPI_VCAP_EILLEGAL_HANDLE;                                                \
        }                                                                                       \
    } while (0)
#define CHECK_MAPI_VCAP_TRUE(value, str, ret)                             \
    do {                                                                  \
        if (HI_TRUE != value) {                                           \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "%s,ret:[%#x]\n", str, ret); \
            return ret;                                                   \
        }                                                                 \
    } while (0)

#define CHECK_MAPI_VCAP_RET(ret, fmt...)                                      \
    do {                                                                      \
        if (ret != HI_SUCCESS) {                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, fmt);                            \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "fail and return:[%#x]\n", ret); \
            return ret;                                                       \
        }                                                                     \
    } while (0)

#define CHECK_MEDIA_VCAP_ALIGN(value, align)                                              \
    do {                                                                                  \
        if (0 != (value % align)) {                                                       \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "%d doesn't align with %d\n", value, align); \
            return HI_MAPI_VCAP_EILLEGAL_PARA;                                            \
        }                                                                                 \
    } while (0)

#define CHECK_MAPI_VCAP_GOTO(str, value, GOTO_LABLE)       \
    do {                                                   \
        if (value != HI_TRUE) {                            \
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "%s\n", str); \
            goto GOTO_LABLE;                               \
        }                                                  \
    } while (0)

/**VCAP global param */
typedef enum tagVCAP_STATUS_E {
    VCAP_STATUS_NOTINIT,
    VCAP_STATUS_INIT,
    VCAP_STATUS_RUNNING,
    VCAP_STATUS_BUTT
} VCAP_STATUS_E;

typedef enum tagDIS_STATUS_E {
    DIS_STATUS_RUNNING,
    DIS_STATUS_STOP,
    DIS_STATUS_BUTT
} DIS_STATUS_E;

typedef enum tagSWITCH_MODE_E {
    SWITCH_MODE_WDR_LINER_SWITCH,  /* wdr and liner switch */
    SWITCH_MODE_RESOLUTION_SWITCH, /* resolution switch */
    SWITCH_MODE_NO_SENSOR_SWITCH,  /* first run and no wdr, no res, no snsmode switch,  such as pipe num; */
    SWITCH_MODE_BUTT
} SWITCH_MODE_E;

typedef struct tagVCAP_DUMP_RAWDATA_INFO_S {
    HI_BOOL bBNRRaw;
    HI_U32 u32GroupCount;
    HI_U32 u32ExposureTime;
    HI_HANDLE VcapPipeHdl;
    HI_MAPI_VCAP_RAW_DATA_S stVCapRawData;
    HI_MAPI_DUMP_ATTR_S stVcapDumpAttr;
    pthread_t g_pthRawDump;
    pthread_mutex_t s_VcapDumpSyncLock;
    pthread_cond_t s_VcapDumpSyncCond;

} VCAP_DUMP_RAWDATA_INFO_S;

typedef struct tagVCAP_ISP_INIT_ATTR_S {
    HI_BOOL bISPInitAttr;
    ISP_INIT_ATTR_S stISPInitAttr;
} VCAP_ISP_INIT_ATTR_S;

typedef struct tagPIPE_ATTR_S {
    HI_MAPI_PIPE_TYPE_E enPipeType;         /**<static attribute,PIPE attribute */
    FRAME_RATE_CTRL_S stFrameRateCtrl;      /**<dynamic attribute£¬ PIPE attribute */
    COMPRESS_MODE_E enCompressMode;         /**<dynamic attribute£¬ PIPE attribute */
    HI_BOOL bIspBypass;                     /**<static attribute£¬ PIPE attribute */
    VI_PIPE_BYPASS_MODE_E enPipeBypassMode; /**<static attribute£¬ PIPE attribute */
#ifndef __HI3559AV100__
    FRAME_INTERRUPT_ATTR_S stFrameIntAttr; /**<static attribute£¬ PIPE attribute */
#endif
} PIPE_ATTR_S;

typedef struct tagVCAP_OSD_ATTR_S {
    VCAP_STATUS_E enOSDStatus;
    HI_MAPI_OSD_ATTR_S stOSDAttr; /**<attributes corresponding to PIPE channel */
} VCAP_OSD_ATTR_S;

#ifdef SUPPORT_DIS
typedef struct tagVCAP_DIS_ATTR_S {
    DIS_MODE_E enDisMotionType;
    DIS_STATUS_E enDisStatus;
} VCAP_DIS_ATTR_S;
#endif

typedef struct tagVCAP_MOTIONSENSOR_ATTR_S {
#ifdef SUPPORT_GYRO
    HI_S32 s32MotionFd;
    HI_BOOL bInitMotionSensor;
    HI_BOOL bSetDrift;
    HI_U64 u64PhyAddr;
    HI_U64 pVirAddr;
#endif
} VCAP_MOTIONSENSOR_ATTR_S;

typedef struct tagPIPE_CHN_ATTR_S {
    HI_BOOL bChnEnable;
    HI_MAPI_PIPE_CHN_ATTR_S stChnAttr; /**<attributes corresponding to PIPE channel */

    VCAP_OSD_ATTR_S astVcapOSDAttr[HI_MAPI_VCAP_OSD_MAX_NUM];
#ifdef SUPPORT_DIS
    VCAP_DIS_ATTR_S stVcapDisAttr;
#endif
} PIPE_CHN_ATTR_S;

typedef struct tagMAPI_GLOB_PIPE_ATTR_S {
    HI_BOOL bPipeCreate;
    HI_BOOL bPipeStart;
    HI_BOOL bIspRun;
    HI_BOOL bSnapPipeEnable;

    pthread_t pthIspRun;
    HI_HANDLE BindDevHdl;
    HI_HANDLE ISPHdl;

    PIPE_ATTR_S stPipeAttr;
    PIPE_CHN_ATTR_S astPipeChnAttr[HI_MAPI_PIPE_MAX_CHN_NUM]; /**<attributes corresponding to PIPE channel */
    HI_MAPI_PIPE_ISP_ATTR_S stIspPubAttr;                     /**<ISP pub attribute */
    VCAP_ISP_INIT_ATTR_S stVcapISPInitAttr;
    HI_MAPI_VCAP_WB_ATTR_S stVcapISPWbAttr;
    HI_MAPI_VCAP_SNAP_ATTR_S stSnapAttr;
    VCAP_DUMP_RAWDATA_INFO_S stDumpRawInfo; /**<Dump Raw */
} MAPI_GLOB_PIPE_ATTR_S;

typedef struct tagMAPI_GLOB_DEV_ATTR_S {
    VCAP_STATUS_E enDevStatus;
    HI_BOOL bSensorInit;
    HI_MAPI_SENSOR_SCENE_E enSensorScene;
    SWITCH_MODE_E enSwitchMode; /* the enum used to tell the switch mdoe */

    HI_U32 u32PipeBindNum;
    HI_HANDLE aPipeIdBind[HI_MAPI_VCAP_MAX_PIPE_NUM];
    HI_HANDLE MasterISPId;

    HI_MAPI_VCAP_DEV_ATTR_S stDevAttr;
    HI_MAPI_SENSOR_ATTR_S stSensorAttr;
} MAPI_GLOB_DEV_ATTR_S;

typedef struct tagMAPI_VCAP_GLOB_ATTR_S {
    HI_BOOL bVcapInited;
    MAPI_GLOB_DEV_ATTR_S astGlobDevAttr[HI_MAPI_VCAP_MAX_DEV_NUM];
    MAPI_GLOB_PIPE_ATTR_S astGlobPipeAttr[HI_MAPI_VCAP_MAX_PIPE_NUM];

} MAPI_VCAP_GLOB_ATTR_S;

HI_S32 MAPI_VCAP_Client_Init(HI_VOID);
HI_S32 MAPI_VCAP_Client_Deinit(HI_VOID);
HI_S32 MAPI_VCAP_Init(HI_VOID);
HI_S32 MAPI_VCAP_Deinit(HI_VOID);
HI_S32 MAPI_VCAP_GetRawFrame(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pstFrameData, HI_S32 *ps32FrameCnt,
                             HI_BOOL bBNRraw);
HI_S32 MAPI_VCAP_ReleaseRawFrame(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pstFrameData, HI_S32 s32DataNum,
                                 HI_BOOL bBNRraw);
#ifdef SUPPORT_GYRO
HI_BOOL MAPI_VCAP_GetMotionSensorStatus(HI_VOID);
HI_S32 MAPI_VCAP_SetMotionSensor(const HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionAttr);
#endif
HI_HAL_AHD_DEV_S *GetVcapAhdDevInfo(HI_HANDLE VcapDevHdl);
MAPI_GLOB_DEV_ATTR_S *GetVcapDevAttr(HI_HANDLE VcapDevHdl);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __MAPI_VCAP_INNER_H__ */
