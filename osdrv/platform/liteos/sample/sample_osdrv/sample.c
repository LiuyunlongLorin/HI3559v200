/*
 *  osdrv sample
 */
#include "sys_config.h"
#include "board.h"
#include "los_typedef.h"

#include "stdlib.h"
#include "stdio.h"
#ifndef LOSCFG_ARCH_CORTEX_M7
#include "arm_neon.h"
#endif
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
#ifndef  LOSCFG_ARCH_CORTEX_M7
extern AARCHPTR g_sys_mem_addr_end;
void board_config(void)
{
    g_sys_mem_addr_end = SYS_MEM_BASE + SYS_MEM_SIZE_DEFAULT;
#ifdef LOSCFG_DRIVERS_USB
    extern unsigned long g_usb_mem_addr_start;
    extern unsigned long g_usb_mem_size;
    g_usb_mem_addr_start = g_sys_mem_addr_end;
    g_usb_mem_size = 0x100000;
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
#if defined(LOSCFG_PLATFORM_HI3559AV100)
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

#ifdef LOSCFG_DRIVERS_HIETH_SF
extern int /*__init*/ ethnet_hieth_init(void);
    (void)ethnet_hieth_init();
#endif

    extern void get_defaultNetif(struct netif **pnetif);
    get_defaultNetif(&pnetif);

    (void)netifapi_netif_set_up(pnetif);
    do {
        LOS_Msleep(60);
        overtime++;
        if (overtime > 100){
            PRINTK("netif_is_link_up overtime!\n");
            break;
        }
    } while(netif_is_link_up(pnetif) == 0);
}
#endif

void app_init(void)
{
#ifdef LOSCFG_PLATFORM_HISI_AMP
    extern int _ipcm_vdd_init(void);
    dprintf("ipcm init ...\n");
    _ipcm_vdd_init();

#ifndef LOSCFG_ARCH_CORTEX_M7
    extern int sharefs_client_init(const char *path);
    dprintf("sharefs init ...\n");
    sharefs_client_init("/sharefs");
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
    dprintf("\n************************************************************\n");
#endif

#ifdef LOSCFG_DRIVERS_RANDOM
    dprintf("random dev init ...\n");
extern int ran_dev_register(void);
    ran_dev_register();
    dprintf("\n************************************************************\n");
#endif
#ifdef LOSCFG_DRIVERS_MMC
    dprintf("MMC dev init ...");
extern int SD_MMC_Host_init(void);
    SD_MMC_Host_init();
    dprintf("\n************************************************************\n");
#endif
	
#ifdef LOSCFG_DRIVERS_MEM
    dprintf("mem dev init ...\n");
extern int mem_dev_register(void);
    mem_dev_register();
    dprintf("\n************************************************************\n");
#endif

#ifdef LOSCFG_FS_PROC
	dprintf("porc fs init ...\n");
extern void proc_fs_init(void);
    proc_fs_init();
    dprintf("\n************************************************************\n");
#endif

#ifdef LOSCFG_DRIVERS_VIDEO
    dprintf("fb dev init ...\n");
    struct fb_info *info = malloc(sizeof(struct fb_info));
extern int register_framebuffer(struct fb_info *info);
	register_framebuffer(info);
    dprintf("\n************************************************************\n");
#endif	

#ifdef LOSCFG_DRIVERS_SPI
    dprintf("spi bus init ...\n");
extern int spi_dev_init(void);
    spi_dev_init();
    dprintf("\n************************************************************\n");
#endif

#ifdef LOSCFG_DRIVERS_I2C
    dprintf("i2c bus init ...\n");
extern int i2c_dev_init(void);
    i2c_dev_init();
    dprintf("\n************************************************************\n");
#endif
    
#ifdef LOSCFG_DRIVERS_GPIO
    dprintf("gpio init ...\n");
extern int gpio_dev_init(void);
    gpio_dev_init();
    dprintf("\n************************************************************\n");
#endif

#ifdef LOSCFG_DRIVERS_HIEDMAC
extern int hiedmac_init(void);
    hiedmac_init();
    dprintf("\n************************************************************\n");
#endif
#ifdef LOSCFG_DRIVERS_HIDMAC
    dprintf("dmac init ...\n");
extern int hi_dmac_init(void);
    hi_dmac_init();
    dprintf("\n************************************************************\n");
#endif

#ifdef LOSCFG_DRIVERS_MTD_NAND
    dprintf("nand init ...\n");
extern int nand_init(void);
    if(!nand_init()) {
        //dprintf("%s(%0d)\n",__func__,__LINE__);
extern int add_mtd_partition( char *type, UINT32 start_addr, UINT32 length, UINT32 partition_num);
        add_mtd_partition("nand", 0x200000, 32*0x100000, 0);
        add_mtd_partition("nand", 0x200000 + 32*0x100000, 32*0x100000, 1);
extern int mount(const char *source, const char *target,
        const char *filesystemtype, unsigned long mountflags,
        const void *data);
        mount("/dev/nandblk0", "/yaffs0", "yaffs", 0, NULL);
        //mount("/dev/nandblk1", "/yaffs1", "yaffs", 0, NULL);
    }
    dprintf("\n************************************************************\n");
#endif

#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
    dprintf("spi nor flash init ...\n");
extern int spinor_init(void);
    if(!spinor_init()){
        /*
         * If you want use last words function,EXCINFO_RECORD_ADDR and EXCINFO_RECORD_BUF_SIZE must be
         * configured according to the spinor flash partition status(such as:uboot,liteos.bin,fs,and user defined partion)!!!!!!
         * EXCINFO_RECORD_ADDR must not overlap with these partition values(:uboot,liteos.bin,fs,and user defined partion)!!!!!!
         * */
#if 0
   extern VOID osReadWriteExceptionInfo(UINT32 uwStartAddr, UINT32 uwSpace, UINT32 uwRWFlag, CHAR * buff);
   CHAR *pBuf = (CHAR*)malloc(EXCINFO_RECORD_BUF_SIZE);
   LOS_ExcInfoRegHook(EXCINFO_RECORD_ADDR, EXCINFO_RECORD_BUF_SIZE, pBuf, osReadWriteExceptionInfo);
#endif

extern int add_mtd_partition( char *type, UINT32 start_addr, UINT32 length, UINT32 partition_num);
        add_mtd_partition("spinor", 0x100000, 2*0x100000, 0);
        add_mtd_partition("spinor", 3*0x100000, 2*0x100000, 1);
extern int mount(const char *source, const char *target,
        const char *filesystemtype, unsigned long mountflags,
        const void *data);
        mount("/dev/spinorblk0", "/jffs0", "jffs", 0, NULL);
        //mount("/dev/spinorblk1", "/jffs1", "jffs", 0, NULL);
    }
    dprintf("\n************************************************************\n");
#endif

#ifdef LOSCFG_DRIVERS_NETDEV
    dprintf("net init ...\n");
    net_init();
    dprintf("\n************************************************************\n");

#endif

#if defined(LOSCFG_PLATFORM_HI3519AV100) || defined(LOSCFG_PLATFORM_HI3556AV100)
extern void security_scheme_config(void);
    //When use secure security scheme, please call this function
	security_scheme_config();
#endif

    dprintf("g_sys_mem_addr_end=0x%p,\n",g_sys_mem_addr_end);
    dprintf("Date:%s.\n", __DATE__);
    dprintf("Time:%s.\n", __TIME__);
    dprintf("\n************************************************************\n");
#ifdef __OSDRV_TEST__
    test_app_init();
#endif
    return;
}
