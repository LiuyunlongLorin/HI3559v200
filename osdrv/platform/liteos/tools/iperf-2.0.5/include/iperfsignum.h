/* Signal number definitions.  Linux version.
   Copyright (C) 1995, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

//#ifdef    _SIGNAL_H

/* Fake signal functions.  */

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef SIG_ERR
#define SIG_ERR ((__sighandler_t) -1)        /* Error return.  */
#endif
#ifndef SIG_DFL
#define SIG_DFL ((__sighandler_t) 0)        /* Default action.  */
#endif
#ifndef SIG_IGN
#define SIG_IGN ((__sighandler_t) 1)        /* Ignore signal.  */
#endif

#ifdef __USE_UNIX98
# define SIG_HOLD    ((__sighandler_t) 2)    /* Add signal to hold mask.  */
#endif


#define SIGHUP         1    /* Hangup (POSIX).  */
#ifndef SIGINT
#define SIGINT         2    /* Interrupt (ANSI).  */
#endif
#define SIGQUIT         3    /* Quit (POSIX).  */
#ifndef SIGILL
#define SIGILL         4    /* Illegal instruction (ANSI).  */
#endif
#define SIGTRAP         5    /* Trace trap (POSIX).  */
#define SIGIOT         6    /* IOT trap (4.2 BSD).  */
#ifndef SIGABRT
#define SIGABRT         6    /* Abort (ANSI).  */
#endif
#define SIGEMT         7
#ifndef SIGFPE
#define SIGFPE         8    /* Floating-point exception (ANSI).  */
#endif
#define SIGKILL         9    /* Kill, unblockable (POSIX).  */
#define SIGBUS        10    /* BUS error (4.2 BSD).  */
#ifndef SIGSEGV
#define SIGSEGV        11    /* Segmentation violation (ANSI).  */
#endif
#define SIGSYS        12
#define SIGPIPE        13    /* Broken pipe (POSIX).  */
#define SIGALRM        14    /* Alarm clock (POSIX).  */
#ifndef SIGTERM
#define SIGTERM        15    /* Termination (ANSI).  */
#endif
#ifndef SIGUSR1
#define SIGUSR1        16    /* User-defined signal 1 (POSIX).  */
#endif
#ifndef SIGUSR2
#define SIGUSR2        17    /* User-defined signal 2 (POSIX).  */
#endif
#define SIGCHLD        18    /* Child status has changed (POSIX).  */
#define SIGCLD        18    /* Same as SIGCHLD (System V).  */
#define SIGPWR        19    /* Power failure restart (System V).  */
#define SIGWINCH    20    /* Window size change (4.3 BSD, Sun).  */
#define SIGURG        21    /* Urgent condition on socket (4.2 BSD).  */
#define SIGIO        22    /* I/O now possible (4.2 BSD).  */
#define SIGPOLL        22    /* Pollable event occurred (System V).  */
#define SIGSTOP        23    /* Stop, unblockable (POSIX).  */
#define SIGTSTP        24    /* Keyboard stop (POSIX).  */
#define SIGCONT        25    /* Continue (POSIX).  */
#define SIGTTIN        26    /* Background read from tty (POSIX).  */
#define SIGTTOU        27    /* Background write to tty (POSIX).  */
#define SIGVTALRM    28    /* Virtual alarm clock (4.2 BSD).  */
#define SIGPROF        29    /* Profiling alarm clock (4.2 BSD).  */
#define SIGXCPU        30    /* CPU limit exceeded (4.2 BSD).  */
#define SIGXFSZ        31    /* File size limit exceeded (4.2 BSD).  */


#define _NSIG        128    /* Biggest signal number + 1
                   (including real-time signals).  */

#define SIGRTMIN    (__libc_current_sigrtmin ())
#define SIGRTMAX    (__libc_current_sigrtmax ())

/* These are the hard limits of the kernel.  These values should not be
   used directly at user level.  */
#define __SIGRTMIN    32
#define __SIGRTMAX    (_NSIG - 1)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

//#endif    /* <signal.h> included.  */
