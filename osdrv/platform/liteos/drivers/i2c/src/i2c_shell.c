/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "sys_config.h"
#include "errno.h"
#ifdef LOSCFG_SHELL
#include "sys/statfs.h"
#include "fcntl.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "shcmd.h"
#include "i2c.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*  i2c shell control  */
static UINT32 cmd_i2c_read(UINT32 argc ,CHAR ** argv)
{
    int ret = 0;
    int fd;
    char file_name[0x20] = {0};
    UINT32  dev_num = 0;
    UINT32  slave_addr = 0;
    UINT32  reg_addr = 0;
    UINT32  end_reg_addr = 0;
    UINT32  reg_width = 1;
    UINT32  data_width = 1;
    UINT32  addr_width = 7;
    UINT32  curr_addr =0;
    UINT32  data = 0;
    unsigned char recvbuf[4];
#ifdef LOSCFG_HOST_TYPE_HIBVT
    struct i2c_rdwr_ioctl_data rdwr;
    struct i2c_msg msg[2];
#endif

    memset(recvbuf, 0x0 ,4);
    if ((argc < 4) || (argc > 7)) {
        dprintf("usage:i2c_read <i2c_num> <device_addr> <start_reg_addr> <end_reg_addr>"
                " [reg_width] [data_width] [addr_width]. \n");
        return OS_FAIL;
    }
    dev_num = strtoul(argv[0],0,0);

    snprintf(file_name, sizeof(file_name), "/dev/i2c-%u", dev_num);

    fd = open(file_name, O_RDWR);
    if (fd < 0) {
        dprintf("open %s fail!\n",file_name);
        goto err;
    }

    slave_addr = strtoul(argv[1],0,0);
    reg_addr = strtoul(argv[2],0,0);
    end_reg_addr = strtoul(argv[3],0,0);

    if (argc > 4)
        reg_width = strtoul(argv[4],0,0);

    if (argc > 5)
        data_width = strtoul(argv[5],0,0);

    if (argc > 6)
        addr_width = strtoul(argv[6],0,0);

    if (reg_width == 2 )
        ret = ioctl(fd, I2C_16BIT_REG, 1);
    else if (reg_width == 1)
        ret = ioctl(fd, I2C_16BIT_REG, 0);
    else {
        dprintf("error reg_width\n");
        goto closefile;
    }
    if (ret < 0) {
        dprintf("CMD_SET_REG_WIDTH error!\n");
        goto closefile;
    }

    if (data_width == 2)
        ret = ioctl(fd, I2C_16BIT_DATA, 1);
    else if (data_width == 1)
        ret = ioctl(fd, I2C_16BIT_DATA, 0);
    else {
        dprintf("error data_width\n");
        goto closefile;
    }
    if (ret < 0) {
        dprintf("CMD_SET_DATA_WIDTH error!\n");
        goto closefile;
    }

    if (addr_width == 10)
        ret = ioctl(fd, I2C_TENBIT, 1);
    else
        ret = ioctl(fd, I2C_TENBIT, 0);
    if (ret < 0) {
        dprintf("CMD_SET_TENBIT_WIDTH error!\n");
        goto closefile;
    }

    dprintf("i2c_num:0x%x, dev_addr:0x%x; start_reg_addr:0x%x; end_reg_addr:0x%x; ",
            dev_num, slave_addr, reg_addr, end_reg_addr);
    dprintf("reg_width: %d; data_width: %d; addr_width: %d;\n",
            reg_width, data_width, addr_width);

    if ( reg_addr > end_reg_addr) {
        dprintf("error: reg_addr > reg_addr_end .\n");
        goto closefile;
    }

#if (defined(HI3518EV200) || defined(HI3516A))
    /* FIXME:Don't do every thing to slave_addr */
#else
    slave_addr = slave_addr >> 1;
#endif

    ret = ioctl(fd, I2C_SLAVE ,((slave_addr & 0xff)));
    if (ret) {
        dprintf("set i2c_slave fail!\n");
        goto closefile;
    }
    int i=0;
#ifdef LOSCFG_HOST_TYPE_HIBVT
    msg[0].addr = slave_addr;
    msg[0].flags = 0;
    msg[0].len = reg_width;
    msg[0].buf = recvbuf;

    msg[1].addr = slave_addr;
    msg[1].flags = 0;
    msg[1].flags |= I2C_M_RD;
    msg[1].len = data_width;
    msg[1].buf = recvbuf;
    rdwr.msgs = &msg[0];
    rdwr.nmsgs = 2;
#endif
    for (curr_addr = reg_addr ; curr_addr <= end_reg_addr ; curr_addr++ )
    {
#ifdef LOSCFG_HOST_TYPE_HIBVT
        if (reg_width == 2) {
            recvbuf[0] = (curr_addr >> 8) & 0xff;
            recvbuf[1] = curr_addr & 0xff;
        }
        else
            recvbuf[0] = curr_addr & 0xff;
        ret = ioctl(fd, I2C_RDWR, &rdwr);
#else
        if (reg_width == 2) {
            recvbuf[0] = curr_addr & 0xff;
            recvbuf[1] = (curr_addr >> 8) & 0xff;
        }
        else
            recvbuf[0] = curr_addr & 0xff;

        ret = read(fd, recvbuf, reg_width + data_width);
#endif
        if (ret < 0) {
            dprintf("read i2c error ! \n");
            goto closefile;
        }

        if (data_width == 2)
            data = recvbuf[0] | (recvbuf[1] << 8);
        else
            data = recvbuf[0];
        if(i%16==0)
            dprintf("\n0x%08x:",curr_addr);
        dprintf("0x%04x ", data);
        i++;
    }

    close(fd);
    dprintf("\n[END]\n");
    return LOS_OK;

closefile:
    close(fd);

err:
    return -1;

}

static UINT32 cmd_i2c_write(UINT32 argc ,CHAR ** argv)
{
    int ret = 0, index = 0;
    int fd;
    char file_name[0x20] = {0};
    UINT32  dev_num = 0;
    UINT32  slave_addr = 0;
    UINT32  reg_addr = 0;
    UINT32  reg_value = 0;
    UINT32  reg_width = 1;
    UINT32  data_width = 1;
    UINT32  addr_width = 7;
    unsigned char sentbuf[4];

    memset(sentbuf, 0x0 ,4);
    if ((argc < 4) || (argc > 7)) {
        dprintf("usage:i2c_write <i2c_num> <device_addr> <reg_addr> <reg_value>"
                " [reg_width] [data_width] [addr_width]. \n");
        return OS_FAIL;
    }
    dev_num = strtoul(argv[0],0,0);


    snprintf(file_name, sizeof(file_name), "/dev/i2c-%u", dev_num);

    slave_addr = strtoul(argv[1],0,0);
    reg_addr = strtoul(argv[2],0,0);
    reg_value = strtoul(argv[3],0,0);

    if (reg_addr > 0xffff) {
        dprintf("wrong reg_addr!\n");
        return -1;
    }
    if (reg_value > 0xffff) {
        dprintf("wrong reg_value!\n");
        return -1;
    }
	fd = open(file_name, O_RDWR);
	if (fd < 0) {
		dprintf("open %s fail!\n",file_name);
		return -OS_FAIL;
	}
    if (argc > 4)
        reg_width = strtoul(argv[4],0,0);
    if (argc > 5)
        data_width = strtoul(argv[5],0,0);
    if (argc > 6)
        addr_width = strtoul(argv[6],0,0);

    if (reg_width == 2 )
        ret = ioctl(fd, I2C_16BIT_REG, 1);
    else if (reg_width == 1)
        ret = ioctl(fd, I2C_16BIT_REG, 0);
    else {
        dprintf("error reg_width\n");
        goto closefile;
    }
    if (ret < 0) {
        dprintf("CMD_SET_REG_WIDTH error!\n");
        goto closefile;
    }

    if (data_width == 2)
        ret = ioctl(fd, I2C_16BIT_DATA, 1);
    else if (data_width == 1)
        ret = ioctl(fd, I2C_16BIT_DATA, 0);
    else {
        dprintf("error data_width\n");
        goto closefile;
    }
    if (ret < 0) {
        dprintf("CMD_SET_DATA_WIDTH error!\n");
        goto closefile;
    }

    if (addr_width == 10)
        ret = ioctl(fd, I2C_TENBIT, 1);
    else
        ret = ioctl(fd, I2C_TENBIT, 0);
    if (ret < 0) {
        dprintf("CMD_SET_TENBIT_WIDTH error!\n");
        goto closefile;
    }

    dprintf("i2c_num:0x%x, dev_addr:0x%x; reg_addr:0x%x; reg_value:0x%x; ",
            dev_num, slave_addr, reg_addr, reg_value);
    dprintf("reg_width: %d; data_width: %d; addr_width: %d;\n",
            reg_width, data_width, addr_width);
#if (defined(HI3518EV200) || defined(HI3516A))
    /* FIXME:Don't do every thing to slave_addr */
#else
    slave_addr = slave_addr >> 1;
#endif

    ret = ioctl(fd, I2C_SLAVE ,((slave_addr & 0xff)));
    if (ret) {
        dprintf("set i2c_slave fail!\n");
        goto closefile;
    }

#ifdef LOSCFG_HOST_TYPE_HIBVT
    if (reg_width == 2) {
        sentbuf[index] = (reg_addr >> 8) & 0xff;
        index++ ;
        sentbuf[index] = reg_addr & 0xff;
        index++;
    } else {
        sentbuf[index] = reg_addr & 0xff;
        index++;
    }

    if (data_width == 2){
        sentbuf[index] = (reg_value >> 8) & 0xff;
        index++ ;
        sentbuf[index] = reg_value & 0xff;
        index++;
    } else {
        sentbuf[index] = reg_value & 0xff;
    }

    ret = write(fd, sentbuf, (reg_width + data_width));
#else
    if (reg_width == 2) {
        sentbuf[index] = reg_addr & 0xff;
        index++ ;
        sentbuf[index] = (reg_addr >> 8) & 0xff;
        index++;
    } else {
        sentbuf[index] = reg_addr & 0xff;
        index++;
    }

    if (data_width == 2){
        sentbuf[index] = reg_value & 0xff;
        index++ ;
        sentbuf[index] = (reg_value >> 8) & 0xff;
        index++;
    } else {
        sentbuf[index] = reg_value & 0xff;
    }

    ret = write(fd, sentbuf, reg_width + data_width);
#endif
    if(ret < 0) {
        dprintf("write value error!\n");
        goto closefile;
    }
    dprintf("\n%s  -->  0x%x\n", argv[2], reg_value);
    dprintf("\n[END]\n");

    close(fd);
    return 0;

closefile:
    close(fd);
err:
    return -1;

}


SHELLCMD_ENTRY(i2c_read_shellcmd,  CMD_TYPE_EX,"i2c_read",  0,(CMD_CBK_FUNC)cmd_i2c_read);
SHELLCMD_ENTRY(i2c_write_shellcmd, CMD_TYPE_EX,"i2c_write", 0,(CMD_CBK_FUNC)cmd_i2c_write);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
