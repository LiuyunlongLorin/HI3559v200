/**
 * @file    hi_timedtask.h
 * @brief   timed-task module struct and interface declaration.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 * @version   1.0

 */
#ifndef __HI_TIMEDTASK_H__
#define __HI_TIMEDTASK_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     TIMEDTASK */
/** @{ */  /** <!-- [TIMEDTASK] */

/** timed-task callback process function */
typedef HI_S32 (*HI_TIMEDTASK_PROC_CALLBACK_FN_PTR)(HI_VOID* pvPrivData);

typedef struct hiTIMEDTASK_ATTR_S
{
    HI_BOOL bEnable;
    HI_U32 u32Time_sec; /**<timed-task trigger time, canbe reset */
} HI_TIMEDTASK_ATTR_S;

typedef struct hiTIMEDTASK_CFG_S
{
    HI_TIMEDTASK_ATTR_S stAttr;
    HI_TIMEDTASK_PROC_CALLBACK_FN_PTR pfnProc;
    HI_VOID* pvPrivData;
} HI_TIMEDTASK_CFG_S;


/**
 * @brief    timed task module initialization.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_Init(HI_VOID);

/**
 * @brief    timed task module deinitialization.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_Deinit(HI_VOID);

/**
 * @brief    create timed task.
 * @param[in] pstTimeTskCfg : timed task config.
 * @param[out] pTimeTskhdl : timed task handle.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_Create(const HI_TIMEDTASK_CFG_S* pstTimeTskCfg, HI_HANDLE* pTimeTskhdl);

/**
 * @brief    destroy timed task.
 * @param[in] TimeTskhdl : timed task handle.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_Destroy(HI_HANDLE TimeTskhdl);

/**
 * @brief    get timed task attribute.
 * @param[in] TimeTskhdl : timed task handle.
 * @param[out] pstTimeTskCfg : timed task attribute.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_GetAttr(HI_HANDLE TimeTskhdl, HI_TIMEDTASK_ATTR_S* pstTimeTskAttr);

/**
 * @brief    set timed task attribute.
 * @param[in] TimeTskhdl : timed task handle.
 * @param[in] pstTimeTskCfg : timed task attribute.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_SetAttr(HI_HANDLE TimeTskhdl, const HI_TIMEDTASK_ATTR_S* pstTimeTskAttr);

/**
 * @brief    reset specified task check time.
 * @param[in] TimeTskhdl : timed task handle.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_ResetTime(HI_HANDLE TimeTskhdl);

/** @}*/  /** <!-- ==== TIMEDTASK End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_TIMEDTASK_H__ */

