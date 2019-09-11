/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: APIWindow manageer moudle API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_WM_H__
#define __HI_GV_WM_H__

/* add include here */
#include "hi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * addtogroup      WindowManager  */
/* *  【Window manager 】; 【窗口管理】 */
/* * Stack capacity ; CNcomment:窗口栈容量 */
#define HIGV_MAX_WINNUM 50

/* * ==== Structure Definition end ==== */
/* ****************************** API declaration **************************** */
/* * addtogroup      WindowManager  */
/* * 【Window manager 】 ; CNcomment: 【窗口管理】 */
/**
* brief Push window to statck.CNcomment:将窗口入栈
* param[in] hLayer  Layer handle. CNcomment:图层句柄
* param[in] hWindow Window handle. CNcomment:窗口句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_FULL Stack full.CNcomment:栈满
* see ::HI_GV_WM_PopWindow
*/
HI_S32 HI_GV_WM_PushWindow(HIGV_HANDLE hLayer, HIGV_HANDLE hWindow);

/**
* brief Pop up window from stack.CNcomment:将窗口出栈
* param[in] hLayer  Layer handle. CNcomment:图层句柄
* param[in] hWindow Window handle.CNcomment:窗口句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_EMPTY Stack empty.CNcomment:栈空
* see ::HI_GV_WM_PushWindow
*/
HI_S32 HI_GV_WM_PopWindow(HIGV_HANDLE hLayer, HIGV_HANDLE hWindow);

/**
* brief Get top window of stack. CNcomment:获取栈顶窗口
* param[in] hLayer  Layer handle. CNcomment:图层句柄
* param[out] pWindow Window handle. CNcomment: 窗口句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_EMPTY Stack empty.CNcomment:栈空
*/
HI_S32 HI_GV_WM_GetTopWindow(HIGV_HANDLE hLayer, HIGV_HANDLE *phWindow);

/**
* brief Clear window of the stack.CNcomment:清除栈中窗口
* param[in] hLayer  Layer handle. CNcomment:图层句柄
* retval ::HI_SUCCESS
*/
HI_S32 HI_GV_WM_ClearAllWindow(HIGV_HANDLE hLayer);

/**
* brief Get capture mouse window. CNcomment:获得鼠标捕获窗口
* param[in]  hLayer    Layer handle. CNcomment:图层句柄
* param[out] phWidget  The window which capture mouse. CNcomment:捕获鼠标的窗口
* retval ::HI_SUCCESS
*/
HI_S32 HI_GV_WM_GetMouseCapture(HIGV_HANDLE hLayer, HIGV_HANDLE *phWidget);

/**
* brief Set the capture mouse window.CNcomment:设置鼠标捕获窗口
* param[in] hLayer  Layer handle.CNcomment:图层句柄
* param[in] hWidget Set the window which capture mouse.CNcomment:设置鼠标捕获窗口
* retval ::HI_SUCCESS
N/A.CNcomment:无
*/
HI_S32 HI_GV_WM_SetMouseCapture(HIGV_HANDLE hLayer, HIGV_HANDLE hWidget);

/**
* brief Get the higo layer handle correspoding layer.CNcomment:获取图层对应的HIGO图层句柄
* attention : CNcomment:部分对图层的操作需要用底层的higo图层接口
* param[in] hLayer     Layer handle.CNcomment:图层句柄
* param[out] pHigoLayer Higo layer handle. CNcomment:HIGO图层句柄
* retval ::HI_SUCCESS
*/
HI_S32 HI_GV_WM_GetHigoLayer(HIGV_HANDLE hLayer, HIGV_HANDLE *pHigoLayer);

/**
* brief Touch msg bind with widget. CNcomment:触摸消息和控件绑定
* param[in] hLayer  Layer handle.CNcomment:图层句柄
* param[in] bIsBind Bind tag, HI_TRUE or HI_FALSE.CNcomment:是否绑定控件
* retval ::HI_SUCCESS
*/
HI_S32 HI_GV_WM_BindTouchMsg(HIGV_HANDLE hLayer, HI_BOOL bIsBind);

/**
* brief Get bind status.CNcomment:获取绑定状态
* param[in] hLayer  Layer handle.CNcomment:图层句柄
* param[in] pIsBind Bind status .CNcomment:绑定状态，HI_TRUE表示已绑定，HI_FALSE表示未绑定
* retval ::HI_SUCCESS
*/
HI_S32 HI_GV_WM_GetBindTouchStatus(HIGV_HANDLE hLayer, HI_U32 *pIsBind);

/* * ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_WM_H__ */
