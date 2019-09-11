
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>


#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/version.h>
#include "hi_gpio.h"

/** gpio group list information */
#include "hi_gpio_grpinfo_hi3559v200.h"

#define GPIO_GRP_CNT ARRAY_SIZE(s_astGpioGrpInfo)

/** virtual group base address and data address, should be update in each ioctl*/
static unsigned long s_GpioGrpBaseAddr = 0;
static unsigned long s_GpioDataBaseAddr = 0;

#define GPIO_DIR_BASE  (s_GpioGrpBaseAddr+0x400)
#define GPIO_INTR_MASK (s_GpioGrpBaseAddr+0x410)
#define GPIO_DATA_BASE (s_GpioDataBaseAddr)

#define GPIO_MAP_LEN (0x500)

#define WRITE_REG(Addr, Value) ((*(volatile unsigned int*)((unsigned long)Addr)) = (Value))
#define READ_REG(Addr)         (*(volatile unsigned int *)((unsigned long)Addr))

#define GET_BIT(value, bitnum)   value &= (1 << bitnum)
#define SET_BIT(value, bitnum)   value |= (1 << bitnum)
#define CLEAR_BIT(value, bitnum) value &= ~(1 << bitnum)

static DEFINE_MUTEX(s_mutexlock);


static int gpio_get_grpindex(const hi_gpio_groupbit_info* pstGrpBitInfo)
{
    int gpio_grp_cnt = GPIO_GRP_CNT;
    int i = 0;

    for (i = 0; i < gpio_grp_cnt; ++i)
    {
        if ((s_astGpioGrpInfo[i].group_num == pstGrpBitInfo->group_num)
            && (pstGrpBitInfo->bit_num <= 7))
        {
            return i;
        }
    }

    printk("group number[%u] or bitnum[%u] beyond extent!\n",
           pstGrpBitInfo->group_num, pstGrpBitInfo->bit_num);
    return -1;
}

static int gpio_update_baseaddr(const hi_gpio_groupbit_info* pstGrpBitInfo, int grpIdx)
{
    hi_gpio_grp_info_s* pstGrpInfo = &s_astGpioGrpInfo[grpIdx];

    if (!pstGrpInfo->base_addr_virtual)
    {
        pstGrpInfo->base_addr_virtual =
            (unsigned long)ioremap_nocache(pstGrpInfo->base_addr, GPIO_MAP_LEN);

        if (!pstGrpInfo->base_addr_virtual)
        {
            printk("ioremap gpio group%d failed!\n", pstGrpInfo->group_num);
            return -1;
        }
    }

    s_GpioGrpBaseAddr = pstGrpInfo->base_addr_virtual;
    s_GpioDataBaseAddr = s_GpioGrpBaseAddr + (1 << (pstGrpBitInfo->bit_num + 2));
    return 0;
}

static int gpio_preproc_param(hi_gpio_groupbit_info* pstGrpBitInfo)
{
    int ret = 0;
    int grp_idx = 0;

    grp_idx = gpio_get_grpindex(pstGrpBitInfo);

    if (0 > grp_idx)
    {
        return -1;
    }

    ret = gpio_update_baseaddr(pstGrpBitInfo, grp_idx);

    if (0 != ret)
    {
        return -1;
    }

    return 0;
}

int hi_gpio_set_dir(hi_gpio_groupbit_info* pstGrpBitInfo)
{
    unsigned int reg_value = 0;
    int ret = 0;
    mutex_lock(&s_mutexlock);
    ret = gpio_preproc_param(pstGrpBitInfo);

    if (0 != ret)
    {
        mutex_unlock(&s_mutexlock);
        return -1;
    }

    reg_value = READ_REG(GPIO_DIR_BASE);

    if (0 == pstGrpBitInfo->value)
    {
        CLEAR_BIT(reg_value, pstGrpBitInfo->bit_num);
    }
    else if (1 == pstGrpBitInfo->value)
    {
        SET_BIT(reg_value, pstGrpBitInfo->bit_num);
    }
    else
    {
        printk("dir beyond of extent!\n");
        mutex_unlock(&s_mutexlock);
        return -1;
    }

    WRITE_REG(GPIO_DIR_BASE, reg_value);
    mutex_unlock(&s_mutexlock);
    return 0;
}


int hi_gpio_get_dir(hi_gpio_groupbit_info* pstGrpBitInfo)
{
    unsigned int reg_value = 0;
    int ret = 0;
    mutex_lock(&s_mutexlock);
    ret = gpio_preproc_param(pstGrpBitInfo);

    if (0 != ret)
    {
        mutex_unlock(&s_mutexlock);
        return -1;
    }

    reg_value = READ_REG(GPIO_DIR_BASE);
    GET_BIT(reg_value, pstGrpBitInfo->bit_num);
    pstGrpBitInfo->value = ((0 != reg_value) ? 1 : 0);
    mutex_unlock(&s_mutexlock);
    return 0;
}

int hi_gpio_write_bit(hi_gpio_groupbit_info* pstGrpBitInfo)
{
    unsigned int reg_value = 0;
    int ret = 0;
    mutex_lock(&s_mutexlock);
    ret = gpio_preproc_param(pstGrpBitInfo);

    if (0 != ret)
    {
        mutex_unlock(&s_mutexlock);
        return -1;
    }

    reg_value = READ_REG(GPIO_DATA_BASE);

    if (0 == pstGrpBitInfo->value)
    {
        CLEAR_BIT(reg_value, pstGrpBitInfo->bit_num);
    }
    else if (1 == pstGrpBitInfo->value)
    {
        SET_BIT(reg_value, pstGrpBitInfo->bit_num);
    }
    else
    {
        printk("write bit beyond of extent!\n");
        mutex_unlock(&s_mutexlock);
        return -1;
    }

    WRITE_REG(GPIO_DATA_BASE, reg_value);
    mutex_unlock(&s_mutexlock);

    return 0;
}

int hi_gpio_read_bit(hi_gpio_groupbit_info* pstGrpBitInfo)
{
    unsigned int reg_value = 0;
    int ret = 0;
    mutex_lock(&s_mutexlock);
    ret = gpio_preproc_param(pstGrpBitInfo);

    if (0 != ret)
    {
        mutex_unlock(&s_mutexlock);
        return -1;
    }

    reg_value = READ_REG(GPIO_DATA_BASE);
    GET_BIT(reg_value, pstGrpBitInfo->bit_num);
    pstGrpBitInfo->value = ((0 != reg_value) ? 1 : 0);
    mutex_unlock(&s_mutexlock);
    return 0;
}


int hi_gpio_init(void)
{
    return 0;
}
int hi_gpio_deinit(void)
{
    int gpio_grp_cnt = GPIO_GRP_CNT;
    int i = 0;

    for (i = 0; i < gpio_grp_cnt; ++i)
    {
        if (s_astGpioGrpInfo[i].base_addr_virtual)
        {
            iounmap((void*)s_astGpioGrpInfo[i].base_addr_virtual);
        }
    }

    return 0;
}
