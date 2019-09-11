#include "stddef.h"
#include "errno.h"
#include "fs/fs.h"

/*
 * Basic sanity checking; wish we could do better.
 */
static int gotdata(char *buf, size_t len)
{
    char    any_set = 0;
    size_t    i;

    for (i = 0; i < len; ++i)
        any_set |= buf[i];
    if (any_set == 0)
        return -1;
    return 0;
}

static int getentropy_urandom(void *buf, size_t len)
{
#if defined(LOSCFG_HW_RANDOM_ENABLE) || defined (LOSCFG_DRIVERS_RANDOM)

#ifdef LOSCFG_HW_RANDOM_ENABLE
    extern int hi_random_hw_init(void);
    extern int hi_random_hw_deinit(void);
    extern int hi_random_hw_getnumber(char *buffer, size_t buflen);
#endif
#ifdef LOSCFG_DRIVERS_RANDOM
    extern int ran_open(struct file *filep);
    extern ssize_t ran_read( struct file *filep, char *buffer, size_t buflen);
#endif
    size_t i;

#ifdef LOSCFG_HW_RANDOM_ENABLE
    (void)hi_random_hw_init();
    for (i = 0; i < len; ) {
        size_t wanted = len - i;
        ssize_t ret = hi_random_hw_getnumber((char *)buf + i, wanted);

        if (ret != 0) {
            (void)hi_random_hw_deinit();
            errno = EIO;
            return -1;
        }
        i += wanted;
    }

    (void)hi_random_hw_deinit();
#elif defined LOSCFG_DRIVERS_RANDOM
    (void)ran_open(NULL);
    for (i = 0; i < len; ) {
        size_t wanted = len - i;
        ssize_t ret = ran_read(NULL,(char *)buf + i, wanted);

        if (ret < 0) {
            errno = EIO;
            return -1;
        }
        i += ret;
    }
#endif
    if (gotdata(buf, len) == 0) {
        return 0;        /* satisfied */
    }
#endif
    errno = EIO;
    return -1;

}

int getentropy(void *buf, size_t len)
{
    int ret = -1;

    if (len > 256) {
        errno = EIO;
        return -1;
    }

    /*
     * Try to get entropy with /dev/urandom
     *
     * This can fail if the process is inside a chroot or if file
     * descriptors are exhausted.
     */
    ret = getentropy_urandom(buf, len);
    if (ret != -1)
        return (ret);

    errno = EIO;
    return (ret);
}
