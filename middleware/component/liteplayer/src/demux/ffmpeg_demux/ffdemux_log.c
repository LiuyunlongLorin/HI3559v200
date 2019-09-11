#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "ffdemux_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MAX_PRINT_STR_LEN (1024)

static HI_FFDEMUX_LOG_LEVEL_E g_senffDemuxLogLevel = HI_FFDEMUX_LOG_LEVEL_ERR;
static HI_FFDEMUX_OUTPUT_FUNC g_spfnServerOutput = HI_NULL;

HI_S32 FFDEMUX_SetEnabledLevel(HI_FFDEMUX_LOG_LEVEL_E enLevel)
{
    g_senffDemuxLogLevel = enLevel;
    return HI_SUCCESS;
}

HI_S32 FFDEMUX_SetOutputFunc(HI_FFDEMUX_OUTPUT_FUNC pFunc)
{
    g_spfnServerOutput = pFunc;
    return HI_SUCCESS;
}

HI_S32 FFDEMUX_Printf(const HI_CHAR* pszModName, HI_FFDEMUX_LOG_LEVEL_E enLevel, const HI_CHAR* pszFmt, ...)
{
    HI_CHAR aszPrintStr[MAX_PRINT_STR_LEN] = {0};

    if ( enLevel < g_senffDemuxLogLevel )
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
        va_start( args, pszFmt );
        vsnprintf_s(aszPrintStr, MAX_PRINT_STR_LEN,MAX_PRINT_STR_LEN-1, pszFmt, args);
        va_end( args );
        printf("[%s] %s",  pszModName, aszPrintStr);
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
