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

#include "pthread.h"
#include "pprivate.h"
#include "errno.h"

//-----------------------------------------------------------------------------
// Thread attribute handling.

//-----------------------------------------------------------------------------
// Initialize attributes object with default attributes:
// detachstate          == PTHREAD_CREATE_JOINABLE
// scope                == PTHREAD_SCOPE_SYSTEM
// inheritsched         == PTHREAD_INHERIT_SCHED
// schedpolicy          == SCHED_OTHER
// schedparam           == unset
// stackaddr            == unset
// stacksize            == 0
int pthread_attr_init(pthread_attr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }

    attr->detachstate                 = PTHREAD_CREATE_JOINABLE;
    attr->schedpolicy                 = SCHED_RR;
    attr->schedparam.sched_priority   = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    attr->inheritsched                = PTHREAD_INHERIT_SCHED;
    attr->scope                       = PTHREAD_SCOPE_SYSTEM;
    attr->stackaddr_set               = 0;
    attr->stackaddr                   = NULL;
    attr->stacksize_set               = 1;
    attr->stacksize                   = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Destroy thread attributes object
int pthread_attr_destroy(pthread_attr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }

    /* Nothing to do here... */
    return ENOERR;
}

//-----------------------------------------------------------------------------
// Set the detachstate attribute
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
    if ((attr != NULL) && (detachstate == PTHREAD_CREATE_JOINABLE || detachstate == PTHREAD_CREATE_DETACHED)) {
        attr->detachstate = (unsigned int)detachstate;
        return ENOERR;
    }

    return EINVAL;
}

//-----------------------------------------------------------------------------
// Get the detachstate attribute
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
    if (attr == NULL || detachstate == NULL) {
        return EINVAL;
    }

    *detachstate = (int)attr->detachstate;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Set scheduling contention scope
int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
    if (attr == NULL) {
        return EINVAL;
    }

    if (scope == PTHREAD_SCOPE_SYSTEM) {
        attr->scope = (unsigned int)scope;
        return ENOERR;
    }

    if (scope == PTHREAD_SCOPE_PROCESS) {
        return ENOTSUP;
    }

    return EINVAL;
}

//-----------------------------------------------------------------------------
// Get scheduling contention scope
int pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
    if (attr == NULL || scope == NULL) {
        return EINVAL;
    }

    *scope = (int)attr->scope;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Set scheduling inheritance attribute
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit)
{
    if ((attr != NULL) && (inherit == PTHREAD_INHERIT_SCHED || inherit == PTHREAD_EXPLICIT_SCHED))
    {
        attr->inheritsched = (unsigned int)inherit;
        return ENOERR;
    }

    return EINVAL;
}

//-----------------------------------------------------------------------------
// Get scheduling inheritance attribute
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inherit)
{
    if (attr == NULL || inherit == NULL) {
        return EINVAL;
    }

    *inherit = (int)attr->inheritsched;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Set scheduling policy
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
    if ((attr != NULL) && (policy == SCHED_RR)) {
        attr->schedpolicy = SCHED_RR;
        return ENOERR;
    }

    return EINVAL;
}

//-----------------------------------------------------------------------------
// Get scheduling policy
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
    if (attr == NULL || policy == NULL) {
        return EINVAL;
    }

    *policy = (int)attr->schedpolicy;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Set scheduling parameters
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
    if (attr == NULL || param == NULL || (param->sched_priority > OS_TASK_PRIORITY_LOWEST)) {
        return EINVAL;
    }

    attr->schedparam = *param;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Get scheduling parameters

int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
    if (attr == NULL || param == NULL) {
        return EINVAL;
    }

    *param = attr->schedparam;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Set starting address of stack. Whether this is at the start or end of
// the memory block allocated for the stack depends on whether the stack
// grows up or down.
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
    if (attr == NULL) {
        return EINVAL;
    }

    attr->stackaddr_set   = 1;
    attr->stackaddr       = stackaddr;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Get any previously set stack address.
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
    if ((attr != NULL && stackaddr != NULL) && (attr->stackaddr_set)) {
        *stackaddr = attr->stackaddr;
        return ENOERR;
    }

    return EINVAL; /* Stack address not set, return EINVAL. */
}

//-----------------------------------------------------------------------------
// Set minimum creation stack size.
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    /* Reject inadequate stack sizes */
    if (attr == NULL || stacksize < PTHREAD_STACK_MIN) {
        return EINVAL;
    }

    attr->stacksize_set = 1;
    attr->stacksize     = stacksize;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Get current minimal stack size.
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
    /* Reject attempts to get a stack size when one has not been set. */
    if (attr == NULL || stacksize == NULL || (!attr->stacksize_set)) {
        return EINVAL;
    }

    *stacksize = attr->stacksize;

    return ENOERR;
}
