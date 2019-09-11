#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "hi_ipcmsg.h"
#include "common_ipcmsg.h"
#include "hi_comm_vb.h"
#include <sys/prctl.h>
#include "mpi_vb.h"

static pthread_t s_CommonRecvThrdId = 0;
static HI_S32 s32CommonSvrIPCMsgID = 0;
static HI_BOOL bCommonSvrInited = HI_FALSE;

#define COMMON_IPCMSG_THREAD_PRIORITY (7)
#define COMMON_IPCMSG_THREAD_STACKSIZE (0x10000)

static HI_S32 COMMON_IPCMSG_SendResp(HI_IPCMSG_MESSAGE_S* pReq,
    HI_S32 s32RetVal, HI_VOID* pBody, HI_U32 u32BodyLen)
{
    HI_IPCMSG_MESSAGE_S* pResp =  HI_IPCMSG_CreateRespMessage(pReq,
        s32RetVal, pBody, u32BodyLen);
    if(!pResp)
    {
        printf ("HI_IPCMSG_CreateRespMessage failed\n");
        return HI_FAILURE;
    }

    //printf "HI_IPCMSG_SendAsync s32SvrIPCMsgID: %d \n", s32SvrIPCMsgID);
    if(HI_IPCMSG_SendAsync(s32CommonSvrIPCMsgID, pResp, NULL) != HI_SUCCESS)
    {
        printf ("HI_IPCMSG_SendAsync failed\n");
        HI_IPCMSG_DestroyMessage(pResp);
        return HI_FAILURE;
    }

    HI_IPCMSG_DestroyMessage(pResp);

    return HI_SUCCESS;
}

extern HI_S32 PLAYER_AO_Open(HI_HANDLE hAoHdl, HI_S32 s32SampleRate, HI_U32 u32ChnCnt);
extern HI_S32 PLAYER_AO_Close(HI_HANDLE hAoHdl);
extern HI_S32 PLAYER_VO_Open(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
    HI_HANDLE hDispHdl, HI_HANDLE hWndHdl, HI_U32 u32Width, HI_U32 u32Height);


extern HI_S32 PLAYER_VO_Close(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
    HI_HANDLE hDispHdl, HI_HANDLE hWndHdl);

static HI_VOID COMMON_IPCMSG_SVR_HandleVoOpen(HI_IPCMSG_MESSAGE_S* pReq)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(pReq->u32BodyLen != sizeof(COMMON_VO_OPEN_REQ_MSG_S))
    {
        COMMON_IPCMSG_SendResp(pReq, HI_FAILURE, NULL, 0);
    }
    else
    {
        COMMON_VO_OPEN_REQ_MSG_S* pstOpen = (COMMON_VO_OPEN_REQ_MSG_S*)pReq->pBody;

        s32Ret = PLAYER_VO_Open(pstOpen->hVpssGrpHdl, pstOpen->hVpssChnHdl,
            pstOpen->hDispHdl, pstOpen->hWndHdl,pstOpen->u32Width,
            pstOpen->u32Height);

        COMMON_IPCMSG_SendResp(pReq, s32Ret, NULL, 0);
    }
}

static HI_VOID COMMON_IPCMSG_SVR_HandleVoClose(HI_IPCMSG_MESSAGE_S* pReq)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(pReq->u32BodyLen != sizeof(COMMON_VO_CLOSE_REQ_MSG_S))
    {
        COMMON_IPCMSG_SendResp(pReq, HI_FAILURE, NULL, 0);
    }
    else
    {
        COMMON_VO_CLOSE_REQ_MSG_S* pstClose = (COMMON_VO_CLOSE_REQ_MSG_S*)pReq->pBody;

        s32Ret = PLAYER_VO_Close(pstClose->hVpssGrpHdl, pstClose->hVpssChnHdl,
            pstClose->hDispHdl, pstClose->hWndHdl);

        COMMON_IPCMSG_SendResp(pReq, s32Ret, NULL, 0);
    }

}

static HI_VOID COMMON_IPCMSG_SVR_HandleAoOpen(HI_IPCMSG_MESSAGE_S* pReq)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(pReq->u32BodyLen != sizeof(COMMON_AO_OPEN_REQ_MSG_S))
    {
        COMMON_IPCMSG_SendResp(pReq, HI_FAILURE, NULL, 0);
    }
    else
    {
        COMMON_AO_OPEN_REQ_MSG_S* pstOpen = (COMMON_AO_OPEN_REQ_MSG_S*)pReq->pBody;

        s32Ret = PLAYER_AO_Open(pstOpen->hAoHdl, pstOpen->s32SampleRate,
            pstOpen->u32ChnCnt);

        COMMON_IPCMSG_SendResp(pReq, s32Ret, NULL, 0);
    }
}

static HI_VOID COMMON_IPCMSG_SVR_HandleAoClose(HI_IPCMSG_MESSAGE_S* pReq)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(pReq->u32BodyLen != sizeof(COMMON_AO_CLOSE_REQ_MSG_S))
    {
        COMMON_IPCMSG_SendResp(pReq, HI_FAILURE, NULL, 0);
    }
    else
    {
        COMMON_AO_CLOSE_REQ_MSG_S* pstClose = (COMMON_AO_CLOSE_REQ_MSG_S*)pReq->pBody;

        s32Ret = PLAYER_AO_Close(pstClose->hAoHdl);

        COMMON_IPCMSG_SendResp(pReq, s32Ret, NULL, 0);
    }

}

static HI_VOID COMMON_IPCMSG_SVR_HandleVbAlloc(HI_IPCMSG_MESSAGE_S* pReq)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(pReq->u32BodyLen != sizeof(COMMON_VB_ALLOC_REQ_MSG_S))
    {
        COMMON_IPCMSG_SendResp(pReq, HI_FAILURE, NULL, 0);
    }
    else
    {
        COMMON_VB_ALLOC_REQ_MSG_S* pstVbAlloc = (COMMON_VB_ALLOC_REQ_MSG_S*)pReq->pBody;
        VB_POOL_CONFIG_S stVbPoolCfg;
        memset(&stVbPoolCfg, 0x00, sizeof(VB_POOL_CONFIG_S));
        stVbPoolCfg.u64BlkSize = (HI_U64)pstVbAlloc->u32FrameBufSize;
        stVbPoolCfg.u32BlkCnt = pstVbAlloc->u32FrameBufCnt;
        stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;
        VB_POOL framePool = HI_MPI_VB_CreatePool(&stVbPoolCfg);
        if (VB_INVALID_POOLID == framePool)
        {
            printf("HI_MPI_VB_CreatePool error \n");
            COMMON_IPCMSG_SendResp(pReq, HI_FAILURE, NULL, 0);
            return;
        }

        COMMON_VB_ALLOC_REPLY_MSG_S stRepyMsg;
        stRepyMsg.u32VbPoolId = framePool;

        COMMON_IPCMSG_SendResp(pReq, s32Ret, &stRepyMsg, sizeof(COMMON_VB_ALLOC_REPLY_MSG_S));
    }
}

static HI_VOID COMMON_IPCMSG_SVR_HandleVbFree(HI_IPCMSG_MESSAGE_S* pReq)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(pReq->u32BodyLen != sizeof(COMMON_VB_FREE_REQ_MSG_S))
    {
        COMMON_IPCMSG_SendResp(pReq, HI_FAILURE, NULL, 0);
    }
    else
    {
        COMMON_VB_FREE_REQ_MSG_S* pstVbFree = (COMMON_VB_FREE_REQ_MSG_S*)pReq->pBody;

        if (VB_INVALID_POOLID == pstVbFree->u32VbPoolId)
        {
            printf( "VBPoolFree invalid pool id: %d \n", pstVbFree->u32VbPoolId);
            COMMON_IPCMSG_SendResp(pReq, HI_FAILURE, NULL, 0);
            return;
        }

        s32Ret = HI_MPI_VB_DestroyPool(pstVbFree->u32VbPoolId);
        if(s32Ret != HI_SUCCESS)
        {
            printf( "HI_MPI_VB_DestroyPool %d failed\n", s32Ret);
        }

        COMMON_IPCMSG_SendResp(pReq, s32Ret, NULL, 0);
    }
}

static void COMMON_IPCMSG_SVR_HandleRecvMsg(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* pMsg)
{
     printf("common ipcmsg, receive msg id:%d, msg cmd %d \n", s32CommonSvrIPCMsgID, pMsg->u32CMD );
    if(s32Id != s32CommonSvrIPCMsgID)
    {
        printf ("ipcmsg receive msg from error id: %d\n", s32Id);
        return;
    }

    if(pMsg->u32Module != IPCMSG_COMMON_MODID)
    {
        printf ("ipcmsg receive msg from error module id: %d\n", pMsg->u32Module);
        return;
    }

    switch(pMsg->u32CMD)
    {
        case COMMON_MSG_CLIENT_VO_OPEN_REQ:
            COMMON_IPCMSG_SVR_HandleVoOpen(pMsg);
            break;

        case COMMON_MSG_CLIENT_VO_CLOSE_REQ:
            COMMON_IPCMSG_SVR_HandleVoClose(pMsg);
            break;

        case COMMON_MSG_CLIENT_AO_OPEN_REQ:
            COMMON_IPCMSG_SVR_HandleAoOpen(pMsg);
            break;

        case COMMON_MSG_CLIENT_AO_CLOSE_REQ:
            COMMON_IPCMSG_SVR_HandleAoClose(pMsg);
            break;

        case COMMON_MSG_CLIENT_VB_ALLOC_REQ:
            COMMON_IPCMSG_SVR_HandleVbAlloc(pMsg);
            break;

        case COMMON_MSG_CLIENT_VB_FREE_REQ:
            COMMON_IPCMSG_SVR_HandleVbFree(pMsg);
            break;

        default:
            printf ("unsupported cmd request from client\n");
            break;
    }
}

static void* COMMON_IPCMSG_SVR_RecvThread(void *arg)
{
    prctl(PR_SET_NAME, "Hi_pTCommonMsg", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;

    do
    {
        if(HI_TRUE == HI_IPCMSG_IsConnected(s32CommonSvrIPCMsgID))
        {
            HI_IPCMSG_Run(s32CommonSvrIPCMsgID);
        }
        else
        {
            s32Ret = HI_IPCMSG_Disconnect(s32CommonSvrIPCMsgID);
            if(s32Ret != HI_SUCCESS)
            {
                printf ("SVR RecvThread HI_IPCMSG_Disconnect fail\n");
            }

            s32Ret = HI_IPCMSG_Connect(&s32CommonSvrIPCMsgID, IPCMSG_COMMON, COMMON_IPCMSG_SVR_HandleRecvMsg);
            if(s32Ret != HI_SUCCESS)
            {
                printf ("RecvThread HI_IPCMSG_Connect fail\n");
            }
        }
    }while(1);

    return NULL;
}

static void* COMMON_IPCMSG_SVR_WaitClientConnect(void *arg)
{
    pthread_attr_t attr;
    prctl(PR_SET_NAME, "Hi_pCommonMsgwait", 0, 0, 0);

    HI_IPCMSG_CONNECT_S stIpcMsgConnect;
    stIpcMsgConnect.u32RemoteId = 0;
    stIpcMsgConnect.u32Port = IPMSG_COMMON_PORT;
    stIpcMsgConnect.u32Priority = 0;

    if(HI_IPCMSG_AddService(IPCMSG_COMMON, &stIpcMsgConnect) != HI_SUCCESS)
    {
        printf ("HI_IPCMSG_AddService fail\n");
        return NULL;
    }

    printf("thread wait for player client connect.......\n");
    /*wait ipcmsg connect ,block interface*/
    if (HI_SUCCESS != HI_IPCMSG_Connect(&s32CommonSvrIPCMsgID,
        IPCMSG_COMMON, COMMON_IPCMSG_SVR_HandleRecvMsg))
    {
        printf ("HI_IPCMSG_Connect fail\n");
        HI_IPCMSG_DelService(IPCMSG_COMMON);
        return NULL;
    }

    printf("player ipc msg client&server connected  id: %d\n", s32CommonSvrIPCMsgID);

    if(0 != pthread_attr_init(&attr))
    {
        printf ("Error pthread_attr_init()!");
        pthread_attr_destroy(&attr);
        return NULL; //Thread attribute initialise error, then exit
    }

    if(0 != pthread_attr_setstacksize(&attr, COMMON_IPCMSG_THREAD_STACKSIZE))
    {
        printf ("Error pthread_attr_setstacksize()!");
        pthread_attr_destroy(&attr);
        return NULL; //Thread attribute initialise error, then exit
    }
    attr.inheritsched = PTHREAD_EXPLICIT_SCHED;
    attr.schedparam.sched_priority = COMMON_IPCMSG_THREAD_PRIORITY;
    if (0 != pthread_create(&s_CommonRecvThrdId, &attr, COMMON_IPCMSG_SVR_RecvThread, &s32CommonSvrIPCMsgID))
    {
        pthread_attr_destroy(&attr);
        printf ("pthread_create HI_AVPLAY_IPCMSG_SVR_RecvThread failed\n");
        return NULL;
    }

    pthread_attr_destroy(&attr); //Destroying threads attribute structure, cannot be used again before restarting the initialization
    return NULL;
}

HI_S32 COMMON_IPCMSG_SVR_Init()
{
    pthread_t waitConThrd;

    if(bCommonSvrInited)
    {
        printf ("common ipcmsg svr already inited\n");
        return HI_SUCCESS;
    }

    if (0 != pthread_create(&waitConThrd, NULL, COMMON_IPCMSG_SVR_WaitClientConnect, NULL))
    {
        printf( "pthread_create HI_AVPLAY_IPCMSG_SVR_WaitClientConnect failed\n");
        return HI_FAILURE;
    }

    if(0 != pthread_detach(waitConThrd))
    {
        printf ("pthread_detach player ipcmsg connect thread failed\n");
        return HI_FAILURE;
    }
    bCommonSvrInited = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 COMMON_IPCMSG_SVR_Deinit()
{
    if(!bCommonSvrInited)
    {
        printf("player ipcmsg svr have not been inited\n");
        return HI_FAILURE;
    }

    printf("HI_AVPLAY_IPCMSG_SVR_Deinit  id: %d\n", s32CommonSvrIPCMsgID);
    if(s32CommonSvrIPCMsgID)
    {
        HI_IPCMSG_Disconnect(s32CommonSvrIPCMsgID);
        s32CommonSvrIPCMsgID = 0;
    }

     if(s_CommonRecvThrdId)
    {
        pthread_join(s_CommonRecvThrdId, NULL);
        s_CommonRecvThrdId = 0;
    }

    if(HI_IPCMSG_DelService(IPCMSG_COMMON) != HI_SUCCESS)
    {
        printf ("HI_IPCMSG_DelService failed\n");
    }
    bCommonSvrInited = HI_FALSE;
    return HI_SUCCESS;
}
