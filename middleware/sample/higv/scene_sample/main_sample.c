#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "hi_gv.h"
#include "hi_gv_parser.h"
#include "hi_gv_adm.h"
#include "hi_gv_ddb.h"
#include "hi_gv_log.h"

#include "higv_cextfile.h"
#include "higv_language.h"

#include "sample_utils.h"

extern "C"
{
#include "hi_mw_media_intf.h"
}


#define PROJECT_ID  MAIN_WIN

HIGV_HANDLE s_LayerId = INVALID_HANDLE;

static HIGV_HANDLE s_hLanTestFont = 1;
HIGV_HANDLE g_hApp = 0;

static HI_S32 HI_GV_TP_ReadData(HIGV_TOUCH_INPUTINFO_S* pstInputData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MW_TOUCH_INPUTINFO_S stInputInfo;
    memset(&stInputInfo, 0x0, sizeof(stInputInfo));

    s32Ret = HI_MW_TP_ReadData(&stInputInfo);

    if (s32Ret != HI_SUCCESS)
    {
        printf("[Func: %s, Line: %d] HI_MW_TP_ReadData Error.\n\n", __FUNCTION__, __LINE__);
        return s32Ret;
    }

    pstInputData->id = stInputInfo.id;
    pstInputData->x  = stInputInfo.x;
    pstInputData->y = stInputInfo.y;
    pstInputData->pressure = stInputInfo.pressure;
    pstInputData->timeStamp = stInputInfo.timeStamp;

    return s32Ret;
}

static HI_S32 HI_GV_Touch_Hook(HIGV_TOUCH_POINT_S* pTouchEvent)
{
    //printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
    return HI_SUCCESS;
}


HI_S32 HI_GV_TOUCH_Register()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_TOUCH_DEVICE_INPUT_S stInputDevice = {};
    stInputDevice.OpenDevice = HI_MW_TP_Open;
    stInputDevice.CloseDevie = HI_MW_TP_Close;
    stInputDevice.ReadData   = HI_GV_TP_ReadData;

    (HI_VOID)HI_GV_Gesture_RegisterDevice(&stInputDevice);
    (HI_VOID)HI_GV_Gesture_SetTouchHook(HI_GV_Touch_Hook);

    return HI_SUCCESS;
}

static void* start_higv_app(void* pArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_HANDLE hFont = INVALID_HANDLE;
    HIGO_LAYER_FLUSHTYPE_E eHigoFlushType = HIGO_LAYER_FLUSH_OVER;
    HIGO_LAYER_E eHigoLayer = HIGO_LAYER_HD_0;

#if (defined(__HI3556__) || defined(__HI3559__) || defined(__HI3559V200__))
    eHigoLayer = HIGO_LAYER_HD_0;
#endif

#if (defined(__HI3556AV100__) || defined(__HI3559AV100__))
    eHigoLayer = HIGO_LAYER_HD_1;
#endif

    /** 图层信息*/

    HIGO_LAYER_INFO_S LayerInfo =
    {
        SCREEN_WIDTH, SCREEN_HEIGHT,
        320, 240, //canvas is rotate,so change width and height
        SCREEN_WIDTH, SCREEN_HEIGHT,
        eHigoFlushType,
        HIGO_LAYER_DEFLICKER_AUTO,
        HIGO_PF_4444, eHigoLayer
    };

    //打开VO输出
    s32Ret = HI_MW_DISP_Open();

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MW_DISP_Open failed! Return: %x\n", s32Ret);
        return NULL;
    }

    s32Ret = HI_GV_TOUCH_Register();

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_TOUCH_Register failed! Return: %x\n", s32Ret);
        return NULL;
    }

    s32Ret = HI_GV_Gesture_SetScrollTV(1);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_Gesture_SetScrollTV failed! Return: %x\n", s32Ret);
        return NULL;
    }

    //higv 初始化
    s32Ret = HI_GV_Init();

    if (HI_SUCCESS != s32Ret)
    {
        return NULL;
    }

    //parser 初始化
    s32Ret = HI_GV_PARSER_Init();

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_PARSER_Init failed! Return: %x\n", s32Ret);
        HI_GV_Deinit();
        return NULL;
    }

    HI_GV_PARSER_SetWidgetEventFunc(g_pfunHIGVAppEventFunc, sizeof(g_pfunHIGVAppEventFunc) / sizeof(HI_CHAR*));

    //加载higv.bin文件
    s32Ret = HI_GV_PARSER_LoadFile("./higv.bin");

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_PARSER_LoadFile failed! Return: %x\n", s32Ret);
        HI_GV_PARSER_Deinit();
        HI_GV_Deinit();
        return NULL;
    }

    //创建图层
    s32Ret = HI_GV_Layer_Create(&LayerInfo, &s_LayerId);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_Layer_Create failed! Return: %x\n", s32Ret);
        HI_GV_PARSER_Deinit();
        HI_GV_Deinit();
        return NULL;
    }

    s32Ret = HI_GV_Layer_SetRotateMode(s_LayerId, HIGV_ROTATE_90);

    if (s32Ret != 0)
    {
        printf("HI_GV_Layer_SetRotateMode failed!: %x\n", s32Ret);
        HI_GV_Layer_Destroy(s_LayerId);
        HI_GV_PARSER_Deinit();
        HI_GV_Deinit();
        return NULL;
    }

    Widget_AppCreateSysFont(&hFont);

    (HI_VOID)HI_GV_Font_SetSystemDefault(hFont);
    s_hLanTestFont = hFont;

    HI_GV_Lan_Register("./res/lan/en.lang",  s_hLanTestFont, LAN_EN);
    HI_GV_Lan_Register("./res/lan/fr.lang",  s_hLanTestFont, LAN_FR);
    HI_GV_Lan_Register("./res/lan/es.lang",  s_hLanTestFont, LAN_ES);
    HI_GV_Lan_Register("./res/lan/pt.lang",  s_hLanTestFont, LAN_PT);
    HI_GV_Lan_Register("./res/lan/de.lang",  s_hLanTestFont, LAN_DE);
    HI_GV_Lan_Register("./res/lan/it.lang",  s_hLanTestFont, LAN_IT);
    HI_GV_Lan_Register("./res/lan/zh.lang",  s_hLanTestFont, LAN_ZH);
    HI_GV_Lan_Register("./res/lan/zh_TW.lang",  s_hLanTestFont, LAN_ZH_TW);
    HI_GV_Lan_Register("./res/lan/ru.lang",  s_hLanTestFont, LAN_RU);
    HI_GV_Lan_Register("./res/lan/ja.lang",  s_hLanTestFont, LAN_JA);
    HI_GV_Lan_Register("./res/lan/po.lang",  s_hLanTestFont, LAN_PO);
    HI_GV_Lan_Register("./res/lan/ar.lang",  s_hLanTestFont, LAN_AR);

    //加载视图
    s32Ret = HI_GV_PARSER_LoadViewById(PROJECT_ID);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_PARSER_LoadViewById failed! Return: %x\n", s32Ret);
        HI_GV_Layer_Destroy(s_LayerId);
        HI_GV_PARSER_Deinit();
        HI_GV_Deinit();
        return NULL;
    }

    //创建应用程序实例
    s32Ret = HI_GV_App_Create("MainApp", (HIGV_HANDLE*)&g_hApp);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_App_Create failed! Return: %x\n", s32Ret);
        HI_GV_Layer_Destroy(s_LayerId);
        HI_GV_PARSER_Deinit();
        HI_GV_Deinit();
        return NULL;
    }

#if 0
    s32Ret = HI_GV_Log_SetLevel(HI_NULL, HIGV_LOG_DEBUG);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_Log_SetLevel failed! Return: %x\n", s32Ret);
    }

#endif
    s32Ret = HI_GV_Widget_Show(PROJECT_ID);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_Widget_Show failed! Return: %x\n", s32Ret);
        HI_GV_Layer_Destroy(s_LayerId);
        HI_GV_PARSER_Deinit();
        HI_GV_Deinit();
        return NULL;
    }

    s32Ret = HI_GV_Widget_Active(PROJECT_ID);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_Widget_Show failed! Return: %x\n", s32Ret);
        HI_GV_Layer_Destroy(s_LayerId);
        HI_GV_PARSER_Deinit();
        HI_GV_Deinit();
        return NULL;
    }

    HI_MW_DISP_SetDisplayGraphicCSC();


    //启动应用程序
    s32Ret = HI_GV_App_Start(g_hApp);

    if (HI_SUCCESS != s32Ret)
    {
        HI_GV_App_Destroy(g_hApp);
        HI_GV_PARSER_Deinit();
        HI_GV_Deinit();
        return NULL;
    }

    HI_GV_App_Destroy(g_hApp);
    HI_GV_Layer_Destroy(s_LayerId);
    HI_GV_PARSER_Deinit();
    HI_GV_Deinit();
    HI_MW_DISP_Close();

    return 0;
}

HI_S32 main(HI_S32 argc, HI_CHAR* argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR cmd[64];
    pthread_t thread_id = 0;

    s32Ret = pthread_create(&thread_id, NULL, start_higv_app, NULL);

    if (s32Ret != HI_SUCCESS)
    {
        printf("pthread_create exec failed \n");
    }

    printf("\nInput CMD: 'quit' or 'q' for quit the program!\n");

    while (fgets(cmd, 10, stdin))
    {
        cmd[10] = '\0';

        if (strncmp(cmd, "quit", 4) == 0)
        {
            HI_GV_App_Stop(g_hApp);
            break;
        }
        else if (strncmp(cmd, "q", 1) == 0 )
        {
            HI_GV_App_Stop(g_hApp);
            break;
        }
        else
        {
            printf("Input CMD: 'quit' or 'q' for quit the program!\n");
        }
    }

    s32Ret = pthread_join(thread_id, NULL);

    if (s32Ret != HI_SUCCESS)
    {
        printf("pthread_create exec failed \n");
    }

    return HI_SUCCESS;
}
