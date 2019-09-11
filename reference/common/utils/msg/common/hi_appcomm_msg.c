/**
 * @file    hi_appcomm_msg.c
 * @brief   appcomm msg function.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/26
 * @version   1.0

 */

#include "hi_ipcmsg.h"
#include "hi_appcomm_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "MSG"

extern HI_List_Head_S g_stMSGProcList;
extern HI_S32 g_s32MsgFd;

static HI_VOID MSG_RespProc(HI_IPCMSG_MESSAGE_S* pstMsg)
{
    //MLOGD("receive resp: %u, len: %u\n", pstMsg->u32CMD, pstMsg->u32BodyLen);
    return;
}

static HI_S32 MSG_Response(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* pstReqMsg, HI_S32 s32RetVal, HI_VOID* pvResponse, HI_U32 u32RespLen)
{
    HI_S32 s32Ret = 0;
    HI_IPCMSG_MESSAGE_S* pstRespMsg = NULL;
    pstRespMsg = HI_IPCMSG_CreateRespMessage(pstReqMsg, s32RetVal, (HI_VOID*)pvResponse, u32RespLen);

    if (pstRespMsg)
    {
        s32Ret = HI_IPCMSG_SendAsync(s32Id, pstRespMsg, NULL);
        HI_IPCMSG_DestroyMessage(pstRespMsg);

        if (HI_SUCCESS != s32Ret)
        {
            HI_LOG_PrintFuncErr(HI_IPCMSG_SendAsync, s32Ret);
            return HI_EINTER;
        }

        return HI_SUCCESS;
    }

    return HI_EINTER;
}

HI_VOID MSG_Handler(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pstMsg, );
    MLOGD("receive msg: 0x%u, len: %u\n", pstMsg->u32CMD, pstMsg->u32BodyLen);

    if (MSG_ID_DISCONNECT == pstMsg->u32CMD)
    {
        MSG_Response(s32Id, pstMsg, 0, NULL, 0);
        HI_usleep(1000);
        s32Ret = HI_IPCMSG_Disconnect(s32Id);

        if (HI_SUCCESS != s32Ret)
        {
            HI_LOG_PrintFuncErr(HI_IPCMSG_Disconnect, s32Ret);
        }

        return;
    }

    MSG_PROC_S* pstMsgProc = NULL;
    HI_List_Head_S* pstNode = NULL;
    HI_VOID* pvResponse = NULL;
    HI_U32 u32RespLen = 0;
    HI_List_For_Each(pstNode, &g_stMSGProcList)
    {
        pstMsgProc = HI_LIST_ENTRY(pstNode, MSG_PROC_S, stList);

        if (pstMsgProc)
        {
            if (pstMsg->u32CMD == pstMsgProc->s32MsgID)
            {
                HI_MSG_PRIV_DATA_S stPrivData;
                memcpy(stPrivData.as32PrivData, pstMsg->as32PrivData, sizeof(HI_S32)*HI_IPCMSG_PRIVDATA_NUM);
                s32Ret = pstMsgProc->pfnMsgProc(pstMsgProc->s32MsgID, pstMsg->pBody, pstMsg->u32BodyLen, &stPrivData, &pvResponse, &u32RespLen, pstMsgProc->pvUserData);
                MSG_Response(s32Id, pstMsg, s32Ret, pvResponse, u32RespLen);
                HI_APPCOMM_SAFE_FREE(pvResponse);
                break;
            }
        }
    }
}

/**
 * @brief    register requst msg process function.
 * @param[in] s32MsgID:unique msg id.
 * @param[in] pfnReqProc:requst process function.
 * @param[in] pvUserData:user data.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 */
HI_S32 HI_MSG_RegisterMsgProc(HI_S32 s32MsgID, HI_MSG_PROC_CALLBACK_FN_PTR pfnMsgProc, HI_VOID* pvUserData)
{
    MSG_PROC_S* pstMsgProc = NULL;
    HI_List_Head_S* pstNode = NULL;
    HI_APPCOMM_CHECK_POINTER(pfnMsgProc, HI_FAILURE);
    HI_List_For_Each(pstNode, &g_stMSGProcList)
    {
        pstMsgProc = HI_LIST_ENTRY(pstNode, MSG_PROC_S, stList);

        if (pstMsgProc)
        {
            if (s32MsgID == pstMsgProc->s32MsgID)
            {
                MLOGW("s32MsgID:%08x has been registered already,replace process function.\n", s32MsgID);
                break;
            }

            pstMsgProc = NULL;
        }
    }

    if (NULL == pstMsgProc)
    {
        pstMsgProc = (MSG_PROC_S*)malloc(sizeof(MSG_PROC_S));
        HI_APPCOMM_CHECK_POINTER(pstMsgProc, HI_ENORES);
        HI_List_Add(&pstMsgProc->stList, &g_stMSGProcList);
    }

    pstMsgProc->s32MsgID = s32MsgID;
    pstMsgProc->pfnMsgProc = pfnMsgProc;
    pstMsgProc->pvUserData = pvUserData;
    return HI_SUCCESS;
}

/**
 * @brief    async send msg.
 * @param[in] s32MsgID:unique msg id.
 * @param[in] pvMsg:msg data.
 * @param[in] u32MsgLen:msg data lenght.
 * @return 0 success,non-zero error code.
 * @exception    the response will be in pfnRespProc.
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 */
HI_S32 HI_MSG_SendASync(HI_S32 s32MsgID, const HI_VOID* pvMsg, HI_U32 u32MsgLen, HI_MSG_PRIV_DATA_S* pstPrivData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S* pstReq = NULL;
    HI_APPCOMM_CHECK_EXPR(-1 != g_s32MsgFd, HI_ENOINIT);

    pstReq = HI_IPCMSG_CreateMessage(0, (HI_U32)s32MsgID, (HI_VOID*)pvMsg, u32MsgLen);

    if (pstReq)
    {
        if(pstPrivData != NULL)
        {
            memcpy(pstReq->as32PrivData, pstPrivData->as32PrivData, sizeof(HI_S32)*HI_IPCMSG_PRIVDATA_NUM);
        }
        s32Ret = HI_IPCMSG_SendAsync(g_s32MsgFd, pstReq, MSG_RespProc);
        HI_IPCMSG_DestroyMessage(pstReq);

        if (HI_SUCCESS != s32Ret)
        {
            HI_LOG_PrintH32(s32MsgID);
            HI_LOG_PrintFuncErr(HI_IPCMSG_SendAsync, s32Ret);
            return HI_EINTER;
        }

        return HI_SUCCESS;
    }

    return HI_EINTER;
}

/**
 * @brief    sync send msg.
 * @param[in] s32MsgID:unique msg id.
 * @param[in] pvRequest:request data.
 * @param[in] u32ReqLen:request data lenght.
 * @param[out] pvResponse:response data.
 * @param[in] u32RespLen:response data lenght.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 */
HI_S32 HI_MSG_SendSync(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData, HI_VOID* pvResponse, HI_U32 u32RespLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S* pstReq = NULL;
    HI_IPCMSG_MESSAGE_S* pstResp = NULL;

    HI_APPCOMM_CHECK_EXPR(-1 != g_s32MsgFd, HI_ENOINIT);

    pstReq = HI_IPCMSG_CreateMessage(0, (HI_U32)s32MsgID, (HI_VOID*)pvRequest, u32ReqLen);

    if (pstReq)
    {
        if(pstPrivData != NULL)
        {
            memcpy(pstReq->as32PrivData, pstPrivData->as32PrivData, sizeof(HI_S32)*HI_IPCMSG_PRIVDATA_NUM);
        }
        s32Ret = HI_IPCMSG_SendSync(g_s32MsgFd, pstReq, &pstResp, MSG_SEND_SYNC_TIMEOUT);
        HI_IPCMSG_DestroyMessage(pstReq);

        if (HI_SUCCESS != s32Ret || NULL == pstResp)
        {
            HI_LOG_PrintH32(s32MsgID);
            HI_LOG_PrintFuncErr(HI_IPCMSG_SendSync, s32Ret);
            return HI_EINTER;
        }
        else
        {
            s32Ret = pstResp->s32RetVal;

            if (NULL != pvResponse && NULL != pstResp->pBody && 0 < u32RespLen)
            {
                memcpy(pvResponse, pstResp->pBody, u32RespLen);
            }

            HI_IPCMSG_DestroyMessage(pstResp);
            return s32Ret;
        }
    }

    return HI_EINTER;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
