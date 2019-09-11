/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: ProgressBar widget API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_PROGRESSBAR_H__
#define __HI_GV_PROGRESSBAR_H__

/* add include here */
#include "hi_go_comm.h"
#include "hi_gv_conf.h"

#ifdef HIGV_USE_WIDGET_PROGRESSBAR
#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * addtogroup      ProgressBar   */
/* *  【ProgressBar widget】; CNcomment: 【ProgressBar控件】 */
/* *the macro of horizontal style ; CNcomment: 横向进度条 */
#define PROGRESSBAR_STYLE_HORIZON 0x1

/* *the macro of vertical style ; CNcomment: 竖向进度条 */
#define PROGRESSBAR_STYLE_VERTICAL 0x2

/* *Horizontal style ; CNcomment: 横向进度条风格 */
#define HIGV_PROGRESSBAR_STYLE_HORIZON HIGV_STYLE_DEFINE(HIGV_WIDGET_PROGRESS, PROGRESSBAR_STYLE_HORIZON)

/* *Vertical style ; CNcomment: 竖向进度条风格 */
#define HIGV_PROGRESSBAR_STYLE_VERTICAL HIGV_STYLE_DEFINE(HIGV_WIDGET_PROGRESS, PROGRESSBAR_STYLE_VERTICAL)

/* *  == Structure Definition end ==== */
/* ****************************** API declaration **************************** */
/* * addtogroup      ProgressBar         */
/* * 【ProgressBar widget】;p CNcomment: 【ProgressBar控件】 */
/**
* brief Set range of progressbar.CNcomment:设置进度条范围
* param[in] hPgBar Progressbar handle.CNcomment:进度条句柄
* param[in] Min    Min valude.CNcomment:最小值
* param[in] Max  Max value. CNcomment: 最大值
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ProgressBar_SetRange(HIGV_HANDLE hPgBar, HI_U32 Min, HI_U32 Max);

/**
* brief Set step.CNcomment:设置步长
* param[in] hPgBar Progressbar handle.CNcomment:进度条句柄
* param[in] Step   Step.CNcomment:步长
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ProgressBar_SetStep(HIGV_HANDLE hPgBar, HI_U32 Step);

/**
* brief Set the posithion of progressbar.CNcomment:设置进度条当前位置
* attention :If set pos out of the range, value will be min or max, return HI_SUCCESS.
* CNcomment:如果值超出范围，将被设置为最大值或最小值，并返回成功
* param[in] hPgBar Progressbar handle.CNcomment:进度条句柄
* param[in] Pos    CNcomment:位置
* retval ::HI_SUCCESS
* \retval ::HI_ERR_COMM_LOST
* see :: HI_GV_ProgressBar_GetPos
*/
HI_S32 HI_GV_ProgressBar_SetPos(HIGV_HANDLE hPgBar, HI_U32 Pos);

/**
* brief Get the posithion of progressbar.CNcomment:获取进度条当前位置
* param[in] hPgBar Progressbar handle.CNcomment:进度条句柄
* param[out] pPos  Position.CNcomment:位置
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see ::HI_GV_ProgressBar_SetPos
*/
HI_S32 HI_GV_ProgressBar_GetPos(HIGV_HANDLE hPgBar, HI_U32 *pPos);

/**
* brief Set the freground of progressbar.CNcomment:设置进度条的前景风格
* param[in] hPgBar  Progressbar handle.CNcomment:进度条句柄
* param[in] pFgRect The range of progress.CNcomment:进度条前景显示的最大范围(相对进度条位置)
* 为空表示前景范围与整个进度条大小相同 \param[in] FgStyle   Fregroud style handle.CNcomment:风格句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_ProgressBar_SetFg(HIGV_HANDLE hPgBar, HI_RECT *pFgRect, HI_RESID FgStyle);

/*  API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif
#endif /* __HI_GV_PROGRESSBAR_H__ */
