/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "sys/types.h"
#include "errno.h"
#include "semaphore.h"

#include "los_sys.ph"
#include "los_tick.ph"
#include "map_error.h"

// -------------------------------------------------------------------------
// Initialize semaphore to value.
// pshared is not supported in Huawei LiteOS.
int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    unsigned int puwSemHandle = 0;
    unsigned int ret;

    if (NULL == sem || (value > OS_SEM_COUNT_MAX)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemCreate(value, &puwSemHandle);

    if (map_errno(ret) != ENOERR) {
        return -1;
    }

    sem->sem= GET_SEM(puwSemHandle);

    return 0;
}

// -------------------------------------------------------------------------
// Destroy the semaphore.
int sem_destroy(sem_t *sem)
{
    unsigned int ret;
    unsigned int uwSemHandle;

    if (NULL != sem && NULL != sem->sem) {
        uwSemHandle = sem->sem->usSemID;
        if (uwSemHandle > SEM_NSEMS_MAX) {
            goto errout;
        }
        /* Check that the semaphore has no waiters */
        if (!LOS_ListEmpty(&sem->sem->stSemList)) {
            errno = EBUSY;
            return -1;
        }
    } else {
        goto errout;
    }

    ret = LOS_SemDelete(uwSemHandle);

    if (map_errno(ret) != ENOERR) {
        goto errout;
    }

    return 0;

errout:
    errno = EINVAL;
    return -1;
}

// -------------------------------------------------------------------------
// Decrement value if >0 or wait for a post.
int sem_wait(sem_t *sem)
{
    unsigned int ret;

    if (NULL == sem || NULL == sem->sem) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemPend(sem->sem->usSemID, LOS_WAIT_FOREVER);

    if (map_errno(ret) == ENOERR) {
        return 0;
    } else {
        return -1;
    }
}

// -------------------------------------------------------------------------
// Decrement value if >0, return -1 if not.
int sem_trywait(sem_t *sem)
{
    unsigned int ret;

    if (NULL == sem || NULL == sem->sem) {
        errno = EINVAL;
        return -1;
    }

    /* Check that whether the semaphore is waited or not */
    if (!LOS_ListEmpty(&sem->sem->stSemList) || (sem->sem->uwSemCount <= 0)) {
        errno = EAGAIN;
        return -1;
    }

    ret = LOS_SemPend(sem->sem->usSemID, LOS_WAIT_FOREVER);

    if (map_errno(ret) == ENOERR) {
        return 0;
    } else {
        return -1;
    }
}

int sem_timedwait(sem_t *sem, const struct timespec *timeout)
{
    unsigned int ret;
    unsigned long ms = 0;
    unsigned long tick_cnt = 0;

    if (NULL == sem || NULL == sem->sem) {
        errno = EINVAL;
        return -1;
    }

    /* Check it as per Posix */
    if (timeout == NULL ||
        timeout->tv_sec < 0 ||
        timeout->tv_nsec < 0 ||
        timeout->tv_nsec > 999999999L) {
        errno = EINVAL;
        return -1;
    }

    ms = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000;
    tick_cnt = LOS_MS2Tick(ms);

    if ((timeout->tv_nsec) && (tick_cnt == 0))
    {
        tick_cnt = 1;
    }

    ret = LOS_SemPend(sem->sem->usSemID, tick_cnt);

    if (map_errno(ret) == ENOERR) {
        return 0;
    } else {
        return -1;
    }
}

// -------------------------------------------------------------------------
// Increment value and wake a waiter if one is present.
int sem_post(sem_t *sem)
{
    unsigned int ret;

    if (NULL == sem || NULL == sem->sem) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemPost(sem->sem->usSemID);

    if (map_errno(ret) != ENOERR) {
        return -1;
    }

    return 0;
}

// -------------------------------------------------------------------------
// Get current value
int sem_getvalue(sem_t *sem, int *sval)
{
    int wVal;

    if (NULL == sem || NULL == sval) {
        errno = EINVAL;
        return -1;
    }
    wVal = sem->sem->uwSemCount;
    if (wVal < 0) {
        wVal = 0;
    }

    *sval = wVal;
    return 0;
}

// -------------------------------------------------------------------------
// Open an existing named semaphore, or create it.
sem_t *sem_open(const char *name, int oflag, ...)
{
    errno = ENOSYS;
    return (sem_t *)NULL;
}

// -------------------------------------------------------------------------
// Close descriptor for semaphore.
int sem_close(sem_t *sem)
{
    if (NULL == sem) {
        errno = EINVAL;
        return -1;
    }
    errno = ENOSYS;
    return -1;
}

// -------------------------------------------------------------------------
// Remove named semaphore
int sem_unlink(const char *name)
{
    errno = ENOSYS;
    return -1;
}
