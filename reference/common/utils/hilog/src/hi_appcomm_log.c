/**
 * @file    hi_appcomm_log.c
 * @brief   appcomm log implementation.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/10
 * @version   1.0

 */
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include "hi_appcomm_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static HI_BOOL s_bLogOn = HI_TRUE;
static HI_BOOL s_bLogTime = HI_TRUE;
/** product log level */
static HI_LOG_LEVEL_E s_enLogLevel = HI_LOG_LEVEL;
static const HI_CHAR* s_apszLOGLevel[HI_LOG_LEVEL_BUTT] = {LIGHT_PURPLE"FATAL"NONE, LIGHT_RED"ERROR"NONE, YELLOW"WARN"NONE, LIGHT_GREEN"INFO"NONE, "DEBUG"};

HI_VOID HI_LOG_Print(HI_LOG_LEVEL_E enLevel, const HI_CHAR* pszModule, const HI_CHAR* pszFunc, HI_U32 u32Line, HI_CHAR* pszFmt, ...)
{
    if ((s_enLogLevel >= enLevel) && (s_bLogOn))
    {
        pszModule = (NULL == pszModule) ? "" : pszModule;
        pszFmt = (NULL == pszFmt) ? "" : pszFmt;
        va_list args;

        if (s_bLogTime)
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            struct tm tm;
            localtime_r(&tv.tv_sec, &tm);
#ifdef __HuaweiLite__
            dprintf("[%02d:%02d:%02d:%03ld %s-%s]:%s[%d]:", tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / 1000, s_apszLOGLevel[enLevel], pszModule, pszFunc, u32Line);
#else
            fprintf(stdout, "[%02d:%02d:%02d:%03ld %s-%s]:%s[%d]:", tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / 1000, s_apszLOGLevel[enLevel], pszModule, pszFunc, u32Line);
#endif
        }
        else
        {
#ifdef __HuaweiLite__
            dprintf("[%s-%s]:%s[%d]:", s_apszLOGLevel[enLevel], pszModule, pszFunc, u32Line);
#else
            fprintf(stdout, "[%s-%s]:%s[%d]:", s_apszLOGLevel[enLevel], pszModule, pszFunc, u32Line);
#endif
        }

        va_start(args, pszFmt);
#ifdef __HuaweiLite__
        vprintf(pszFmt, args);
#else
        vfprintf(stdout, pszFmt, args);
#endif
        va_end(args );
    }

    return;
}

HI_VOID HI_LOG_Config(HI_BOOL bLogOn, HI_BOOL bLogTime, HI_LOG_LEVEL_E enLevel)
{
    s_bLogOn = bLogOn;
    s_bLogTime = bLogTime;
    s_enLogLevel = enLevel;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
