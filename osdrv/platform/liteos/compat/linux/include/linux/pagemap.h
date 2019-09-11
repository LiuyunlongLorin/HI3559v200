#ifndef __LINUX_PAGEMAP_H__
#define __LINUX_PAGEMAP_H__

#include <asm/bug.h>
#include <asm/page.h>



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define COMPAT_PAGE_SHIFT  12
#define COMPAT_PAGE_CACHE_SHIFT        COMPAT_PAGE_SHIFT
#define COMPAT_PAGE_CACHE_SIZE           PAGE_SIZE

#define PageLocked(page) 1
#define Page_Uptodate(page) 0
#define UnlockPage(page)
#define PAGE_BUG(page) BUG()
#define ClearPageUptodate(page)
#define SetPageError(page)
#define ClearPageError(page)
#define SetPageUptodate(page)

#define PAGE_SHIFT  COMPAT_PAGE_SHIFT
#define PAGE_CACHE_SHIFT        COMPAT_PAGE_CACHE_SHIFT
#define PAGE_CACHE_SIZE         COMPAT_PAGE_CACHE_SIZE

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_PAGEMAP_H__ */
