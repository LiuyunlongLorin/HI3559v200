/**
 * @file      ui_busy_page.c
 * @brief     ui window for waiting something
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "ui_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */


HI_S32 PDT_UI_BUSY_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");



    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_BUSY_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_BUSY_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_BUSY_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_BUSY_OnKeyUp(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);


    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_BUSY_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    if (PDT_UI_KEY_BACK == wParam)
    {
#if 0
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        if(s32Ret)
        {
            MLOGE("PDT_UI_WINMNG_FinishWindow failed: %x\n", s32Ret);
        }
#endif
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {

    }
    else if (PDT_UI_KEY_OK == wParam)
    {

    }
    else
    {
        MLOGE("KEY CODE %#lx not processed\n", (HI_UL)wParam);
    }

    return HIGV_PROC_GOON;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

