/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_messagehandler.h
* @brief     EVTHUB module header file
* @author    HiMobileCam middleware develop team
* @date      2017.11.30
*/
#ifndef HI_EVENT_HUB_H
#define HI_EVENT_HUB_H
#include "hi_mw_type.h"
#include "hi_error_def.h"
#include "hi_defs.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/** \addtogroup     EVTHUB */
/** @{ */  /** <!-- [EVTHUB] */

typedef enum hiAPP_EVTHUB_ERR_CODE_E
{
    /*general error code*/
    APP_EVTHUB_ERR_HANDLE_INVALID = 0x40,   /**<EVTHUB handle invalid*/
    APP_EVTHUB_ERR_INVALID_ARG = 0x41,      /**<param is null or invalid*/
    APP_EVTHUB_ERR_MALLOC_FAIL = 0x42,      /**<malloc memory fail*/
    APP_EVTHUB_ERR_CREATE = 0x43,           /**<create fail*/
    APP_EVTHUB_ERR_DESTROY = 0x44,          /**<destory fail*/
    APP_EVTHUB_ERR_NOT_CREATE = 0x45,       /**<not created*/
    APP_EVTHUB_ERR_EVENT_NO_RIGEST = 0x46,  /**<state not exist*/
    APP_EVTHUB_ERR_NO_EVENT_HISTORY = 0x47, /**<no history*/
    APP_EVTHUB_ERR_MSGHDL_SEND = 0x48, /**<send msg error*/
    APP_EVTHUB_ERR_BUTT = 0xFF
} HI_APP_EVTHUB_ERR_CODE_E;

/*general error code*/
#define HI_ERR_EVTHUB_NULL_PTR       HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_ERR_NULL_PTR)
#define HI_ERR_EVTHUB_NOT_INIT       HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_ERR_SYS_NOTREADY)
#define HI_ERR_EVTHUB_HANDLE_INVALID HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_EVTHUB_ERR_HANDLE_INVALID)
#define HI_ERR_EVTHUB_INVALIDARG     HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_EVTHUB_ERR_INVALID_ARG)
#define HI_ERR_EVTHUB_MALLOC         HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_EVTHUB_ERR_MALLOC_FAIL)
#define HI_ERR_EVTHUB_CREATE         HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_EVTHUB_ERR_CREATE)
#define HI_ERR_EVTHUB_DESTROY        HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_EVTHUB_ERR_DESTROY)
#define HI_ERR_EVTHUB_NOT_CREATE     HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_EVTHUB_ERR_NOT_CREATE)
#define HI_ERR_EVTHUB_EVENT_NO_RIGEST HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_EVTHUB_ERR_EVENT_NO_RIGEST)
#define HI_ERR_EVTHUB_NO_EVENT_HISTORY HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_EVTHUB_ERR_NO_EVENT_HISTORY)
#define HI_ERR_EVTHUB_MSGHDL_SEND HI_APP_DEF_ERR(HI_APPID_EVTHUB,APP_ERR_LEVEL_ERROR,APP_EVTHUB_ERR_MSGHDL_SEND)

#define MODULE_NAME_EVTHUB  "EVTHUB"

#define EVENT_PAYLOAD_LEN               (512)
#define HI_EVTHUB_SUBSCRIBE_NAME_LEN    (16)
#define HI_EVTHUB_MESSAGEQURUR_MAX_SIZE (32)

typedef HI_U32 HI_EVENT_ID;

typedef struct hiEVENT_S
{
    HI_EVENT_ID EventID;
    HI_S32 arg1;
    HI_S32 arg2;
    HI_S32 s32Result;
    HI_U64 u64CreateTime;
    HI_CHAR aszPayload[EVENT_PAYLOAD_LEN];
} HI_EVENT_S;

typedef struct hiSUBSCRIBER_S
{
    HI_CHAR azName[HI_EVTHUB_SUBSCRIBE_NAME_LEN];
    HI_S32 (*HI_EVTHUB_EVENTPROC_FN_PTR)(HI_EVENT_S* pEvent, HI_VOID* argv);
    HI_VOID* argv;
    HI_BOOL bSync;
}HI_SUBSCRIBER_S;

/**
  *    @brief init eventhub model
  *
  *    init eventhub model, this model is single Instance.
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_Init(HI_VOID);

/**
  *    @brief deinit eventhub model
  *
  *    deinit eventhub model
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_Deinit(HI_VOID);

/**
  *    @brief register event id
  *
  *    register event id
  *    @param[in]  EventID: event ID
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_Register(HI_EVENT_ID EventID);

/**
  *    @brief unregister event id
  *
  *    unregister event id
  *    @param[in]  EventID: event ID
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_UnRegister(HI_EVENT_ID EventID);

/**
  *    @brief publish event
  *
  *    publish event
  *    @param[in]  pEvent: event point
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_Publish(HI_EVENT_S *pEvent);

/**
  *    @brief create subscriber
  *
  *    create subscriber
  *    @param[in]  pstSubscriber: point of subscriber attribute
  *    @param[out]  ppSubscriber: the handle of subscriber
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_CreateSubscriber(HI_SUBSCRIBER_S *pstSubscriber,HI_MW_PTR *ppSubscriber);

/**
  *    @brief destroy subscriber
  *
  *    destroy subscriber
  *    @param[in]  pSubscriber: the handle of subscriber
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_DestroySubscriber(HI_MW_PTR pSubscriber);

/**
  *    @brief subscribe event
  *
  *    subscribe event
  *    @param[in]  pSubscriber: subscribe handle
  *    @param[in]  EventID: event ID
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_Subscribe(HI_MW_PTR pSubscriber,HI_EVENT_ID EventID);

/**
  *    @brief unsubscribe event
  *
  *    unsubscribe event
  *    @param[in]  pSubscriber: subscribe handle
  *    @param[in]  EventID: event ID
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_UnSubscribe(HI_MW_PTR pSubscriber,HI_EVENT_ID EventID);

/**
  *    @brief get event history
  *
  *    get event history
  *    @param[in]  EventID: event id
  *    @param[in]  pEvent: point of event
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_GetEventHistory(HI_S32 EventID,HI_EVENT_S *pEvent);

/**
  *    @brief set enable
  *
  *    set enable
  *    @param[in]  bFlag: flag
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_SetEnabled(HI_BOOL bFlag);

/**
  *    @brief get enable
  *
  *    get enable
  *    @param[in]  pFlag: point of flag
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_EVTHUB_GetEnabled(HI_BOOL *pFlag);

/** @}*/  /** <!-- ==== EVTHUB End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
