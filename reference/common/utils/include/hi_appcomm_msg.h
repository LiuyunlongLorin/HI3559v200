/**
 * @file    hi_appcomm_msg.h
 * @brief   appcomm msg define.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 * @version   1.0

 */
#ifndef __HI_APPCOMM_MSG_H__
#define __HI_APPCOMM_MSG_H__

#include "hi_list.h"
#include "hi_comm_ipcmsg.h"
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     MSG */
/** @{ */  /** <!-- [MSG] */

#define HI_APPCOMM_MSG_SRVPORT (201)
#define HI_APPCOMM_MSG_SRVNAME "AppcommMSG"

/** App MSG BaseId : [28bit~31bit] unique */
#define HI_APPCOMM_MSG_BASEID (0x20000000L)

/** App MSG ID Rule [ --base[4bit]--|--module[8bit]--|--msgid[20bit]--]
    * module : module enum value [HI_APP_MOD_E]
    * msgid : msg code in specified module, unique in module
    */
#define HI_APPCOMM_MSG_ID(module, msgid) \
    ((HI_S32)( (HI_APPCOMM_MSG_BASEID) | ((module) << 20 ) | (msgid) ))

/** this msg id is reserved*/
#define MSG_ID_DISCONNECT (0)
#define MSG_SEND_SYNC_TIMEOUT (10000)

typedef struct hiMSG_PRIV_DATA_S
{
    HI_S32 as32PrivData[HI_IPCMSG_PRIVDATA_NUM];
} HI_MSG_PRIV_DATA_S;

typedef HI_S32 (*HI_MSG_PROC_CALLBACK_FN_PTR)(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,
                                                                    HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData);

typedef struct tagMSG_PROC_S
{
    HI_List_Head_S stList;
    HI_S32 s32MsgID;
    HI_MSG_PROC_CALLBACK_FN_PTR pfnMsgProc;
    HI_VOID* pvUserData;
} MSG_PROC_S;

/**
 * @brief    register requst msg process function.
 * @param[in] s32MsgID:unique msg id.
 * @param[in] pfnMsgProc:msg process function.
 * @param[in] pvUserData:user data.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 */
HI_S32 HI_MSG_RegisterMsgProc(HI_S32 s32MsgID, HI_MSG_PROC_CALLBACK_FN_PTR pfnMsgProc, HI_VOID* pvUserData);

/**
 * @brief    async send msg.
 * @param[in] s32MsgID:unique msg id.
 * @param[in] pvMsg:msg data.
 * @param[in] u32MsgLen:msg data lenght.
 * @param[in] pstPrivData:private data.
 * @return 0 success,non-zero error code.
 * @exception    the response will be in pfnRespProc.
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 */
HI_S32 HI_MSG_SendASync(HI_S32 s32MsgID, const HI_VOID* pvMsg, HI_U32 u32MsgLen, HI_MSG_PRIV_DATA_S* pstPrivData);

/**
 * @brief    sync send msg.
 * @param[in] s32MsgID:unique msg id.
 * @param[in] pvRequest:request data.
 * @param[in] u32ReqLen:request data lenght.
 * @param[in] pstPrivData:private data.
 * @param[out] pvResponse:response data.
 * @param[in] u32RespLen:response data lenght.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 */
HI_S32 HI_MSG_SendSync(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID* pvResponse, HI_U32 u32RespLen);

/** @}*/  /** <!-- ==== MSG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef __HI_APPCOMM_MSG_H__ */
