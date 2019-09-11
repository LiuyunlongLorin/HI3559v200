#ifndef __LINUX_SLAB_H__
#define __LINUX_SLAB_H__

#include <stdlib.h>

#include <asm/page.h> /* Don't ask. Linux headers are a mess. */


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define GFP_KERNEL    (0)
#define GFP_ATOMIC    (0)

#define __kmalloc(x) malloc(x)
#define kmalloc(x, y) __kmalloc(x)

#define __kfree(x)  free(x)
#define kfree(x) __kfree(x)

#define __vmalloc(x)  malloc(x)
#define vmalloc(x) __vmalloc(x)

#define __vfree(x)  free(x)
#define vfree(x) __vfree(x)

#define kzalloc(s, f)  __kzalloc(s,f)
#define __kzalloc(s,f) calloc(1, s)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_SLAB_H__ */

