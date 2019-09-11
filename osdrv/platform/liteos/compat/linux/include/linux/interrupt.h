/** @defgroup interrupt Interrupt mechanism
 *  @ingroup linux
*/
#ifndef __LINUX_INTERRUPT_H__
#define __LINUX_INTERRUPT_H__

#include "linux/kernel.h"
#include "los_base.h"
#include "linux/workqueue.h"
#include "asm/hal_platform_ints.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define IRQ_RETVAL(x)    ((x) != IRQ_NONE)

/**
 * These correspond to the IORESOURCE_IRQ_* defines in
 * linux/ioport.h to select the interrupt line behaviour.
 */
#define IRQF_TRIGGER_LOW          0x00000008
#define IRQF_TRIGGER_HIGH         0x00000004
#define IRQF_TRIGGER_FALLING    0x00000002
#define IRQF_TRIGGER_RISING      0x00000001
#define IRQF_TRIGGER_NONE        0x00000000
#define IRQF_TRIGGER_MASK    (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW | \
                         IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)
#define IRQF_PROBE_SHARED    0x00000100

typedef enum irqreturn {
    IRQ_NONE        = (0 << 0),                      /* interrupt was not from this device*/
    IRQ_HANDLED        = (1 << 0),               /*interrupt was handled by this device*/
    IRQ_WAKE_THREAD        = (1 << 1),   /*handler requests to wake the handler thread*/
}irqreturn_t;

typedef irqreturn_t (*irq_handler_t)(int, void *);
typedef  VOID (*irq_bottom_half_handler_t)(struct work_struct *);

typedef struct irq_args
{
    int             iIrq;
    void           *pDevId;
    const char     *pName;
}irq_args;

/*
 *@ingroup interrupt
 *@brief Request an interrupt.
 *
 *@par Description:
 *This API is used to request the interrupt that has a specified interrupt ID and register an interrupt handler.
 *@attention
 *<ul>
 *<li>The maximum number of interrupts supported by the Huawei LiteOS Kernel is pre-configured (for details on how to pre-configure an appropriate upper threshold, see the reference document provided by the chip supplier). If the maximum number of interrupts is exceeded, the request fails.</li>
 *<li>The value of registered interrupt handler must not be null. Devices that share a same interrupt ID must each be assigned a unique value of the dev parameter.</li>
 *<li>The input parameter dev must be valid, otherwise, the system may be abnormal.</li>
 *</ul>
 *
 *@param irq   [IN] ID of the interrupt to be requested.[OS_USER_HWI_MIN,OS_USER_HWI_MAX].
 *@param handler   [IN] Interrupt handler to be registered.
 *@param flags   [IN] Attributes of the interrupt processing, NOT used.
 *@param name   [IN] Interrupt name.
 *@param dev   [IN] Input parameter of the interrupt handler.
 *
 * @retval #OS_ERRNO_HWI_INTERR                     The API is called during an interrupt, which is forbidden.
 * @retval #OS_ERRNO_HWI_PROC_FUNC_NULL             The hardware interrupt processing function is null.
 * @retval #OS_ERRNO_HWI_NUM_INVALID                The interrupt ID is invalid.
 * @retval #OS_ERRNO_HWI_NO_MEMORY                  The memory is insufficient for creating a hardware interrupt.
 * @retval #OS_ERRNO_HWI_ALREADY_CREATED            The interrupt being created has already been created.
 * @retval #LOS_OK                                  The interrupt request is accepted.
 *@par Dependency:
 *<ul><li>interrupt.h: the header file that contains the API declaration.</li></ul>
 *@see free_irq
 *@since Huawei LiteOS V100R001C00
 */
int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
        const char *name, void *dev);


/*
 *@ingroup interrupt
 *@brief Delete an interrupt.
 *
 *@par Description:
 *This API is used to delete the interrupt handler that has a specified input parameter.
 *@attention
 *<ul>
 *<li>The maximum number of interrupts supported by the Huawei LiteOS Kernel is pre-configured (for details on how to pre-configure an appropriate upper threshold, see the reference document provided by the chip supplier). If the maximum number of interrupts is exceeded, the interrupt fails to be deleted.</li>
* <li>Please make sure that the parameters irq and dev_id is that specified by calling request_irq(), or else delete an interrupt would be fails.</li>
 *</ul>
 *
 *@param irq   [IN] ID of the interrupt to be deleted.[OS_USER_HWI_MIN,OS_USER_HWI_MAX].
 *@param dev_id   [IN] Input parameter of the interrupt handler to be deleted.
 *
 *@retval None.
 *@par Dependency:
 *<ul><li>interrupt.h: the header file that contains the API declaration.</li></ul>
 *@see request_irq
 *@since Huawei LiteOS V100R001C00
 */
void free_irq(unsigned int irq, void *dev_id);

/*
 *@ingroup interrupt
 *@brief Enable an interrupt.
 *
 *@par Description:
 *This API is used to enable the interrupt that has a specified interrupt ID.
 *@attention
 *<ul>
 *<li>The maximum number of interrupts supported by the Huawei LiteOS Kernel is pre-configured (for details on how to pre-configure an appropriate upper threshold, see the reference document provided by the chip supplier). If the maximum number of interrupts is exceeded, the interrupt fails to be enabled.</li>
 *<li>Please do make sure the specified irq has corresponding interrupt handler, otherwise, the system would do nothing  but only respond the specified interrupt if the specified interrupt is pending.</li>
 *</ul>
 *
 *@param irq   [IN] ID of the interrupt to be enabled.[OS_USER_HWI_MIN,OS_USER_HWI_MAX].
 *
 *@retval None.
 *@par Dependency:
 *<ul><li>interrupt.h: the header file that contains the API declaration.</li></ul>
 *@see disable_irq
 *@since Huawei LiteOS V100R001C00
 */
void enable_irq(unsigned int irq);

/*
 *@ingroup interrupt
 *@brief Disable an interrupt.
 *
 *@par Description:
 *This API is used to disable the interrupt that has a specified interrupt ID.
 *@attention
 *<ul>
 *<li>The maximum number of interrupts supported by the Huawei LiteOS Kernel is pre-configured (for details on how to pre-configure an appropriate upper threshold, see the reference document provided by the chip supplier). If the maximum number of interrupts is exceeded, the interrupt fails to be disabled.</li>
 *</ul>
 *
 *@param irq   [IN] ID of the interrupt to be disabled.[OS_USER_HWI_MIN,OS_USER_HWI_MAX].
 *
 *@retval None.
 *@par Dependency:
 *<ul><li>interrupt.h: the header file that contains the API declaration.</li></ul>
 *@see enable_irq
 *@since Huawei LiteOS V100R001C00
 */
void disable_irq(unsigned int irq);

/*
 *@ingroup interrupt
 *@brief Bottom half interrupt handler.
 *
 *@par Description:
 *This API is used to realize the function of the bottom half of an interrupt handler. The bottom half is executed in handler.
 *@attention
 *<ul>
 *<li>The passed work_queue must be valid rather than a wild pointer.</li>
 *<li>The input parameter handler and data must be valid, otherwise, the system may be abnormal.</li>
 *<li>The memory that is pointed to by work_struct and dynamically requested by this API needs to be released in the bottom half interrupt handler specified by handler. If it is not released, memory leak will result.</li>
 *<li>The input parameter pri should be in [0,OS_WORK_PRIORITY_DEFAULT)</li>
 *</ul>
 *
 *@param work_queue   [IN] Work queue created when Huawei LiteOS Kernel is initialized or created by users.
 *@param handler   [IN] Bottom half interrupt handler.
 *@param data   [IN] input param of work func.
 *@param pri   [IN] input param of work pri.
 *@retval  #FALSE                                                              The bottom half interrupt handler fails to be created.
 *@retval  #TRUE                                                               The bottom half interrupt handler is successfully created.
 *@par Dependency:
 *<ul><li>interrupt.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
bool irq_bottom_half(struct workqueue_struct * work_queue, irq_bottom_half_handler_t handler, void *data, unsigned int pri);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_INTERRUPT_H__ */

