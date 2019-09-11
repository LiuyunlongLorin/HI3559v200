1.osdrv 顶层 Makefile 使用说明
本目录下的编译脚本支持 arm-himix100-linux,arm-himix100-linux工具链对应uclibc库。具体命令如下(以hi3559v200举例)

(1)编译整个osdrv目录：
	注意：默认不发布内核源码包，只发布补丁文件。内核源码包需自行从开源社区上下载。
	      从linux开源社区下载v4.9.37版本的内核：
     	1)进入网站：www.kernel.org
     	2)选择HTTP协议资源的https://www.kernel.org/pub/选项,进入子页面
     	3)选择linux/菜单项，进入子页面
     	4)选择kernel/菜单项，进入子页面
     	5)选择v4.x/菜单项，进入子页面
     	6)下载linux-4.9.37.tar.gz (或者linux-4.9.37.tar.xz)到osdrv/opensource/kernel目录下     
    
	make all
	或者
	make BOOT_MEDIA=spi CHIP=hi3559v200 IRQ_BIND=liteos all  (SVP 业务中断(NNIE iVE)绑定到 liteos 端)
	make BOOT_MEDIA=spi CHIP=hi3559v200 all
	或者
	make BOOT_MEDIA=nand CHIP=hi3559v200 all
	或者
	make BOOT_MEDIA=emmc CHIP=hi3559v200 all

注意：由于Makefile中文件系统编译依赖组件较多，不能保证单独编译的文件系统可用，建议采用make all编译；
        但可单独编译uboot，kernel；
    可以传如下参数：
    a, BOOT_MEDIA:spi,nand,emmc;
        spi：SPINOR FLASH; nand:SPINAND FLASH;
    b, CHIP:hi3559v200,hi3556v200
    c, IRQ_BIND:liteos SVP中断绑定到 liteos,默认 SVP 中断绑定到 linux 端
	d, 如果客户想要更换可以使用TARGET_XLSM=*.xlsm来指定所需要的u-boot表格文件(OSDRV默认发布DMEB板表格)

(2)清除整个osdrv目录的编译文件：
	make OSDRV_CROSS=arm-himix100-linux clean
(3)彻底清除整个osdrv目录的编译文件，除清除编译文件外，还删除已编译好的镜像：
	make OSDRV_CROSS=arm-himix100-linux distclean

(4)单独编译kernel(以hi3559v200举例,hi3556v200 编译类似)：
	注意：单独编译内核之前请先阅读osdrv/opensource/kernel下的readme_cn.txt说明。

	待进入内核源代码目录后，执行以下操作
    	存储介质为 spi nor 时执行如下操作：
	cp arch/arm/configs/hi3559v200_amp_defconfig  .config

	存储介质为 spi nand时执行如下操作：
	cp arch/arm/configs/hi3559v200_amp_spinand_defconfig  .config

    	存储介质为 emmc 时执行如下操作   : 
	cp arch/arm/configs/hi3559v200_amp_emmc_defconfig .config

	hi3556v200 可选的配置文件如下：
	hi3556v200_amp_defconfig
	hi3556v200_amp_spinand_defconfig
	hi3556v200_amp_emmc_defconfig

	make ARCH=arm CROSS_COMPILE=arm-himix100-linux- menuconfig
	make ARCH=arm CROSS_COMPILE=arm-himix100-linux- uImage

(5)单独编译模块(以hi3559v200举例,hi3556v200 编译类似)：
	待进入内核源代码目录后，执行以下操作
	cp arch/arm/configs/hi3559v200_amp_defconfig  .config
    存储介质为 spi nand时执行如下操作(其他存储介质操作类似)：
    	cp arch/arm/configs/hi3559v200_amp_spinand_defconfig  .config
	make ARCH=arm CROSS_COMPILE=arm-himix100-linux- menuconfig
	make ARCH=arm CROSS_COMPILE=arm-himix100-linux- modules

(6)单独编译uboot(以hi3559v200举例,hi3556v200 编译类似)：
	(注意：发布包默认按照DMEB板的配置发布，如果客户单板和DEMB板不一致，需要根据客户自己的单板环境修改uboot表格才能使用，否则可能导致uboot无法启动或者其他问题。)
    
    待进入boot源代码目录后，执行以下操作
    spi nor 启动时执行如下操作 ：
	make  ARCH=arm CROSS_COMPILE=arm-himix100-linux- hi3559v200_defconfig

    spi nand 启动时执行如下操作：
    	make  ARCH=arm CROSS_COMPILE=arm-himix100-linux- hi3559v200_nand_defconfig

    emmc 启动时执行如下操作    :
        make  ARCH=arm CROSS_COMPILE=arm-himix100-linux- hi3559v200_emmc_defconfig

    hi3556v200 可选的配置文件如下：
	hi3556v200_defconfig
	hi3556v200_emmc_defconfig
	hi3556v200_nand_defconfig
        
	make ARCH=arm CROSS_COMPILE=arm-himix100-linux- -j 20

    make -C ../../../tools/pc/hi_gzip/

    cp ../../../tools/pc/hi_gzip/bin/gzip arch/arm/cpu/armv7/hi3559v200/hw_compressed/ -rf 

    Windowns下进入到osdrv/tools/pc/uboot_tools/目录下打开对应单板的Excel文件,在main标签中点击"Generate reg bin file"按钮,生成reg_info.bin即为对应平台的表格文件。
    从osdrv/tools/pc/uboot_tools目录拷贝reg_info.bin到boot源代码目录,重命名为.reg
    cp ../../../tools/pc/uboot_tools/reg_info.bin .reg

    make ARCH=arm CROSS_COMPILE=arm-himix100-linux- u-boot-z.bin
    
    opensource/uboot/u-boot-2016.11下生成的u-boot-hi3559v200.bin即为可用的u-boot镜像
	
(7)制作文件系统镜像：
在osdrv/pub/中有已经编译好的文件系统，因此无需再重复编译文件系统，只需要根据单板上flash的规格型号制作文件系统镜像即可。

	SPI NOR Flash使用jffs2格式的镜像，制作jffs2镜像时，需要用到SPI NOR Flash的块大小。这些信息会在uboot启动时
	会打印出来。建议使用时先直接运行mkfs.jffs2工具，根据打印信息填写相关参数。
	下面以块大小为64KB为例：
	osdrv/pub/bin/pc/mkfs.jffs2 -d osdrv/pub/rootfs_uclibc -l -e 0x10000 -o osdrv/pub/rootfs_uclibc_64k.jffs2
	或者
	osdrv/pub/bin/pc/mkfs.jffs2 -d osdrv/pub/rootfs_uclibc -l -e 0x10000 -o osdrv/pub/rootfs_uclibc_64k.jffs2

    	Nand Flash使用yaffs2格式的镜像，制作yaffs2镜像时，需要用到nand flash的pagesize和ecc。这些信息会在uboot启动时会打
	印出来。建议使用时先直接运行mkyaffs2image工具，根据打印信息填写相关参数。
	下面以并口NAND的2KB pagesize、4bit ecc为例：
    	osdrv/pub/bin/pc/mkyaffs2image610 osdrv/pub/rootfs_uclibc osdrv/pub/rootfs_uclibc_2k_4bit.yaffs2 1 2
    	或者
    	osdrv/pub/bin/pc/mkyaffs2image610 osdrv/pub/rootfs_uclibc osdrv/pub/rootfs_uclibc_2k_4bit.yaffs2 1 2

    	Nand Flash使用UBI文件系统，在osdrv/tools/pc/ubi_sh下提供mkubiimg.sh工具用于制作UBI文件系统，需要用到Nand
	Flash的pagesize、blocksize和UBIFS分区的大小。
	以2KB pagesize, 128KB blocksize和UBI文件系统分区大小32MB为例：
	
	mkubiimg.sh hi3559v200 2k 128k osdrv/pub/rootfs 32M osdrv/pub/bin/pc
	
	osdrv/pub/rootfs是根文件系统文件夹目录
	osdrv/pub/bin/pc是制作UBI文件系统镜像的工具目录
    	生成的rootfs_hi3559v200_2k_128k_32M.ubifs，就是最终用于烧写的UBI文件系统镜像。

2. 镜像存放目录说明
编译完的image，rootfs等存放在osdrv/pub目录下
pub
├─ bin
│   ├─ board_uclibc -------------------------------------------- himix100编译出的单板用工具
│   │   ├── ethtool
│   │   ├── flashcp
│   │   ├── flash_erase
│   │   ├── flash_otp_dump
│   │   ├── flash_otp_info
│   │   ├── gdb-arm-himix100-linux
│   │   ├── mtd_debug
│   │   ├── mtdinfo
│   │   ├── nanddump
│   │   ├── nandtest
│   │   ├── nandwrite
│   │   ├── sumtool
│   │   ├── ubiattach
│   │   ├── ubicrc32
│   │   ├── ubidetach
│   │   ├── ubiformat
│   │   ├── ubimkvol
│   │   ├── ubinfo
│   │   ├── ubinize
│   │   ├── ubirename
│   │   ├── ubirmvol
│   │   ├── ubirsvol
│   │   └── ubiupdatevol
│   └─ pc
│       ├── lzma
│       ├── mkfs.cramfs
│       ├── mkfs.jffs2
│       ├── mkfs.ubifs
│       ├── mkimage
│       ├── mksquashfs
│       └── ubinize
├─image_uclibc ------------------------------------------------- himix100编译出的镜像文件
│   ├── uImage_hi3559v200 ----------------------------------------- kernel镜像
│   ├── u-boot-hi3559v200.bin ------------------------------------- u-boot镜像
│   ├── rootfs_hi3559v200_64k.jffs2 ------------------------------- 64K jffs2 文件系统镜像
│   ├── rootfs_hi3559v200_128k.jffs2 ------------------------------ 128K jffs2 文件系统镜像
│   ├── rootfs_hi3559v200_256k.jffs2 ------------------------------ 256K jffs2 文件系统镜像
│   ├── rootfs_hi3559v200_2k_4bit.yaffs2 -------------------------- yaffs2 文件系统镜像
│   ├── rootfs_hi3559v200_2k_24bit.yaffs2 ------------------------- yaffs2 文件系统镜像
│   ├── rootfs_hi3559v200_4k_4bit.yaffs2 -------------------------- yaffs2 文件系统镜像
│   ├── rootfs_hi3559v200_4k_24bit.yaffs2 ------------------------- yaffs2 文件系统镜像
│   ├── rootfs_hi3559v200_2k_128k_32M.ubifs------------------------ 2k 128k UBI 文件系统镜像
│   └── rootfs_hi3559v200_4k_256k_50M.ubifs------------------------ 4k 256k UBI 文件系统镜像
│
├─ rootfs.ubiimg ----------------------------------------------- UBIFS根文件系统
├─ rootfs_uclibc.tgz  ------------------------------------------- himix100编译出的rootfs文件系统

3.osdrv目录结构说明：
osdrv
├─Makefile ------------------------------ osdrv目录编译脚本
├─tools --------------------------------- 存放各种工具的目录
│  ├─board ------------------------------ 各种单板上使用工具
│  │  ├─ethtools ----------------------- ethtools工具
│  │  ├─reg-tools-1.0.0 ----------------- 寄存器读写工具
│  │  ├─udev-167 ------------------------ udev工具集
│  │  ├─mtd-utils ----------------------- flash裸读写工具集
│  │  ├─gdb ----------------------------- gdb工具
│  │  └─e2fsprogs ----------------------- mkfs工具集
│  └─pc ---------------------------------- 各种pc上使用工具
│      ├─jffs2_tool----------------------- jffs2文件系统制作工具
│      ├─cramfs_tool --------------------- cramfs文件系统制作工具
│      ├─squashfs4.3 --------------------- squashfs文件系统制作工具
│      ├─nand_production ----------------- nand量产工具
│      ├─lzma_tool ----------------------- lzma压缩工具
│      ├─zlib ---------------------------- zlib工具
│      ├─mkyaffs2image -- ---------------- yaffs2文件系统制作工具
│      └─uboot_tools ----------------------uboot镜像制作工具、xls文件及ddr初始化脚本、reg_info.bin制作工具
├─pub ----------------------------------- 存放各种镜像的目录
│  ├─image_uclibc ------------------------ 基于himix100工具链编译，可供FLASH烧写的映像文件，包括uboot、内核、文件系统
│  ├─bin -------------------------------- 各种未放入根文件系统的工具
│  │  ├─pc ------------------------------ 在pc上执行的工具
│  │  ├─board_uclibc --------------------- 基于himix100工具链编译，在单板上执行的工具
│  └─rootfs_uclibc.tgz ------------------- 基于himix100工具链编译的根文件系统
├─opensource----------------------------- 存放各种开源源码目录
│  ├─busybox ---------------------------- 存放busybox源代码的目录
│  ├─uboot ------------------------------ 存放uboot源代码的目录
│  └─kernel ----------------------------- 存放kernel源代码的目录
└─rootfs_scripts ------------------------- 存放根文件系统制作脚本的目录

4.注意事项
(1)在windows下复制源码包时，linux下的可执行文件可能变为非可执行文件，导致无法编译使用；u-boot或内核下编译后，会有很多符号链接文件，在windows下复制这些源码包, 会使源码包变的巨大，因为linux下的符号链接文件变为windows下实实在在的文件，因此源码包膨胀。因此使用时请注意不要在windows下复制源代码包。
(2)使用某一工具链编译后，如果需要更换工具链，请先将原工具链编译文件清除，然后再更换工具链编译。
(3)编译板端软件
    a.此芯片具有浮点运算单元和neon。文件系统中的库是采用软浮点和neon编译而成，因此请用户注意，所有此芯片板端代码编译时需要在Makefile里面添加选项-mcpu=cortex-a7、-mfloat-abi=softfp和-mfpu=neon-vfpv4。
如：
对于A7：
    CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fno-aggressive-loop-optimizations
    CXXFlAGS +=-mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fno-aggressive-loop-optimizations
其中CXXFlAGS中的XX根据用户Makefile中所使用宏的具体名称来确定，e.g:CPPFLAGS。
