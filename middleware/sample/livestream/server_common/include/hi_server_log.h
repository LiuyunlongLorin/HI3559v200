/**
  * Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
  * All rights reserved.
  *
  * @file         hi_server_log.h
  * @brief      middleware log function.
  * @author   HiMobileCam middleware develop team
  * @date      2016.06.29
  */
#ifndef __HI_SERVER_LOG_H__
#define __HI_SERVER_LOG_H__

#include "hi_mw_type.h"
#include "hi_defs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define hi_usleep(usec) \
do { \
    struct timespec req; \
    req.tv_sec  = (usec) / 1000000; \
    req.tv_nsec = ((usec) % 1000000) * 1000; \
    nanosleep (&req, NULL); \
} while (0)

/*
 * @brief  log level type.
 */
typedef enum hiSERVER_LOG_LEVEL_E
{
    HI_SERVER_LOG_LEVEL_DEBUG = 0,
    HI_SERVER_LOG_LEVEL_INFO,
    HI_SERVER_LOG_LEVEL_WARN,
    HI_SERVER_LOG_LEVEL_ERR,
    HI_SERVER_LOG_LEVEL_FATAL,
    HI_SERVER_LOG_LEVEL_BUTT
} HI_SERVER_LOG_LEVEL_E;

typedef HI_VOID(*HI_SERVER_OUTPUT_FUNC)(const HI_CHAR* fmt, ... );

/**
*   @brief  set enabled log level, logs with equal or higher level than enabled will be output
*   @param[in] enLevel : HI_SERVER_LOG_LEVEL_E: enabled level
*   @retval  0 success,others failed
*/
HI_S32 HI_SERVER_LOG_SetEnabledLevel(HI_SERVER_LOG_LEVEL_E enLevel);

/**
*   @brief  use this API to set log output function instread of 'printf'
*   @param[in] pFunc : HI_SERVER_OUTPUT_FUNC: output function implements by user
*   @retval  0 success,others failed
*/
HI_S32 HI_SERVER_LOG_SetOutputFunc(HI_SERVER_OUTPUT_FUNC pFunc);

/**
*   @brief  output log
*   @param[in] pszModName : HI_CHAR*: module name
*   @param[in] enLevel : HI_SERVER_LOG_LEVEL_E: log level
*   @param[in] pszFmt : HI_CHAR*: log content, accept multi-parameters
*   @retval  0 success,others failed
*/
HI_S32 HI_SERVER_LOG_Printf(const HI_CHAR* pszModName, HI_SERVER_LOG_LEVEL_E enLevel, const HI_CHAR* pszFmt, ...) __attribute__((format(printf,3,4)));


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_SERVER_LOG_H__ */
