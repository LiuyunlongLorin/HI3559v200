/*    $NetBSD: ioctl_compat.h,v 1.15 2005/12/03 17:10:46 christos Exp $    */

/*
 * Copyright (c) 1990, 1993
 *    The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *    @(#)ioctl_compat.h    8.4 (Berkeley) 1/21/94
 */

#ifndef _SYS_IOCTL_COMPAT_H_
#define    _SYS_IOCTL_COMPAT_H_

/*#include <sys/ttychars.h>*/
/*#include <sys/ttydev.h>*/

#if !defined(__mips__)
struct tchars {
    char    t_intrc;    /* interrupt */
    char    t_quitc;    /* quit */
    char    t_startc;    /* start output */
    char    t_stopc;    /* stop output */
    char    t_eofc;        /* end-of-file */
    char    t_brkc;        /* input delimiter (like nl) */
};

struct ltchars {
    char    t_suspc;    /* stop process signal */
    char    t_dsuspc;    /* delayed stop process signal */
    char    t_rprntc;    /* reprint line */
    char    t_flushc;    /* flush output (toggles) */
    char    t_werasc;    /* word erase */
    char    t_lnextc;    /* literal next character */
};

/*
 * Structure for TIOCGETP and TIOCSETP ioctls.
 */
#ifndef _SGTTYB_
#define    _SGTTYB_
struct sgttyb {
    char    sg_ispeed;        /* input speed */
    char    sg_ospeed;        /* output speed */
    char    sg_erase;        /* erase character */
    char    sg_kill;        /* kill character */
    short    sg_flags;        /* mode flags */
};
#endif
#endif

#ifdef USE_OLD_TTY
# undef  TIOCGETD
# define TIOCGETD    _IOR('t', 0, int)    /* get line discipline */
# undef  TIOCSETD
# define TIOCSETD    _IOW('t', 1, int)    /* set line discipline */
#else
# define OTIOCGETD    _IOR('t', 0, int)    /* get line discipline */
# define OTIOCSETD    _IOW('t', 1, int)    /* set line discipline */
#endif
#define    TIOCHPCL    _IO('t', 2)        /* hang up on last close */
#if !defined(__mips__)
#define    TIOCGETP    _IOR('t', 8,struct sgttyb)/* get parameters -- gtty */
#define    TIOCSETP    _IOW('t', 9,struct sgttyb)/* set parameters -- stty */
#define    TIOCSETN    _IOW('t',10,struct sgttyb)/* as above, but no flushtty*/
#endif
#define    TIOCSETC    _IOW('t',17,struct tchars)/* set special characters */
#define    TIOCGETC    _IOR('t',18,struct tchars)/* get special characters */
#define    TIOCLBIS    _IOW('t', 127, int)    /* bis local mode bits */
#define    TIOCLBIC    _IOW('t', 126, int)    /* bic local mode bits */
#define    TIOCLSET    _IOW('t', 125, int)    /* set entire local mode word */
#define    TIOCLGET    _IOR('t', 124, int)    /* get local modes */
#define        LCRTBS        (CRTBS>>16)
#define        LPRTERA        (PRTERA>>16)
#define        LCRTERA        (CRTERA>>16)
#define        LTILDE        (TILDE>>16)
#define        LMDMBUF        (MDMBUF>>16)
#define        LLITOUT        (LITOUT>>16)
#define        LTOSTOP        (TOSTOP>>16)
#define        LFLUSHO        (FLUSHO>>16)
#define        LNOHANG        (NOHANG>>16)
#define        LCRTKIL        (CRTKIL>>16)
#define        LPASS8        (PASS8>>16)
#define        LCTLECH        (CTLECH>>16)
#define        LPENDIN        (PENDIN>>16)
#define        LDECCTQ        (DECCTQ>>16)
#define        LNOFLSH        (NOFLSH>>16)
#if !defined(__mips__)
#define    TIOCSLTC    _IOW('t',117,struct ltchars)/* set local special chars*/
#define    TIOCGLTC    _IOR('t',116,struct ltchars)/* get local special chars*/
#endif
#define OTIOCCONS    _IO('t', 98)    /* for hp300 -- sans int arg */
#define    OTTYDISC    0
#define    NETLDISC    1
#define    NTTYDISC    2

#endif /* !_SYS_IOCTL_COMPAT_H_ */
