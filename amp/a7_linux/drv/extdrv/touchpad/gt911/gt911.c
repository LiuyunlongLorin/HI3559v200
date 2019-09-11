/*
 * GOODIX GT911 TouchScreen driver.
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

#if defined(HI3559V200)
#define RST_GPIO_CHIP (7)
#define RST_GPIO_OFFSET (0)
#define INT_GPIO_CHIP (8)
#define INT_GPIO_OFFSET (7)
#endif
#define gpio_num(gpio_chip_num, gpio_offset_num)  ((gpio_chip_num) * 8 + (gpio_offset_num))

/*start define of ft*/
static struct input_dev* tp_ts_dev = NULL;

volatile unsigned long gpio_base;

/**The TP can support 5 points, but we only use 1*/
#define MAX_TOUCH_POINTS             (1)

/**screen resolution*/
#define TP_SCREEN_WIDTH_NUM             (1600)
#define TP_SCREEN_HEIGHT_NUM            (400)

#define TP_EVENT_PRESS_DOWN             (0)
#define TP_EVENT_LIFT_UP                (1)
#define TP_EVENT_CONTACT                (2)

#define GT_BUFFER_STAT_ADDR             (0x814E)
#define GT_COORDINATE_X_LOWBYPE_BASE    (0x8150)
#define GT_COORDINATE_X_HIGHBYPE_BASE   (0x8151)
#define GT_COORDINATE_Y_LOWBYPE_BASE    (0x8152)
#define GT_COORDINATE_Y_HIGHBYPE_BASE   (0x8153)



#define DEFAULT_MD_LEN (128)



static const struct i2c_device_id ft_id[] =
{
    { "ft", },
    { }
};

typedef struct tagts_event
{
    u16 au16_x[MAX_TOUCH_POINTS]; /*x coordinate */
    u16 au16_y[MAX_TOUCH_POINTS]; /*y coordinate */
    u16 pressure[MAX_TOUCH_POINTS];
    u8 au8_touch_event[MAX_TOUCH_POINTS]; /* touch event: 0 -- down; 1-- up; 2 -- contact */
    u8 au8_finger_id[MAX_TOUCH_POINTS];   /*touch ID */
    u8 area[MAX_TOUCH_POINTS];
    u8 touch_point;
    u8 point_num;
} ts_event;

MODULE_DEVICE_TABLE(i2c, ft_id);

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

static int touch_reset_buffer(void)
{
    int i = 0;
    for(i = 0; i < 5; i++)
    {
        hi_i2c_write(GT_BUFFER_STAT_ADDR, 2, 0x0, 1);
    }
    return 0;
}

/**only support onetouch*/
static int touch_event_handler(ts_event *event)
{
    int ret = -1;
    int i = 0;
    //int data = 0;
    int fingernum = 0;
    int coord_x = 0;
    int coord_y = 0;
    static bool pretouch = 0;
    unsigned char data = 0;
    memset(event, 0, sizeof(ts_event));
    ret = hi_i2c_read(GT_BUFFER_STAT_ADDR, 2, &data, 1);
    if (data == 0x80)
    {
        if(pretouch)
        {
            event->point_num = 1;
            event->au8_finger_id[0] = 0;
            event->au8_touch_event[0] = TP_EVENT_LIFT_UP;
        }
        pretouch = 0;
        goto exit_work_func;
    }
    else if (data == 0)
    {
        return 0;
    }
    pretouch = 1;
    fingernum = data & 0x3;

    if (fingernum > 0)
    {
        unsigned char finggerdata[4] = {0};
        //coord_x = (hi_i2c_read_16bit(GT_COORDINATE_X_LOWBYPE_BASE, 2, 1) & 0xFF) |
        //      ((hi_i2c_read_16bit(GT_COORDINATE_X_HIGHBYPE_BASE, 2, 1) & 0xFF) << 8);

        //coord_y = (hi_i2c_read_16bit(GT_COORDINATE_Y_LOWBYPE_BASE, 2, 1) & 0xFF) |
        //      ((hi_i2c_read_16bit(GT_COORDINATE_Y_HIGHBYPE_BASE, 2, 1) & 0xFF) << 8);

        ret = hi_i2c_read(GT_COORDINATE_X_LOWBYPE_BASE, 2, finggerdata, 4);
        coord_x = (finggerdata[0] & 0xFF) | ((finggerdata[1] & 0xFF) << 8);
        coord_y = (finggerdata[2] & 0xFF) | ((finggerdata[3] & 0xFF) << 8);

        memset(event, 0, sizeof(ts_event));
        event->point_num = fingernum;

        event->touch_point = 0;

        for (i = 0; i < MAX_TOUCH_POINTS; i++)
        {
            event->touch_point++;

            /**Notice the  array bound !!*/
            event->au16_x[i] =
                coord_x;
            event->au16_y[i] =
                coord_y;
            event->au8_touch_event[i] =
                TP_EVENT_CONTACT;
            event->au8_finger_id[i] =
                0;

            if ((event->au8_touch_event[i]==0 || event->au8_touch_event[i]==2)&&(event->point_num==0))
            {
                printk("abnormal touch data from fw");
                return -1;
            }
        }
    }
    else
    {
        printk("The touch_handler is other number. %d. The finggernum is %d.\n",data, fingernum);
    }
exit_work_func:
    touch_reset_buffer();

    return 0;
}

static irqreturn_t tpint_irq(int irq, void *data)
{
    int i;
    bool act;
    struct input_dev* input = tp_ts_dev;
    ts_event event;

    int ret = touch_event_handler(&event);
    if(ret)
        printk( "\n buffer_read failed \n");

    #if 1
    if(event.point_num != 0)
    {
        for(i = 0; i < event.touch_point; i++)
        {
            input_mt_slot(input, i);
            act = (event.au8_touch_event[i] == TP_EVENT_PRESS_DOWN || event.au8_touch_event[i] == TP_EVENT_CONTACT);
            input_mt_report_slot_state(input, MT_TOOL_FINGER, act);
            if (!act)
                continue;
            input_report_abs(input, ABS_MT_POSITION_X, event.au16_x[i]);
            input_report_abs(input, ABS_MT_POSITION_Y, event.au16_y[i]);
        }
    }
    input_mt_sync_frame(input);
    input_sync(input);
    #endif
    touch_set_reg(0x120D841C, 0x1);
    return IRQ_HANDLED;
}

static int tp_irq_register(void)
{
    int error = 0;
    int ret = 0;
    int irq_num = 0;
    ret = gpio_request(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET), NULL);
    irq_num = gpio_to_irq(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET));

    touch_reset_buffer();

    error = request_threaded_irq(irq_num, NULL, tpint_irq, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "tpirq", tp_ts_dev);

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
    tp_ts_dev = input_allocate_device();
    if (tp_ts_dev == NULL)
    {
        printk(" func:%s line:%d \r\n", __FUNCTION__, __LINE__);
        return -1;
    }
    tp_ts_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
    set_bit(EV_SYN, tp_ts_dev->evbit);
    set_bit(EV_KEY, tp_ts_dev->evbit);
    set_bit(EV_ABS, tp_ts_dev->evbit);
    set_bit(INPUT_PROP_DIRECT, tp_ts_dev->propbit);
    input_set_abs_params(tp_ts_dev, ABS_MT_POSITION_X, 0 , TP_SCREEN_WIDTH_NUM, 0, 0);
    input_set_abs_params(tp_ts_dev, ABS_MT_POSITION_Y, 0, TP_SCREEN_HEIGHT_NUM, 0, 0);
    input_set_abs_params(tp_ts_dev, ABS_MT_TOUCH_MAJOR,
                         0, 0xff, 0, 0);
    input_set_abs_params(tp_ts_dev, ABS_MT_PRESSURE,
                         0, 0xff, 0, 0);
    input_set_abs_params(tp_ts_dev, ABS_MT_TRACKING_ID,
                         0, 0xff, 0, 0);

    error = input_mt_init_slots(tp_ts_dev, MAX_TOUCH_POINTS,
                                INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);

    if (error)
    { return error; }
    tp_ts_dev->name = "tp";
    tp_ts_dev->id.bustype = BUS_I2C;

    error = input_register_device(tp_ts_dev);
    if (error)
    {
        dev_err(NULL, "failed to register input device: %d\n", error);
        return error;
    }

    /**request_threaded_irq(unsigned int irq,irq_handler_t handler,irq_handler_t thread_fn,unsigned long irqflags,const char * devname,void * dev_id)*/
    error = tp_irq_register();
    if(error)
    {
        dev_err(NULL, "failed to register input device: %d\n", error);
        return error;
    }

    return 0;
}

static int __init tp_init(void)
{
    int ret = 0;
    hi_gpio_groupbit_info stIntInfo;
    hi_gpio_groupbit_info stRstInfo;

    stIntInfo.group_num = INT_GPIO_CHIP;
    stIntInfo.bit_num = INT_GPIO_OFFSET;
    stIntInfo.value = 0;

    stRstInfo.group_num = RST_GPIO_CHIP;
    stRstInfo.bit_num = RST_GPIO_OFFSET;
    stRstInfo.value = 0;

    ret = i2cdev_init();
    if (ret)
    {
        dev_err(NULL, " i2cdev_init fail!\n");
        goto error_end;
    }

    /**set INT and GPIO to be output*/
    stIntInfo.value = 1;
    stRstInfo.value = 1;
    hi_gpio_set_dir(&stIntInfo);
    hi_gpio_set_dir(&stRstInfo);

    msleep(1);

    /**Set Reset*/
    stRstInfo.value = 0;
    hi_gpio_write_bit(&stRstInfo);

    stIntInfo.value = 0;
    hi_gpio_write_bit(&stIntInfo);

    msleep(5);

    stRstInfo.value = 1;
    hi_gpio_write_bit(&stRstInfo);

    msleep(10);

    stIntInfo.value = 0;
    hi_gpio_set_dir(&stIntInfo);

    stRstInfo.value = 0;
    hi_gpio_set_dir(&stRstInfo);

    msleep(50);

    //ft_inputconfig();
    ret = devinput_init();
    if (ret)
    {
        dev_err(NULL, " devinput_init fail!\n");
        goto error_end;
    }
    printk(" func:%s line:%d .TouchPad Init OK.\r\n", __FUNCTION__, __LINE__);
    return 0;
error_end:
    return -1;
}

static void __exit tp_exit(void)
{
    free_irq(gpio_to_irq(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET)), tp_ts_dev);
    input_unregister_device(tp_ts_dev);
    input_free_device(tp_ts_dev);
    i2cdev_exit();
}

module_init(tp_init);
module_exit(tp_exit);


MODULE_AUTHOR("monster");
MODULE_DESCRIPTION("FocalTech FT TouchScreen driver");
MODULE_LICENSE("GPL");
