/**
 * @file      ui_init.c
 * @brief     ui init and deinit
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/7
 * @version   1.0

 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <semaphore.h>


#include "ui_common.h"
#include "hi_gv_resm.h"
#include "hi_gv_parser.h"
#include "hi_gv.h"
#include "hi_gv_scrollgrid.h"
#include "hi_gv_input.h"
#include "hi_gv_msg.h"
#include "hi_gv_mlayer.h"
#include "hi_gv_log.h"
#include "hi_product_media.h"
#include "hi_appcomm.h"
#include "hi_appcomm_util.h"
#include "hi_product_ui.h"
#include "hi_hal_screen.h"
#include "hi_voiceplay.h"
#include "ui_powercontrol.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */




#define LANG_EN_FILE   "./en.lang"
#define LANG_CN_FILE   "./zh.lang"
#define LAN_CHINESE    "zh"
#define LAN_ENGLISH    "en"

#define PDT_UI_BITMAP_PIXELFORMAT   HIGO_PF_4444

#define PDT_UI_HIFB_ALIGNMENT       0xf


static HI_HANDLE s_AppHandle;
static HI_HANDLE s_LayerHandle = INVALID_HANDLE;
static pthread_t s_UiThread;


static HI_VOID * PDT_UI_StartApp(HI_VOID* pVoid)
{
    HI_S32 s32Ret;

    s32Ret = HI_GV_App_Create("MainApp", (HI_HANDLE *)&s_AppHandle);
    if (HI_SUCCESS != s32Ret)
    {
        HI_GV_Deinit();
        HI_GV_PARSER_Deinit();
        return NULL;
    }
    prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);

    /*HI_GV_App_Start will not return until  HI_GV_App_Stop is called. */
    s32Ret = HI_GV_App_Start(s_AppHandle);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, NULL, "HI_GV_App_Start");

    return NULL;
}


/*  public section between LCD and HDMI initialization*/
static HI_S32 PDT_UI_InitHigv(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Init();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_Init");

    HI_GV_Log_SetLevel(HI_NULL, HIGV_LOG_FATAL);

    s32Ret = HI_GV_PARSER_Init();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_PARSER_Init");

    /* set widget event callback function */
    s32Ret = HI_GV_PARSER_SetWidgetEventFunc(g_pfunHIGVAppEventFunc, UI_ARRAY_SIZE(g_pfunHIGVAppEventFunc));
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"HI_GV_PARSER_SetWidgetEventFunc");

    s32Ret = HI_GV_ScrollGrid_RegisterWidget();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "RegisterWidget");

#if defined(CFG_LCD_TOUCHPAD_ON)
    s32Ret = PDT_UI_RegisterTouchDevice();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"UI_RegisterTouchDevice");
#endif
    /* load higv.bin file for parser */
    s32Ret = HI_GV_PARSER_LoadFile("./higv.bin");
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_GV_PARSER_LoadFile ret:%x\n", s32Ret);
        HI_GV_Deinit();
        HI_GV_PARSER_Deinit();
        return HI_FAILURE;
    }

    s32Ret = HI_GV_Lan_Register(LANG_EN_FILE, FONT32, LAN_ENGLISH);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Lan_Register(LANG_CN_FILE, FONT32, LAN_CHINESE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    HI_CHAR szContent[SETTING_CONTENT_SIZE];
    HI_S32 s32CamID = 0;

    s32CamID = HI_PDT_STATEMNG_GetPreViewCamID();
    s32Ret = PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_BUTT, s32CamID, HI_PDT_PARAM_TYPE_LANGUAGE,
            szContent, SETTING_CONTENT_SIZE);
    s32Ret = HI_GV_Lan_Change(szContent);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(HI_TRUE == HI_GV_IsVsyncLostFrame())
    {
        s32Ret = HI_GV_SetVsyncLostFrame(HI_FALSE);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }

    return HI_SUCCESS;
}

#ifdef CONFIG_SCREEN
static HI_S32 PDT_UI_StartLCD(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HAL_SCREEN_ATTR_S stScreenAttr = {};
    HI_S32 s32ScreenWidth = 0;
    HI_S32 s32ScreenHeight = 0;
    HIGV_ROTATE_E enRotate = HIGV_ROTATE_BUTT;

    s32Ret = PDT_UI_WINMNG_Init();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"PDT_UI_WINMNG_Inits");

    HI_PDT_PARAM_MEDIA_COMM_CFG_S stMediaCommCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stMediaCommCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"HI_PDT_PARAM_GetMediaCommCfg");

    HIGO_LAYER_E LayerID = (stMediaCommCfg.stVideoOutCfg.astDispCfg[0].VdispHdl + HIGO_LAYER_HD_0);

    s32Ret = HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_0, &stScreenAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32ScreenWidth = (HI_S32)stScreenAttr.stAttr.u32Width;
    s32ScreenHeight = (HI_S32)stScreenAttr.stAttr.u32Height;

    MLOGD(YELLOW"Screen Width:[%d], Height: [%d]\n"NONE,s32ScreenWidth, s32ScreenHeight);

#if defined(CONFIG_VERTICAL_SCREEN)
    enRotate = HIGV_ROTATE_90;
#else
    enRotate = HIGV_ROTATE_NONE;
#endif

    HIGO_LAYER_INFO_S LayerInfo =
    {
        s32ScreenWidth, s32ScreenHeight, /*Screen*/
        XML_LAYOUT_WIDTH, XML_LAYOUT_HEIGHT, /*Canvas,  same as the window width,height in XML*/
        s32ScreenWidth, s32ScreenHeight, /*Display */
        (HIGO_LAYER_FLUSHTYPE_E)(HIGO_LAYER_FLUSH_DOUBBUFER),
        HIGO_LAYER_DEFLICKER_AUTO, PDT_UI_BITMAP_PIXELFORMAT, LayerID
    };

    /* create graphical layer */
    s32Ret = HI_GV_Layer_Create(&LayerInfo, &s_LayerHandle);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "Create Layer");

    s32Ret = HI_GV_Layer_SetRotateMode(s_LayerHandle, enRotate);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "SetRotateMode");


    s32Ret = HI_GV_PARSER_LoadViewById(ALARM_WINDOW);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"HI_GV_PARSER_LoadViewById");

    s32Ret = HI_PDT_MEDIA_SetDisplayGraphicCSC();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"HI_PDT_MEDIA_SetDisplayGraphicCSC");

    s32Ret = PDT_UI_WINMNG_StartWindow(HOME_PAGE,HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"PDT_UI_WINMNG_StartWindow");

    s32Ret = pthread_create(&s_UiThread, NULL, (HI_VOID *)PDT_UI_StartApp, NULL);
    if (0 != s32Ret)
    {
        MLOGE("pthread_create UI_StartApp Failed, s32Ret=%#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

#else

static HI_S32 PDT_UI_StartHDMI(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_S32 s32ScreenWidth = 0;
    HI_S32 s32ScreenHeight = 0;

    s32Ret = PDT_UI_WINMNG_Init();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"UI_WINMNG_Deinit");

    /*TODO later:  get HDMI output resolution,  temp value 1920x1080 */
    s32ScreenWidth = XML_LAYOUT_WIDTH;
    s32ScreenHeight = XML_LAYOUT_HEIGHT;

    HIGO_LAYER_INFO_S LayerInfo =
    {
        s32ScreenWidth, s32ScreenHeight,   /*Screen*/
        XML_LAYOUT_WIDTH, XML_LAYOUT_HEIGHT,  /*Canvas,  same as the width,height in XML*/
        s32ScreenWidth, s32ScreenHeight,   /*Display */
        (HIGO_LAYER_FLUSHTYPE_E)(HIGO_LAYER_FLUSH_DOUBBUFER),
        HIGO_LAYER_DEFLICKER_AUTO,
        PDT_UI_BITMAP_PIXELFORMAT, HIGO_LAYER_HD_0
    };

    s32Ret = HI_GV_Layer_Create(&LayerInfo, &s_LayerHandle);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "Create Layer");

    s32Ret = HI_GV_Layer_SetRotateMode(s_LayerHandle, HIGV_ROTATE_NONE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "SetRotateMode");

    s32Ret = HI_GV_PARSER_LoadViewById(ALARM_WINDOW);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"HI_GV_PARSER_LoadViewById");

    s32Ret = PDT_UI_WINMNG_StartWindow(HOME_PAGE, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"UI_StartWindow");

    s32Ret = HI_PDT_MEDIA_SetDisplayGraphicCSC();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"HI_PDT_MEDIA_SetDisplayGraphicCSC");

    s32Ret = pthread_create(&s_UiThread, NULL, (HI_VOID *)PDT_UI_StartApp, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("pthread_create UI_StartApp Failed, s32Ret=%#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
#endif

static HI_VOID PDT_UI_LoadHiFB(HI_VOID)
{
    MLOGD(YELLOW"load HiFB ko"NONE"\n");
    HI_U32 u32Fb0Size_KB;
    HI_CHAR szCmdParam[256] = {0,};
    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;
    HI_U32 u32BitsPerPixel = 0;

#if defined(CONFIG_SCREEN)
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HAL_SCREEN_ATTR_S stScreenAttr;
    memset(&stScreenAttr, 0, sizeof(stScreenAttr));
    s32Ret = HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_0, &stScreenAttr);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_SCREEN_GetAttr fail\n");
        return;
    }
    u32Width = stScreenAttr.stAttr.u32Width;
    u32Height = stScreenAttr.stAttr.u32Height;
#else
    /**HDMI*/
    u32Width = XML_LAYOUT_WIDTH;
    u32Height = XML_LAYOUT_HEIGHT;
#endif

    switch(PDT_UI_BITMAP_PIXELFORMAT)
    {
        case HIGO_PF_4444:
            u32BitsPerPixel = 4 * 4;
            break;
        case HIGO_PF_8888:
            u32BitsPerPixel = 4 * 8;
            break;
        default:
            u32BitsPerPixel = 0;
            MLOGE("bitmap %d pixel format need to be added.\n",PDT_UI_BITMAP_PIXELFORMAT);
            break;
    }

    u32Fb0Size_KB = ((u32Height *((((u32Width * u32BitsPerPixel) >> 3) + PDT_UI_HIFB_ALIGNMENT) & (~PDT_UI_HIFB_ALIGNMENT)))+1023)/1024;

#if defined(HI3556AV100) || defined(HI3519AV100)
    snprintf(szCmdParam, sizeof(szCmdParam), "video=\"hifb:vram0_size:1250,vram1_size:%d,vram2_size:0\"", u32Fb0Size_KB);
#elif defined(HI3559V200)
    snprintf(szCmdParam, sizeof(szCmdParam), "video=\"hifb:vram0_size:%d\"", u32Fb0Size_KB);
#endif
    MLOGI(YELLOW"u32Fb0Size_KB is %d.\n"NONE"\n",u32Fb0Size_KB);
    HI_insmod(HI_APPFS_KOMOD_PATH"/hifb.ko", szCmdParam);

    return;
}

/*public, it should be called by main()*/
HI_S32 HI_PDT_UI_Init(HI_VOID)
{
    HI_S32 s32Ret;

    PDT_UI_LoadHiFB();

    s32Ret = PDT_UI_InitHigv();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

#ifdef HDMI_TEST
    s32Ret = PDT_UI_StartHDMI();
#else
    s32Ret = PDT_UI_StartLCD();
#endif
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_COMM_SubscribeEvents();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"PDT_UI_COMM_SubscribeEvents");

    s32Ret = PDT_UI_POWERCTRL_Init();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_UI_POWERCTRL_Init");

    HI_PDT_PARAM_MEDIA_COMM_CFG_S stMediaCommCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stMediaCommCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"HI_PDT_PARAM_GetMediaCommCfg");

    HI_VOICEPLAY_CFG_S stVoicePlayCfg;
    stVoicePlayCfg.stAoutOpt.hAudDevHdl = stMediaCommCfg.stAudioOutCfg.astAoCfg[0].AoHdl;
    stVoicePlayCfg.stAoutOpt.hAudTrackHdl = (HI_MAPI_AO_CHN_MAX_NUM - 1);
    stVoicePlayCfg.u32MaxVoiceCnt = PDT_UI_VOICE_MAX_NUM;
    HI_VOICEPLAY_VOICETABLE_S astVoiceTab[PDT_UI_VOICE_MAX_NUM]=
    {
        {PDT_UI_VOICE_SD_NO_EXIST_IDX,PDT_UI_VOICE_SD_NO_EXIST_SRC},
        {PDT_UI_VOICE_EMR_START_IDX,PDT_UI_VOICE_EMR_START_SRC},
        {PDT_UI_VOICE_WIFI_IDX,PDT_UI_VOICE_WIFI_SRC},
        {PDT_UI_VOICE_OPEN_IDX,PDT_UI_VOICE_OPEN_SRC},
        {PDT_UI_VOICE_CLOSE_IDX,PDT_UI_VOICE_CLOSE_SRC}
    };
    stVoicePlayCfg.pstVoiceTab = astVoiceTab;
    s32Ret = HI_VOICEPLAY_Init(&stVoicePlayCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"HI_VOICEPLAY_Init");
    return HI_SUCCESS;
}


HI_S32 HI_PDT_UI_Deinit(HI_VOID)
{
    HI_S32 s32Ret = 0;

    if(HI_INVALID_HANDLE == s_AppHandle)
    {
        MLOGD("hApp invalid\n");
        return HI_FAILURE;
    }

    HI_VOICEPLAY_Deinit();

#if defined(CONFIG_WIFI_ON)
    HI_BOOL bWifiState = HI_FALSE;
    s32Ret = HI_HAL_WIFI_GetStartedStatus(&bWifiState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    if(HI_TRUE == bWifiState)
    {
        s32Ret = HI_HAL_WIFI_Stop();
        MLOGD("HI_HAL_WIFI_Stop\n");

        s32Ret = HI_HAL_WIFI_Deinit();
    }
#endif
    s32Ret = HI_GV_App_Stop(s_AppHandle);
    MLOGD("HI_GV_App_Stop. %#x\n", s32Ret);

    s32Ret = pthread_join(s_UiThread, NULL);
    MLOGD("UI thread exit. %d\n", s32Ret);

    s32Ret = HI_GV_App_Destroy(s_AppHandle);
    MLOGD("HI_GV_App_Destroy. %#x\n", s32Ret);

    s32Ret = HI_GV_Layer_Destroy(s_LayerHandle);
    MLOGD("HI_GV_Layer_Destroy. %#x\n", s32Ret);
    s_AppHandle = HI_INVALID_HANDLE;

    HI_GV_PARSER_Deinit();
    MLOGD("HI_GV_PARSER_Deinit\n");

    HI_GV_Deinit();
    MLOGD("HI_GV_Deinit\n");

    s32Ret = PDT_UI_WINMNG_Deinit();
    MLOGD("PDT_UI_WINMNG_Deinit. %#x\n", s32Ret);

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

