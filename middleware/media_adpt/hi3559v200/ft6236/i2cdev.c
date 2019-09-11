#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include <linux/input-polldev.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <asm/dma.h>

#include "i2cdev.h"

/** The Number should be change, if I2C device not used*/

#if defined (HI3556AV100)
#define I2C_NUM 4
#elif defined (HI3559V200)
#define I2C_NUM 3
#endif

#define FST_ADDR 0x38

static struct i2c_board_info hi_info =
{
    I2C_BOARD_INFO("i2c-ft6236", FST_ADDR),
};

static struct i2c_client* hi_client;

unsigned char hi_i2c_read(unsigned char reg_addr, unsigned char* reg_data, unsigned int cnt)
{
    int ret;
    struct i2c_msg msg[2];

    memset(msg, 0x0, sizeof(struct i2c_msg) * 2);
    reg_data[0] = reg_addr;   //the first byte is reg addr;

    msg[0].addr   = hi_client->addr;
    msg[0].flags  = hi_client->flags & I2C_M_TEN;
    msg[0].len    = 1;
    msg[0].buf    = &reg_addr;

    msg[1].addr   = hi_client->addr;
    msg[1].flags  = hi_client->flags & I2C_M_TEN;
    msg[1].flags |= I2C_M_RD;
    msg[1].len    = cnt;
    msg[1].buf    = reg_data;
    ret = i2c_transfer(hi_client->adapter, msg, 2);

    if (ret != 2)
    {
        printk("hi_i2c_read err\n");
        return -EIO;
    }

    return 0;
}

int hi_i2c_write(unsigned char dev_addr, unsigned int reg_addr,
                 unsigned int reg_addr_num, unsigned int data, unsigned int data_byte_num)
{
    unsigned char tmp_buf[8];
    int ret = 0;
    int idx = 0;
    struct i2c_client client;
    memcpy(&client, hi_client, sizeof(struct i2c_client));
    client.addr = (dev_addr >> 1);

    /* reg_addr config */
    if (reg_addr_num == 1)
    {
        tmp_buf[idx++] = reg_addr & 0xff;
    }
    else
    {
        tmp_buf[idx++] = (reg_addr >> 8) & 0xff;
        tmp_buf[idx++] = reg_addr & 0xff;
    }

    /* data config */
    if (data_byte_num == 1)
    {
        tmp_buf[idx++] = data;
    }
    else
    {
        tmp_buf[idx++] = (data >> 8) & 0xff;
        tmp_buf[idx++] = data & 0xff;
    }

    while (1)
    {
        ret = i2c_master_send(&client, tmp_buf, idx);

        if (ret == idx)
        {
            break;
        }
        else if ((ret == -EAGAIN) && (in_atomic() || irqs_disabled()))
        {
            continue;
        }
        else
        {
            printk("[%s %d] i2c_master_send error, ret=%d. \n", __func__, __LINE__,
                   ret);
            return ret;
        }
    }

    return 0;
}

int i2cdev_write(char* buf, unsigned int count,
                 unsigned int reg_addr_num, unsigned int data_byte_num)
{
    int ret;
    struct i2c_client* client = hi_client;

    if (reg_addr_num == 2)
    { client->flags |= I2C_M_16BIT_REG; }

    if (data_byte_num == 2)
    { client->flags |= I2C_M_16BIT_DATA; }


    ret = i2c_master_send(client, buf, count);

    return ret;
}

int i2cdev_read(char* buf, unsigned int count,
                unsigned int reg_addr_num, unsigned int data_byte_num)
{
    int ret;

    struct i2c_client* client = hi_client;

    if (reg_addr_num == 2)
    { client->flags |= I2C_M_16BIT_REG; }

    if (data_byte_num == 2)
    { client->flags |= I2C_M_16BIT_DATA; }

    ret = i2c_master_recv(client, buf, count);

    return ret;
}

int i2cdev_init(void)
{
    struct i2c_adapter* i2c_adap;
    i2c_adap = i2c_get_adapter(I2C_NUM);

    if (i2c_adap == NULL)
    {
        dev_err(NULL , "i2c_get_adapter error()!\n");
        return -1;
    }

    hi_client = i2c_new_device(i2c_adap, &hi_info);

    if (hi_client == NULL)
    {
        dev_err(NULL, "i2c_new_device error()!\n");
        return -1;
    }

#ifdef use_dma

    ret = i2cdev_mfd_init();

    if (ret != 0)
    {
        printk("%s, %s, %d line\n", __FILE__, __func__, __LINE__);
        return ret;
    }

#else
    i2c_put_adapter(i2c_adap);
#endif

    return 0;
}

void i2cdev_exit(void)
{
    if (hi_client != NULL)
    {
        i2c_unregister_device(hi_client);
    }

#ifdef use_dma
    i2cdev_mfd_exit();
#endif
}
