/* Copyright (C) 2012 mbed.org, MIT License
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**********************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which
 * might include those applicable to Huawei LiteOS of U.S. and the country in which you
 * are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance
 * with such applicable export control laws and regulations.
 **********************************************************************************/

#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

#include "lwip/opt.h"

#if NO_SYS == 0
#include "los_sem.h"
#include "pthread.h"
#include "los_typedef.h"
#include "semaphore.h"

#define LWIP_OFFSET_OF     LOS_OFF_SET_OF
typedef struct posix_sem sys_sem_t;

#define MBOX_NO_EXPAND    0
#define MBOX_AUTO_EXPAND  1

struct sys_mbox {
  int first, last;
  void **msgs;
  int mbox_size;
  unsigned char isFull;
  unsigned char isEmpty;
  unsigned char isAutoExpand;
  pthread_cond_t not_empty;
  pthread_cond_t not_full;
  pthread_mutex_t mutex;
};

struct sys_dual_mbox {
  int first, last;
  int first_p, last_p;
  void **msgs;
  void **msgs_priority;
  int mbox_size;
  int mbox_size_priority;
  int mbox_used_priority;
  int mbox_used;
  int mbox_total_used;
  int isFull;
  int isEmpty;
  pthread_cond_t not_empty;
  pthread_cond_t not_full;
  pthread_mutex_t mutex;
};
typedef struct sys_dual_mbox * sys_dual_mbox_t;

typedef struct sys_mbox * sys_mbox_t;

struct sys_thread {
  struct sys_thread *next;
  pthread_t pthread;
};

typedef unsigned int sys_thread_t;

#define sys_sem_valid(x)        (((*x).sem == NULL) ? 0 : 1)
#define sys_sem_set_invalid(x)  ( (*x).sem = NULL)


#define SYS_MBOX_NULL               (NULL)
#define sys_mbox_valid(mbox) (((mbox) != NULL) && (*(mbox) != NULL))
#define sys_mbox_set_invalid(mbox) do { if((mbox) != NULL) { *(mbox) = NULL; }}while(0)

/* Note: Please make sure the mbox passed is an valid pointer */
#define sys_dual_mbox_valid(mbox) (*(mbox) != NULL)

// === PROTECTION ===
typedef int sys_prot_t;

typedef u32_t sys_mutex_t;

#if (MEM_MALLOC_DMA_ALIGN != 1)
static inline void *sys_align_malloc(u16_t length)
{
  return LOS_MemAllocAlign((void *)OS_SYS_MEM_ADDR, length, MEM_MALLOC_DMA_ALIGN);
}

static inline void sys_align_free(void *mem)
{
  (void)LOS_MemFree((void *)OS_SYS_MEM_ADDR, mem);
}
#endif

#else
#ifdef  __cplusplus
extern "C" {
#endif

/** \brief  Get the current systick time in milliSeconds
 *
 *  Returns the current systick time in milliSeconds. This function is only
 *  used in standalone systems.
 *
 *  /returns current systick time in milliSeconds
 */
u32_t sys_now(void);

#ifdef  __cplusplus
}
#endif
#endif

#endif /* __ARCH_SYS_ARCH_H__ */
