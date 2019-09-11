/**
 * @file    vio_main.c
 * @brief   vio sample
 *
 * Copyright (c) 2017 Huawei Tech.Co., Ltd.
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2019/4/2
 * @version   1.0

 */

#include "hi_appcomm_log.h"
#include "hi_appcomm_util.h"
#include "hi_hal_touchpad.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#ifndef CHECK_RET
#define CHECK_RET(express)\
    do{\
        HI_S32 ret__ = express;\
        if (HI_SUCCESS != ret__)\
        {\
            MLOGE("fail[%x]\n", ret__);\
            goto exit;\
        }\
    }while(0)
#endif

static HI_S32 g_tpfd = -1;
static HI_BOOL g_threadRun = HI_FALSE;
static pthread_t g_threadId = -1;


static HI_VOID StopTouchpad(HI_VOID)
{
    HI_HAL_TOUCHPAD_Stop();
    HI_HAL_TOUCHPAD_Deinit();
}

static HI_VOID TPShowlog(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(g_tpfd, &readset);
    struct timeval timeout = {0};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    ret = select(g_tpfd + 1, &readset, 0, 0, &timeout);

    if (ret == 0) {
        return ;
    }
    else if (ret < 0) {
        MLOGE("Error.\n");
        return ;
    }

    if (FD_ISSET(g_tpfd, &readset)) {
        HI_HAL_TOUCHPAD_INPUTINFO_S tpInputInfo = {0};
        ret = HI_HAL_TOUCHPAD_ReadInputEvent(&tpInputInfo);
        if (HI_SUCCESS != ret) {
            MLOGE("Error.\n");
            return ;
        }
        MLOGI("Tp-->id is %d.\n",tpInputInfo.s32ID);
        MLOGI("Tp-->X is %d.\n",tpInputInfo.s32X);
        MLOGI("Tp-->Y is %d.\n",tpInputInfo.s32Y);
        MLOGI("Tp-->Pressure is %d.\n",tpInputInfo.u32Pressure);
    }
    return ;
}

static HI_VOID* TPCheckThread(HI_VOID *pData)
{

    MLOGD("thread TP Check enter\n");
    HI_CHAR szThreadName[64];
    snprintf(szThreadName, 64, "%s", "TP_CHECK_");
    prctl(PR_SET_NAME, szThreadName, 0, 0, 0); /**< Set Task Name */

    while (g_threadRun) {
        TPShowlog();
    }

    MLOGD("thread TP Check exit\n");
    return NULL;
}

static HI_S32 StartTouchpad(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    CHECK_RET(HI_HAL_TOUCHPAD_Init());
    CHECK_RET(HI_HAL_TOUCHPAD_Resume());
    CHECK_RET(HI_HAL_TOUCHPAD_Start(&g_tpfd));
    if (g_tpfd < 0) {
        MLOGE("Open TouchPad Device failed.\n");
        goto exit;
    }
    g_threadRun = HI_TRUE;
    ret = pthread_create(&g_threadId, HI_NULL, TPCheckThread,HI_NULL);
    if (ret != HI_SUCCESS) {
        MLOGE("Create Tp Thread Fail!\n");
        goto exit;
    }
    return HI_SUCCESS;
exit:
    return HI_FAILURE;
}

static HI_VOID CrashHandler(HI_S32 s32Sig, siginfo_t *pstSigInfo, HI_VOID *pvSecret)
{
    MLOGE("Signal is %d\n", s32Sig);
    exit(EXIT_FAILURE);
}

static HI_VOID SetCrashHandleType(HI_VOID)
{
    MLOGD("use sigaction to handle crash\n\n");
    struct sigaction stAct;
    sigemptyset(&stAct.sa_mask);
    stAct.sa_flags = SA_SIGINFO;
    stAct.sa_sigaction = CrashHandler;
    sigaction(SIGSEGV, &stAct, NULL);
    sigaction(SIGFPE, &stAct, NULL);
    sigaction(SIGABRT, &stAct, NULL);
    sigaction(SIGBUS, &stAct, NULL);
}

static HI_VOID TP_Usage(HI_VOID)
{
    printf("Usage : \n");
    printf("\t 0) TP Start. Please touch and see terminal log.\n");
    printf("\t other quit\n");
    return;
}

static HI_VOID TP_Start(HI_VOID)
{
    CHECK_RET(StartTouchpad());
    HI_usleep(2*1000*1000);
    printf("Press Enter key to stop Touchpad...\n");
    while (getchar() != '\n') {
        continue;
    }
    g_threadRun = HI_FALSE;
    pthread_join(g_threadId, NULL);
exit:
    StopTouchpad();
}

int main(int argc, char *argv[])
{
    HI_S32 ret = HI_SUCCESS;
    SetCrashHandleType();

    TP_Usage();

    HI_CHAR readCh = 0;
    readCh = (char)getchar();
    (void)getchar();

    switch (readCh) {
        case '0':
            TP_Start();
            break;
        default:
            MLOGE("invalid input param\n");
            TP_Usage();
            return HI_FAILURE;
    }

    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

