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
#include <errno.h>
#ifndef __UCLIBC__
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
#ifdef CONFIG_ACC_ON
#include "hi_accmng.h"
#endif
#include "hi_gpsmng.h"
#include "hi_gaugemng.h"
#include "hi_keymng.h"
#include "hi_storagemng.h"
#include "hi_gsensormng.h"
#include "hi_ahdmng.h"
#include "hi_filemng_dtcf.h"
#include "hisnet.h"
#include "hi_filetrans.h"
#include "hi_appcomm.h"
#include "hi_appcomm_util.h"
#include "hi_timestamp.h"
#include "hi_appcomm_msg_client.h"
#include "hi_timedtask.h"
#ifdef CONFIG_RAWCAP_ON
#include "hi_rawcapture.h"
#endif
#include "hi_system.h"
#include "hi_player.h"
#include "hi_playback.h"
#include "hi_upgrade.h"
#include "hi_liveserver.h"

/** pdt */
#include "hi_product_param.h"
#include "hi_product_statemng.h"
#include "hi_product_ui.h"
#include "hi_product_usbctrl.h"
#include "hi_product_media.h"
#include "hi_product_netctrl.h"
#include "hi_product_scene.h"

#ifdef CONFIG_GAUGE_ON
#include "hi_gaugemng.h"
#endif

#ifdef CONFIG_WATCHDOG_ON
#include "hi_hal_watchdog.h"
#include "hi_watchdogmng.h"
#endif


#ifdef CONFIG_MOTIONDETECT_ON
#include "hi_videodetect.h"
#endif
#include "hi_mapi_hal_ahd.h"

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
#include "hi_tempcycle.h"
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

/** function ret value check */
#define PDT_APP_CHECK_RET(retValue,errstring)\
do{\
    if(HI_SUCCESS != retValue)\
    {\
        MLOGE(RED"%s failed, s32Ret(%d)\n\n"NONE, errstring, retValue);\
        return HI_FAILURE;\
    }\
    else\
    {\
        MLOGD("%s success, s32Ret(%d)\n\n", errstring, retValue);\
        HI_TIME_STAMP;\
    }\
  }while(0)

/** function ret value check */
#define PDT_APP_CHECK_RET_WITH_UNRETURN(retValue,errstring)\
do{\
    if(HI_SUCCESS != retValue)\
    {\
        MLOGE(RED"%s failed, s32Ret(%d)\n\n"NONE, errstring, retValue);\
    }\
    else\
    {\
        MLOGD("%s success, s32Ret(%d)\n\n", errstring, retValue);\
        HI_TIME_STAMP;\
    }\
  }while(0)

static pthread_t s_KoThread;
static pthread_t s_SrvPostThread;
static sem_t s_PowerOffSem;/** power off semaphore */
static HI_EXIT_MODE_E s_ExitMode = HI_EXIT_MODE_BUTT;/** exit mode */

#define PDT_MAIN_WAITSHAREFS_TIMEOUT       (4)

#ifndef CONFIG_COREDUMP_ON

HI_VOID PDT_INIT_MediaDeinit()
{
    HI_PDT_MEDIA_VideoOutStop();
    HI_PDT_MEDIA_VideoDeinit(HI_TRUE);
    HI_PDT_MEDIA_VideoOutDeinit();
    HI_PDT_MEDIA_AudioDeinit();
    HI_PDT_MEDIA_AudioOutStop();
    HI_PDT_MEDIA_Deinit();
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
#ifndef __UCLIBC__
    HI_VOID *pvBuffer[64];
    HI_CHAR **ppStrings;
    HI_S32 s32Nptrs = 0;
    /** need gcc option "-g -rdynamic" */
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
#endif
}

/** load driver task */
static HI_U32 PDT_LoadDriverThread(void* pVoid)
{
    pthread_detach(pthread_self());

    /** Let insmod touch driver before insmod sdcard driver.
     ** before settle boot service, need to wait for semaphore hundreds ms */
#ifdef CFG_LCD_TOUCHPAD_ON
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_HAL_TOUCHPAD_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_HAL_TOUCHPAD_Init");

    s32Ret = HI_HAL_TOUCHPAD_Suspend();
    PDT_APP_CHECK_RET(s32Ret, "HI_HAL_TOUCHPAD_Suspend");
#endif
    return HI_SUCCESS;
}

static HI_VOID PDT_LoadMmcDriver(HI_VOID)
{
#if !defined(CFG_EMMC_FLASH)
    HI_insmod(HI_APPFS_KOMOD_PATH"/mmc_core.ko", NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/mmc_block.ko", NULL);

#if defined(HI3556AV100) || defined(HI3519AV100)
    HI_insmod(HI_APPFS_KOMOD_PATH"/sdhci.ko",NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/sdhci-pltfm.ko",NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/sdhci-of-hisi.ko",NULL);
#elif defined(HI3559V200)
    HI_insmod(HI_APPFS_KOMOD_PATH"/hisi_mci.ko",NULL);
#endif
#endif
}

static HI_VOID PDT_LoadUSBPhy(HI_VOID)
{
    HI_insmod(HI_APPFS_KOMOD_PATH"/dwc3.ko",NULL);
}

/** create thread to load driver */
static HI_S32 PDT_LoadDriver()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = pthread_create(&s_KoThread, NULL, (void*)PDT_LoadDriverThread, NULL);
    PDT_APP_CHECK_RET(s32Ret, "pthread_create for PDT_LoadDriver");
    return s32Ret;
}

/** delayed services start task */
static HI_S32 PDT_ServiceDelayedStart()
{
    pthread_detach(pthread_self());
    HI_S32 s32Ret = HI_SUCCESS;

#ifdef CONFIG_ACC_ON
    s32Ret = HI_ACCMNG_Init();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_ACCMNG_Init");
#endif

#ifdef CONFIG_GPS_ON
    s32Ret = HI_GPSMNG_Init();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_GPSMNG_Init");
    s32Ret = HI_GPSMNG_Start();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_GPSMNG_Start");
#endif

    HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_STARTUP;
    HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
    if(HI_SYSTEM_STARTUP_SRC_WAKEYP != enStartupSrc)
    {
        /** keymng init */
        HI_KEYMNG_CFG_S stKeyCfg;
        s32Ret = HI_PDT_PARAM_GetKeyMngCfg(&stKeyCfg);
        PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_PARAM_GetKeyMngCfg");
        s32Ret = HI_KEYMNG_Init(&stKeyCfg);
        PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_KEYMNG_Init");
        HI_PrintBootTime("Linux key available");

#ifdef CFG_LCD_TOUCHPAD_ON
        /** active touchpad */
        s32Ret = HI_HAL_TOUCHPAD_Resume();
        PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_HAL_TOUCHPAD_Resume");
#endif
    }

#ifdef CONFIG_GAUGE_ON
    HI_GAUGEMNG_CFG_S stGaugeCfg;

    s32Ret = HI_PDT_PARAM_GetGaugeMngCfg(&stGaugeCfg);
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_PARAM_GetGaugeMngCfg");
    s32Ret = HI_GAUGEMNG_Init(&stGaugeCfg);
    PDT_APP_CHECK_RET(s32Ret, "HI_GAUGEMNG_Init");
#endif

    HI_GSENSORMNG_CFG_S stCfg = {0};
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY,(HI_S32*)&stCfg.enSensitity);
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"GetGsensorCfg");
    s32Ret = HI_GSENSORMNG_Init(&stCfg);
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_GSENSORMNG_Init");

    //hi_usleep(50 * 1000);
    //PDT_LoadMmcDriver();
#ifdef CONFIG_WIFI_ON
    /** init wifi */
    HI_PDT_WIFI_INFO_S stInfo = {0};
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI,(HI_PDT_WIFI_INFO_S*)&stInfo);
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"Get wifi info");
    if(HI_TRUE == stInfo.bEnable)
    {
        HI_HAL_WIFI_APMODE_CFG_S stApCfg = {};
        HI_HAL_WIFI_CFG_S stCfg = {};

        stCfg.enMode = HI_HAL_WIFI_MODE_AP;
        s32Ret = HI_HAL_WIFI_Init(stCfg.enMode);
        PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_HAL_WIFI_Init");

        /* get ap param must init wifi first */
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stApCfg);
        PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"Get wifi ap cfg");

        memcpy(&stCfg.unCfg.stApCfg, &stApCfg, sizeof(HI_HAL_WIFI_APMODE_CFG_S));
        s32Ret = HI_HAL_WIFI_Start(&stCfg);
        PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"HI_HAL_WIFI_Start");
        MLOGD("HI_HAL_WIFI_Start\n");

    }
#endif

    /** init netCtrl */
    s32Ret = HI_PDT_NETCTRL_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_NETCTRL_Init");

#ifdef CONFIG_RAWCAP_ON
    /** init rawcap */
    s32Ret = HI_RAWCAP_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_RAWCAP_Init");
#endif

    signal(SIGPIPE, SIG_IGN);

    sleep(10);
    HI_TIME_PRINT

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
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_FILETRANS_CloseAllLink();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"HI_FILETRANS_CloseAllLink");
    s32Ret = HISNET_CleanMmapCache();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"HISNET_CleanMmapCache");
    return HI_SUCCESS;
}

static HI_S32 PDT_ExitModeCallback(HI_EXIT_MODE_E enExitMode)
{
    s_ExitMode = enExitMode;/** exit mode */
    sem_post(&s_PowerOffSem);
    return HI_SUCCESS;
}

#define PDT_APP_COMMAND_DEBUG 0

#if PDT_APP_COMMAND_DEBUG
static HI_VOID PDT_CommandDebugOption(HI_VOID)
{
    printf("\n");
    printf(GREEN"0  --- quit \n"
                "1  --- switch to norm_rec \n"
                "2  --- start \n"
                "3  --- stop \n"
                "4  --- trigger \n"
                "5  --- get state \n"
                "6  --- get storage state \n"
                "7  --- set media mode\n"
                "8  --- switch workmode \n"
                "9 --- setting atrribute \n"
                "10 --- choose Charge mode \n"
                "11 --- choose UVC mode \n"
                "12 --- choose USB Storage mode \n"
                "13 --- poweroff \n"
                "14 --- factory reset \n"
                "15 --- EMR record \n"
                "16 --- VO switch \n"
                "17 --- set wifi \n"
                "18 --- set flip \n"
                "19 --- debug switch \n"NONE);
}

static HI_S32 PDT_CommandDebug(HI_PDT_WORKMODE_E enPoweronWorkmode,
    HI_STORAGEMNG_CFG_S* pstStorageMngCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 CMD = 0;
    HI_PDT_WORKMODE_E enCurrWorkMode = enPoweronWorkmode;

    while(1)
    {
        PDT_CommandDebugOption();
        scanf("%d", &CMD);

        if(7 != CMD)
        {
            HI_PDT_STATEMNG_WORKMODE_STATE_S stCrrentWorkModeState;
            s32Ret = HI_PDT_STATEMNG_GetState(&stCrrentWorkModeState);
            PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_STATEMNG_GetState");
            enCurrWorkMode = stCrrentWorkModeState.enWorkMode;
        }

        switch(CMD)
        {
            case 0:
                MLOGE(RED" input 0 \n\n"NONE);
                break;
            case 1:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;

                printf(GREEN"\nsend switch workmode to HI_PDT_WORKMODE_NORM_REC\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "SendMessage switch workmode to normal record");
                enCurrWorkMode = HI_PDT_WORKMODE_NORM_REC;
                sleep(2);
                break;
            }
            case 2:
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
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "SendMessage start");
                sleep(7);
                break;
            }
            case 3:
            {
                HI_MESSAGE_S stMessage;
                stMessage.what = HI_EVENT_STATEMNG_STOP;
                stMessage.arg1 = HI_TRUE;;
                stMessage.arg2 = enCurrWorkMode;

                printf(GREEN"\nsend message HI_EVENT_STATEMNG_STOP\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "SendMessage stop");
                sleep(2);
                break;
            }
            case 4:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_TRIGGER;
                stMessage.arg2 = enCurrWorkMode;

                printf(GREEN"\nsend message HI_EVENT_STATEMNG_TRIGGER\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "SendMessage trigger");
                sleep(5);
                break;
            }
            case 5:
            {
                HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
                memset(&stWorkModeState, 0, sizeof(HI_PDT_STATEMNG_WORKMODE_STATE_S));

                printf(GREEN"\nget state HI_PDT_STATEMNG_GetState\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_STATEMNG_GetState");

                printf("current workmode(%d), bRunning(%d), bStateMngInProgress(%d)\n\n",
                       stWorkModeState.enWorkMode, stWorkModeState.bRunning,
                       stWorkModeState.bStateMngInProgress);
                HI_TIME_PRINT;
                HI_TIME_CLEAN;
                sleep(2);
                break;
            }
            case 6:
            {
                HI_STORAGE_STATE_E enGetState;
                s32Ret = HI_STORAGEMNG_GetState(pstStorageMngCfg->szMntPath, &enGetState);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"get storage state");
                printf(GREEN"\nSTATEMNG current storage state(%d)\n"NONE, enGetState);
                sleep(2);
                break;
            }
            case 7:
            {
                extern HI_S32 PDT_STATEMNG_GetPreViewCamID(HI_VOID);
                HI_S32 s32PreviewCamID;
                s32PreviewCamID =PDT_STATEMNG_GetPreViewCamID();
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SETTING;
                stMessage.arg1 = HI_PDT_PARAM_TYPE_MEDIAMODE;
                stMessage.arg2 = enCurrWorkMode;

                /** get surpport list*/
                HI_PDT_ITEM_VALUESET_S stValueSet;
                memset(&stValueSet, 0, sizeof(HI_PDT_ITEM_VALUESET_S));
                /*TODO*/
                s32Ret = HI_PDT_PARAM_GetCamItemValues(enCurrWorkMode,
                                                       s32PreviewCamID,
                                                       HI_PDT_PARAM_TYPE_MEDIAMODE,
                                                       &stValueSet);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"PARAM_GetWorkModeItemValues");

                printf("enCurrWorkMode(%d), attribute type(%d), support list has [%d] item:\n",
                       enCurrWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE, stValueSet.s32Cnt);
                HI_S32 i = 0;
                for(i = 0; i < stValueSet.s32Cnt; i++)
                {
                    printf("item[%d]: value %d\n", i, stValueSet.astValues[i].s32Value);
                }
                printf("\n");

                HI_S32 s32SelectItem = 0;
                printf("please select an item value:");
                scanf("%d", &s32SelectItem);

                //*((HI_S32*)stMessage.aszPayload) = s32SelectItem;
                memcpy(stMessage.aszPayload,&s32SelectItem,sizeof(HI_S32));

                printf(GREEN"\nsend message HI_EVENT_STATEMNG_SETTING, "
                            "setting type:HI_PDT_PARAM_TYPE_MEDIAMODE, workmode:%d, value:%d \n"NONE,
                            enCurrWorkMode, s32SelectItem);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_STATEMNG_SendMessage HI_EVENT_STATEMNG_SETTING");
                sleep(5);
                break;
            }
            case 8:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                printf("the support workmode type in the system:\n");
                HI_S32 i = 0;
                printf("item[%d]: HI_PDT_WORKMODE_NORM_REC, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_NORM_REC);
                printf("item[%d]: HI_PDT_WORKMODE_PLAYBACK, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_PLAYBACK);
                printf("item[%d]: HI_PDT_WORKMODE_UVC, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_UVC);
                printf("item[%d]: HI_PDT_WORKMODE_USB_STORAGE, workmode type value %d\n",
                    i++, HI_PDT_WORKMODE_USB_STORAGE);

                HI_S32 s32SelectItem = 0;
                printf("please select a workmode type value:");
                scanf("%d", &s32SelectItem);
                stMessage.arg2 = s32SelectItem;

                printf(GREEN"\nsend switch workmode to workmode(%d)\n"NONE, s32SelectItem);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_STATEMNG_SendMessage switch workmode");
                enCurrWorkMode = s32SelectItem;
                sleep(2);
                break;
            }
            case 9:
            {
                extern HI_S32 PDT_STATEMNG_GetPreViewCamID(HI_VOID);
                HI_S32 s32PreviewCamID;
                s32PreviewCamID =PDT_STATEMNG_GetPreViewCamID();

                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SETTING;

                printf("the support setting type in the system:\n");
                HI_S32 i = 0;
                printf("item[%d]: HI_PDT_PARAM_TYPE_MEDIAMODE, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_MEDIAMODE);
                printf("item[%d]: HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE);
                printf("item[%d]: HI_PDT_PARAM_TYPE_FLIP, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_FLIP);
                printf("item[%d]: HI_PDT_PARAM_TYPE_MIRROR, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_MIRROR);
                printf("item[%d]: HI_PDT_PARAM_TYPE_LDC, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_LDC);
                printf("item[%d]: HI_PDT_PARAM_TYPE_REC, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_REC);
                printf("item[%d]: HI_PDT_PARAM_TYPE_OSD, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_OSD);
                printf("item[%d]: HI_PDT_PARAM_TYPE_AUDIO, setting type value %d\n",
                    i++, HI_PDT_PARAM_TYPE_AUDIO);

                printf("\n");

                HI_S32 s32SelectItem = 0;
                printf("please select a setting type value:");
                scanf("%d", &s32SelectItem);

                stMessage.arg1 = s32SelectItem;
                stMessage.arg2 = enCurrWorkMode;

                if( HI_PDT_PARAM_TYPE_FLIP != s32SelectItem &&
                    HI_PDT_PARAM_TYPE_MIRROR != s32SelectItem &&
                    HI_PDT_PARAM_TYPE_LDC != s32SelectItem &&
                    HI_PDT_PARAM_TYPE_REC != s32SelectItem &&
                    HI_PDT_PARAM_TYPE_OSD != s32SelectItem )
                {
                    /** get surpport list*/
                    HI_PDT_ITEM_VALUESET_S stValueSet;
                    memset(&stValueSet, 0, sizeof(HI_PDT_ITEM_VALUESET_S));
                    s32Ret = HI_PDT_PARAM_GetCamItemValues(enCurrWorkMode,
                                                           s32PreviewCamID,
                                                           s32SelectItem,
                                                           &stValueSet);
                    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"PARAM_GetWorkModeItemValues");
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

                //*((HI_S32*)stMessage.aszPayload) = s32SelectItem;
                memcpy(stMessage.aszPayload,&s32SelectItem,sizeof(HI_S32));

                printf(GREEN"\nsend message HI_EVENT_STATEMNG_SETTING, setting type:%d, "
                            "workmode:%d, value:%d \n"NONE,
                            stMessage.arg1, enCurrWorkMode, s32SelectItem);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_STATEMNG_SendMessage HI_EVENT_STATEMNG_TRIGGER");
                sleep(5);
                break;
            }
            case 10:
            {
                #if 0
                printf("set to charge mode:\n");
                HI_USB_SetMode(HI_USB_MODE_CHARGE);
                #endif
                break;
            }
            case 11:
            {
                #if 0
                printf("set to UVC mode:\n");
                HI_USB_SetMode(HI_USB_MODE_UVC);
                #endif
                break;
            }
            case 12:
            {
                #if 0
                printf("set to USB Storage mode:\n");
                HI_USB_SetMode(HI_USB_MODE_STORAGE);
                #endif
                break;
            }
            case 13:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_POWEROFF;

                printf(GREEN"\nsend HI_EVENT_STATEMNG_POWEROFF\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "SendMessage poweroff message");
                break;
            }
            case 14:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_FACTORY_RESET;

                printf(GREEN"\nsend HI_EVENT_STATEMNG_FACTORY_RESET\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "SendMessage factory reset message");
                break;
            }
            case 15:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_EMR_BEGIN;
                stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;

                printf(GREEN"\nsend EMR_Begin to HI_PDT_WORKMODE_NORM_REC\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "SendMessage switch workmode to normal record");
                enCurrWorkMode = HI_PDT_WORKMODE_NORM_REC;
                break;
            }
            case 16:
            {
                extern HI_S32 PDT_STATEMNG_GetPreViewCamID(HI_VOID);
                HI_S32 s32PreviewCamID;
                s32PreviewCamID =PDT_STATEMNG_GetPreViewCamID();
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_VO_SWITCH;
                stMessage.arg2 = enCurrWorkMode;

                s32PreviewCamID = (s32PreviewCamID + 1)%HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;
                memcpy(stMessage.aszPayload,&s32PreviewCamID,sizeof(HI_S32));

                printf(GREEN"\nsend message HI_EVENT_STATEMNG_VO_SWITCH\n"NONE);
                HI_TIME_CLEAN;
                HI_TIME_STAMP;
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "SendMessage vo switch");
                sleep(2);
                break;
            }
            case 17:
            {
#if defined(CONFIG_WIFI_ON)
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
                    HI_HAL_WIFI_Deinit();
                    printf("stop wifi ok\n");
                }
                else if(1 == cmd)
                {
                    /** init wifi service */
                    HI_HAL_WIFI_Init(HI_HAL_WIFI_MODE_AP);
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
                HI_S32 s32CamID = 0,s32CurValue = 0,s32NewValue = 0;
                extern HI_S32 PDT_STATEMNG_GetPreViewCamID(HI_VOID);
                s32CamID =PDT_STATEMNG_GetPreViewCamID();
                HI_PDT_PARAM_GetCamParam(enCurrWorkMode, s32CamID, HI_PDT_PARAM_TYPE_FLIP, &s32CurValue);

                s32NewValue = (HI_TRUE == (HI_BOOL)s32CurValue) ? HI_FALSE : HI_TRUE;

                printf("flip test s32CamID[%d],s32CurValue[%d],s32NewValue[%d]\n",s32CamID,s32CurValue,s32NewValue);
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SETTING;
                stMessage.arg1 = HI_PDT_PARAM_TYPE_FLIP;
                stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
                memcpy(stMessage.aszPayload,&s32NewValue,sizeof(HI_S32));

                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "SendMessage vo switch");
            }
            case 19:
            {
                #ifdef CONFIG_RAWCAP_ON
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_DEBUG_SWITCH;
                stMessage.arg2 = enCurrWorkMode;

                HI_S32 s32SelectItem = 0;
                 HI_BOOL bDebug = HI_FALSE;
                printf("please select switch state for Debug(0 --- off, 1 --- on):");
                scanf("%d", &s32SelectItem);
                bDebug = (0 ==s32SelectItem) ? HI_FALSE : HI_TRUE;
                memcpy(&stMessage.aszPayload,&bDebug,sizeof(HI_BOOL));

                printf(GREEN"\nsend HI_EVENT_STATEMNG_DEBUG_SWITCH\n"NONE);
                s32Ret = HI_PDT_STATEMNG_SendMessage((const HI_MESSAGE_S*)&stMessage);
                PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "SendMessage debug switch message");
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

        if(0 == CMD || CMD == 13 || CMD == 14)
        {
            break;
        }
    }
    return HI_SUCCESS;
}

#endif

#ifdef CONFIG_AHD_ON
static HI_S32 PDT_AHDMNG_Init()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_AHDMNG_CFG_S stCfg;
#ifdef CONFIG_SNS1_BT656
    stCfg.u32Cnt = 1;
    stCfg.astCamCfg[0].CamID = 1;
    stCfg.astCamCfg[0].u32ChnNum = HI_HAL_AHD_CHN_MAX;
    s32Ret = HI_AHDMNG_Init(&stCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "AHDMNG Init");
#endif
    return s32Ret;
}
#else
static HI_S32 PDT_AHDMNG_Init()
{
    return HI_SUCCESS;
}

#endif

static HI_VOID PDT_SetDebugLogLevel(HI_VOID)
{
    HI_LOG_Config(HI_TRUE, HI_TRUE, HI_LOG_LEVEL_INFO);
    HI_MAPI_LOG_SetEnabledLevel(HI_MAPI_LOG_LEVEL_WARN);
}

static HI_S32 PDT_Init(HI_PDT_WORKMODE_E *penPoweronWorkmode,
    HI_STORAGEMNG_CFG_S* pstStorageMngCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PrintBootTime("linux start main_app");
    /** init debug related setting */
    PDT_SetDebugLogLevel();
    PDT_SetCrashHandleType();

    /** init semaphore */
    sem_init(&s_PowerOffSem, 0, 0);

#ifdef CONFIG_WATCHDOG_ON
    HI_S32 s32FeedTime_s = 10;   /**10s periodly feed dog*/
    s32Ret = HI_WATCHDOGMNG_Init(s32FeedTime_s);
    PDT_APP_CHECK_RET(s32Ret, "HI_HAL_WATCHDOG_Init");
#endif

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
    extern HI_HAL_SCREEN_OBJ_S stHALSCREENObj;
    s32Ret = HI_HAL_SCREEN_Register(HI_HAL_SCREEN_IDX_0, &stHALSCREENObj);
    PDT_APP_CHECK_RET(s32Ret, "HI_HAL_SCREEN_Register");

    s32Ret = HI_HAL_SCREEN_Init(HI_HAL_SCREEN_IDX_0);
    PDT_APP_CHECK_RET(s32Ret, "HI_HAL_SCREEN_Init");
#endif

    /* init eventhub */
    s32Ret = HI_EVTHUB_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_EVTHUB_Init");
    HI_STORAGEMNG_RegisterEvent();
    HI_RECMNG_RegisterEvent();
    HI_PHOTOMNG_RegisterEvent();
    HI_FILEMNG_RegisterEvent();
    HI_PDT_USBCTRL_RegisterEvent();
    HI_PDT_STATEMNG_RegisterEvent();
    HI_KEYMNG_RegisterEvent();
    HI_GSENSORMNG_RegisterEvent();
    HI_PDT_PARAM_RegisterEvent();
    HI_LIVESVR_RegisterEvent();
    HI_PDT_NETCTRL_RegisterEvent();
    HI_UPGRADE_RegisterEvent();
#ifdef CONFIG_GAUGE_ON
    HI_GAUGEMNG_RegisterEvent();
#endif
#ifdef CONFIG_AHD_ON
    HI_AHDMNG_RegisterEvent();
#endif
#ifdef CONFIG_MOTIONDETECT_ON
    HI_VIDEODETECT_RegisterEvent();
#endif
    /** init custom message, wait head file */
#ifdef CONFIG_TEMPRATURE_CYCLE_ON
    HI_TEMPCYCLE_RegisterEvent();
#endif

#ifdef CONFIG_ACC_ON
    HI_ACCMNG_RegisterEvent();
#endif

    /** init media message */
    s32Ret = HI_MAPI_Sys_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_MAPI_Sys_Init");

    HI_PDT_WORKMODE_E enWorkmode = HI_PDT_WORKMODE_NORM_REC;
    HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_STARTUP;
    HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
    if(HI_SYSTEM_STARTUP_SRC_WAKEYP == enStartupSrc)
    {
        enWorkmode = HI_PDT_WORKMODE_PARKING_REC;
    }
    else if(HI_SYSTEM_STARTUP_SRC_STARTUP == enStartupSrc)
    {
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enWorkmode);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE,"HI_PDT_PARAM_GetCommParam");
    }

    HI_PDT_PARAM_CAM_MEDIAMODE_CFG_S stCamCfg;
    HI_PDT_PARAM_CAM_CONTEXT_S astCamCtx[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_S32 i = 0;
    s32Ret = HI_PDT_PARAM_GetMediaModeCfg(enWorkmode,&stCamCfg);
    PDT_APP_CHECK_RET(s32Ret,"HI_PDT_PARAM_GetMediaModeCfg");
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        astCamCtx[i].stMediaModeCfg.s32CamID = stCamCfg.astMediaModeCfg[i].s32CamID;
        astCamCtx[i].stMediaModeCfg.enMediaMode = stCamCfg.astMediaModeCfg[i].enMediaMode;
        s32Ret = HI_PDT_PARAM_GetCamParam(enWorkmode, i, HI_PDT_PARAM_TYPE_CAM_STATUS,
            &astCamCtx[i].bEnable);
        //astCamCtx[i].bEnable = HI_TRUE;
    }

    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enWorkmode, astCamCtx, &stMediaCfg);
    PDT_APP_CHECK_RET(s32Ret,"HI_PDT_PARAM_GetMediaCfg");

    s32Ret = HI_PDT_MEDIA_Init(&stMediaCfg.stViVpssMode, &stMediaCfg.stVBCfg);
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_MEDIA_Init");

    /* init osd */
    s32Ret = HI_PDT_MEDIA_InitOSD(&stMediaCfg.stVideoCfg.stOsdCfg);
    PDT_APP_CHECK_RET(s32Ret, "InitOSD");

    /** init vo */
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; i++)
    {
        s32Ret = HI_PDT_Media_UpdateDispCfg(&stMediaCfg.stVideoOutCfg.astDispCfg[i],&stMediaCfg.stVideoCfg.stVprocCfg, NULL);
        PDT_APP_CHECK_RET(s32Ret, "HI_PDT_Media_UpdateDispCfg");
    }

    s32Ret = HI_PDT_MEDIA_VideoOutInit(&(stMediaCfg.stVideoOutCfg));
    PDT_APP_CHECK_RET(s32Ret,"HI_PDT_MEDIA_VideoOutInit");

    /** init timedtask */
    s32Ret = HI_TIMEDTASK_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_TIMEDTASK_Init");

    if(HI_SYSTEM_WaitSharefsReady(PDT_MAIN_WAITSHAREFS_TIMEOUT))
    {
        /** init player */
        s32Ret = HI_PLAYER_Init();
        PDT_APP_CHECK_RET(s32Ret, "HI_PLAYER_Init");
    }

    /** init ui */
    s32Ret = HI_PDT_UI_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_UI_Init");
    HI_PrintBootTime("linux start ui");
    PDT_AHDMNG_Init();

#ifndef CONFIG_RELEASE
    s32Ret = HI_PDT_SCENE_Init();
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_SCENE_Init");
#endif

    /** init statemng */
    HI_PDT_STATEMNG_CONFIG_S stStatemngCfg;
    stStatemngCfg.pfnExitMode = PDT_ExitModeCallback;
    stStatemngCfg.pfnFormatPreProc = PDT_StoragemngFormatPreProc;
    s32Ret = HI_PDT_STATEMNG_Init(&stStatemngCfg);
    PDT_APP_CHECK_RET(s32Ret, "HI_PDT_STATEMNG_Init");

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
    HI_TEMPCYCLE_Init(HI_NULL);
#endif
    /** create load driver thread */
    s32Ret = PDT_LoadDriver();
    PDT_APP_CHECK_RET(s32Ret, "PDT_LoadDriver");

    /** load mmc driver */
    PDT_LoadMmcDriver();

    /** init storagemng */
    memset(pstStorageMngCfg, 0, sizeof(HI_STORAGEMNG_CFG_S));
    s32Ret = HI_PDT_PARAM_GetStorageCfg(pstStorageMngCfg);
    PDT_APP_CHECK_RET(s32Ret, "GetStorageCfg");

    HI_STORAGEMNG_CALLBACK_S stCallback;
    stCallback.pfnFormatPreProc = PDT_StoragemngFormatPreProc;
    s32Ret = HI_STORAGEMNG_Create(pstStorageMngCfg,&stCallback);
    PDT_APP_CHECK_RET(s32Ret, "HI_STORAGEMNG_Create");

    /*init usb module*/
    if(HI_SYSTEM_STARTUP_SRC_WAKEYP != enStartupSrc)
    {
        PDT_LoadUSBPhy();

        /** init usb ctrl module*/
        s32Ret = HI_PDT_USBCTRL_Init();
        PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"init usb ctrl module");
    }

    /** create delay services start thread */
    s32Ret = PDT_ServiceDelayedStartThread();
    PDT_APP_CHECK_RET(s32Ret, "PDT_ServiceDelayedStartThread");
    return HI_SUCCESS;
}

static HI_S32 PDT_Deinit(HI_STORAGEMNG_CFG_S* pstStorageMngCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

#ifdef CONFIG_ACC_ON
    s32Ret = HI_ACCMNG_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_ACCMNG_Deinit");
#endif

#ifdef CONFIG_GPS_ON
    s32Ret = HI_GPSMNG_Stop();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_GPSMNG_Stop");
    s32Ret = HI_GPSMNG_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_GPSMNG_Deinit");
#endif

#ifdef CONFIG_RAWCAP_ON
    /** deinit rawcap */
    s32Ret = HI_RAWCAP_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_RAWCAP_Deinit");
#endif

#ifdef CONFIG_SCREEN
    /** close screen */
    s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"set hal screen back light state off");

    s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"set hal screen display state off");
#endif
    /** deinit scene */
    s32Ret = HI_PDT_SCENE_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_SCENE_Deinit");

    /** deinit keymng */
    s32Ret = HI_KEYMNG_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_KEYMNG_Deinit");

    /** deinit netCtrl */
    s32Ret = HI_PDT_NETCTRL_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_NETCTRL_Deinit");

    /** deinit ui */
    s32Ret = HI_PDT_UI_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_UI_Deinit");

    /** deinit touchpad */
#ifdef CFG_LCD_TOUCHPAD_ON
    s32Ret = HI_HAL_TOUCHPAD_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_HAL_TOUCHPAD_Deinit");
#endif

    /** deinit statemng */
    s32Ret = HI_PDT_STATEMNG_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_STATEMNG_Deinit");

    /** deinit filemng */
    s32Ret = HI_FILEMNG_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_FILEMNG_Deinit");

    /** deinit timedtask */
    s32Ret = HI_TIMEDTASK_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_TIMEDTASK_Deinit");

    /** deinit storagemng */
    s32Ret = HI_STORAGEMNG_Destroy(pstStorageMngCfg->szMntPath);
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_STORAGEMNG_Destroy");

    /** deinit vo */
    HI_PDT_MEDIA_VideoOutDeinit();

    /** deinit media */
    s32Ret = HI_PDT_MEDIA_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret,"HI_PDT_MEDIA_Deinit");

    /** deinit media message */
    s32Ret = HI_MAPI_Sys_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_MAPI_Sys_DeInit");

    /** deinit eventhub */
    s32Ret = HI_EVTHUB_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_EVTHUB_Deinit");

    /** deinit custom msg client */
    s32Ret = HI_MSG_CLIENT_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_MSG_CLIENT_Deinit");

    HI_TIME_STAMP
    HI_TIME_PRINT
    HI_TIME_CLEAN

    /** deinit shareFs */
    extern void sharefs_server_deinit(void);
    sharefs_server_deinit();

    /* deinit Param */
    s32Ret = HI_PDT_PARAM_Deinit();
    PDT_APP_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_PARAM_Deinit");

    switch(s_ExitMode)
    {
        case HI_EXIT_MODE_POWEROFF:
        {
            MLOGD(GREEN"###### POWEROFF #####\n\n"NONE);
            HI_SYSTEM_Poweroff();
            break;
        }
        case HI_EXIT_MODE_REBOOT:
        {
            MLOGD(GREEN"###### reboot #####\n\n"NONE);
            HI_SYSTEM_Reboot();
            break;
        }
        default:
            MLOGD(RED"s_ExitMode error\n\n"NONE);
            HI_SYSTEM_Reboot();
            break;
    }
    return HI_SUCCESS;
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
