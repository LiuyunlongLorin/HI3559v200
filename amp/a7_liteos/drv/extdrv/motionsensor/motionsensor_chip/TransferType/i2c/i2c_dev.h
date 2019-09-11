#ifndef __I2C_DEV_H__
#define __I2C_DEV_H__
#include <linux/i2c.h>
#include "hi_type.h"

#if 0

#define ICM20690_I2C_ADDRESS (0x68)
#define ICM20690_I2C_ADAPT	 (0x9)
extern struct i2c_client *hi_ICM20690_client;

extern unsigned char ICM20690_I2C_write(unsigned char reg_addr, unsigned char *reg_data, unsigned int cnt);
extern unsigned char ICM20690_I2C_read(unsigned char reg_addr, unsigned char *reg_data, unsigned int cnt);
extern int i2cdev_ICM20690_init(void);
extern void i2cdev_ICM20690_exit(void);



#define BMM050_I2C_ADDRESS                 (0x10)
#define I2C_BMM150_ADAPTER 					(0x8)
extern struct i2c_client *hi_BMM150_client;

extern int i2cdev_BMM150_init(void);
extern void i2cdev_BMM150_exit(void);

extern unsigned char  BMM150_I2C_write(unsigned char reg_addr, unsigned char *reg_data, unsigned char cnt);
extern unsigned char BMM150_I2C_read(unsigned char reg_addr, unsigned char *reg_data, unsigned char cnt);
#endif


HI_U8  MotionSersor_I2C_write(struct i2c_client* hi_i2c_client, 
									HI_U8 u8reg_addr, HI_U8* u8reg_data, HI_U32 u32cnt);
HI_U8 MotionSersor_I2C_read(struct i2c_client* hi_i2c_client, 
									HI_U8 reg_addr, HI_U8* reg_data, HI_U32 cnt);

HI_U32	MotionSersor_I2C_GetFrequency(struct i2c_client* hi_i2c_client, HI_S32* s32frequency);

HI_U32 MotionSersor_i2c_init(struct i2c_client** hi_i2c_client, 
									struct i2c_board_info hi_i2c_board_info, HI_S32 s32adapt_num);

void MotionSersor_i2c_exit(struct i2c_client** hi_i2c_client);

#endif

