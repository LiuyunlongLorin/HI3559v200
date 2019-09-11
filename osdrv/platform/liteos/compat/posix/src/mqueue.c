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

#include "mqueue.h"
#include "stdio.h"
#include "fcntl.h"
#include "string.h"
#include "los_memory.h"
#include "errno.h"
#include "los_task.h"
#include "limits.h"

#define FNONBLOCK   O_NONBLOCK

/* GLOBALS */
struct mqarray *g_squeuetable;
__attribute__((section(".mqcb"))) struct mqarray g_squeuetable_0;

/* LOCAL FUNCTIONS */
STATIC INLINE int mqname_check(const char *mq_name)
{
    if (!mq_name) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    if (strlen(mq_name) == 0) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    if (strlen(mq_name) > PATH_MAX - 1) {
        errno = ENAMETOOLONG;
        MQ_POSIX_DEBUG(0);
        return -1;
    }
    return 0;
}
STATIC INLINE UINT32 get_mqcb_by_qid(UINT32 qid, QUEUE_CB_S **qcb)
{
    UINT32 int_save;
    if(qcb == NULL)
    {
        errno = EINVAL;
        return LOS_ERRNO_QUEUE_READ_PTR_NULL;
    }
    if (qid >= LOSCFG_BASE_IPC_QUEUE_LIMIT) {
        return LOS_ERRNO_QUEUE_INVALID;
    }

    int_save = LOS_IntLock();

    *qcb = (QUEUE_CB_S *)GET_QUEUE_HANDLE(qid);

    LOS_IntRestore(int_save);

    return LOS_OK;
}

STATIC INLINE struct mqarray *get_mqcb_by_name(const char *name)
{
    unsigned int index;
    struct mqarray *ptr_mqcb = (struct mqarray *)NULL;

    for (index = 0; index < LOSCFG_BASE_IPC_QUEUE_LIMIT; index++) {
        if (0 == strncmp(name, (const char *)(g_squeuetable[index].mq_name), PATH_MAX - 1)) {  /*lint !e668*/
            ptr_mqcb = &(g_squeuetable[index]);
            break;
        }
    }
    return ptr_mqcb;
}

static int do_mq_delete(struct mqarray *ptr_mqcb)
{
    INT32 ret;
    UINT32 int_save;

    if (NULL == ptr_mqcb) {
        errno = EINVAL;
        return -1;
    }

    int_save = LOS_IntLock();
    ptr_mqcb->mq_name[0] = '\0';
    ptr_mqcb->mqcb = (QUEUE_CB_S *)NULL;
    LOS_IntRestore(int_save);

    ret = LOS_QueueDelete(ptr_mqcb->mq_id);
    switch (ret) {
    case LOS_OK:
        return 0;
    case LOS_ERRNO_QUEUE_NOT_FOUND:
    case LOS_ERRNO_QUEUE_NOT_CREATE:
    case LOS_ERRNO_QUEUE_IN_TSKUSE:
    case LOS_ERRNO_QUEUE_IN_TSKWRITE:
        errno = EAGAIN;
        return -1;
    default:
        errno = EINVAL;
        return -1;
    }
}

mqd_t mq_open(const char *mq_name, int oflag, ...)
{
    struct mqarray *ptr_mqcb = (struct mqarray *)NULL;
    struct mqpersonal *ptr_private = (struct mqpersonal *)NULL;
    va_list ap;
    struct mq_attr *attr;
    struct mq_attr default_attr = {0, MQ_MAX_MSG_NUM, MQ_MAX_MSG_LEN};
    UINT32 umqerr;
    UINT32 mqid = 0;
    UINT32 int_save;

    if (-1 == mqname_check(mq_name)){
        MQ_POSIX_DEBUG(0);
        return (mqd_t)-1;
    }

    ptr_mqcb = get_mqcb_by_name(mq_name);

    if (NULL != ptr_mqcb) {
        if (O_EXCL == (oflag & O_EXCL)) {
            errno = EEXIST;
            MQ_POSIX_DEBUG(0);
            goto errout;
        }
    } else {
        if (O_CREAT != (oflag & O_CREAT)) {
            errno = ENOENT;
            MQ_POSIX_DEBUG(0);
            goto errout;
        }

        va_start(ap, oflag);
        (void)va_arg(ap, int); /* lint !e506 */
        attr = va_arg(ap, struct mq_attr *); /* lint !e506 */
        va_end(ap);

        if (!attr) {
            attr = &default_attr;
        } else {
            if (attr->mq_maxmsg < 0 || attr->mq_maxmsg > (long int)USHRT_MAX ||
                attr->mq_msgsize < 0 || attr->mq_msgsize > (long int)(USHRT_MAX - sizeof(UINT32))) {
                errno = EINVAL;
                MQ_POSIX_DEBUG(0);
                goto errout;
            }
        }

        umqerr = LOS_QueueCreate((char *)NULL, attr->mq_maxmsg, &mqid, 0, attr->mq_msgsize);
        switch (umqerr) {
            case LOS_OK:
                break;
            case LOS_ERRNO_QUEUE_CREAT_PTR_NULL:
            case LOS_ERRNO_QUEUE_PARA_ISZERO:
                errno = EINVAL;
                MQ_POSIX_DEBUG(0);
                goto errout;
            case LOS_ERRNO_QUEUE_CREATE_NO_MEMORY:
                errno = ENOSPC;
                MQ_POSIX_DEBUG(0);
                goto errout;
            case LOS_ERRNO_QUEUE_CB_UNAVAILABLE:
                errno = ENFILE;
                MQ_POSIX_DEBUG(0);
                goto errout;
            default:
                errno = EINVAL;
                MQ_POSIX_DEBUG(0);
                goto errout;
        }

        if (g_squeuetable[mqid].mqcb == NULL) {
            ptr_mqcb = &(g_squeuetable[mqid]);
            ptr_mqcb->mq_id = mqid;
        }

        if(ptr_mqcb == NULL)
        {
            errno = EINVAL;
            MQ_POSIX_DEBUG(0);
            goto errout;
        }

        umqerr = get_mqcb_by_qid(ptr_mqcb->mq_id, &(ptr_mqcb->mqcb));
        if (umqerr != LOS_OK) {
            errno = ENOSPC;
            MQ_POSIX_DEBUG(0);
            goto errout;
        }

        ptr_mqcb->mq_personal = (struct mqpersonal *)LOS_MemAlloc(OS_SYS_MEM_ADDR, sizeof(struct mqpersonal));
        if (ptr_mqcb->mq_personal == NULL) {
            (void)LOS_QueueDelete(ptr_mqcb->mq_id);
            int_save = LOS_IntLock();
            ptr_mqcb->mqcb->pucQueue = (unsigned char *)NULL;
            ptr_mqcb->mqcb = (QUEUE_CB_S *)NULL;
            LOS_IntRestore(int_save);
            MQ_POSIX_DEBUG(0);
            errno = ENOSPC;
            goto errout;
        }

        (VOID)strncpy(ptr_mqcb->mq_name, mq_name, PATH_MAX - 1);
        ptr_mqcb->unlinkflag = FALSE;
        ptr_mqcb->mq_personal->mq_status = MQ_USE_MAGIC;

        ptr_mqcb->mq_personal->mq_next = (struct mqpersonal *)NULL;
        ptr_mqcb->mq_personal->mq_posixdes = ptr_mqcb;
        ptr_mqcb->mq_personal->mq_flags = oflag | (attr->mq_flags & FNONBLOCK);

        return (mqd_t)(ptr_mqcb->mq_personal);
    }

    /* already have the same name of g_squeuetable */
    if (ptr_mqcb->unlinkflag == TRUE) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }
    /* alloc mqprivate and add to mqarray */
    ptr_private = (struct mqpersonal *)LOS_MemAlloc(OS_SYS_MEM_ADDR, sizeof(struct mqpersonal));
    if (ptr_private == NULL) {
        errno = ENOSPC;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    ptr_private->mq_next = ptr_mqcb->mq_personal;
    ptr_mqcb->mq_personal = ptr_private;

    ptr_private->mq_posixdes = ptr_mqcb;
    ptr_private->mq_flags = oflag;
    ptr_private->mq_status = MQ_USE_MAGIC;

    return (mqd_t)ptr_private;
errout:
    return (mqd_t)-1;
}

int mq_close(mqd_t personal)
{
    int ret;
    struct mqarray *ptr_mqcb = (struct mqarray *)NULL;
    struct mqpersonal *ptr_private = (struct mqpersonal *)NULL;
    struct mqpersonal *ptr_ptemp = (struct mqpersonal *)NULL;

    if ((personal == (mqd_t)NULL) || (personal == (mqd_t)-1)) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    ptr_private = (struct mqpersonal *)personal;

    if (ptr_private->mq_status != MQ_USE_MAGIC) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    ptr_mqcb = ptr_private->mq_posixdes;
    if (ptr_mqcb->mq_personal == NULL) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    /* find the personal and remove */
    if (ptr_mqcb->mq_personal == ptr_private) {
        ptr_mqcb->mq_personal = ptr_private->mq_next;
    } else {
        for (ptr_ptemp = ptr_mqcb->mq_personal; ptr_ptemp->mq_next != NULL; ptr_ptemp = ptr_ptemp->mq_next) {
            if (ptr_ptemp->mq_next == ptr_private) {
                break;
            }
        }
        if (NULL == ptr_ptemp->mq_next) {
            errno = EBADF;
            MQ_POSIX_DEBUG(0);
            return -1;
        }
        ptr_ptemp->mq_next = ptr_private->mq_next;
    }
    /* flag no use */
    ptr_private->mq_status = 0;

    /* free the personal */
    ret = LOS_MemFree(OS_SYS_MEM_ADDR, ptr_private);
    if(ret != ENOERR){
        errno = EFAULT;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    if ((TRUE == ptr_mqcb->unlinkflag) && (NULL == ptr_mqcb->mq_personal)) {
        ret = do_mq_delete(ptr_mqcb);
    }

    return ret;
}

int mq_setattr(mqd_t personal, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
    struct mqpersonal *ptr_private = (struct mqpersonal *)NULL;

    if ((personal == (mqd_t)NULL) || (personal == (mqd_t)-1)) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    if (mqstat == NULL) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    ptr_private = (struct mqpersonal *)personal;

    if (ptr_private->mq_status != MQ_USE_MAGIC) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    if (NULL != omqstat) {
        (void)mq_getattr((mqd_t)ptr_private, omqstat);

    }

    LOS_TaskLock();
    ptr_private->mq_flags &= ~FNONBLOCK; /* clear */
    if ((mqstat->mq_flags & FNONBLOCK) == FNONBLOCK) {
    	ptr_private->mq_flags |= FNONBLOCK;
    }

    LOS_TaskUnlock();

    return 0;
}

int mq_getattr(mqd_t personal, struct mq_attr *mqstat)
{
    struct mqarray *ptr_mqcb = (struct mqarray *)NULL;
    struct mqpersonal *ptr_private = (struct mqpersonal *)NULL;

    if ((personal == (mqd_t)NULL) || (personal == (mqd_t)-1)) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    if (mqstat == NULL) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    ptr_private = (struct mqpersonal *)personal;
    if (ptr_private->mq_status != MQ_USE_MAGIC) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    ptr_mqcb = ptr_private->mq_posixdes;
    mqstat->mq_maxmsg = ptr_mqcb ->mqcb->usQueueLen;
    mqstat->mq_msgsize = ptr_mqcb ->mqcb->usQueueSize - sizeof(UINT32);
    mqstat->mq_curmsgs = ptr_mqcb ->mqcb->usReadWriteableCnt[OS_QUEUE_READ];
    mqstat->mq_flags = ptr_private->mq_flags;

    return 0;
}

int mq_unlink(const char *mq_name)
{
    int ret = 0;
    unsigned int index;
    struct mqarray *ptr_mqcb = (struct mqarray *)NULL;

    if (-1 == mqname_check(mq_name)){
        MQ_POSIX_DEBUG(0);
        return -1;
    }

    for (index = 0; index < LOSCFG_BASE_IPC_QUEUE_LIMIT; index++) {
        if (0 == strncmp(mq_name, g_squeuetable[index].mq_name, PATH_MAX - 1)) {
            ptr_mqcb = &g_squeuetable[index];
            break;
        }
    }

    if (NULL == ptr_mqcb) {
        errno = ENOENT;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (ptr_mqcb->mq_personal != NULL) {
        ptr_mqcb->unlinkflag = TRUE;
        MQ_POSIX_DEBUG(0);
    } else {
        ret = do_mq_delete(ptr_mqcb);
    }
    return ret;

errout:
    return -1;
}

/* not support the prio */
int mq_send(mqd_t personal, const char *msg_ptr, size_t msg_len, unsigned int msg_prio)
{
    UINT32 err;
    UINT32 time_out = LOS_WAIT_FOREVER;
    struct mqarray *ptr_mqcb = (struct mqarray *)NULL;
    struct mqpersonal *ptr_private = (struct mqpersonal *)NULL;

    if ((personal == (mqd_t)NULL) || (personal == (mqd_t)-1)) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (msg_ptr == NULL || msg_len == 0) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (msg_prio > MQ_PRIO_MAX - 1) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    ptr_private = (struct mqpersonal *)personal;
    if (ptr_private->mq_status != MQ_USE_MAGIC) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    ptr_mqcb = ptr_private->mq_posixdes;
    if (msg_len > (size_t)(ptr_mqcb->mqcb->usQueueSize - sizeof(UINT32))) {
        errno = EMSGSIZE;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    /* access */
    if ((O_WRONLY != (ptr_private->mq_flags & O_WRONLY)) && (O_RDWR != (ptr_private->mq_flags & O_RDWR))) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (ptr_private->mq_flags & FNONBLOCK) {
        time_out = LOS_NO_WAIT;
    }

    err = LOS_QueueWriteCopy(ptr_mqcb->mq_id, (VOID *)msg_ptr, (UINT32)msg_len, time_out);
    switch (err) {
    case LOS_OK:
        return 0;
    case LOS_ERRNO_QUEUE_INVALID:
    case LOS_ERRNO_QUEUE_WRITE_PTR_NULL:
    case LOS_ERRNO_QUEUE_WRITESIZE_ISZERO:
    case LOS_ERRNO_QUEUE_WRITE_SIZE_TOO_BIG:
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    case LOS_ERRNO_QUEUE_WRITE_IN_INTERRUPT:
        errno = EINTR;
        MQ_POSIX_DEBUG(0);
        goto errout;
    case LOS_ERRNO_QUEUE_TIMEOUT:
        errno = ETIMEDOUT;
        MQ_POSIX_DEBUG(0);
        goto errout;
    case LOS_ERRNO_QUEUE_ISFULL:
        errno = EAGAIN;
        MQ_POSIX_DEBUG(0);
        goto errout;
    default:
        goto errout;
    }

errout:
    return -1;
}

ssize_t mq_receive(mqd_t personal, char *msg_ptr, size_t msg_len, unsigned int *msg_prio)
{
    UINT32 err;
    UINT32 receive_len = 0;
    UINT32 time_out = LOS_WAIT_FOREVER;
    struct mqarray *ptr_mqcb = (struct mqarray *)NULL;
    struct mqpersonal *ptr_private = (struct mqpersonal *)NULL;

    if ((personal == (mqd_t)NULL) || (personal == (mqd_t)-1)) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (msg_ptr == NULL || msg_len == 0) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (msg_prio != NULL) {
        errno = EOPNOTSUPP;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    ptr_private = (struct mqpersonal *)personal;
    if (ptr_private->mq_status != MQ_USE_MAGIC) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
    	goto errout;
    }

    ptr_mqcb = ptr_private->mq_posixdes;
    if (msg_len < (size_t)(ptr_mqcb->mqcb->usQueueSize - sizeof(UINT32))) {
        errno = EMSGSIZE;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (O_WRONLY == (ptr_private->mq_flags & O_WRONLY)) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (ptr_private->mq_flags & FNONBLOCK) {
        time_out = LOS_NO_WAIT;
    }

    receive_len = msg_len;
    err = LOS_QueueReadCopy(ptr_mqcb->mq_id, (VOID *)msg_ptr, &receive_len, time_out);
    if (err == LOS_OK) {
        return (ssize_t)receive_len;
    }

    switch (err) {
    case LOS_ERRNO_QUEUE_TIMEOUT:
        errno = ETIMEDOUT;
        MQ_POSIX_DEBUG(0);
        goto errout;
    case LOS_ERRNO_QUEUE_READ_IN_INTERRUPT:
        errno = EINTR;
        MQ_POSIX_DEBUG(0);
        goto errout;
    case LOS_ERRNO_QUEUE_ISEMPTY:
        errno = EAGAIN;
        MQ_POSIX_DEBUG(0);
        goto errout;
    default:
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

errout:
    return (ssize_t)-1;
}

int mq_timedsend(mqd_t personal, const char *msg_ptr, size_t msg_len,
        unsigned int msg_prio, const struct timespec *abs_timeout)
{
    UINT32 err;
    UINT64 abs_ticks;
    struct mqarray *ptr_mqcb = (struct mqarray *)NULL;
    struct mqpersonal *ptr_private = (struct mqpersonal *)NULL;

    if ((personal == (mqd_t)NULL) || (personal == (mqd_t)-1)) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (msg_ptr == NULL || msg_len == 0 || abs_timeout == NULL) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (msg_prio > MQ_PRIO_MAX - 1) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    ptr_private = (struct mqpersonal *)personal;
    if (ptr_private->mq_status != MQ_USE_MAGIC) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    ptr_mqcb = ptr_private->mq_posixdes;
    if (msg_len > (size_t)(ptr_mqcb->mqcb->usQueueSize - sizeof(UINT32))) {
        errno = EMSGSIZE;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if ((O_WRONLY != (ptr_private->mq_flags & O_WRONLY)) &&
            (O_RDWR != (ptr_private->mq_flags & O_RDWR))) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (abs_timeout->tv_sec < 0 || abs_timeout->tv_nsec < 0 || abs_timeout->tv_nsec > 999999999L) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (ptr_private->mq_flags & FNONBLOCK) {
        abs_ticks = LOS_NO_WAIT;
    } else {
        abs_ticks = (UINT64)abs_timeout->tv_sec * LOSCFG_BASE_CORE_TICK_PER_SECOND /*lint !e571*/
                    + (abs_timeout->tv_nsec / 1000000) / (1000 / LOSCFG_BASE_CORE_TICK_PER_SECOND);
    }

    if ((abs_timeout->tv_nsec) && (abs_ticks == 0))
    {
        abs_ticks = 1;
    }
    else if (abs_ticks >= LOS_WAIT_FOREVER)
    {
        abs_ticks = LOS_WAIT_FOREVER;
    }

    err = LOS_QueueWriteCopy(ptr_mqcb->mq_id, (VOID *)msg_ptr, (UINT32)msg_len, (UINT32)abs_ticks);
    switch (err) {
    case LOS_OK:
        return 0;
    case LOS_ERRNO_QUEUE_WRITE_IN_INTERRUPT:
        errno = EINTR;
        MQ_POSIX_DEBUG(0);
        goto errout;
    case LOS_ERRNO_QUEUE_TIMEOUT:
        errno = ETIMEDOUT;
        MQ_POSIX_DEBUG(0);
        goto errout;
    case LOS_ERRNO_QUEUE_ISFULL:
        errno = EAGAIN;
        MQ_POSIX_DEBUG(0);
        goto errout;
    case LOS_ERRNO_QUEUE_INVALID:
    case LOS_ERRNO_QUEUE_WRITE_PTR_NULL:
    case LOS_ERRNO_QUEUE_WRITESIZE_ISZERO:
    case LOS_ERRNO_QUEUE_WRITE_SIZE_TOO_BIG:
    default:
       errno = EINVAL;
       MQ_POSIX_DEBUG(0);
       goto errout;
    }

errout:
    return -1;
}

ssize_t mq_timedreceive(mqd_t personal, char *msg_ptr, size_t msg_len,
        unsigned int *msg_prio, const struct timespec *abs_timeout)
{
    UINT32 err;
    UINT32 receive_len;
    UINT64 abs_ticks;
    struct mqarray *ptr_mqcb = (struct mqarray *)NULL;
    struct mqpersonal *ptr_private = (struct mqpersonal *)NULL;

    if ((personal == (mqd_t)NULL) || (personal == (mqd_t)-1)) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (msg_ptr == NULL || msg_len == 0 || abs_timeout == NULL) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (msg_prio != NULL) {
        errno = EOPNOTSUPP;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    ptr_private = (struct mqpersonal *)personal;
    if (ptr_private->mq_status != MQ_USE_MAGIC) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    ptr_mqcb = ptr_private->mq_posixdes;
    if (msg_len < (size_t)(ptr_mqcb->mqcb->usQueueSize - sizeof(UINT32))) {
        errno = EMSGSIZE;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (abs_timeout->tv_sec < 0 ||
            abs_timeout->tv_nsec < 0 ||
            abs_timeout->tv_nsec > 999999999L) {
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (O_WRONLY == (ptr_private->mq_flags & O_WRONLY)) {
        errno = EBADF;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

    if (ptr_private->mq_flags & FNONBLOCK) {
        abs_ticks = LOS_NO_WAIT;
    } else {
        abs_ticks = (UINT64)abs_timeout->tv_sec * LOSCFG_BASE_CORE_TICK_PER_SECOND /*lint !e571*/
                    + (abs_timeout->tv_nsec / 1000000) / (1000 / LOSCFG_BASE_CORE_TICK_PER_SECOND);
    }

    if ((abs_timeout->tv_nsec) && (abs_ticks == 0))
    {
        abs_ticks = 1;
    }
    else if (abs_ticks >= LOS_WAIT_FOREVER)
    {
        abs_ticks = LOS_WAIT_FOREVER;
    }

    receive_len = msg_len;
    err = LOS_QueueReadCopy(ptr_mqcb->mq_id, (VOID *)msg_ptr, &receive_len, (UINT32)abs_ticks);

    if (err == LOS_OK) {
        return (ssize_t)receive_len;
    }

    switch (err) {
    case LOS_ERRNO_QUEUE_TIMEOUT:
        errno = ETIMEDOUT;
        MQ_POSIX_DEBUG(0);
        goto errout;
    case LOS_ERRNO_QUEUE_READ_IN_INTERRUPT:
        errno = EINTR;
        MQ_POSIX_DEBUG(0);
        goto errout;
    case LOS_ERRNO_QUEUE_ISEMPTY:
        errno = EAGAIN;
        MQ_POSIX_DEBUG(0);
        goto errout;
    default:
        errno = EINVAL;
        MQ_POSIX_DEBUG(0);
        goto errout;
    }

errout:
    return -1;
}
