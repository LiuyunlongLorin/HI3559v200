/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: HIGV LOG Moudle
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_WIDGETMOD_H__
#define __HI_GV_WIDGETMOD_H__

#include "hi_type.h"
#include "hi_gv_coremod.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* ************************** Structure Definition *************************** */
/* * addtogroup      WidgetType  */
/* *  【Widget type】; CNcomment: 【Widget分类】 */
typedef enum {  /*  CNcomment:  CORE (HIGV_COMMON_MODID_START)  PARSER (HIGV_COMMON_MODID_START + 12) WIDGET
                        (HIGV_COMMON_MODID_START + 13) */
    HIGV_MODID_GROUPBOX = HIGV_COMMON_MODID_START + 13,  /*groupbox widget ; CNcomment: groubox控件 */
    HIGV_MODID_WIN,  /* window widget ; CNcomment: 窗口控件 */
    HIGV_MODID_MSGBOX, /*dialogbox widget; CNcomment: 对话框控件 */
    HIGV_MODID_BUTTON,  /* button widget; CNcomment:  button控件 */
    HIGV_MODID_EDIT,  /* edit widget ; CNcomment: edit控件 */
    HIGV_MODID_SCROLLBOX, /* scrollbox widget; CNcomment: scrollbox控件 */
    HIGV_MODID_SPIN,  /* spin widget ; spin控件 */
    HIGV_MODID_LABEL,  /* label widget ; CNcomment: label控件 */
    HIGV_MODID_PROGRESSBAR,  /* progressbar widget ; CNcomment: progressbar控件 */
    HIGV_MODID_LIST,  /* listbox widget; CNcomment: list控件 */
    HIGV_MODID_SCROLLBAR, /* scrollbar widget; CNcomment: scrollbar控件 */
    HIGV_MODID_SCROLLGRID, /* scrollgrid widget; CNcomment: scrollgrid控件 */
    HIGV_MODID_TIMETRACK, /* timetrack widget; CNcomment:  TimeTrack控件 */
    HIGV_MODID_CALENDAR, /* calendar widget; CNcomment:  日历控件 */
    HIGV_MODID_DAYLIST,  /* daylist widget; CNcomment: 日期表控件 */
    HIGV_MODID_VIDEOSHELTER, /* videoshelter widget ; CNcomment: 视频遮挡控件 */
    HIGV_MODID_MOVEMONITOR,  /* movemonitor widget ; CNcomment: 移动检测控件 */
    HIGV_MODID_IMSY,
    HIGV_MODID_SCROLLTEXT,
    HIGV_MODID_IMAGE,
    HIGV_MODID_IMAGEEX,
    HIGV_MODID_GRID,
    HIGV_MODID_IMAGECIPHER,
    HIGV_MODID_CLOCK,
    HIGV_MODID_IPEDIT,
    HIGV_MODID_COMBOBOX,
    HIGV_MODID_IMEWINDOW,
    HIGV_MODID_CONTAINERWIDGET,
    HIGV_MODID_SCROLLVIEW,
    HIGV_MODID_SLIDEUNLOCK,
    HIGV_MODID_WHEELVIEW,

    HIGV_MODID_WIDGET_BUTT
} HIGV_MODID_WIDGET_E;

/* Structure Definition end */
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __HI_GV_MOD_H__ */
