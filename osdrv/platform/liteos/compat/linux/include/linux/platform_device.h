/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

/**@defgroup drive base
 * @ingroup drivers
 */
#ifndef _PLATFORM_DEVICE_H
#define _PLATFORM_DEVICE_H

#include <linux/device.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#define IORESOURCE_IO       0x00000100  /* PCI/ISA I/O ports */
#define IORESOURCE_MEM      0x00000200
#define IORESOURCE_REG      0x00000300  /* Register offsets */
#define IORESOURCE_IRQ      0x00000400
#define IORESOURCE_DMA      0x00000800
#define IORESOURCE_BUS      0x00001000

typedef long resource_size_t;

struct resource {
    resource_size_t start;
    resource_size_t end;
    const char *name;
    unsigned long flags;
    unsigned long desc;
    struct resource *parent, *sibling, *child;
};

/**
 * @ingroup los_drivers
 * Define the structure of the parameters used for platform device creation.
 *.
 */
struct platform_device{
    const char *name;
    struct device dev;
    int id;
    UINT32 num_resources;
    struct resource *resource;
};

struct pm_message_t{
    int event;
};
/**
 * @ingroup los_drivers
 * Define the structure of the parameters used for platform driver creation.
 *.
 */
struct platform_driver{
    int (*probe)(struct platform_device *);
    int (*remove)(struct platform_device *);
    void (*shutdown)(struct platform_device *);
    int (*suspend)(struct platform_device *);
    int (*resume)(struct platform_device *);
    struct device_driver driver;
};
/**
 *@ingroup los_drivers
 *@brief register a platform_driver to platform bus.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to register a platform_driver to platform bus.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The platform bus system is a soft bus that is used to deal the host device and driver.</li>
 *<li>The same bus node can not be registered twice.</li>
 *</ul>
 *
 *@param  drv      [IN]A point to platform_driver.
 *
 *@retval #LOS_ERRNO_DRIVER_INPUT_INVALID        Invalid input.drv and drv.driver->name can not be NULL.
 *@retval #LOS_ERRNO_DRIVER_DRIVER_REGISTERED    Drvier register twice.
 *@retval #LOS_ERRNO_DRIVER_BUS_MUX_FAIL         Mux create failed.
 *@retval #LOS_ERRNO_DRIVER_BUS_INVALID          Bus is not in system.
 *@retval #LOS_ERRNO_DRIVER_BUS_MATCH_FAIL       Do match failed.
 *@retval #LOS_ERRNO_DRIVER_BUS_PROBE_FAIL       Do probe failed.
 *@retval #LOS_OK        The platform_driver register success.
 *@par Dependency:
 *<ul><li>platform_device.h: the header file that contains the API declaration.</li></ul>
 *@since Huawei LiteOS V200R001C00
 */
extern UINT32 platform_driver_register(struct platform_driver *drv);
/**
 *@ingroup los_drivers
 *@brief unregister a  platform_driver from the platform bus.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to unregister a platform_driver from the platform bus.</li>
 *</ul>
 *
 *@param  drv      [IN]A point to platform_driver. drv/drv->name/drv->bus can not be NULL.
 *
 *@par Dependency:
 *<ul><li>platform_device.h: the header file that contains the API declaration.</li></ul>
 *@since Huawei LiteOS V200R001C00
 */
extern VOID platform_driver_unregister(struct platform_driver *drv);
/**
 *@ingroup los_drivers
 *@brief register a platform_device to platform bus.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to register a platform_device to platform bus.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The same platform_device node can not be registered twice.</li>
 *</ul>
 *
 *@param  pdev      [IN] A point to platform_device. num_resources can not bigger than the count of resource_array.
 *
  *@retval #LOS_ERRNO_DRIVER_INPUT_INVALID       Invalid input.pdev and pdev->name can not be NULL.
 *@retval #LOS_ERRNO_DRIVER_DEVICE_REGISTERED    Deviec register twice.
 *@retval #LOS_ERRNO_DRIVER_DEVICE_INITIALFAIL   Mux create failed.
 *@retval #LOS_ERRNO_DRIVER_DEVICE_BOUNDED       Do attach failed.device has bounded.
 *@retval #LOS_ERRNO_DRIVER_BUS_MATCH_FAIL       Do match failed.
 *@retval #LOS_ERRNO_DRIVER_BUS_PROBE_FAIL       Do probe failed.
 *@retval #LOS_OK        The platform_device register success.
 *@par Dependency:
 *<ul><li>platform_device.h: the header file that contains the API declaration.</li></ul>
 *@since Huawei LiteOS V200R001C00
 */
extern UINT32 platform_device_register(struct platform_device *pdev);
/**
 *@ingroup los_drivers
 *@brief unregister a  platform_device from the platform bus.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to unregister a platform_device from the platform bus.</li>
 *</ul>
 *
 *@param  pdev      [IN]A point to platform_device.If pdev is NULL,this function will do nothing.
 *
 *@par Dependency:
 *<ul><li>platform_device.h: the header file that contains the API declaration.</li></ul>
 *@since Huawei LiteOS V200R001C00
 */
extern VOID platform_device_unregister(struct platform_device *pdev);

extern UINT32 platform_device_add(struct platform_device *pdev);

extern struct resource *platform_get_resource(struct platform_device *,
                                  unsigned int, unsigned int);
extern long platform_get_irq(struct platform_device *, unsigned int);

extern VOID *platform_ioremap_resource(struct resource *res);

#define to_platform_driver(drv) (container_of((drv), struct platform_driver, \
                             driver))
#define to_platform_device(x) container_of((x), struct platform_device, dev)

STATIC INLINE VOID *platform_get_drvdata(const struct platform_device *pdev)
{
    if(NULL == pdev)
    {
        PRINT_WARN("platform_get_drvdata :the input dev is NULL!\n");
        return NULL;
    }
    return dev_get_drvdata(&pdev->dev);
}

STATIC INLINE VOID platform_set_drvdata(struct platform_device *pdev, VOID *data)
{
    if(NULL == pdev)
    {
        PRINT_WARN("platform_set_drvdata :the input dev is NULL!\n");
        return;
    }
    dev_set_drvdata(&pdev->dev, data);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif

