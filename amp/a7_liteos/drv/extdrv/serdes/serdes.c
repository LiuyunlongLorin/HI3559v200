#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include "isp_ext.h"
#include "hi_serdes.h"

#define SERDES_DEBUG    0

/*for 241 bank setting*/
typedef enum hiSERDES_BANK_TYPE_E
{
    BANK_TYPE_00 = 0,
    BANK_TYPE_10,
    BANK_TYPE_11,
    BANK_TYPE_BUTT,
} SERDES_BANK_TYPE_E;

static struct i2c_client *client;

static struct i2c_board_info board_info =
{
    I2C_BOARD_INFO(SERDES_DEVICE_TYPE, (SERDES_DEVICE_ADDR >> 1)),
};

static SERDES_BANK_TYPE_E enBankType;

SERDES_CONFIG_S IMX307_4Lane_12BIT_30FPS_CFG =
{
    .txPllValue = { 0x15, 0x00, 0x00, 0x00, 0x74, 0x01 },
    .txDPhyValue = 0x07,
};

SERDES_CONFIG_S IMX307_4Lane_12BIT_30FPS_2To1WDR_CFG =
{
    .txPllValue = { 0x1c, 0x00, 0x00, 0x00, 0x72, 0x01 },
    .txDPhyValue = 0x09,
};

static inline void delay_us(const int us)
{
    udelay(us);
    return;
}

static int serdes_i2c_write(const unsigned char dev_addr, const unsigned int reg_addr, const unsigned int data)
{
    int i = 0;
    int ret = 0;
    unsigned char w_buf[4];

#ifdef __HuaweiLite__
    client->addr = ((dev_addr >> 1) & 0xff);
    client->flags |= I2C_M_16BIT_REG;
    w_buf[i++] = (reg_addr >> 8) & 0xff;
    w_buf[i++] = reg_addr & 0xff;

    client->flags &= ~I2C_M_16BIT_DATA;
    w_buf[i++] = data & 0xff;
#else
    client->addr = dev_addr >> 1;

    w_buf[i++] = reg_addr >> 8 & 0xFF;
    w_buf[i++] = reg_addr & 0xFF;
    w_buf[i++] = data & 0xFF;
#endif

    ret = hi_i2c_master_send(client, w_buf, i);
    if (ret != i)
    {
        printk("[%s %d] hi_i2c_master_send error, ret=%d.\n", __FUNCTION__, __LINE__, ret);
        return -1;
    }

    return 0;
}

static int serdes_i2c_read(const unsigned char dev_addr, const unsigned int reg_addr, unsigned int *data)
{
    int ret = 0;
    static struct i2c_msg msg[2];
    unsigned char buffer[2];

    buffer[0] = reg_addr >> 8 & 0xFF;
    buffer[1] = reg_addr & 0xFF;

    msg[0].addr = client->addr;
    msg[0].flags = 0;
    msg[0].len = 2;
    msg[0].buf = buffer;

    msg[1].addr = client->addr;
    msg[1].flags = client->flags | I2C_M_RD;
    msg[1].len = 1;
    msg[1].buf = buffer;

    ret = hi_i2c_transfer(client->adapter, msg, 2);
    if (ret != 2)
    {
        printk("[%s %d] hi_i2c_transfer error, ret=%d.\n", __FUNCTION__, __LINE__, ret);
        return -1;
    }

    *data = buffer[0];

    return 0;
}

void write_serdes_rx_register(const unsigned int reg_addr, const unsigned int data)
{
    serdes_i2c_write(SERDES_DEVICE_ADDR, reg_addr, data);
    return;
}

int read_serdes_rx_register(const unsigned int reg_addr)
{
    int r_data = 0;

    serdes_i2c_read(SERDES_DEVICE_ADDR, reg_addr, &r_data);
    return r_data;
}

/*for 241, use bank setting*/
void write_serdes_tx_register(const unsigned int reg_addr, const unsigned int data)
{
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, (reg_addr & 0x00FF));
    write_serdes_rx_register(0x00D1, data);
    write_serdes_rx_register(0x00E0, SERDES_DEVICE_ADDR); // 8bit
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    return;
}

void serdes_write_sensor_register(const unsigned char dev_addr, const unsigned int reg_addr, const unsigned int data)
{
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, (reg_addr & 0xFF00 >> 8));
    write_serdes_rx_register(0x00D1, (reg_addr & 0x00FF));
    write_serdes_rx_register(0x00D2, data);
    write_serdes_rx_register(0x00E0, dev_addr); // 8bit
    write_serdes_rx_register(0x00E1, 0x10);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    return;
}
EXPORT_SYMBOL(serdes_write_sensor_register);

void serdes_read_sensor_register(const unsigned char dev_addr, const unsigned int reg_addr, unsigned int *data)
{
    int i = 0;

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, reg_addr & 0xFF00 >> 8);
    write_serdes_rx_register(0x00D1, reg_addr & 0x00FF);
    write_serdes_rx_register(0x00E0, dev_addr | 0x1); // 8bit
    write_serdes_rx_register(0x00E1, 0x10);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    for (i = 0; i < 10; ++i)
    {
        delay_us(I2C_TM_SCL_TMO);
    }

    *data = read_serdes_rx_register(0x00D0);

    return;
}
EXPORT_SYMBOL(serdes_read_sensor_register);

/*set the bank type of 241*/
static void serdes_set_bank_type(const SERDES_BANK_TYPE_E enType)
{
    unsigned char val = 0x0;

    if (enType == enBankType)
    {
        return;
    }

    val = (enType == BANK_TYPE_10) ? 0x21 : (enType == BANK_TYPE_00 ? 0x11 : 0x31);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xfe);
    write_serdes_rx_register(0x00D1, 0x00);
    write_serdes_rx_register(0x00E0, 0x16);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x00);
    write_serdes_rx_register(0x00D1, 0xfe);
    write_serdes_rx_register(0x00D2, val); // 0x21: "10", 0x11: "00", 0x31: "11"
    write_serdes_rx_register(0x00E0, 0x16);
    write_serdes_rx_register(0x00E1, 0x10);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);
    delay_us(I2C_TM_SCL_TMO);

    enBankType = enType;

    return;
}

void serdes_reset_unreset_sensor(int reset)
{
    serdes_set_bank_type(BANK_TYPE_10);

    if (reset)
    {
        write_serdes_tx_register(0x103e, 0x00);
    }
    else
    {
        write_serdes_tx_register(0x103e, 0x30);
    }
}
EXPORT_SYMBOL(serdes_reset_unreset_sensor);

void serdes_enable_disable_sensor_clock(int enable)
{
    const unsigned int w_val = enable ? 0x10 : 0x50;

    serdes_set_bank_type(BANK_TYPE_10);

    write_serdes_tx_register(0x1076, w_val);

    return;
}
EXPORT_SYMBOL(serdes_enable_disable_sensor_clock);

static int hi_sensor_i2c_write(unsigned char i2c_dev, unsigned char dev_addr, unsigned int reg_addr,
                               unsigned int reg_addr_num, unsigned int data, unsigned int data_num)
{
    if (reg_addr_num != SENSOR_ADDR_BYTE || data_num != SENSOR_DATA_BYTE)
    {
        return 0;
    }

    // printk("hi_sensor_i2c_write called......\n");

    serdes_write_sensor_register(dev_addr, reg_addr, data);

    return 0;
}

void serdes_241_242_init(const SERDES_CONFIG_S *cfg)
{
    int result = 0;

    /*---------------------------------------------------------
    Step1: THCV242 sub-link mode : 2wire trigger mode setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x0004, 0x01);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x0004);
    printk("read 242 0x0004: 0x%x, should be 0x1\n", result);
    #endif

    /*---------------------------------------------------------
    Step2: THCV242 sub link en/pol : lane 0 enable polling off
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x0010, 0x10); // Polling Off

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x0010);
    printk("read 242 0x0010: 0x%x, should be 0x10\n", result);
    #endif

    /*---------------------------------------------------------
    Step3: THCV242 sub link power on
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1704, 0x01);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1704);
    printk("read 242 0x1704: 0x%x, should be 0x1\n", result);
    #endif

    /*---------------------------------------------------------
    Step4: THCV242 sub link tx term 50ohm
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x0102, 0x02); // 50ohm

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x0102);
    printk("read 242 0x0102: 0x%x, should be 0x2\n", result);
    #endif

    /*---------------------------------------------------------
    Step5: THCV242 sub link tx drv 12ma
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x0103, 0x02); // 12mA

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x0103);
    printk("read 242 0x0103: 0x%x, should be 0x2\n", result);
    #endif

    /*---------------------------------------------------------
    Step6: THCV242 sub link rx term : 50ohm
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x0104, 0x02); // 50ohm

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x0104);
    printk("read 242 0x0104: 0x%x, should be 0x2\n", result);
    #endif

    /*---------------------------------------------------------
    Step7: THCV242 rx drv : 12ma
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x0105, 0x02); // 12mA

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x0105);
    printk("read 242 0x0105: 0x%x, should be 0x2\n", result);
    #endif

    /*---------------------------------------------------------
    Step8: THCV242 reserved setting : key is unlocked
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x0100, 0x03);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x0100);
    printk("read 242 0x0100: 0x%x, should be 0x3\n", result);
    #endif

    /*---------------------------------------------------------
    Step9: THCV242 reserved setting : key2 is unlocked
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x010f, 0x25);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x010f);
    printk("read 242 0x010f: 0x%x, should be 0x25\n", result);
    #endif

    /*---------------------------------------------------------
    Step10: THCV242 reserved setting : sub link width
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x010a, 0x18); // 0x15

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x010a);
    printk("read 242 0x010a: 0x%x, should be 0x18\n", result);
    #endif

    /*---------------------------------------------------------
    Step11: THCV242 sub link transaction write lane select
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x00e4, 0x01); // for lane 0

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x00e4);
    printk("read 242 0x00e4: 0x%x, should be 0x1\n", result);
    #endif

    delay_us(I2C_TM_SCL_TMO);

    /*---------------------------------------------------------
    Sep12: THCV241 2byte mode setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xfe);
    write_serdes_rx_register(0x00D1, 0x00);
    write_serdes_rx_register(0x00E0, 0x16);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);

    /*---------------------------------------------------------
    THCV241 Word Address Write Access from Bank Setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x00);
    write_serdes_rx_register(0x00D1, 0xfe);
    write_serdes_rx_register(0x00D2, 0x11); // bank 0x00 setting
    write_serdes_rx_register(0x00E0, 0x16);
    write_serdes_rx_register(0x00E1, 0x10);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);

    enBankType = BANK_TYPE_00;

    /*---------------------------------------------------------
    Step13: THCV241 sub link rx term/drv : 50ohm 12ma
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x00f3, 0x22); // 50ohm/12mA

    /*---------------------------------------------------------
    Step14: THCV241 sub link tx term / drv setting:50ohm 12ma
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x00f2, 0x22); // 50ohm/12mA

    /*---------------------------------------------------------
    Step15: THCV241 reserved setting : key is unlocked
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x00f0, 0x03);

    /*---------------------------------------------------------
    THCV241 reserved setting : key 2 is unlocked
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x00ff, 0x19);

    /*---------------------------------------------------------
    Step16: THCV241 reserved setting : sub link width
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x00f6, 0x18); //0x15

    /*---------------------------------------------------------
    Step17: THCV242 sub link en/pol : lane 0 enable polling on
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x0010, 0x11);

    #if 0
    /*---------------------------------------------------------
    THCV241 test mode :for ripple check
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    // write_serdes_tx_register(0x00fd, 0x71);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xfd);
    write_serdes_rx_register(0x00D1, 0x71);
    write_serdes_rx_register(0x00E0, 0x16);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);
    #endif

    #if SERDES_DEBUG
    /*---------------------------------------------------------
    THCV241 read
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xf2);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x00f2: 0x%x, should be 0x22\n", result);

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xf3);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x00f3: 0x%x, should be 0x22\n", result);

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xf6);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x00f6: 0x%x, should be 0x18\n", result);

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xf0);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x00f0: 0x%x, should be 0x3\n", result);

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xff);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x00ff: 0x%x, should be 0x19\n", result);

    #if 0
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xfd);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x00fd: 0x%x, should be 0x71\n", result);
    #endif
    #endif

    /*---------------------------------------------------------
    THCV241 2byte mode setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xfe);
    write_serdes_rx_register(0x00D1, 0x00);
    write_serdes_rx_register(0x00E0, 0x16);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);

    /*---------------------------------------------------------
    THCV241 Word Address Write Access from Bank Setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x00);
    write_serdes_rx_register(0x00D1, 0xfe);
    write_serdes_rx_register(0x00D2, 0x21); // bank 0x10 setting
    write_serdes_rx_register(0x00E0, 0x16);
    write_serdes_rx_register(0x00E1, 0x10);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);

    enBankType = BANK_TYPE_10;

    /*---------------------------------------------------------
    THCV241 csi-2 clock lane enable
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x102c, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x2c);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x102c: 0x%x, should be 0x1\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 csi-2 data lane enable : 4lane
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x102d, 0x13);

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x2d);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x102d: 0x%x, should be 0x13\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 vx1hs 1lane
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x1000, 0x00);

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x00);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1000: 0x%x, should be 0x0\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 CKO setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x1076, 0x10);

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x76);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1076: 0x%x, should be 0x10\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 pll manual setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x100f, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x0f);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x100f: 0x%x, should be 0x1\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 pll setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x11);
    write_serdes_rx_register(0x00D1, cfg->txPllValue[0]);
    write_serdes_rx_register(0x00D2, cfg->txPllValue[1]);
    write_serdes_rx_register(0x00D3, cfg->txPllValue[2]);
    write_serdes_rx_register(0x00D4, cfg->txPllValue[3]);
    write_serdes_rx_register(0x00D5, cfg->txPllValue[4]);
    write_serdes_rx_register(0x00D6, cfg->txPllValue[5]);
    write_serdes_rx_register(0x00E0, 0x16);
    write_serdes_rx_register(0x00E1, 0x05);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x11);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1011: 0x%x, should be 0x15\n", result);

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x12);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1012: 0x%x, should be 0x0\n", result);

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x13);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1013: 0x%x, should be 0x0\n", result);

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x14);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1014: 0x%x, should be 0x0\n", result);

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x15);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1015: 0x%x, should be 0x74\n", result);

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x16);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1016: 0x%x, should be 0x1\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 mipi d-phy setting : hs settle
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x102b, cfg->txDPhyValue);

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x2b);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x102b: 0x%x, should be 0x7\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 mipi d-phy setting : hsen delay
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x102f, 0x00); // 0x00

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x2f);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x102f: 0x%x, should be 0x0\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 pll soft reset release
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x1005, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x05);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1005: 0x%x, should be 0x1\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 vx1hs tx soft reset release
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x1006, 0x01);

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x06);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1006: 0x%x, should be 0x1\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 ignore non-image csi-2 data
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x1027, 0x00);

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x27);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1027: 0x%x, should be 0x0\n", result);
    #endif

    /*---------------------------------------------------------
    THCV241 gpio control : gpio 0_ reset/gpio 1_power down
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x103e, 0x00); // low
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x103e, 0x30); // high

    delay_us(I2C_TM_SCL_TMO);

    #if SERDES_DEBUG
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x3e);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x103e: 0x%x, should be 0x30\n", result);
    #endif

    #if 0
    /*---------------------------------------------------------
    THCV241 gpio control : for test
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x1045, 0x24);
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_tx_register(0x103f, 0x08); // high

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x45);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x1045: 0x%x, should be 0x24\n", result);

    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x3f);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x103f: 0x%x, should be 0x8\n", result);


    delay_us(I2C_TM_SCL_TMO);
    #endif

    #if SERDES_DEBUG
   /*---------------------------------------------------------
    THCV241 read 0x102c
    ---------------------------------------------------------*/
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x2c);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x102c: 0x%x, should be 0x1\n", result);

    delay_us(I2C_TM_SCL_TMO);

   /*---------------------------------------------------------
    THCV241 read 0x102d
    ---------------------------------------------------------*/
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x2d);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x102d: 0x%x, should be 0x13\n", result);

    delay_us(I2C_TM_SCL_TMO);

   /*---------------------------------------------------------
    THCV241 read 0x102f
    ---------------------------------------------------------*/
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x2f);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x102f: 0x%x, should be 0x0\n", result);

    delay_us(I2C_TM_SCL_TMO);

   /*---------------------------------------------------------
    THCV241 read 0x103d
    ---------------------------------------------------------*/
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x3d);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x103d: 0x%x, should be 0x0\n", result);

    delay_us(I2C_TM_SCL_TMO);

   /*---------------------------------------------------------
    THCV241 read 0x103e
    ---------------------------------------------------------*/
    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x3e);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x103e: 0x%x, should be 0x30\n", result);

    delay_us(I2C_TM_SCL_TMO);
    #endif

    /*---------------------------------------------------------
    THCV241 2byte mode setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xfe);
    write_serdes_rx_register(0x00D1, 0x00);
    write_serdes_rx_register(0x00E0, 0x16);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);

    /*---------------------------------------------------------
    THCV241 Word Address Write Access from Bank Setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0x00);
    write_serdes_rx_register(0x00D1, 0xfe);
    write_serdes_rx_register(0x00D2, 0x11); // bank 0x00 setting
    write_serdes_rx_register(0x00E0, 0x16);
    write_serdes_rx_register(0x00E1, 0x10);
    write_serdes_rx_register(0x00E4, 0x01);
    write_serdes_rx_register(0x00E5, 0x01);

    enBankType = BANK_TYPE_00;

    #if SERDES_DEBUG
    /*---------------------------------------------------------
    THCV241 read
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1725, 0x10);
    write_serdes_rx_register(0x00D0, 0xfe);
    write_serdes_rx_register(0x00E0, 0x17);
    write_serdes_rx_register(0x00E1, 0x00);
    write_serdes_rx_register(0x00E3, 0x00);
    write_serdes_rx_register(0x00E5, 0x01);

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    result = read_serdes_rx_register(0x00D0);
    printk("read 241 0x00fe: 0x%x, should be 0x11\n", result);
    #endif

    /*---------------------------------------------------------
    THCV242 vx1hs std. 4byte mode setting
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1010, 0xa1);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1010);
    printk("read 242 0x1010: 0x%x, should be 0xa1\n", result);
    #endif

    /*---------------------------------------------------------
    THCV242 pll setting : fb div *32
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1021, 0x20); // 720Mbps, 384Mbps, 360Mbps, 300Mbps

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1021);
    printk("read 242 0x1021: 0x%x, should be 0x20\n", result);
    #endif

    /*---------------------------------------------------------
    THCV242 pll setting : ref div 1/1
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1022, 0x01);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1022);
    printk("read 242 0x1022: 0x%x, should be 0x1\n", result);
    #endif

    /*---------------------------------------------------------
    THCV242 pll setting : ref div 1/2 , 1/2
    ---------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1023, 0x22);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1023);
    printk("read 242 0x1023: 0x%x, should be 0x22\n", result);
    #endif

    /*--------------------------------------------------------------
    THCV242 pll bypass : no bypass, post div/vco working, frac. pll
    ---------------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1027, 0x07);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1027);
    printk("read 242 0x1027: 0x%x, should be 0x7\n", result);
    #endif

    /*--------------------------------------------------------------
    THCV242 vx1hs master clock select lane0
    ---------------------------------------------------------------*/
    // delay_us(I2C_TM_SCL_TMO);
    // write_serdes_rx_register(0x1030, 0x00);
    // result = read_serdes_rx_register(0x1030);
    // printk("read 242 0x1030: 0x%x, should be 0x0\n", result);

    /*--------------------------------------------------------------
    THCV242 csi-2 lane0 : ph used for 241
    ---------------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1100, 0x01);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1100);
    printk("read 242 0x1100: 0x%x, should be 0x1\n", result);
    #endif

    /*--------------------------------------------------------------
    THCV242 csi-2 lane0 : crc used for 241
    ---------------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1101, 0x01);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1101);
    printk("read 242 0x1101: 0x%x, should be 0x1\n", result);
    #endif

    /*--------------------------------------------------------------
    THCV242 csi-2 lane0 : sp used for 241
    ---------------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1102, 0x01);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1102);
    printk("read 242 0x1102: 0x%x, should be 0x1\n", result);
    #endif

    /*--------------------------------------------------------------
    THCV242 csi-2 power on, soft reset release
    ---------------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1600, 0x1a);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1600);
    printk("read 242 0x1600: 0x%x, should be 0x1a\n", result);
    #endif

    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    /*--------------------------------------------------------------
    THCV242 csi-2 power on, soft reset release
    ---------------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1605, 0x2b);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1605);
    printk("read 242 0x1605: 0x%x, should be 0x2b\n", result);
    #endif

    /*--------------------------------------------------------------
    THCV242 csi-2 clock lane0 : hs clock on in hblank, off in vblank
    ---------------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    write_serdes_rx_register(0x1606, 0x4F);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1606);
    printk("read 242 0x1606: 0x%x, should be 0x4F\n", result);
    #endif

    /*--------------------------------------------------------------
    THCV242 main link lane 0 power on, sub link power on
    ---------------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1704, 0x11);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1704);
    printk("read 242 0x1704: 0x%x, should be 0x11\n", result);
    #endif

    /*--------------------------------------------------------------
    pll power on
    ---------------------------------------------------------------*/
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);
    delay_us(I2C_TM_SCL_TMO);

    write_serdes_rx_register(0x1703, 0x01);

    #if SERDES_DEBUG
    result = read_serdes_rx_register(0x1703);
    printk("read 242 0x1703: 0x%x, should be 0x1\n", result);

    result = read_serdes_rx_register(0x1012);
    printk("read 242 0x1012: 0x%x, should be 0x0\n", result);

    result = read_serdes_rx_register(0x170f);
    printk("read 242 0x170f: 0x%x, should be 0x3\n", result);
    #endif

    return;
}

static int hi_dev_isp_register(void)
{
    int i = 0;
    ISP_BUS_CALLBACK_S stBusCb = {0};

    stBusCb.pfnISPWriteI2CData = hi_sensor_i2c_write;
    if ((NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)) && (CKFN_ISP_RegisterBusCallBack()))
    {
        for (i = 0; i < ISP_MAX_DEV; ++i)
        {
            // printk("serdes CALL_ISP_RegisterBusCallBack.\n");
            CALL_ISP_RegisterBusCallBack(i, ISP_BUS_TYPE_I2C, &stBusCb);
        }
    }
    else
    {
        printk("register i2c_write_callback to isp failed, serdes init is failed!\n");
        return -1;
    }

    return 0;
}


#ifdef __HuaweiLite__
#define SERDES_I2C_MAX_NUM 9
static struct i2c_client serdes_client_obj[SERDES_I2C_MAX_NUM];
static struct i2c_client *hi_serdes_i2c_client_init(int i2c_dev)
{
    int ret = 0;
    struct i2c_client *i2c_client0 = &serdes_client_obj[i2c_dev];
    i2c_client0->addr = SERDES_DEVICE_ADDR >> 1;
    i2c_client0->flags = 0;
    ret = client_attach(i2c_client0, i2c_dev);
    if (ret)
    {
        dprintf("Fail to attach client!\n");
        return NULL;
    }
    return &serdes_client_obj[i2c_dev];
}

int serdes_mod_init(void)
{
    int i  = 0;

    client = hi_serdes_i2c_client_init(0);
    serdes_241_242_init(&IMX307_4Lane_12BIT_30FPS_CFG);
    hi_dev_isp_register();

    return 0;
}

void serdes_mod_exit(void)
{
    return;
}

#else

int serdes_mod_init(void)
{
    struct i2c_adapter *adapter = i2c_get_adapter(0);

    client = i2c_new_device(adapter, &board_info);

    i2c_put_adapter(adapter);

    serdes_241_242_init(&IMX307_4Lane_12BIT_30FPS_CFG);

    hi_dev_isp_register();

    return 0;
}

void serdes_mod_exit(void)
{
    i2c_unregister_device(client);

    return;
}
#endif
module_init(serdes_mod_init);
module_exit(serdes_mod_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hisilicon");
