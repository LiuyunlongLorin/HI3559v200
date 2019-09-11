#ifndef __LCD_MW_ADAPT_H__
#define __LCD_MW_ADAPT_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** Mutex Lock */
#define HI_MUTEX_INIT_LOCK(mutex)  do { (HI_VOID) pthread_mutex_init(&mutex, NULL); } while(0)
#define HI_MUTEX_LOCK(mutex)       do { (HI_VOID) pthread_mutex_lock(&mutex); } while(0)
#define HI_MUTEX_UNLOCK(mutex)     do { (HI_VOID) pthread_mutex_unlock(&mutex); } while(0)
#define HI_MUTEX_DESTROY(mutex)    do { (HI_VOID) pthread_mutex_destroy(&mutex); } while(0)

/** Memory Safe Free */
#define HI_APPCOMM_SAFE_FREE(p)    do { if (NULL != (p)){ free(p); (p) = NULL; } }while(0)

HI_S32 MW_SCREEN_ST7789_Init(HI_VOID);

HI_S32 MW_TOUCHPAD_FT6236_Init(HI_VOID);
HI_S32 MW_SCREEN_VDP_CFG(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* End of #ifndef __LCD_MW_ADAPT_H__ */
