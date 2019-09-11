#include "linux/i2c.h"
#include "i2c.h"

#include "pm_device.h"

#include "sys/statfs.h"
#include "fcntl.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "shcmd.h"

//--------------------------------NOTE !!!-------------------------------------------------------------------------------

//----------------This is for hi3556av100 demb pmu regulator, User must modify the Following code if different pmu  used !!!.-------------


#ifndef PM_REGULATOR_TYPE_DCDC

#define PMU_DEV_ADDR 0xb4
#define PMU_CPU_REG_ADDR 0x51
#define PMU_MEDIA_REG_ADDR 0x31
#define I2C_NUM 6

struct  i2c_client  i2c_client_obj;

int pm_pmu_i2c_client_init(void)
{
    int ret = 0;
    struct i2c_client* i2c_client0 = &i2c_client_obj;
    i2c_client0->addr = PMU_DEV_ADDR >> 1;
    ret = client_attach(i2c_client0, I2C_NUM);
    if (ret) {
        dprintf("Fail to attach client!\n");
        return -1;
    }
    PM_DEBUG(" pm_pmu_i2c_client_init ok! \n");
    return 0;
}

int pm_pmu_i2c_client_deinit(void)
{
    int ret = 0;
    struct i2c_client* i2c_client0 = &i2c_client_obj;
    ret = client_deinit(i2c_client0);
    if (ret) {
        dprintf("Fail to deattach client!\n");
        return -1;
    }
    PM_DEBUG(" pm_pmu_i2c_client_deinit ok! \n");
    return 0;
}


int pm_pmu_i2c_write(int reg_addr, unsigned char value)
{
    int ret = 0;
    struct i2c_client* i2c_client0 = & i2c_client_obj;
    unsigned  char buf[4] = {0};
#if 0
    buf[0] = reg_addr & 0xff;
    buf[1] = (reg_addr >> 8) & 0xff;
    buf[2] = value;
    ret = i2c_master_send(i2c_client0, &buf, 3);
#else
    buf[0] = reg_addr & 0xff;
    buf[1] = (value & 0xff);
    ret = i2c_master_send(i2c_client0, buf, 2);
#endif
    PM_DEBUG(" i2c write: i2c_master_send ,value:%d,ret :%d! \n", value, ret);
    return ret;
}

int pm_pmu_i2c_read(int reg_addr, unsigned char* pvalue)
{
    int ret;
    struct i2c_client* i2c_client0 = & i2c_client_obj;
    unsigned char recvbuf[4] = {0};
#if 1
    struct i2c_rdwr_ioctl_data rdwr;
    struct i2c_msg msg[2] = {0};
    memset(recvbuf, 0x0, 4);
    msg[0].addr = PMU_DEV_ADDR >> 1;
    msg[0].flags = 0;
    msg[0].len = 1;   /*reg_width*/
    msg[0].buf = recvbuf;
    msg[1].addr = PMU_DEV_ADDR >> 1;
    msg[1].flags = 0;
    msg[1].flags |= I2C_M_RD;
    msg[1].len = 1;    /*data_width*/
    msg[1].buf = recvbuf;
    rdwr.msgs = &msg[0];
    rdwr.nmsgs = 2;
    recvbuf[0] = (unsigned char )(reg_addr & 0xff);
    i2c_transfer(i2c_client0->adapter, rdwr.msgs, rdwr.nmsgs);
#else
    recvbuf[0] = (unsigned char )(reg_addr & 0xff);
    i2c_master_recv(i2c_client0, recvbuf, 2);
#endif
    PM_DEBUG(" [reg_addr]: 0x%x,  [ value ]= 0x%x\n", reg_addr, recvbuf[0]);
    *pvalue = recvbuf[0];
    return ret;
}


void pm_media_pmu_set_voltage(int step, int regulator_max, int set_volt)
{
    unsigned char value = 0, reg_addr = PMU_MEDIA_REG_ADDR;
    int  set_volt_mv = set_volt / 1000;
    value = (unsigned char)((set_volt_mv * 10 - 6000) / 93);
    PM_DEBUG("set media voltage, value:%d,set_volt:%d! \n", value, set_volt);
    pm_pmu_i2c_write(reg_addr, value);
    return 0;
}

int  pm_media_pmu_get_voltage(int step, int regulator_max)
{
    unsigned char value = 0, reg_addr = PMU_MEDIA_REG_ADDR;
    int  set_volt_uv;
    pm_pmu_i2c_read(reg_addr, &value);
    set_volt_uv = (int)( value * 9300 + 600 * 1000);
    //  PM_DEBUG(" get media voltage reg:0x%x,  value:%d,get_volt_uv:%d!! \n", PMU_MEDIA_REG_ADDR,value,set_volt_uv);
    return set_volt_uv;
}

int  pm_media_pmu_get_regulator_step(int min_uV, int max_uV)
{
    int step, period;
    return 0;
}

void pm_cpu_pmu_set_voltage(int step, int regulator_max, int set_volt)
{
    unsigned char value = 0, reg_addr = PMU_CPU_REG_ADDR;
    int  set_volt_mv = set_volt / 1000;
    value = (unsigned char)((set_volt_mv * 10 - 6000) / 93);
    PM_DEBUG("set cpu voltage, value:%d,set_volt:%d! \n", value, set_volt);
    pm_pmu_i2c_write(reg_addr, value);
    return 0;
}

int  pm_cpu_pmu_get_voltage(int step, int regulator_max)
{
    unsigned char value = 0, reg_addr = PMU_CPU_REG_ADDR;
    int set_volt_uv;
    pm_pmu_i2c_read(reg_addr, &value);
    set_volt_uv = (int)( value * 9300 + 600 * 1000);
//    PM_DEBUG(" get cpu voltage reg:0x%x,  value:%d,   get_volt_uv:%d!! \n", PMU_CPU_REG_ADDR,value,set_volt_uv);
    return set_volt_uv;
}

int  pm_cpu_pmu_get_regulator_step(int min_uV, int max_uV)
{
    int step, period;
    return 0;
}


void pm_core_pmu_set_voltage(int step, int regulator_max, int set_volt)
{
    /*not support by hi3559v200 demb,user can add your pmu code here*/
    return ;
}

int  pm_core_pmu_get_voltage(int step, int regulator_max)
{
    /*not support by hi3559v200 demb,user can add your pmu code here*/
    return ;
}

#endif


