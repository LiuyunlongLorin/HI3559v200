/**
  * Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
  * All rights reserved.
  *
  * @file         ffdemux_log.h
  * @brief      middleware log function.
  * @author   HiMobileCam middleware develop team
  * @date      2016.06.29
  */
#ifndef __FFDEMUX_LOG_H__
#define __FFDEMUX_LOG_H__

#include "hi_type.h"
#include "hi_defs.h"
#define MODULE_NAME_DEMUX  "ffmpegDemux"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/*
 * @brief  log level type.
 */
typedef enum hiFFDEMUX_LOG_LEVEL_E
{
    HI_FFDEMUX_LOG_LEVEL_DEBUG = 0,
    HI_FFDEMUX_LOG_LEVEL_INFO,
    HI_FFDEMUX_LOG_LEVEL_WARN,
    HI_FFDEMUX_LOG_LEVEL_ERR,
    HI_FFDEMUX_LOG_LEVEL_FATAL,
    HI_FFDEMUX_LOG_LEVEL_BUTT
} HI_FFDEMUX_LOG_LEVEL_E;

typedef HI_VOID(*HI_FFDEMUX_OUTPUT_FUNC)(const HI_CHAR* fmt, ... );

/**
*   @brief  set enabled log level, logs with equal or higher level than enabled will be output
*   @param[in] enLevel : HI_FFDEMUX_LOG_LEVEL_E: enabled level
*   @retval  0 success,others failed
*/
HI_S32 FFDEMUX_SetEnabledLevel(HI_FFDEMUX_LOG_LEVEL_E enLevel);

/**
*   @brief  use this API to set log output function instread of 'printf'
*   @param[in] pFunc : HI_FFDEMUX_OUTPUT_FUNC: output function implements by user
*   @retval  0 success,others failed
*/
HI_S32 FFDEMUX_SetOutputFunc(HI_FFDEMUX_OUTPUT_FUNC pFunc);

/**
*   @brief  output log
*   @param[in] pszModName : HI_CHAR*: module name
*   @param[in] enLevel : HI_FFDEMUX_LOG_LEVEL_E: log level
*   @param[in] pszFmt : HI_CHAR*: log content, accept multi-parameters
*   @retval  0 success,others failed
*/
HI_S32 FFDEMUX_Printf(const HI_CHAR* pszModName, HI_FFDEMUX_LOG_LEVEL_E enLevel, const HI_CHAR* pszFmt, ...) __attribute__((format(printf,3,4)));


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __FFDEMUX_LOG_H__ */
