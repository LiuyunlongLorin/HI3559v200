/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: hi_gv_coremod.h
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_COREMOD_H__
#define __HI_GV_COREMOD_H__

#include "hi_type.h"
#include "hi_gv_conf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* ************************** Structure Definition *************************** */
/* *  addtogroup      Core */
/* * 【core mode】    ; CNcomment:【Core模块】    */

typedef enum {
    /* * CNcomment: CORE (HIGV_COMMON_MODID_START)
        PARSER (HIGV_COMMON_MODID_START + 12) WIDGET (HIGV_COMMON_MODID_START
       + 13) */
    /* Null module or all modules */
    HIGV_MODID_NULL = HIGV_COMMON_MODID_START,
    HIGV_MODID_WIDGET,
    /* <Widget framework , CNcomment: 控件框架 */
    HIGV_MODID_CONTAINER,
    /* Window container,    CNcomment: 窗口容器 */
    HIGV_MODID_GC,
    /* graphic context,         CNcomment: 绘制类 */
    HIGV_MODID_MTASK,
    /*  Multi task,      CNcomment: 多任务 */
    HIGV_MODID_MSGM,
    /*  message framework       CNcomment:  消息框架 */
    HIGV_MODID_TIMER,
    /*  Timer,       CNcomment:  定时器 */
    HIGV_MODID_RESM,
    /*  Resouce manager ,   CNcomment:  资源管理器 */
    HIGV_MODID_IM,
    /*  Input method ,  CNcomment: 输入法 */
    HIGV_MODID_WM,
    /*  Window manager, CNcomment: 窗口管理 */
    /* Default Data Buffer */
    HIGV_MODID_DDB,
    /* Abstract Data Model */
    HIGV_MODID_ADM,
    HIGV_MODID_PARSER,
    HIGV_MODID_ANIM,
    HIGV_MODID_CORE_BUTT

} HIGV_MODID_CORE_E;
/* * ==== Structure Definition end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __HI_GV_MOD_H__ */
