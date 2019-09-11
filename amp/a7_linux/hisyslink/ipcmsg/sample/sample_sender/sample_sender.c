#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "hi_ipcmsg.h"

static HI_BOOL s_bStopSend = HI_FALSE;

void handle_message(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* msg)
{
    printf("receive msg: %s\n", (char*)msg->pBody);
}

void handle_resp(HI_IPCMSG_MESSAGE_S* pResp)
{
    printf("receive async resp: %s, return:%x\n\n", (char*)pResp->pBody, pResp->s32RetVal);
}

void* receive_thread(void* arg)
{
    HI_S32* pId = (HI_S32*)arg;
    printf("Run...\n");
    HI_IPCMSG_Run(*pId);
    printf("after Run...\n");
    return NULL;
}



void* sendasync_thread(void* arg)
{
    HI_S32* ps32Id = (HI_S32*)arg;
    static HI_U32 s_u32Index = 0;
    char content[16];

    while (HI_FALSE == s_bStopSend)
    {
        memset(content, 0, 16);
        snprintf(content, 16, "async %u", s_u32Index++);
        HI_IPCMSG_MESSAGE_S* pReq = HI_IPCMSG_CreateMessage(2, 5, content, 16);
        HI_IPCMSG_SendAsync(*ps32Id, pReq, handle_resp);
        HI_IPCMSG_DestroyMessage(pReq);

        sleep(1);
    }

    return NULL;
}

void* sendsync_thread(void* arg)
{
    HI_S32* ps32Id = (HI_S32*)arg;
    static HI_U32 s_u32Index = 0;
    char content[16];
    HI_IPCMSG_MESSAGE_S* pReq = NULL;
    HI_IPCMSG_MESSAGE_S* pResp = NULL;

    while (HI_FALSE == s_bStopSend)
    {
        memset(content, 0, 16);
        snprintf(content, 16, "sync %u", s_u32Index++);
        pReq = HI_IPCMSG_CreateMessage(1, 1, content, 16);
        if(NULL == pReq)
        {
            printf("HI_IPCMSG_CreateMessage return null.\n");
            return NULL;
        }

        HI_S32 retVal = HI_IPCMSG_SendSync(*ps32Id, pReq, &pResp, 2000);
        if (HI_SUCCESS == retVal)
        {
            printf("receive sync resp: %s, return:%x\n\n", (char*)pResp->pBody, pResp->s32RetVal);
            HI_IPCMSG_DestroyMessage(pResp);
        }
        else if (HI_IPCMSG_ETIMEOUT == retVal)
        {
            printf("SendSync timeout\n");
        }
        else
        {
            printf("Send fail. Unknow Error\n");
        }

        HI_IPCMSG_DestroyMessage(pReq);

        sleep(2);
    }

    return NULL;
}

int sample_send(int argc, char** argv)
{
    pthread_t receivePid;
    pthread_t sendPid0, sendPid1;

    HI_S32 s32Id;

    if (HI_SUCCESS != HI_IPCMSG_TryConnect(&s32Id, "Test", handle_message))
    {
        printf("Connect fail\n");
        return -1;
    }

    printf("-----Connect: %d\n", HI_IPCMSG_IsConnected(s32Id));

    while (HI_FALSE == HI_IPCMSG_IsConnected(s32Id))
    {
        printf("Wait connection ok\n");
        usleep(200000);
    }

    if (0 != pthread_create(&receivePid, NULL, receive_thread, &s32Id))
    {
        printf("pthread_create receive_thread fail\n");
        return -1;
    }

    if (0 != pthread_create(&sendPid0, NULL, sendsync_thread, &s32Id))
    {
        printf("pthread_create fun fail\n");
        return -1;
    }

    if (0 != pthread_create(&sendPid1, NULL, sendasync_thread, &s32Id))
    {
        printf("pthread_create fun fail\n");
        return -1;
    }

    HI_CHAR cmd[64];

    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Enter q to exit\n");
    }

    s_bStopSend = HI_TRUE;
    pthread_join(sendPid0, NULL);
    pthread_join(sendPid1, NULL);

    HI_IPCMSG_Disconnect(s32Id);

    pthread_join(receivePid, NULL);

    printf("exit\n");

    return 0;
}


int main(int argc, char** argv)
{
    HI_IPCMSG_CONNECT_S stConnectAttr;
    stConnectAttr.u32RemoteId = 1;
    stConnectAttr.u32Port = 201;
    stConnectAttr.u32Priority = 0;
    HI_IPCMSG_AddService("Test", &stConnectAttr);

    sample_send(argc, argv);

    HI_IPCMSG_DelService("Test");

    return 0;
}
