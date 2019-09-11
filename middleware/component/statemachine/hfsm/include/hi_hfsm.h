/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_hfsm.h
* @brief     HFSM module header file
* @author    HiMobileCam middleware develop team
* @date      2017.11.30
*/
#ifndef HI_HIERARCHICAL_STATE_MACHINE_H
#define HI_HIERARCHICAL_STATE_MACHINE_H

#include "hi_mw_type.h"
#include "hi_defs.h"
#include "hi_error_def.h"
#include "hi_message.h"
#include "hi_type.h"

/** \addtogroup     HFSM */
/** @{ */  /** <!-- [HFSM] */

typedef enum hiAPP_HFSM_ERR_CODE_E
{
    /*general error code*/
    APP_HFSM_ERR_HANDLE_INVALID = 0x40,   /**<HFSM handle invalid*/
    APP_HFSM_ERR_INVALID_ARG = 0x41,      /**<param is null or invalid*/
    APP_HFSM_ERR_MALLOC_FAIL = 0x42,      /**<malloc memory fail*/
    APP_HFSM_ERR_CREATE = 0x43,           /**<create HFSM fail*/
    APP_HFSM_ERR_DESTROY = 0x44,          /**<destory HFSM  fail*/
    APP_HFSM_ERR_NOT_CREATE = 0x45,       /**<HFSM not created*/
    APP_HFSM_ERR_STATE_NO_EXIST = 0x46,   /**<state not exist*/
    APP_HFSM_ERR_ROOT_STATE_EXIST = 0x47, /**<root state exist*/
    APP_HFSM_ERR_STATE_EXIST = 0x48,      /**<state has exist*/
    APP_HFSM_ERR_STATE_ERROR = 0x49,      /**<state error*/
    APP_HFSM_ERR_STATE_EXCEED_MAX_CNT = 0x50,      /**<state error*/
    APP_HFSM_ERR_NOT_START = 0x51,       /**<HFSM not start*/
    APP_HFSM_ERR_MSGHDL_SEND = 0x52,       /**<HFSM send msg error*/
    APP_HFSM_ERR_MSGHDL_START = 0x53,       /**<msg hander start error*/
    APP_HFSM_ERR_MSGHDL_STOP = 0x54,       /**<msg hander stop error*/
    APP_HFSM_ERR_BUTT = 0xFF
} HI_APP_HFSM_ERR_CODE_E;

/*general error code*/
#define HI_ERR_HFSM_NULL_PTR       HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_ERR_NULL_PTR)
#define HI_ERR_HFSM_HANDLE_INVALID HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_HANDLE_INVALID)
#define HI_ERR_HFSM_INVALIDARG     HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_INVALID_ARG)
#define HI_ERR_HFSM_MALLOC         HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_MALLOC_FAIL)
#define HI_ERR_HFSM_CREATE         HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_CREATE)
#define HI_ERR_HFSM_DESTROY        HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_DESTROY)
#define HI_ERR_HFSM_NOT_CREATE     HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_NOT_CREATE)
#define HI_ERR_HFSM_STATE_NO_EXIST HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_STATE_NO_EXIST)
#define HI_ERR_HFSM_ROOT_STATE_EXIST HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_ROOT_STATE_EXIST)
#define HI_ERR_HFSM_STATE_EXIST    HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_STATE_EXIST)
#define HI_ERR_HFSM_STATE_ERROR    HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_STATE_ERROR)
#define HI_ERR_HFSM_STATE_EXCEED_MAX_CNT    HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_STATE_EXCEED_MAX_CNT)
#define HI_ERR_HFSM_NOT_START     HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_NOT_START)
#define HI_ERR_HFSM_MSGHDL_SEND    HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_MSGHDL_SEND)
#define HI_ERR_HFSM_MSGHDL_START    HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_MSGHDL_START)
#define HI_ERR_HFSM_MSGHDL_STOP    HI_APP_DEF_ERR(HI_APPID_HFSM,APP_ERR_LEVEL_ERROR,APP_HFSM_ERR_MSGHDL_STOP)


#define MODULE_NAME_HFSM  "HFSM"

#define HI_STATE_NAME_LEN                (64)
#define HI_STATE_MAX_AMOUNT              (32)
#define HI_PROCESS_MSG_RESULTE_OK        HI_SUCCESS
#define HI_PROCESS_MSG_UNHANDLER         (-1)

typedef HI_U32 HI_STATE_ID;

typedef struct hiSTATE_S{
    HI_STATE_ID stateID;                        /**<state id*/
    HI_CHAR name[HI_STATE_NAME_LEN];            /**<state name*/
    HI_S32 (*enter)(HI_MW_PTR argv);            /**<call when state enter*/
    HI_S32 (*exit)(HI_MW_PTR argv);             /**<call when state exit*/
    HI_S32 (*processMessage)(HI_MESSAGE_S *pMsg, HI_MW_PTR argv, HI_STATE_ID *pStateID); /**<call when process a message*/
    HI_MW_PTR argv;                             /**<User private, used by processMessage*/
}HI_STATE_S;

/**hfsm event enum*/
typedef enum hiHFSM_EVENT_E
{
    HI_HFSM_EVENT_UNHANDLE_MSG = 0,     /**<unhandler message*/
    HI_HFSM_EVENT_TRANSTION_ERROR,      /**<transtion error*/

    HI_HFSM_EVENT_BUTT
} HI_HFSM_EVENT_E;

/** HFSM unhandled message event information*/
typedef struct hiHFSM_UNHANDLED_MSG_INFO_S
{
    HI_MESSAGE_S *pstunHandlerMsg;
} HI_HFSM_UNHANDLED_MSG_INFO_S;

/** HFSM event information*/
typedef struct hiHFSM_TRANSTION_INFO_S
{
    HI_STATE_S *pstOrgState;
    HI_STATE_S *pstCurrentState;
    HI_STATE_S *pstDestState;
    HI_S32 s32ErrorNo;
} HI_HFSM_TRANSTION_INFO_S;

/** HFSM event information*/
typedef struct hiHFSM_EVENT_INFO_S
{
    HI_HFSM_EVENT_E enEventCode;
    union
    {
        HI_HFSM_UNHANDLED_MSG_INFO_S stunHandlerMsgInfo;  /**<unhandled message event info*/
        HI_HFSM_TRANSTION_INFO_S  stTranstionInfo;        /**<file event info*/
    } unEventInfo;
} HI_HFSM_EVENT_INFO_S;

typedef HI_S32 (*HI_HFSM_EVENT_CALLBACK_FN)(HI_MW_PTR pHfsmHandle, const HI_HFSM_EVENT_INFO_S* pstEventInfo);

typedef struct hiHFSM_ATTR_S{
    HI_HFSM_EVENT_CALLBACK_FN fnHfsmEventCallback;
    HI_U32 u32StateMaxAmount;
    HI_U32 u32MessageQueueSize;
}HI_HFSM_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
  *    @brief create HFSM
  *
  *    create HFSM
  *    @param[in]  pstFsmAttr: attribute of HFSM
  *    @param[out]  ppHfsm: point of FHSM handle
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_HFSM_Create(IN HI_HFSM_ATTR_S* pstFsmAttr, OUT HI_MW_PTR *ppHfsm);

/**
  *    @brief destroy HFSM
  *
  *    destroy HFSM
  *    @param[in]  pHfsm handle
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_HFSM_Destroy(IN HI_MW_PTR pHfsm);

/**
  *    @brief add state
  *
  *    add state for HFSM
  *    @param[in]  pHfsm: point of FHSM handle
  *    @param[in]  pSate: state to add
  *    @param[in]  pParent: parent of state to add
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_HFSM_AddState(HI_MW_PTR pHfsm, HI_STATE_S *pSate, HI_STATE_S* pParent);

/**
  *    @brief set initial state
  *
  *    set initial state for HFSM
  *    @param[in]  pHfsm: point of FHSM handle
  *    @param[in]  pazStateName: the name of initial state
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_HFSM_SetInitialState(HI_MW_PTR pHfsm, HI_CHAR* pazStateName);

/**
  *    @brief get current state
  *
  *    get current state
  *    @param[in]  pHfsm: point of FHSM handle
  *    @param[out]  pSate: current state point
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_HFSM_GetCurrentState(HI_MW_PTR pHfsm, HI_STATE_S *pSate);

/**
  *    @brief start
  *
  *    start
  *    @param[in]  pHfsm: point of FHSM handle
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_HFSM_Start(HI_MW_PTR pHfsm);

/**
  *    @brief stop
  *
  *    stop
  *    @param[in]  pHfsm: point of FHSM handle
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_HFSM_Stop(HI_MW_PTR pHfsm);

/**
  *    @brief send an asynchronous message
  *
  *    send an asynchronous message to HFSM
  *    @param[in]  pHfsm: point of FHSM handle
  *    @param[in]  pMsg: message point to send
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_HFSM_SendAsyncMessage(HI_MW_PTR pHfsm, HI_MESSAGE_S *pMsg);

/** @}*/  /** <!-- ==== HFSM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
