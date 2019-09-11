/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: hi_gv_wheelview.h
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_WHEELVIEW_H__
#define __HI_GV_WHEELVIEW_H__
#include "hi_gv_conf.h"
#include "hi_gv.h"

#ifdef HIGV_USE_WIDGET_WHEELVIEW
/* add include here */
#include "hi_gv_resm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * addtogroup      Wheelview     */
/* * 【Wheelview widget】 ; CNcomment:  【wheelview控件】 */
typedef struct hi_HIGV_WHEELVIEW_STYLE_S {
    HI_U32 RowNum; /* The count of display item ; CNcomment: 屏幕上显示的行数 */
    HIGV_HANDLE DataModel; /* Data Model Handle ; CNcomment:  数据模型句柄 */
    HIGV_HANDLE UpCoverSkin; /* Up Cover Skin ; CNcomment: 上覆盖皮肤句柄 */
    HIGV_HANDLE DownCoverSkin; /* Down Cover Skin; CNcomment: 下覆盖皮肤句柄 */
    HI_BOOL IsCircleScroll; /*Circle tag ; CNcomment: 循环滚动标志 */
    HI_FLOAT ScrollParam; /* Scroll Param ; CNcomment: 滑动灵敏度系数 */
    HI_FLOAT FlingParam; /* Fling Param ; CNcomment: 轻扫灵敏度系数 */
    HI_U32 LeftMargin; /* Left margin ; CNcomment:  左边距 */
    HI_U32 RightMargin; /* Right margin ; CNcomment:  右边距 */
    HI_U32 TopMargin; /* Up margin ; CNcomment:  上边距 */
    HI_U32 BtmMargin; /* Bottom margin ; CNcomment:  下边距 */
} HIGV_WHEELVIEW_STYLE_S;

/* *  -- Structure Definition end */
/* ****************************** API declaration **************************** */
/* * addtogroup      Wheelview     */
/* *  【wheelview widget】  CNcomment:  【wheelview控件】 */
/**
* brief Create wheelview.CNcomment:WheelView控件初始化
* param[in] pCreatInfo    The Layout infomation of wheelview. CNcomment:控件风格布局属性指针
* param[out] phWheelView   the pointer of wheelview handle. CNcomment:WheelView控件句柄指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_INVA
* retval ::HI_ERR_COMM_NORES
*/
HI_S32 HI_GV_WheelView_Create(const HIGV_WCREATE_S *pCreatInfo, HIGV_HANDLE *phWheelView);

/**
* brief Set selected item of  wheelview. CNcomment: 设置 WheelView当前条目
* param[in] hWheelview WheelView handle. CNcomment: 控件句柄
* param[in] Item  Item index.CNcomment: 条目编号
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see ::HI_GV_WheelView_GetSelItem
*/
HI_S32 HI_GV_WheelView_SetSelItem(HIGV_HANDLE hWheelview, HI_U32 Item);

/**
* brief Get the item of WheelView.CNcomment: 获取WheelView当前条目
* param[in] hWheelview  WheelView handle.CNcomment: 控件句柄
* param[out] pItem Item index.CNcomment: 条目编号
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see ::HI_GV_WheelView_SetSelItem
*/
HI_S32 HI_GV_WheelView_GetSelItem(HIGV_HANDLE hWheelview, HI_U32 *pItem);

/**
* brief Get the total item number of WheelView.CNcomment: 获取WheelView当前数据总条数
* param[in]  hWheelview  WheelView handle. CNcomment:  控件句柄
* param[out] pItemNum Toal item number.CNcomment: 总条目数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_WheelView_GetItemNum(HIGV_HANDLE hWheelview, HI_U32 *pItemNum);

/**
* brief Set up image resource.CNcomment:设置上遮挡皮肤资源
* param[in] hWheelview Image handle.CNcomment:控件句柄
* param[in] hRes The resource ID of image.CNcomment:图片资源句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_WheelView_SetUpImage(HIGV_HANDLE hWheelview, HI_RESID hRes);

/**
* brief Set up image resource.CNcomment:设置下遮挡皮肤资源
* param[in] hWheelview Image handle.CNcomment:控件句柄
* param[in] hRes The resource ID of image.CNcomment:图片资源句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_WheelView_SetDownImage(HIGV_HANDLE hWheelview, HI_RESID hRes);

/**
* brief Enable wheelview touch.CNcomment:设置允许触摸功能
* param[in] hWheelview Image handle.CNcomment:控件句柄
* param[in] bEnable  Enable flag.CNcomment: 使能标志，HI_TRUE表示使能，HI_FALSE表示去使能
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_WheelView_Enable(HIGV_HANDLE hWheelview, HI_BOOL bEnable);

/**
* brief Is touch enable.CNcomment:获取是否允许触摸状态
* param[in] hWheelview Image handle.CNcomment:控件句柄
* retval ::HI_TRUE  Touch is Enable.CNcomment: 触摸可用
* retval ::HI_FALSE Touch is Disable.CNcomment: 触摸禁用
*/
HI_BOOL HI_GV_WheelView_IsEnable(HIGV_HANDLE hWheelview);

/**
* rief Set scroll param.CNcomment:设置滑动灵敏度系数(默认为1.0，大于等于1.0)
* param[in] hWheelview wheelview handle.    CNcomment:控件句柄
* param[in] ScrollParam scroll index.    CNcomment:滑动灵敏度系数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_WheelView_SetScrollParam(HIGV_HANDLE hWheelview, HI_FLOAT ScrollParam);

/**
* brief Set fling param.CNcomment:设置轻扫灵敏度系数(默认为1.0，大于等于1.0)
* param[in] hWheelview wheelview handle.    CNcomment:控件句柄
* param[in] FlingParam fling index.    CNcomment:轻扫灵敏度系数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_WheelView_SetFlingParam(HIGV_HANDLE hWheelview, HI_FLOAT FlingParam);

/*  API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif
#endif /* __HI_GV_WHEELVIEW_H__ */
