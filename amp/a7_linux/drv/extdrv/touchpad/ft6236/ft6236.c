/*
 * FocalTech FT6236 TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/property.h>
#include <asm/irq.h>


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
//#include <mach/io.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/i2c.h>
#include "i2cdev.h"
#include "hi_gpio.h"

#if (defined(BOARD_ACTIONCAM_REFB) && defined(HI3556AV100))
#define RST_GPIO_CHIP (7)
#define RST_GPIO_OFFSET (2)
#define INT_GPIO_CHIP (8)
#define INT_GPIO_OFFSET (1)
#elif (defined(BOARD_DEMB) && (defined(HI3556AV100) || defined(HI3519AV100)))
#define RST_GPIO_CHIP (2)
#define RST_GPIO_OFFSET (6)
#define INT_GPIO_CHIP (2)
#define INT_GPIO_OFFSET (5)
#elif defined(HI3559V200)
#define RST_GPIO_CHIP (0)
#define RST_GPIO_OFFSET (3)
#define INT_GPIO_CHIP (0)
#define INT_GPIO_OFFSET (4)
#endif
#define gpio_num(gpio_chip_num, gpio_offset_num)  ((gpio_chip_num) * 8 + (gpio_offset_num))

/*start define of ft*/
static struct input_dev* ft_ts_dev = NULL;

volatile unsigned long gpio_base;

/**The TP can support 5 points, but we only use 1*/
#define FT_MAX_TOUCH_POINTS             (1)

/**if 5 points, the number should be 63*/
#define FT_MAX_BUFUSED_NUM              (7)

/**screen resolution*/
#define FT_SCREEN_WIDTH_NUM             (240)
#define FT_SCREEN_HEIGHT_NUM            (320)

#define FT_EVENT_PRESS_DOWN             (0)
#define FT_EVENT_LIFT_UP                (1)
#define FT_EVENT_CONTACT                (2)
#define FT_EVENT_NO_EVENT               (3)
#define FT_TOUCH_EVENT_POS              (3)
#define FT_TOUCH_X_H_POS                (3)
#define FT_TOUCH_X_L_POS                (4)
#define FT_TOUCH_Y_H_POS                (5)
#define FT_TOUCH_FINGER_POS             (5)
#define FT_TOUCH_Y_L_POS                (6)
#define FT_TOUCH_PRE_POS                (7)
#define FT_TOUCH_AREA_POS               (8)
//#define FT_IRQ_NUM                      (220)


#define DEFAULT_MD_LEN (128)


static const struct i2c_device_id ft_id[] =
{
    { "ft", },
    { }
};

typedef struct tagts_event
{
    u16 au16_x[FT_MAX_TOUCH_POINTS]; /*x coordinate */
    u16 au16_y[FT_MAX_TOUCH_POINTS]; /*y coordinate */
    u16 pressure[FT_MAX_TOUCH_POINTS];
    u8 au8_touch_event[FT_MAX_TOUCH_POINTS]; /* touch event: 0 -- down; 1-- up; 2 -- contact */
    u8 au8_finger_id[FT_MAX_TOUCH_POINTS];   /*touch ID */
    u8 area[FT_MAX_TOUCH_POINTS];
    u8 touch_point;
    u8 point_num;
} ts_event;


MODULE_DEVICE_TABLE(i2c, ft_id);


static int ft_reset(hi_gpio_groupbit_info* pstRstInfo)
{
    pstRstInfo->value = 1;
    hi_gpio_write_bit(pstRstInfo);
    pstRstInfo->value = 0;
    hi_gpio_write_bit(pstRstInfo);
    msleep(5);
    pstRstInfo->value = 1;
    hi_gpio_write_bit(pstRstInfo);
    return 0;
}

static int touch_set_reg(unsigned int Addr, unsigned int Value)
{
    void* pmem = ioremap_nocache(Addr, DEFAULT_MD_LEN);
    if (pmem == NULL)
    {
        return -1;
    }

    *(unsigned int*)pmem = Value;
    iounmap(pmem);
    return 0;
}

static int touch_event_handler(ts_event *event)
{
    u8 buf[FT_MAX_BUFUSED_NUM] = {0};
    int ret = -1;
    int i = 0;

    ret = hi_i2c_read(0x0, buf, FT_MAX_BUFUSED_NUM);
    if (ret)
    {
        printk("[B]Read touchdata failed, ret: %d", ret);
        return ret;
    }

    memset(event, 0, sizeof(ts_event));
    event->point_num = buf[2] & 0x0F;

    if (event->point_num > 5)
    {
        event->point_num = 5;
    }
    event->touch_point = 0;

    for (i = 0; i < FT_MAX_TOUCH_POINTS; i++)
    {
        event->touch_point++;

        /**Notice the  array bound !!*/
        event->au16_x[i] =
            (s16) (buf[FT_TOUCH_X_H_POS + 6 * i] & 0x0F) << 8 | (s16) buf[FT_TOUCH_X_L_POS + 6 * i];
        event->au16_y[i] =
            (s16) (buf[FT_TOUCH_Y_H_POS + 6 * i] & 0x0F) << 8 | (s16) buf[FT_TOUCH_Y_L_POS + 6 * i];
        event->au8_touch_event[i] =
            buf[FT_TOUCH_EVENT_POS + 6 * i] >> 6;
        event->au8_finger_id[i] =
            (buf[FT_TOUCH_FINGER_POS + 6 * i]) >> 4;

        /**pressure and area not used !!*/
#if 0
        event->area[i] =
            (buf[FT_TOUCH_AREA_POS + 6 * i]) >> 4;
        event->pressure[i] =
            (s16) buf[FTS_TOUCH_PRE_POS + 6 * i];

        if (0 == event->area[i])
            event->area[i] = 0x09;

        if (0 == event->pressure[i])
            event->pressure[i] = 0x3f;
#endif
        if ((event->au8_touch_event[i]==0 || event->au8_touch_event[i]==2)&&(event->point_num==0))
        {
            printk("abnormal touch data from fw");
            return -1;
        }
    }
    return 0;
}

static irqreturn_t ftint_irq(int irq, void *data)
{
    int i;
    bool act;
    struct input_dev* input = ft_ts_dev;
    ts_event event;
    int ret = touch_event_handler(&event);
    if(ret)
        printk( "\n buffer_read failed \n");
    if(event.point_num != 0)
    {
        for(i = 0; i < event.touch_point; i++)
        {
            input_mt_slot(input, i);
            act = (event.au8_touch_event[i] == FT_EVENT_PRESS_DOWN || event.au8_touch_event[i] == FT_EVENT_CONTACT);
            input_mt_report_slot_state(input, MT_TOOL_FINGER, act);
            if (!act)
                continue;
            input_report_abs(input, ABS_MT_POSITION_X, event.au16_x[i]);
            input_report_abs(input, ABS_MT_POSITION_Y, event.au16_y[i]);
        }
    }
    input_mt_sync_frame(input);
    input_sync(input);
    return IRQ_HANDLED;
}

static int fts_irq_register(void)
{
    int error = 0;
    int ret = 0;
    int irq_num = 0;
    ret = gpio_request(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET), NULL);
    irq_num = gpio_to_irq(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET));
#if defined(BOARD_ACTIONCAM_REFB)
    error = request_threaded_irq(irq_num, NULL, ftint_irq, IRQF_TRIGGER_RISING | IRQF_ONESHOT, "ftirq", ft_ts_dev);
#else
    error = request_threaded_irq(irq_num, NULL, ftint_irq, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "ftirq", ft_ts_dev);
#endif
    if (error)
    {
        dev_err(NULL, "request irq failed: %d\n", error);
        return error;
    }
    return 0;
}

static int devinput_init(void)
{
    int error = 0;
    /* 1. distribution a "input_dev" structure */
    ft_ts_dev = input_allocate_device();
    if (ft_ts_dev == NULL)
    {
        printk(" func:%s line:%d \r\n", __FUNCTION__, __LINE__);
        return -1;
    }
    ft_ts_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
    set_bit(EV_SYN, ft_ts_dev->evbit);
    set_bit(EV_KEY, ft_ts_dev->evbit);
    set_bit(EV_ABS, ft_ts_dev->evbit);
    set_bit(INPUT_PROP_DIRECT, ft_ts_dev->propbit);
    input_set_abs_params(ft_ts_dev, ABS_MT_POSITION_X, 0 , FT_SCREEN_WIDTH_NUM, 0, 0);
    input_set_abs_params(ft_ts_dev, ABS_MT_POSITION_Y, 0, FT_SCREEN_HEIGHT_NUM, 0, 0);
    input_set_abs_params(ft_ts_dev, ABS_MT_TOUCH_MAJOR,
                         0, 0xff, 0, 0);
    input_set_abs_params(ft_ts_dev, ABS_MT_PRESSURE,
                         0, 0xff, 0, 0);
    input_set_abs_params(ft_ts_dev, ABS_MT_TRACKING_ID,
                         0, 0xff, 0, 0);

    error = input_mt_init_slots(ft_ts_dev, FT_MAX_TOUCH_POINTS,
                                INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);

    if (error)
    { return error; }
    ft_ts_dev->name = "ft";
    ft_ts_dev->id.bustype = BUS_I2C;

    error = input_register_device(ft_ts_dev);
    if (error)
    {
        dev_err(NULL, "failed to register input device: %d\n", error);
        return error;
    }

    /**request_threaded_irq(unsigned int irq,irq_handler_t handler,irq_handler_t thread_fn,unsigned long irqflags,const char * devname,void * dev_id)*/
    error = fts_irq_register();
    if(error)
    {
        dev_err(NULL, "failed to register input device: %d\n", error);
        return error;
    }
    /**Clear INT*/
    touch_set_reg(0x045f241C, 0x20);
    return 0;
}


#if 0
static void fts_gpio_deconfigure(void)
{
    if (gpio_is_valid(gpio_num(RST_GPIO_CHIP, RST_GPIO_OFFSET)))
        gpio_free(gpio_num(RST_GPIO_CHIP, RST_GPIO_OFFSET));
    if (gpio_is_valid(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET)))
        gpio_free(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET));
}
#endif

static int __init ft_init(void)
{
    int ret = 0;
    hi_gpio_groupbit_info stIntInfo;
    hi_gpio_groupbit_info stRstInfo;

    stIntInfo.group_num = INT_GPIO_CHIP;
    stIntInfo.bit_num = INT_GPIO_OFFSET;
    stIntInfo.value = 0;

    stRstInfo.group_num = RST_GPIO_CHIP;
    stRstInfo.bit_num = RST_GPIO_OFFSET;
    stRstInfo.value = 1;

    ret = i2cdev_init();
    if (ret)
    {
        dev_err(NULL, " i2cdev_init fail!\n");
        goto error_end;
    }

    hi_gpio_set_dir(&stIntInfo);
    hi_gpio_set_dir(&stRstInfo);

    ret = ft_reset(&stRstInfo);
    if (ret)
    {
        dev_err(NULL, " ft_reset fail!\n");
        goto error_end;
    }
    ret = devinput_init();
    if (ret)
    {
        dev_err(NULL, " devinput_init fail!\n");
        goto error_end;
    }

    return 0;
error_end:
    return -1;
}

static void __exit ft_exit(void)
{
    free_irq(gpio_to_irq(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET)), ft_ts_dev);
    input_unregister_device(ft_ts_dev);
    input_free_device(ft_ts_dev);
    i2cdev_exit();
}

module_init(ft_init);
module_exit(ft_exit);


MODULE_AUTHOR("monster");
MODULE_DESCRIPTION("FocalTech FT TouchScreen driver");
MODULE_LICENSE("GPL");
