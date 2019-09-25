/**
 * @file      hi_product_main.c
 * @brief     the product main function.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 * @version   1.0
 */
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <signal.h>
#ifdef __HI_GLIBC__
#include <execinfo.h>
#endif
#include <ucontext.h>
#include <semaphore.h>

/** ndk and sdk */
#include "hi_type.h"
#include "hi_mapi_sys.h"


/** middleware */
#include "hi_eventhub.h"
#include "hi_message.h"
#include "hi_hfsm.h"

/** hal */
#include "hi_hal_gauge.h"
#include "hi_hal_key.h"
#include "hi_hal_screen.h"
#include "hi_hal_touchpad.h"
#include "hi_hal_wifi.h"

/** common */
#include "hi_gaugemng.h"
#include "hi_keymng.h"
#include "hi_storagemng.h"
#include "hi_filemng_dcf.h"
//#include "hi_liveserver.h"
#include "hi_appcomm.h"
#include "hi_appcomm_util.h"
#include "hi_timestamp.h"
#include "hi_appcomm_msg_client.h"
#include "hi_timedtask.h"
#include "hi_system.h"
#include "hisnet.h"
#include "hi_filetrans.h"
#include "hi_player.h"

/** pdt */
#include "hi_product_param.h"
#include "hi_product_statemng.h"
#include "hi_product_ui.h"
#include "ui_common.h"
#include "hi_product_usbctrl.h"
#include "hi_product_media.h"
#include "hi_product_netctrl.h"
#include "hi_upgrade.h"

#ifdef CONFIG_RAWCAP_ON
#include "hi_rawcapture.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** redefine module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "Init"

#define PDT_APP_COMMAND_DEBUG (0)
#define PDT_APP_FAST_EXIT (1)

#define PDT_BATTERT_LOW_LEVEL       (10)
#define PDT_BATTERT_ULTRA_LOW_LEVEL (5)

/** function ret value check */
#define PDT_APP_CHECK_RET(RetValue,ErrString)\
do{\
    if(HI_SUCCESS != RetValue)\
    {\
        MLOGE(RED"%s failed, s32Ret[0x%08X]\n\n"NONE, ErrString, RetValue);\
    }\
    else\
    {\
        MLOGD("%s success, s32Ret[0x%08X]\n\n", ErrString, RetValue);\
        HI_TIME_STAMP;\
    }\
  }while(0)

static pthread_t s_KoThread;
static pthread_t s_SrvPostThread;
static sem_t s_PowerOffSem;/** power off semaphore */
static HI_EXIT_MODE_E s_ExitMode = HI_EXIT_MODE_BUTT;/** exit mode */

#define PDT_APP_WAITSHAREFS_TIMEOUT       (4)

#ifndef CONFIG_COREDUMP_ON

HI_VOID PDT_INIT_MediaDeinit()  //Lorin Modity
{
    //HI_PDT_MEDIA_VideoOutStop();
    HI_PDT_MEDIA_VideoDeinit(HI_TRUE);
    //HI_PDT_MEDIA_VideoOutDeinit();
    HI_PDT_MEDIA_AudioDeinit();
    HI_PDT_MEDIA_AudioOutStop();
    HI_PDT_MEDIA_Deinit();

#ifdef CONFIG_MOTIONSENSOR
    HI_PDT_MEDIA_DeinitMotionsensor();
#endif

    HI_PDT_STATEMNG_Deinit();
    HI_PDT_PARAM_Deinit();
    HI_MAPI_Sys_Deinit();
}

/** crash handler function */
static HI_VOID PDT_CrashHandler(HI_S32 s32Sig, siginfo_t *pstSigInfo, HI_VOID *pvSecret)
{
    ucontext_t *pstUc = (ucontext_t*)pvSecret;

    MLOGE("Signal is %d\n", s32Sig);
    PDT_INIT_MediaDeinit();

    if(NULL != pstSigInfo)
    {
        MLOGE("info.si_signo:  %d \n", pstSigInfo->si_signo);
        MLOGE("info.si_code:   %d \n", pstSigInfo->si_code);
        MLOGE("info.si_errno:  %d \n", pstSigInfo->si_errno);
        MLOGE("info.si_addr:   %p \n", pstSigInfo->si_addr);
        MLOGE("info.si_status: %d \n", pstSigInfo->si_status);
        MLOGE("info.si_band:   %ld\n", pstSigInfo->si_band);
    }

    if(NULL != pstUc)
    {
        MLOGE("uc_mcontext.fault_address:   %#lx\n", (HI_UL)pstUc->uc_mcontext.fault_address);
    }

    /** need gcc option "-g -rdynamic" */
#ifdef __HI_GLIBC__
    HI_VOID *pvBuffer[64];
    HI_CHAR **ppStrings;
    HI_S32 s32Nptrs = 0;
    s32Nptrs = backtrace(pvBuffer, sizeof(pvBuffer)/sizeof(pvBuffer[0]));
    MLOGE("backtrace() got %d addresses\n", s32Nptrs);

    ppStrings = backtrace_symbols(pvBuffer, s32Nptrs);
    if(ppStrings == NULL)
    {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    HI_S32 j = 0;
    for(j = 0; j < s32Nptrs; j++)
    {
        MLOGE("[%d] %s\n", j, ppStrings[j]);
    }
    free(ppStrings);
#endif
    exit(EXIT_FAILURE);
}
#endif

static HI_VOID PDT_SetCrashHandleType(HI_VOID)
{
#ifndef CONFIG_COREDUMP_ON
    MLOGD("use sigaction to handle crash\n\n");
    struct sigaction stAct;
    sigemptyset(&stAct.sa_mask);
    stAct.sa_flags = SA_SIGINFO;
    stAct.sa_sigaction = PDT_CrashHandler;
    sigaction(SIGSEGV, &stAct, NULL);
    sigaction(SIGFPE, &stAct, NULL);
    sigaction(SIGABRT, &stAct, NULL);
    sigaction(SIGBUS, &stAct, NULL);
#else
    MLOGD("use coredump to handle crash\n\n");
    system("echo \"/dev/core.%e-%p-%t\"> /proc/sys/kernel/core_pattern");
#endif
}
static HI_VOID PDT_LoadMmcDriver(HI_VOID)
{
    /** load mmc drv */
    HI_insmod("/app/komod/mmc_core.ko",NULL);
    HI_insmod("/app/komod/mmc_block.ko",NULL);
#if defined(HI3556AV100) || defined(HI3559AV100)
    HI_insmod("/app/komod/sdhci.ko",NULL);
    HI_insmod("/app/komod/sdhci-pltfm.ko",NULL);
    HI_insmod("/app/komod/sdhci-of-hisi.ko",NULL);
#elif defined(HI3559V200)
    HI_insmod("/app/komod/hisi_mci.ko",NULL);
#endif
}

// static HI_VOID PDT_LoadUSBPhy(HI_VOID)
// {
//     HI_insmod("/app/komod/dwc3.ko",NULL);
// }
/** load driver task */
static HI_VOID* PDT_LoadDriver(void* pVoid)
{
    //HI_S32 s32Ret = HI_SUCCESS;
    pthread_detach(pthread_self());

#ifdef CFG_LCD_TOUCHPAD_ON  //Lroin modity
    // s32Ret = HI_HAL_TOUCHPAD_Init();
    // PDT_APP_CHECK_RET(s32Ret, "HI_HAL_TOUCHPAD_Init");

    // s32Ret = HI_HAL_TOUCHPAD_Suspend();
    // PDT_APP_CHECK_RET(s32Ret, "HI_HAL_TOUCHPAD_Suspend");
#endif

    //PDT_LoadUSBPhy();
    return HI_NULL;
}

/** create thread to load driver */
static HI_S32 PDT_LoadDriverThread()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = pthread_create(&s_KoThread, NULL, (void*)PDT_LoadDriver, NULL);
    PDT_APP_CHECK_RET(s32Ret, "pthread_create for PDT_LoadDriver");
    return s32Ret;
}

/** delayed services start task */
static HI_S32 PDT_ServiceDelayedStart()
{
    pthread_detach(pthread_self());
    HI_S32 s32Ret = HI_SUCCESS;

    /** keymng init */
    HI_KEYMNG_CFG_S stKeyCfg;
    s32Ret = HI_PDT_PARAM_GetKeyMngCfg(&stKeyCfg);
    PDT_APP_CHECK_RET(s32Ret, "get keymng cfg");

    s32Ret = HI_KEYMNG_Init((const HI_KEYMNG_CFG_S*)&stKeyCfg);
    PDT_APP_CHECK_RET(s32Ret, "HI_KEYMNG_Init");
    MLOGD(GREEN"\nLinux key available\n"NONE);

#ifdef CFG_TIME_MEASURE_ON
    HI_PrintBootTime("KeyAvailable");
#endif

#ifdef CFG_LCD_TOUCHPAD_ON
    /** active touchpad */
    // s32Ret = HI_HAL_TOUCHPAD_Resume();
    // PDT_APP_CHECK_RET(s32Ret, "HI_HAL_TOUCHPAD_Resume");
#endif

    //PDT_LoadMmcDriver();

    struct sigaction stAct = {};
    stAct.sa_handler = SIG_IGN;
    sigaction( SIGPIPE, &stAct, 0 );
    return s32Ret;
}

/** create thread to start delay services */
static HI_S32 PDT_ServiceDelayedStartThread()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = pthread_create(&s_SrvPostThread, NULL, (void*)PDT_ServiceDelayedStart, NULL);
    PDT_APP_CHECK_RET(s32Ret, "pthread_create for PDT_ServiceDelayedStart");
    return s32Ret;
}

HI_S32 PDT_StoragemngFormatPreProc(const HI_CHAR* pszMntPath)
{
    MLOGD("PDT Pre Proccess Format\n");
    (HI_VOID)HI_FILETRANS_CloseAllLink();
    (HI_VOID)HISNET_CleanMmapCache();
    return HI_SUCCESS;
}

static HI_S32 PDT_ExitModeCallback(HI_EXIT_MODE_E enExitMode)
{
    MLOGD(GREEN"post s_PowerOffSem enExitMode(%d)\n"NONE, enExitMode);
    s_ExitMode = enExitMode;/** exit mode */
    sem_post(&s_PowerOffSem);
    return HI_SUCCESS;
}

#if PDT_APP_COMMAND_DEBUG
static HI_VOID PDT_CommandDebugOption(HI_VOID)
{
    printf("\n");
    printf(GREEN"0  --- quit \n"
                "1  --- switch to siglephoto \n"
                "2  --- switch to normal record \n"
                "3  --- switch to record snap \n"
                "4  --- start \n"
                "5  --- stop \n"
                "6  --- get state \n"
                "7  --- get storage state \n"
                "8  --- set media mode\n"
                "9  --- switch workmode \n"
                "10 --- setting atrribute \n"
                "11 --- choose Charge mode \n"
                "12 --- choose UVC mode \n"
                "13 --- choose USB Storage mode \n"
                "14 --- poweroff \n"
                "15 --- factory reset \n"
                "16 --- set spot metry \n"
                "17 --- set wifi \n"
                "18 --- set raw cap on/off\n"NONE);
}

static HI_VOID PDT_CommandDebug(HI_PDT_WORKMODE_E enPoweronWorkmode,
    HI_STORAGEMNG_CFG_S* pstStorageMngCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 CMD = 0;
    HI_PDT_WORKMODE_E enCurrWorkMode = enPoweronWorkmode;
    while(1)
    {
        PDT_CommandDebugOption();
        scanf("%d", &CMD);

        /** get current workmode */
        if(7 != CMD)
        {
            HI_PDT_STATEMNG_WORKMODE_STATE_S stCrrentWorkModeState;
            s32Ret = HI_PDT_STATEMNG_GetState(&stCrrentWorkModeState);
            PDT_APP_CHECK_RET(s32Ret, "HI_PDT_STATEMNG_GetState");
            enCurrWorkMode = stCrrentWorkModeState.enWorkMode;
        }

        switch(CMD)
        {
            case 0:
            {
                printf(GREEN"\nQuit command line debug\n"NONE);
                break;
            }
            case 1:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                stMessage.arg2 = HI_PDT_WORKMODE_SING_PHOTO;
                printf(GREEN"\nsend switch workmode to HI_PDT_WORKMODE_SING_PHOTO\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage switch workmode to single photo");
                enCurrWorkMode = HI_PDT_WORKMODE_SING_PHOTO;
                sleep(2);
                break;
            }
            case 2:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
                printf(GREEN"\nsend switch workmode to HI_PDT_WORKMODE_NORM_REC\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage switch workmode to normal record");
                enCurrWorkMode = HI_PDT_WORKMODE_NORM_REC;
                sleep(2);
                break;
            }
            case 3:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                stMessage.arg2 = HI_PDT_WORKMODE_RECSNAP;
                printf(GREEN"\nsend switch workmode to HI_PDT_WORKMODE_RECSNAP\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage switch workmode to recordsnap");
                enCurrWorkMode = HI_PDT_WORKMODE_RECSNAP;
                sleep(2);
                break;
            }
            case 4:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_START;
                stMessage.arg2 = enCurrWorkMode;
                printf(GREEN"\nsend message HI_EVENT_STATEMNG_START, workmode(%d)\n"NONE,
                    enCurrWorkMode);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage start");
                sleep(5);
                break;
            }
            case 5:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_STOP;
                stMessage.arg2 = enCurrWorkMode;
                printf(GREEN"\nsend message HI_EVENT_STATEMNG_STOP\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage stop");
                sleep(2);
                break;
            }
            case 6:
            {
                HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
                printf(GREEN"\nget state HI_PDT_STATEMNG_GetState\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
                PDT_APP_CHECK_RET(s32Ret, "HI_PDT_STATEMNG_GetState");
                printf("current workmode(%d), bRunning(%d), bStateMngInProgress(%d)\n\n",
                    stWorkModeState.enWorkMode, stWorkModeState.bRunning,
                    stWorkModeState.bStateMngInProgress);
                HI_TIME_PRINT;
                HI_TIME_CLEAN;
                sleep(2);
                break;
            }
            case 7:
            {
                HI_STORAGE_STATE_E enGetState;
                s32Ret = HI_STORAGEMNG_GetState(pstStorageMngCfg->szMntPath, &enGetState);
                PDT_APP_CHECK_RET(s32Ret,"get storage state");
                printf(GREEN"\nSTATEMNG current storage state(%d)\n"NONE, enGetState);
                sleep(2);
                break;
            }
            case 8:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SETTING;
                stMessage.arg1 = HI_PDT_PARAM_TYPE_MEDIAMODE;
                stMessage.arg2 = enCurrWorkMode;

                /** get surpport list*/
                HI_PDT_ITEM_VALUESET_S stValueSet;
                s32Ret = HI_PDT_PARAM_GetWorkModeItemValues(enCurrWorkMode,
                    HI_PDT_PARAM_TYPE_MEDIAMODE, &stValueSet);
                PDT_APP_CHECK_RET(s32Ret,"PARAM_GetWorkModeItemValues");
                printf("enCurrWorkMode(%d), attribute type(%d), support list has [%d] item:\n",
                    enCurrWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE, stValueSet.s32Cnt);
                HI_S32 i = 0;
                for(i = 0; i < stValueSet.s32Cnt; i++)
                {
                    printf("item[%d]: value %d\n", i+1, stValueSet.astValues[i].s32Value);
                }
                printf("\n");

                HI_S32 s32SelectItem = 0;
                printf("please select an item value:");
                scanf("%d", &s32SelectItem);
                char* payload = (stMessage.aszPayload);
                *((HI_S32*)payload) = s32SelectItem;
                printf(GREEN"\nsend message HI_EVENT_STATEMNG_SETTING, "
                    "setting type:HI_PDT_PARAM_TYPE_MEDIAMODE, workmode:%d, value:%d \n"NONE,
                    enCurrWorkMode, s32SelectItem);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage HI_EVENT_STATEMNG_SETTING for media mode");
                sleep(5);
                break;
            }
            case 9:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                printf("the support workmode type in the system:\n");
                HI_S32 i = 0;
                printf("item[%d]: HI_PDT_WORKMODE_NORM_REC, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_NORM_REC);
                printf("item[%d]: HI_PDT_WORKMODE_LOOP_REC, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_LOOP_REC);
                printf("item[%d]: HI_PDT_WORKMODE_LPSE_REC, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_LPSE_REC);
                printf("item[%d]: HI_PDT_WORKMODE_SLOW_REC, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_SLOW_REC);
                printf("item[%d]: HI_PDT_WORKMODE_SING_PHOTO, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_SING_PHOTO);
                printf("item[%d]: HI_PDT_WORKMODE_DLAY_PHOTO, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_DLAY_PHOTO);
                printf("item[%d]: HI_PDT_WORKMODE_LPSE_PHOTO, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_LPSE_PHOTO);
                printf("item[%d]: HI_PDT_WORKMODE_BURST, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_BURST);
                printf("item[%d]: HI_PDT_WORKMODE_RECSNAP, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_RECSNAP);
                printf("item[%d]: HI_PDT_WORKMODE_PLAYBACK, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_PLAYBACK);
                printf("item[%d]: HI_PDT_WORKMODE_UVC, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_UVC);
                printf("item[%d]: HI_PDT_WORKMODE_USB_STORAGE, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_USB_STORAGE);
                printf("item[%d]: HI_PDT_WORKMODE_SUSPEND, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_SUSPEND);
                printf("item[%d]: HI_PDT_WORKMODE_HDMI_PREVIEW, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_HDMI_PREVIEW);
                printf("item[%d]: HI_PDT_WORKMODE_HDMI_PLAYBACK, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_HDMI_PLAYBACK);
                printf("\n");

                HI_S32 s32SelectItem = 0;
                printf("please select a workmode type value:");
                scanf("%d", &s32SelectItem);
                stMessage.arg2 = s32SelectItem;

                printf(GREEN"\nsend switch workmode to workmode(%d)\n"NONE, s32SelectItem);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage switch workmode");
                enCurrWorkMode = s32SelectItem;
                sleep(2);
                break;
            }
            case 10:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SETTING;

                printf("the support setting type in the system:\n");
                HI_S32 i = 0;
                printf("item[%d]: HI_PDT_PARAM_TYPE_MEDIAMODE, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_MEDIAMODE);
                printf("item[%d]: HI_PDT_PARAM_TYPE_PHOTO_SCENE, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_PHOTO_SCENE);
                printf("item[%d]: HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT);
                printf("item[%d]: HI_PDT_PARAM_TYPE_DELAY_TIME, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_DELAY_TIME);
                printf("item[%d]: HI_PDT_PARAM_TYPE_LAPSE_INTERVAL, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_LAPSE_INTERVAL);
                printf("item[%d]: HI_PDT_PARAM_TYPE_BURST_TYPE, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_BURST_TYPE);
                printf("item[%d]: HI_PDT_PARAM_TYPE_LOOP_TIME, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_LOOP_TIME);
                printf("item[%d]: HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE);
                printf("item[%d]: HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV);
                printf("item[%d]: HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME);
                printf("item[%d]: HI_PDT_PARAM_TYPE_PROTUNE_ISO, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_PROTUNE_ISO);
                printf("item[%d]: HI_PDT_PARAM_TYPE_PROTUNE_WB, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_PROTUNE_WB);
                printf("item[%d]: HI_PDT_PARAM_TYPE_PROTUNE_METRY, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_PROTUNE_METRY);
                printf("item[%d]: HI_PDT_PARAM_TYPE_OSD, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_OSD);
                printf("item[%d]: HI_PDT_PARAM_TYPE_AUDIO, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_AUDIO);
                printf("item[%d]: HI_PDT_PARAM_TYPE_FLIP, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_FLIP);
                printf("item[%d]: HI_PDT_PARAM_TYPE_DIS, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_DIS);
                printf("item[%d]: HI_PDT_PARAM_TYPE_LDC, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_LDC);
                printf("\n");

                HI_S32 s32SelectItem = 0;
                printf("please select a setting type value:");
                scanf("%d", &s32SelectItem);
                stMessage.arg1 = s32SelectItem;
                stMessage.arg2 = enCurrWorkMode;

                if( HI_PDT_PARAM_TYPE_OSD != s32SelectItem &&
                    HI_PDT_PARAM_TYPE_AUDIO != s32SelectItem &&
                    HI_PDT_PARAM_TYPE_FLIP != s32SelectItem &&
                    HI_PDT_PARAM_TYPE_DIS != s32SelectItem &&
                    HI_PDT_PARAM_TYPE_LDC != s32SelectItem )
                {
                    /** get surpport list*/
                    HI_PDT_ITEM_VALUESET_S stValueSet;
                    s32Ret = HI_PDT_PARAM_GetWorkModeItemValues(enCurrWorkMode,
                        s32SelectItem, &stValueSet);
                    PDT_APP_CHECK_RET(s32Ret,"PARAM_GetWorkModeItemValues");
                    if(HI_PDT_PARAM_EUNSUPPORT == s32Ret)
                    {
                        printf(YELLOW"unsupport setting type\n\n"NONE);
                        break;
                    }

                    printf("enCurrWorkMode(%d), attribute type(%d), support list has [%d] item:\n",
                           enCurrWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE, stValueSet.s32Cnt);

                    for(i = 0; i < stValueSet.s32Cnt; i++)
                    {
                        printf("item[%d]: value %d\n", i+1, stValueSet.astValues[i].s32Value);
                    }
                    printf("\n");
                }
                else
                {
                    printf("enable: value 1\ndisable: value 0\n");
                }
                printf("please select an item value for setting value:");
                scanf("%d", &s32SelectItem);
                char* payload = (stMessage.aszPayload);
                *((HI_S32*)payload) = s32SelectItem;
                printf(GREEN"\nsend message HI_EVENT_STATEMNG_SETTING, setting type:%d, "
                            "workmode:%d, value:%d \n"NONE,
                            stMessage.arg1, enCurrWorkMode, s32SelectItem);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage HI_EVENT_STATEMNG_SETTING");
                sleep(5);
                break;
            }
            case 11:
            {
                printf("set to charge mode, to be enable and verify\n");
                #if 0
                HI_USB_SetMode(HI_USB_MODE_CHARGE);
                #endif
                break;
            }
            case 12:
            {
                printf("set to UVC mode, to be enable and verify\n");
                #if 0
                HI_USB_SetMode(HI_USB_MODE_UVC);
                #endif
                break;
            }
            case 13:
            {
                printf("set to USB Storage mode, to be enable and verify\n");
                #if 0
                HI_USB_SetMode(HI_USB_MODE_STORAGE);
                #endif
                break;
            }
            case 14:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_POWEROFF;
                printf(GREEN"\nsend HI_EVENT_STATEMNG_POWEROFF\n"NONE);
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage poweroff message");
                break;
            }
            case 15:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_FACTORY_RESET;
                printf(GREEN"\nsend HI_EVENT_STATEMNG_FACTORY_RESET\n"NONE);
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage factory reset message");
                break;
            }
            case 16:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SETTING;
                stMessage.arg1 = HI_PDT_PARAM_TYPE_PROTUNE_METRY;

                char* payload = (stMessage.aszPayload);
                *(HI_PDT_SCENE_METRY_TYPE_E *)payload = HI_PDT_SCENE_METRY_TYPE_SPOT;

                HI_PDT_SCENE_METRY_SPOT_PARAM_S stSpotMetryParam;


                HI_S32 s32Percents32X = 0, s32Percents32Y = 0, s32Percentu32Height = 0, s32Percentu32Width = 0;
                printf("please input percent value for s32X s32Y u32Height u32Width value:");
                scanf("%d %d %d %d", &s32Percents32X, &s32Percents32Y, &s32Percentu32Height, &s32Percentu32Width);
                stSpotMetryParam.stPoint.s32X = s32Percents32X;
                stSpotMetryParam.stPoint.s32Y = s32Percents32Y;
                stSpotMetryParam.stSize.u32Height = s32Percentu32Height;
                stSpotMetryParam.stSize.u32Width = s32Percentu32Width;

                memcpy((payload + sizeof(HI_PDT_SCENE_METRY_TYPE_E)),
                    &stSpotMetryParam, sizeof(HI_PDT_SCENE_METRY_SPOT_PARAM_S));

                printf(GREEN"\nsend HI_EVENT_STATEMNG_SETTING\n"NONE);
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage factory reset message");
                break;
            }
            case 17:
            {
#ifdef CONFIG_WIFI_ON
                HI_S32 i = 0;
                HI_HAL_WIFI_APMODE_CFG_S stApCfg;
                HI_HAL_WIFI_CFG_S stCfg;
                printf("item[%d]: stop wifi\n",i++);
                printf("item[%d]: start wifi\n",i++);
                printf("\n");

                HI_S32 cmd = 0;
                printf("please select cmd type value:");
                scanf("%d", &cmd);
                memset(&stApCfg,'\0',sizeof(HI_HAL_WIFI_APMODE_CFG_S));
                HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP,&stApCfg);
                stCfg.enMode = HI_HAL_WIFI_MODE_AP;
                memcpy(&stCfg.unCfg.stApCfg,&stApCfg,sizeof(HI_HAL_WIFI_APMODE_CFG_S));
                if(0 == cmd)
                {
                    HI_HAL_WIFI_Stop();
                    printf("stop wifi ok\n");
                }
                else if(1 == cmd)
                {
                    HI_HAL_WIFI_Start(&stCfg);
                    printf("start wifi ok\n");
                }
                else
                {
                    printf("error value:%d\n",cmd);
                }
#else
                printf("not support wifi\n");
#endif
                break;
            }
            case 18:
            {
#ifdef CONFIG_RAWCAP_ON
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_DEBUG_SWITCH;
                stMessage.arg2 = enCurrWorkMode;

                HI_S32 s32SelectItem = 0;
                printf("please select switch state for Debug(0 --- off, 1 --- on):");
                scanf("%d", &s32SelectItem);
                char* payload = (stMessage.aszPayload);
                *(HI_S32 *)payload = s32SelectItem;

                printf(GREEN"\nsend HI_EVENT_STATEMNG_DEBUG_SWITCH\n"NONE);
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET(s32Ret, "SendMessage debug switch message");
#else
                printf("not support Debug\n");
#endif
                break;
            }
            default:
            {
                printf(RED"err input\n"NONE);
                continue;
            }
        }

        if(0 == CMD || CMD == 14 || CMD == 15)
        {
            break;
        }
    }
}
#endif

static HI_VOID PDT_SetDebugLogLevel(HI_VOID)
{
    HI_LOG_Config(HI_TRUE, HI_TRUE, HI_LOG_LEVEL_INFO);
    HI_MAPI_LOG_SetEnabledLevel(HI_MAPI_LOG_LEVEL_WARN);
}

static HI_VOID PDT_Init(HI_PDT_WORKMODE_E *penPoweronWorkmode,
    HI_STORAGEMNG_CFG_S* pstStorageMngCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** init debug related setting */
    PDT_SetDebugLogLevel();
    PDT_SetCrashHandleType();

    /** init semaphore */
    sem_init(&s_PowerOffSem, 0, 0);

    /** init Param */
    s32Ret = HI_PDT_PARAM_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_PARAM_Init");

    /** init ShareFs */
    extern void sharefs_server_init(void);
    sharefs_server_init();

    /** init custom msg client */
    s32Ret = HI_MSG_CLIENT_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_MSG_CLIENT_Init");

    /** register hal sreen ops */
#if defined(CONFIG_SCREEN)
    // extern HI_HAL_SCREEN_OBJ_S stHALSCREENObj;
    // s32Ret = HI_HAL_SCREEN_Register(HI_HAL_SCREEN_IDX_0, &stHALSCREENObj);
    // PDT_APP_CHECK_RET(s32Ret, "HI_HAL_SCREEN_Register");

    // s32Ret = HI_HAL_SCREEN_Init(HI_HAL_SCREEN_IDX_0);
    // PDT_APP_CHECK_RET(s32Ret, "HI_HAL_SCREEN_Init");
#endif

    /** create load driver thread */
    s32Ret = PDT_LoadDriverThread();
    PDT_APP_CHECK_RET(s32Ret, "PDT_LoadDriverThread");

    /** get rtc time from liteOS */

    /** init eventhub */
    s32Ret = HI_EVTHUB_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_EVTHUB_Init");
    HI_STORAGEMNG_RegisterEvent();
    HI_RECMNG_RegisterEvent();
    HI_PHOTOMNG_RegisterEvent();
    HI_FILEMNG_RegisterEvent();
    HI_PDT_USBCTRL_RegisterEvent();
    HI_PDT_STATEMNG_RegisterEvent();
    HI_KEYMNG_RegisterEvent();
    HI_PDT_MEDIA_RegisterEvent();
    HI_PDT_PARAM_RegisterEvent();
    HI_LIVESVR_RegisterEvent();
    HI_PDT_SCENE_RegisterEvent();
    HI_UPGRADE_RegisterEvent();
    HI_PDT_NETCTRL_RegisterEvent();
#ifdef CONFIG_GAUGE_ON
    // HI_GAUGEMNG_RegisterEvent();
#endif

    /** init mapi sys */
    s32Ret = HI_MAPI_Sys_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_MAPI_Sys_Init");

    HI_PDT_WORKMODE_E enPoweronWorkmode = HI_PDT_WORKMODE_BUTT;
    HI_PDT_POWERON_ACTION_E enPoweronAction = HI_PDT_POWERON_ACTION_BUTT;
    s32Ret = HI_PDT_STATEMNG_GeneratePoweronWorkmode(&enPoweronAction, &enPoweronWorkmode);
    PDT_APP_CHECK_RET(s32Ret,"GetPowerOnWorkMode");
    MLOGD("PowerOn WorkMode(%d)\n", enPoweronWorkmode);
    *penPoweronWorkmode = enPoweronWorkmode;

    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enPoweronWorkmode, HI_PDT_PARAM_TYPE_MEDIAMODE,
        (HI_VOID *)&enMediaMode);
    PDT_APP_CHECK_RET(s32Ret,"GetWorkModeParam MediaMode");
    MLOGD("Init MediaMode[%d]\n", enMediaMode);

    HI_PDT_MEDIA_CFG_S stMediaCfg;
    HI_PDT_SCENE_MODE_S stSceneModeCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enPoweronWorkmode, enMediaMode, &stMediaCfg, &stSceneModeCfg);
    PDT_APP_CHECK_RET(s32Ret,"GetMediaModeCfg");

    HI_BOOL bLoadDsp = HI_FALSE;

#ifdef CFG_POST_PROCESS
    if (HI_PDT_WORKMODE_SING_PHOTO == enPoweronWorkmode
        || HI_PDT_WORKMODE_DLAY_PHOTO == enPoweronWorkmode)
    {
        bLoadDsp = HI_TRUE;
    }
#endif

    /** init media */
    s32Ret = HI_PDT_MEDIA_Init(&stMediaCfg.stViVpssMode, &stMediaCfg.stVBCfg, bLoadDsp);
    PDT_APP_CHECK_RET(s32Ret,"HI_MEDIA_Init");

#ifdef CONFIG_MOTIONSENSOR
    s32Ret = HI_PDT_MEDIA_InitMotionsensor();
    PDT_APP_CHECK_RET(s32Ret, "init motionsensor");
#endif

    // /** update vo config */
    // s32Ret = HI_PDT_Media_UpdateDispCfg(&stMediaCfg, &stMediaCfg.stVideoOutCfg.astDispCfg[0]);
    // PDT_APP_CHECK_RET(s32Ret, "HI_PDT_Media_UpdateDispCfg");

    /** init video out */
    // s32Ret = HI_PDT_MEDIA_VideoOutInit(&(stMediaCfg.stVideoOutCfg));
    // PDT_APP_CHECK_RET(s32Ret,"HI_MEDIA_VideoOutInit");
    // MLOGD(GREEN"vo init done\n"NONE);

    /** init timedtask */
    s32Ret = HI_TIMEDTASK_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_TIMEDTASK_Init");

    /** init ui */
    // s32Ret = HI_PDT_UI_Init();
    // PDT_APP_CHECK_RET(s32Ret, "HI_PDT_UI_Init");
    // MLOGD(GREEN"UI init done\n"NONE);

    if(HI_SYSTEM_WaitSharefsReady(PDT_APP_WAITSHAREFS_TIMEOUT))
    {
        /** init player */
        s32Ret = HI_PLAYER_Init();
        PDT_APP_CHECK_RET(s32Ret, "HI_PLAYER_Init");
    }

    /** load mmc driver */
    PDT_LoadMmcDriver();

    /** init storagemng */
    memset(pstStorageMngCfg, 0, sizeof(HI_STORAGEMNG_CFG_S));
    s32Ret = HI_PDT_PARAM_GetStorageCfg(pstStorageMngCfg);
    PDT_APP_CHECK_RET(s32Ret, "GetStorageCfg");
    MLOGD("DevPath[%s] MountPath[%s]\n"NONE,
        pstStorageMngCfg->szDevPath, pstStorageMngCfg->szMntPath);

    HI_STORAGEMNG_CALLBACK_S stCallback;
    stCallback.pfnFormatPreProc = PDT_StoragemngFormatPreProc;
    s32Ret = HI_STORAGEMNG_Create(pstStorageMngCfg,&stCallback);
    PDT_APP_CHECK_RET(s32Ret, "HI_STORAGEMNG_Create");

    /* init osd */
    HI_PDT_MEDIA_OSD_VIDEO_ATTR_S stOsdVideoAttr;
    HI_PDT_PARAM_GetOSDVideoAttr(&stOsdVideoAttr);
    s32Ret = HI_PDT_MEDIA_InitOSD(&stOsdVideoAttr, &stMediaCfg.stVideoCfg.stOsdCfg);
    PDT_APP_CHECK_RET(s32Ret, "InitOSD");

    /** init netCtrl */
    s32Ret = HI_PDT_NETCTRL_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_NETCTRL_Init");

#ifdef CONFIG_GAUGE_ON
    HI_GAUGEMNG_CFG_S stGaugeCfg = {};
    stGaugeCfg.s32LowLevel = PDT_BATTERT_LOW_LEVEL;
    stGaugeCfg.s32UltraLowLevel = PDT_BATTERT_ULTRA_LOW_LEVEL;
    s32Ret = HI_GAUGEMNG_Init(&stGaugeCfg);
    PDT_APP_CHECK_RET(s32Ret, "HI_GAUGEMNG_Init");
#endif

#ifdef CONFIG_RAWCAP_ON
    /** init rawcap */
    s32Ret = HI_RAWCAP_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_RAWCAP_Init");
#endif

    /** init statemng */
    HI_PDT_STATEMNG_CONFIG_S stStatemngCfg;
    stStatemngCfg.pfnExitCB = PDT_ExitModeCallback;
    stStatemngCfg.pfnFormatPreProc = PDT_StoragemngFormatPreProc;

    s32Ret = HI_PDT_STATEMNG_Init(&stStatemngCfg);
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_STATEMNG_Init");

    /** create delay services start thread */
    s32Ret = PDT_ServiceDelayedStartThread();
    PDT_APP_CHECK_RET(s32Ret, "PDT_ServiceDelayedStartThread");

}

static HI_VOID PDT_Deinit(HI_STORAGEMNG_CFG_S* pstStorageMngCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

#if PDT_APP_FAST_EXIT

#ifdef CONFIG_SCREEN
    /** close screen */
    s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
    PDT_APP_CHECK_RET(s32Ret,"set hal screen back light state off");

    s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
    PDT_APP_CHECK_RET(s32Ret,"set hal screen display state off");
#endif

#ifdef CONFIG_MOTIONSENSOR
    s32Ret = HI_PDT_MEDIA_DeinitMotionsensor();
    PDT_APP_CHECK_RET(s32Ret,"deinit motionsensor");
#endif

    /** deinit ui */
    s32Ret = HI_PDT_UI_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_UI_Deinit");

    /** deinit storagemng */
    s32Ret = HI_STORAGEMNG_Destroy(pstStorageMngCfg->szMntPath);
    PDT_APP_CHECK_RET(s32Ret, "HI_STORAGEMNG_Destroy");

    /* deinit Param */
    s32Ret = HI_PDT_PARAM_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_PARAM_Deinit");

    sem_destroy(&s_PowerOffSem);

    switch(s_ExitMode)
    {
        case HI_EXIT_MODE_POWEROFF:
        {
            printf(GREEN"###### POWEROFF #####\n\n"NONE);
            HI_SYSTEM_Poweroff();
            break;
        }
        case HI_EXIT_MODE_RESET:
        {
            printf(GREEN"###### FACTORY RESET #####\n\n"NONE);
            HI_SYSTEM_Reboot();
            break;
        }
        default:
            printf(RED"s_ExitMode error(%d)\n\n"NONE, s_ExitMode) ;
            HI_SYSTEM_Reboot();
            break;
    }
#else
#ifdef CONFIG_RAWCAP_ON
    /** deinit rawcap */
    s32Ret = HI_RAWCAP_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_RAWCAP_Deinit");
#endif

#ifdef CONFIG_SCREEN
    /** close screen */
    s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
    PDT_APP_CHECK_RET(s32Ret,"set hal screen back light state off");

    s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
    PDT_APP_CHECK_RET(s32Ret,"set hal screen display state off");
#endif

    /** deinit keymng */
    s32Ret = HI_KEYMNG_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_KEYMNG_Deinit");

    /** deinit netCtrl */
    s32Ret = HI_PDT_NETCTRL_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_NETCTRL_Deinit");

    /** deinit ui */
    s32Ret = HI_PDT_UI_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_UI_Deinit");

    /** deinit touchpad */
#ifdef CFG_LCD_TOUCHPAD_ON
    s32Ret = HI_HAL_TOUCHPAD_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_HAL_TOUCHPAD_Deinit");
#endif

    /** deinit statemng */
    s32Ret = HI_PDT_STATEMNG_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_STATEMNG_Deinit");

    /** deinit filemng */
    s32Ret = HI_FILEMNG_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_FILEMNG_Deinit");

    /** deinit timedtask */
    s32Ret = HI_TIMEDTASK_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_TIMEDTASK_Deinit");

    /** deinit storagemng */
    s32Ret = HI_STORAGEMNG_Destroy(pstStorageMngCfg->szMntPath);
    PDT_APP_CHECK_RET(s32Ret, "HI_STORAGEMNG_Destroy");

    /** deinit vo */
    //HI_PDT_MEDIA_VideoOutDeinit();

    /** deinit media */
    s32Ret = HI_PDT_MEDIA_Deinit();
    PDT_APP_CHECK_RET(s32Ret,"HI_PDT_MEDIA_Deinit");

    /** deinit media message */
    s32Ret = HI_MAPI_Sys_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_MAPI_Sys_DeInit");

    /** deinit eventhub */
    s32Ret = HI_EVTHUB_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_EVTHUB_Deinit");

    /** deinit custom msg client */
    s32Ret = HI_MSG_CLIENT_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_MSG_CLIENT_Deinit");

    /** deinit shareFs */
    extern void sharefs_server_deinit(void);
    sharefs_server_deinit();

    /* deinit Upgrade */
    s32Ret = HI_UPGRADE_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_PARAM_Deinit");

    /* deinit Param */
    s32Ret = HI_PDT_PARAM_Deinit();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_PARAM_Deinit");

    sem_destroy(&s_PowerOffSem);

    switch(s_ExitMode)
    {
        case HI_EXIT_MODE_POWEROFF:
        {
            printf(GREEN"###### POWEROFF #####\n\n"NONE);
            HI_SYSTEM_Poweroff();
            break;
        }
        case HI_EXIT_MODE_RESET:
        {
            printf(GREEN"###### FACTORY RESET #####\n\n"NONE);
            HI_SYSTEM_Reboot();
            break;
        }
        default:
            printf(RED"s_ExitMode error(%d)\n\n"NONE, s_ExitMode) ;
            HI_SYSTEM_Reboot();
            break;
    }
#endif
}

HI_S32 main(HI_S32 s32Argc, HI_CHAR* pszArgv[])
{
    HI_TIME_STAMP;

    HI_PDT_WORKMODE_E enPoweronWorkmode = HI_PDT_WORKMODE_BUTT;
    HI_STORAGEMNG_CFG_S stStorageMngCfg;
    PDT_Init(&enPoweronWorkmode, &stStorageMngCfg);

#if PDT_APP_COMMAND_DEBUG
    printf(GREEN"\nenable command line debug tool\n\n"NONE);
    PDT_CommandDebug(enPoweronWorkmode, &stStorageMngCfg);
#endif

    /** wait for poweroff Semaphore, it can not run to here when
     ** PDT_APP_COMMAND_DEBUG is true and the while loop is not over */
    while((HI_SUCCESS != sem_wait(&s_PowerOffSem)) && (errno == EINTR));

    PDT_Deinit(&stStorageMngCfg);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
