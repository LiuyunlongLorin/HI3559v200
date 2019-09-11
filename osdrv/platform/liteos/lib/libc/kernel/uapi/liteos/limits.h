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
#ifndef _LINUX_LIMITS_H
#define _LINUX_LIMITS_H

#include "stdint.h"
#include "sched.h"
#include "vfs_config.h"

#define NAME_MIN                            5
#define    CHAR_BIT    8            /* number of bits in a char */

#define    SCHAR_MAX    0x7f        /* max value for a signed char */
#define SCHAR_MIN    (-0x7f-1)    /* min value for a signed char */

#define    UCHAR_MAX    0xffU        /* max value for an unsigned char */
#ifdef __CHAR_UNSIGNED__
# define CHAR_MIN    0            /* min value for a char */
# define CHAR_MAX    0xff        /* max value for a char */
#else
# define CHAR_MAX    0x7f
# define CHAR_MIN    (-0x7f-1)
#endif

#define    USHRT_MAX    0xffffU                         /* max value for an unsigned short */
#define    SHRT_MAX    0x7fff                         /* max value for a short */
#define SHRT_MIN        (-0x7fff-1)             /* min value for a short */

#define    UINT_MAX    0xffffffffU                    /* max value for an unsigned int */
#define    INT_MAX        0x7fffffff                    /* max value for an int */
#define    INT_MIN        (-0x7fffffff-1)                /* min value for an int */

#define ULLONG_MAX    0xffffffffffffffffULL       /* max value for unsigned long long */
#define LLONG_MAX    0x7fffffffffffffffLL        /* max value for a signed long long */
#define LLONG_MIN    (-0x7fffffffffffffffLL-1)   /* min value for a signed long long */

/* A pointer is 4 bytes */
#define PTR_MIN                             (-PTR_MAX - 1)
#define PTR_MAX                             2147483647
#define UPTR_MAX                            4294967295U

/* New code should use sysconf(_SC_PAGE_SIZE) instead. */
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#ifndef PAGESIZE
#define  PAGESIZE  PAGE_SIZE
#endif

#define TZ_PATH_LENTGH                      100

#define _POSIX_CLK_TICK                     time((time_t *)NULL)

#define _POSIX_GETGR_R_SIZE_MAX             0

#define _POSIX_GETPW_R_SIZE_MAX             0

/* Maximum number of message queues open for a process.  */
#define _LITEOS_MQ_OPEN_MAX                  LOSCFG_BASE_IPC_QUEUE_LIMIT

/* Number of files one process can have open at once.  */
#ifdef LOSCFG_FS_VFS
#define OPEN_MAX                            CONFIG_NFILE_DESCRIPTORS
#else
#define OPEN_MAX                            255
#endif
#define _POSIX_PAGESIZE                     PAGE_SIZE

/* The number of data keys per process.  */
#define _POSIX_THREAD_KEYS_MAX              128

/* We support priority inheritence.  */
#define _POSIX_PTHREAD_PRIO_INHERIT         PTHREAD_PRIO_INHERIT

/* We support priority protection, though only for non-robust
     mutexes.  */
#define _POSIX_PTHREAD_PRIO_PROTECT         PTHREAD_PRIO_PROTECT

/* Thread process-shared synchronization is supported.  */
#define _POSIX_PTHREAD_PROCESS_SHARED       PTHREAD_PROCESS_SHARED

/* Maximum number of characters in a tty name.  */
#define _POSIX_TTY_NAME_MAX                 9

/* Largest value of a `ssize_t'.  */
#define _POSIX_SSIZE_MAX                    32767

/* Maximum clock resolution in nanoseconds.  */
#define _POSIX_CLOCKRES_MIN                 20000000

#define _POSIX_SSIZE_MIN                    -32768

#define SYS_CLK_TCK                         _POSIX_CLK_TICK
#define DELAYTIMER_MAX                      32
#define GETGR_R_SIZE_MAX                    _POSIX_GETGR_R_SIZE_MAX
#define GETPW_R_SIZE_MAX                    _POSIX_GETPW_R_SIZE_MAX
#define LOGIN_NAME_MAX                      9
#define MQ_OPEN_MAX                         _LITEOS_MQ_OPEN_MAX
#define SIGQUEUE_MAX                        32
#define STREAM_MAX                          8
#define PTHREAD_ATTR_STACKSIZE              LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE
#define PTHREAD_DESTRUCTOR_ITERATIONS       4
#define PTHREAD_KEYS_MAX                    _POSIX_THREAD_KEYS_MAX
#define PTHREAD_PRIORITY_SCHEDULING         SCHED_OTHER
#define PTHREAD_THREADS_MAX                 (LOSCFG_BASE_CORE_TSK_LIMIT + 1)
#define TIMERS                              LOSCFG_BASE_CORE_SWTMR
#define TIMER_MAX                           _POSIX_TIMER_MAX
#define TTY_NAME_MAX                        _POSIX_TTY_NAME_MAX
#define TZNAME_MAX                          TZ_PATH_LENTGH
#define POSIX_VERSION                       _POSIX_VERSION

#define PTHREAD_STACK_OVERHEAD              (0x180 + (PTHREAD_KEYS_MAX * sizeof(void *)))

#define SSIZE_MAX                           LONG_MAX

#define NR_OPEN            1024

#define NGROUPS_MAX        0    /* supplemental group IDs are available */
#define ARG_MAX       4096  /* # bytes of args + environ for exec() */
#define LINK_MAX         8      /* # links a file may have */
#define MAX_CANON        255    /* size of the canonical input queue */
#define MAX_INPUT        255    /* size of the type-ahead buffer */
#define NAME_MAX         255    /* # chars in a file name */
#define PATH_MAX        260     /* # chars in a path name including nul */
#define PIPE_BUF        512     /* # bytes in atomic write to a pipe */
#define XATTR_NAME_MAX   255    /* # chars in an extended attribute name */
#define XATTR_SIZE_MAX 65536    /* size of an extended attribute value (64k) */
#define XATTR_LIST_MAX 65536    /* size of extended attribute namelist (64k) */

#define RTSIG_MAX      8

#define CONFIG_NAME_MAX                     NAME_MAX
#define CONFIG_PATH_MAX                     PATH_MAX

#endif
