/**
 * @file      ui_lock_page.c
 * @brief     ui lock page, unlock by touch.
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


#define LOCK_RADIUS    (30)
#define BORDER_RADIUS  (100)

static HI_BOOL bIsDraging = HI_FALSE;
static HI_S32 s32SavedX, s32SavedY; /* last touch's coordinate */
static HI_S32 s32LockX, s32LockY;   /* lock button's coordinate */

static HI_VOID ResetLockStatus(HI_VOID)
{
    HI_RECT rectLock = {};
    HI_S32 s32Ret = 0;

    s32Ret = HI_GV_Widget_GetRect(LOCK_BUTTON, &rectLock);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("failed. %x\n", s32Ret);
        return;
    }

    bIsDraging = HI_FALSE;
    s32SavedX = 0;
    s32SavedY = 0;
    s32LockX = UI_SCREEN_WIDTH/2 - rectLock.w/2;
    s32LockY = UI_SCREEN_HEIGHT/2 - rectLock.h/2;

    s32Ret = HI_GV_Widget_Move(LOCK_BUTTON, s32LockX, s32LockY);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("failed. %x\n", s32Ret);
        return;
    }

}
HI_S32 PDT_UI_LOCK_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");

    ResetLockStatus();

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_LOCK_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);

    HI_S32 s32Ret = HI_SUCCESS;
    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(LOCK_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    }
    else
    {
        MLOGD("KEY CODE %#lx not processed\n", (HI_UL)wParam);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_LOCK_OnTouchAction(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = 0, x=0, y=0, centerX, centerY;
    HIGV_TOUCH_EVENT_S* pstTouchEvent = (HIGV_TOUCH_EVENT_S*)lParam;

    x = pstTouchEvent->last.x;
    y = pstTouchEvent->last.y;
    centerX = UI_SCREEN_WIDTH / 2;
    centerY = UI_SCREEN_HEIGHT / 2;

    switch(pstTouchEvent->last.type)
    {
        case HIGV_TOUCH_START:
            if( DISTANCE(x,y, centerX, centerY) <= LOCK_RADIUS )
            {
                bIsDraging = HI_TRUE;
                s32SavedX = x;
                s32SavedY = y;
            }
            break;

        case HIGV_TOUCH_MOVE:
            if( ! bIsDraging)
            {
                break;
            }
            s32LockX = s32LockX + (x - s32SavedX);
            s32LockY = s32LockY + (y - s32SavedY);
            s32Ret = HI_GV_Widget_Move(LOCK_BUTTON, s32LockX, s32LockY);
            if(HI_SUCCESS != s32Ret)
            {
                MLOGE("failed. %x\n", s32Ret);
            }
            if( BORDER_RADIUS <= DISTANCE(centerX,centerY, s32LockX+LOCK_RADIUS, s32LockY+LOCK_RADIUS) )
            {
                s32Ret = PDT_UI_WINMNG_FinishWindow(LOCK_PAGE);
                MLOGD("Unlock screen. %x\n", s32Ret);
            }
            s32SavedX = x;
            s32SavedY = y;
            break;

        case HIGV_TOUCH_CANCEL:
        case HIGV_TOUCH_END:
            ResetLockStatus();
            break;

        //case HIGV_TOUCH_POINTER_START:
        //case HIGV_TOUCH_POINTER_END:
        default:
            MLOGD("not handled touch\n");
            break;


    }

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

