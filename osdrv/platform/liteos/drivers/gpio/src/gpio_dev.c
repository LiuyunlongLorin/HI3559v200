#include "fcntl.h"
#include "linux/kernel.h"
#include "fs/fs.h"
#include "sys/ioctl.h"

#include "gpio.h"
#include "gpio_dev.h"

struct gpio_descriptor *gpio = NULL;

static int gpio_open(struct file *filep)
{
    return 0;
}

static int gpio_close(struct file *filep)
{
    return 0;
}

static int gpio_ioctl(struct file *filep, int cmd, unsigned long arg)
{
    int ret = 0;
    struct inode * inode = filep ->f_inode;
    struct gpio_descriptor *gd = (struct gpio_descriptor *)(inode->i_private);

    if(!gd)
    {
        gpio_err("gpio_descriptor is null!\n");
        return -1;
    }

    switch(cmd)
    {
        case GPIO_SET_DIR:
            if(gd->ops->setdir)
                ret = gd->ops->setdir(gd,(gpio_groupbit_info*)arg);
            else
                ret = -1;

            break;
        case GPIO_GET_DIR:
            if(gd->ops->getdir)
                ret = gd->ops->getdir(gd,(gpio_groupbit_info*)arg);
            else
                ret = -1;

            break;
        case GPIO_READ_BIT:
            if(gd->ops->readbit)
                ret = gd->ops->readbit(gd,(gpio_groupbit_info*)arg);
            else
                ret = -1;
            break;
        case GPIO_WRITE_BIT:
            if(gd->ops->writebit)
                ret = gd->ops->writebit(gd,(gpio_groupbit_info*)arg);
            else
                ret = -1;
            break;

        default:
            ret = -1;
    }

    return ret;

}

const struct file_operations_vfs gpio_dev_ops =
{
    gpio_open,  /* open */
    gpio_close, /* close */
    0,  /* read */
    0, /* write */
    0,          /* seek */
    gpio_ioctl  /* ioctl */
#ifndef CONFIG_DISABLE_POLL
        , 0 /* poll */
#endif
};
