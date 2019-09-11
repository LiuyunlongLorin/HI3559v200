#ifndef __HI_THTTPD_H__
#define __HI_THTTPD_H__

#include <semaphore.h>
#include <sys/prctl.h>
#include "timers.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#define WEBSERVER_NO_DISTRIBLINK 200//when PTR_FUNC_DistribLink don't process pMsgBuffAddr, return this value

typedef int (*PTR_FUNC_DistribLink)(int s32LinkFd,char* pMsgBuffAddr,unsigned int u32MsgLen);
int HI_THTTPD_RegisterDistribLink(PTR_FUNC_DistribLink pfunDistribLink);

void* thttpd_start_main(void* pvParam);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_THTTPD_H__ */


