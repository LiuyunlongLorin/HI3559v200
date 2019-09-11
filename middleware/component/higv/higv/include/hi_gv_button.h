/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: Button API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_BUTTON_H__
#define __HI_GV_BUTTON_H__

/* add include here */
#include "hi_gv_resm.h"
#include "hi_gv_conf.h"

#ifdef HIGV_USE_WIDGET_BUTTON
#ifdef __cplusplus
extern "C" {
#endif
/* ************************** Structure Definition *************************** */
/* *addtogroup      Button    */
/* * 【Button Widget】 ; 【Button控件】 */

/* * Style of button, it is only support one style */
/* * CNcomment:button控件私有风格，只能选择其中的一种风格 */
/* *Normal button ; CNcomment:普通按钮 */
#define HIGV_BS_PUSH HIGV_STYLE_DEFINE(HIGV_WIDGET_BUTTON, BUTTON_STYLE_PUSH)

/* * Radio button ; CNcomment:单选按钮 */
#define HIGV_BS_RADIO HIGV_STYLE_DEFINE(HIGV_WIDGET_BUTTON, BUTTON_STYLE_RADIO)

/* * Check button ; CNcomment:复选按钮 */
#define HIGV_BS_CHECK HIGV_STYLE_DEFINE(HIGV_WIDGET_BUTTON, BUTTON_STYLE_CHECK)

/* * Softkey button ; CNcomment:软键盘按钮 */
#define HIGV_BS_SOFTKEY HIGV_STYLE_DEFINE(HIGV_WIDGET_BUTTON, BUTTON_STYLE_SOFTKEY)

/* * Switch button ; CNcomment:开关按钮 */
#define HIGV_BS_SWITCH HIGV_STYLE_DEFINE(HIGV_WIDGET_BUTTON, BUTTON_STYLE_SWITCH)

/* * Toggle button ; CNcomment:纽扣开关按钮 */
#define HIGV_BS_TOGGLE HIGV_STYLE_DEFINE(HIGV_WIDGET_BUTTON, BUTTON_STYLE_TOGGLE)

/* * Paint toggle min height ; CNcomment:绘制toggle的控件最小高度，小于此高度的toggle按钮不绘制纽扣 */
#define HIGV_BUTTON_TOGGLE_MIN_HEIGHT 5

/* * Paint toggle min width ; CNcomment:绘制toggle的控件最小宽度，小于此宽度的toggle按钮不绘制纽扣 */
#define HIGV_BUTTON_TOGGLE_MIN_WIDTH 19

/* *Button type ; CNcomment: button类型枚举 */
typedef enum {
    BUTTON_STYLE_PUSH = 0,
    /* Push button , CNcomment: 普通按钮 */
    BUTTON_STYLE_RADIO,
    /* Radio button , CNcomment: 单选按钮 */
    BUTTON_STYLE_CHECK,
    /* Check button , CNcomment: 复选按钮 */
    BUTTON_STYLE_SOFTKEY,
    /* Softkey button , CNcomment: 软键盘按钮，与普通按钮类似 */
    BUTTON_STYLE_SWITCH,
    /* Switch button , CNcomment: 开关按钮 */
    BUTTON_STYLE_TOGGLE,
    /* Toggle button , CNcomment: 纽扣开关按钮 */
    BUTTON_STYLE_BUTT
} HIGV_BUTTON_SYLE_E;

/* *Button status , CNcomment: button状态 */
typedef enum {
    BUTTON_STATUS_UNCHECKED = 0,
    /* Unchecked status , CNcomment:未选中状态 */
    BUTTON_STATUS_CHECKED,
    /* Checked status , CNcomment: 选中状态 */
    BUTTON_STATUS_BUTT
} HIGV_BUTTON_STATUS_E;

/* Button switch status , CNcomment: button开关状态 */
typedef enum {
    BUTTON_SWITCH_OFF = 0,
    /* Switch off , CNcomment: 关闭开关 */
    BUTTON_SWITCH_ON,
    /* Switch on , CNcomment: 打开开关 */
    BUTTON_SWITCH_BUTT
} HIGV_BUTTON_SWITCH_E;

/* Switch button text layout , CNcomment: switch风格文字布局 */
typedef enum {
    BUTTON_SWITCHLAYOUT_OFF_ON = 0,
    /* "off:on" layout , CNcomment: off在左边on在右边的文本布局 */
    BUTTON_SWITCHLAYOUT_ON_OFF,
    /* "on:off" layout , CNcomment: on在左边off在右边的文本布局 */
    BUTTON_SWITCHLAYOUT_BUTT
} HIGV_BUTTON_SWITCHLAYOUT_E;
/* -- Structure Definition end */
/* ****************************** API declaration **************************** */
/* * addtogroup      Button    */
/* * 【Button wdiget】; CNcomment: 【Button控件】 */
/**
* brief Set checked status.CNcomment:设置Button的选中状态
* param[in] hButton Button handle.CNcomment:Button句柄
* param[in] eStatus  Checked status.CNcomment:Button选中状态
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Button_SetCheck(HIGV_HANDLE hButton, HIGV_BUTTON_STATUS_E eStatus);

/**
* brief Get checked status.CNcomment:获取Button的选中状态
* param[in] hButton Button handle.CNcomment:Button句柄
* param[out] peStatus  Checked status.CNcomment:Button选中状态
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Button_GetCheck(HIGV_HANDLE hButton, HIGV_BUTTON_STATUS_E *peStatus);

/**
* brief Set switch status of button.CNcomment:设置Button的开关状态
* param[in] hButton Button handle.CNcomment:Button句柄
* param[in] eSwitch  CNcomment:Button选中状态
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Button_SetSwitch(HIGV_HANDLE hButton, HIGV_BUTTON_SWITCH_E eSwitch);

/**
* brief Get the status of button.CNcomment:获取Button的开关状态
* param[in] hButton Button handle.CNcomment:Button句柄
* param[out] peSwitch CNcomment: Button选中状态
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Button_GetSwitch(HIGV_HANDLE hButton, HIGV_BUTTON_SWITCH_E *peSwitch);

/**
* brief Set the color for toggle.CNcomment:设置toggle颜色
* param[in] hButton Button handle.CNcomment:Button句柄
* param[in] ToggleOnColor BUTTON_SWITCH_ON status color.CNcomment:打开状态toggle颜色值
* param[in] ToggleOffColor BUTTON_SWITCH_OFF status color.CNcomment:关闭状态toggle颜色值
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Button_SetToggleColor(HIGV_HANDLE hButton, HI_COLOR ToggleOnColor, HI_COLOR ToggleOffColor);

/**
* brief Set text by language ID for switch/toggle Button.CNcomment:设置switch/toggle风格按钮的显示多语言文本ID
* param[in] hButton Button handle.CNcomment:Button句柄
* param[in] OnStrID BUTTON_SWITCH_ON status text ID.CNcomment:打开状态多语言字串ID
* param[in] OffStrID BUTTON_SWITCH_ON status textID.CNcomment:关闭状态多语言字串ID
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Button_SetSwitchTextByID(HIGV_HANDLE hButton, const HI_U32 OnStrID, const HI_U32 OffStrID);

/**
* brief Set switch button text layout.CNcomment:设置switch风格按钮的文本布局
* param[in] hButton Button handle.CNcomment:Button句柄
* param[in] TextLayout text layout.CNcomment:布局方向
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Button_SetSwitchLayout(HIGV_HANDLE hButton, HIGV_BUTTON_SWITCHLAYOUT_E TextLayout);

/* * ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif
#endif /* * __HI_GV_BUTTON_H__ */
