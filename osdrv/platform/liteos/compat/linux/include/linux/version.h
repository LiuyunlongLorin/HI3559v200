#ifndef __LINUX_VERSION_H__
#define __LINUX_VERSION_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define KERNEL_VERSION(a,b,c)   (((a)<<16)|((b)<<8)|(c))
#define LINUX_VERSION_CODE      KERNEL_VERSION(3,4,0)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_VERSION_H__ */
