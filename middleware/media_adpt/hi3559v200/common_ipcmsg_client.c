#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "hi_ipcmsg.h"
#include "common_ipcmsg.h"
#include <sys/prctl.h>

static HI_S32 s_s32CommonIPCMsgID = 0;
static pthread_t s_CommonRecvThrdId = 0;
#define  AVPLAY_MSG_LONG_TIMEOUT (10000)

static HI_VOID COMMON_IPCMSG_CLIENT_HandleRecvMsg(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* pMsg)
{
    if (s32Id != s_s32CommonIPCMsgID)
    {
        printf( "ipcmsg receive msg from error id: %d\n", s32Id);
        return;
    }

    if (pMsg->u32Module != IPCMSG_COMMON_MODID)
    {
        printf( "ipcmsg receive msg from error module id: %d\n", pMsg->u32Module);
        return;
    }

    printf( "common receive msg cmd:%d \n", pMsg->u32CMD);
}

static HI_VOID* COMMON_IPCMSG_CLIENT_RecvThread(void* arg)
{
    prctl(PR_SET_NAME, "common_ipcmsg_cli_rcvThread", 0, 0, 0);

    HI_S32* s32Id = (HI_S32*)arg;

    printf("common receive from %d\n", *s32Id);
    HI_IPCMSG_Run(*s32Id);

    return NULL;
}

HI_S32 COMMON_IPCMSG_CLIENT_Init()
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_IPCMSG_CONNECT_S stIpcMsgConnect;
	stIpcMsgConnect.u32RemoteId = 1;
	stIpcMsgConnect.u32Port = IPMSG_COMMON_PORT;
	stIpcMsgConnect.u32Priority = 0;

    if (HI_IPCMSG_AddService(IPCMSG_COMMON,&stIpcMsgConnect) != HI_SUCCESS)
    {
        printf("HI_IPCMSG_AddService IPCMSG_COMMON fail\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != HI_IPCMSG_Connect(&s_s32CommonIPCMsgID, IPCMSG_COMMON, COMMON_IPCMSG_CLIENT_HandleRecvMsg))
    {
        printf("HI_IPCMSG_Connect IPCMSG_COMMON fail\n");
        HI_IPCMSG_DelService(IPCMSG_COMMON);
        return HI_FAILURE;
    }


    s32Ret = pthread_create(&s_CommonRecvThrdId, NULL, COMMON_IPCMSG_CLIENT_RecvThread, &s_s32CommonIPCMsgID);
    if(s32Ret != HI_SUCCESS)
    {
        s_s32CommonIPCMsgID = 0;
        HI_IPCMSG_Disconnect(s_s32CommonIPCMsgID);
        HI_IPCMSG_DelService(IPCMSG_COMMON);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 COMMON_IPCMSG_CLIENT_Deinit()
{
    if (s_s32CommonIPCMsgID)
    {
        HI_IPCMSG_Disconnect(s_s32CommonIPCMsgID);
        s_s32CommonIPCMsgID = 0;
    }

    if (s_CommonRecvThrdId)
    {
        if(pthread_join(s_CommonRecvThrdId, NULL) != HI_SUCCESS)
        {
            printf("pthread_join s_CommonRecvThrdId failed\n");
        }
        s_CommonRecvThrdId = 0;
    }

    if (HI_IPCMSG_DelService(IPCMSG_COMMON) != HI_SUCCESS)
    {
        printf("HI_IPCMSG_DelService failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 COMMON_IPCMSG_CLIENT_SendSync(HI_U32 enMsgId,
    HI_VOID* pBody, HI_U32 u32BodyLen, HI_VOID* pRespBody, HI_U32 u32RespLen,  HI_U32 u32TimeoutMs)
{
    HI_S32  s32Ret = HI_SUCCESS;

    HI_IPCMSG_MESSAGE_S* pReq = HI_IPCMSG_CreateMessage(IPCMSG_COMMON_MODID, enMsgId, pBody, u32BodyLen);
    HI_IPCMSG_MESSAGE_S* pResp = NULL;

    s32Ret = HI_IPCMSG_SendSync(s_s32CommonIPCMsgID, pReq, &pResp, u32TimeoutMs);

    if (s32Ret == HI_IPCMSG_ETIMEOUT)
    {
        printf( "HI_IPCMSG_SendSync msgId: %d timeout,ret:%d\n", enMsgId, s32Ret);
        HI_IPCMSG_DestroyMessage(pReq);
        return HI_FAILURE;
    }
    else if (s32Ret != HI_SUCCESS)
    {
        printf( "HI_IPCMSG_SendSync msgId: %d error,ret:%d\n", enMsgId, s32Ret);
        HI_IPCMSG_DestroyMessage(pReq);
        return HI_FAILURE;
    }

    s32Ret = pResp->s32RetVal;
    if (s32Ret == HI_SUCCESS && u32RespLen > 0)
    {
        if((u32RespLen == pResp->u32BodyLen)
            && pRespBody && pResp->pBody)
        {
            memcpy(pRespBody, pResp->pBody, u32RespLen);
        }
        else
        {
            printf( "IPCMsg resp len %d error or Null ptr\n", pResp->u32BodyLen);
            s32Ret = HI_FAILURE;
        }
    }

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return s32Ret;
}

HI_S32 COMMON_Client_VO_Open(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
    HI_HANDLE hDispHdl, HI_HANDLE hWndHdl, HI_U32 u32Width, HI_U32 u32Height)
{
    HI_S32 s32Ret = HI_SUCCESS;
    COMMON_VO_OPEN_REQ_MSG_S stOpenReq;
    stOpenReq.hDispHdl = hDispHdl;
    stOpenReq.hWndHdl = hWndHdl;
    stOpenReq.hVpssGrpHdl = hVpssGrpHdl;
    stOpenReq.hVpssChnHdl = hVpssChnHdl;
    stOpenReq.u32Width = u32Width;
    stOpenReq.u32Height = u32Height;

    s32Ret = COMMON_IPCMSG_CLIENT_SendSync(COMMON_MSG_CLIENT_VO_OPEN_REQ,
        &stOpenReq, sizeof(COMMON_VO_OPEN_REQ_MSG_S), NULL, 0, AVPLAY_MSG_LONG_TIMEOUT);

    if(s32Ret != HI_SUCCESS)
    {
        printf("common vo open send ipcmsg exec error ret:%d \n",  s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}


HI_S32 COMMON_Client_VO_Close(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
    HI_HANDLE hDispHdl, HI_HANDLE hWndHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    COMMON_VO_CLOSE_REQ_MSG_S stCloseReq;
    stCloseReq.hDispHdl = hDispHdl;
    stCloseReq.hWndHdl = hWndHdl;
    stCloseReq.hVpssGrpHdl = hVpssGrpHdl;
    stCloseReq.hVpssChnHdl = hVpssChnHdl;

    s32Ret = COMMON_IPCMSG_CLIENT_SendSync(COMMON_MSG_CLIENT_VO_CLOSE_REQ,
        &stCloseReq, sizeof(COMMON_VO_CLOSE_REQ_MSG_S), NULL, 0, AVPLAY_MSG_LONG_TIMEOUT);

    if(s32Ret != HI_SUCCESS)
    {
        printf("common vo close send ipcmsg exec error ret:%d \n",  s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 COMMON_Client_AO_Open(HI_HANDLE hAoHdl, HI_S32 s32SampleRate, HI_U32 u32ChnCnt)
{
    HI_S32 s32Ret = HI_SUCCESS;
    printf("PLAYER_AO_Open s32SampleRate: %d u32ChnCnt: %d\n", s32SampleRate, u32ChnCnt);

    COMMON_AO_OPEN_REQ_MSG_S stOpenReq;
    stOpenReq.hAoHdl = hAoHdl;
    stOpenReq.u32ChnCnt = u32ChnCnt;
    stOpenReq.s32SampleRate = s32SampleRate;

    s32Ret = COMMON_IPCMSG_CLIENT_SendSync(COMMON_MSG_CLIENT_AO_OPEN_REQ,
        &stOpenReq, sizeof(COMMON_AO_OPEN_REQ_MSG_S), NULL, 0, AVPLAY_MSG_LONG_TIMEOUT);

    if(s32Ret != HI_SUCCESS)
    {
        printf("common Ao open send ipcmsg exec error ret:%d \n",  s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 COMMON_Client_AO_Close(HI_HANDLE hAoHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    COMMON_AO_CLOSE_REQ_MSG_S stCloseReq;
    stCloseReq.hAoHdl = hAoHdl;
    s32Ret = COMMON_IPCMSG_CLIENT_SendSync(COMMON_MSG_CLIENT_AO_CLOSE_REQ,
        &stCloseReq, sizeof(COMMON_AO_CLOSE_REQ_MSG_S), NULL, 0, AVPLAY_MSG_LONG_TIMEOUT);

    if(s32Ret != HI_SUCCESS)
    {
        printf("common Ao close send ipcmsg exec error ret:%d \n",  s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 COMMON_Client_Vb_Alloc(HI_U32* pPoolId, HI_U32 u32FrameSize, HI_U32 u32FrameCnt)
{
    HI_S32 s32Ret = HI_SUCCESS;

    COMMON_VB_ALLOC_REQ_MSG_S stAllocReq;
    stAllocReq.u32FrameBufCnt = u32FrameCnt;
    stAllocReq.u32FrameBufSize = u32FrameSize;
    COMMON_VB_ALLOC_REPLY_MSG_S stAllocReply;


    s32Ret = COMMON_IPCMSG_CLIENT_SendSync(COMMON_MSG_CLIENT_VB_ALLOC_REQ,
        &stAllocReq, sizeof(COMMON_VB_ALLOC_REQ_MSG_S), &stAllocReply, sizeof(COMMON_VB_ALLOC_REPLY_MSG_S), AVPLAY_MSG_LONG_TIMEOUT);

    if(s32Ret != HI_SUCCESS)
    {
        printf("common vb alloc send ipcmsg exec error ret:%d \n",  s32Ret);
        return HI_SUCCESS;
    }
    *pPoolId = stAllocReply.u32VbPoolId;
    return HI_SUCCESS;
}

HI_S32 COMMON_Client_Vb_Free(HI_U32 poolid)
{
    HI_S32 s32Ret = HI_SUCCESS;

    COMMON_VB_FREE_REQ_MSG_S stFreeReq;
    stFreeReq.u32VbPoolId = poolid;
    s32Ret = COMMON_IPCMSG_CLIENT_SendSync(COMMON_MSG_CLIENT_VB_FREE_REQ,
        &stFreeReq, sizeof(COMMON_VB_FREE_REQ_MSG_S), NULL, 0, AVPLAY_MSG_LONG_TIMEOUT);

    if(s32Ret != HI_SUCCESS)
    {
        printf("common vb free send ipcmsg exec error ret:%d \n",  s32Ret);
        return HI_SUCCESS;
    }
    return HI_SUCCESS;
}
