/**
  * Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
  * All rights reserved.
  *
  * @file         hi_server_log.h
  * @brief      middleware log function.
  * @author   HiMobileCam middleware develop team
  * @date      2016.06.29
  */
#ifndef __DNG_LOG_H__
#define __DNG_LOG_H__

#include "hi_mw_type.h"
#include "hi_defs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define MODULE_NAME_DNG "DNG"

/*
 * @brief  log level type.
 */
typedef enum hiDNG_LOG_LEVEL_E {
    DNG_LOG_LEVEL_DEBUG = 0,
    DNG_LOG_LEVEL_INFO,
    DNG_LOG_LEVEL_WARN,
    DNG_LOG_LEVEL_ERR,
    DNG_LOG_LEVEL_FATAL,
    DNG_LOG_LEVEL_BUTT
} DNG_LOG_LEVEL_E;

typedef HI_VOID (*DNG_OUTPUT_FUNC)(const HI_CHAR *fmt, ...);

/**
*   @brief  set enabled log level, logs with equal or higher level than enabled will be output
*   @param[in] enLevel : DNG_LOG_LEVEL_E: enabled level
*   @retval  0 success,others failed
*/
HI_S32 DNG_LOG_SetEnabledLevel(DNG_LOG_LEVEL_E enLevel);

/**
*   @brief  use this API to set log output function instread of 'printf'
*   @param[in] pFunc : DNG_OUTPUT_FUNC: output function implements by user
*   @retval  0 success,others failed
*/
HI_S32 DNG_LOG_SetOutputFunc(DNG_OUTPUT_FUNC pFunc);

/**
*   @brief  output log
*   @param[in] pszModName : HI_CHAR*: module name
*   @param[in] enLevel : DNG_LOG_LEVEL_E: log level
*   @param[in] pszFmt : HI_CHAR*: log content, accept multi-parameters
*   @retval  0 success,others failed
*/
HI_S32 DNG_LOG_Printf(const HI_CHAR *pszModName, DNG_LOG_LEVEL_E enLevel, const HI_CHAR *pszFmt,
                      ...) __attribute__((format(printf, 3, 4)));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __DNG_LOG_H__ */
