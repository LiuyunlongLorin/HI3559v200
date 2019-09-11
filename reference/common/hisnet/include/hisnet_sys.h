/****************************************************************************
*              Copyright 2004 - 2050, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: hisnet_sys.h
* Description: Describe API of sys.
*
* History:
* Version   Date         Author     DefectNum    Description
* B020      2012-09-11   l00228447  NULL         Create this file.
* B030
******************************************************************************/

#ifndef HISNET_SYS_H
#define HISNET_SYS_H

#include "hisnet_type_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef HI_S32 (*Hisnet_SYS_GetDevAttr_FN_PTR)(HISNET_DEVICE_ATTR_S* pstDevAttr);
typedef HI_S32 (*Hisnet_SYS_GetSDStatus_FN_PTR)(HISNET_SDSTATUS_S* pstSDStatus);
typedef HI_S32 (*Hisnet_SYS_SDCommand_FN_PTR)(HISNET_SYS_SDCOMMAND_S* pstSDCommand);
typedef HI_S32 (*Hisnet_SYS_SetSysTime_FN_PTR)(const HISNET_TIME_ATTR_S* pstSysTime);
typedef HI_S32 (*Hisnet_SYS_GetSysTime_FN_PTR)(HISNET_TIME_ATTR_S* pstSysTime);
typedef HI_S32 (*Hisnet_SYS_Reset_FN_PTR)(HI_VOID);
typedef HI_S32 (*Hisnet_SYS_GetBatteryStatus_FN_PTR)(HISNET_BATTERY_STATUS_S* pstBatteryStatus);
typedef HI_S32 (*Hisnet_SYS_SetWifiAttr_FN_PTR)(const HISNET_WIFI_ATTR_S* pstWifiAttr);
typedef HI_S32 (*Hisnet_SYS_GetWifiAttr_FN_PTR)(HISNET_WIFI_ATTR_S* pstWifiAttr);

typedef struct hisnet_SYS_SVR_CB_S
{
    Hisnet_SYS_GetDevAttr_FN_PTR pfnGetDevAttr;
    Hisnet_SYS_GetSDStatus_FN_PTR pfnGetSDStatus;
    Hisnet_SYS_SDCommand_FN_PTR  pfnSDCommand;
    Hisnet_SYS_SetSysTime_FN_PTR pfnSetSysTime;
    Hisnet_SYS_GetSysTime_FN_PTR pfnGetSysTime;
    Hisnet_SYS_Reset_FN_PTR pfnReset;
    Hisnet_SYS_GetBatteryStatus_FN_PTR pfnGetBatteryStatus;
    Hisnet_SYS_SetWifiAttr_FN_PTR pfnSetWifiAttr;
    Hisnet_SYS_GetWifiAttr_FN_PTR pfnGetWifiAttr;
} HISNET_SYS_SVR_CB_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif/* HISNET_SYS_H */
