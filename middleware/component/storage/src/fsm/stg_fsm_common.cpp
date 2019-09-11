#include "securec.h"
#include <unistd.h>
#include <sys/time.h>
#include "hi_type.h"
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>


#define FSM_MAX_PRINT_STR_LEN (1024)

HI_U64 STG_FSM_GetCurTimeUs()
{
    struct timespec ts = {0, 0};
    HI_U64 curT = 0;

#ifndef __HuaweiLite__
    (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    (HI_VOID)clock_gettime(CLOCK_REALTIME, &ts);
#endif
    curT = (((HI_U64)ts.tv_sec)*1000000) + (((HI_U64)ts.tv_nsec)/1000);

    return (HI_U64)curT;
}

HI_VOID STG_FSM_Cond_InitRelative(pthread_cond_t* pCond)
{
#ifndef __HuaweiLite__
    pthread_condattr_t condAttr;
    (HI_VOID)pthread_condattr_init(&condAttr);

    (HI_VOID)pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
    (HI_VOID)pthread_cond_init(pCond, &condAttr);
#else
    (HI_VOID)pthread_cond_init(pCond, HI_NULL);
#endif
}

#define NSEC2SEC_SCALE (1*1000*1000*1000ll)
#define USEC2NSEC_SCALE (1000ll)

HI_S32 STG_FSM_Cond_Timewait(pthread_cond_t* pCond, pthread_mutex_t* pMutex, HI_U64 u64delayUs)
{
    struct timespec ts;

#ifndef __HuaweiLite__
    (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
#endif

    HI_U64 u64DelayNs = u64delayUs*USEC2NSEC_SCALE;
    ts.tv_sec += (u64DelayNs/NSEC2SEC_SCALE);
    u64DelayNs = u64DelayNs%NSEC2SEC_SCALE;

    if(ts.tv_nsec + u64DelayNs > NSEC2SEC_SCALE)
    {
        ts.tv_sec ++;
        ts.tv_nsec = (ts.tv_nsec + u64DelayNs) - NSEC2SEC_SCALE;
    }
    else
    {
        ts.tv_nsec += u64DelayNs;
    }

    return pthread_cond_timedwait(pCond, pMutex, &ts);
}
