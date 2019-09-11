#ifndef __LINUX_FILTER_H__
#define __LINUX_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

struct sock_filter
{
    unsigned short    code; /* compiler code */
    unsigned char      jt; /* jump if true */
    unsigned char      jf; /* jump if false */
    unsigned int    k;  /* misc field */
};

struct sock_fprog
{
    unsigned short       len;
    struct sock_filter   *filter;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LINUX_FILTER_H__ */