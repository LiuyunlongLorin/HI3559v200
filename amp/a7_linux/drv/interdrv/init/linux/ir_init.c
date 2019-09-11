#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/of_platform.h>

#define OSDRV_MODULE_VERSION_STRING "HISI_ir @HiMPP"
extern unsigned int ir_irq;
extern volatile void *pIrRegBase;
extern int hiir_init(void);
extern void hiir_exit(void);

static int hi_ir_probe(struct platform_device *pdev)
{   
    struct resource *mem;
    
    ir_irq = platform_get_irq(pdev, 0);
    if (ir_irq <= 0) {
            dev_err(&pdev->dev, "cannot find ir IRQ%d. \n", ir_irq);
    }

    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    pIrRegBase = (volatile void *)devm_ioremap_resource(&pdev->dev, mem);
    if (IS_ERR((void* )pIrRegBase))
    {
        //printk("mem->start %#x. \n", mem->start);
		dev_err(&pdev->dev, "ir reg map failed. \n");
	}

    return hiir_init();
}

static int hi_ir_remove(struct platform_device *pdev)
{
    hiir_exit();
    return 0;
}

static const struct of_device_id hi_ir_match[] = {
    { .compatible = "hisilicon,hi_ir" },
    {},
};

static struct platform_driver hi_ir_driver = {
    .probe  = hi_ir_probe,
    .remove = hi_ir_remove,
    .driver =  { .name = "hi_ir",
                .of_match_table = hi_ir_match,
               },
};

module_platform_driver(hi_ir_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon");
MODULE_DESCRIPTION("Hisilicon Infrared remoter(HIIR11) Device Driver");
MODULE_VERSION("HI_VERSION=" OSDRV_MODULE_VERSION_STRING);
