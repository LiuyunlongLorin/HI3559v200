/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: ScrollView widget API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_SCROLLVIEW_H__
#define __HI_GV_SCROLLVIEW_H__

/* add include here */
#include "hi_gv_widget.h"
#ifdef HIGV_USE_WIDGET_SCROLLVIEW
#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * addtogroup      scroll view  */
/* * 【ScrollView widget】; CNcomment:【ScrollView控件】 */
/* *Scroll direction; CNcomment: ScrollView控件的滚动方向枚举 */
/* *NOTE: It is only valid for touch operation or mouse wheel operation */
typedef enum _SCROLLVIEW_DIRECTION_E {
    SCROLLVIEW_DIRECTION_HORI = 0,  /*Horizal direction ; CNcomment: 水平方向延伸 */
    SCROLLVIEW_DIRECTION_VERT,  /*Vertial direction ; CNcomment:垂直方向延伸 */
    SCROLLVIEW_DIRECTION_BUTT
} HIGV_SCROLLVIEW_DIRECTION_E;

/* *Content box style */
typedef enum {
    SCROLLVIEW_STYLE_DEFAULTE = 0x00,
    SCROLLVIEW_STYLE_ALWAY_CENTER = 0x01,
    SCROLLVIEW_STYLE_BUTT = 0x10000
} HIGV_SCROLLVIEW_STYLE_E;

typedef struct hiHIGV_SCROLLVIEW_Init_S {
    HIGV_HANDLE
    hVerticalScrollBar; /* Bind vertical scrollbar handle; CNcomment: 绑定的垂直滚动条句柄 */
    HIGV_HANDLE
    hHorizontalScrollbar; /* Bind horizontal scrollbar handle ; CNcomment: 绑定的水平滚动条句柄 */
    HI_U32 LeftMargin; /* Invalid value ; CNcomment: 当前版本无效 */
    HI_U32 RightMargin; /* Child widget right margin of background ; CNcomment: 孩子控件在背景的右边距 */
    HI_U32 TopMargin; /* Invalid value ; CNcomment: 当前版本无效 */
    HI_U32 BottomMargin; /* Child widget bottom margin of background ; CNcomment: 孩子控件在背景的底边距 */
    HI_U32 Step; /* Scroll step ; CNcomment: 滚动步长 */
    HI_U32 Interval; /* Scroll interval ; CNcomment: 滚动间隔 */
    HI_U32 ScrollContentWidth; /* The scroll content invariable width ; CNcomment: 滚动内容的固定宽度 */
    HI_U32 ScrollContentHeight; /* The scroll content invariable height ; CNcomment: 滚动内容的固定高度 */
    HI_U32 ScrollViewStyle; /* Scroll view private style ; CNcomment: 私有风格 */
    HIGV_SCROLLVIEW_DIRECTION_E Direction; /* Scroll direction ; CNcomment: 滚动方向 */
} HIGV_SCROLLVIEW_Init_S;
/* *  -- Macro Definition end */
/* ****************************** API declaration **************************** */
/* *  addtogroup      scroll view */
/* *  【ScrollView wiget】   【ScrollView控件】 */
/**
* brief Init the scrollview.CNcomment:初始化scrollview控件
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* param[in]  pStyle The info of scrollview when created. CNcomment:控件创建信息
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_EXIST
*/
HI_S32 HI_GV_ScrollView_Init(HIGV_HANDLE hScrollView, HIGV_SCROLLVIEW_Init_S *pInitAttr);

/**
* brief Set content invariable size.CNcomment:设置内容的固定大小
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* param[in]  Width The invariable width. CNcomment:固定宽度，范围需大于等于控件的宽度
* param[in]  Height The invariable height. CNcomment:固定高度，范围需大于等于控件的高度
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM
*/
HI_S32 HI_GV_ScrollView_SetContentSize(HIGV_HANDLE hScrollView, HI_U32 Width, HI_U32 Height);

/**
* brief Move the view to center.CNcomment:将视口移至中央
* param[in]  hScrollView scroll view handle. CNcomment:控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_FAILURE
*/
HI_S32 HI_GV_ScrollView_MoveToCenter(HIGV_HANDLE hScrollView);

/**
* brief Get the content rectangle.CNcomment:获取内容的矩形
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* param[out] pRect Rectangle. CNcomment:矩形输出参数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ScrollView_GetContentRect(HIGV_HANDLE hScrollView, HI_RECT *pRect);

/**
* brief Get the view coordinate in the background content.CNcomment:获取视口相对内容的坐标
* attention \n
     _____________________
    |  background content |
    |        ____         |
    |       |view|        |
    |       |____|        |
    |_____________________|
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* param[out] px X abscissa. CNcomment:x坐标
* param[out] py Y abscissa. CNcomment:y坐标
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ScrollView_GetViewCoordinate(HIGV_HANDLE hScrollView, HIGV_CORD *px, HIGV_CORD *py);

/**
* brief Set the view coordinate in the background content.CNcomment:设置视口相对内容的坐标
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* param[out] px X abscissa. CNcomment:x坐标
* param[out] py Y abscissa. CNcomment:y坐标
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ScrollView_SetViewCoordinate(HIGV_HANDLE hScrollView, HIGV_CORD x, HIGV_CORD y);

/**
* brief Set scroll step.CNcomment:设置滚动步长
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* param[in]  Step Scroll step.CNcomment:滚动步长(以pixel为单位，默认为20pixel)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ScrollView_SetStep(HIGV_HANDLE hScrollView, HI_U32 Step);

/**
* brief Set scroll Interval.CNcomment:设置滚动间隔
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* param[in]  Interval Interval. CNcomment:间隔
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ScrollView_SetInterval(HIGV_HANDLE hScrollView, HI_U32 Interval);

/**
* brief Bind the scrollbar to scrollbar.CNcomment:绑定滚动条
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* param[in]  hVertical vertical scrollbar. CNcomment:垂直滚动条
* param[in]  hHorizontal horizontal scrollbar. CNcomment:水平滚动条
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ScrollView_BindScrollBar(HIGV_HANDLE hScrollView, HIGV_HANDLE hVertical, HIGV_HANDLE hHorizontal);

/**
* brief Check the focus widget in the view.CNcomment:检查确保焦点控件在视口内
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ScrollView_CheckFocusPos(HIGV_HANDLE hScrollView);

/**
* brief Set scroll sensibility.CNcomment: 设置滑动操作的灵敏度系数，默认是1.0
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* param[in]  ScrollParam scroll sensibility param. CNcomment:滑动灵敏度系数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ScrollView_SetScrollParam(HIGV_HANDLE hScrollView, HI_FLOAT ScrollParam);

/**
* brief Set fling sensibility.CNcomment:设置轻扫操作的灵敏度系数，默认是1.0
* param[in]  hScrollView Scroll view handle. CNcomment:控件句柄
* param[in]  FlingParam fling sensibility param. CNcomment:轻扫灵敏度系数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ScrollView_SetFlingParam(HIGV_HANDLE hScrollView, HI_FLOAT FlingParam);

/* API declaration end */
#ifdef __cplusplus
}
#endif
#endif
#endif /* __HI_GV_SCROLLVIEW_H__ */
