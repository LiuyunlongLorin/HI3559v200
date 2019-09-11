#ifndef __ASM_PAGE_H__
#define __ASM_PAGE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* These aren't used by much yet. If that changes, you might want
   to make them actually correct :) */
#ifndef PAGE_SIZE
#define PAGE_SIZE (0x1000)
#endif
#define PAGE_MASK    (~(PAGE_SIZE-1))



#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __ASM_PAGE_H__ */
