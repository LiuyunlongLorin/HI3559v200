#ifndef __LINUX_SPINLOCK_H__
#define __LINUX_SPINLOCK_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


typedef int spinlock_t;
#define SPIN_LOCK_UNLOCKED (spinlock_t) 0
#define DEFINE_SPINLOCK(x) spinlock_t x = SPIN_LOCK_UNLOCKED

#define SPIN_MACRO_START do {
#define SPIN_MACRO_END   } while (0)

#define SPIN_EMPTY_STATEMENT SPIN_MACRO_START SPIN_MACRO_END

#define SPIN_UNUSED_PARAM( _type_, _name_ ) SPIN_MACRO_START      \
  _type_ __tmp1 = (_name_);                                     \
  _type_ __tmp2 = __tmp1;                                       \
  __tmp1 = __tmp2;                                              \
SPIN_MACRO_END

#define spin_lock_init(lock)             \
SPIN_MACRO_START;                         \
SPIN_UNUSED_PARAM(spinlock_t *, lock);    \
SPIN_MACRO_END

#define spin_lock(lock) do { LOS_TaskLock();}while(0)

#define spin_unlock(lock) do { LOS_TaskUnlock();}while(0)

#define spin_lock_bh(lock)               \
SPIN_MACRO_START;                         \
SPIN_UNUSED_PARAM(spinlock_t *, lock);    \
SPIN_MACRO_END

#define spin_unlock_bh(lock)             \
SPIN_MACRO_START;                         \
SPIN_UNUSED_PARAM(spinlock_t *, lock);    \
SPIN_MACRO_END

#define spin_lock_irqsave(lock, flags)  do { flags = LOS_IntLock();spin_lock(lock);} while(0)
#define spin_unlock_irqrestore(lock, flags)  do { spin_unlock(lock);LOS_IntRestore(flags);} while(0)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_SPINLOCK_H__ */
