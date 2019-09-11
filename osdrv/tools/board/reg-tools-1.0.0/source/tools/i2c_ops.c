#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>

#include <linux/fb.h>
#include <sys/mman.h>
#include <memory.h>

#include "i2c-dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define READ_MIN_CNT 4
#define WRITE_MIN_CNT 5

static int atoul(char *str, int * pulValue)
{
	int ulResult=0;

	while (*str)
	{
		if (isdigit((int)*str))
		{
			if ((ulResult<429496729) || ((ulResult==429496729) && (*str<'6')))
			{
				ulResult = ulResult*10 + (*str)-48;
			}
			else
			{
				*pulValue = ulResult;
				return -1;
			}
		}
		else
		{
			*pulValue=ulResult;
			return -1;
		}
		str++;
	}
	*pulValue=ulResult;
	return 0;
}

#define ASC2NUM(ch) (ch - '0')
#define HEXASC2NUM(ch) (ch - 'A' + 10)

static int  atoulx(char *str, int * pulValue)
{
	int   ulResult=0;
	char ch;

	while (*str)
	{
		ch=toupper(*str);
		if (isdigit(ch) || ((ch >= 'A') && (ch <= 'F' )))
		{
			if (ulResult < 0x10000000)
			{
				ulResult = (ulResult << 4) + ((ch<='9')?(ASC2NUM(ch)):(HEXASC2NUM(ch)));
			}
			else
			{
				*pulValue=ulResult;
				return -1;
			}
		}
		else
		{
			*pulValue=ulResult;
			return -1;
		}
		str++;
	}

	*pulValue=ulResult;
	return 0;
}

static int StrToNumber(char *str , int *pulValue)
{
	if ( *str == '0' && (*(str+1) == 'x' || *(str+1) == 'X') )
	{
		if (*(str+2) == '\0')
		{
			return -1;
		}
		else
		{
			return atoulx(str+2,pulValue);
		}
	}
	else
	{
		return atoul(str,pulValue);
	}
}

static void print_r_usage(void)
{
	printf("usage: i2c_read <i2c_num> <dev_addr> <reg_addr> <reg_addr_end>"
			"<reg_width> <data_width> <reg_step>. sample: \n");
	printf("\t\t0x1 0x56 0x0 0x10 2 2. \n");
	printf("\t\t0x1 0x56 0x0 0x10 2 2 2. \n");
	printf("\t\t0x1 0x56 0x0 0x10. default reg_width, data_width, reg_step is 1. \n");
}

static void print_w_usage(void)
{
	printf("usage: i2c_write <i2c_num> <dev_addr> <reg_addr> <value> <reg_width> <data_width>. sample:\n");
	printf("\t\t 0x1 0x56 0x0 0x28 2 2. \n");
	printf("\t\t 0x1 0x56 0x0 0x28. default reg_width and data_width is 1. \n");
}

int i2c_read(int argc , char* argv[])
{
	int retval = 0;
	int fd = -1, tmp, i;
	char file_name[0x10];
	unsigned char buf[4];
	unsigned int i2c_num, dev_addr, reg_addr, reg_addr_end;
	unsigned int reg_width = 1, data_width = 1, reg_step = 1;
	int cur_addr;
	static struct i2c_rdwr_ioctl_data rdwr;
	static struct i2c_msg msg[2];
	unsigned int data;

	memset(buf, 0x0, 4);

	if (argc < READ_MIN_CNT) {
		print_r_usage();
		retval = -1;
		goto end0;
	}

	for (i = 1; i < argc; i++) {
		if (StrToNumber(argv[i], &tmp) != 0) {
			print_r_usage();
			retval = -1;
			goto end0;
		}
		switch (i) {
			case 1:
				i2c_num = tmp;
				break;
			case 2:
				dev_addr = tmp >> 1;
				break;
			case 3:
				reg_addr = tmp;
				reg_addr_end = reg_addr;
				break;
			case 4:
				reg_addr_end = tmp;
				if (reg_addr_end < reg_addr) {
					printf("reg_addr_end < reg_addr error!\n");
					retval = -1;
					goto end0;
				}
				break;
			case 5:
				reg_width = tmp;
				if ((reg_width != 1) && (reg_width != 2)) {
					printf("reg_width must be 1 or 2\n");
					retval = -1;
					goto end0;
				}
				break;
			case 6:
				data_width = tmp;
				if ((data_width != 1) && (data_width != 2)) {
					printf("data_width must be 1 or 2\n");
					retval = -1;
					goto end0;
				}
				break;
			case 7:
				reg_step = tmp;
				if ((reg_addr % reg_step) || (reg_addr_end % reg_step)) {
					printf("((reg_addr or reg_addr_end) %% reg_step) != 0, error!\n");
					retval = -1;
					goto end0;
				}
				break;
			default:
				break;
		}
	}

	printf("i2c_num:0x%x, dev_addr:0x%x; reg_addr:0x%x; reg_addr_end:0x%x; \
			reg_width: %d; data_width: %d; reg_step: %d. \n\n",
			i2c_num, dev_addr << 1, reg_addr, reg_addr_end,
			reg_width, data_width, reg_step);

	sprintf(file_name, "/dev/i2c-%u", i2c_num);
	fd = open(file_name, O_RDWR);
	if (fd < 0) {
		printf("Open %s error!\n",file_name);
		retval = -1;
		goto end0;
	}

	retval = ioctl(fd, I2C_SLAVE_FORCE, dev_addr);
	if (retval < 0) {
		printf("CMD_SET_I2C_SLAVE error!\n");
		retval = -1;
		goto end1;
	}

	msg[0].addr = dev_addr;
	msg[0].flags = 0;
	msg[0].len = reg_width;
	msg[0].buf = buf;

	msg[1].addr = dev_addr;
	msg[1].flags = 0;
	msg[1].flags |= I2C_M_RD;
	msg[1].len = data_width;
	msg[1].buf = buf;

	rdwr.msgs = &msg[0];
	rdwr.nmsgs = (__u32)2;
	for (cur_addr = reg_addr; cur_addr <= reg_addr_end; cur_addr += reg_step) {
		if (reg_width == 2) {
			buf[0] = (cur_addr >> 8) & 0xff;
			buf[1] = cur_addr & 0xff;
		} else
			buf[0] = cur_addr & 0xff;

		retval = ioctl(fd, I2C_RDWR, &rdwr);
		if (retval != 2) {
			printf("CMD_I2C_READ error!\n");
			retval = -1;
			goto end1;
		}

		if (data_width == 2) {
			data = buf[1] | (buf[0] << 8);
		} else
			data = buf[0];

		printf("0x%x: 0x%x\n", cur_addr, data);
	}

	retval = 0;

end1:
	close(fd);
end0:
	return retval;
}

int i2c_write(int argc , char* argv[])
{
	int retval = 0;
	int fd = -1;
	int index = 0, tmp, i;
	char file_name[0x10];
	unsigned char buf[4];
	unsigned int i2c_num, dev_addr, reg_addr, data;
	unsigned int reg_width = 1, data_width = 1;

	if(argc < WRITE_MIN_CNT) {
		print_w_usage();
		retval = -1;
		goto end0;
	}

	for (i = 1; i < argc; i++) {
		if (StrToNumber(argv[i], &tmp) != 0) {
			print_r_usage();
			retval = -1;
			goto end0;
		}
		switch (i) {
			case 1:
				i2c_num = tmp;
				break;
			case 2:
				dev_addr = tmp >> 1;
				break;
			case 3:
				reg_addr = tmp;
				break;
			case 4:
				data = tmp;
				break;
			case 5:
				reg_width = tmp;
				if ((reg_width != 1) && (reg_width != 2)) {
					printf("reg_width must be 1 or 2\n");
					print_r_usage();
					retval = -1;
					goto end0;
				}
				break;
			case 6:
				data_width = tmp;
				if ((data_width != 1) && (data_width != 2)) {
					printf("data_width must be 1 or 2\n");
					print_r_usage();
					retval = -1;
					goto end0;
				}
			default:
				break;
		}
	}

	printf("i2c_num:0x%x, dev_addr:0x%x; reg_addr:0x%x; data:0x%x; reg_width: %d; data_width: %d.\n",
			i2c_num, dev_addr << 1, reg_addr, data, reg_width, data_width);

	sprintf(file_name, "/dev/i2c-%u", i2c_num);
	fd = open(file_name, O_RDWR);
	if (fd<0) {
		printf("Open %s error!\n", file_name);
		retval = -1;
		goto end0;
	}

	retval = ioctl(fd, I2C_SLAVE_FORCE, dev_addr);
	if(retval < 0) {
		printf("set i2c device address error!\n");
		retval = -1;
		goto end1;
	}

	if (reg_width == 2) {
		buf[index] = (reg_addr >> 8) & 0xff;
		index++;
		buf[index] = reg_addr & 0xff;
		index++;
	} else {
		buf[index] = reg_addr & 0xff;
		index++;
	}

	if (data_width == 2) {
		buf[index] = (data >> 8) & 0xff;
		index++;
		buf[index] = data & 0xff;
		index++;
	} else {
		buf[index] = data & 0xff;
		index++;
	}

	retval = write(fd, buf, (reg_width + data_width));
	if(retval < 0) {
		printf("i2c write error!\n");
		retval = -1;
		goto end1;
	}

	retval = 0;

end1:
	close(fd);
end0:
	return retval;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
