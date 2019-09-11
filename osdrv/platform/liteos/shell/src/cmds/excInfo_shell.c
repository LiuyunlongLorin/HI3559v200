#include "shcmd.h"
#include "los_memory.h"
#ifdef LOSCFG_SHELL_EXCINFO
extern log_read_write_fn  g_pfnExcInfoRW;
extern UINT32 g_uwRecordAddr;
extern UINT32 g_uwRecordSpace;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */
INT32 osShellCmdReadExcInfo(INT32 argc, CHAR **argv)
{
     CHAR *pBuf = (CHAR*)LOS_MemAlloc((void *)OS_SYS_MEM_ADDR, g_uwRecordSpace + 1);
     if(pBuf == NULL)
     {
         return LOS_NOK;
     }
     pBuf[g_uwRecordSpace] = '\0';
     if(g_pfnExcInfoRW != NULL)
     {
         g_pfnExcInfoRW(g_uwRecordAddr, g_uwRecordSpace, 1, pBuf);
     }
     PRINTK("%s\n", pBuf);
     LOS_MemFree((void *)OS_SYS_MEM_ADDR, pBuf);
     pBuf = NULL;
     return LOS_OK;
}
SHELLCMD_ENTRY(readExcInfo_shellcmd, CMD_TYPE_EX, "excInfo", 0, (CMD_CBK_FUNC)osShellCmdReadExcInfo);/*lint !e19*/
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif