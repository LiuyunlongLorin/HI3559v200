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
#include "hi_appcomm_msg_client.h"
#include "hi_product_media.h"
#include "lcd_param.h"
#include "hi_hal_screen.h"

#include "hi_mapi_sys.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

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
static HI_VOID DeinitMedia(HI_VOID)
{
    HI_PDT_MEDIA_VideoOutStop();
    HI_PDT_MEDIA_VideoOutDeinit();
    HI_PDT_MEDIA_Deinit();
    HI_HAL_SCREEN_Deinit(HI_HAL_SCREEN_IDX_0);
}

/**0 is horizental, 1 is vertial*/
static HI_VOID ShowColorbar(HI_VOID)
{
#if (defined(HI3556AV100) || defined(HI3519AV100))
himm(0x0470d000,0xc0000011);
#elif (defined(HI3556V200) || defined(HI3559V200))
himm(0x1144d000,0xc0000011);
#elif (defined(HI3518EV300))
himm(0x1128d000,0xc0000011);
#endif
}

static HI_S32 InitMedia(HI_VOID)
{
    extern HI_HAL_SCREEN_OBJ_S stHALSCREENObj;
    MEDIA_Cfg mediaCfg;
    CHECK_RET(HI_HAL_SCREEN_Register(HI_HAL_SCREEN_IDX_0, &stHALSCREENObj));
    CHECK_RET(HI_HAL_SCREEN_Init(HI_HAL_SCREEN_IDX_0));
    CHECK_RET(HI_PARAM_GetMediaCfg(&mediaCfg));
    CHECK_RET(HI_PDT_MEDIA_Init(&mediaCfg.viVpssMode, &mediaCfg.vbCfg));
    CHECK_RET(HI_PDT_MEDIA_VideoOutInit(&mediaCfg.videoOutCfg));
    CHECK_RET(HI_PDT_MEDIA_VideoOutStart(&mediaCfg.videoOutCfg));
    ShowColorbar();
    return HI_SUCCESS;

exit:

    DeinitMedia();
    return HI_FAILURE;
}

static HI_VOID CrashHandler(HI_S32 s32Sig, siginfo_t *pstSigInfo, HI_VOID *pvSecret)
{
    MLOGE("Signal is %d\n", s32Sig);
    DeinitMedia();
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

static HI_VOID VO_Usage(HI_VOID)
{
    printf("Usage : \n");
    printf("\t 0) LCD Display. Show image.\n");
    printf("\t other quit\n");
    return;
}

static HI_VOID VO_LCDPreview(HI_VOID)
{
    CHECK_RET(InitMedia());
    printf("Press Enter key to stop preview...\n");
    while (getchar() != '\n') {
        continue;
    }
exit:
    DeinitMedia();
}

int main(int argc, char *argv[])
{
    HI_S32 ret = HI_SUCCESS;
    SetCrashHandleType();

    ret = HI_MAPI_Sys_Init();
    if (ret != HI_SUCCESS) {
        MLOGE("HI_MAPI_Sys_Init fail[%x]\n", ret);
        return HI_FAILURE;
    }

    ret = HI_MSG_CLIENT_Init();
    if (ret != HI_SUCCESS) {
        MLOGE("HI_MSG_CLIENT_Init fail[%x]\n", ret);
        return HI_FAILURE;
    }

    VO_Usage();

    HI_CHAR readCh = 0;
    readCh = (char)getchar();
    (void)getchar();

    switch (readCh) {
        case '0':
            VO_LCDPreview();
            break;
        default:
            MLOGE("invalid input param\n");
            VO_Usage();
            return HI_FAILURE;
    }

    ret = HI_MSG_CLIENT_Deinit();
    if (ret != HI_SUCCESS) {
        MLOGE("HI_MSG_CLIENT_Deinit fail[%x]\n", ret);
    }

    ret = HI_MAPI_Sys_Deinit();
    if (ret == HI_SUCCESS) {
        MLOGI(GREEN"exited successfully!"NONE"\n");
    } else {
        MLOGE(RED"exits abnormally!"NONE"\n");
    }

    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

