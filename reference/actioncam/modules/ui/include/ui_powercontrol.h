/**
 * @file      ui_powercontrol.h
 * @brief     ui power control interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#ifndef __UI_POWERCONTROL_H__
#define __UI_POWERCONTROL_H__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "hi_powercontrol.h"

#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "UI"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

HI_S32 PDT_UI_POWERCTRL_Init(HI_VOID);
HI_S32 PDT_UI_POWERCTRL_Deinit(HI_VOID);
HI_BOOL PDT_UI_POWERCTRL_IsScreenDormant(HI_VOID);
HI_BOOL PDT_UI_POWERCTRL_IsSystemDormant(HI_VOID);
HI_S32 PDT_UI_POWERCTRL_PreProcessEvent(HI_EVENT_S* pstEvent, HI_BOOL* pbEventContinue);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
