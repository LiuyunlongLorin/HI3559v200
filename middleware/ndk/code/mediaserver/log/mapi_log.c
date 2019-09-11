/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_log.c
 * @brief   log server function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "hi_mapi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static HI_MAPI_LOG_LEVEL_E g_senEnabledLevel = HI_MAPI_LOG_LEVEL_ERR;
static HI_OUTPUT_FUNC g_spfnOutput = HI_NULL;

static HI_CHAR g_szModeNames[HI_MAPI_MOD_BUTT][HI_MAPI_MODE_NAME_LEN] = {
    "SYS_S",
    "VCAP_S",
    "VPROC_S",
    "VENC_S",
    "ACAP_S",
    "AENC_S",
    "AO_S",
    "DISP_S",
    "HDMI_S",
    "VDEC_S",
    "PM_S"
};

static inline HI_CHAR *MAPI_GetModeNameById(HI_MAPI_MOD_ID_E enModId)
{
    return g_szModeNames[enModId];
}

HI_S32 HI_MAPI_LOG_SetEnabledLevel(HI_MAPI_LOG_LEVEL_E enLevel)
{
    g_senEnabledLevel = enLevel;
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_LOG_SetOutputFunc(HI_OUTPUT_FUNC pFunc)
{
    g_spfnOutput = pFunc;
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_LOG_Printf(HI_MAPI_MOD_ID_E enModId, HI_MAPI_LOG_LEVEL_E enLevel, const HI_CHAR *pszFmt, ...)
{
    const HI_CHAR *pszModName = MAPI_GetModeNameById(enModId);
    if (enLevel < g_senEnabledLevel) {
        return HI_SUCCESS;
    }

    if (!pszModName || !pszFmt) {
        return HI_FAILURE;
    }

    va_list args;
    if (!g_spfnOutput) {
        printf("[%s] ", pszModName);
        va_start(args, pszFmt);
        vprintf(pszFmt, args);
        va_end(args);
    } else {
        g_spfnOutput("[%s] ", pszModName);
        va_start(args, pszFmt);
        g_spfnOutput(pszFmt, args);
        va_end(args);
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_LOG_FormatPrintf(HI_MAPI_LOG_LEVEL_E enLevel, const HI_CHAR *pszFmt, ...)
{
    if (enLevel < g_senEnabledLevel) {
        return HI_SUCCESS;
    }

    if (!pszFmt) {
        return HI_FAILURE;
    }

    va_list args;
    if (!g_spfnOutput) {
        va_start(args, pszFmt);
        vprintf(pszFmt, args);
        va_end(args);
    } else {
        va_start(args, pszFmt);
        g_spfnOutput(pszFmt, args);
        va_end(args);
    }
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
