
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
#if defined(HI3559AV100)
#include "hi_gpio_grpinfo_hi3559av100.h"
#elif defined(HI3556AV100)
#include "hi_gpio_grpinfo_hi3556av100.h"
#elif defined(HI3519AV100)
#include "hi_gpio_grpinfo_hi3519av100.h"
#elif defined(HI3559V200)
#include "hi_gpio_grpinfo_hi3559v200.h"
#endif

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




#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36))
static int gpio_open(struct inode* inode, struct file* filp)
#else
static int gpio_open( struct file* filp)
#endif
{
    return 0;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36))
static int gpio_release(struct inode* inode, struct file* filp)
#else
static int gpio_release(struct file* filp)
#endif
{
    return 0;
}

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

EXPORT_SYMBOL(hi_gpio_set_dir);

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

EXPORT_SYMBOL(hi_gpio_get_dir);

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

EXPORT_SYMBOL(hi_gpio_write_bit);

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

EXPORT_SYMBOL(hi_gpio_read_bit);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static int gpio_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg)
#else
static long gpio_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
#endif
{
    hi_gpio_groupbit_info group_bit_info;
    int ret = 0;

    switch (_IOC_NR(cmd))
    {
        case _IOC_NR(GPIO_SET_DIR):

            ret = copy_from_user(&group_bit_info, (hi_gpio_groupbit_info*)arg, sizeof(hi_gpio_groupbit_info));
            if (0 != ret)
            {
                printk("copy_from_user error!\n");
                return -1;
            }

            ret = hi_gpio_set_dir(&group_bit_info);
            if (0 != ret)
            {
                printk("hi_gpio_set_dir error!\n");
                return -1;
            }

            break;

        case _IOC_NR(GPIO_GET_DIR):

            ret = copy_from_user(&group_bit_info, (hi_gpio_groupbit_info*)arg, sizeof(hi_gpio_groupbit_info));
            if (0 != ret)
            {
                printk("copy_from_user error!\n");
                return -1;
            }

            ret = hi_gpio_get_dir(&group_bit_info);
            if (0 != ret)
            {
                printk("hi_gpio_set_dir error!\n");
                return -1;
            }

            ret = copy_to_user((void __user*)arg, &group_bit_info, sizeof(hi_gpio_groupbit_info));
            if (0 != ret)
            {
                return -1;
            }

            break;

        case _IOC_NR(GPIO_READ_BIT):
            ret = copy_from_user(&group_bit_info, (hi_gpio_groupbit_info*)arg, sizeof(hi_gpio_groupbit_info));
            if (0 != ret)
            {
                printk("copy_from_user error!\n");
                return -1;
            }

            ret = hi_gpio_read_bit(&group_bit_info);
            if (0 != ret)
            {
                printk("hi_gpio_read_bit error!\n");
                return -1;
            }

            ret = copy_to_user((void __user*)arg, &group_bit_info, sizeof(hi_gpio_groupbit_info));
            if (0 != ret)
            {
                return -1;
            }

            break;

        case _IOC_NR(GPIO_WRITE_BIT):

            ret = copy_from_user(&group_bit_info, (hi_gpio_groupbit_info*)arg, sizeof(hi_gpio_groupbit_info));
            if (0 != ret)
            {
                printk("copy_from_user error!\n");
                return -1;
            }

            ret = hi_gpio_write_bit(&group_bit_info);
            if (0 != ret)
            {
                printk("hi_gpio_write_bit error!\n");
                return -1;
            }

            break;

        default:
            return -1;
    }
    return 0;
}



static struct file_operations gpio_fops =
{
    .owner = THIS_MODULE,
    .open  = gpio_open,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
    .ioctl = gpio_ioctl,
#else
    .unlocked_ioctl = gpio_ioctl,
#endif

    .release = gpio_release,
};

static struct miscdevice gpio_dev =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "hi_gpio",
    .fops  = &gpio_fops,
};

/*
 * gpio configure should be set in uboot tables
 */
static int __init hi_gpio_init(void)
{
    int ret = 0;
    ret = misc_register(&gpio_dev);
    if (ret)
    {
        printk(KERN_ERR "register misc dev for gpio fail!\n");
        return ret;
    }
    return 0;
}

static void __exit hi_gpio_exit(void)
{
    int gpio_grp_cnt = GPIO_GRP_CNT;
    int i = 0;

    printk("unregister gpio ko\n");
    misc_deregister(&gpio_dev);

    for (i = 0; i < gpio_grp_cnt; ++i)
    {
        if (s_astGpioGrpInfo[i].base_addr_virtual)
        {
            iounmap((void*)s_astGpioGrpInfo[i].base_addr_virtual);
        }
    }
}

module_init(hi_gpio_init);
module_exit(hi_gpio_exit);

MODULE_LICENSE("GPL");

