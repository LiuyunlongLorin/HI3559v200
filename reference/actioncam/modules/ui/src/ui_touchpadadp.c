/**
 * @file      ui_touchadp.c
 * @brief     touch adapter.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <string.h>

#include "hi_type.h"
#include "hi_appcomm.h"
#include "hi_gv_gesture.h"
#include "hi_hal_touchpad.h"
#include "hi_hal_screen.h"
#include "hi_eventhub.h"
#include "hi_product_ui.h"
#include "ui_common.h"
#include "ui_powercontrol.h"

#undef MODULE_NAME
#define MODULE_NAME "UI"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#if defined(CFG_LCD_TOUCHPAD_ON)
static HI_S32 s_s32ScreenWidth = 0;
static HI_S32 s_s32ScreenHeight = 0;

HI_S32 PDT_UI_TouchHook(HIGV_TOUCH_POINT_S* pTouchEvent)
{
    static HI_BOOL s_bPressure = HI_FALSE;
    HI_EVENT_S stEvent = {};
    stEvent.EventID = HI_EVENT_UI_TOUCH;

    if(HI_FALSE == s_bPressure && 1 == pTouchEvent->pressure)
    {
        s_bPressure = HI_TRUE;
    }
    else if(0 == pTouchEvent->pressure)
    {
        s_bPressure = HI_FALSE;
    }
    else
    {
        return HI_SUCCESS;
    }

    return HI_EVTHUB_Publish(&stEvent);
}


static HI_S32 PDT_UI_ReadTouchData(HIGV_TOUCH_INPUTINFO_S* info)
{
    HI_S32 s32Ret;
    HIGV_ROTATE_E enRotate = HIGV_ROTATE_BUTT;
    HI_HAL_TOUCHPAD_INPUTINFO_S stHalInputInfo = {};

    if(0 == s_s32ScreenWidth || 0 == s_s32ScreenHeight)
    {
#if defined(CONFIG_SCREEN)
        HI_HAL_SCREEN_ATTR_S stScreenAttr = {};

        s32Ret = HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_0, &stScreenAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

        if (0 == stScreenAttr.stAttr.u32Width ||
            0 == stScreenAttr.stAttr.u32Height)
        {
            MLOGE(RED"resolution[%d, %d] error\n"NONE, stScreenAttr.stAttr.u32Width, 0 == stScreenAttr.stAttr.u32Height);
            return HI_FAILURE;
        }

        s_s32ScreenWidth = (HI_S32)stScreenAttr.stAttr.u32Width;
        s_s32ScreenHeight = (HI_S32)stScreenAttr.stAttr.u32Height;

        MLOGD(YELLOW"resolution[%d, %d]\n"NONE, s_s32ScreenWidth, s_s32ScreenHeight);
#else
    s_s32ScreenWidth = LCD_XML_LAYOUT_WIDTH;
    s_s32ScreenHeight = LCD_XML_LAYOUT_HEIGHT;
#endif
    }

    s32Ret = HI_HAL_TOUCHPAD_ReadInputEvent(&stHalInputInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    /** only support single touch */
    info->id = 0;

#if defined(CONFIG_FLIP_TOUCHPAD)
    stHalInputInfo.s32X = s_s32ScreenWidth - stHalInputInfo.s32X;
    stHalInputInfo.s32Y = s_s32ScreenHeight - stHalInputInfo.s32Y;
#else
    stHalInputInfo.s32X = stHalInputInfo.s32X;
    stHalInputInfo.s32Y = stHalInputInfo.s32Y;
#endif


#if defined(CONFIG_VERTICAL_SCREEN)
    enRotate = HIGV_ROTATE_90;
#else
    enRotate = HIGV_ROTATE_NONE;
#endif

    if(HIGV_ROTATE_90 == enRotate)
    {
        info->x =  stHalInputInfo.s32Y * LCD_XML_LAYOUT_WIDTH / s_s32ScreenHeight;
        /** info->y = (s_s32ScreenWidth - stHalInputInfo.s32X) * LCD_XML_LAYOUT_HEIGHT / s_s32ScreenWidth; */
        info->y = LCD_XML_LAYOUT_HEIGHT - stHalInputInfo.s32X * LCD_XML_LAYOUT_HEIGHT / s_s32ScreenWidth;

        if(LCD_XML_LAYOUT_HEIGHT == s_s32ScreenWidth && LCD_XML_LAYOUT_HEIGHT == info->y)
        {
            info->y -= 1;
        }
    }
    else if(HIGV_ROTATE_NONE== enRotate)
    {
        /** info->x =  (s_s32ScreenHeight - stHalInputInfo.s32X) * LCD_XML_LAYOUT_HEIGHT / s_s32ScreenHeight; */
        info->x =  LCD_XML_LAYOUT_HEIGHT - stHalInputInfo.s32X * LCD_XML_LAYOUT_HEIGHT / s_s32ScreenHeight;
        info->y =   stHalInputInfo.s32Y * LCD_XML_LAYOUT_WIDTH / s_s32ScreenWidth;

        if(LCD_XML_LAYOUT_WIDTH == s_s32ScreenWidth && LCD_XML_LAYOUT_WIDTH == info->y)
        {
            info->y -= 1;
        }
    }

    info->pressure = stHalInputInfo.u32Pressure;
    info->timeStamp = stHalInputInfo.u32TimeStamp;

    return HI_SUCCESS;
}

HI_S32 PDT_UI_RegisterTouchDevice(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_TOUCH_DEVICE_INPUT_S stInputDevice = {};
    stInputDevice.OpenDevice = HI_HAL_TOUCHPAD_Start;
    stInputDevice.CloseDevie = HI_HAL_TOUCHPAD_Stop;
    stInputDevice.ReadData   = PDT_UI_ReadTouchData;

    s32Ret = HI_GV_Gesture_RegisterDevice(&stInputDevice);
    s32Ret |= HI_GV_Gesture_SetTouchHook(PDT_UI_TouchHook);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

