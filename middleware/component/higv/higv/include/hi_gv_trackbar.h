/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: trackbar widget API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_TRACKBAR_H__
#define __HI_GV_TRACKBAR_H__

#include "hi_gv_conf.h"
#ifdef HIGV_USE_WIDGET_TRACKBAR
/* add include here */
#include "hi_type.h"
#include "hi_go.h"
#include "hi_gv_resm.h"

#ifdef __cplusplus
extern "C" {
#endif
/* ************************** Structure Definition *************************** */
/* * addtogroup      TrackBar  */
/* * 【TrackBar widget】 ; CNcomment: 【TrackBar控件】 */

typedef enum HI_TRACKBAR_STYLE {
    TRACKBAR_H = 0,  /* <Horizontal ; <CNcomment: 水平 */
    TRACKBAR_V,  /* <Vertical ; <CNcomment: 竖直 */
    TRACKBAR_BUT
} TRACKBAR_STYLE;

typedef enum hiHIGV_TRACKBAR_E {    /* Trackbar type ; CNcomment:  滑块控件种类 */
    HIGV_TRACKBAR_NORMAL = 0,
    HIGV_TRACKBAR_ACTIVE,
    HIGV_TRACKBAR_MOUSEDOWN,
    HIGV_TRACKBAR_BUTT
} HIGV_TRACKBAR_E;

#define HIGV_TRACKBAR_H HIGV_STYLE_DEFINE(HIGV_WIDGET_TRACKBAR, TRACKBAR_H)
#define HIGV_TRACKBAR_V HIGV_STYLE_DEFINE(HIGV_WIDGET_TRACKBAR, TRACKBAR_V)

/* *  Structure Definition end */
/* ****************************** API declaration **************************** */
/* * addtogroup      TrackBar  */
/* *  【TrackBar widget】 ; CNcomment: 【TrackBar控件】 */
/**
* brief Set skin of trackbar. CNcomment:设置TrackBar滑杆的皮肤
* param[in] hTrack Trackbar handle.CNcomment:Track句柄
* param[in] hSkinIndex  Skin index.CNcomment:Track皮肤状态
* param[in] hRes    Picture resource ID.CNcomment:Track图片资源ID
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Track_SetSliImage(HIGV_HANDLE hTrack, HI_U32 hSkinIndex, HI_RESID hRes);

/**
* brief Set the skin of slideblock.CNcomment:设置TrackBar滑块的皮肤
* param[in] hTrack    Trackbar handle.CNcomment:Track句柄
* param[in] hSkinIndex  Skin index.CNcomment:Track皮肤状态
* param[in] hRes   Picture resource ID of track bar.CNcomment:Track图片资源ID
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Track_SetTraImage(HIGV_HANDLE hTrack, HI_U32 hSkinIndex, HI_RESID hRes);

/**
* brief Set the value of trackbar.CNcomment:设置TrackBar当前值
* param[in] hTrack    Trackbar handle.   CNcomment:TrackBar句柄
* param[in] value     Get current valude of cursor.  CNcomment:得到游标的当前值
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Track_SetCurVal(HIGV_HANDLE hTrack, HI_U32 Value);

/**
* brief Get current graduation of cursror.CNcomment:得到游标当前刻度
* param[in] hTrack      Trackbar handle.CNcomment: TrackBar句柄
* param[out] pValue     Get value of current cursor. CNcomment:得到游标的当前值
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Track_GetCurVal(HIGV_HANDLE hTrack, HI_U32 *pValue);

/**
* brief Set the range of cursor. CNcomment:设定游标的范围值
* param[in] hTrack     Trackbar handle.CNcomment:Track句柄
* param[in] MaxnValue Max value of cursor.CNcomment: 游标最大值
* param[in] MinValue  Min value of cursor.CNcomment: 游标最小值
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Track_SetRange(HIGV_HANDLE hTrack, HI_U32 MinValue, HI_U32 MaxValue);

/**
* brief Get the range of cursor.CNcomment:获取游标的范围
* param[in]  hTrack     Trackbar handle.CNcomment:Track句柄
* param[out] pMaxValue  Max value of cursor.CNcomment:得到游标最大值
* param[out] pMinValue  Min value of cursor.CNcomment:得到游标最小值
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Track_GetRange(HIGV_HANDLE hTrack, HI_U32 *pMinValue, HI_U32 *pMaxValue);

/**
* brief Set gesture enable.CNcomment:设置控件是否支持触摸
* param[in]  hTrack  Scrollgrid handle. CNcomment:控件句柄
* param[in]  bIsEnable   Is enable gesture. CNcomment:是否支持触摸( HI_TRUE表示支持，HI_FALSE表示禁止)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Track_EnableGesture(HIGV_HANDLE hTrack, HI_BOOL bIsEnable);

/**
* brief Get gesture enable.CNcomment:获取控件是否支持触摸
* param[in]  hTrack  Scrollgrid handle. CNcomment:控件句柄
* param[in]  pIsEnable   Is enable gesture. CNcomment:是否支持触摸( HI_TRUE表示支持，HI_FALSE表示禁止，系统默认为HI_TRUE)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Track_IsGestureEnable(HIGV_HANDLE hTrack, HI_BOOL *pIsEnable);

/**
* brief Register track widget.CNcomment:注册track控件
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Track_RegisterWidget(HI_VOID);

/* ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_TRACKBAR_H__ */
#endif
