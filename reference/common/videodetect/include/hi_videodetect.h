/**
* @file    hi_videodetect.h
* @brief    videodetect manager struct and interface
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version  1.0

*/
#ifndef _HI_VIDEODETECT_H_
#define _HI_VIDEODETECT_H_

#include "hi_comm_video.h"
#include "hi_appcomm.h"
#include "hi_md.h"
#include "hi_ive.h"
#include "hi_mapi_comm_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/** macro define */
#define HI_VIDEODETECT_TASK_NUM_MAX            (2)
#define HI_VIDEODETECT_TASK_CALLBACK_NUM_MAX   (2)
#define HI_VIDEODETECT_YUV_DEPTH_MAX           (2)


/** event define */
#define HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE        HI_APPCOMM_EVENT_ID(HI_APP_MOD_VIDEODETECT, 1)


/** error id define */
#define HI_VIDEODETECT_EINVAL                HI_APPCOMM_ERR_ID(HI_APP_MOD_VIDEODETECT, HI_EINVAL) /**< illegal param */
#define HI_VIDEODETECT_EINITIALIZED          HI_APPCOMM_ERR_ID(HI_APP_MOD_VIDEODETECT, HI_EINITIALIZED) /**< already initialed */
#define HI_VIDEODETECT_EFULL                 HI_APPCOMM_ERR_ID(HI_APP_MOD_VIDEODETECT, HI_EFULL) /**< container is full */
#define HI_VIDEODETECT_ENOTINIT              HI_APPCOMM_ERR_ID(HI_APP_MOD_VIDEODETECT, HI_ENOINIT) /**< not init */
#define HI_VIDEODETECT_EBUSY                 HI_APPCOMM_ERR_ID(HI_APP_MOD_VIDEODETECT, HI_EBUSY) /**< task is busy */
#define HI_VIDEODETECT_ENORES                HI_APPCOMM_ERR_ID(HI_APP_MOD_VIDEODETECT, HI_ENORES) /**< no resource ,eg. malloc fail */
#define HI_VIDEODETECT_EOTHER                HI_APPCOMM_ERR_ID(HI_APP_MOD_VIDEODETECT, HI_EOTHER)  /**<other fail */

typedef enum hiVIDEODETECT_ALG_TYPE_E
{
    HI_VIDEODETECT_ALG_TYPE_MD = 0,   /**motion detect*/
    HI_VIDEODETECT_ALG_TYPE_BUTT
} HI_VIDEODETECT_ALG_TYPE_E;

typedef enum hiVIDEODETECT_YUV_MOD_E
{
    HI_VIDEODETECT_YUV_MOD_VCAP = 0,  /**currently not support*/
    HI_VIDEODETECT_YUV_MOD_VPSS,
    HI_VIDEODETECT_YUV_MOD_BUTT
} HI_VIDEODETECT_YUV_MOD_E;

typedef struct hiVIDEODETECT_MD_INFO_S
{
    IVE_CCBLOB_S iveCCBlob;
} HI_VIDEODETECT_MD_INFO_S;

/** get photo param callback set */
typedef struct hiVIDEODETECT_DATA_S
{
    HI_VIDEODETECT_ALG_TYPE_E algType;
    union
    {
        HI_VIDEODETECT_MD_INFO_S mdInfo;
    } unData;
} HI_VIDEODETECT_DATA_S;

typedef HI_S32 (*HI_VIDEODETECT_TASK_CALLBACK_FN_PTR)(HI_VIDEODETECT_DATA_S* svpData, HI_VOID* pPrivateData);

/** get photo param callback set */
typedef struct hiVIDEODETECT_TASK_CALLBACK_S
{
    HI_VIDEODETECT_TASK_CALLBACK_FN_PTR fnGetParamCallBack; /**< task callback */
    HI_VOID* privateData;  /**< task callback private data */
} HI_VIDEODETECT_TASK_CALLBACK_S;

/** yuv src struct */
typedef struct hiVIDEODETECT_YUV_SRC_S
{
    HI_VIDEODETECT_YUV_MOD_E yuvMode;
    HI_HANDLE modHdl;
    HI_HANDLE chnHdl;
    HI_U32 dumpDepth;   /**need or not? FD=1, MD=2*/
} HI_VIDEODETECT_YUV_SRC_S;

/** yuv src struct */
typedef struct hiVIDEODETECT_ALG_ATTR_S
{
    HI_CHAR modelPath[HI_APPCOMM_MAX_PATH_LEN];  /**svp alg model path, FD need ModelPath, MD need not, can be null*/
    HI_VIDEODETECT_ALG_TYPE_E algType;
    union
    {
        MD_ATTR_S mdAttr;
    } unAttr;
} HI_VIDEODETECT_ALG_ATTR_S;

typedef struct hiVIDEODETECT_MD_ATTR_S
{
    HI_S32 sensiValue;
} HI_VIDEODETECT_MD_ATTR_S;

typedef enum hiVIDEODETECT_ALGPROC_TYPE_E
{
    HI_VIDEODETECT_ALGPROC_TYPE_EVENT = 0,
    HI_VIDEODETECT_ALGPROC_TYPE_CALLBACK,
    HI_VIDEODETECT_ALGPROC_TYPE_BUTT
} HI_VIDEODETECT_ALGPROC_TYPE_E;

typedef struct hiVIDEODETECT_ALGPROC_MD_S
{
    HI_U32 mdSensitivity;    /**[0,100]*/
    HI_U32 mdStableCnt;
} HI_VIDEODETECT_ALGPROC_MD_S;

typedef struct hiVIDEODETECT_ALGPROC_CFG_S
{
    HI_VIDEODETECT_ALG_TYPE_E svpAlgType;
    HI_VIDEODETECT_ALGPROC_TYPE_E algProcType;
    HI_VIDEODETECT_TASK_CALLBACK_S taskCB;
    union
    {
        HI_VIDEODETECT_ALGPROC_MD_S algProcMdAttr;
    } unAlgProcAttr;
} HI_VIDEODETECT_ALGPROC_CFG_S;

/** svpmng task configs */
typedef struct hiVIDEODETECT_TASK_CFG_S
{
    HI_U32 algCnt;
    HI_VIDEODETECT_YUV_SRC_S yuvSrc;
    HI_VIDEODETECT_ALG_ATTR_S algAttr[HI_VIDEODETECT_ALG_TYPE_BUTT];   /**svp alg attr*/
} HI_VIDEODETECT_TASK_CFG_S;

/**
* @brief    init videodetect, it should be called first
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_VIDEODETECT_Init(HI_VOID);

/**
* @brief    deinit videodetect
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_VIDEODETECT_Deinit(HI_VOID);

/**
* @brief     create task, it should be called before HI_VIDEODETECT_TaskStart if task has not been created
* @param[in] pstTaskConfig: videodetecttask config
* @param[out] pTaskHdl: videodetect task handle pointer.
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
/**same yuv source has shared one task*/
HI_S32 HI_VIDEODETECT_TaskCreate(const HI_VIDEODETECT_TASK_CFG_S* taskConfig, HI_HANDLE* taskHdl);

/**
* @brief     destroy task
* @param[in] TaskHdl: videodetect task handle
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
/**same yuv source has shared one task*/
HI_S32 HI_VIDEODETECT_TaskDestroy(HI_HANDLE taskHdl);

/**
 * @brief register call back for video encode instance
 * @param[in] VencHdl HI_HANDLE: handle of VENC
 * @param[in] pstVencCB HI_MAPI_VENC_CALLBACK_S: call back function of VENC
 * @return Non 0 indicate failed
 * @return  0 indicate success
 */
HI_S32 HI_VIDEODETECT_AddAlgProc(HI_HANDLE taskHdl, const HI_VIDEODETECT_ALGPROC_CFG_S* algProcCfg);

/**
 * @brief unregister call back for video encode instance
 * @param[in] VencHdl HI_HANDLE: handle of VENC
 * @param[in] pstVencCB HI_MAPI_VENC_CALLBACK_S: call back function of VENC
 * @return Non 0 indicate failed
 * @return  0 indicate success
 */
HI_S32 HI_VIDEODETECT_RemoveAlgProc(HI_HANDLE taskHdl, const HI_VIDEODETECT_ALGPROC_CFG_S* algProcCfg);

/**
* @brief     start videodetect task
* @param[in] TaskHdl: videodetect task handle
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
/**same yuv source has shared one task*/
HI_S32 HI_VIDEODETECT_TaskStart(HI_HANDLE taskHdl);

/**
* @brief     stop videodetect task
* @param[in] TaskHdl: videodetect task handle
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
/**same yuv source has shared one task*/
HI_S32 HI_VIDEODETECT_TaskStop(HI_HANDLE taskHdl);

#if 0
/**
* @brief      directly send frame to do alg process to get result
* @param[in] TaskHdl: svp task handle
* @param[in] enSvpAlgType: svp alg type
* @param[in] pstFrameData: frame data
* @param[out] pstSvpData: svp data.
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
/**Process Send Frame*/
HI_S32 HI_SVPMNG_ProcessSendFrame(HI_HANDLE taskHdl, HI_SVPMNG_ALG_TYPE_E svpAlgType, const HI_MAPI_FRAME_DATA_S* frameData,
                                                HI_SVPMNG_DATA_S* svpData);
#endif

/**
* @brief      directly send frame to do alg process to get result
* @param[in] TaskHdl: videodetect task handle
* @param[in] enSvpAlgType: svp alg type
* @param[in] pstFrameData: frame data
* @param[out] pstSvpData: svp data.
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
/**Register Event*/
HI_S32 HI_VIDEODETECT_RegisterEvent(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif/*_HI_COMM_SVP_ALG_H_*/
