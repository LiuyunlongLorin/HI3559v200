#include "errno.h"
#include "string.h"

/*
 * XXX Fixme
 *
 * This is very ugly but temporary. THIS NEEDS SERIOUS ENHANCEMENTS.
 * But it's very tricky to get right even in C.
 */
extern unsigned int do_csum(const unsigned char *, int);

unsigned int
csum_partial_copy_from_user(const void *src, void *dst,
                        int len, unsigned int psum, int *errp)
{
    unsigned long result;

    /* XXX Fixme
     * for now we separate the copy from checksum for obvious
     * alignment difficulties. Look at the Alpha code and you'll be
     * scared.
     */

    memcpy(dst, src, len);

    result = do_csum(dst, len);

    /* add in old sum, and carry.. */
    result += (unsigned int)psum;
    /* 32+c bits -> 32 bits */
    result = (result & 0xffffffff) + (result >> 32); /*lint !e572*/
    return (unsigned int )result;
}


unsigned int
csum_partial_copy_nocheck(const void *src, void *dst, int len, unsigned int sum)
{
    return csum_partial_copy_from_user(( const void *)src,
                       dst, len, sum, NULL);
}

