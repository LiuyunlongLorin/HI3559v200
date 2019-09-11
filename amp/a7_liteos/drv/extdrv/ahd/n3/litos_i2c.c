#include "i2c.h"
#include "sys/statfs.h"
#include "fcntl.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "shcmd.h"

#define DEV_REG_WIDTH   1
#define MAX_SENDDATA_LEN 8
static int g_i2c_fd = -1;

static unsigned char devaddr = 0;

unsigned char HI_LITOS_I2C_Read(int reg, unsigned char * val, unsigned int data_len)
{
    int ret = 0;
    static struct i2c_rdwr_ioctl_data rdwr;
    static struct i2c_msg msg[2];
    unsigned char buf[4];

    memset(buf, 0x0, 4);
    msg[0].addr = devaddr;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = buf;

    msg[1].addr = devaddr;
    msg[1].flags = 0;
    msg[1].flags |= I2C_M_RD;
    msg[1].len = 1;
    msg[1].buf = buf;

    rdwr.msgs = &msg[0];
    rdwr.nmsgs = (unsigned int)2;

    if (DEV_REG_WIDTH == 2)
    {
        buf[0] = (reg >> 8) & 0xff;
        buf[1] = reg & 0xff;
    } else
        buf[0] = reg & 0xff;

    ret = ioctl(g_i2c_fd, I2C_RDWR, &rdwr);
    if (ret != 2) {
        printf("CMD_I2C_READ error!\n");
        ret = -1;
        goto err;
    }
    *val = buf[0];

    return 0;
err:
    return -1;
}


unsigned char HI_LITOS_I2C_Write(int reg, unsigned char * val, unsigned char data_len)
{
    int ret = 0;
    unsigned char sendbuf[MAX_SENDDATA_LEN];

    if (DEV_REG_WIDTH == 2) {
        sendbuf[0] = reg & 0xff;
        sendbuf[1] = (reg >> 8) & 0xff;
        memcpy(&sendbuf[2], val, data_len);
    }
    else
    {
        sendbuf[0] = reg & 0xff;
        memcpy(&sendbuf[1], val, data_len);
    }

    ret = write(g_i2c_fd, sendbuf, DEV_REG_WIDTH + data_len);
    if(ret < 0)
    {
        dprintf("write value error!\n");
        goto err;
    }

    return LOS_OK;
err:
    return -1;

}


int HI_LITOS_I2C_Init(unsigned int i2c_num,unsigned int dev_addr)
{
    int ret = 0;
    char file_name[0x20];

    snprintf(file_name, sizeof(file_name), "/dev/i2c-%u", i2c_num);
    g_i2c_fd = open(file_name, O_RDWR);
    if (g_i2c_fd < 0) {
        dprintf("open %s fail!\n",file_name);
        return -OS_FAIL;
    }
    ret = ioctl(g_i2c_fd, I2C_SLAVE_FORCE ,(dev_addr & 0xff));
    if (ret) {
        dprintf("set i2c_slave fail!\n");
        goto closefile;
    }
    devaddr = dev_addr;
    return LOS_OK;

closefile:
    close(g_i2c_fd);
    g_i2c_fd = -1;
    return -1;
}

int HI_LITOS_I2C_Exit()
{
    close(g_i2c_fd);
    g_i2c_fd = -1;

    return LOS_OK;
}
