/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_comm_inner.h
 * @brief   comm module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MAPI_COMM_INNER_H__
#define __MAPI_COMM_INNER_H__

#include "hi_mapi_comm_define.h"

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef MAPI_LOG_NONE
#define MAPI_DEBUG_TRACE(Module, fmt...)
#define MAPI_INFO_TRACE(Module, fmt...)
#define MAPI_WARN_TRACE(Module, fmt...)
#define MAPI_ERR_TRACE(Module, fmt...)
#define MAPI_FATAL_TRACE(Module, fmt...)
#elif defined MAPI_LOG_ERR
#define MAPI_DEBUG_TRACE(Module, fmt...)
#define MAPI_INFO_TRACE(Module, fmt...)
#define MAPI_WARN_TRACE(Module, fmt...)
#define MAPI_ERR_TRACE(Module, fmt...) \
    do {                                                                                               \
        HI_MAPI_LOG_Printf(Module, HI_MAPI_LOG_LEVEL_ERR, "[%s][%4d][Info]:", __FUNCTION__, __LINE__); \
        HI_MAPI_LOG_FormatPrintf(HI_MAPI_LOG_LEVEL_ERR, fmt);                                          \
    } while (0)

#define MAPI_FATAL_TRACE(Module, fmt...)                                                                 \
    do {                                                                                                 \
        HI_MAPI_LOG_Printf(Module, HI_MAPI_LOG_LEVEL_FATAL, "[%s][%4d][Info]:", __FUNCTION__, __LINE__); \
        HI_MAPI_LOG_FormatPrintf(HI_MAPI_LOG_LEVEL_FATAL, fmt);                                          \
    } while (0)

#elif defined MAPI_LOG_ALL
#define MAPI_DEBUG_TRACE(Module, fmt...)                                                                 \
    do {                                                                                                 \
        HI_MAPI_LOG_Printf(Module, HI_MAPI_LOG_LEVEL_DEBUG, "[%s][%4d][Info]:", __FUNCTION__, __LINE__); \
        HI_MAPI_LOG_FormatPrintf(HI_MAPI_LOG_LEVEL_DEBUG, fmt);                                          \
    } while (0)

#define MAPI_INFO_TRACE(Module, fmt...)                                                                 \
    do {                                                                                                \
        HI_MAPI_LOG_Printf(Module, HI_MAPI_LOG_LEVEL_INFO, "[%s][%4d][Info]:", __FUNCTION__, __LINE__); \
        HI_MAPI_LOG_FormatPrintf(HI_MAPI_LOG_LEVEL_INFO, fmt);                                          \
    } while (0)

#define MAPI_WARN_TRACE(Module, fmt...)                                                                 \
    do {                                                                                                \
        HI_MAPI_LOG_Printf(Module, HI_MAPI_LOG_LEVEL_WARN, "[%s][%4d][Info]:", __FUNCTION__, __LINE__); \
        HI_MAPI_LOG_FormatPrintf(HI_MAPI_LOG_LEVEL_WARN, fmt);                                          \
    } while (0)

#define MAPI_ERR_TRACE(Module, fmt...) \
    do {                                                                                               \
        HI_MAPI_LOG_Printf(Module, HI_MAPI_LOG_LEVEL_ERR, "[%s][%4d][Info]:", __FUNCTION__, __LINE__); \
        HI_MAPI_LOG_FormatPrintf(HI_MAPI_LOG_LEVEL_ERR, fmt);                                          \
    } while (0)

#define MAPI_FATAL_TRACE(Module, fmt...)                                                                 \
    do {                                                                                                 \
        HI_MAPI_LOG_Printf(Module, HI_MAPI_LOG_LEVEL_FATAL, "[%s][%4d][Info]:", __FUNCTION__, __LINE__); \
        HI_MAPI_LOG_FormatPrintf(HI_MAPI_LOG_LEVEL_FATAL, fmt);                                          \
    } while (0)
#endif

typedef struct tagMAPI_RgnHdlInfo_S {
    HI_U32 GrpHdl;
    HI_U32 ChnHdl;
    HI_U32 OsdHdl;
    MOD_ID_E enModId;
} MAPI_RgnHdlInfo_S;

HI_S32 MAPI_AddRgnHdl(HI_HANDLE GrpHdl, HI_HANDLE ChnHdl, HI_HANDLE OsdHdl, MOD_ID_E enModId);
HI_U32 MAPI_GetRgnHdl(HI_HANDLE GrpHdl, HI_HANDLE ChnHdl, HI_HANDLE OsdHdl, MOD_ID_E enModId);
HI_S32 MAPI_DelRgnHdl(HI_HANDLE GrpHdl, HI_HANDLE ChnHdl, HI_HANDLE OsdHdl, MOD_ID_E enModId);

HI_BOOL MAPI_GetMediaInitStatus(HI_VOID);
HI_BOOL MAPI_GetSysInitStatus(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

