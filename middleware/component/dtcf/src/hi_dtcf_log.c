#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "hi_dtcf_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static HI_DTCF_LOG_LEVEL_E g_senDtcfLogLevel = HI_DTCF_LOG_LEVEL_ERR;
static HI_DTCF_OUTPUT_FUNC g_spfnServerOutput = HI_NULL;

HI_S32 HI_DTCF_LOG_SetEnabledLevel(HI_DTCF_LOG_LEVEL_E enLevel)
{
    g_senDtcfLogLevel = enLevel;
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_LOG_SetOutputFunc(HI_DTCF_OUTPUT_FUNC pFunc)
{
    g_spfnServerOutput = pFunc;
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_LOG_Printf(const HI_CHAR* pszModName, HI_DTCF_LOG_LEVEL_E enLevel, const HI_CHAR* pszFmt, ...)
{
    if ( enLevel < g_senDtcfLogLevel )
    {
        return HI_SUCCESS;
    }

    if (  !pszModName || !pszFmt)
    {
        return HI_FAILURE;
    }

    va_list args;
    if ( !g_spfnServerOutput )
    {
        printf("[%s] ",  pszModName);
        va_start( args, pszFmt );
        vprintf(pszFmt, args);
        va_end( args );
    }
    else
    {
        g_spfnServerOutput("[%s] ",  pszModName);
        va_start( args, pszFmt );
        g_spfnServerOutput(pszFmt, args);
        va_end( args );
    }

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
