#include "los_base.h"
#ifndef do_csum
static inline UINT16 from32to16(UINT32 n)
{
    /* add up 16-bit and 16-bit for 16+c bit */
    n = (n & 0xffff) + (n >> 16);
    /* add up carry.. */
    n = (n & 0xffff) + (n >> 16);
    return n;
}

UINT32 do_csum(const unsigned char *buf, int len)
{
    int odd;
    UINT32 ret = 0;

    if (len <= 0)
        goto out;
    odd = 1 & (unsigned long) buf;
    if (odd) {
#ifndef __LITTLE_ENDIAN
        ret += (*buf << 8);
#else
        ret = *buf;
#endif
        len--;
        buf++;
    }
    if (len >= 2) {
        if (2 & (unsigned long) buf) {
            ret += *(unsigned short *) buf;
            len -= 2;
            buf += 2;
        }
        if (len >= 4) {
            const unsigned char *end = buf + ((unsigned)len & ~3);
            unsigned int carry = 0;
            do {
                unsigned int w = *(unsigned int *) buf;
                buf += 4;
                ret += carry;
                ret += w;
                carry = (w > ret);
            } while (buf < end);
            ret += carry;
            ret = (ret & 0xffff) + (ret >> 16);
        }
        if (len & 2) {
            ret += *(unsigned short *) buf;
            buf += 2;
        }
    }
    if (len & 1)
#ifndef __LITTLE_ENDIAN
        ret += *buf;
#else
        ret += (*buf << 8);
#endif
    ret = from32to16(ret);
    if (odd)
        ret = ((ret >> 8) & 0xff) | ((ret & 0xff) << 8);
out:
    return ret;
}
#endif


UINT32 csum_partial(const void *buf, int len, UINT32 wsum)
{
    UINT32 sum = wsum;
    UINT32 ret = do_csum(buf, len);

    /* add in old sum, and carry.. */
    ret += sum;
    if (sum > ret)
        ret += 1;
    return ret;
}
