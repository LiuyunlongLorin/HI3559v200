#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/slab.h>
// #include "muttypes.h"

int osal_mutex_init(osal_mutex_t *mutex)
{
    pthread_mutex_t *p = NULL;
    if (mutex == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = kmalloc(sizeof(pthread_mutex_t), GFP_KERNEL);
    if (p == NULL) {
        osal_printk("%s - kmalloc error!\n", __FUNCTION__);
        return -1;
    }
    mutex_init(p);
    mutex->mutex = p;
    return 0;
}

int osal_mutex_lock(osal_mutex_t *mutex)
{
    pthread_mutex_t *p = NULL;
    if (mutex == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (pthread_mutex_t *)(mutex->mutex);
    mutex_lock(p);
    return 0;
}

int osal_mutex_lock_interruptible(osal_mutex_t *mutex)
{
    return osal_mutex_lock(mutex);
}

int osal_mutex_trylock(osal_mutex_t *mutex)
{
    pthread_mutex_t *p = NULL;
    if (mutex == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (pthread_mutex_t *)(mutex->mutex);

    return pthread_mutex_trylock(p);
}

void osal_mutex_unlock(osal_mutex_t *mutex)
{
    pthread_mutex_t *p = NULL;
    p = (pthread_mutex_t *)(mutex->mutex);

    (void)mutex_unlock(p);
}

void osal_mutex_destory(osal_mutex_t *mutex)
{
    pthread_mutex_t *p = NULL;
    p = (pthread_mutex_t *)(mutex->mutex);
    kfree(p);
    mutex->mutex = NULL;
}

