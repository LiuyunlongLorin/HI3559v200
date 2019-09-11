/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef __ASM_GENERIC_POLL_H
#define __ASM_GENERIC_POLL_H
#define POLLIN 0x0001
#define POLLPRI 0x0002
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010
#define POLLNVAL 0x0020
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define POLLRDNORM 0x0040
#define POLLRDBAND 0x0080
#ifndef POLLWRNORM
#define POLLWRNORM 0x0100
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
#ifndef POLLWRBAND
#define POLLWRBAND 0x0200
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#ifndef POLLMSG
#define POLLMSG 0x0400
#endif
#ifndef POLLREMOVE
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define POLLREMOVE 0x1000
#endif
#ifndef POLLRDHUP
#define POLLRDHUP 0x2000
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
#define POLLFREE 0x4000
#define POLL_BUSY_LOOP 0x8000
#ifndef __LITEOS__
struct pollfd {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  int fd;
  short events;
  short revents;
};
#else
/* poll events. */
typedef unsigned int pollevent_t;

#include "semaphore.h"
/* This is the Nuttx variant of the standard pollfd structure. */
struct pollfd
{
    int         fd;       /* The descriptor being polled */
    sem_t      *sem;      /* Pointer to semaphore used to post output event */
    pollevent_t events;   /* The input event flags */
    pollevent_t revents;  /* The output event flags */
    void   *priv;     /* For use by drivers */
};

struct tag_poll_wait_entry;

typedef struct tag_poll_wait_entry * poll_wait_head;

typedef struct tag_poll_table
{
    poll_wait_head wait;
    pollevent_t key;
} poll_table;

struct wait_queue_head;

typedef struct wait_queue_head wait_queue_head_t;

extern void notify_poll(wait_queue_head_t *wait_address);
extern void notify_poll_with_key(wait_queue_head_t *wait_address, pollevent_t key);
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
