//#include <liteos/i2c.h>
#include "linux/i2c.h"

#include "motionsensor.h"

#include "i2c_dev.h"

struct i2c_client i2c_client_obj; //i2c control struct
#define SLAVE_ADDR 0x34 //i2c dev addr
#define SLAVE_REG_ADDR 0x300f //i2c dev register
/* client initial */

HI_S32 i2c_client_init(struct i2c_client** hi_i2c_client)
{
	int ret = 0;
	//struct i2c_client * i2c_client0 = &i2c_client_obj;
	//i2c_client0->addr = SLAVE_ADDR >> 1;
	hi_i2c_client->addr = ICM20690_DEV_ADDR;
	ret = client_attach(*hi_i2c_client, 0);
	if(ret) {
		dprintf("Fail to attach client!\n");
		return -1;
	}
	return 0;
}

HI_S32 sample_i2c_write(struct i2c_client* hi_i2c_client, HI_U8 reg_addr, HI_U8* reg_data, HI_U32 cnt)
{
	int ret;
	//struct i2c_client * i2c_client0 = & i2c_client_obj;
	char buf[4] = {0};
	//i2c_client_init();
	buf[0] = reg_addr & 0xff;
	osal_memcpy(&buf[1], reg_data, cnt);
	//buf[1] = (SLAVE_REG_ADDR >> 8) & 0xff;
	//buf[2] = 0x03; //write value to i2c
	//call I2C standar function drv to write
	ret = i2c_master_send(hi_i2c_client, &buf, cnt+1);
	return ret;
}

HI_S32 sample_i2c_read(struct i2c_client* hi_i2c_client, HI_U8 reg_addr, HI_U8* reg_data, HI_U32 cnt)
{
	int ret;
	//struct i2c_client *i2c_client0 = & i2c_client_obj;
	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msg[2];
	unsigned char recvbuf[4];
	osal_memcpy(recvbuf, 0x0 ,4);
	//i2c_client_init();
	msg[0].addr  =  hi_i2c_client->addr;
	msg[0].flags = hi_i2c_client->flags & I2C_M_TEN;
	msg[0].len   = 1;
	msg[0].buf   = reg_addr;
	msg[1].addr  =  hi_i2c_client->addr;
	msg[1].flags = hi_i2c_client->flags & I2C_M_TEN;
	msg[1].flags |=  I2C_M_RD;
	msg[1].len   = cnt;
	msg[1].buf   = reg_data;
	rdwr.msgs = &msg[0];
	rdwr.nmsgs = 2;
	recvbuf[0] = SLAVE_REG_ADDR & 0xff;
	recvbuf[1] = (SLAVE_REG_ADDR >> 8) & 0xff;
	i2c_transfer(hi_i2c_client->adapter, msg, rdwr.nmsgs);
	//dprintf("val = 0x%x\n",recvbuf[0]); //buf[0] save the value read from i2c dev
	return ret;
}

HI_U8  MotionSersor_I2C_write(struct i2c_client* hi_i2c_client, 
									HI_U8 u8reg_addr, HI_U8* u8reg_data, HI_U32 u32cnt)
{
    return sample_i2c_write(hi_i2c_client, u8reg_addr, u8reg_data, u32cnt);
}


HI_U8 MotionSersor_I2C_read(struct i2c_client* hi_i2c_client, 
									HI_U8 reg_addr, HI_U8* reg_data, HI_U32 cnt)
{
    return sample_i2c_read(hi_i2c_client, reg_addr, reg_data, cnt);
}




