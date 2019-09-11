
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/reboot.h>
#include "hi_osal.h"

int osal_register_reboot_notifier(struct osal_notifier_block *ob)
{
    return 0;
}

int osal_unregister_reboot_notifier(struct osal_notifier_block *nb)
{
    return 0;
}


