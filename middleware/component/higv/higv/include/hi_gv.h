/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: HiGv API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_H__
#define __HI_GV_H__

/* add include here */
#include <stdio.h>
#include "hi_go.h"
#include "hi_gv_conf.h"
#include "hi_gv_errno.h"
#include "hi_gv_mlayer.h"
#include "hi_gv_app.h"
#include "hi_gv_msg.h"
#include "hi_gv_resm.h"
#include "hi_gv_timer.h"
#include "hi_gv_widget.h"
#include "hi_gv_gesture.h"
#include "hi_gv_animation.h"

#if HI_OS_TYPE == HI_OS_LINUX
#include <sys/time.h>
#endif
#ifdef HIGV_USE_WIDGET_PROGRESSBAR
#include "hi_gv_progressbar.h"
#endif

#ifdef HIGV_USE_WIDGET_EDIT
#include "hi_gv_edit.h"
#endif

#ifdef HIGV_USE_WIDGET_SPINBOX
#include "hi_gv_spin.h"
#ifdef HIGV_USE_WIDGET_CALENDAR
#include "hi_gv_calendar.h"
#endif
#endif

#include "hi_gv_win.h"
#include "hi_gv_timer.h"
#include "hi_gv_wm.h"

#ifdef HIGV_USE_WIDGET_SCROLLTEXT
#include "hi_gv_scrolltext.h"
#endif

#ifdef HIGV_USE_WIDGET_IMAGECIPHER
#include "hi_gv_imagecipher.h"
#endif

#ifdef HIGV_USE_WIDGET_IMAGE
#include "hi_gv_image.h"
#endif

#ifdef HIGV_USE_WIDGET_IMAGEEX
#include "hi_gv_imageex.h"
#endif

#ifdef HIGV_USE_WIDGET_CLOCK
#include "hi_gv_clock.h"
#endif

#ifdef HIGV_USE_WIDGET_LISTBOX
#include "hi_gv_listbox.h"
#endif

#ifdef HIGV_USE_WIDGET_TIMETRACK
#include "hi_gv_timetrack.h"
#endif

#if defined(HIGV_USE_WIDGET_IPEDIT) && defined(HIGV_USE_WIDGET_EDIT)
#include "hi_gv_ipedit.h"
#endif

#ifdef HIGV_USE_WIDGET_SCROLLBAR
#include "hi_gv_scrollbar.h"
#endif

#ifdef HIGV_USE_WIDGET_SCROLLBOX
#include "hi_gv_scrollbox.h"
#endif

#include "hi_gv_lan.h"

#ifdef HIGV_USE_MODULE_CURSOR
#include "hi_gv_cursor.h"
#endif
#ifdef HIGV_USE_WIDGET_TRACKBAR
#include "hi_gv_trackbar.h"
#endif

#if defined(HIGV_USE_WIDGET_COMBOBOX) && defined(HIGV_USE_WIDGET_LISTBOX) && defined(HIGV_USE_WIDGET_SPINBOX)
#include "hi_gv_combobox.h"
#endif

#if defined(HIGV_USE_WIDGET_MSGBOX) && defined(HIGV_USE_WIDGET_BUTTON) && defined(HIGV_USE_WIDGET_IMAGE) && \
    defined(HIGV_USE_WIDGET_LABEL)
#include "hi_gv_msgbox.h"
#endif

#ifdef HIGV_USE_MODULE_IME
#include "hi_gv_imewindow.h"
#endif

#ifdef HIGV_USE_WIDGET_VIDEOSHELTER
#include "hi_gv_videoshelter.h"
#endif

#ifdef HIGV_USE_WIDGET_MOVEMONITOR
#include "hi_gv_movemonitor.h"
#endif

#ifdef HIGV_USE_WIDGET_MULTIEDIT
#include "hi_gv_multiedit.h"
#endif

#ifdef HIGV_USE_WIDGET_SCROLLVIEW
#include "hi_gv_scrollview.h"
#endif

#ifdef HIGV_USE_WIDGET_SCROLLGRID
#include "hi_gv_scrollgrid.h"
#endif
#include "hi_gv_ddb.h"

#ifdef HIGV_USE_WIDGET_SLIDEUNLOCK
#include "hi_gv_slideunlock.h"
#endif

#ifdef HIGV_USE_WIDGET_WHEELVIEW
#include "hi_gv_wheelview.h"
#endif

#ifdef HIGV_USE_WIDGET_IMAGEVIEW
#include "hi_gv_imageview.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
/* ************************** Structure Definition *************************** */
/* * addtogroup      Common  */
/* *【Common】 ; CNcomment: 【Common模块】 */
/* *#define __PARSER_PERFORMANCE */
#ifdef __PARSER_PERFORMANCE
#define PARSER_ADP_TINIT()                \
    struct timeval g_tv_start, g_ tv_end; \
    unsigned int g_time_cost; \
    unsigned int g_line_start = __LINE__
#define PARSER_ADP_TSTART()  gettimeofday(&g_tv_start, HI_NULL)
#define PARSER_ADP_TEND(str) \
    gettimeofday(&tv_end, HI_NULL);                                       \
    g_time_cost = (tv_end.tv_usec / 1000 + g_ tv_end.tv_sec * 1000) -     \
                  (g_tv_start.tv_usec / 1000 + g_tv_start.tv_sec * 1000); \
    if ((g_time_cost >= 5) && (*str != '\0'))                             \
    HIGV_Printf("%s (%s %d - %d) COST: %d ms %s\n", __FUNCTION__, __FILE__, g_line_start, __LINE__, g_time_cost, str)
extern struct timeval g_tv_start, g_ tv_end;
extern unsigned int g_time_cost, g_line_start;
#define PRINT_CUR_TIME1(str)                                                 \
    do {                                                                     \
        struct timeval now;                                                  \
        gettimeofday(&now, HI_NULL);                                         \
        HIGV_Printf("%s ===%s %d===now:%ums====\n", str, __FILE__, __LINE__, \
            (now.tv_sec * 1000 + now.tv_usec / 1000));                       \
    } while (0)
/* *   decode statistic */
/* * CNcomment:解码统计 */
#define PARSER_ADP_DECINIT()                   \
    struct timeval g_tvdec_start, g_tvdec_end; \
    unsigned int g_timedec_cost; \
    unsigned int g_linedec_start = __LINE__
#define PARSER_ADP_DECSTART() gettimeofday(&g_tvdec_start, HI_NULL)
#define PARSER_ADP_DECEND()   gettimeofday(&g_tvdec_end, HI_NULL)
#define PARSER_ADP_DECDIS()   \
    ((g_tvdec_end.tv_sec * 1000 + g_tvdec_end.tv_usec / 1000) - \
        (g_tvdec_start.tv_sec * 1000 + g_tvdec_start.tv_usec / 1000))
extern struct timeval g_tvdec_start, g_tvdec_end;
extern unsigned int g_timedec_cost, g_linedec_start;
HI_VOID HI_GV_PER_InitDecTimeCount(HI_VOID);
HI_VOID HI_GV_PER_PrintDecTimeCount(HI_VOID);
/* * paint statistic */
/* * CNcomment:绘制统计 */
#define PARSER_ADP_PAINTINIT()                     \
    struct timeval g_tvpaint_start, g_tvpaint_end; \
    unsigned int g_timepaint_cost; \
    unsigned int linedepaint_start = __LINE__
#define PARSER_ADP_PAINTSTART() gettimeofday(&g_tvpaint_start, HI_NULL)
#define PARSER_ADP_PAINTEND()   gettimeofday(&g_tvpaint_end, HI_NULL)
#define PARSER_ADP_PAINTDIS()   \
    ((g_tvpaint_end.tv_sec * 1000 + g_tvpaint_end.tv_usec / 1000) - \
        (g_tvpaint_start.tv_sec * 1000 + g_tvpaint_start.tv_usec / 1000))
extern struct timeval g_tvpaint_start, g_tvpaint_end;
extern unsigned int g_timepaint_cost, g_ linepaint_start;
HI_VOID HI_GV_PER_InitPaintTimeCount(HI_VOID);
HI_VOID HI_GV_PER_PrintPaintTimeCount(HI_VOID);
/* *  script statistic */
/* * CNcomment:脚本执行时间统计 */
#define PARSER_ADP_SCRIPTINIT()                      \
    struct timeval g_tvscript_start, g_tvscript_end; \
    unsigned int g_timescript_cost; \
    unsigned int g_linescript_start = __LINE__
#define PARSER_ADP_SCRIPTSTART() gettimeofday(&g_tvscript_start, HI_NULL)
#define PARSER_ADP_SCRIPTEND()   gettimeofday(&g_tvscript_end, HI_NULL)
#define PARSER_ADP_SCRIPTDIS()   \
    ((g_tvscript_end.tv_sec * 1000 + g_tvscript_end.tv_usec / 1000) - \
        (g_tvscript_start.tv_sec * 1000 + g_tvscript_start.tv_usec / 1000))
extern struct timeval g_tvscript_start, g_tvscript_end;
extern unsigned int g_timescript_cost, g_linescript_start;
HI_VOID HI_GV_PER_InitScriptTimeCount(HI_VOID);
HI_VOID HI_GV_PER_PrintScriptTimeCount(HI_VOID);
/* * compute the time from key down to display */
/* * CNcomment:按键到新界面出现的时间 */
#define PARSER_ADP_SWINIT()                  \
    struct timeval g_tvsw_start, g_tvsw_end; \
    unsigned int g_timesw_cost; \
    unsigned int g_linesw_start = __LINE__
#define PARSER_ADP_SWSTART() gettimeofday(&g_tvsw_start, HI_NULL)
#define PARSER_ADP_SWEND()   gettimeofday(&g_tvsw_end, HI_NULL)
#define PARSER_ADP_SWDIS()   \
    ((g_tvsw_end.tv_sec * 1000 + g_tvsw_end.tv_usec / 1000) - \
        (g_tvsw_start.tv_sec * 1000 + g_tvsw_start.tv_usec / 1000))
extern struct timeval g_tvsw_start, g_tvsw_end;
extern unsigned int g_timesw_cost, g_linesw_start;
HI_VOID HI_GV_PER_InitSWTimeCount(HI_VOID);
HI_VOID HI_GV_PER_PrintTimeCount(HI_VOID);
HI_VOID HI_GV_PER_InitPaintTime(HI_VOID);
HI_VOID HI_GV_PER_PrintPaintPart(HI_VOID);
#define PARSER_TMP_TINIT()                   \
    struct timeval tv_start_tmp, tv_end_tmp; \
    unsigned int tv_cost_tmp; \
    unsigned int g_line_start = __LINE__
#define PARSER_TMP_TSTART()        gettimeofday(&tv_start_tmp, HI_NULL)
#define PARSER_TMP_TEND(str, diff) \
    do {                                                                                                    \
        gettimeofday(&tv_end_tmp, HI_NULL);                                                                 \
        tv_cost_tmp = (tv_end_tmp.tv_usec / 1000 + tv_end_tmp.tv_sec * 1000) -                              \
                      (tv_start_tmp.tv_usec / 1000 + tv_start_tmp.tv_sec * 1000);                           \
        if (*str != '\0')                                                                                   \
            HIGV_Printf("%s (%s %d - %d) COST: %d ms %s\n", __FUNCTION__, __FILE__, g_line_start, __LINE__, \
                tv_cost_tmp, str);                                                                          \
        diff += tv_cost_tmp;                                                                                \
    } while (0)
#else
#define PARSER_ADP_TINIT()
#define PARSER_ADP_TSTART()
#define PARSER_ADP_TEND(str)
#define PRINT_CUR_TIME1(str)
#define PARSER_ADP_DECINIT()
#define PARSER_ADP_DECSTART()
#define PARSER_ADP_DECEND()
#define PARSER_ADP_DECDIS()

#define HI_GV_PER_InitDecTimeCount()
#define HI_GV_PER_PrintDecTimeCount()

#define PARSER_ADP_PAINTINIT()
#define PARSER_ADP_PAINTSTART()
#define PARSER_ADP_PAINTEND()
#define PARSER_ADP_PAINTDIS()
#define HI_GV_PER_InitPaintTimeCount()  ;
#define HI_GV_PER_PrintPaintTimeCount() ;
#define PARSER_ADP_SCRIPTSTART()
#define PARSER_ADP_SCRIPTEND()
#define PARSER_ADP_SCRIPTDIS()
#define HI_GV_PER_InitScriptTimeCount()
#define HI_GV_PER_PrintScriptTimeCount()
#define PARSER_ADP_SWINIT()
#define PARSER_ADP_SWSTART()
#define PARSER_ADP_SWEND()
#define HI_GV_PER_InitSWTimeCount()
#define HI_GV_PER_PrintTimeCount()
#define HI_GV_PER_InitPaintTime()
#define HI_GV_PER_PrintPaintPart()
#define PARSER_TMP_TINIT()
#define PARSER_TMP_TSTART()
#define PARSER_TMP_TEND(str, diff)
#endif

/*  ==== Structure Definition end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_H__ */
