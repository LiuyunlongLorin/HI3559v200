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
#include "hi_appcomm_msg_client.h"
#include "hi_product_media.h"
#include "vio_param.h"
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

/** sensor name */
#if defined(CONFIG_SNS0_IMX307) && defined(CONFIG_SNS1_IMX307)
#define SNS_NAME  "imx307+imx307"
#elif defined(CONFIG_SNS0_IMX377)
#define SNS_NAME  "imx377"
#elif defined(CONFIG_SNS0_IMX458)
#define SNS_NAME  "imx458"
#elif defined(CONFIG_SNS0_IMX335)
#define SNS_NAME  "imx335"
#elif defined(CONFIG_SNS0_OS05A)
#define SNS_NAME  "os05a10"
#elif defined(CONFIG_SNS0_OS08A10)
#define SNS_NAME  "os08a10"
#elif defined(CONFIG_SNS0_IMX307)
#define SNS_NAME  "imx307"
#else
#define SNS_NAME  ""
#endif

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
    HI_PDT_MEDIA_VideoDeinit();
    HI_PDT_MEDIA_Deinit();
}

static HI_S32 InitMedia(HI_S32 snsIdx, PARAM_DisplayType dispType)
{
    MEDIA_Cfg mediaCfg;
    CHECK_RET(HI_PARAM_GetMediaCfg(snsIdx, dispType, &mediaCfg));
    CHECK_RET(HI_PDT_MEDIA_Init(&mediaCfg.viVpssMode, &mediaCfg.vbCfg));
    CHECK_RET(HI_PDT_MEDIA_VideoInit(&mediaCfg.videoCfg));
    CHECK_RET(HI_PDT_MEDIA_VideoOutInit(&mediaCfg.videoOutCfg));
    CHECK_RET(HI_PDT_MEDIA_VideoOutStart(&mediaCfg.videoOutCfg));
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

static HI_VOID VIO_Usage(HI_VOID)
{
    printf("Usage : \n");
    printf("\t 0) VIO HDMI Video preview.\n");
    printf("\t 1) VIO LCD Video preview.\n");
    printf("\t 2) VIO Video no preview.\n");
    printf("\t other quit\n");
    return;
}

static HI_VOID VIO_SnsSeqUsage(HI_VOID)
{
    printf("Sensor(%s) Seq List: \n", SNS_NAME);
    HI_S32 i;
    HI_CHAR desc[256] = {0,};
    for (i = 0; i < HI_PARAM_GetSnsSeqCnt(); ++i) {
        HI_PARAM_GetSnsSeqDesc(i, desc, sizeof(desc));
        printf("\t %d)\t %s\n", i, desc);
    }
    printf("\t other) quit\n");
}

static HI_VOID VIO_SnsSeqProc(PARAM_DisplayType dispType)
{
    HI_CHAR readStr[128] = {0,};
    HI_S32  snsSeqIdx = 0;

    while(1) {
        VIO_SnsSeqUsage();

        memset(readStr, 0, sizeof(readStr));
        if(gets(readStr) == NULL) {
            MLOGW("quit\n");
            break;
        }
        if (sscanf(readStr, "%d", &snsSeqIdx) != 1) {
            MLOGW("quit\n");
            break;
        }
        if (snsSeqIdx >= 0 && snsSeqIdx < HI_PARAM_GetSnsSeqCnt()) {
            DeinitMedia();
            CHECK_RET(InitMedia(snsSeqIdx, dispType));
        } else {
            MLOGW("quit\n");
            break;
        }
    }

exit:

    DeinitMedia();
}

static HI_VOID VIO_HdmiPreview(HI_VOID)
{
#if defined(HDMI_SUPPORT)
    VIO_SnsSeqProc(PARAM_DISPLAY_TYPE_HDMI);
#else
    MLOGW("not support hdmi\n");
#endif
}

static HI_VOID VIO_LcdPreview(HI_VOID)
{
#if defined(CONFIG_SCREEN)
    extern HI_HAL_SCREEN_OBJ_S stHALSCREENObj;
    CHECK_RET(HI_HAL_SCREEN_Register(HI_HAL_SCREEN_IDX_0, &stHALSCREENObj));
    CHECK_RET(HI_HAL_SCREEN_Init(HI_HAL_SCREEN_IDX_0));

    VIO_SnsSeqProc(PARAM_DISPLAY_TYPE_LCD);
exit:
#else
    MLOGW("not support lcd\n");
#endif
    return;
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
        goto exit;
    }

    VIO_Usage();

    HI_CHAR readCh = 0;
    readCh = (char)getchar();
    (void)getchar();

    switch (readCh) {
        case '0':
            VIO_HdmiPreview();
            break;
        case '1':
            VIO_LcdPreview();
            break;
        case '2':
            VIO_SnsSeqProc(PARAM_DISPLAY_TYPE_NONE);
            break;
        default:
            MLOGE("invalid input param\n");
            VIO_Usage();
            break;
    }

exit:
    ret = HI_MSG_CLIENT_Deinit();
    ret |= HI_MAPI_Sys_Deinit();
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

