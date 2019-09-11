#include "asm/semaphore.h"
#include "linux/wait.h"
#include "poll.h"

int _sema_init(losMutexDef_t *sem,unsigned int value)
{
    UINT32 puwSemHandle;
    UINT32 uwRet;


    if (NULL == sem)
    {
        return -1;
    }

    if (value > OS_SEM_COUNT_MAX)
    {
        return -1;
    }

    uwRet = LOS_SemCreate(value, &puwSemHandle);

    if (uwRet != LOS_OK)
    {
        return -1;
    }

    sem->sem = GET_SEM(puwSemHandle);

    return(0);
}


int _sema_lock(losMutexDef_t *sem)
{
    int retval = 0;

    if ((NULL == sem) || (NULL == sem->sem))
    {
        return -1;
    }

    if((AARCHPTR)sem->sem == 0xffffffff)
    {
        (VOID)_sema_init(sem,sem->count);
    }

    retval = LOS_SemPend(sem->sem->usSemID, LOS_WAIT_FOREVER);

    if(retval == LOS_OK)
        return 0;

    retval = -1;

    return(retval);
}

 int _sema_trylock(losMutexDef_t *sem)
{
    int retval = 0;


    if ((NULL == sem) || (NULL == sem->sem))
    {
        return -1;
    }

    // Check that whether the semaphore is waited or not
    if(!LOS_ListEmpty(&sem->sem->stSemList) || ( sem->sem->uwSemCount <= 0 ))
    {
        return -1;
    }
    retval = LOS_SemPend(sem->sem->usSemID, LOS_WAIT_FOREVER);
    if(retval == LOS_OK)
        return 0;

    retval = -1;

    return (retval);
}

 int _sema_unlock  (losMutexDef_t *sem)
{
    UINT32    uwRet;

    if ((NULL == sem) || (NULL == sem->sem))
    {
        return -1;
    }

    uwRet = LOS_SemPost(sem->sem->usSemID);

    if (uwRet != LOS_OK)
    {
        return -1;
    }

    return 0;
}

int _sema_destory(losMutexDef_t *sem)
{
    UINT32 uwRet;

    if((NULL == sem) || (NULL == sem->sem))
    {
        return -1;
    }
    if((AARCHPTR)sem->sem == 0xffffffff)
    {
        return -1;
    }
    uwRet = LOS_SemDelete(sem->sem->usSemID);

    if (uwRet != LOS_OK)
    {
        return -1;
    }

    return 0;

}

void __init_waitqueue_head(wait_queue_head_t *wait)
{
    if (NULL == wait)
    {
        return;
    }
    (VOID)LOS_EventInit(&wait->stEvent);
    (VOID)pthread_mutex_init(&wait->mutex, (const pthread_mutexattr_t *)NULL);
    LOS_ListInit(&wait->poll_queue);
}

void __wake_up_interruptible(wait_queue_head_t *wait)
{
    if (NULL == wait)
    {
        return;
    }
    (VOID)LOS_EventWrite(&wait->stEvent, 0x1);
    notify_poll(wait);
}

void __wake_up_interruptible_poll(wait_queue_head_t *wait, pollevent_t key)
{
    if (NULL == wait)
    {
        return;
    }
    (VOID)LOS_EventWrite(&wait->stEvent, 0x1);
    notify_poll_with_key(wait, key);
}
// -------------------------------------------------------------------------
// EOF ipc.c

