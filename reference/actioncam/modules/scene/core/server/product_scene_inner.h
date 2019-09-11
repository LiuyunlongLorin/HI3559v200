/**
* @file    hi_scene_param.h
* @brief   scene module struct and interface declaration
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date       2017/12/13
* @version   1.0

*/
#include <pthread.h>
#include "hi_product_scene.h"
#include "hi_product_scene_setparam.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifdef CFG_POST_PROCESS

#define PDT_SCENE_HDR_DARK_LINE       (44)
#define PDT_SCENE_HDR_HIGH_LINE       (696)
#define PDT_SCENE_HDR_DARK_PERCENT    (2500)
#define PDT_SCENE_HDR_HIGH_PERCENT    (100)
#define PDT_SCENE_SFNR_ISO_THRESH     (400)
#define PDT_SCENE_MFNR_ISO_THRESH     (2000)
#define PDT_SCENE_DAYNIGHT_ISO_THRESH (1200)

#endif

#define PDT_SCENE_ORIGINAL_EVBIAS    (1024)
#define PDT_SCENE_EXPTIME_LOWERLIMIT_US (500000)
#define PDT_SCENE_EXPTIME_LIMIT_US (30000000)
#define PDT_SCENE_EXPTIME_VIDEOLIMIT_US (30000)
#define PDT_SCENE_MAX_METRYAEWEIGHT_CNT (2)

/**For HDR Ver.1, HDRDarkRatio = 64, as noticed */
#define PDT_SCENE_HDR_DARKRATIO (64)


/**-------------------------------EXPARAM-------------------------------- */
/**aeweight param */
typedef struct tagPDT_SCENE_AEWEIGHT_PARAM_S
{
    HI_U8 au8AEWeight[AE_ZONE_ROW][AE_ZONE_COLUMN];
} PDT_SCENE_AEWEIGHT_PARAM_S;

typedef struct tagPDT_SCENE_EXPARAM_S
{
    PDT_SCENE_AEWEIGHT_PARAM_S astAvergeMetryAeWeight[PDT_SCENE_MAX_METRYAEWEIGHT_CNT];
    PDT_SCENE_AEWEIGHT_PARAM_S astCenterMetryAeWeight[PDT_SCENE_MAX_METRYAEWEIGHT_CNT];
} PDT_SCENE_EXPARAM_S;

/**-------------------------------module special structure-------------------------------- */
typedef struct PDT_SCENE_SPECIALPARAM_S
{
    HI_U32 u32MinimumSysgain;   /** Sensor should be have the MinimumSysgain, with out which should get to saturated value of pixel, default is 1024 * 1.5*/
} PDT_SCENE_SPECIALPARAM_S;


/**scene module params, need to maintance by the module*/
typedef struct tagPDT_SCENE_PARAM_S
{
    HI_PDT_SCENE_PIPE_PARAM_S astPipeParam[PDT_SCENE_PIPETYPE_CNT];
    HI_PDT_SCENE_SNAP_AEROUTE_PARAM_S stSnapAeRouteParam;
    PDT_SCENE_SPECIALPARAM_S stSpecialParam;
    PDT_SCENE_EXPARAM_S stExParam;
} PDT_SCENE_PARAM_S;


/**-------------------------------Inner Used param-------------------------------- */
typedef struct tagPDT_SCENE_THREAD_S
{
    HI_BOOL bThreadFlag;
    pthread_t pThread;
} PDT_SCENE_THREAD_S;

/**mainIsp state*/
typedef struct tagPDT_SCENE_MAINPIPE_STATE_S
{
    HI_HANDLE MainPipeHdl;
    HI_U32 u32SubPipeNum;
    HI_HANDLE aSubPipeHdl[HI_PDT_SCENE_PIPE_MAX_NUM];

    HI_BOOL bMetryFixed;
    HI_BOOL bDISEnable;
    HI_BOOL bLongExp;

    HI_U32 u32LongExpIso;
    HI_U64 u64LastNormalExposure;
    HI_U32 u32LastNormalIso;
    HI_U64 u64LastLuminanceExposure;
    HI_U32 u32LastLuminanceISO;
    HI_U32 u32Iso;
    HI_U64 u64Exposure;
    HI_U32 u32ActRation;
    HI_U32 u32HDRBrightRatio;

    HI_PDT_SCENE_PIPE_MODE_E enPipeMode;
}PDT_SCENE_MAINPIPE_STATE_S;

/**scene state*/
typedef struct tagPDT_SCENE_STATE_S
{
    HI_BOOL bSceneInit;
    HI_BOOL bSupportPhotoProcessing;
    HI_BOOL bPause;
    HI_BOOL bRefreshIgnore;
    PDT_SCENE_THREAD_S stThreadNormal;
    PDT_SCENE_THREAD_S stThreadLuminance;
    pthread_attr_t stThreadnormattr;
    pthread_attr_t stThreadluminanceattr;
    HI_U32 u32MainPipeNum;
    PDT_SCENE_MAINPIPE_STATE_S astMainPipe[HI_PDT_SCENE_PIPE_MAX_NUM];
    HI_PDT_SCENE_LONGEXP_AEROUTE_S stLongExpAeRoute;
} PDT_SCENE_STATE_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


