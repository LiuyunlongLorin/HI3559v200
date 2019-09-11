
/**
* @file    hi_product_scene_define.h
* @brief   scene module struct and interface declaration
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date       2017/12/13
* @version   1.0

*/
#ifndef _HI_PRODUCT_SCENE_DEFINE_H_
#define _HI_PRODUCT_SCENE_DEFINE_H_

#include "hi_appcomm.h"
#include "hi_comm_video.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCENE */
/** @{ */  /** <!-- [SCENE] */

/** macro define */
#define HI_PDT_SCENE_PIPE_MAX_NUM                 (2)  /**< pipe max count*/

/**-------------------------------scene ExParam structure-------------------------------- */


/**scene ev enum*/
typedef enum hiPDT_SCENE_EV_E
{
    HI_PDT_SCENE_EV_B2,    /**<(-2) */
    HI_PDT_SCENE_EV_B1_5,  /**<(-1.5) */
    HI_PDT_SCENE_EV_B1,    /**<(-1) */
    HI_PDT_SCENE_EV_B0_5,  /**<(-0.5) */
    HI_PDT_SCENE_EV_0,     /**<(0) */
    HI_PDT_SCENE_EV_0_5,   /**<(0.5) */
    HI_PDT_SCENE_EV_1,     /**<(1) */
    HI_PDT_SCENE_EV_1_5,   /**<(1.5) */
    HI_PDT_SCENE_EV_2,     /**<(2) */
    HI_PDT_SCENE_EV_BUTT
} HI_PDT_SCENE_EV_E;

/** metry mode*/
typedef enum hiPDT_SCENE_METRY_TYPE_E
{
    HI_PDT_SCENE_METRY_TYPE_CENTER = 0,
    HI_PDT_SCENE_METRY_TYPE_AVERAGE,
    HI_PDT_SCENE_METRY_TYPE_SPOT,
    HI_PDT_SCENE_METRY_TYPE_BUTT
} HI_PDT_SCENE_METRY_TYPE_E;

/** spot metry param, indicates the rect range of spot*/
typedef struct hiPDT_SCENE_METRY_SPOT_PARAM_S
{
    POINT_S stPoint; /**<spot position, percent; should be modified with filp/mirror by caller */
    SIZE_S stSize;   /**<size, percent, should be modified with filp/mirror by caller */
} HI_PDT_SCENE_METRY_SPOT_PARAM_S;

/**center metry param */
typedef struct hiPDT_SCENE_METRY_CENTER_PARAM_S
{
    HI_U32 u32Index;
} HI_PDT_SCENE_METRY_CENTER_PARAM_S;

/**average metry param */
typedef struct hPDT_iSCENE_METRY_AVERAGE_PARAM_S
{
    HI_U32 u32Index;
} HI_PDT_SCENE_METRY_AVERAGE_PARAM_S;

/**metry param */
typedef struct hiPDT_SCENE_METRY_S
{
    HI_PDT_SCENE_METRY_TYPE_E enMetryType;  /**<metry type */
    union tagPDT_SCENE_METRY_PARAM_U
    {
        HI_PDT_SCENE_METRY_SPOT_PARAM_S stSpotMetryParam;
        HI_PDT_SCENE_METRY_CENTER_PARAM_S stCenterMetryParam;
        HI_PDT_SCENE_METRY_AVERAGE_PARAM_S stAverageMetryParam;
    } unParam;
} HI_PDT_SCENE_METRY_S;

/**-------------------------------sceneauto structure-------------------------------- */
/**pipe type*/
typedef enum hiPDT_SCENE_PIPE_TYPE_E
{
    HI_PDT_SCENE_PIPE_TYPE_SNAP = 0,
    HI_PDT_SCENE_PIPE_TYPE_VIDEO,
    HI_PDT_SCENE_PIPE_TYPE_BUTT
} HI_PDT_SCENE_PIPE_TYPE_E;

/**pipe mode*/
typedef enum hiPDT_SCENE_PIPE_MODE_E
{
    HI_PDT_SCENE_PIPE_MODE_LINEAR = 0,       /**Linear Mode*/
    HI_PDT_SCENE_PIPE_MODE_WDR,              /**WDR Mode*/
    HI_PDT_SCENE_PIPE_MODE_HDR,              /**When HDR , WDR existed at same time*/
    HI_PDT_SCENE_PIPE_MODE_BUTT
} HI_PDT_SCENE_PIPE_MODE_E;

/**pipe attr*/
typedef struct hiPDT_SCENE_PIPE_ATTR_S
{
    HI_BOOL bEnable;                /**< Pipe Enabel State */
    HI_BOOL bBypassIsp;             /**< isp bypass state */
    HI_BOOL bBypassVpss;            /**< isp bypass state */
    HI_BOOL bBypassVenc;            /**< isp bypass state */
    HI_HANDLE MainPipeHdl;          /**< MainIsp Pipe hdl, Isp in this  pipe can Control sensor */
    HI_HANDLE VcapPipeHdl;          /**< VcapPipe hdl */
    HI_HANDLE PipeChnHdl;           /**< VcapPipe CHn hdl */
    HI_HANDLE VencHdl;              /**< venc hdl */
    HI_HANDLE VpssHdl;              /**< vpss group hdl */
    HI_HANDLE VPortHdl;             /**< vpss chn hdl */
    HI_U8 u8PipeParamIndex;         /**<The index means this pipe param we set , is from the param array we put to the module by bin. the typeindex is the arrayIndex */
    HI_PDT_SCENE_PIPE_TYPE_E enPipeType;/**<pipe type means the pipe used to snap or video, 0 is snap 1 is video. 3DNR and DIS need*/
    HI_PDT_SCENE_PIPE_MODE_E enPipeMode;/**pipe mode*/
} HI_PDT_SCENE_PIPE_ATTR_S;

/**scene mode, an encapsulation of pipe attr array*/
typedef struct hiPDT_SCENE_MODE_S
{
    HI_PDT_SCENE_PIPE_ATTR_S astPipeAttr[HI_PDT_SCENE_PIPE_MAX_NUM];  /**<Pipe Attr */
}HI_PDT_SCENE_MODE_S;


/** @}*/  /** <!-- ==== SCENE DEFINE End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* End of #ifndef __HI_PDT_SCENE_DEFINE_H__ */
