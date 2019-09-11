#ifndef __STORAGE_COMMON_H__
#define __STORAGE_COMMON_H__

#include "securec.h"
#include "hi_mw_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MODULE_STORAGE              "STORAGE"
#define STORAGE_MAX_THREAD_NAME_LENGTH   (16)
#define STG_THREAD_MUTEX_LOCK(pLock) \
    do \
    {\
        HI_S32 s32Ret = HI_SUCCESS;\
        s32Ret = pthread_mutex_lock(pLock);\
        if (0 != s32Ret)\
        {\
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  mutex lock Failed, and ret:%x, system errno(%d)!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);\
            return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;\
        }\
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
