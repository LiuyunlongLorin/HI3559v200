
/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : sensor_i2c.c

  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/11/07
  Description   :
  History       :
  1.Date        : 2013/11/07
    Author      :
    Modification: Created file

******************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/i2c.h>

#include <linux/delay.h>
#include "isp_ext.h"

#define I2C_MAX_NUM 3
#define ISP_MAX_DEV 4
#ifndef __HuaweiLite__
static struct i2c_board_info hi_info =
{
    I2C_BOARD_INFO("sensor_i2c", (0x6c >> 1)),
};
#endif

static struct i2c_client *sensor_client[I2C_MAX_NUM];

static int hi_sensor_i2c_write(unsigned char i2c_dev, unsigned char dev_addr,
                               unsigned int reg_addr, unsigned int reg_addr_num,
                               unsigned int data, unsigned int data_byte_num)
{
    unsigned char tmp_buf[8];
    int ret = 0;
    int idx = 0;
    struct i2c_client client;
    HI_U32 u32Tries = 0;

    if (I2C_MAX_NUM <= i2c_dev)
    {
        return HI_FAILURE;
    }

    if (NULL == sensor_client[i2c_dev])
    {
        return HI_FAILURE;
    }

    osal_memcpy(&client, sensor_client[i2c_dev], sizeof(struct i2c_client));

#ifdef __HuaweiLite__
    client.addr = ((dev_addr >> 1) & 0xff);

    /* reg_addr config */
    if (reg_addr_num == 1)
    {
        client.flags &= ~I2C_M_16BIT_REG;
        tmp_buf[idx++] = reg_addr & 0xff;
    }
    else
    {
        client.flags |= I2C_M_16BIT_REG;
        tmp_buf[idx++] = (reg_addr >> 8) & 0xff;
        tmp_buf[idx++] = reg_addr & 0xff;
    }

    /* data config */
    if (data_byte_num == 1)
    {
        client.flags &= ~I2C_M_16BIT_DATA;
        tmp_buf[idx++] = data & 0xff;
    }
    else
    {
        client.flags |= I2C_M_16BIT_DATA;
        tmp_buf[idx++] = (data >> 8) & 0xff;
        tmp_buf[idx++] = data & 0xff;
    }
#else
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
        tmp_buf[idx++] = data & 0xff;
    }
    else
    {
        tmp_buf[idx++] = (data >> 8) & 0xff;
        tmp_buf[idx++] = data & 0xff;
    }
#endif

    while (1)
    {
        ret = hi_i2c_master_send(&client, tmp_buf, idx);
        if (ret == idx)
        {
            break;
        }
#ifdef __HuaweiLite__
        else if ((ret == -EAGAIN))
#else
        else if ((ret == -EAGAIN) && (in_atomic() || irqs_disabled()))
#endif
        {
            u32Tries++;
            if (u32Tries > 5)
            {
                return HI_FAILURE;
            }
        }
        else
        {
            osal_printk("[%s %d] hi_i2c_master_send error, ret=%d. \n", __func__, __LINE__, ret);
            return ret;
        }
    }

    return 0;
}

static int hi_dev_isp_register(void)
{
    int i = 0;
    ISP_BUS_CALLBACK_S stBusCb = {0};

    stBusCb.pfnISPWriteI2CData = hi_sensor_i2c_write;
    if ((NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)) && (CKFN_ISP_RegisterBusCallBack()))
    {
        for (i = 0; i < ISP_MAX_DEV; i++)
        {
            CALL_ISP_RegisterBusCallBack(i, ISP_BUS_TYPE_I2C, &stBusCb);
        }
    }
    else
    {
        osal_printk("register i2c_write_callback to isp failed, hi_i2c init is failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


#ifdef __HuaweiLite__
static struct i2c_client sensor_client_obj[I2C_MAX_NUM];
static struct i2c_client *hi_sensor_i2c_client_init(int i2c_dev )
{
    int ret = 0;
    struct i2c_client *i2c_client0 = &sensor_client_obj[i2c_dev];
    i2c_client0->addr = 0x6c >> 1;
    ret = client_attach(i2c_client0, i2c_dev);
    if (ret)
    {
        dprintf("Fail to attach client!\n");
        return NULL;
    }
    return &sensor_client_obj[i2c_dev];
}

int hi_dev_init(void)
{
    int i  = 0;
    //struct i2c_adapter *i2c_adap;

    for (i = 0; i < I2C_MAX_NUM; i++)
    {
        sensor_client[i] = hi_sensor_i2c_client_init(i);
    }

    hi_dev_isp_register();

    return 0;
}

void hi_dev_exit(void)
{
}

#else
static int __init hi_dev_init(void)
{
    int i  = 0;
    struct i2c_adapter *i2c_adap;

    for (i = 0; i < I2C_MAX_NUM; i++)
    {
        i2c_adap = i2c_get_adapter(i);
        if (NULL == i2c_adap)
        {
            osal_printk("i2c_get_adapter error!\n");
            return -1;
        }

        sensor_client[i] = i2c_new_device(i2c_adap, &hi_info);

        i2c_put_adapter(i2c_adap);
    }

    hi_dev_isp_register();

    return 0;
}
static void __exit hi_dev_exit(void)
{
    int i = 0;

    for (i = 0; i < I2C_MAX_NUM; i++)
    {
        i2c_unregister_device(sensor_client[i]);
    }
}
module_init(hi_dev_init);
module_exit(hi_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hisilicon");

#endif
