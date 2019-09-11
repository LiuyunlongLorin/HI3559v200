#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/atomic.h>
#include <linux/slab.h>

int osal_atomic_init(osal_atomic_t *atomic)
{
    atomic_t *p = NULL;
    if (atomic == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (atomic_t *)kmalloc(sizeof(atomic_t), GFP_KERNEL);
    if (p == NULL) {
        osal_printk("%s - kmalloc error!\n", __FUNCTION__);
        return -1;
    }
    memset(p, 0, sizeof(atomic_t));
    atomic->atomic = p;
    return 0;
}
void osal_atomic_destory(osal_atomic_t *atomic)
{
    kfree(atomic->atomic);
    atomic->atomic = NULL;
}
int osal_atomic_read(osal_atomic_t *atomic)
{
    atomic_t *p = NULL;
    if (atomic == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    return atomic_read(p);
}
void osal_atomic_set(osal_atomic_t *atomic, int i)
{
    atomic_t *p = NULL;
    p = (atomic_t *)(atomic->atomic);
    atomic_set((volatile unsigned int *)p, i);
}
int osal_atomic_inc_return(osal_atomic_t *atomic)
{
    atomic_t *p = NULL;
    if (atomic == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    return atomic_inc_return(p);
}
int osal_atomic_dec_return(osal_atomic_t *atomic)
{
    atomic_t *p = NULL;
    if (atomic == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    return atomic_dec_return(p);
}
