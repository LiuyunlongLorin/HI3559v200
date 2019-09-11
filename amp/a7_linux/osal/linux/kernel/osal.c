#include <linux/module.h>
#include <linux/kernel.h>
#include "hi_osal.h"

extern void osal_proc_init(void);
extern void osal_proc_exit(void);
extern int himedia_init(void);
extern void himedia_exit(void);
extern int media_mem_init(void);
extern void media_mem_exit(void);
extern int mem_check_module_param(void);
extern void osal_device_init(void);

static int __init osal_init(void)
{
    if (-1 == mem_check_module_param()) {
        return -1;
    }

    osal_device_init();
    osal_proc_init();
    himedia_init();
    media_mem_init();
    osal_printk("hi_osal %s init success!\n", HI_OSAL_VERSION);
    return 0;
}

static void __exit osal_exit(void)
{
    osal_proc_exit();
    himedia_exit();
    media_mem_exit();
    osal_printk("hi_osal v1.0 exit!\n");
}

module_init(osal_init);
module_exit(osal_exit);

MODULE_AUTHOR("Hisilicon");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
