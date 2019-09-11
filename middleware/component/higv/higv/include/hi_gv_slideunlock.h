/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: slideunlock widget API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_SLIDEUNLOCK_H__
#define __HI_GV_SLIDEUNLOCK_H__

#include "hi_gv_conf.h"
#ifdef HIGV_USE_WIDGET_SLIDEUNLOCK
/* add include here */
#include "hi_type.h"
#include "hi_go.h"
#include "hi_gv_resm.h"

#ifdef __cplusplus
extern "C" {
#endif
/* ************************** Structure Definition *************************** */
/* * addtogroup      SlideUnlock  */
/* * 【SlideUnlock widget】 ; CNcomment: 【SlideUnlock控件】 */

#define SLIDEUNLOCK_LENGTH  40  /* <length of slideunlock ; CNcomment: 滑槽长度，相对值 */
#define SLIDEUNLOCK_DUATION 250 /* <kickback duation ; CNcomment: 回弹时间 */

typedef enum HI_SLIDEUNLOCK_STYLE {
    SLIDEUNLOCK_H = 0,  /* <Horizontal ; <CNcomment: 水平 */
    SLIDEUNLOCK_V,   /* <Vertical ; <CNcomment: 竖直 */
    SLIDEUNLOCK_BUT
} SLIDEUNLOCK_STYLE;

typedef enum hiHIGV_SLIDEUNLOCK_E { /* * SlideUnlock type ; CNcomment:  滑块控件种类 */
    HIGV_SLIDEUNLOCK_NORMAL = 0,
    HIGV_SLIDEUNLOCK_TOUCH,
    HIGV_SLIDEUNLOCK_DONE,
    HIGV_SLIDEUNLOCK_BUTT
} HIGV_SLIDEUNLOCK_E;

#define HIGV_SLIDEUNLOCK_H HIGV_STYLE_DEFINE(HIGV_WIDGET_SLIDEUNLOCK, SLIDEUNLOCK_H)
#define HIGV_SLIDEUNLOCK_V HIGV_STYLE_DEFINE(HIGV_WIDGET_SLIDEUNLOCK, SLIDEUNLOCK_V)

/* * Structure Definition end */
/* ****************************** API declaration **************************** */
/* * addtogroup      SlideUnlock  */
/* *  【SlideUnlock widget】; CNcomment: 【SlideUnlock控件】 */
/**
* brief Set skin of trackbar. CNcomment:设置SlideUnlock滑杆的皮肤
* param[in] hSlideUnlock SlideUnlock handle.CNcomment:hSlideUnlock句柄
* param[in] hRes    Picture resource ID.CNcomment:hSlideUnlock图片资源ID
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_SlideUnlock_SetSliImage(HIGV_HANDLE hSlideUnlock, HI_RESID hRes);

/**
* brief Set the skin of slideblock.CNcomment:设置SlideUnlock滑块的皮肤
* param[in] hSlideUnlock    SlideUnlock handle.CNcomment:hSlideUnlock句柄
* param[in] hSkinIndex  Skin index.CNcomment:hSlideUnlock皮肤状态
* param[in] hRes   Picture resource ID of hSlideUnlock.CNcomment:hSlideUnlock图片资源ID
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_SlideUnlock_SetTraImage(HIGV_HANDLE hSlideUnlock, HI_U32 hSkinIndex, HI_RESID hRes);

/**
* brief Set status of slideunlock.CNcomment:设置SlideUnlock业务的使能状态
* param[in] hSlideUnlock    SlideUnlock handle.CNcomment:hSlideUnlock句柄
* param[in] bIsEnable    enable status.CNcomment:业务使能状态
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_SlideUnlock_SetStatus(HIGV_HANDLE hSlideUnlock, HI_BOOL bIsEnable);

/**
* brief get status of slideunlock.CNcomment:获取SlideUnlock业务的使能状态
* param[in] hSlideUnlock SlideUnlock handle.CNcomment:hSlideUnlock句柄
* param[out] pIsEnable enable status pointer.CNcomment:业务使能指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_SlideUnlock_GetStatus(HIGV_HANDLE hSlideUnlock, HI_U32 *pIsEnable);

/**
* brief back to init status.CNcomment:恢复到初始状态
* param[in] hSlideUnlock SlideUnlock handle.CNcomment:hSlideUnlock句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_SlideUnlock_ReSet(HIGV_HANDLE hSlideUnlock);

/**
* brief Register track widget.CNcomment:注册SlideUnlock控件
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_SlideUnlock_RegisterWidget(HI_VOID);

/* API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_SLIDEUNLOCK_H__ */
#endif
