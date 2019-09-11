#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/barrier.h>

void osal_mb(void)
{
    // mb();
}
void osal_rmb(void)
{
    // rmb();
}
void osal_wmb(void)
{
    // wmb();
}
void osal_smp_mb(void)
{
    // smp_mb();
}
void osal_smp_rmb(void)
{
    // smp_rmb();
}
void osal_smp_wmb(void)
{
    // smp_wmb();
}
void osal_isb(void)
{
    isb();
}
void osal_dsb(void)
{
    dsb();
}
void osal_dmb(void)
{
    dmb();
}
