/**
 * @file    hi_appcomm_msg_server.c
 * @brief   appcomm msg server function.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/29
 * @version   1.0

 */
#include <pthread.h>
#include <errno.h>
#include <sys/prctl.h>
#include "hi_ipcmsg.h"
#include "hi_appcomm_msg_server.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "APPCOMM_SERVER"

/** msg proc node information */


HI_List_Head_S g_stMSGProcList = HI_LIST_INIT_HEAD_DEFINE(g_stMSGProcList);
HI_S32 g_s32MsgFd = -1;
static HI_BOOL s_bMSGServerRun = HI_FALSE;
static pthread_t s_MSGServerTid = 0;

extern HI_VOID MSG_Handler(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* pstMsg);

static HI_VOID* MSG_SERVER_Run(HI_VOID* arg)
{
    prctl(PR_SET_NAME, __func__, 0, 0, 0);
    HI_S32 s32Ret = 0;

    while (s_bMSGServerRun)
    {
        s32Ret = HI_IPCMSG_Connect(&g_s32MsgFd, HI_APPCOMM_MSG_SRVNAME, MSG_Handler);

        if (HI_SUCCESS != s32Ret)
        {
            HI_LOG_PrintFuncErr(HI_IPCMSG_Connect, s32Ret);
            break;
        }

        if (0 < g_s32MsgFd)
        {
            HI_IPCMSG_Run(g_s32MsgFd);
            HI_IPCMSG_Disconnect(g_s32MsgFd);
            g_s32MsgFd = -1;
        }
    }

    HI_LOG_FuncExit();
    return NULL;
}

/**
 * @brief    init the msg server.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 */
HI_S32 HI_MSG_SERVER_Init(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_EXPR(0 == s_MSGServerTid, HI_EINITIALIZED);
    HI_IPCMSG_CONNECT_S stConnectAttr = {0, HI_APPCOMM_MSG_SRVPORT, 1};
    s32Ret = HI_IPCMSG_AddService(HI_APPCOMM_MSG_SRVNAME, &stConnectAttr);

    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_IPCMSG_AddService, s32Ret);
        return HI_EINTER;
    }

    pthread_attr_t threadattr;
    pthread_attr_init(&threadattr);
#ifdef __HuaweiLite__
    threadattr.stacksize = 0x10000;
    threadattr.inheritsched = PTHREAD_EXPLICIT_SCHED;
    threadattr.schedparam.sched_priority = 5;
#endif
    s_bMSGServerRun = HI_TRUE;
    s32Ret = pthread_create(&s_MSGServerTid, &threadattr, MSG_SERVER_Run, NULL);
    pthread_attr_destroy(&threadattr);

    if (HI_SUCCESS != s32Ret)
    {
        HI_IPCMSG_DelService(HI_APPCOMM_MSG_SRVNAME);
        s_bMSGServerRun = HI_FALSE;
        MLOGE("pthread_create fail:%s\n", strerror(errno));
        return HI_ENORES;
    }

    return HI_SUCCESS;
}

/**
 * @brief    deinit the msg server.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 */
HI_S32 HI_MSG_SERVER_Deinit(HI_VOID)
{
    HI_S32 s32Ret = 0;

    s_bMSGServerRun = HI_FALSE;

    if (0 < g_s32MsgFd)
    {
        s32Ret = HI_IPCMSG_Disconnect(g_s32MsgFd);

        if (HI_SUCCESS != s32Ret)
        {
            HI_LOG_PrintFuncErr(HI_IPCMSG_Disconnect, s32Ret);
        }
    }

    pthread_join(s_MSGServerTid, NULL);

    s32Ret = HI_IPCMSG_DelService(HI_APPCOMM_MSG_SRVNAME);

    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_IPCMSG_DelService, s32Ret);
    }

    MSG_PROC_S* pstMsgProc = NULL;
    HI_List_Head_S* pstNode = NULL;
    HI_List_For_Each(pstNode, &g_stMSGProcList)
    {
        pstMsgProc = HI_LIST_ENTRY(pstNode, MSG_PROC_S, stList);
        HI_List_Del(pstNode);
        HI_APPCOMM_SAFE_FREE(pstMsgProc);
        pstNode = &g_stMSGProcList;
    }
    s_MSGServerTid = 0;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

