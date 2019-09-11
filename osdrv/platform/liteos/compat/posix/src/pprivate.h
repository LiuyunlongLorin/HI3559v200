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

#ifndef _HWLITEOS_POSIX_PPRIVATE_H
#define _HWLITEOS_POSIX_PPRIVATE_H

#include "sys/types.h"

#include "los_sem.ph"
#include "los_task.ph"

#include "pthread.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/* Thread control data structure
   Per-thread information needed by POSIX
   */
typedef struct
{
    pthread_attr_t      attr;               /* Current thread attributes */

    pthread_t           id;                 /* My thread ID */
    LOS_TASK_CB         *task;              /* pointer to Huawei LiteOS thread object */
    char                name[20];           /* name string for debugging */
    UINT8               state;              /* Thread state */

    UINT8               cancelstate;        /* Cancel state of thread */
    volatile UINT8      canceltype;         /* Cancel type of thread */
    volatile UINT8      canceled;           /* pending cancel flag */
    struct pthread_cleanup_buffer *cancelbuffer;  /* stack of cleanup buffers */

    SEM_CB_S            *joiner;            /* sem for pthread_join */

    UINT32              freestack;          /* stack malloced, must be freed */
    UINT32              stackmem;           /* base of stack memory area
                                               only valid if freestack == true
                                               */
    /* Per-thread data table pointer */
    void                **thread_data;
} _pthread_data;

/* Values for the state field. These are solely concerned with the
   states visible to POSIX. The thread's run state is stored in the
   eCos thread object.
   */
/* Note: numerical order here is important, do not rearrange. */

#define PTHREAD_STATE_FREE      0       /* This structure is free for reuse */
#define PTHREAD_STATE_DETACHED  1       /* The thread is running but detached */
#define PTHREAD_STATE_RUNNING   2       /* The thread is running and will wait */
                                        /* to join when it exits */
#define PTHREAD_STATE_JOIN      3       /* The thread has exited and is waiting */
                                        /* to be joined */
#define PTHREAD_STATE_EXITED    4       /* The thread has exited and is ready to */
                                        /* be reaped */
#define   PTHREAD_STATE_ALRDY_JOIN 5 /*The thread state is in join*/

#define PTHREAD_TESTCANCEL() pthread_testcancel()

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
