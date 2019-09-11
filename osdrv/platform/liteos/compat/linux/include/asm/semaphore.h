#ifndef __ASM_SEMAPHORE_H__
#define __ASM_SEMAPHORE_H__

#include "los_sem.ph"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#define ERESTARTSYS 512 //linux, down_interruptiable return value

struct semaphore {
  SEM_CB_S *sem;
  int count;
};

typedef struct semaphore losMutexDef_t;
#define DECLARE_MUTEX(x) struct semaphore x = {(SEM_CB_S *)0xffffffff,1}
#define DECLARE_MUTEX_LOCKED(x) struct semaphore x = {(SEM_CB_S *)0xffffffff,0}
#define DEFINE_SEMAPHORE(x) DECLARE_MUTEX(x)

extern int _sema_lock(losMutexDef_t *sem);
extern int _sema_unlock  (losMutexDef_t *sem);
extern int _sema_init(losMutexDef_t *sem,unsigned int value);
extern int _sema_destory(losMutexDef_t *sem);
extern int _sema_trylock(losMutexDef_t *sem);

#define init_MUTEX(sem) _sema_init((losMutexDef_t*)sem,1)
#define destory_MUTEX(sem) _sema_destory((losMutexDef_t*)sem)
#define init_MUTEX_LOCKED(sem) do {  (VOID)_sema_init((losMutexDef_t*)sem,0); } while(0)
#define down(sem) _sema_lock((losMutexDef_t*)sem)
#define down_interruptible(sem)  _sema_lock((losMutexDef_t*)sem)
#define down_trylock(sem)  _sema_trylock((losMutexDef_t*)sem)
#define up(sem) _sema_unlock((losMutexDef_t*)sem)

#define sema_init(sem, n) \
    do { \
        if (n == 1) \
        { init_MUTEX(sem); } \
        else \
        { init_MUTEX_LOCKED(sem); } \
    } while(0)

#define sema_destory(sem) \
    do{ \
        destory_MUTEX(sem); \
    }while(0)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __ASM_SEMAPHORE_H__ */
