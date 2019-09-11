#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

int osal_request_irq(unsigned int irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn, const char *name,
                     void *dev)
{
    unsigned long flags = IRQF_SHARED;

    return request_irq(irq, (irq_handler_t)handler, flags, name, dev);
}
void osal_free_irq(unsigned int irq, void *dev)
{
    free_irq(irq, dev);
}

int osal_in_interrupt(void)
{
    return in_interrupt();
}

int osal_register_gic_handle(unsigned int index, unsigned int irq, osal_gic_handle_t handle, const char *name,
                             void *dev)
{
    unsigned long flags = 0;

    return request_irq(irq, (irq_handler_t)handle, flags, name, dev);
}

int osal_unregister_gic_handle(unsigned int index, unsigned int irq, void *dev)
{
    free_irq(irq, dev);
    return 0;
}

