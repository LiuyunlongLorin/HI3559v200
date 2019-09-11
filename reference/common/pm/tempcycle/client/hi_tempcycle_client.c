/**
 * @file    hi_tempcycle_client.c
 * @brief   tempcycle module client interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 * @version   1.0

 */
#include "hi_tempcycle.h"

#include "hi_eventhub.h"
#include "hi_appcomm_msg.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


HI_S32 HI_TEMPCYCLE_RegisterEvent(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    ret |= HI_EVTHUB_Register(HI_EVENT_TEMPCYCLE_TEMP_CHANGE);
    ret |= HI_EVTHUB_Register(HI_EVENT_TEMPCYCLE_HIGHTEMP_ALARM);
    return HI_SUCCESS;
}

static HI_S32 TEMPCYCLE_MonitorCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pstPrivData,HI_PWRCTRL_EINVAL);

    HI_EVENT_S event;
    memset(&event, '\0', sizeof(HI_EVENT_S));
    event.EventID = HI_EVENT_TEMPCYCLE_TEMP_CHANGE;
    event.arg1 = pstPrivData->as32PrivData[1];
    ret = HI_EVTHUB_Publish(&event);
    return ret;
}

static HI_S32 TEMPCYCLE_AlarmCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pstPrivData,HI_PWRCTRL_EINVAL);

    HI_EVENT_S event;
    memset(&event, '\0', sizeof(HI_EVENT_S));
    event.EventID = HI_EVENT_TEMPCYCLE_HIGHTEMP_ALARM;
    ret = HI_EVTHUB_Publish(&event);
    return ret;
}

static HI_S32 TEMPCYCLE_RegisterMsgProc(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    HI_S32 msgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_PM, TEMPCYCLE_NOTIFY_TEMPERATURE_CHANGE);
    ret |= HI_MSG_RegisterMsgProc(msgID, TEMPCYCLE_MonitorCallback, NULL);
    msgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_PM, TEMPCYCLE_NOTIFY_HIGHTEMPERATURE_ALARM);
    ret |= HI_MSG_RegisterMsgProc(msgID, TEMPCYCLE_AlarmCallback, NULL);

    return ret;
}

HI_S32 HI_TEMPCYCLE_Init(const HI_TEMPCYCLE_MONITORCFG_S* tempMonitorCfg)
{
    HI_S32 ret = HI_SUCCESS;
    ret |= TEMPCYCLE_RegisterMsgProc();
    return ret;
}

HI_S32 HI_TEMPCYCLE_SetPMParam(const HI_MPI_PM_SIGLE_MEDIA_CFG_S* pmParam)
{
    HI_S32 ret = HI_SUCCESS;
    HI_S32 msgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_PM, TEMPCYCLE_SET_PMPARAM);

    HI_MSG_PRIV_DATA_S privData;
    privData.as32PrivData[0] = TEMPCYCLE_SET_PMPARAM;
    ret = HI_MSG_SendSync(msgID, pmParam, sizeof(HI_MPI_PM_SIGLE_MEDIA_CFG_S), &privData, NULL, 0);
    return ret;
}

HI_S32 HI_TEMPCYCLE_Deinit(HI_VOID)
{
    return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

