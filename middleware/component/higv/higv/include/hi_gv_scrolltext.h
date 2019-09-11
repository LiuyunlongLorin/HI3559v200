/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: ScrollText widget API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_SCROLLTEXT_H__
#define __HI_GV_SCROLLTEXT_H__

/* add include here */
#include "hi_gv.h"

#ifdef HIGV_USE_WIDGET_SCROLLTEXT

#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * addtogroup      ScrollText        */
/* * 【ScrollText widget】CNcomment: 【ScrollText控件】 */
/* *Scrolltext style: alawy scroll ; CNcomment: ScrollText控件自定义风格--滚动开的状态下，始终循环滚动 */
#define ST_STYLE_ALWAYSSCROLL 0x4

/* * Scrolltext style: alawy scroll; CNcomment:ScrollText控件自定义风格--滚动开的状态下，始终循环滚动 */
#define HIGV_ST_STYLE_ALWAYSSCROLL HIGV_STYLE_DEFINE(HIGV_WIDGET_SCROLLTEXT, ST_STYLE_ALWAYSSCROLL)

/* *scroll direction ; CNcomment: 滚动方向 */
typedef enum {
    HIGV_DIRECTION_LEFT = 0,
    /* left scroll ; CNcomment: 自右向左滚动 */
    HIGV_DIRECTION_RIGHT,
    /* right scroll ; CNcomment: 自左向右滚动 */
    HIGV_DIRECTION_BUTT
} HIGV_DIRECTION_E;

/* * ==== Structure Definition end ==== */
/* ****************************** API declaration **************************** */
/* * addtogroup      ScrollText        */
/* *  【ScrollText widget】 ; CNcomment: 【ScrollText控件】 */
/**
* brief Set content of scrolltext. CNcomment:设置滚动字幕控件中的内容
* param[in] hScrollText Scrolltext handle.CNcomment:控件句柄
* param[in] Image Picture resouce ID.CNcomment:图片资源ID，INVALID_RESID为无图片
* param[in] String String.CNcomment:文本字符串
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see ::HI_GV_ScrollText_GetContent
*/
HI_S32 HI_GV_ScrollText_SetContent(HIGV_HANDLE hScrollText, HI_RESID Image, HI_CHAR *String);

/**
* brief Set content of scrolltext.CNcomment:设置滚动字幕控件中的内容
* param[in] hScrollText Scrolltext handle.CNcomment:控件句柄
* param[in] Image Picture resouce ID.CNcomment:图片资源ID，INVALID_RESID为无图片
* param[in] String String.CNcomment:文本字符串
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see ::HI_GV_ScrollText_GetContent
*/
HI_S32 HI_GV_ScrollText_SetContentByID(HIGV_HANDLE hScrollText, HI_RESID Image, HI_U32 StrID);

/**
* brief Get content of scrolltext.CNcomment:获取滚动字幕控件的内容
* param[in] hScrollText Scrolltext handle.CNcomment:控件句柄
* param[out] pImage Picture resouce ID.CNcomment:图片资源ID
* param[out] pString String.CNcomment:文本
* param[in] TextBufLen String length.CNcomment:文本长度
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL  CNcomment:指针为空或文本长度超出最大文本长度
* see ::HI_GV_ScrollText_SetContent
*/
HI_S32 HI_GV_ScrollText_GetContent(HIGV_HANDLE hScrollText, HI_RESID *pImage, HI_CHAR *pString, HI_U32 TextBufLen);

/**
* brief Set start position of scrolltext.CNcomment:设置图片的起始位置
* param[in] hScrollText  Scrolltext handle.CNcomment:控件句柄
* param[in] x the x coordiate of picture.CNcomment:图片在控件中的x坐标(相对于控件原点，默认相对控件居中)
* param[in] y the y coordiate of picture.CNcomment:图片在控件中的y坐标(相对于控件原点，默认相对控件居中)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ScrollText_SetImagePos(HIGV_HANDLE hScrollText, HI_S32 x, HI_S32 y);

/**
* brief Set start position of scrolltext.CNcomment:设置文本的起始位置
* param[in] hScrollText   Scrolltext handle.CNcomment:控件句柄
* param[in] x the x coordiate of text.CNcomment:文本在控件中的x坐标(相对于控件原点，默认相对控件居中)
* param[in] y the y coordiate of text.CNcomment:文本在控件中的y坐标(相对于控件原点，默认相对控件居中)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ScrollText_SetTextPos(HIGV_HANDLE hScrollText, HI_S32 x, HI_S32 y);

/**
* brief Set interval of scrolltext.CNcomment:设置控件滚动的时间间隔
* param[in] hScrollText Scrolltext handle.CNcomment:控件句柄
* param[in] TimeInterval Time interval.CNcomment:时间间隔(以ms为单位，默认为30ms)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ScrollText_SetTimeInterval(HIGV_HANDLE hScrollText, HI_U32 TimeInterval);

/**
* brief Set step of scrolltext.CNcomment:设置控件滚动步长
* param[in] hScrollText CNcomment:控件句柄
* param[in] Step Scroll step.CNcomment:滚动步长(以pixel为单位，默认为5pixel)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ScrollText_SetStep(HIGV_HANDLE hScrollText, HI_U32 Step);

/**
* brief Set direction of scrolltext.CNcomment:设置滚动方向
* param[in] hScrollText Scrolltext handle.CNcomment:控件句柄
* param[in] Direction Scroll direction.CNcomment:滚动方向(默认自右向左)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ScrollText_SetDirection(HIGV_HANDLE hScrollText, HIGV_DIRECTION_E Direction);

/**
* brief Set widget content.CNcomment:设置控件状态
* param[in] hScrollText Scrolltext handle.CNcomment:控件句柄
* param[in] Status The satus of scrolltext.CNcomment:控件状态(TRUE为滚动，FALSE为停止滚动，默认为TRUE)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ScrollText_SetStatus(HIGV_HANDLE hScrollText, HI_BOOL Status);

/**
* brief Get the position of the content.CNcomment:获取控件内容当前所在的位置
* param[in] hScrollText Scrolltext handle.CNcomment:控件句柄
* param[out] x         x coordinate. CNcomment:x坐标
* param[out] y        y coordinate.  CNcomment:y坐标
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ScrollText_GetCurPos(HIGV_HANDLE hScrollText, HI_S32 *x, HI_S32 *y);

/**
* brief Reset  the position of the content.CNcomment:复位控件内容的位置
* param[in] hScrollText Scrolltext handle.CNcomment:控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ScrollText_ResetPos(HIGV_HANDLE hScrollText);

/* API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif
#endif /* __HI_GV_SCROLLTEXT_H__ */
