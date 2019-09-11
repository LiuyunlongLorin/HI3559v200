#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/version.h>

#include "hi_common.h"
#include "hi_osal.h"

extern HI_U8 g_u8IvePowerSaveEn;
extern HI_U16 g_u16IveNodeNum;

extern int IVE_ModInit(void);
extern void IVE_ModExit(void);

module_param_named(save_power, g_u8IvePowerSaveEn, byte, S_IRUGO);
module_param_named(max_node_num, g_u16IveNodeNum, ushort, S_IRUGO);


#include <linux/of_platform.h>
#define IVE_DEV_NAME_LENGTH 10
extern void *g_pstIveRegs;
extern unsigned int ive_irq;

static int hi35xx_ive_probe(struct platform_device *pdev)
{
    struct resource *pstMem = NULL;
	HI_CHAR szDevName[IVE_DEV_NAME_LENGTH] = {"ive"};

	pstMem = osal_platform_get_resource_byname(pdev, IORESOURCE_MEM, szDevName);
	g_pstIveRegs = devm_ioremap_resource(&pdev->dev, pstMem);
	if (IS_ERR((void*)g_pstIveRegs))
	{
		return PTR_ERR((const void*)g_pstIveRegs);
	}

    ive_irq = osal_platform_get_irq_byname(pdev, szDevName);
    if (ive_irq <= 0)
    {
        dev_err(&pdev->dev, "cannot find ive IRQ\n");
        printk("cannot find ive IRQ\n");
    }

    return IVE_ModInit();
}

static int hi35xx_ive_remove(struct platform_device *pdev)
{
    IVE_ModExit();
	g_pstIveRegs = NULL;

    return 0;
}


static const struct of_device_id hi35xx_ive_match[] = {
        { .compatible = "hisilicon,hisi-ive" },
        {},
};
MODULE_DEVICE_TABLE(of, hi35xx_ive_match);

static struct platform_driver hi35xx_ive_driver = {
        .probe          = hi35xx_ive_probe,
        .remove         = hi35xx_ive_remove,
        .driver         = {
                .name   = "hi35xx_ive",
                .of_match_table = hi35xx_ive_match,
        },
};

osal_module_platform_driver(hi35xx_ive_driver);

MODULE_LICENSE("Proprietary");
