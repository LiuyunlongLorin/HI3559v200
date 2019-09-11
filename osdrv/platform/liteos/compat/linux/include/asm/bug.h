#ifndef __ASM_BUG_H__
#define __ASM_BUG_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define BUG() do { \
        diag_printf("BUG() at %s %d\n", __FILE__, __LINE__); \
        } while (0)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __ASM_BUG_H__ */
