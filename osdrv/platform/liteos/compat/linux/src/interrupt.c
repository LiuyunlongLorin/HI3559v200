#include <linux/interrupt.h>
#include <los_hwi.ph>

int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
        const char *name, void *dev)
{
    UINT32 uwRet = 0;
    HWI_IRQ_PARAM_S stuwIrqPara;

    if(OS_INT_ACTIVE)
        return OS_ERRNO_HWI_INTERR;

    stuwIrqPara.swIrq = irq;
    stuwIrqPara.pDevId = dev;
    stuwIrqPara.pName = name;

    uwRet = LOS_HwiCreate(irq, 0, flags, (HWI_PROC_FUNC)handler, &stuwIrqPara);

    if (uwRet == LOS_OK)
        hal_interrupt_unmask(irq);

    return uwRet;
}

void free_irq(unsigned int irq, void *dev_id)
{
    HWI_IRQ_PARAM_S stuwIrqPara;

    if(OS_INT_ACTIVE)
        return ;

    stuwIrqPara.swIrq = irq;
    stuwIrqPara.pDevId = dev_id;

    (void)LOS_HwiDelete(irq, &stuwIrqPara);
    return;
}

void enable_irq(unsigned int irq)
{
    hal_interrupt_unmask(irq);
}

void disable_irq(unsigned int irq)
{
    hal_interrupt_mask(irq);
}

bool irq_bottom_half(struct workqueue_struct * work_queue, irq_bottom_half_handler_t handler, void *data, unsigned int pri)
{
    struct work_struct *mywork;

    if(NULL == work_queue || NULL == handler)
        return FALSE;

    if(pri >= OS_WORK_PRIORITY_DEFAULT || pri >= OS_WORK_PRIORITY_LOWEST)
        return FALSE;

    mywork = (struct work_struct *)LOS_MemAlloc(m_aucSysMem0, sizeof (struct work_struct));
    if(NULL == mywork)
        return FALSE;

    INIT_WORK(mywork, handler);

    mywork->data = (atomic_long_t)data;
    mywork->work_pri = pri;
    if(!(queue_work(work_queue, mywork)))
    {
        (VOID)LOS_MemFree(m_aucSysMem0, mywork);
        return FALSE;
    }
    return TRUE; /*lint !e429*/
}

