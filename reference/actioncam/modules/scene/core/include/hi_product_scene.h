/**
* @file    hi_product_scene.h
* @brief   scene module struct and interface declaration
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date       2017/12/13
* @version   1.0

*/

#ifndef _HI_PRODUCT_SCENE_H_
#define _HI_PRODUCT_SCENE_H_

#include "hi_product_scene_define.h"
#include "hi_eventhub.h"
#include "hi_ipcmsg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCENE */
/** @{ */  /** <!-- [SCENE] */

/** error code define */
#define HI_PDT_SCENE_EINVAL         HI_APPCOMM_ERR_ID(HI_APP_MOD_SCENE, HI_EINVAL)      /**<Invalid argument */
#define HI_PDT_SCENE_EINITIALIZED   HI_APPCOMM_ERR_ID(HI_APP_MOD_SCENE, HI_EINITIALIZED)/**<Already initialized */
#define HI_PDT_SCENE_ENOTINIT       HI_APPCOMM_ERR_ID(HI_APP_MOD_SCENE, HI_ENOINIT)     /**<Not inited */
#define HI_PDT_SCENE_EINTER         HI_APPCOMM_ERR_ID(HI_APP_MOD_SCENE, HI_EINTER)      /**<Internal error */

#define HI_EVENT_PDT_SCENE_FRAMERATE_CHANGE  HI_APPCOMM_EVENT_ID(HI_APP_MOD_SCENE, 1)

/**
 * @brief          Init SCENE module
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_Init(HI_VOID);

/**
 * @brief          SetScenemode, start sceneauto thread.
 * @param[in]   PstSceneMode : Media config(each pipe state), and decide Sceneauto param in which pipe
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_SetSceneMode(const HI_PDT_SCENE_MODE_S* pstSceneMode);

/**
 * @brief          GetSceneMode.
 * @param[out] pstSceneMode : Media config
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_GetSceneMode(HI_PDT_SCENE_MODE_S* pstSceneMode);

/**
 * @brief          Pause sceneauto when isp stoped(in playback state or when reset mediaRoute)
 *                   Note: when Pause is true, the sceneauto thread can's set param executed
 * @param[in]   bEnable: True is Pause, False is Resume
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_Pause(HI_BOOL bEnable);

#ifdef CFG_POST_PROCESS
/**
 * @brief          Get Photo suggest info
 * @param[in]   VcapPipeHdl : Snap PipeHdl
 * @param[out] pstDetectInfo : SuggestPostProcessInfo
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_GetDetectPostProcessInfo(HI_HANDLE VcapPipeHdl, HI_PDT_SCENE_POSTPROCESS_INFO_S* pstDetectInfo);

/**
 * @brief          Lock AE
 * @param[in]   VcapPipeHdl : Snap PipeHdl
 * @param[in]   bLock :  Lock State
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2018/05/21
 */
HI_S32 HI_PDT_SCENE_LockAE(HI_HANDLE VcapPipeHdl, const HI_BOOL bLock);

/**
 * @brief          Lock AWB
 * @param[in]   VcapPipeHdl : Snap PipeHdl
 * @param[in]   bLock :  Lock State
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2018/05/21
 */
HI_S32 HI_PDT_SCENE_LockAWB(HI_HANDLE VcapPipeHdl, const HI_BOOL bLock);

#endif

/**
 * @brief          Get Long Exp Suggest info
 * @param[in]   VcapPipeHdl : Snap PipeHdl
 * @param[in]   enSnapLongExpType :  Long Exp Choice, PhotoAlg and nightmode state
 * @param[out] pstLongExpParam : LongExp Param
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_GetDetectLongExpInfo(HI_HANDLE VcapPipeHdl, HI_PDT_SCENE_SNAP_LONGEXP_TYPE_E enSnapLongExpType, HI_PDT_SCENE_LONGEXP_PARAM_S* pstLongExpParam);

/**
 * @brief          refresh photo tipe
 * @param[in]   VcapPipeHdl : Snap PipeHdl
 * @param[in]   u8PipeParamIndex : This Pipe use which sceneauto param
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_RefreshPhotoPipe(HI_HANDLE VcapPipeHdl, HI_U8 u8PipeParamIndex);

/**
 * @brief          Get LongExpSysGain by ExpTime
                      note: if need long exp such as: when Exp_Auto and suggested to benable LongExp
                      and do not do postprocess in multi lapse. or if Exp not auto and do postprocess in single
 * @param[in]   VcapPipeHdl : MainIsp Pipe (Sensor Control Pipe)
 * @param[in]   u32ExpTime_us : and exptime. Time is us, range is 500000-3000000?
 * @param[out] pu32SysGain : sysGain
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_GetLongExpGain(HI_HANDLE VcapPipeHdl, HI_U32 u32ExpTime_us, HI_U32* pu32SysGain);

/**
 * @brief          Get LongExpTime by ISO
                      note: if need long exp such as: when Exp_Auto and suggested to benable LongExp
                      and do not do postprocess in multi lapse. or if Exp not auto and do postprocess in single
 * @param[in]   VcapPipeHdl : MainIsp Pipe (Sensor Control Pipe)
 * @param[in]   u32ISO : ISO
 * @param[out] pu32ExpTime_us : ExpTime
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_GetLongExpTime(HI_HANDLE VcapPipeHdl, HI_U32 u32ISO, HI_U32* pu32ExpTime_us);

/**
 * @brief          Tell SceneAuto Module, if need to Adjust DIS Param
 * @param[in]   VcapPipeHdl : MainIsp Pipe (Sensor Control Pipe)
 * @param[in]   bEnable : enable flag
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_SetDIS(HI_HANDLE VcapPipeHdl, HI_BOOL bEnable);

/**
 * @brief          Set Metry
 * @param[in]   VcapPipeHdl : MainIsp Pipe (Sensor Control Pipe)
 * @param[in]   pstMetry : Metry type and param.
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_SetMetry(HI_HANDLE VcapPipeHdl, const HI_PDT_SCENE_METRY_S* pstMetry);

/**
 * @brief          Set ISO
 * @param[in]   VcapPipeHdl : MainIsp Pipe (Sensor Control Pipe)
 * @param[in]   u32ISO : ISO value, range [100,6400], 0 is auto.
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_SetISO(HI_HANDLE VcapPipeHdl, HI_U32 u32ISO);

/**
 * @brief          Set ExpTime, must smaller than 1 / FrameRate
 * @param[in]   VcapPipeHdl : MainIsp Pipe (Sensor Control Pipe)
 * @param[in]   u32ExpTime_us : 0 is auto, must smaller than 1 / FrameRate
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_SetExpTime(HI_HANDLE VcapPipeHdl, HI_U32 u32ExpTime_us);

/**
 * @brief          Set EV
 * @param[in]   VcapPipeHdl : MainIsp Pipe (Sensor Control Pipe)
 * @param[in]   enEV : EV value
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_SetEV(HI_HANDLE VcapPipeHdl, HI_PDT_SCENE_EV_E enEV);

/**
 * @brief          Set WB
 * @param[in]   VcapPipeHdl : MainIsp Pipe (Sensor Control Pipe)
 * @param[in]   u32WB : WB Value,[1600,16000] 0 is auto
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */

HI_S32 HI_PDT_SCENE_SetWB(HI_HANDLE VcapPipeHdl, HI_U32 u32WB);

/**
 * @brief          Deinit SCENE module.
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_Deinit(HI_VOID);

/**
 * @brief          Register SCENE Event.
 * @return        0 success, non-zero error code.
 * @exception   None
 * @author       HiMobileCam Reference Develop Team
 * @date          2017/12/11
 */
HI_S32 HI_PDT_SCENE_RegisterEvent(HI_VOID);
/** @}*/  /** <!-- ==== SCENE End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* End of #ifndef __HI_SCENE_H__ */
