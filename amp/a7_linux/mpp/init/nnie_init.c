#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/version.h>

#include "hi_common.h"
#include "hi_osal.h"

extern HI_U8 g_u8NniePowerSaveEn;
extern HI_U16 g_u16NnieTskBufNum;

extern int SVP_NNIE_ModInit(void);
extern void SVP_NNIE_ModExit(void);
module_param_named(nnie_save_power, g_u8NniePowerSaveEn, byte, S_IRUGO);
module_param_named(nnie_max_tskbuf_num, g_u16NnieTskBufNum, ushort, S_IRUGO);

#include <linux/of_platform.h>
#define SVP_NNIE_DEV_NAME_LENGTH 8
#define SVP_NNIE_DEV_NUM 1
extern void* g_apvNnieReg[SVP_NNIE_DEV_NUM];
extern HI_U32 g_au32NnieIrg[SVP_NNIE_DEV_NUM];
extern HI_U32 g_au32GdcIrq[GDC_IP_NUM];

static int hi35xx_svp_nnie_probe(struct platform_device *pdev)
{
    HI_U32 i;
    HI_S32 s32Irq = 0;
    HI_CHAR acNnieDevName[SVP_NNIE_DEV_NAME_LENGTH] = {'\0'};
    HI_CHAR acGdcDevName[SVP_NNIE_DEV_NAME_LENGTH] = "gdc";
    struct resource *pstMem;
    for(i = 0; i < SVP_NNIE_DEV_NUM; i++)
    {
        snprintf(acNnieDevName,SVP_NNIE_DEV_NAME_LENGTH,"nnie%d",i);
        pstMem = osal_platform_get_resource_byname(pdev, IORESOURCE_MEM, acNnieDevName);
        g_apvNnieReg[i] = devm_ioremap_resource(&pdev->dev,pstMem);
        if(IS_ERR(g_apvNnieReg[i]))
        {
            return PTR_ERR(g_apvNnieReg[i]);
        }
        s32Irq = osal_platform_get_irq_byname(pdev, acNnieDevName);

        if(s32Irq <= 0)
        {
            dev_err(&pdev->dev,"can't find nnie%d IRQ\n",i);
            return -1;
        }
        g_au32NnieIrg[i] = (HI_U32)s32Irq;
    }

    for (i = 0; i < GDC_IP_NUM; i++)
    {
        g_au32GdcIrq[i] = osal_platform_get_irq_byname(pdev, acGdcDevName);
        if (g_au32GdcIrq[i] <= 0)
        {
            dev_err(&pdev->dev, "cannot find gdc%d IRQ\n",i);
        }
    }

    return SVP_NNIE_ModInit();
}

static int hi35xx_svp_nnie_remove(struct platform_device *pdev)
{

    HI_U32 i;
    SVP_NNIE_ModExit();
    for (i = 0; i < SVP_NNIE_DEV_NUM; i++)
    {
        g_apvNnieReg[i] = HI_NULL;
    }
    return 0;
}


static const struct of_device_id hi35xx_svp_nnie_match[] = {
        { .compatible = "hisilicon,hisi-nnie" },
        {},
};
MODULE_DEVICE_TABLE(of, hi35xx_svp_nnie_match);

static struct platform_driver hi35xx_svp_nnie_driver = {
        .probe          = hi35xx_svp_nnie_probe,
        .remove         = hi35xx_svp_nnie_remove,
        .driver         = {
                .name   = "hi35xx_nnie",
                .of_match_table = hi35xx_svp_nnie_match,
        },
};

osal_module_platform_driver(hi35xx_svp_nnie_driver);

MODULE_LICENSE("Proprietary");


