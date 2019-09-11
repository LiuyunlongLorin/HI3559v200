
#ifdef LOSCFG_SHELL
#include "sys/statfs.h"
#include "stdio.h"
#include "stdlib.h"
#include "fcntl.h"
#include "string.h"
#include "unistd.h"
#include "hisoc/uart.h"
#include "uart.h"
#include "shcmd.h"
#include "sys/prctl.h"
#include "sys/select.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

//#if (2 == UART_NUM)
//static int fd[UART_NUM] = {-1, -1};

#if  (2 == UART_NUM)
static int fd[UART_NUM] = {-1, -1};
#elif (3 == UART_NUM)
static int fd[UART_NUM] = {-1, -1, -1};
#elif (4 == UART_NUM)
static int fd[UART_NUM] = {-1, -1, -1, -1};
#elif (5 == UART_NUM)
static int fd[UART_NUM] = {-1, -1, -1, -1, -1};
#endif

#define BUF_LENGTH    512

static char uart_putc_wait( char c)
{
    UINT32 base = UART_REG_BASE;
    UINT32 status;
    do {
        READ_UINT32(status, base + UART_FR);
    } while (status & 0x28) ;
    WRITE_UINT8(c, base + UART_DR);
    return c;
}

static UINT32 cmd_uart_write(UINT32 argc ,CHAR ** argv)
{
    UINT32 ret = 0;
    char file_name[0x20] = {0};
    int num = 0;
    int len = 0;
    int i = 0;
    num = strtoul(argv[0], NULL, 0);
    len = strlen(argv[1]);

    if ((num < 0) || (num >= UART_NUM) ||
            (len < 0) || (len > BUF_LENGTH)||
            (0 == strcmp("-h",argv[0])||
             (2 != argc))) {
        dprintf("usage: uart_wirte <num> <buf>\n" );
        dprintf("num=0,1,2 ...; the uart port number\n" );
        dprintf("buf: ascii; send buffer.\n" );
        dprintf("note: max write length is 256.\n" );
        ret = 1;
        goto end;
    }
    snprintf(file_name, sizeof(file_name), "/dev/uartdev-%d", num);

    if (fd[num] < 0) {
        fd[num] = open(file_name, O_RDWR);
        dprintf("\nopen uart %d fd is: %d\n", num, fd[num]);
    }
    if (fd[num] < 0) {
        dprintf("open \"%s\" fail.\n", file_name);
        ret = 2;
        goto end;
    }
    dprintf("length is: %d, sent string:\n", len);
    for(i=0; i<len; i++) {
        uart_putc_wait(argv[1][i]);
    }
    ret = write(fd[num], argv[1], len);
    if (ret != len) {
        dprintf("write error \n");
    }
end:
    return ret;
}

int thread_exit;

static void *uart_read_routine(void *arg)
{
    struct timeval tv;
    fd_set fds;
    int ret;
    char file_name[0x20] = {0};
    char str[BUF_LENGTH];
    int num = (int)arg;

    prctl(PR_SET_NAME, (unsigned long)"uart_read_routine", 0, 0, 0);
	dprintf("uart receive thread enter...\n");
    memset(str, 0, BUF_LENGTH);
    if ((num < 0) || (num >= UART_NUM)) {
        dprintf("invalid arg,which is %d\n", num);
        return NULL;
    }
    snprintf(file_name, sizeof(file_name), "/dev/uartdev-%d", num);
    if (fd[num] < 0) {
        fd[num] = open(file_name, O_RDWR);
        dprintf("\nopen uart %d fd is: %d\n", num, fd[num]);
    }
    if (fd[num] < 0) {
        dprintf("open \"%s\" fail.\n", file_name);
        return NULL;
    }
    FD_ZERO(&fds);
    FD_SET(fd[num], &fds);
    while (1) {
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        ret = select(fd[num] + 1, &fds, NULL, NULL, &tv);
        if (ret < 0) {
            dprintf("select error ret:%d\n", ret);
            break;
        } else if (0 == ret) {
            //dprintf("select timeout\n");
			if (thread_exit)
				break;
        }
        ret = read(fd[num], str, BUF_LENGTH);
        //dprintf("read length: %d, string:\n", ret);
        if (ret > 0) {
            UINT32 i=0;
            for(; i<ret; i++) {
                uart_putc_wait(str[i]);
            }
            dprintf("\n");
        }
    }
	thread_exit = 0;
	dprintf("uart receive thread exit.\n");
    return NULL;
}

static UINT32 cmd_uart_read(UINT32 argc ,CHAR ** argv)
{
    UINT32 ret = 0;
    int num;
    int len;
    pthread_t id;

    pthread_attr_t use_attr;

	if (0 == strcmp("-q", argv[0])) {
		thread_exit = 1;
		return 0;
	}

    (void)pthread_attr_init(&use_attr);

    use_attr.inheritsched = PTHREAD_EXPLICIT_SCHED;
    //use_attr.schedparam.sched_priority = 9;
    use_attr.detachstate = PTHREAD_CREATE_DETACHED;

    num = strtoul(argv[0], NULL, 0);
    len = strtoul(argv[1], NULL, 0);
    if ((num < 0) || (num >= UART_NUM) ||
            (len > BUF_LENGTH)||
            (0 == strcmp("-h",argv[0])) ||\
            (2 != argc)) {
        dprintf("usage: uart_read   <num> <len>\n\
                num=0,1,2 ...; the uart port number \n\
                len: read length,0~256\n" );
        ret = 0;
        goto end;
    }
    ret = pthread_create(&id, &use_attr, uart_read_routine, (void *)num);
    if (ret) {
        dprintf("uart read thread create failed!\n");
    }
end:
    return ret;
}

static UINT32 cmd_uart_config(UINT32 argc ,CHAR ** argv)
{
    UINT32 ret = 0;
    int cmd = 0;
    char file_name[0x20] = {0};
    int arg = 0;
    int num;
    struct uart_attr attr;

    num = strtoul(argv[0], NULL, 0);
    if ((num < 0) || (num >= UART_NUM)||\
            (0 == strcmp("-h",argv[0]))||\
            (3 != argc)) {
        dprintf("usage: uart_config <num>  <cmd>  <arg> \n");
        dprintf("num=0,1,2 ...; the uart port number\n" );
        dprintf("cmd: 0x101; config baudrate\n" );
        dprintf("cmd: 0x102; config dma or irq receive,1:dma, 0:irq\n" );
        dprintf("cmd: 0x103; config dma or normal send, 1:dma, 0:normal\n" );
        dprintf("cmd: 0x104; config  read block,1:block, 0:non block\n" );
        dprintf("cmd: 0x105; config RTS,CTS, data bits, stop bits, ect\n" );
        dprintf("cmd: 0x110; uart private command.\n" );
        dprintf("arg: the cmd parameter.\n" );
        ret = 1;
        goto end;
    }
    snprintf(file_name, sizeof(file_name), "/dev/uartdev-%d", num);
    if (fd[num] < 0) {
        fd[num] = open(file_name, O_RDWR);
        dprintf("\nopen uart %d fd is: %d\n", num, fd[num]);
    }
    if (fd[num] < 0) {
        dprintf("open \"%s\" fail.\n", file_name);
        ret = 2;
        goto end;
    }
    cmd = strtoul(argv[1],0,0);
    arg = strtoul(argv[2],0,0);

    dprintf("begin config cmd:0x%x, arg:%d .\n", cmd, arg);
    if (UART_CFG_ATTR == cmd) {
        memcpy(&attr, &arg, sizeof(arg));
        ret = ioctl(fd[num], cmd, &attr);
    } else {
        ret = ioctl(fd[num], cmd, arg);
    }
    if ((signed int)ret < 0) {
        dprintf("config error \n");
        ret = (unsigned int)-1;
    }
end:
    return ret;
}

static UINT32 cmd_uart_close(UINT32 argc ,CHAR ** argv)
{
    UINT32 ret = 0;
    char file_name[0x20] = {0};
    int num;

    num = strtoul(argv[0], NULL, 0);
    if ((num < 0) || (num >= UART_NUM)||\
            (0 == strcmp("-h",argv[0])) ||\
            (1 != argc)) {
        dprintf("usage: uart_close   <num>\n" );
        dprintf("num=1,2 ...; the uart port number\n" );
        ret = 1;
        goto end;
    }
    snprintf(file_name, sizeof(file_name), "/dev/uartdev-%d", num);
    if (fd[num] < 0) {
        dprintf("uart%d is not open.\n", num);
        goto end;
    }
    dprintf("uart \"%s\" close,fd is: %d.\n", file_name, fd[num]);
    ret = close(fd[num]);
    if ((signed int)ret < 0) {
        dprintf("close error \n");
        ret = (unsigned int)-1;
        goto end;
    }
    fd[num] = -1;
end:
    return ret;

}

SHELLCMD_ENTRY(uart_write_shellcmd, CMD_TYPE_EX,"uart_write", 0,(CMD_CBK_FUNC)cmd_uart_write);
SHELLCMD_ENTRY(uart_read_shellcmd,  CMD_TYPE_EX,"uart_read",  0,(CMD_CBK_FUNC)cmd_uart_read);
SHELLCMD_ENTRY(uart_config_shellcmd,CMD_TYPE_EX,"uart_config",0,(CMD_CBK_FUNC)cmd_uart_config);
SHELLCMD_ENTRY(uart_close_shellcmd, CMD_TYPE_EX,"uart_close", 0,(CMD_CBK_FUNC)cmd_uart_close);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* LOSCFG_SHELL */
