#ifndef __HI_LITOS_I2C_H__
#define __HI_LITOS_I2C_H__

int HI_LITOS_I2C_Init(unsigned int i2c_num,unsigned int dev_addr);

int HI_LITOS_I2C_Exit(void);

int HI_LITOS_I2C_Read(int reg, unsigned char *val, int data_len);

int HI_LITOS_I2C_Write(int reg, unsigned char *val, int data_len);

#endif
