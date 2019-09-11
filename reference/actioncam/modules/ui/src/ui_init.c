/**
 * @file      ui_init.c
 * @brief     ui init and deinit
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <semaphore.h>

#include "hi_gv_resm.h"
#include "hi_gv_parser.h"
#include "hi_gv.h"
#include "hi_gv_scrollgrid.h"
#include "hi_gv_input.h"
#include "hi_gv_msg.h"
#include "hi_gv_mlayer.h"
#include "hi_gv_log.h"

#include "hi_appcomm.h"
#include "hi_appcomm_util.h"
#include "ui_common.h"
#include "hi_product_ui.h"
#include "ui_powercontrol.h"
#include "hi_hal_screen.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define LANG_EN_FILE   "./en.lang"
#define LANG_CN_FILE   "./zh.lang"
#define LAN_CHINESE    "zh"
#define LAN_ENGLISH    "en"

static HI_BOOL s_bLCD = HI_TRUE;
static HIGV_HANDLE s_AppHandle;
static HIGV_HANDLE s_LCDLayerHandle = INVALID_HANDLE;
static HIGV_HANDLE s_HDMILayerHandle = INVALID_HANDLE;
static pthread_t s_UiThread;

static HI_VOID * PDT_UI_StartApp(HI_VOID* pVoid)
{
    HI_S32 s32Ret;

    s32Ret = HI_GV_App_Create("MainApp", (HIGV_HANDLE *)&s_AppHandle);
    if (HI_SUCCESS != s32Ret)
    {
        HI_GV_Deinit();
        HI_GV_PARSER_Deinit();
        return NULL;
    }
    prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);

    /*HI_GV_App_Start will not return until  HI_GV_App_Stop is called. */
    s32Ret = HI_GV_App_Start(s_AppHandle);
    HI_APPCOMM_CHECK_RETURN(s32Ret, NULL);

    return NULL;
}


/*  public section between LCD and HDMI initialization*/
static HI_S32 PDT_UI_InitHigv(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
#if defined(CFG_LCD_TOUCHPAD_ON)
    s32Ret = PDT_UI_RegisterTouchDevice();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
#endif

    s32Ret = HI_GV_SetVsyncType(HIGV_VSYNC_HW);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(s_bLCD)
    {
        s32Ret = HI_GV_SetSWVsyncPeriod(16666); /** us */
        s32Ret |= HI_GV_SetLostFrameThreshold(8000); /** us */
    }
    else
    {
        s32Ret = HI_GV_SetSWVsyncPeriod(33333); /** us */
        s32Ret |= HI_GV_SetLostFrameThreshold(15000); /** us */
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    HI_GV_Log_SetLevel(HI_NULL, HIGV_LOG_FATAL);

    s32Ret = HI_GV_PARSER_Init();

    /* set widget event callback function */
    s32Ret |= HI_GV_PARSER_SetWidgetEventFunc(g_pfunHIGVAppEventFunc, ARRAY_SIZE(g_pfunHIGVAppEventFunc));
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_ScrollGrid_RegisterWidget();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "RegisterWidget");

    /* load higv.bin file for parser */
    s32Ret = HI_GV_PARSER_LoadFile("./higv.bin");
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("LoadFile ret:%x\n", s32Ret);
        HI_GV_Deinit();
        HI_GV_PARSER_Deinit();
        return HI_FAILURE;
    }

    s32Ret = HI_GV_SetVsyncLostFrame(HI_FALSE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Lan_Register(LANG_EN_FILE, FONT18, LAN_ENGLISH);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Lan_Register(LANG_CN_FILE, FONT18, LAN_CHINESE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    HI_CHAR szContent[SETTING_CONTENT_SIZE];
    HI_CHAR szLanId[SETTING_CONTENT_SIZE];

    s32Ret = PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_BUTT, HI_PDT_PARAM_TYPE_LANGUAGE, szContent, SETTING_CONTENT_SIZE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_COMM_GetStringByString(szContent, szLanId, SETTING_CONTENT_SIZE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Lan_Change(szLanId);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = pthread_create(&s_UiThread, NULL, (HI_VOID *)PDT_UI_StartApp, NULL);
    if (0 != s32Ret)
    {
        MLOGE("pthread_create ret:%#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID PDT_UI_DeinitHigv(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_INVALID_HANDLE == s_AppHandle)
    {
        MLOGD("s_AppHandle is invalid\n");
        return;
    }

    s32Ret = PDT_UI_WINMNG_Deinit();
    MLOGD("PDT_UI_WINMNG_Deinit. %#x\n", s32Ret);

    s32Ret = HI_GV_App_Stop(s_AppHandle);
    MLOGD("HI_GV_App_Stop. %#x\n", s32Ret);

    s32Ret = pthread_join(s_UiThread, NULL);
    MLOGD("UI thread exit. %d\n", s32Ret);

    s32Ret = HI_GV_RenderCmdSync();
    MLOGD("HI_GV_RenderCmdSync. %#x\n", s32Ret);

    s32Ret = HI_GV_App_Destroy(s_AppHandle);
    MLOGD("HI_GV_App_Destroy. %#x\n", s32Ret);

    s_AppHandle = HI_INVALID_HANDLE;

    if(INVALID_HANDLE != s_LCDLayerHandle)
    {
        s32Ret = HI_GV_Layer_Destroy(s_LCDLayerHandle);
        MLOGD("HI_GV_Layer_Destroy. %#x\n", s32Ret);
        s_LCDLayerHandle = INVALID_HANDLE;
    }

    if(INVALID_HANDLE != s_HDMILayerHandle)
    {
        s32Ret = HI_GV_Layer_Destroy(s_HDMILayerHandle);
        MLOGD("HI_GV_Layer_Destroy. %#x\n", s32Ret);
        s_HDMILayerHandle = INVALID_HANDLE;
    }

    HI_GV_PARSER_Deinit();
    MLOGD("HI_GV_PARSER_Deinit\n");

    HI_GV_Deinit();
    MLOGD("HI_GV_Deinit\n");
}

static HI_S32 PDT_UI_GetLayerId(HI_PDT_WORKMODE_E enWorkMode, HIGO_LAYER_E* penLayerId)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE, &enMediaMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    HI_PDT_MEDIA_CFG_S stMediaCfg;
    HI_PDT_SCENE_MODE_S stSceneCfg;
    HI_PDT_PARAM_GetMediaCfg(enWorkMode, enMediaMode, &stMediaCfg, &stSceneCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    /* disphdl:0->HIGO_LAYER_HD_0/disphdl:1->HIGO_LAYER_HD_1 */
    *penLayerId = stMediaCfg.stVideoOutCfg.astDispCfg[0].VdispHdl + HIGO_LAYER_HD_0;
    MLOGI("LayerId[%d]\n", *penLayerId);
    return HI_SUCCESS;
}

HI_S32 PDT_UI_StartLCD(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HAL_SCREEN_ATTR_S stScreenAttr = {};
    HI_S32 s32ScreenWidth = 0;
    HI_S32 s32ScreenHeight = 0;

    s_bLCD = HI_TRUE;

    s32Ret = PDT_UI_InitHigv();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_WINMNG_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(INVALID_HANDLE == s_LCDLayerHandle)
    {
        /* GetLayer Id */
        HIGO_LAYER_E enLayerId = HIGO_LAYER_HD_0;
        s32Ret = PDT_UI_GetLayerId(HI_PDT_WORKMODE_NORM_REC, &enLayerId);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

#ifdef CONFIG_SCREEN
        s32Ret = HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_0, &stScreenAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
#endif

        s32ScreenWidth = (HI_S32)stScreenAttr.stAttr.u32Width;
        s32ScreenHeight = (HI_S32)stScreenAttr.stAttr.u32Height;

        HIGO_LAYER_INFO_S LayerInfo = {
            s32ScreenWidth, s32ScreenHeight,             /* Screen */
            LCD_XML_LAYOUT_WIDTH, LCD_XML_LAYOUT_HEIGHT, /* Canvas, same as the window width,height in XML */
            s32ScreenWidth, s32ScreenHeight,             /* Display */
            HIGO_LAYER_FLUSH_OVER, HIGO_LAYER_DEFLICKER_AUTO, HIGO_PF_4444,  enLayerId};

        MLOGD(GREEN"Create layer, Screen[%u * %u], Canvas[%u * %u], Display[%u * %u]\n"NONE,
            s32ScreenWidth, s32ScreenHeight,
            LCD_XML_LAYOUT_WIDTH, LCD_XML_LAYOUT_HEIGHT,
            s32ScreenWidth, s32ScreenHeight);

        /* create graphical layer */
        s32Ret = HI_GV_Layer_Create(&LayerInfo, &s_LCDLayerHandle);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

#if defined(CONFIG_VERTICAL_SCREEN)
        HIGV_ROTATE_E enRotate = HIGV_ROTATE_90;
#else
        HIGV_ROTATE_E enRotate = HIGV_ROTATE_NONE;
#endif

        s32Ret = HI_GV_Layer_SetRotateMode(s_LCDLayerHandle, enRotate);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    s32Ret = HI_GV_Layer_SetDefault(s_LCDLayerHandle);

    s32Ret |= HI_GV_PARSER_LoadViewById(ALARM_WINDOW);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    s32Ret = HI_GV_Widget_Hide(DIALOG_WINDOW_GROUPBOX_FORMAT);

    s32Ret = PDT_UI_WINMNG_StartWindow(HOME_PAGE,HI_FALSE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_PDT_MEDIA_SetDisplayGraphicCSC();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_COMM_InitKeyTone();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_StartHDMI(HI_VOID)
{
    HI_S32 s32Ret = 0;
    SIZE_S stDispSize;

    s_bLCD = HI_FALSE;

    s32Ret = PDT_UI_InitHigv();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_WINMNG_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(INVALID_HANDLE == s_HDMILayerHandle)
    {
        /* GetLayer Id */
        HIGO_LAYER_E enLayerId = HIGO_LAYER_HD_0;
        s32Ret = PDT_UI_GetLayerId(HI_PDT_WORKMODE_HDMI_PLAYBACK, &enLayerId);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

        s32Ret = HI_PDT_MEDIA_GetHDMIDispSize(&stDispSize);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        HIGO_LAYER_INFO_S LayerInfo = {
            stDispSize.u32Width, stDispSize.u32Height,     /* Screen */
            HDMI_XML_LAYOUT_WIDTH, HDMI_XML_LAYOUT_HEIGHT, /* Canvas, same as the width, height in XML */
            stDispSize.u32Width, stDispSize.u32Height,     /* Display */
            HIGO_LAYER_FLUSH_DOUBBUFER, HIGO_LAYER_DEFLICKER_AUTO, HIGO_PF_4444, enLayerId};

        MLOGI(GREEN"Create layer, Screen[%u * %u], Canvas[%u * %u], Display[%u * %u]\n"NONE,
            stDispSize.u32Width, stDispSize.u32Height,
            HDMI_XML_LAYOUT_WIDTH, HDMI_XML_LAYOUT_HEIGHT,
            stDispSize.u32Width, stDispSize.u32Height);

        s32Ret = HI_GV_Layer_Create(&LayerInfo, &s_HDMILayerHandle);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = HI_GV_Layer_SetRotateMode(s_HDMILayerHandle, HIGV_ROTATE_NONE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    s32Ret = HI_GV_Layer_SetDefault(s_HDMILayerHandle);

    s32Ret |= HI_GV_PARSER_LoadViewById(HDMI_ALARM_WINDOW);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_WINMNG_StartWindow(HDMI_HOME, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_PDT_MEDIA_SetDisplayGraphicCSC();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    PDT_UI_COMM_SdIsReady();

    s32Ret = PDT_UI_COMM_InitKeyTone();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}


/*public, it should be called by main()*/
HI_S32 HI_PDT_UI_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_StartLCD();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

#ifdef CFG_TIME_MEASURE_ON
    HI_PrintBootTime("UI");
#endif

    s32Ret = PDT_UI_COMM_SubscribeEvents();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_POWERCTRL_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}


HI_S32 HI_PDT_UI_Deinit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

#if defined(CONFIG_WIFI_ON)

    PDT_UI_WIFI_STATUS_S stWiFiStatus;

    s32Ret = PDT_UI_COMM_GetWiFiStatus(&stWiFiStatus);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    if(HI_TRUE == stWiFiStatus.bStart)
    {
        s32Ret = HI_HAL_WIFI_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
        MLOGD("HI_HAL_WIFI_Stop\n");

        stWiFiStatus.bStart = HI_FALSE;
    }

    if(HI_TRUE == stWiFiStatus.bInit)
    {
        /** deinit wifi service */
        s32Ret = HI_HAL_WIFI_Deinit();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        stWiFiStatus.bInit = HI_FALSE;
    }

    s32Ret = PDT_UI_COMM_SetWiFiStatus(&stWiFiStatus);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
#endif

    s32Ret = PDT_UI_COMM_DeinitKeyTone();
    MLOGD("DeinitKeyTone. %#x\n", s32Ret);


    PDT_UI_DeinitHigv();

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

