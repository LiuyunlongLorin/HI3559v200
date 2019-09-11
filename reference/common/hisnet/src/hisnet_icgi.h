#ifndef __HISNET_ICGI_H__
#define __HISNET_ICGI_H__
#include "hisnet.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MAX_CNT_CMD 4098 /* Command Line Maximum Length */

HI_S32 HI_CGI_CommandProc(char* strcmdin, PfnCgiStringProc pfnCgiStringGet, void* pData);

HI_S32 HI_CGI_RegisterCgi(cgi_cmd_trans_s* pstCgiCmd);

HI_S32 HI_CGI_RegisterDefaultCmd(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HISNET_ICGI_H__ */
