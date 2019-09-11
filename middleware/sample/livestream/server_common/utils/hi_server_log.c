#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "hi_server_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static HI_SERVER_LOG_LEVEL_E g_senServerLogLevel = HI_SERVER_LOG_LEVEL_ERR;
static HI_SERVER_OUTPUT_FUNC g_spfnServerOutput = HI_NULL;

HI_S32 HI_SERVER_LOG_SetEnabledLevel(HI_SERVER_LOG_LEVEL_E enLevel)
{
    g_senServerLogLevel = enLevel;
    return HI_SUCCESS;
}

HI_S32 HI_SERVER_LOG_SetOutputFunc(HI_SERVER_OUTPUT_FUNC pFunc)
{
    g_spfnServerOutput = pFunc;
    return HI_SUCCESS;
}

HI_S32 HI_SERVER_LOG_Printf(const HI_CHAR* pszModName, HI_SERVER_LOG_LEVEL_E enLevel, const HI_CHAR* pszFmt, ...)
{
    if ( enLevel < g_senServerLogLevel )
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
