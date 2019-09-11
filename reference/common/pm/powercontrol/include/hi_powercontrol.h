/**
* @file    hi_powercontrol.h
* @brief   product powercontrol struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/
#ifndef __HI_POWERCONTROL_H__
#define __HI_POWERCONTROL_H__

#include "hi_type.h"
#include "hi_appcomm.h"
#include "hi_timedtask.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     POWERCONTROL */
/** @{ */  /** <!-- [POWERCONTROL] */
/** macro define */
#define HI_PWRCTRL_EINVAL            HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_EINVAL)    /**<parm invlid*/
#define HI_PWRCTRL_EINTER            HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_EINTER)    /**<intern error*/
#define HI_PWRCTRL_ENOINIT           HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_ENOINIT)  /**< no initialize*/
#define HI_PWRCTRL_EBUSY             HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_EBUSY)  /**<service busy*/
#define HI_PWRCTRL_EINITIALIZED      HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_EINITIALIZED) /**< already initialized */
#define HI_PWRCTRL_ETIMEDTASK        HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_ERRNO_CUSTOM_BOTTOM) /**<timedtask module error*/
#define HI_PWRCTRL_EWAKEUPCB         HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_ERRNO_CUSTOM_BOTTOM+1) /**<wakeup callback error*/
#define HI_PWRCTRL_EDORMANTCB        HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_ERRNO_CUSTOM_BOTTOM+2) /**<dormant callback error*/
#define HI_PWRCTRL_ELOGICFLOW        HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_ERRNO_CUSTOM_BOTTOM+3) /**<logical flow error*/
#define HI_PWRCTRL_EFATA             HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_ERRNO_CUSTOM_BOTTOM+4) /**<fata error*/

/**set screen time > sys time,sys time self-adjusting with screen time;
   set sys time < screen timescreen time self-adjusting with screen time*/
#define HI_PWRCTRL_ETASKTIMEAUTO     HI_APPCOMM_ERR_ID(HI_APP_MOD_PM,HI_ERRNO_CUSTOM_BOTTOM+5)


/**  ==== power control task configure begin ==== */

typedef HI_S32 (*HI_PWRCTRL_TASK_PROC_CALLBACK_FN_PTR)(HI_VOID* pvPrivData);/**< dormant callback process function */

typedef enum hiPWRCTRL_TASK_E /**<pwrctrl task type */
{
    HI_PWRCTRL_TASK_SCREENDORMANT = 0,
    HI_PWRCTRL_TASK_SYSTEMDORMANT,
    HI_PWRCTRL_TASK_BUIT
}HI_POWERCTRL_TASK_E;

typedef struct hiPWRCTRL_TASK_CFG__S /**< task static attribute */
{
    HI_TIMEDTASK_ATTR_S stAttr;
    HI_PWRCTRL_TASK_PROC_CALLBACK_FN_PTR pfnDormantProc;
    HI_VOID* pvDormantPrivData;
    HI_PWRCTRL_TASK_PROC_CALLBACK_FN_PTR pfnWakeupProc;
    HI_VOID* pvWakeupPrivData;
} HI_PWRCTRL_TASK_CFG_S;

typedef struct hiPWRCTRL_CFG__S
{
    HI_PWRCTRL_TASK_CFG_S astTaskCfg[HI_PWRCTRL_TASK_BUIT];
} HI_PWRCTRL_CFG_S;

/** ==== power control task attribute end===*/


/**  ==== event configuue begin ==== */
typedef enum hiPWRCTRL_EVENT_TYPE_E /**<event type */
{
    HI_PWRCTRL_EVENT_TYPE_WAKEUP = 0,    /**<wakeup dormant*/
    HI_PWRCTRL_EVENT_TYPE_CONTROL,       /**<pause or resumme dormant check */
    HI_PWRCTRL_EVENT_TYPE_COMMON,        /**<general event,no both of wakeup and control function */
    HI_PWRCTRL_EVENT_TYPE_BUIT
}HI_POWERCTRL_EVENT_TYPE_E;


typedef enum hiPWRCTRL_EVENT_SCOPE_E /**< event action scope */
{
    /**control event: control(pause or resume) system dormant check at normal state(no system dormant)
       common event:  do not continue handle the event at sys dormant
       wakeup event:  wakeup sstem at sys dormant  */
    HI_PWRCTRL_EVENT_SCOPE_SYSTEM = 0,
    /**control event: control(pause or resume) system dormant and screen dormant check at normal state
       common event:  do not continue handle the event at sys dormant or screen dormant
       wakeup event:  wakeup sstem at sys dormant or screen dormant */
    HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN,
    HI_PWRCTRL_EVENT_SCOPE_BUIT
}HI_PWRCTRL_EVENT_SCOPE_E;


typedef struct hiPWRCTRL_EVENT_COMMON_ATTR_S/**< common event attribute */
{
    HI_PWRCTRL_EVENT_SCOPE_E enType;
    HI_BOOL bResetTimer;
} HI_PWRCTRL_EVENT_COMMON_ATTR_S;


typedef enum hiPWRCTRL_WAKEUP_TACTICS_E /**< wakeup event process tactics type,whether event need continue proc or not*/
{
    HI_PWRCTRL_WAKEUP_TACTICS_DISCARD = 0, /**<after dong wakeup,event need not continue to proc*/
    HI_PWRCTRL_WAKEUP_TACTICS_CONTINUE,    /**<after dong wakeup,event need continue to proc*/
    HI_PWRCTRL_WAKEUP_TACTICS_BUIT
}HI_PWRCTRL_WAKEUP_TACTICS_E;


typedef struct hiPWRCTRL_EVENT_WAKEUP_ATTR_S/**< wakeup event attribute */
{
    HI_PWRCTRL_WAKEUP_TACTICS_E enType;
    HI_PWRCTRL_EVENT_COMMON_ATTR_S stCommonCfg;
} HI_PWRCTRL_EVENT_WAKEUP_ATTR_S;


typedef enum hiPWRCTRL_EVENT_CONTROL_E/**< control event type of action*/
{
    HI_PWRCTRL_EVENT_CONTROL_PAUSE = 0,/**<after wakeup ,ui need not to proc*/
    HI_PWRCTRL_EVENT_CONTROL_RESUME,/**<after wakeup ,ui continue to proc*/
    HI_PWRCTRL_EVENT_CONTROL_BUIT
}HI_PWRCTRL_EVENT_CONTROL_E;



typedef struct hiPWRCTRL_EVENT_CONTROL_ATTR_S/**< control event attribute*/
{
    HI_PWRCTRL_EVENT_CONTROL_E enType;
    HI_PWRCTRL_EVENT_COMMON_ATTR_S stCommonCfg;
} HI_PWRCTRL_EVENT_CONTROL_ATTR_S;


typedef struct hiPWRCTRL_EVENT_ATTR_S/**< event configure */
{
    HI_POWERCTRL_EVENT_TYPE_E enType;
    union tagPWRCTRL_EVENT_ATTR_U
    {
        HI_PWRCTRL_EVENT_WAKEUP_ATTR_S stWakeupCfg;/**<wakeup event cfg*/
        HI_PWRCTRL_EVENT_CONTROL_ATTR_S stCtrlCfg;/**<control event cfg*/
        HI_PWRCTRL_EVENT_COMMON_ATTR_S stCommonCfg;
    } unCfg;
} HI_PWRCTRL_EVENT_ATTR_S;

/**  ==== event configuue end ==== */


/**
* @brief  power control init,get cfg from parm module and invoke timetask interface
* @param[in] pfnSysDormantProc,invoke pfnSysDormantProc when check the system is on standby,pwrctrl proc screen drmant independent
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/1/10
*/
HI_S32 HI_POWERCTRL_Init(const HI_PWRCTRL_CFG_S* pstCfg);


/**
* @brief   get task dynamic attribute:dormant time and enable
* @param[in] enType :task type
* @param[in] pstTaskAttr:include task switch and task time
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/1/10
*/
HI_S32 HI_POWERCTRL_GetTaskAttr(HI_POWERCTRL_TASK_E enType,HI_TIMEDTASK_ATTR_S* pstTaskAttr);

/**
* @brief   set task dynamic attribute:dormant time and enable
* @param[in] enType :task type
* @param[in] pstTaskAttr:include task switch and task time
* @return HI_SUCCESS              :success,
          HI_PWRCTRL_ETASKTIMEAUTO:sys and screen time Adjust to equal
          HI_FAILURE              :error
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/1/10
*/
HI_S32 HI_POWERCTRL_SetTaskAttr(HI_POWERCTRL_TASK_E enType,const HI_TIMEDTASK_ATTR_S* pstTaskAttr);


/**
* @brief    event pretreatment,
* @param[in] pstEventCfg: event info
* @param[out] pbEventContinueHandle: true:continue proc the event,false:discard the event
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/1/10
*/
HI_S32 HI_POWERCTRL_EventPreProc(const HI_PWRCTRL_EVENT_ATTR_S* pstEventAttr,HI_BOOL* pbEventContinueHandle);


/**
* @brief   invoke timetask interface to exit dormant check && local resource release;
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/1/10
*/
HI_S32 HI_POWERCTRL_Deinit(HI_VOID);

/** @}*/  /** <!-- ==== POWERCONTROL End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_WIFI_H__*/
