/**
 * @file    hi_sync.c
 * @brief   common sync functions.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/10
 * @version   1.0

 */
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "hi_appcomm_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef O_NOATIME
#define O_NOATIME 0
#endif

static pthread_t s_UpdateTid = -1;
static sem_t s_SyncSem;

static HI_VOID* Update_Thread(HI_VOID* pvargv)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME,__func__,0,0,0);
    while(1)
    {
        MLOGD("before sync.\n");
        sync();
        MLOGD("after sync finished.\n");
        if(0!=sem_wait(&s_SyncSem))
        {
            break;
        }
    }
    sem_destroy(&s_SyncSem);
    s_UpdateTid = -1;
    return NULL;
}

HI_S32 HI_async(HI_VOID)
{
    if(-1==s_UpdateTid)
    {
        sem_init(&s_SyncSem,0,0);
        if(0!=pthread_create(&s_UpdateTid, NULL, Update_Thread, NULL))
        {
            MLOGE("create Update_Thread fail:%s\n",strerror(errno));
            return HI_EINTER;
        }
    }
    else
    {
        sem_post(&s_SyncSem);
    }
    return HI_SUCCESS;
}

HI_S32 HI_fsync(const HI_CHAR* pszPath)
{
    HI_S32 fd = open(pszPath, O_NOATIME|O_NOCTTY|O_RDONLY, 0666);
    if (-1==fd)
    {
        MLOGE("open %s fail:%s\n",pszPath,strerror(errno));
        return HI_EINVAL;
    }
    fsync(fd);
    close(fd);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
