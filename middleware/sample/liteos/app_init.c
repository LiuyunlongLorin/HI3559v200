/*
 *  osdrv sample
 */
#include "sys_config.h"
#include "board.h"
#include "los_typedef.h"

#include "stdlib.h"
#include "stdio.h"

#ifdef LOSCFG_DRIVERS_VIDEO
#include "linux/fb.h"
#endif
#ifdef LOSCFG_SHELL
#include "shell.h"
#include "shcmd.h"
#endif

#ifdef LOSCFG_DRIVERS_UART
#include "console.h"
#include "hisoc/uart.h"
#include "uart.h"
#endif

#if defined(LOSCFG_DRIVERS_HIGMAC) || defined(LOSCFG_DRIVERS_HIETH_SF)
#include "eth_drv.h"
#endif
#ifndef  LOSCFG_CORTEX_M7
extern AARCHPTR g_sys_mem_addr_end;
void board_config(void)
{
    g_sys_mem_addr_end = SYS_MEM_BASE + SYS_MEM_SIZE_DEFAULT;
#ifdef LOSCFG_DRIVERS_USB
    extern unsigned long g_usb_mem_addr_start;
    extern unsigned long g_usb_mem_size;
    g_usb_mem_addr_start = g_sys_mem_addr_end;
    g_usb_mem_size = 0x20000; //recommend 128K nonCache for usb
#endif
#ifdef LOSCFG_DRIVERS_EMMC
    size_t part0_start_sector = 16   * (0x100000/512);
    size_t part0_count_sector = 1024 * (0x100000/512);
    size_t part1_start_sector = 16   * (0x100000/512) + part0_count_sector;
    size_t part1_count_sector = 1024 * (0x100000/512);
    extern struct disk_divide_info emmc;
    add_mmc_partition(&emmc, part0_start_sector, part0_count_sector);
    add_mmc_partition(&emmc, part1_start_sector, part1_count_sector);
#endif
#if defined(LOSCFG_DRIVERS_HIGMAC) || defined(LOSCFG_DRIVERS_HIETH_SF)
    //different board should set right mode:"rgmii" "rmii" "mii"
    //if you don't set :
#if defined(LOSCFG_PLATFORM_HI3559AV100) || defined(LOSCFG_PLATFORM_HI3556AV100)
    hisi_eth_set_phy_mode("rgmii");
#endif
#if defined(LOSCFG_PLATFORM_HI3559AV100)
	//use eth1 higmac,the phy addr is 3
    hisi_eth_set_phy_addr(3);//0~31
#endif
    //different board should set right addr:0~31
    //if you don't set ,driver will detect it automatically
    //hisi_eth_set_phy_addr(0);//0~31
#endif
}
#endif

#ifdef LOSCFG_DRIVERS_NETDEV
#include "lwip/tcpip.h"
#include "lwip/netif.h"

void net_init(void)
{
    struct netif *pnetif;

    extern int secure_func_register(void);
    (void)secure_func_register();
    extern void tcpip_init(tcpip_init_done_fn initfunc, void *arg);
    tcpip_init(NULL, NULL);
    static unsigned int overtime = 0;
    PRINTK("Ethernet start.");

#ifdef LOSCFG_DRIVERS_HIGMAC
    extern int /*__init*/ ethnet_higmac_init(void);
    (void)ethnet_higmac_init();
#endif
    extern void get_defaultNetif(struct netif **pnetif);
    get_defaultNetif(&pnetif);

    (void)netifapi_netif_set_up(pnetif);
    do
    {
        LOS_Msleep(60);
        overtime++;
        if (overtime > 100)
        {
            PRINTK("netif_is_link_up overtime!\n");
            break;
        }
    }
    while(netif_is_link_up(pnetif) == 0);
}
#endif


extern int app_main(int argc, char *argv[]);

#define ARGS_SIZE_T 20
#define ARG_BUF_LEN_T 256
static char *ptask_args[ARGS_SIZE_T];
static char *args_buf_t = NULL;

static unsigned int taskid = -1;

void com_app(unsigned long p0, unsigned long p1, unsigned long p2, unsigned long p3)
{
    int i = 0;
    unsigned int argc = p0;
    char** argv = (char**)p1;
    //Set_Interupt(0);
    dprintf("\ninput command:\n");

    for (i = 0; i < argc; i++)
    {
        dprintf("%s ", argv[i]);
    }

    dprintf("\n");
    app_main(argc, argv);
    dprintf("\nmain out\n");
    dprintf("[END]:app_test finish!\n");
    free(args_buf_t);
    args_buf_t = NULL;
    taskid = -1;
}


unsigned int app_sample(int argc, char **argv )
{
    int i = 0;
    unsigned int ret = 0;
    int len = 0;
    char* pch = NULL;
    TSK_INIT_PARAM_S stappTask;

    if (argc < 1)
    {
        dprintf("illegal parameter!\n");
    }

    if (taskid != -1)
    {
        dprintf("There's a app_main task existed.");
    }

    args_buf_t = zalloc(ARG_BUF_LEN_T);
    memset(&stappTask, 0, sizeof(TSK_INIT_PARAM_S));
    pch = args_buf_t;

    for (i = 0; i < ARGS_SIZE_T; i++)
    {
        ptask_args[i] = NULL;
    }

    argc++;
    ptask_args[0] = "sdk_test";

    for (i = 1; i < argc; i++)
    {
        len = strlen(argv[i - 1]);
        memcpy(pch, argv[i - 1], len);
        ptask_args[i] = pch;
        //keep a '\0' at the end of a string.
        pch = pch + len + 1;

        if (pch >= args_buf_t + ARG_BUF_LEN_T)
        {
            dprintf("args out of range!\n");
            break;
        }
    }

    memset(&stappTask, 0, sizeof(TSK_INIT_PARAM_S));
    stappTask.pfnTaskEntry = (TSK_ENTRY_FUNC)com_app;
    stappTask.uwStackSize = 0x10000;
    stappTask.pcName = "sdk_test";
    stappTask.usTaskPrio = 10;
    stappTask.uwResved = LOS_TASK_STATUS_DETACHED;
    stappTask.auwArgs[0] = argc;
    stappTask.auwArgs[1] = (unsigned long)ptask_args;
    ret = LOS_TaskCreate(&taskid, &stappTask);

    dprintf("camera_Task %d\n", taskid);

    //chdir("/sd0");
    chdir("/app/sharefs");

    return ret;
}

void sample_command(void)
{
    osCmdReg(CMD_TYPE_EX,"sample", 0, (CMD_CBK_FUNC)app_sample);
}

extern void SDK_init(void);
extern int Media_Msg_Init(void);
extern void CatLogShell(void);
extern int mw_init();
void app_init(void)
{
#ifdef LOSCFG_PLATFORM_HISI_AMP
    extern int _ipcm_vdd_init(void);
    dprintf("ipcm init ...\n");
    _ipcm_vdd_init();

#ifndef LOSCFG_PLATFORM_HI3559AV100_CORTEX_M7
    extern int sharefs_client_init(const char *path);
    dprintf("sharefs init ...\n");
    sharefs_client_init("/app/sharefs");
#endif

    extern int virt_tty_dev_init(void);
    dprintf("virt tty init ...\n");
    virt_tty_dev_init();
#endif

#ifdef LOSCFG_DRIVERS_UART
    dprintf("uart init ...\n");
    if (uart_dev_init() != 0)
    {
        PRINT_ERR("uart_dev_init failed");
    }
    if (virtual_serial_init(TTY_DEVICE) != 0)
    {
        PRINT_ERR("virtual_serial_init failed");
    }
    if (system_console_init(SERIAL) != 0)
    {
        PRINT_ERR("system_console_init failed\n");
    }

#endif

#ifdef LOSCFG_DRIVERS_RANDOM
    dprintf("random dev init ...\n");
    extern int ran_dev_register(void);
    ran_dev_register();
#endif
#ifdef LOSCFG_DRIVERS_MMC
    dprintf("MMC dev init ...");
    extern int SD_MMC_Host_init(void);
    SD_MMC_Host_init();
#endif

#ifdef LOSCFG_DRIVERS_MEM
    dprintf("mem dev init ...\n");
    extern int mem_dev_register(void);
    mem_dev_register();
#endif

#ifdef LOSCFG_FS_PROC
    dprintf("porc fs init ...\n");
    extern void proc_fs_init(void);
    proc_fs_init();
#endif

#ifndef __HI3556AV100__
#ifdef LOSCFG_DRIVERS_SPI
    dprintf("spi bus init ...\n");
    extern int spi_dev_init(void);
    spi_dev_init();
#endif


#ifdef LOSCFG_DRIVERS_I2C
    dprintf("i2c bus init ...\n");
    extern int i2c_dev_init(void);
    i2c_dev_init();
#endif
#endif

#ifdef LOSCFG_DRIVERS_GPIO
    dprintf("gpio init ...\n");
    extern int gpio_dev_init(void);
    gpio_dev_init();
#endif

#ifdef LOSCFG_DRIVERS_HIEDMAC
    extern int hiedmac_init(void);
    hiedmac_init();
#endif
#ifdef LOSCFG_DRIVERS_HIDMAC
    dprintf("dmac init ...\n");
    extern int hi_dmac_init(void);
    hi_dmac_init();
#endif

#ifdef LOSCFG_DRIVERS_MTD_NAND
    dprintf("nand init ...\n");
    extern int nand_init(void);

    if (!nand_init())
    {
        extern int add_mtd_partition( char * type, UINT32 start_addr, UINT32 length, UINT32 partition_num);
        add_mtd_partition("nand", 0x200000, 32 * 0x100000, 0);
        add_mtd_partition("nand", 0x200000 + 32 * 0x100000, 32 * 0x100000, 1);
        extern int mount(const char * source, const char * target,
                         const char * filesystemtype, unsigned long mountflags,
                         const void * data);
        mount("/dev/nandblk0", "/yaffs0", "yaffs", 0, NULL);
        //mount("/dev/nandblk1", "/yaffs1", "yaffs", 0, NULL);
    }

#endif

#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
    dprintf("spi nor flash init ...\n");
    extern int spinor_init(void);

    if (!spinor_init())
    {
        extern int add_mtd_partition( char * type, UINT32 start_addr, UINT32 length, UINT32 partition_num);
        add_mtd_partition("spinor", 0x100000, 2 * 0x100000, 0);
        add_mtd_partition("spinor", 3 * 0x100000, 2 * 0x100000, 1);
        extern int mount(const char * source, const char * target,
                         const char * filesystemtype, unsigned long mountflags,
                         const void * data);
        mount("/dev/spinorblk0", "/jffs0", "jffs", 0, NULL);
        //mount("/dev/spinorblk1", "/jffs1", "jffs", 0, NULL);
    }
#endif

#ifndef __HI3556AV100__
#ifdef LOSCFG_DRIVERS_NETDEV
    dprintf("net init ...\n");
    net_init();
#endif
#endif

    dprintf("g_sys_mem_addr_end=0x%p,\n",g_sys_mem_addr_end);
    dprintf("Date:%s.\n", __DATE__);
    dprintf("Time:%s.\n", __TIME__);

    SDK_init();
    mw_init();
    if(0 != LOS_PathAdd("/app/sharefs"))
    {
        printf("add path /app/sharefs failed \n");
    }

    CatLogShell();
    return;
}
