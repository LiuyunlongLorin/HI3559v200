#ifndef __LINUX_RWSEM_H__
#define __LINUX_RWSEM_H__


#include <asm/semaphore.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

//struct rw_semaphore;

#define down_read(sem) _sema_lock((losMutexDef_t*)sem)
#define down_read_trylock(sem) _sema_trylock((losMutexDef_t*)sem)
#define down_write(sem) _sema_lock((losMutexDef_t*)sem)
#define down_write_trylock(sem) _sema_trylock((losMutexDef_t*)sem)
#define up_read(sem) _sema_unlock((losMutexDef_t*)sem)
#define up_write(sem) _sema_unlock((losMutexDef_t*)sem)
#define downgrade_write(sem)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif // __LINUX_RWSEM_H__
