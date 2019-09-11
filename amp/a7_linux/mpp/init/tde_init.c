#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/version.h>
#include "hi_type.h"
#include "hi_common.h"
#include "hi_osal.h"

static char* g_pszTdeMmzName = NULL;
module_param(g_pszTdeMmzName, charp, S_IRUGO);

HI_U32 g_u32TdeTmpBuf = 1658880;
module_param(g_u32TdeTmpBuf, uint, S_IRUGO);

bool g_bResizeFilter = true;
module_param(g_bResizeFilter, bool, S_IRUGO);

HI_U32 g_u32TdeBuf = 0x20000;
module_param(g_u32TdeBuf, uint, S_IRUGO);


extern int TDE_DRV_ModInit(void);
extern void TDE_DRV_ModExit(void);

#include <linux/of_platform.h>
extern unsigned int tde_irq;
extern HI_U32 *s_pu32BaseVirAddr;

static int hi35xx_tde_probe(struct platform_device *pdev)
{
    struct resource* mem;
    mem = osal_platform_get_resource_byname(pdev, IORESOURCE_MEM, "tde");
    s_pu32BaseVirAddr = devm_ioremap_resource(&pdev->dev, mem);

    if (IS_ERR(s_pu32BaseVirAddr))
    { return PTR_ERR(s_pu32BaseVirAddr); }

    tde_irq = osal_platform_get_irq_byname(pdev, "tde_osr_isr");
    if (tde_irq <= 0) {
            dev_err(&pdev->dev, "cannot find tde IRQ\n");
    }

    if (HI_SUCCESS != TDE_DRV_ModInit())
    {
        osal_printk("load tde.ko for %s...FAILED!\n",CHIP_NAME);
        return -1;
    }
    osal_printk("load tde.ko for %s...OK!\n",CHIP_NAME);
    return 0;
}

static int hi35xx_tde_remove(struct platform_device *pdev)
{
    TDE_DRV_ModExit();
    osal_printk("unload tde.ko for %s...OK!\n",CHIP_NAME);
    return 0;
}

static const struct of_device_id hi35xx_tde_match[] = {
        { .compatible = "hisilicon,hisi-tde" },
        {},
};
MODULE_DEVICE_TABLE(of, hi35xx_tde_match);

static struct platform_driver hi35xx_tde_driver = {
        .probe          = hi35xx_tde_probe,
        .remove         = hi35xx_tde_remove,
        .driver         = {
                .name   = "hi35xx_tde",
                .of_match_table = hi35xx_tde_match,
        },
};

osal_module_platform_driver(hi35xx_tde_driver);

MODULE_LICENSE("Proprietary");

