#include "los_mux.h"

#include <stdlib.h>
#include "hisoc/gpio.h"
#include "gpio-pl061.h"
#include "asm/io.h"
#include "gpio.h"
#include "gpio_dev.h"

#include "linux/module.h"
#include "linux/device.h"
#include "linux/platform_device.h"


GPIO_IRQ_TAB;
GPIO_GROUP_TAB;

#define GPIO_DATA(group_reg_base, gpio_offset)         (((group_reg_base) + 0x000) + (1 << ((gpio_offset) + 2)))
#define GPIO_DIR(group_reg_base)                  ((group_reg_base) + 0x400)
#define GPIO_IS(group_reg_base)                   ((group_reg_base) + 0x404)
#define GPIO_IBE(group_reg_base)                  ((group_reg_base) + 0x408)
#define GPIO_IEV(group_reg_base)                  ((group_reg_base) + 0x40C)
#define GPIO_IE(group_reg_base)                   ((group_reg_base) + 0x410)
#define GPIO_RIS(group_reg_base)                  ((group_reg_base) + 0x414)
#define GPIO_MIS(group_reg_base)                  ((group_reg_base) + 0x418)
#define GPIO_IC(group_reg_base)                   ((group_reg_base) + 0x41C)

#define gpio_irq_lock(flags)  do { (*(flags)) = LOS_IntLock();} while(0)
#define gpio_irq_unlock(flags)  do { LOS_IntRestore(flags);} while(0)

void gpio_dir_config(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    addr = GPIO_DIR(pl061_group_ctrl[gpio_group].gpio_group_reg_base);
    val = readl(addr);
    if(flag)
    {
        val |= 1 << gpio_offset;
    }
    else
    {
        val &= ~(1 << gpio_offset);
    }
    writel(val, addr);
}

void gpio_write(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag)
{
    unsigned int val = 0;
    unsigned int addr = 0;

    addr = GPIO_DATA(pl061_group_ctrl[gpio_group].gpio_group_reg_base, gpio_offset);

    val = readl(addr);

    if(flag)
    {
        val |= (1 << gpio_offset);
    }
    else
    {
        val &= ~(1 << gpio_offset);
    }
    writel(val,addr);
}

unsigned int gpio_read(unsigned char gpio_group, unsigned char gpio_offset)
{
    unsigned int val = 0;
    unsigned int addr = 0;

    addr = GPIO_DATA(pl061_group_ctrl[gpio_group].gpio_group_reg_base, gpio_offset);
    val = readl(addr);
    if(val & (1 << gpio_offset))
    {
        val = 1;
    }
    else
    {
        val = 0;
    }
    return val;
}

void gpio_is_config(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag)
{
    unsigned int val = 0;
    unsigned int addr = 0;

    addr = GPIO_IS(pl061_group_ctrl[gpio_group].gpio_group_reg_base);
    val = readl(addr);
    if(flag)
        val |= 1 << gpio_offset;
    else
        val &= ~(1 << gpio_offset);
    writel(val, addr);
}

void gpio_ibe_config(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    addr = GPIO_IBE(pl061_group_ctrl[gpio_group].gpio_group_reg_base);
    val = readl(addr);
    if(flag)
        val |= 1 << gpio_offset;
    else
        val &= ~(1 << gpio_offset);
    writel(val, addr);
}

void gpio_iev_config(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    addr = GPIO_IEV(pl061_group_ctrl[gpio_group].gpio_group_reg_base);
    val = readl(addr);
    if(flag)
        val |= 1 << gpio_offset;
    else
        val &= ~(1 << gpio_offset);
    writel(val, addr);
}

void gpio_ie_config(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    addr = GPIO_IE(pl061_group_ctrl[gpio_group].gpio_group_reg_base);
    val = readl(addr);
    if(flag)
        val |= 1 << gpio_offset;
    else
        val &= ~(1 << gpio_offset);
    writel(val, addr);
}

unsigned int gpio_mis_read(unsigned char gpio_group, unsigned char gpio_offset)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    addr = GPIO_MIS(pl061_group_ctrl[gpio_group].gpio_group_reg_base);
    val = readl(addr);
    if(val & (1 << gpio_offset))
    {
        val = 1;
    }
    else
    {
        val = 0;
    }
    return val;
}

void gpio_ic_clear(unsigned char gpio_group, unsigned char gpio_offset)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    addr = GPIO_IC(pl061_group_ctrl[gpio_group].gpio_group_reg_base);
    val = readl(addr);
    val |= 1 << gpio_offset;
    writel(val, addr);
}


gpio_group_ctrl* get_gpio_group_ctrl(gpio_groupbit_info * gpio_info)
{
    if((gpio_info->groupnumber > GPIO_GROUP_NUM - 1)||(gpio_info->bitnumber > GPIO_BIT_NUM - 1)) {
        gpio_err("group number or bitnum beyond extent!\n");
        return NULL;
    }
    if (pl061_group_ctrl[gpio_info->groupnumber].group_num == GPIO_GROUP_DISABLE) {
        gpio_err("group %d, is disable or not exist!\n", gpio_info->groupnumber);
        return NULL;
    }

    return &pl061_group_ctrl[gpio_info->groupnumber];

}

gpio_irq_handler_tab * get_gpio_irq_handle(unsigned int irq_num)
{
    int i = 0;

    for(i = 0; i< GPIO_IRQ_NUM; i++)
    {
        if(pl061_irq_handler_tab[i].irq == irq_num)
            return &pl061_irq_handler_tab[i];
    }

    return NULL;
}

int gpio_get_direction(gpio_groupbit_info * gpio_info)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    gpio_group_ctrl * group_ctrl = NULL;

    group_ctrl = get_gpio_group_ctrl(gpio_info);
    if (!group_ctrl) {
        gpio_err("get group_ctrl fail ! group num = %d\n",gpio_info->groupnumber);
        return -1;
    }

    addr = GPIO_DIR(group_ctrl->gpio_group_reg_base);

    val = readl(addr);
    if(val & (1 << gpio_info->bitnumber))
        gpio_info->direction = GPIO_DIR_OUT;
    else
        gpio_info->direction = GPIO_DIR_IN;

    return 0;
}

int gpio_direction_input(gpio_groupbit_info * gpio_info)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    gpio_group_ctrl * group_ctrl = NULL;

    group_ctrl = get_gpio_group_ctrl(gpio_info);
    if (!group_ctrl) {
        gpio_err("get group_ctrl fail ! group num = %d\n",gpio_info->groupnumber);
        return -1;
    }

    if(gpio_info->direction == GPIO_DIR_IN)
    {
        (void)LOS_MuxPend(group_ctrl->group_lock, LOS_WAIT_FOREVER);
        addr = GPIO_DIR(group_ctrl->gpio_group_reg_base);
        val = readl(addr);
        val &= ~(1 << gpio_info->bitnumber);
        writel(val,addr);
        (void)LOS_MuxPost(group_ctrl->group_lock);

        return 0;
    }
    return -1;
}

int gpio_direction_output(gpio_groupbit_info * gpio_info)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    gpio_group_ctrl * group_ctrl = NULL;

    group_ctrl = get_gpio_group_ctrl(gpio_info);
    if (!group_ctrl) {
        gpio_err("get group_ctrl fail ! group num = %d\n",gpio_info->groupnumber);
        return -1;
    }

    if(gpio_info->direction == GPIO_DIR_OUT)
    {
        addr = GPIO_DIR(group_ctrl->gpio_group_reg_base);
        (void)LOS_MuxPend(group_ctrl->group_lock, LOS_WAIT_FOREVER);
        val = readl(addr);
        val |= (1 << gpio_info->bitnumber);
        writel(val,addr);
        (void)LOS_MuxPost(group_ctrl->group_lock);

        return 0;
    }
    return -1;
}

int gpio_get_value(gpio_groupbit_info * gpio_info)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    gpio_group_ctrl * group_ctrl = NULL;

    group_ctrl = get_gpio_group_ctrl(gpio_info);
    if (!group_ctrl) {
        gpio_err("get group_ctrl fail ! group num = %d\n",gpio_info->groupnumber);
        return -1;
    }

    addr = GPIO_DATA(group_ctrl->gpio_group_reg_base, gpio_info->bitnumber);
    val = readl(addr);
    if(val & (1 << gpio_info->bitnumber))
        gpio_info->value = GPIO_VALUE_HIGH;
    else
        gpio_info->value = GPIO_VALUE_LOW;

    return 0;
}

int gpio_set_value(gpio_groupbit_info * gpio_info)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    gpio_group_ctrl * group_ctrl = NULL;


    group_ctrl = get_gpio_group_ctrl(gpio_info);
    if (!group_ctrl) {
        gpio_err("get group_ctrl fail ! group num = %d\n",gpio_info->groupnumber);
        return -1;
    }

    addr = GPIO_DATA(group_ctrl->gpio_group_reg_base,gpio_info->bitnumber);
    (void)LOS_MuxPend(group_ctrl->group_lock, LOS_WAIT_FOREVER);
    val = readl(addr);

    if (gpio_info->value == GPIO_VALUE_HIGH)
        val |= 1 << gpio_info->bitnumber;
    else if (gpio_info->value == GPIO_VALUE_LOW)
        val &= ~(1 << gpio_info->bitnumber);
    else
        gpio_err("gpio_info->value = %d is illegal!\n", gpio_info->value);

    writel(val, addr);
    (void)LOS_MuxPost(group_ctrl->group_lock);

    return 0;
}

irqreturn_t public_func_noshare(int irq, void *arg)
{
    unsigned int i = 0;
    unsigned int val = 0;
    gpio_group_ctrl * group_ctrl = (gpio_group_ctrl*)arg;

    val = readl(GPIO_MIS(group_ctrl->gpio_group_reg_base));
    writel(val, GPIO_IC(group_ctrl->gpio_group_reg_base));
    for(i = 0; i < GPIO_BIT_NUM; i++) {
        if ((val & 0x1) && group_ctrl->handler_ctrl[i].handler) {
            group_ctrl->handler_ctrl[i].handler(irq, group_ctrl->handler_ctrl[i].data);
        }
        val >>= 1;
        if (val == 0)
            break;
    }
    return IRQ_HANDLED;
}

irqreturn_t public_func_share(int irq, void *arg)
{
    int i = 0,j = 0;
    unsigned int val = 0;
    unsigned int share = 0;
    void * data = NULL;
    gpio_irq_handler_tab * irq_tab = NULL;
    gpio_group_ctrl * group_ctrl = NULL;

    irq_tab = get_gpio_irq_handle(irq);
    if(!irq_tab) {
        gpio_err("get gpio_group fail!\n");
        goto out;
    }
    share = irq_tab->share;

    do
    {
        if (i >= GPIO_GROUP_NUM) {
            gpio_err("irq not match: irq :%d", irq);
            goto out;
        }

        if (pl061_group_ctrl[i].group_num == GPIO_GROUP_DISABLE)
            continue;

        group_ctrl = &pl061_group_ctrl[i];
        i++;
        if (group_ctrl->irq != irq)
            continue;

        /* match irq */
        val = readl(GPIO_MIS(group_ctrl->gpio_group_reg_base));
        writel(val, GPIO_IC(group_ctrl->gpio_group_reg_base));
        for(j = 0; j < GPIO_BIT_NUM; j++) {
            if ((val & 0x1) && group_ctrl->handler_ctrl[j].handler) {
                group_ctrl->handler_ctrl[j].handler(irq, group_ctrl->handler_ctrl[j].data);
            }
            val >>= 1;
            if (val == 0)
                break;
        }
        share--;
    } while(share);

out:
    return IRQ_HANDLED;
}

int gpio_irq_register(gpio_groupbit_info * gpio_info)
{
    int ret = 0;
    unsigned long flags = 0;
    gpio_group_ctrl * group_ctrl = NULL;
    gpio_irq_handler_tab * irq_tab = NULL;

    if (!gpio_info->irq_handler) {
        gpio_err("irq handler is NULL!\n");
        return -1;
    }

    group_ctrl = get_gpio_group_ctrl(gpio_info);
    if (!group_ctrl) {
        gpio_err("get group_ctrl fail ! group num = %d\n",gpio_info->groupnumber);
        return -1;
    }

    irq_tab = get_gpio_irq_handle(group_ctrl->irq);
    if(!irq_tab) {
        gpio_err("get gpio_group fail!\n");
        return -1;
    }

    gpio_set_irq_type(gpio_info);

    /* lock irq here */
    gpio_irq_lock(&flags);
    group_ctrl->handler_ctrl[gpio_info->bitnumber].data = gpio_info->data;
    group_ctrl->handler_ctrl[gpio_info->bitnumber].handler = gpio_info->irq_handler;
    gpio_irq_unlock(flags);

    if (irq_tab->irq_handler != NULL) {
        gpio_msg("Success:gpio public irq func was registered!\n");
        return ret;
    }

    if (irq_tab->share != GPIO_IRQ_NOSHAREABLE)
    {
        ret = request_irq(irq_tab->irq, (irq_handler_t)public_func_share, 0, "GPIO", NULL);
        if (ret) {
            gpio_err("register irq for gpio error!,irq num = %d\n", irq_tab->irq);
            goto out;
        }
        irq_tab->irq_handler = (irq_func)public_func_share;
    } else if(irq_tab->share == GPIO_IRQ_NOSHAREABLE) {
        ret = request_irq(irq_tab->irq, (irq_handler_t)public_func_noshare, 0, "GPIO", group_ctrl);
        if (ret) {
            gpio_err("register irq for gpio error!,irq num = %d\n",irq_tab->irq);
            goto out;
        }
        irq_tab->irq_handler = (irq_func)public_func_noshare;
    }

out:
    return ret;
}

int gpio_clear_irq(gpio_groupbit_info * gpio_info)
{
    unsigned int gpio_ic = 0;
    gpio_group_ctrl * group_ctrl = NULL;

    group_ctrl = get_gpio_group_ctrl(gpio_info);
    if (!group_ctrl) {
        gpio_err("get group_ctrl fail ! group num = %d\n",gpio_info->groupnumber);
        return -1;
    }

    (void)LOS_MuxPend(group_ctrl->group_lock, LOS_WAIT_FOREVER);
    gpio_ic = readl(GPIO_IC(group_ctrl->gpio_group_reg_base));
    gpio_ic |= 1 << gpio_info->bitnumber;
    writel(gpio_ic , GPIO_IC(group_ctrl->gpio_group_reg_base));
    (void)LOS_MuxPost(group_ctrl->group_lock);
    return 0;
}

int gpio_set_irq_type(gpio_groupbit_info * gpio_info)
{
    unsigned int gpio_ibe = 0;
    unsigned int gpio_is = 0;
    unsigned int gpio_iev = 0;
    gpio_group_ctrl * group_ctrl = NULL;

    if (gpio_info->irq_type == IRQ_TYPE_NONE)
        return 0;

    group_ctrl = get_gpio_group_ctrl(gpio_info);
    if (!group_ctrl) {
        gpio_err("get group_ctrl fail ! group num = %d\n",gpio_info->groupnumber);
        return -1;
    }

    /* lock gpio group here */
    (void)LOS_MuxPend(group_ctrl->group_lock, LOS_WAIT_FOREVER);
    gpio_is = readl(GPIO_IS(group_ctrl->gpio_group_reg_base));
    gpio_iev = readl(GPIO_IEV(group_ctrl->gpio_group_reg_base));

    if (gpio_info->irq_type & (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH)) {
        gpio_is |= 1 << gpio_info->bitnumber;
        if ((gpio_info->irq_type & IRQ_TYPE_LEVEL_HIGH) == IRQ_TYPE_LEVEL_HIGH)
            gpio_iev |= 1 << gpio_info->bitnumber;
        else
            gpio_iev &= ~(1 << gpio_info->bitnumber);
    } else {
        gpio_ibe = readl(GPIO_IBE(group_ctrl->gpio_group_reg_base));
        gpio_is &= ~(1 << gpio_info->bitnumber);
        if ((gpio_info->irq_type & IRQ_TYPE_EDGE_BOTH) == IRQ_TYPE_EDGE_BOTH) {
            gpio_ibe |= 1 << gpio_info->bitnumber;
        } else if ((gpio_info->irq_type & IRQ_TYPE_EDGE_RISING) == IRQ_TYPE_EDGE_RISING) {
            gpio_ibe &= ~(1 << gpio_info->bitnumber);
            gpio_iev |= 1 << gpio_info->bitnumber;
        } else if ((gpio_info->irq_type & IRQ_TYPE_EDGE_FALLING) == IRQ_TYPE_EDGE_FALLING) {
            gpio_ibe &= ~(1 << gpio_info->bitnumber);
            gpio_iev &= ~(1 << gpio_info->bitnumber);
        }
        writel(gpio_ibe, GPIO_IBE(group_ctrl->gpio_group_reg_base));
    }
    writel(gpio_is, GPIO_IS(group_ctrl->gpio_group_reg_base));
    writel(gpio_iev, GPIO_IEV(group_ctrl->gpio_group_reg_base));
    (void)LOS_MuxPost(group_ctrl->group_lock);
    return 0;
}

int gpio_irq_enable(gpio_groupbit_info * gpio_info)
{
    unsigned int val = 0;
    unsigned int addr = 0;
    gpio_group_ctrl * group_ctrl = NULL;

    group_ctrl = get_gpio_group_ctrl(gpio_info);
    if (!group_ctrl) {
        gpio_err("get group_ctrl fail ! group num = %d\n",gpio_info->groupnumber);
        return -1;
    }

    if(gpio_info->irq_enable == GPIO_IRQ_ENABLE) {
        (void)LOS_MuxPend(group_ctrl->group_lock, LOS_WAIT_FOREVER);
        addr = GPIO_IE(group_ctrl->gpio_group_reg_base);
        val = readl(addr);
        val |= (1 << gpio_info->bitnumber);
        writel(val,addr);
        (void)LOS_MuxPost(group_ctrl->group_lock);
    } else {
        (void)LOS_MuxPend(group_ctrl->group_lock, LOS_WAIT_FOREVER);
        addr = GPIO_IE(group_ctrl->gpio_group_reg_base);
        val = readl(addr);
        val &= ~(1 << gpio_info->bitnumber);
        writel(val,addr);
        (void)LOS_MuxPost(group_ctrl->group_lock);
    }
    return 0;
}

int gpio_get_irq_status(gpio_groupbit_info * gpio_info)
{
    unsigned int status;
    gpio_group_ctrl * group_ctrl = NULL;

    group_ctrl = get_gpio_group_ctrl(gpio_info);
    if (!group_ctrl) {
        gpio_err("get group_ctrl fail ! group num = %d\n",gpio_info->groupnumber);
        return -1;
    }

    gpio_info->irq_status = gpio_mis_read(gpio_info->groupnumber,gpio_info->bitnumber);
    status = readl(GPIO_MIS(group_ctrl->gpio_group_reg_base));
    gpio_info->irq_status = (status >> gpio_info->bitnumber) & 0x1;

    return 0;
}


static int pl061_setdir(struct gpio_descriptor *gd,gpio_groupbit_info* gpio_info)
{
    int ret = 0;

    if (gpio_info->direction == GPIO_DIR_OUT)
        ret = gpio_direction_output(gpio_info);
    else if (gpio_info->direction == GPIO_DIR_IN)
        ret = gpio_direction_input(gpio_info);
    else
        gpio_err("illegal input!\n");

    if (ret)
        gpio_err("gpio setdir fail!\n");

    return 0;
}

static int pl061_getdir(struct gpio_descriptor *gd,gpio_groupbit_info* gpio_info)
{
    int ret = 0;

    ret = gpio_get_direction(gpio_info);
    if (ret) {
        gpio_err("gpio getbit fail!\n");
        return -1;
    }

    return 0;
}

static int pl061_readbit(struct gpio_descriptor *gd,gpio_groupbit_info* gpio_info)
{
    int ret = 0;

    ret = gpio_get_value(gpio_info);
    if (ret) {
        gpio_err("gpio readbit fail!\n");
        return -1;
    }
    return 0;
}

static int pl061_writebit(struct gpio_descriptor *gd,gpio_groupbit_info* gpio_info)
{
    int ret = 0;
    ret = gpio_set_value(gpio_info);
    if (ret) {
        gpio_err("gpio writebit fail!\n");
        return -1;
    }

    return 0;
}

static struct gpio_ops pl061_ops = {
    .setdir        = pl061_setdir,
    .getdir        = pl061_getdir,
    .readbit       = pl061_readbit,
    .writebit      = pl061_writebit
};

int gpio_chip_init(struct gpio_descriptor *gd)
{
    unsigned int i = 0;
    gpio_group_ctrl * group_ctrl = NULL;

#ifdef LOSCFG_FS_VFS
    if(gd) {
        gd->private = NULL;
        gd->ops = &pl061_ops;
    } else {
        gpio_err("gpio_chip_init due to sys mem not enough!\n");
        return -1;
    }
#endif
    /* init lock for every gpio group , if one of group lock fail ,
     * delet all lock and return fail.
     * */
    for (i = 0;i < GPIO_GROUP_NUM;i++) {
        if (pl061_group_ctrl[i].group_num == GPIO_GROUP_DISABLE) {
            gpio_err("group %d, is disable or not exist!\n", i);
            continue;
        }
        group_ctrl = &pl061_group_ctrl[i];

        (void)LOS_MuxCreate(&(group_ctrl->group_lock));

    }
    return 0;
}

int gpio_chip_deinit(struct gpio_descriptor *gd)
{
    int ret = 0, i = 0;
    gpio_group_ctrl * group_ctrl = NULL;

#ifdef LOSCFG_FS_VFS
    if(!gd) {
        gpio_err("gd is null!\n");
        return -1;
    }
#endif
    for (i = 0;i < GPIO_GROUP_NUM;i++) {
        if (pl061_group_ctrl[i].group_num == GPIO_GROUP_DISABLE)
            continue;

        group_ctrl = &pl061_group_ctrl[i];
        LOS_MuxDelete(group_ctrl->group_lock);
    }

    return ret;
}

extern const struct file_operations_vfs gpio_dev_ops;
int pl061_probe(struct platform_device *pdev)
{
    struct gpio_descriptor *gd = NULL;
    int ret = -1;
    gpio_groupbit_info* gpio_info = NULL;

    gd = (struct gpio_descriptor *)dev_get_drvdata(&pdev->dev);
    if (gd == NULL)
    {
        gpio_err("get gpio data err!\n");
        return ret;
    }

    ret = gpio_chip_init(gd);
    if (ret) {
        gpio_err("gpio_init fail!\n");
        return ret;
    }

#ifdef LOSCFG_FS_VFS
    gpio_info = (gpio_groupbit_info*)malloc(sizeof(gpio_groupbit_info));
    if (!gpio_info) {
        gpio_err("get gpio descptor fail!\n");
        return -1;
    }

    gd->private = gpio_info;
    ret = register_driver("/dev/gpio", &gpio_dev_ops, 0666, gd);
    if (ret) {
        gpio_chip_deinit(gd);
        gpio_err("register_driver /dev/gpio failed!\n");
        free(gpio_info);
        return -1;
    }
#endif

    return 0;
}
int pl061_remove(struct platform_device *pdev)
{
    int ret = 0;
    struct gpio_descriptor *gd = NULL;

    gd = (struct gpio_descriptor *)dev_get_drvdata(&pdev->dev);
    if (gd == NULL)
    {
        gpio_err("get gpio data err!\n");
        return -1;
    }
    else
    {
#ifdef LOSCFG_FS_VFS
        ret = unregister_driver("/dev/gpio");
        if(!ret)
        {
            gpio_err("unregister_driver fail, ret= %d\n", ret);
        }
        free(gd->private);
        gd->private = NULL;
#endif
        gpio_chip_deinit(gd);
    }

    return ret;

}

static struct platform_driver pl061_driver = {
    .probe      = pl061_probe,
    .remove     = pl061_remove,
    .driver     = {
        .name   = "gpio_pl061",
    },
};

struct gpio_descriptor gpio0_pl061_driver = { /*lint !e10 !e129 !e121*/
    .group_num = GPIO_GROUP_NUM,
    .bit_num   = GPIO_BIT_NUM,
};

struct platform_device device_gpio0 = { /*lint !e10 !e129 !e121*/
    .name   = "gpio_pl061",
    .id     = 0,
    .dev    = {
        .driver_data = &gpio0_pl061_driver,
    },
};

int pl061_init(void)
{
    int ret = 0;

    ret = platform_device_register(&device_gpio0);
    if (ret)
    {
        gpio_err("register gpio device failed!\n");
        return -1;
    }

    ret = platform_driver_register(&pl061_driver);
    if (ret)
    {
        platform_device_unregister(&device_gpio0);
        gpio_err("register gpio driver failed!\n");
        return -1;
    }

    return ret;
}

void pl061_exit(void)
{
    platform_device_unregister(&device_gpio0);
    platform_driver_unregister(&pl061_driver);
}
module_init(pl061_init);
module_exit(pl061_exit);

int gpio_dev_init(void)
{
    return pl061_init();
}

