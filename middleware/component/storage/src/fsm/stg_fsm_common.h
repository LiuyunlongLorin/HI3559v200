#ifndef __FSM_COMMON_H__
#define __FSM_COMMON_H__

#include <stdarg.h>
#include <pthread.h>
#include "hi_type.h"
#include "stg_log.h"

#define MOD_NAME_FSM  "FSM"


#define FSM_CHECK_NULL(condition) \
do \
{ \
    if(condition == NULL) \
    { \
        printf(""#condition" is NULL error %s %d\n", __FUNCTION__, __LINE__);  \
        return HI_FAILURE; \
    }\
}while(0)


#define CHECK_FSM_HANDLE_EXIST(checkFunc, handle) \
    do \
    { \
        if(!checkFunc(handle))  \
        { \
            printf("invalid FSM handle %p, not exist \n", handle); \
            return HI_FAILURE; \
        } \
    }while(0)

#define FSM_LOCK(mutex) \
do \
{ \
    (void)pthread_mutex_lock(&mutex); \
}while(0)


#define FSM_UNLOCK(mutex) \
do \
{ \
    (void)pthread_mutex_unlock(&mutex); \
}while(0)


#define FSM_COND_WAIT(cond, mutex) \
do \
{ \
    (void)pthread_cond_wait(&cond, &mutex); \
}while(0)

#define FSM_COND_SIGNAL(cond) \
do \
{ \
    (void)pthread_cond_signal(&cond); \
}while(0)


HI_U64 STG_FSM_GetCurTimeUs();

HI_VOID STG_FSM_Cond_InitRelative(pthread_cond_t* pCond);

HI_S32 STG_FSM_Cond_Timewait(pthread_cond_t* pCond, pthread_mutex_t* pMutex, HI_U64 u64delayUs);

#endif /*__FSM_COMMON_H__*/
