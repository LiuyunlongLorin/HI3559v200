/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: Scrollbar widget API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_SCROOBAL_H__
#define __HI_GV_SCROOBAL_H__

/* add include here */
#include "hi_gv_widget.h"

#ifdef HIGV_USE_WIDGET_SCROLLBAR

#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * addtogroup      ScrollBar       */
/* * 【ScrollBar widget】; CNcomment:  【ScrollBar控件】 */
/* *CNcomment:  ScrollBar类型 */
typedef enum {
    SCROLLBAR_STYLE_VERTICAL,
    /* *<Vertical style ; CNcomment:  垂直风格ScrollBar */
    SCROLLBAR_STYLE_HORIZONTAL,
    /* *<Horizontal style ; CNcomment:  水平风格ScrollBar */
    SCROLLBAR_BUTT
} HIGV_SCROLLBAR_SYLE_E;

/* *Only suport vetical scrollbar ; CNcomment:  控件默认风格为垂直风格,目前暂时只支持垂直风格 */
/* * Vertical  scrollbar ; CNcomment:垂直风格ScrollBar */
#define HIGV_SCROLLBAR_STYLE_VERTICAL HIGV_STYLE_DEFINE(HIGV_WIDGET_SCROLLBAR, SCROLLBAR_STYLE_VERTICAL)
/* * CNcomment: Horizonal ScrollBar ; CNcomment: 水平风格ScrollBar */
#define HIGV_SCROLLBAR_STYLE_HORIZONTAL HIGV_STYLE_DEFINE(HIGV_WIDGET_SCROLLBAR, SCROLLBAR_STYLE_HORIZONTAL)

/* *   ==== Structure Definition end ==== */
/* ****************************** API declaration **************************** */
/* * addtogroup      ScrollBar       */
/* *  【ScrollBar widget】 ; 【ScrollBar控件】 */
/**
* brief Set scrollbar infomation. CNcomment: 设置滑快信息
* attention :The offset of slide block is dicided by the height of left & top corner picture.
* CNcomment: 滑块的初始偏移位置是由ScrollBar的背景皮肤的左上角ltopindex图片高度决定的
* param[in] hScrollBar Scrollbar handle.CNcomment: 滚动条句柄
* param[in] SlideRes Resource information.CNcomment: 资源信息
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* retval ::HI_ERR_COMM_INVAL CNcomment: 无效参数
*/
HI_S32 HI_GV_ScrollBar_SetSlideRes(HIGV_HANDLE hScrollBar, HI_RESID SlideRes);

/**
* brief Set the position of slide block. CNcomment: 设置滑快的位置
* param[in] hScrollBar Scrollbar handle.CNcomment: 滚动条句柄
* param[in] Pos  Slide block position.CNcomment: 滑块的位置，此属性可以随时设置生效设置，计数从0开始。
* retval ::HI_SUCCESS SuccessCNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* retval ::HI_ERR_COMM_INVAL CNcomment: 无效参数
* see ::HI_GV_ScrollBar_GetPos
*/
HI_S32 HI_GV_ScrollBar_SetPos(HIGV_HANDLE hScrollBar, HI_U32 Pos);

/**
* brief Get the position of slide block. CNcomment: 获取滑快的位置
* param[in]  hScrollBar Scrollbar handle.CNcomment: 滚动条句柄
* param[out] pPos Position pointer. CNcomment: 滑块的位置指针
* retval ::HI_SUCCESS SuccessCNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* retval ::HI_ERR_COMM_INVAL Invalid paramater.CNcomment: 无效参数
* see ::HI_GV_ScrollBar_SetPos
*/
HI_S32 HI_GV_ScrollBar_GetPos(HIGV_HANDLE hScrollBar, HI_U32 *pPos);

/**
* brief Set the range of slide block. CNcomment: 设置滚动条滚动范围
* param[in] hScrollBar  Scrollbar handle.CNcomment: 滚动条句柄
* param[in] ScrollRange The range of slide block, CNcomment: 滚动条滚动范围
* retval ::HI_SUCCESS SuccessCNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
*/
HI_S32 HI_GV_ScrollBar_SetScrollRange(HIGV_HANDLE hScrollBar, HI_U32 ScrollRange);

/**
* brief Set the length of visual area. CNcomment: 设置可见区域长度
* param[in] hScrollBar  Scrollbar handle.CNcomment: 滚动条句柄
* param[in] Len The length of visual area.CNcomment: 控件可见区域的长度
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
*/
HI_S32 HI_GV_ScrollBar_SetVisibleLen(HIGV_HANDLE hScrollBar, HI_U32 Len);

/**
* brief Set the length of content.CNcomment: 设置内容长度
* param[in] hScrollBar  Scrollbar handle.CNcomment: 滚动条句柄
* param[in] Len CNcomment: 控件有效内容长度，此属性设置后即时生效
* retval ::HI_SUCCESS SuccessCNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
*/
HI_S32 HI_GV_ScrollBar_SetContentLen(HIGV_HANDLE hScrollBar, HI_U32 Len);

/**
* brief Set the picture resouce of the button which is pressed or normal state.CNcomment:
* 设置滚动条上下按钮正常和按下状态图片 \attention \n This api is not support 9 pitchs style.
* The noraml state and pressed
* state will not effect at the same time. CNcomment:
* 正常状态和按下状态下的图片，不能同时无效。如果一个状态有效，
* 另一个状态无效，则无效的状态与有效的状态使用同一个图片
* 图片必须是一整张，不支持九宫格贴图方式
* param[in] hScrollBar  Scrollbar handle.CNcomment: 滚动条句柄
* param[in] UpNormalImg Up arrow which is normal state.CNcomment: 上按钮正常状态图片
* param[in] UpPressImg Up arrow which is pressed picture..CNcomment: 上按钮按下状态图片
* param[in] DownNormalImg Down arrow which is normal picture.CNcomment: 下按钮正常状态图片
* param[in] DownPressImg Down arrow which is pressed picture. CNcomment: 下按钮按下状态图片
* retval ::HI_SUCCESS SuccessCNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
*/
HI_S32 HI_GV_ScrollBar_SetButtonImg(HIGV_HANDLE hScrollBar, HI_RESID UpNormalImg, HI_RESID UpPressImg,
                                    HI_RESID DownNormalImg, HI_RESID DownPressImg);

/**
* brief No paint  scrollbar when suspend on the container widget  and content length
* not above container widget length.
* CNcomment: 当前scrollbar为悬浮状态并且绑定的控件的内容的长度小于等于控件的长度时，不绘制scrollbar控件
* param[in] hScrollBar Scrollbar handle.CNcomment: 滚动条句柄
* param[in] bIsSuspending  Is suspending status.CNcomment: 滑块状态是否悬浮
* retval ::HI_SUCCESS SuccessCNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* retval ::HI_ERR_COMM_INVAL CNcomment: 无效参数
*/
HI_S32 HI_GV_ScrollBar_SetStatus(HIGV_HANDLE hScrollBar, HI_BOOL bIsSuspending);

/* * API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif
#endif /* __HI_GV_SCROOBAL_H__ */
