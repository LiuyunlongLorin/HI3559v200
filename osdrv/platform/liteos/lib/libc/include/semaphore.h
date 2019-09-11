/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#ifndef __LITEOS__
#include <sys/cdefs.h>
#else
#include "time.h"
#include "los_sem.ph"
#endif

__BEGIN_DECLS

#ifndef __LITEOS__
struct timespec;

typedef struct {
  unsigned int count;
#ifdef __LP64__
  int __reserved[3];
#endif
} sem_t;
#else
#ifdef SEM_VALUE_MAX
#undef SEM_VALUE_MAX
/**
 * @ingroup semaphore
 * Maximum semaphore value.
 */
#define  SEM_VALUE_MAX      OS_SEM_COUNT_MAX
#endif

#ifdef SEM_NSEMS_MAX
#undef SEM_NSEMS_MAX
#endif

/**
 * @ingroup semaphore
 * Maximum number of supported semaphores.
 */
#define  SEM_NSEMS_MAX      LOSCFG_BASE_IPC_SEM_LIMIT

/**
 * @ingroup semaphore
 * API parameter sructure
 */

typedef struct posix_sem
{
    SEM_CB_S* sem;        /**< Semaphore attribute structure*/
} sem_t;
#endif

#define SEM_FAILED NULL

int sem_destroy(sem_t*);
int sem_getvalue(sem_t*, int*);
int sem_init(sem_t*, int, unsigned int);
int sem_post(sem_t*);
int sem_timedwait(sem_t*, const struct timespec*);
int sem_trywait(sem_t*);
int sem_wait(sem_t*);

/* These aren't actually implemented. */
sem_t* sem_open(const char*, int, ...);
int sem_close(sem_t*);
int sem_unlink(const char*);

__END_DECLS

#endif /* _SEMAPHORE_H */
