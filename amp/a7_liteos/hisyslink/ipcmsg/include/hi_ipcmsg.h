/**
* Copyright (C), 2016, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_ipcmsg.h
* @brief     Inter-Processor Communication
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __IPCMSG_H
#define __IPCMSG_H
#include "hi_comm_ipcmsg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/** \addtogroup     IPCMSG*/
/** @{ */  /** <!-- [IPCMSG] */



/**
 * @brief Add services to global service table before use of IPCMSG.
 * This function should be called in two side arm with the same parameters.
 * @param[in] pszServiceName Service running on two side(a7/a17).
 * @param[in] u32Port The two service has unique port.
 * @param[in] u32Priority Priority of the communication.
 * @return HI_SUCCESS Register success.
 * @return HI_FAILURE Register fail.
 * @return HI_IPCMSG_EINVAL Parameter is invalid
 * @return HI_IPCMSG_EINTER Open device fail or other internal error
 */
HI_S32 HI_IPCMSG_AddService(const HI_CHAR* pszServiceName, const HI_IPCMSG_CONNECT_S* pstConnectAttr);

/**
 * @brief Delete service from global service table when don't need IPCMSG.
 * @param[in] pszServiceName Service running on two side(a7/a17).
 * @return HI_IPCMSG_EINVAL Parameter is invalid
 * @return HI_SUCCESS Unregister success.
 * @return HI_FAILURE Unregister fail.
 */
HI_S32 HI_IPCMSG_DelService(const HI_CHAR* pszServiceName);

/**
 * @brief Establish the connection between master and slave.Attention: Message can send successfuly only the two side call this function. So ::HI_IPCMSG_IsConnected should be called to ensure the connection is established before send or receive
 * @param[out] ps32Id Handle of IPCMSG. All operation in IPCMSG need this handle
 * @param[in] pszServiceName Service name running on the other side.
 * @param[in] pfnMessageHandle Callback function to receive message.
 * @return HI_IPCMSG_EINVAL Parameter is invalid
 * @return HI_IPCMSG_EINTER Open device fail or other internal error
 * @return HI_SUCCESS Connect success.
 * @return HI_FAILURE Conectt fail.
 */
HI_S32 HI_IPCMSG_TryConnect(HI_S32* ps32Id, const HI_CHAR* pszServiceName, HI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle);

/**
 * @brief Establish the connection between master and slave. This function will block until two side connections established.
 * @param[out] ps32Id Handle of IPCMSG. All operation in IPCMSG need this handle
 * @param[in] pszServiceName Service name running on the other side.
 * @param[in] pfnMessageHandle Callback function to receive message.
 * @return HI_IPCMSG_EINVAL Parameter is invalid
 * @return HI_IPCMSG_EINTER Open device fail or other internal error
 * @return HI_SUCCESS Connect success.
 * @return HI_FAILURE Conectt fail.
 */
HI_S32 HI_IPCMSG_Connect(HI_S32* ps32Id, const HI_CHAR* pszServiceName, HI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle);

/**
 * @brief Disconnect when don't want to send or receive message.
 * @param[in] s32Id Handle of IPCMSG.
 * @return HI_IPCMSG_EINVAL Parameter is invalid
 * @return HI_SUCCESS Disconnect success.
 * @return HI_FAILURE Disconnect fail.
 */
HI_S32 HI_IPCMSG_Disconnect(HI_S32 s32Id);

/**
 * @brief Whether the communication can work.
 * @param[in] s32Id Handle of IPCMSG
 * @return HI_TRUE connection is done. can send message.
 * @return HI_FALSE connection is not finish yet. send message will return failure.
 */
HI_BOOL HI_IPCMSG_IsConnected(HI_S32 s32Id);

/**
 * @brief Send message only, don't need response.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Message to send.
 * @return HI_SUCCESS Send success.
 * @return HI_FAILURE Send fail.
 */
HI_S32 HI_IPCMSG_SendOnly(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S *pstRequest);

/**
 * @brief Send message asynchronously. the function will return immediately.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Message to send.
 * @param[in] pfnRespHandle Callback function to receive response.
 * @return HI_SUCCESS Send success.
 * @return HI_FAILURE Send fail.
 */
HI_S32 HI_IPCMSG_SendAsync(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* pstMsg, HI_IPCMSG_RESPHANDLE_FN_PTR pfnRespHandle);

/**
 * @brief Send message synchronously. the function will block until response message received.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Message to send.
 * @param[out] ppstMsg Received response message.
 * @param[in] s32TimeoutMs When response not received in s32TimeoutMs, function will return ::HI_IPCMSG_ETIMEOUT
 * @return HI_SUCCESS Send success.
 * @return HI_FAILURE Send fail.
 * #return HI_IPCMSG_ETIMEOUT Timeout to receive response
 */
HI_S32 HI_IPCMSG_SendSync(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* pstMsg, HI_IPCMSG_MESSAGE_S** ppstMsg, HI_S32 s32TimeoutMs);

/**
 * @brief In this function, Message will be received and dispatched to message callback function.
 * User should create thread to run this function.
 * @param[in] s32Id Handle of IPCMSG.
 */
HI_VOID HI_IPCMSG_Run(HI_S32 s32Id);

/**
 * @brief Create the message, used by HI_IPCMSG_SendXXX and receive callback function.
 * @param[in] u32Module Module ID defined by user. user can use it to dispatch to different modules.
 * @param[in] u32CMD CMD ID, defined by user. user can use it to identify which command.
 * @param[in] pBody Message body, mustn't contain pointer because pointer will be useless in other side.
 * @param[in] u32BodyLen Length of pBody.
 * @return ::HI_IPCMSG_MESSAGE_S* Created message.
 */
HI_IPCMSG_MESSAGE_S* HI_IPCMSG_CreateMessage(HI_U32 u32Module, HI_U32 u32CMD, HI_VOID* pBody, HI_U32 u32BodyLen);

/**
 * @brief Create the response message.
 * @param[in] pstRequest Request message received by user.
 * @param[in] s32RetVal Integer return value.
 * @param[in] pBody Message body.
 * @param[in] u32BodyLen Length of pBody.
 * @return ::HI_IPCMSG_MESSAGE_S* Created message.
 */
HI_IPCMSG_MESSAGE_S* HI_IPCMSG_CreateRespMessage(HI_IPCMSG_MESSAGE_S* pstRequest, HI_S32 s32RetVal, HI_VOID* pBody, HI_U32 u32BodyLen);

/**
 * @brief Destroy the message. Message must be destroyed when send and receive finish
 * @param[in] pstMsg Message to destroy.
 */
HI_VOID HI_IPCMSG_DestroyMessage(HI_IPCMSG_MESSAGE_S* pstMsg);


/** @}*/  /** <!-- ==== IPCMSG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
