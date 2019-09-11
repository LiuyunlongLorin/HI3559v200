#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "dng_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static DNG_LOG_LEVEL_E g_senDngLogLevel = DNG_LOG_LEVEL_ERR;
static DNG_OUTPUT_FUNC g_spfnDngOutput = HI_NULL;

HI_S32 DNG_LOG_SetEnabledLevel(DNG_LOG_LEVEL_E enLevel)
{
    g_senDngLogLevel = enLevel;
    return HI_SUCCESS;
}

HI_S32 DNG_LOG_SetOutputFunc(DNG_OUTPUT_FUNC pFunc)
{
    g_spfnDngOutput = pFunc;
    return HI_SUCCESS;
}

HI_S32 DNG_LOG_Printf(const HI_CHAR *pszModName, DNG_LOG_LEVEL_E enLevel, const HI_CHAR *pszFmt, ...)
{
    if (enLevel < g_senDngLogLevel) {
        return HI_SUCCESS;
    }

    if (!pszModName || !pszFmt) {
        return HI_FAILURE;
    }

    va_list args;
    if (!g_spfnDngOutput) {
        printf("[%s] ", pszModName);
        va_start(args, pszFmt);
        vprintf(pszFmt, args);
        va_end(args);
    } else {
        g_spfnDngOutput("[%s] ", pszModName);
        va_start(args, pszFmt);
        g_spfnDngOutput(pszFmt, args);
        va_end(args);
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
