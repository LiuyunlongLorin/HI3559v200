#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/slab.h>

int osal_sema_init(osal_semaphore_t *sem, int val)
{
    struct semaphore *p = NULL;
    if (sem == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    if (p == NULL) {
        osal_printk("%s - kmalloc error!\n", __FUNCTION__);
        return -1;
    }
    sema_init(p, val);
    sem->sem = p;
    return 0;
}
int osal_down_interruptible(osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;
    if (sem == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (struct semaphore *)(sem->sem);
    return down_interruptible(p);
}
int osal_down(osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;
    if (sem == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (struct semaphore *)(sem->sem);
    return down_interruptible(p);
}
int osal_down_trylock(osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;
    if (sem == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (struct semaphore *)(sem->sem);
    return down_trylock(p);
}
void osal_up(osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;
    p = (struct semaphore *)(sem->sem);
    up(p);
}
void osal_sema_destory(osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;
    p = (struct semaphore *)(sem->sem);

    sema_destory(p);

    kfree(p);
    sem->sem = NULL;
}

