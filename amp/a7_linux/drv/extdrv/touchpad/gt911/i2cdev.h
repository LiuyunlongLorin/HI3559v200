#ifndef __HI_I2C_H__
#define __HI_I2C_H__


int i2cdev_write(char *buf, unsigned int count,
        unsigned int reg_addr_num, unsigned int data_byte_num);

int i2cdev_read(char *buf, unsigned int count,
        unsigned int reg_addr_num, unsigned int data_byte_num);

int i2cdev_dma_read(int reg,int len, uint8_t *val);

int i2cdev_dma_write(int reg,int len, uint8_t *val);



int hi_i2c_read(unsigned int reg_addr, unsigned int reg_addr_num, unsigned char *reg_data, unsigned int data_num);

int hi_i2c_write(unsigned int reg_addr,unsigned int reg_addr_num, unsigned int data, unsigned int data_byte_num);


int i2cdev_init(void);

void i2cdev_exit(void);
#endif
