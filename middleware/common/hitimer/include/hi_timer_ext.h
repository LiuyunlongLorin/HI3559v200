/**
  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
  * @Description  hi timer head file.
  * @Author   HiMobileCam middleware develop team
  * @Create:      2016.06.29
*/

#ifndef HI_TIMER_EXT_H
#define HI_TIMER_EXT_H

#include <pthread.h>
#include "hi_mw_type.h"
#include <sys/time.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
typedef void *HI_TIMER_HANDLE;
typedef void HI_TIMER_PROC(HI_VOID *client_data, struct timeval *nowP);

/** timer config info */
typedef struct hiTIMER_CONF {
    struct timeval *now;       /**< timer create time point */
    long interval_ms;          /**< periodic timer timeout interval or one shot timer timeout interval */
    HI_BOOL periodic;          /**< periodic or ont-shot */
    HI_TIMER_PROC *timer_proc; /**< time out call back */
    HI_VOID *clientData;
} HI_TIMER_S;

/* ---------------------------------------------
                  Function Claim
   --------------------------------------------- */
/**
  * @brief  init a  timer group.
  * @param[in] bBlock :   timer group flag.
  * @return timer group handle.
  * @return failure init failure.
  */
HI_S32 HI_Timer_Init(HI_BOOL bBlock);

/**
 * @brief  deinit  timer group.
 * @param[in] grpHdl :   timer group handle.
 * @return 0  deinit Successs.
 * @return non 0  error happened
 */
HI_S32 HI_Timer_DeInit(HI_S32 grpHdl);

/**
 * @brief  create a  timer.
 * @param[in] grpHdl :   timer group handle.
 * @param[in] timerConf : timer attribute.
 * @return non 0  create Successs and return timer handle.
 * @return 0 create failed.
 */
HI_MW_PTR HI_Timer_Create(HI_S32 grpHdl, HI_TIMER_S *timerConf);

/**
 * @brief  reset timer.
 * @param[in] grpHdl :   timer group handle.
 * @param[in] tmrHdle : timer to reset.
 * @return 0  destroy Successs.
 * @return non 0  error happened
 */
HI_S32 HI_Timer_Reset(HI_S32 grpHdl, HI_MW_PTR tmrHdle, struct timeval *timeVal, HI_U32 timeLen);

/**
 * @brief  set tickvalue  of timer group.
 * @param[in] grpHdl : timer group handle.
 * @param[in] u32TickVal_us : tick value.
 * @return 0  set Successs.
 * @return non 0  error happened
 */
HI_S32 HI_Timer_SetTickValue(HI_S32 grpHdl, HI_U32 u32TickVal_us);

/**
 * @brief  destory timer.
 * @param[in] grpHdl : timer group handle.
 * @param[in] tmrHdle : timer to destroy.
 * @return 0  destroy Successs.
 * @return non 0  error happened
 */
HI_S32 HI_Timer_Destroy(HI_S32 grpHdl, HI_MW_PTR tmrHdle);

/**
 * @brief  clean up  timers group.
 * @param[in] grpHdl : timers group handle.
 * @return 0  set Successs.
 * @return non 0  error happened
 */
HI_S32 HI_Timer_CleanUp(HI_S32 grpHdl);

/**
 * @brief  set periodic attr of timer.
 * @param[in] tmrHdle : timer handle.
 * @param[out] periodic : flag of peridic.
 * @return 0  set Successs.
 * @return non 0  error happened
 */
HI_S32 HI_Timer_SetPeriodicAttr(HI_MW_PTR tmrHdle, HI_BOOL periodic);

/**
 * @brief  get past time of timer.
 * @param[in] tmrHdle : timer handle.
 * @param[out] pu32Time : pointer to past time in million second.
 * @return 0  get past time Successs.
 * @return non 0  error happened
 */
HI_S32 HI_Timer_GetPastTime(HI_MW_PTR tmrHdle, HI_U32 *pu32Time);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

