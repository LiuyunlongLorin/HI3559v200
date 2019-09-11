/*    $OpenBSD: arc4random.c,v 1.50 2014/07/21 18:13:12 deraadt Exp $    */

/************************************************************************
 * Copyright (c) 1996, David Mazieres <dm@uun.org>
 * Copyright (c) 2008, Damien Miller <djm@openbsd.org>
 * Copyright (c) 2013, Markus Friedl <markus@openbsd.org>
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ************************************************************************/

/************************************************************************
 * ChaCha based random number generator for OpenBSD.
 ************************************************************************/

/*lint -esym(459,_rs_forked)*/

#include "openbsd-compat.h"
#include "signal.h"

#include "arc4random.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "string.h"
#include "stdlib.h"
#include "errno.h"
#include "sys/types.h"

#define KEYSTREAM_ONLY
#include "chacha_private.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

#define KEYSZ   32
#define IVSZ    8
#define BLOCKSZ 64
#define RSBUFSZ (16*BLOCKSZ)

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void(*child)(void))
{
    _rs_forkhandler();
    return 0;
}

/* Marked MAP_INHERIT_ZERO, so zero'd out in fork children. */
static struct _rs {
    size_t      rs_have;    /* valid bytes at end of rs_buf */
    size_t      rs_count;   /* bytes till reseed */
} *rs;

/* Maybe be preserved in fork children, if _rs_allocate() decides. */
static struct _rsx {
    chacha_ctx  rs_chacha;  /* chacha context for random keystream */
    u_char      rs_buf[RSBUFSZ];    /* keystream blocks */
} *rsx;

pthread_mutex_t arc4_mutex = PTHREAD_MUTEX_INITIALIZER;

void _thread_arc4_lock(void)
{
    (void)pthread_mutex_lock(&arc4_mutex);
}/*lint !e454*/

void _thread_arc4_unlock(void)
{
    (void)pthread_mutex_unlock(&arc4_mutex);/*lint !e455*/
}

static inline int _rs_allocate(struct _rs **rsp, struct _rsx **rsxp)
{
    *rsp = (struct _rs *)malloc(sizeof(**rsp));
    if (*rsp == NULL)
    {
        return (-1);
    }
    *rsxp = (struct _rsx *)malloc(sizeof(**rsxp));
    if (*rsxp == NULL)
    {
        free(*rsp);
        *rsp = NULL;
        return (-1);
    }
    (void)_ARC4_ATFORK(_rs_forkhandler);
    return (0);
}

static inline void _rs_init(u_char *buf, size_t n)
{
    if (n < KEYSZ + IVSZ)
        return;

    if (rs == NULL) {
        if (_rs_allocate(&rs, &rsx) == -1)
            abort();
    }

    chacha_keysetup(&rsx->rs_chacha, buf, KEYSZ * 8, 0);
    chacha_ivsetup(&rsx->rs_chacha, buf + KEYSZ);
}

static inline void _rs_rekey(u_char *dat, size_t datlen)
{
#ifndef KEYSTREAM_ONLY
    memset(rsx->rs_buf, 0, sizeof(rsx->rs_buf));
#endif
    /* fill rs_buf with the keystream */
    chacha_encrypt_bytes(&rsx->rs_chacha, rsx->rs_buf,
        rsx->rs_buf, sizeof(rsx->rs_buf));
    /* mix in optional user provided data */
    if (dat) {
        size_t i, m;

        m = min(datlen, (KEYSZ + IVSZ));
        for (i = 0; i < m; i++)
            rsx->rs_buf[i] ^= dat[i];
    }
    /* immediately reinit for backtracking resistance */
    _rs_init(rsx->rs_buf, KEYSZ + IVSZ);
    memset(rsx->rs_buf, 0, KEYSZ + IVSZ);
    rs->rs_have = sizeof(rsx->rs_buf) - KEYSZ - IVSZ;
}

static void _rs_stir(void)
{
    u_char rnd[KEYSZ + IVSZ];

    if (getentropy(rnd, sizeof rnd) == -1)
        _getentropy_fail();

    if (!rs)
        _rs_init(rnd, sizeof(rnd));
    else
        _rs_rekey(rnd, sizeof(rnd));
    explicit_bzero(rnd, sizeof(rnd));    /* discard source seed */

    /* invalidate rs_buf */
    rs->rs_have = 0;
    memset(rsx->rs_buf, 0, sizeof(rsx->rs_buf));

    rs->rs_count = 1600000;
}

static inline void _rs_forkdetect(void)
{
    static pid_t _rs_pid = 0;
    //pid_t pid = getpid();
    pid_t pid = (pid_t)LOS_CurTaskIDGet();

    if (_rs_pid == 0 || _rs_pid != pid || _rs_forked)
    {
        _rs_pid = pid;
        _rs_forked = 0;
        if (rs)
            memset(rs, 0, sizeof(*rs));
    }
}

static inline void _rs_stir_if_needed(size_t len)
{
    _rs_forkdetect();
    if (!rs || rs->rs_count <= len)
        _rs_stir();
    if (rs->rs_count <= len)
        rs->rs_count = 0;
    else
        rs->rs_count -= len;
}

static inline void _rs_random_u32(uint32_t *val)
{
    u_char *keystream;

    _rs_stir_if_needed(sizeof(*val));
    if (rs->rs_have < sizeof(*val))
        _rs_rekey(NULL, 0);
    keystream = rsx->rs_buf + sizeof(rsx->rs_buf) - rs->rs_have;
    memcpy(val, keystream, sizeof(*val));
    memset(keystream, 0, sizeof(*val));
    rs->rs_have -= sizeof(*val);
}
uint32_t arc4random(void)
{
    uint32_t val;
    _ARC4_LOCK();
    _rs_random_u32(&val);
    _ARC4_UNLOCK();
    return val;
}
