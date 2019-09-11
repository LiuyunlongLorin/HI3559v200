/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_log.h
 * @brief   log module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_LOG_H__
#define __HI_MAPI_LOG_H__

#include "hi_mapi_comm_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/*
 * @brief  log level type.
 */
typedef enum hiMAPI_LOG_LEVEL_E {
    HI_MAPI_LOG_LEVEL_VERBOSE = -1,  // assigned to verbose -1 because need to be compatible with previouse version
    HI_MAPI_LOG_LEVEL_DEBUG = 0,
    HI_MAPI_LOG_LEVEL_INFO,
    HI_MAPI_LOG_LEVEL_WARN,
    HI_MAPI_LOG_LEVEL_ERR,
    HI_MAPI_LOG_LEVEL_FATAL,
    HI_MAPI_LOG_LEVEL_BUTT
} HI_MAPI_LOG_LEVEL_E;

typedef HI_VOID (*HI_OUTPUT_FUNC)(const HI_CHAR *fmt, ...);

/**
*   @brief  set enabled log level, logs with equal or higher level than enabled will be output
*   @param[in] enLevel : HI_MAPI_LOG_LEVEL_E: enabled level
*   @retval  0 success,others failed
*/
HI_S32 HI_MAPI_LOG_SetEnabledLevel(HI_MAPI_LOG_LEVEL_E enLevel);

/**
*   @brief  use this API to set log output function instread of 'printf'
*   @param[in] pFunc : HI_OUTPUT_FUNC: output function implements by user
*   @retval  0 success,others failed
*/
HI_S32 HI_MAPI_LOG_SetOutputFunc(HI_OUTPUT_FUNC pFunc);

/**
*   @brief  output log
*   @param[in] enModId : HI_MAPI_MOD_ID_E: module id
*   @param[in] enLevel : HI_MAPI_LOG_LEVEL_E: log level
*   @param[in] pszFmt : HI_CHAR*: log content, accept multi-parameters
*   @retval  0 success,others failed
*/
HI_S32 HI_MAPI_LOG_Printf(HI_MAPI_MOD_ID_E enModId, HI_MAPI_LOG_LEVEL_E enLevel, const HI_CHAR *pszFmt,
                          ...) __attribute__((format(printf, 3, 4)));

/**
*   @brief  output log
*   @param[in] pszFmt : HI_CHAR*: log content, accept multi-parameters
*   @retval  0 success,others failed
*/
HI_S32 HI_MAPI_LOG_FormatPrintf(HI_MAPI_LOG_LEVEL_E enLevel, const HI_CHAR *pszFmt,
                                ...) __attribute__((format(printf, 2, 3)));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_DEBUG_DEFINE_H__ */
