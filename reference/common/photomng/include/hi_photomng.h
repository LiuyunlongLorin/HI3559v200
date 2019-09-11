/**
* @file    hi_photomng.h
* @brief    photo manager struct and interface
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version  1.0

*/

#ifndef __HI_PHOTOMNG_H__
#define __HI_PHOTOMNG_H__

#include "hi_mapi_vcap_define.h"
#include "hi_mapi_vproc_define.h"
#ifdef CFG_DNG
#include "hi_dng.h"
#endif
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     PHOTOMNG */
/** @{ */  /** <!-- [PHOTOMNG] */

/** macro define */
#define HI_PHOTOMNG_TASK_NUM_MAX   (2)
#define HI_PHOTOMNG_FILECNT_MAX    (2)     /**< max file names count that get at one time*/

#ifndef DNG_MUXER_DCF_DESCRIPTION_LENGTH
#define DNG_MUXER_DCF_DESCRIPTION_LENGTH      (32)
#endif
#ifndef DNG_MUXER_LENINFO_LEN
#define DNG_MUXER_LENINFO_LEN (4)
#endif

/** error id define */
#define HI_PHOTOMNG_EINVAL                HI_APPCOMM_ERR_ID(HI_APP_MOD_PHOTOMNG, HI_EINVAL) /**< illegal param */
#define HI_PHOTOMNG_EINITIALIZED          HI_APPCOMM_ERR_ID(HI_APP_MOD_PHOTOMNG, HI_EINITIALIZED) /**< already initialed */
#define HI_PHOTOMNG_EFULL                 HI_APPCOMM_ERR_ID(HI_APP_MOD_PHOTOMNG, HI_EFULL) /**< container is full */
#define HI_PHOTOMNG_ENOTINIT              HI_APPCOMM_ERR_ID(HI_APP_MOD_PHOTOMNG, HI_ENOINIT) /**< not init */
#define HI_PHOTOMNG_EBUSY                 HI_APPCOMM_ERR_ID(HI_APP_MOD_PHOTOMNG, HI_EBUSY) /**< task is busy */
#define HI_PHOTOMNG_ENORES                HI_APPCOMM_ERR_ID(HI_APP_MOD_PHOTOMNG, HI_ENORES) /**< no resource ,eg. malloc fail */
#define HI_PHOTOMNG_EOTHER                HI_APPCOMM_ERR_ID(HI_APP_MOD_PHOTOMNG, HI_EOTHER)  /**<other fail */
#define HI_PHOTOMNG_ETIMERCREATE          HI_APPCOMM_ERR_ID(HI_APP_MOD_PHOTOMNG, HI_ERRNO_CUSTOM_BOTTOM + 1) /**< timer create fail */
#define HI_PHOTOMNG_ETAKEPHOTO            HI_APPCOMM_ERR_ID(HI_APP_MOD_PHOTOMNG, HI_ERRNO_CUSTOM_BOTTOM + 2) /**< take photo fail */

typedef struct hiPHOTOMNG_FILENAMES_CTX_S
{
    HI_CHAR szFileName[HI_APPCOMM_MAX_FILENAME_LEN];
    HI_U32 u32PreAllocLen;
} HI_PHOTOMNG_FILENAME_S;

/** get file name callback define,
u32PreAllocLen is 0,it means not preallocate,u32PreAllocLen is larger than 0,it is preallocate file size
*/
typedef HI_S32 (*HI_PHOTOMNG_GETFILENAMES_CALLBACK_FN_PTR)(HI_S32 s32FileCnt, HI_PHOTOMNG_FILENAME_S* pstFileList, HI_VOID* pvPrivateData);

/** photo type enum */
typedef enum hiPHOTOMNG_PHOTO_TYPE_E
{
    HI_PHOTOMNG_PHOTO_TYPE_SINGLE = 0,
    HI_PHOTOMNG_PHOTO_TYPE_LAPSE,
    HI_PHOTOMNG_PHOTO_TYPE_MULTIPLE,
    HI_PHOTOMNG_PHOTO_TYPE_BUTT
} HI_PHOTOMNG_PHOTO_TYPE_E;

/** output format enum */
typedef enum hiPHOTOMNG_OUTPUT_FORMAT_E
{
    HI_PHOTOMNG_OUTPUT_FORMAT_JPEG = 0,
    HI_PHOTOMNG_OUTPUT_FORMAT_DNG,
    HI_PHOTOMNG_OUTPUT_FORMAT_DNG_JPEG,
    HI_PHOTOMNG_OUTPUT_FORMAT_BUTT
} HI_PHOTOMNG_OUTPUT_FORMAT_E;

/** single photo attr */
typedef struct hiPHOTOMNG_SINGLE_ATTR_S
{
    HI_S32 s32Time_sec;        /**< s32TimeSec is 0 that means photo immediately,larger than 0 that means photo delay s32TimeSec second */
    HI_BOOL bNeedTrigger;      /**< need trigger or not*/
} HI_PHOTOMNG_SINGLE_ATTR_S;

/** lapse photo attr */
typedef struct hiPHOTOMNG_LAPSE_ATTR_S
{
    HI_S32 s32Interval_ms;     /**< unit: millisecond */
    HI_BOOL bNeedTrigger;      /**< need trigger or not*/
} HI_PHOTOMNG_LAPSE_ATTR_S;

/** burst photo attr */
typedef struct hiPHOTOMNG_MULTIPLE_ATTR_S
{
    HI_S32 s32Count;                        /**< s32Count is -1 that means continuous photo, larger than 0 that meas photo fix picture*/
} HI_PHOTOMNG_MULTIPLE_ATTR_S;

/** Event Id Define */
typedef enum hiEVENT_PHOTOMNG_E
{
    HI_EVENT_PHOTOMNG_FILE_BEGIN = HI_APPCOMM_EVENT_ID(HI_APP_MOD_PHOTOMNG, 0),
    HI_EVENT_PHOTOMNG_FILE_END,
    HI_EVENT_PHOTOMNG_TASK_STARTED,
    HI_EVENT_PHOTOMNG_TASK_END,
    HI_EVENT_PHOTOMNG_ERROR,
    HI_EVENT_PHOTOMNG_BUTT
} HI_EVENT_PHOTOMNG_E;

/** just for jpeg userinfo */
typedef struct hiPHOTOMNG_JPEG_USERINFO_S
{
    HI_U32 u32UniqueCameraModelLen;
    const HI_VOID* pUniqueCameraModel;       /**< type: 8-bit bytes w/ last byte null, count: u32UniqueCameraModelLen */
    HI_U32 u32MakerNoteLen;
    const HI_VOID* pMakerNote;               /**< type: 8-bit untyped data, count: u32MakerNoteLen */
    HI_U32 u32XMPPacketLen;
    const HI_VOID* pXMPPacket;               /**< type: 8-bit unsigned integer, count: u32XMPPacketLen */
} HI_PHOTOMNG_JPEG_USERINFO_S;

/**just for dng userinfo*/
typedef struct hiPHOTOMNG_DNG_USERINFO_S
{
    HI_CHAR szCameraSerialNumber[DNG_MUXER_DCF_DESCRIPTION_LENGTH];
    HI_CHAR szUniqueCameraModel[DNG_MUXER_DCF_DESCRIPTION_LENGTH];
    HI_FLOAT afLensInfo[DNG_MUXER_LENINFO_LEN];
    HI_U8* pu8DNGPrivData;                  /**< count: any */
    HI_U32 u32DNGPrivDataLen;
    HI_FLOAT* pfProfileHueSatMapData1;      /**< type: FLOAT, count: HueDivisions * SaturationDivisions * ValueDivisions * 3 */
    HI_U32 u32Map1Len;
    HI_FLOAT* pfProfileHueSatMapData2;      /**< type: FLOAT, count: HueDivisions * SaturationDivisions * ValueDivisions * 3 */
    HI_U32 u32Map2Len;
} HI_PHOTOMNG_DNG_USERINFO_S;

/** photo src struct */
typedef struct hiPHOTOMNG_PHOTO_SRC_S
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE VcapPipeChnHdl;
    HI_HANDLE VprocHdl; /**<vpss or stitch */
    HI_HANDLE VportHdl;
    HI_HANDLE VencHdl;
    HI_HANDLE VencThmHdl; /**< video encoder handle2,when output format is dng needed ,it need two venc ,one for thumbnail ,another for  screennail */
} HI_PHOTOMNG_PHOTO_SRC_S;

/** photo param */
typedef struct hiPHOTOMNG_PHOTO_PARAM_S
{
    HI_MAPI_SNAP_EXIF_INFO_S stExifInfo;
    HI_MAPI_VCAP_SNAP_ATTR_S stSnapAttr; /**< snap attr */
    HI_MAPI_PHOTO_TYPE_E enPhotoType;
} HI_PHOTOMNG_PHOTO_PARAM_S;

/** start venc callback define*/
typedef HI_S32 (*HI_PHOTOMNG_VENCSTART_CALLBACK_FN_PTR)(HI_HANDLE VencHdl, HI_S32 s32FrameCnt);

/** stop venc callback define*/
typedef HI_S32 (*HI_PHOTOMNG_VENCSTOP_CALLBACK_FN_PTR)(HI_HANDLE VencHdl);

/** get param callback define*/
typedef HI_S32 (*HI_PHOTOMNG_GETPHOTOPARAM_CALLBACK_FN_PTR)(HI_PHOTOMNG_PHOTO_PARAM_S*  pstPhotoParam, HI_VOID* pvPrivateData);

/** get dng userinfo callback define*/
typedef HI_S32 (*HI_PHOTOMNG_GETDNGUSERINFO_CALLBACK_FN_PTR)(HI_PHOTOMNG_DNG_USERINFO_S* pstDNGUserInfo);

/** get jpeg userinfo callback define*/
typedef HI_S32 (*HI_PHOTOMNG_GETJPEGUSERINFO_CALLBACK_FN_PTR)(HI_PHOTOMNG_JPEG_USERINFO_S* pstJPEGUserInfo);

typedef struct hiPHOTOMNG_VENC_OPERATE_S
{
    HI_PHOTOMNG_VENCSTART_CALLBACK_FN_PTR pfnStart;
    HI_PHOTOMNG_VENCSTOP_CALLBACK_FN_PTR pfnStop;
} HI_PHOTOMNG_VENC_OPERATE_S;

/** photo task attr */
typedef struct hiPHOTOMNG_PHOTO_ATTR_S
{
    HI_PHOTOMNG_PHOTO_TYPE_E enPhotoType;
    HI_PHOTOMNG_PHOTO_SRC_S stPhotoSRC;
    HI_PHOTOMNG_OUTPUT_FORMAT_E enOutPutFmt;
    union tagPhotoTypeAttr
    {
        HI_PHOTOMNG_SINGLE_ATTR_S stSingleAttr;
        HI_PHOTOMNG_LAPSE_ATTR_S stLapseAttr;
        HI_PHOTOMNG_MULTIPLE_ATTR_S stMultipleAttr;
    } unPhotoTypeAttr;
} HI_PHOTOMNG_PHOTO_ATTR_S;

/** get photo file name callback set */
typedef struct hiPHOTOMNG_GET_FILENAME_S
{
    HI_PHOTOMNG_GETFILENAMES_CALLBACK_FN_PTR pfnGetNameCallBack; /**< get filename callback */
    HI_VOID* pvPrivateData; /**< get filename callback private data */
} HI_PHOTOMNG_GET_FILENAME_S;

/** get photo param callback set */
typedef struct hiPHOTOMNG_GET_PHOTOPARAM_S
{
    HI_PHOTOMNG_GETPHOTOPARAM_CALLBACK_FN_PTR pfnGetParamCallBack; /**< get photo param callback */
    HI_VOID* pvPrivateData; /**< get photo param callback private data */
} HI_PHOTOMNG_GET_PHOTOPARAM_S;

/** photo task configs */
typedef struct hiPHOTOMNG_TASK_CFG_S
{
    HI_PHOTOMNG_GET_FILENAME_S stGetNameCB;
    HI_PHOTOMNG_GET_PHOTOPARAM_S stGetParamCB;
} HI_PHOTOMNG_TASK_CFG_S;

/**
* @brief    init photomng,it should be called first
* @param[in] pstVencOps: venc operate callback.
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_PHOTOMNG_Init(const HI_PHOTOMNG_VENC_OPERATE_S* pstVencOps);

/**
* @brief    deinit photomng
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_PHOTOMNG_Deinit(HI_VOID);

/**
* @brief    create task, it should be called before HI_PHOTOMNG_TaskStart if task has not been created
* @param[in] pstTaskConfig: phototask config
* @param[out] pTaskHdl: photo task handle pointer.
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_PHOTOMNG_TaskCreate(const HI_PHOTOMNG_TASK_CFG_S* pstTaskConfig, HI_HANDLE* pTaskHdl);

/**
* @brief    destroy task
* @param[in] TaskHdl:photo task handle.
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_PHOTOMNG_TaskDestroy(HI_HANDLE TaskHdl);

/**
* @brief    start photo task
* @param[in] TaskHdl:photo task handle.
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_PHOTOMNG_TaskStart(HI_HANDLE TaskHdl);

/**
* @brief    stop photo task
* @param[in] TaskHdl:photo task handle.
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_PHOTOMNG_TaskStop(HI_HANDLE TaskHdl);

/**
* @brief set photo task attr.
* @param[in] TaskHdl: photo task handle.
* @param[in] pstPhotoAttr
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_PHOTOMNG_SetPhotoAttr(HI_HANDLE TaskHdl, const HI_PHOTOMNG_PHOTO_ATTR_S* pstPhotoAttr);

/**
* @brief  get photo attr
* @param[in] TaskHdl: photo task handle.
* @param[out] pstPhotoAttr: task attr pointer.
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_PHOTOMNG_GetPhotoAttr(HI_HANDLE TaskHdl, HI_PHOTOMNG_PHOTO_ATTR_S* pstPhotoAttr);

/**
* @brief    register get dng user info  function, it can register repeately and new callback function will replace old
* @param[in] pfnGetDngUserInfoCB: get dng user info callback
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/21
*/
HI_S32 HI_PHOTOMNG_RegGetDNGUserInfoCB(HI_PHOTOMNG_GETDNGUSERINFO_CALLBACK_FN_PTR pfnGetDngUserInfo);

/**
* @brief    register get jpeg user info  function, it can register repeately and new callback function will replace old function
* @param[in] pfnGetJPEGUserInfoCB: get jpeg user info callback
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/21
*/
HI_S32 HI_PHOTOMNG_RegGetJPEGUserInfoCB(HI_PHOTOMNG_GETJPEGUSERINFO_CALLBACK_FN_PTR pfnGetJPEGUserInfo);

/**
* @brief    register photomng event, it can call before function HI_PHOTOMNG_Init
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/27
*/
HI_S32 HI_PHOTOMNG_RegisterEvent(HI_VOID);

/** @}*/  /** <!-- ==== PHOTOMNG End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  /* End of __HI_PHOTOMNG_H__*/
