/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: Window widget API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_WIN_H__
#define __HI_GV_WIN_H__

/* add include here */
#include "hi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ****************************** API declaration **************************** */
/* * addtogroup      Window   */
/* *  【Window widget】 CNcomment: 【窗口控件】 */

/**
* brief Set window opacity.CNcomment:设置窗口可见度
* param[in] hWindow Window handle.CNcomment:窗口句柄
* param[in] Opacity Window opacity.CNcomment:窗口可见度
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Win_SetOpacity(HIGV_HANDLE hWindow, HI_U8 Opacity);

/**
* brief Get first highlight child widget of widnow CNcomment:获取窗口第一个高亮的孩子句柄
* attention :This api is uded to get the hightlight widget for softkeyboard.
* CNcomment:主要是应用在软键盘上获取当前高亮控件
* param[in] hWindow Window handle.CNcomment:窗口句柄
* param[out] phHilghtChild Hightlight child.CNcomment:高亮孩子
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Win_GetHilightChild(HIGV_HANDLE hWindow, HIGV_HANDLE *phHilghtChild);

/**
* brief Reset all hightlight child and hightlight the first child.CNcomment:复位窗口中的高亮孩子，将窗口第一个子控件高亮
* attention:This API is used to redisplay the softkeyboard.
* CNcomment:主要是应用重新显示软键盘时复位软键盘
* param[in] hWindow Window handle.CNcomment:窗口句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Win_ResetHilightChild(HIGV_HANDLE hWindow);

/**
* brief Sync show win and get widget handle of done this show.CNcomment:同步显示window并获取结束显示的控件句柄
* attention:Return the second arg of HI_GV_Win_EndSyncShow.CNcomment:返回HI_GV_Win_EndSyncShow接口的第二个参数
* param[in] hWindow Window handle.CNcomment:窗口句柄
* retval ::HIGV_HANDLE
* retval ::HI_FAILURE
* see : HI_GV_Win_EndSyncShow
*/
HI_S32 HI_GV_Win_SyncShow(HIGV_HANDLE hWindow);

/**
* brief Done the sync show.CNcomment:结束窗口同步显示
* param[in] hWindow Window handle.CNcomment:窗口句柄
* param[in] hWidget Widget handle of end window sync show .CNcomment:结束同步显示时返回的控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_NOOP
* see : HI_GV_Win_SyncShow
*/
HI_S32 HI_GV_Win_EndSyncShow(HIGV_HANDLE hWindow, HIGV_HANDLE hWidget);

/**
* brief Get window level.CNcomment:获取窗口等级
* param[in] hWindow Window handle.CNcomment:窗口句柄
* param[out] pLevel Window level.CNcomment:传出的窗口等级
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Win_GetWinLevel(HIGV_HANDLE hWindow, HI_U32 *pLevel);

/**
* brief Advance window and child to load skin.CNcomment:加载窗口及其子控件的的皮肤
* attention : HIGV_STYLE_HIDE_UNLODRES style .CNcomment:当隐藏释放资源风格时有效
* param[in] hWindow Window handle.CNcomment:窗口句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Win_LoadSkin(HIGV_HANDLE hWindow);
/* *  ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_WIN_H__ */
