#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

int osal_spin_lock_init(osal_spinlock_t *lock)
{
    spinlock_t *p = NULL;
    if (lock == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (spinlock_t *)kmalloc(sizeof(spinlock_t), GFP_KERNEL);
    if (p == NULL) {
        osal_printk("%s - kmalloc error!\n", __FUNCTION__);
        return -1;
    }
    spin_lock_init(p);
    lock->lock = p;
    return 0;
}
void osal_spin_lock(osal_spinlock_t *lock)
{
    spin_lock ((spinlock_t *)(lock->lock));
}
int osal_spin_trylock(osal_spinlock_t *lock)
{
    /* not support yet in liteos */
    dprintf("Do not support in liteos\n");

    return -1;
}
void osal_spin_unlock(osal_spinlock_t *lock)
{
    spin_unlock ((spinlock_t *)(lock->lock));
}
void osal_spin_lock_irqsave(osal_spinlock_t *lock, unsigned long *flags)
{
    unsigned long f;

    spin_lock_irqsave ((spinlock_t *)(lock->lock), f);
    *flags = f;
}
void osal_spin_unlock_irqrestore(osal_spinlock_t *lock, unsigned long *flags)
{
    unsigned long f;

    f = *flags;
    spin_unlock_irqrestore ((spinlock_t *)(lock->lock), f);
}
void osal_spin_lock_destory(osal_spinlock_t *lock)
{
    spinlock_t *p = NULL;
    p = (spinlock_t *)(lock->lock);
    kfree(p);
    lock->lock = NULL;
}
