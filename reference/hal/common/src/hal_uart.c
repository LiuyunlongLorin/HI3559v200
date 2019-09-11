/**
* @file    hal_adc.c
* @brief   product hal adc implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/18
* @version   1.0
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include "hi_appcomm_log.h"
#include "hi_hal_uart_inner.h"
#include "hi_hal_common.h"

static HI_S32 g_speed[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, };
static UARTHAL_BIT_RATE g_name[]  = {HAL_UART_BITRATE_115200, HAL_UART_BITRATE_38400, HAL_UART_BITRATE_19200, HAL_UART_BITRATE_9600,  \
                           HAL_UART_BITRATE_4800, HAL_UART_BITRATE_2400, HAL_UART_BITRATE_1200, HAL_UART_BITRATE_300, };

static HI_S32 UART_SetSpeed(HI_S32 fd, UARTHAL_BIT_RATE speed)
{
    HI_S32 i;
    HI_S32 status;
    struct termios Opt;
    tcgetattr(fd, &Opt);

    for (i = 0; i  < (sizeof(g_speed)/sizeof(g_speed[0])); i++)
    {
        if (speed == g_name[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, g_speed[i]);
            cfsetospeed(&Opt, g_speed[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);

            if (status != 0)
            {
                perror("tcsetattr fd1");
            }

            tcflush(fd, TCIOFLUSH);
            break;
        }
    }

    if (i == (sizeof(g_speed) / sizeof(g_speed[0])))
    {
        MLOGE("no this bitrate\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 UART_SetParity(HI_S32 fd, UARTHAL_DATA_BIT dataBits, UARTHAL_STOP_BIT stopBits, UARTHAL_PARITY parity)
{
    struct termios options;

    if  ( tcgetattr(fd, &options)  !=  0)
    {
        perror("SetupSerial 1");
        return (HI_FAILURE);
    }

    options.c_cflag &= ~CSIZE;

    switch (dataBits) /* Set the number of data bits */
    {
        case HAL_UART_DATABIT_7:
            options.c_cflag |= CS7;
            break;

        case HAL_UART_DATABIT_8:
            options.c_cflag |= CS8;
            break;

        default:
            fprintf(stderr, "Unsupported data size\n");
            return (HI_FAILURE);
    }

    switch (parity)
    {
        case HAL_UART_PARITY_N:
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;

        case HAL_UART_PARITY_O:
            options.c_cflag |= (PARODD | PARENB); /* Set to odd effect */
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;

        case HAL_UART_PARITY_E:
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;    /* Conversion to even test */
            options.c_iflag |= INPCK;      /* Disnable parity checking */
            break;

        case HAL_UART_PARITY_S:
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;

        default:
            fprintf(stderr, "Unsupported parity\n");
            return (HI_FAILURE);
    }

    /* Set stop bit */
    switch (stopBits)
    {
        case HAL_UART_STOPBIT_1:
            options.c_cflag &= ~CSTOPB;
            break;

        case HAL_UART_STOPBIT_2:
            options.c_cflag |= CSTOPB;
            break;

        default:
            fprintf(stderr, "Unsupported stop bits\n");
            return (HI_FAILURE);
    }

    /* Set input parity option */
    if (parity != HAL_UART_PARITY_N)
    {
        options.c_iflag |= INPCK;
    }

    tcflush(fd, TCIFLUSH);
    options.c_cc[VTIME] = 30;  /* Set timeout 3 seconds */
    options.c_cc[VMIN] = 0;  /* Update the options and do it NOW */

    /* If it is not a development terminal or the like, only the serial port transmits data,
     * and does not need the serial port to process, then the Raw Mode is used to communicate.
     */

    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON | IXOFF | IXANY);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN);
    options.c_oflag &= ~OPOST;

    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("SetupSerial 3");
        return (HI_FAILURE);
    }

    return HI_SUCCESS;
}

HI_S32 HAL_UART_Init(HI_CHAR *uartDev, HI_S32 *uartFd)
{
    *uartFd = open(uartDev, O_RDWR  | O_NOCTTY | O_NONBLOCK);

    if (*uartFd == -1)
    {
        MLOGE("uart dev open failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HAL_UART_SetAttr(HI_S32 uartFd, UARTHAL_ATTR *uartAttr)
{
    HI_S32 ret;

    ret = UART_SetSpeed(uartFd, uartAttr->bitRate);
    if(HI_SUCCESS != ret)
    {
        MLOGE("Set bit rate Error\n");
        return HI_HAL_EINTER;
    }

    ret = UART_SetParity(uartFd, uartAttr->dataBits, uartAttr->stopBits, uartAttr->parity);

    if (ret != HI_SUCCESS)
    {
        MLOGE("Set Parity Error\n");
        return HI_HAL_EINTER;
    }

    return HI_SUCCESS;
}

HI_S32 HAL_UART_DeInit(HI_S32 uartFd)
{

    close(uartFd);
    return HI_SUCCESS;
}
