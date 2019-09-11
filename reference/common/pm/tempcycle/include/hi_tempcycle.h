/**
 * @file    hi_tempcycle.h
 * @brief   tempcycle module interface and struct define.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 * @version   1.0

 */
#include "hi_comm_pm.h"
#include "hi_appcomm.h"
#include "hi_powercontrol.h"

#ifndef __HI_TEMPCYCLE_H
#define __HI_TEMPCYCLE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     TEMPCYCLE */
/** @{ */  /** <!-- [TEMPCYCLE] */
#define TEMPCYCLE_NOTIFY_TEMPERATURE_CHANGE    (1)
#define TEMPCYCLE_SET_PMPARAM                  (2)
#define TEMPCYCLE_NOTIFY_HIGHTEMPERATURE_ALARM (3)


/**The event was published with ONE value*/
/**The value 1 means: form normal temperature to high temprature*/
/**The value 0 means: form high temperature to normal temprature*/
#define HI_EVENT_TEMPCYCLE_TEMP_CHANGE     HI_APPCOMM_EVENT_ID(HI_APP_MOD_PM, 0) /**<temp change event*/

/**The event was published with NO value*/
#define HI_EVENT_TEMPCYCLE_HIGHTEMP_ALARM  HI_APPCOMM_EVENT_ID(HI_APP_MOD_PM, 1) /**<high temp alarm*/

/** struct define */
typedef struct hiTEMPCYCLE_MONITORCFG_S
{
    HI_U32 highTemperature;
    HI_U32 normalTemperature;
    HI_U32 tempLastTime_s;      /**Uint: second. The temperature lasts time, if the current temperature is touched the goal temp in tempLastTime second. Then push event.*/
    HI_U32 tempHighAlarmTime_s; /**Uint: second. If High temperature lasts too long, then push event.*/
} HI_TEMPCYCLE_MONITORCFG_S;

/** function interface */
/**
 * @brief   create temperature cycle check monitor.
 * @param[in] tempMonitorCfg:temp monitor config
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_TEMPCYCLE_Init(const HI_TEMPCYCLE_MONITORCFG_S* tempMonitorCfg);


/**
 * @brief   destroy temperature cycle check monitor.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_TEMPCYCLE_Deinit(HI_VOID);

/**
 * @brief   set media param.
 * @param[in] pstPmParam:pm param
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_TEMPCYCLE_SetPMParam(const HI_MPI_PM_SIGLE_MEDIA_CFG_S* pmParam);

/**
 * @brief   register event.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_TEMPCYCLE_RegisterEvent(HI_VOID);

/** @}*/  /** <!-- ==== OSD End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif
