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
#include "fcntl.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "shcmd.h"
#include "spi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define READ_MIN_CNT 4
#define WRITE_MIN_CNT 5

extern int toupper(int c);
extern int isdigit(int c);
static int atoul(char *str, int * pulValue)
{
    unsigned int ulResult=0;

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
        ch=toupper((int)*str);
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

static int  StrToNumber(char *str , int * pulValue)
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
    dprintf("Usage: ssp_read <spi_num> <csn> <dev_addr> <reg_addr> [num_reg] [dev_width] [reg_width] [data_width].\n");
    dprintf("\tnum_reg and dev_width and reg_width and data_width can be omitted, the default is 0x1.\n");
    dprintf("eg:\n");
    dprintf("\tssp_read 0x0 0x0 0x2 0x0 0x10 0x1 0x1 0x1.\n");
    dprintf("\tssp_read 0x0 0x0 0x2 0x0. default num_reg and dev_width and reg_width and data_width is 0x1.\n");
    return;
}

static void print_w_usage(void)
{
    dprintf("Usage: ssp_write <spi_num> <csn> <dev_addr> <reg_addr> <data> [dev_width] [reg_width] [data_width].\n");
    dprintf("\tdev_width and reg_width and data_width can be omitted, the default is 0x1.\n");
    dprintf("eg:\n");
    dprintf("\tssp_write 0x0 0x0 0x2 0x0 0x65 0x1 0x1 0x1.\n");
    dprintf("\tssp_write 0x0 0x0 0x2 0x0 0x65. default dev_width and reg_width and data_width is 0x1.\n");
    return;
}

static UINT32 cmd_ssp_read(int argc, char* argv[])
{
    int retval = 0;
    int i = 0, index = 0;
    int tmp = 0;
    int fd;
    char file_name[0x20] = {0};
    unsigned char  buf[0x10];
    struct spi_ioc_transfer transfer[1];
    unsigned int spi_num=0, csn=0, dev_addr = 0, reg_addr = 0, cur_reg_addr;
    unsigned int num_reg = 1, dev_width = 1, reg_width = 1, data_width = 1;
    buf[0] = 0x8f;
    buf[1] = 0x00;
    if (argc < READ_MIN_CNT) {
        print_r_usage();
        retval = -1;
        goto end0;
    }

    for (i = 0; i < argc; i++) {
        if (StrToNumber(argv[i], &tmp) != 0) {
            print_r_usage();
            retval = -1;
            goto end0;
        }

        switch (i) {
            case 0:
                spi_num = tmp;
                break;
            case 1:
                csn = tmp;
                break;
            case 2:
                dev_addr = tmp;
                break;
            case 3:
                reg_addr = tmp;
                break;
            case 4:
                num_reg = tmp;
                break;
            case 5:
                dev_width = tmp;
                if ((dev_width != 1) && (dev_width != 2)) {
                    dprintf("dev_width must be 1 or 2\n");
                    print_r_usage();
                    retval = -1;
                    goto end0;
                }
                break;
            case 6:
                reg_width = tmp;
                if ((reg_width != 1) && (reg_width != 2)) {
                    dprintf("reg_width must be 1 or 2\n");
                    print_r_usage();
                    retval = -1;
                    goto end0;
                }
                break;
            case 7:
                data_width = tmp;
                if ((data_width != 1) && (data_width != 2)) {
                    dprintf("data_width must be 1 or 2\n");
                    print_r_usage();
                    retval = -1;
                    goto end0;
                }
                break;
            default:
                break;
        }
    }

    dprintf("spi_num:%u, csn:%u\n"
            "dev_addr:0x%04x, reg_addr:0x%04x, num_reg:%d, "
            "dev_width:%d, reg_width:%d, data_width:%d\n",
            spi_num, csn, dev_addr, reg_addr, num_reg,
            dev_width, reg_width, data_width);


    memset(transfer, 0, sizeof transfer);

    transfer[0].tx_buf = (char *)buf;
    transfer[0].rx_buf = (char *)buf;
    transfer[0].len = dev_width + reg_width + data_width;
    transfer[0].cs_change = 1;

    snprintf(file_name, sizeof(file_name), "/dev/spidev%u.%u", spi_num, csn);

    fd = open(file_name, O_RDWR);
    if (fd < 0) {
        dprintf("Open %s error!\n",file_name);
        retval = -1;
        goto end0;
    }

    //tmp = SPI_MODE_3 | SPI_LSB_FIRST;
    tmp = SPI_MODE_3 ;

    retval = ioctl(fd, SPI_IOC_WR_MODE, &tmp);
    if (retval) {
        dprintf("set spi mode fail!\n");
        retval = -1;
        goto end1;
    }

    //memset(buf, 0, sizeof buf);
     retval = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer[0]);
     if (retval  != transfer[0].len) {
            dprintf("SPI_IOC_MESSAGE error \n");
            retval = -1;
            goto end1;
        }
    dprintf("buf1= %02x\n", buf[1]);
    dprintf("====reg_addr:0x%04x====\n", reg_addr);

    // for (cur_reg_addr = reg_addr, i = 0; cur_reg_addr < reg_addr + num_reg; cur_reg_addr++, i++) {
    //     index = 0;

    //     if(dev_width == 1) {
    //         *(uint8_t *)(&buf[index]) = (dev_addr & 0xff) | 0x80;
    //         index++;
    //     }else {
    //         uint16_t *tempp = (uint16_t *)(&buf[index]);
    //         *tempp = (dev_addr & 0xffff) | 0x8000;
    //         index += 2;
    //     }

    //     if(reg_width == 1) {
    //         *(uint8_t *)(&buf[index]) = cur_reg_addr & 0xff;
    //         index++;
    //     } else {
    //         *(uint8_t *)(&buf[index]) = (cur_reg_addr>>8) & 0xff;
    //         index += 1;
    //         *(uint8_t *)(&buf[index]) = cur_reg_addr & 0xff;
    //         index += 1;
    //     }

    //     if(data_width == 1) {
    //         *(uint8_t *)(&buf[index]) = 0x00;
    //     } else {
    //         uint16_t *tempp = (uint16_t *)(&buf[index]);
    //         *tempp = 0x0000;
    //     }

    //     retval = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer[0]);

    //     if (retval  != transfer[0].len) {
    //         dprintf("SPI_IOC_MESSAGE error \n");
    //         retval = -1;
    //         goto end1;
    //     }

    //     retval = 0;
    //     if(data_width == 1) {
    //         tmp = *(uint8_t *)(&buf[index]);
    //     } else {
    //         uint16_t *tempp = (uint16_t *)(&buf[index]);
    //         tmp = *tempp;
    //     }

    //     if ((i % 0x10) == 0) {
    //         dprintf("\n0x%04x:  ", i);
    //     }
    //     dprintf("0x%04x ", tmp);
    // }

    dprintf("\n[END]\n");

end1:
    close(fd);
end0:
    return retval;
}

static UINT32 cmd_ssp_write(int argc, char* argv[])
{
    int retval = 0;
    int i = 0, index = 0;
    int tmp = 0;
    int fd;
    char file_name[0x20] = {0};
    unsigned char  buf[0x10];
    struct spi_ioc_transfer transfer[1];
    unsigned int spi_num=0, csn=0, reg_addr=0, dev_addr = 0, data = 0;
    unsigned int dev_width = 1, reg_width = 1, data_width = 1;

    if (argc < WRITE_MIN_CNT) {
        print_w_usage();
        retval = -1;
        goto end0;
    }

    for (i = 0; i < argc; i++) {
        if (StrToNumber(argv[i], &tmp) != 0) {
            print_w_usage();
            retval = -1;
            goto end0;
        }

        switch (i) {
            case 0:
                spi_num = tmp;
                break;
            case 1:
                csn = tmp;
                break;
            case 2:
                dev_addr = tmp;
                break;
            case 3:
                reg_addr = tmp;
                break;
            case 4:
                data = tmp;
                break;
            case 5:
                dev_width = tmp;
                if ((dev_width != 1) && (dev_width != 2)) {
                    dprintf("dev_width must be 1 or 2\n");
                    print_w_usage();
                    retval = -1;
                    goto end0;
                }
                break;
            case 6:
                reg_width = tmp;
                if ((reg_width != 1) && (reg_width != 2)) {
                    dprintf("reg_width must be 1 or 2\n");
                    print_w_usage();
                    retval = -1;
                    goto end0;
                }
                break;
            case 7:
                data_width = tmp;
                if ((data_width != 1) && (data_width != 2)) {
                    dprintf("data_width must be 1 or 2\n");
                    print_w_usage();
                    retval = -1;
                    goto end0;
                }
                break;
            default:
                break;
        }
    }

    dprintf("spi_num:%u, csn:%u\n"
            "dev_addr:0x%04x, reg_addr:0x%04x, data:0x%04x, "
            "dev_width:%d, reg_width:%d, data_width:%d\n",
            spi_num, csn, dev_addr, reg_addr, data,
            dev_width, reg_width, data_width);


    memset(transfer, 0, sizeof transfer);

    transfer[0].tx_buf = (char *)buf;
    transfer[0].rx_buf = (char *)buf;
    transfer[0].len = dev_width + reg_width + data_width;
    transfer[0].cs_change = 1;

    snprintf(file_name, sizeof(file_name), "/dev/spidev%u.%u", spi_num, csn);

    fd = open(file_name, 0);
    if (fd < 0) {
        dprintf("Open %s error!\n",file_name);
        retval = -1;
        goto end0;
    }

    tmp = SPI_MODE_3 | SPI_LSB_FIRST;

    retval = ioctl(fd, SPI_IOC_WR_MODE, &tmp);
    if (retval) {
        dprintf("set spi mode fail!\n");
        retval = -1;
        goto end1;
    }

    memset(buf, 0, sizeof buf);

    if(dev_width == 1) {
        *(uint8_t *)(&buf[index]) = dev_addr & 0xff ;
        index++;
    }else {
        uint16_t * tempp = (uint16_t *)(&buf[index]);
        *tempp = dev_addr & 0xffff & (~0x8000);
        index += 2;
    }

    if(reg_width == 1) {
        *(uint8_t *)(&buf[index]) = reg_addr & 0xff;
        index++;
    } else {
        *(uint8_t *)(&buf[index]) = (reg_addr >>8) & 0xff;
        index += 1;
        *(uint8_t *)(&buf[index]) = reg_addr & 0xff;
        index += 1;
    }

    if(data_width == 1) {
        *(uint8_t *)(&buf[index]) = data & 0xff;
    } else {
        uint16_t * tempp = (uint16_t *)(&buf[index]);
         *tempp = data & 0xffff;
    }

    retval = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer[0]);

    if (retval != transfer[0].len) {
        dprintf("SPI_IOC_MESSAGE error \n");
        retval = -1;
        goto end1;
    }
    retval = 0;

    dprintf("\n[END]\n");

end1:
    close(fd);
end0:
    return retval;
}

SHELLCMD_ENTRY(ssp_read_shellcmd, CMD_TYPE_EX,"ssp_read",0,(CMD_CBK_FUNC)cmd_ssp_read);
SHELLCMD_ENTRY(ssp_write_shellcmd, CMD_TYPE_EX,"ssp_write",0,(CMD_CBK_FUNC)cmd_ssp_write);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
