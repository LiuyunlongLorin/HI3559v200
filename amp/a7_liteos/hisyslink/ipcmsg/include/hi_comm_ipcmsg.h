/**
* Copyright (C), 2016, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_ipcmsg_message.h
* @brief     Include dual-core communication message struct and it's create destroy function
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

/*************************** Macro Definition ****************************/

#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <stdlib.h>
#include <string.h>
#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus*/

/** \addtogroup     IPCMSG*/
/** @{ */  /** <!-- [IPCMSG] */

#define HI_IPCMSG_MAX_CONTENT_LEN (1024)
#define HI_IPCMSG_PRIVDATA_NUM (8)
#define HI_IPCMSG_INVALID_MSGID (0xFFFFFFFFFFFFFFFF)

/** @} */  /*! <!-- Macro Definition End */

typedef struct hiIPCMSG_CONNECT_S
{
    HI_U32 u32RemoteId;
    HI_U32 u32Port;
    HI_U32 u32Priority;
} HI_IPCMSG_CONNECT_S;

/**Message structure*/
typedef struct hiIPCMSG_MESSAGE_S
{
    HI_BOOL bIsResp;    /**<Identify the response messgae*/
    HI_U64 u64Id;       /**<Message ID*/
    HI_U32 u32Module;   /**<Module ID, user-defined*/
    HI_U32 u32CMD;      /**<CMD ID, user-defined*/
    HI_S32 s32RetVal;   /**<Retrun Value in response message*/
    HI_S32 as32PrivData[HI_IPCMSG_PRIVDATA_NUM]; /**<Private data, can be modify directly after ::HI_IPCMSG_CreateMessage or ::HI_IPCMSG_CreateRespMessage*/
    HI_VOID* pBody;     /**<Message body*/
    HI_U32 u32BodyLen;  /**<Length of pBody*/
} HI_IPCMSG_MESSAGE_S;


/** Error number base */
#define HI_IPCMSG_ERRNO_BASE 0x1900
/** Parameter is invalid */
#define HI_IPCMSG_EINVAL (HI_IPCMSG_ERRNO_BASE+1)
/** The function run timeout */
#define HI_IPCMSG_ETIMEOUT (HI_IPCMSG_ERRNO_BASE+2)
/** IPC driver open fail */
#define HI_IPCMSG_ENOOP (HI_IPCMSG_ERRNO_BASE+3)
/** Internal error */
#define HI_IPCMSG_EINTER (HI_IPCMSG_ERRNO_BASE+4)
/** Null pointer*/
#define HI_IPCMSG_ENULL_PTR (HI_IPCMSG_ERRNO_BASE+5)


#define HI_IPCMSG_MAX_SERVICENAME_LEN (16)


/**
 * @brief Callback of receiving message.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Received message.
 */
typedef void (*HI_IPCMSG_HANDLE_FN_PTR)(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* pstMsg);

/**
 * @brief Callback of receiving response message. used by HI_IPCMSG_SendAsync
 * @param[in] pstMsg Response message.
 */
typedef void (*HI_IPCMSG_RESPHANDLE_FN_PTR)(HI_IPCMSG_MESSAGE_S* pstMsg);


/** @}*/  /** <!-- ==== IPCMSG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus*/

#endif
