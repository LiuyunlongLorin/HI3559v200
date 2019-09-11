#ifndef __HI_I2C_H__
#define __HI_I2C_H__


int i2cdev_write(char *buf, unsigned int count,
        unsigned int reg_addr_num, unsigned int data_byte_num);

int i2cdev_read(char *buf, unsigned int count,
        unsigned int reg_addr_num, unsigned int data_byte_num);

int i2cdev_dma_read(int reg,int len, uint8_t *val);

int i2cdev_dma_write(int reg,int len, uint8_t *val);


unsigned char hi_i2c_read(unsigned char reg_addr, unsigned char *reg_data, unsigned int cnt);


int hi_i2c_write(unsigned char dev_addr, unsigned int reg_addr,
    unsigned int reg_addr_num, unsigned int data, unsigned int data_byte_num);

int i2cdev_init(void);

void i2cdev_exit(void);
#endif
