#ifndef __HI_I2C_H__
#define __HI_I2C_H__

#if 0
int i2cdev_write(char *buf, unsigned int count,
        unsigned int reg_addr_num, unsigned int data_byte_num);

int i2cdev_read(char *buf, unsigned int count,
        unsigned int reg_addr_num, unsigned int data_byte_num);
#endif

unsigned char hi_i2c_read(unsigned char reg_addr, unsigned char *reg_data);

int hi_i2c_write(unsigned int reg_addr,unsigned int reg_addr_num, unsigned int data, unsigned int data_byte_num);

int hi_i2cdev_init(int I2C_NUM);

void hi_i2cdev_exit(void);
#endif
