1.How to use Makefile of directory osdrv:
The arm-himix100-linux toolchain for uclibc. Specific compiling commands are as follows:

(1)Compile the entire osdrv directory:
	Notes: The linux_v4.9.37 source file is not released default. Please download it from www.kernel.org. 
        1) Go to the website: www.kernel.org
		2) Select HTTP protocol resources https://www.kernel.org/pub/ option, enter the sub-page
        3) Select linux/ menu item, enter the sub-page
        4) Select the kernel/ menu item, enter the sub-page
        5) Select v4.x/ menu item, enter sub-page
        6) Download linux-4.9.37.tar.gz (or linux-4.9.37.tar.bz2/linux-4.9.37.tar.xz) to osdrv/opensource/kernel
	       
	make OSDRV_CROSS=arm-himix100-linux all
	or
	make BOOT_MEDIA=spi CHIP=hi3559v200 IRQ_BIND=liteos all (SVP Business's Interrupt(NNIE IVE) escalation to CPU1)
	make BOOT_MEDIA=spi CHIP=hi3559v200 all         		(default Interrupt escalation to CPU0)	
	or
	make BOOT_MEDIA=nand CHIP=hi3559v200 all
	or
	make BOOT_MEDIA=emmc CHIP=hi3559v200 all

	reg info file for U-boot. The TARGET_XLSM parameter is using for choosing the
	other reg info files. As below:
	make OSDRV_CROSS=arm-himix100-linux all TARGET_XLSM=*.xlsm

(2)Clear all compiled files under osdrv directory:
	make OSDRV_CROSS=arm-himix100-linux clean
(3)Completely remove all compiled files under osdrv directory, and the generated images:
	make OSDRV_CROSS=arm-himix100-linux distclean
(4)Separately compile kernel(for example compiling hi3559v200 and hi3556v200 is similar to hi3559v200):
	Notes: before separately compile kernel, please read the readme_en.txt at osdrv/opensource/kernel.

	Enter the top directory the kernel source code, do the following:
	When a storage medium for spi nor perform the following operations:
	cp arch/arm/configs/hi3559v200_amp_defconfig  .config

	When a storage medium for spi nand perform the following operations:
	cp arch/arm/configs/hi3559v200_amp_spinand_defconfig  .config

	When a storage medium for emmc perform the following operations:
	cp arch/arm/configs/hi3559v200_amp_emmc_defconfig .config

	These config files are available for hi3556v200:
	hi3556v200_amp_defconfig
	hi3556v200_amp_spinand_defconfig
	hi3556v200_amp_emmc_defconfig

	make ARCH=arm CROSS_COMPILE=arm-himix100-linux- menuconfig
	make ARCH=arm CROSS_COMPILE=arm-himix100-linux- uImage
(5)Separately compile modules:
	Enter the top directory the kernel source code, do the following:
	When a storage medium for spi nor perform the following operations and other storage mediums are similar to spi nor:
	cp arch/arm/configs/hi3559v200_amp_defconfig  .config
	make ARCH=arm CROSS_COMPILE=arm-himix100-linux- menuconfig
	make ARCH=arm CROSS_COMPILE=arm-himix100-linux- modules
(6)Separately compile uboot(for example compiling hi3559v200 and hi3556v200 is Similar to hi3559v200):
	(Note: The package is released on the DMEB board by default. If the customer's board is inconsistent with the DEMB board, you need to modify the uboot table ac cording to the customer's own board environment. Otherwise, uboot may not start or other problems may occur.)

    Enter the top directory of boot source code, do the following:
    When a storage medium for spi nor perform the following operations:
    make ARCH=arm CROSS_COMPILE=arm-himix100-linux- hi3559v200_defconfig

    When a storage medium for spi nand perform the following operations:
    make  ARCH=arm CROSS_COMPILE=arm-himix100-linux- hi3556v200_nand_defconfig

    When a storage medium for emmc perform the following operations:
    make  ARCH=arm CROSS_COMPILE=arm-himix100-linux- hi3556v200_emmc_defconfig

    These config files are available for hi3556v200:
    hi3556v200_defconfig
    hi3556v200_emmc_defconfig
    hi3556v200_nand_defconfig
    
    make ARCH=arm CROSS_COMPILE=arm-himix100-linux-
    
    make -C ../../../tools/pc/hi_gzip

    cp ../../../tools/pc/hi_gzip/bin/gzip arch/arm/cpu/armv7/hi3559v200/hw_compressed/ -rf 
    
    cp ../../../tools/pc/uboot_tools/reg_info.bin .reg
    
    make ARCH=arm CROSS_COMPILE=arm-himix100-linux- u-boot-z.bin


(7)Build file system image:
        A compiled file system has already been in osdrv/pub/, so no need to re-build file system. What you need to do is to build the right file system image according to the flash specification of the single-board. 
	Filesystem images of jffs2 format is available for spi flash. While making jffs2 image or squashfs image, you need to specify the spi flash block size. flash block size will be printed when uboot runs. run mkfs.jffs2 first to get the right parameters from it's printed information. Here the block size is 64KB, for example:

	jffs2 filesystem image:
	osdrv/pub/bin/pc/mkfs.jffs2 -d osdrv/pub/rootfs_uclibc -l -e 0x10000 -o osdrv/pub/rootfs_uclibc_64k.jffs2
	or:
	osdrv/pub/bin/pc/mkfs.jffs2 -d osdrv/pub/rootfs_uclibc -l -e 0x10000 -o osdrv/pub/rootfs_uclibc_64k.jffs2

    	Filesystem image of yaffs2 format is available for nand flash. While making yaffs2 image, you need to specify its page size and ECC. This information will be printed when uboot runs. run mkyaffs2image first to get the right parr ameters from it's printed information. Here to 2KB pagesize, 4bit ecc of parallel nand flash, for example:

    	osdrv/pub/bin/pc/mkyaffs2image610 osdrv/pub/rootfs_uclibc osdrv/pub/rootfs_uclibc_2k_4bit.yaffs2 1 2
    	or:
    	osdrv/pub/bin/pc/mkyaffs2image610 osdrv/pub/rootfs_uclibc osdrv/pub/rootfs_uclibc_2k_4bit.yaffs2 1 2
	
    	UBIFS is available for nand flash. The mkubiimg.sh tool is specialized for making UBIFS image at the path: osdrv/tools/pc/ubi_sh.
    	Here to 2KB pagesize,128KiB blocksize, and 32MB mtdpartition for example:
	
	mkubiimg.sh hi3559v200 2k 128k osdrv/pub/rootfs 32M osdrv/pub/bin/pc
	
	osdrv/pub/rootfs is the route of rootfs;
	osdrv/pub/bin/pc is the tool path of making UBI iamge;
	rootfs_hi3559v200_2k_128k_32M.ubifs is the UBIFS image for burning;	
	
2. Output directory description
All compiled images, rootfs are located in directory osdrv/pub/.
pub
├── bin
│   ├── board_uclibc -------------------------------------------- tools used on board with himix100
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
│   └── pc
│       ├── lzma
│       ├── mkfs.cramfs
│       ├── mkfs.jffs2
│       ├── mkfs.ubifs
│       ├── mkimage
│       ├── mksquashfs
│       └── ubinize
├─image_uclibc ------------------------------------------------- Images compiled with himix100
│   ├── uImage_hi3559v200 ----------------------------------------- kernel imag
│   ├── u-boot-hi3559v200.bin ------------------------------------- u-boot image
│   ├── rootfs_hi3559v200_64k.jffs2 ------------------------------- jffs2 rootfs image(SPI NOR flash blocksize = 64K)
│   ├── rootfs_hi3559v200_128k.jffs2 ------------------------------ jffs2 rootfs image(SPI NOR flash blocksize = 128K)
│   ├── rootfs_hi3559v200_256k.jffs2 ------------------------------ jffs2 rootfs image(SPI NOR flash blocksize = 256K)
│   ├── rootfs_hi3559v200_2k_4bit.yaffs2 -------------------------- yaffs2 rootfs image(nand-flash pagesize=2K ecc=4bit)
│   ├── rootfs_hi3559v200_2k_24bit.yaffs2 ------------------------- yaffs2 rootfs image(nand-flash pagesize=2K ecc=24bit)
│   ├── rootfs_hi3559v200_4k_4bit.yaffs2 -------------------------- yaffs2 rootfs image(nand-flash pagesize=4K ecc=4bit)
│   ├── rootfs_hi3559v200_4k_24bit.yaffs2 ------------------------- yaffs2 rootfs image(nand-flash pagesize=4K ecc=24bit)
│   ├── rootfs_hi3559v200_2k_128k_32M.ubifs------------------------ ubifs rootfs image(SPI nand-flash pagesize=2K blocksize=128K)
│   └── rootfs_hi3559v200_4k_256k_50M.ubifs------------------------ ubifs rootfs image(SPI nand-flash pagesize=4K blocksize=256K)
│
├─ rootfs.ubiimg ----------------------------------------------- UBIFS rootfs
├─ rootfs_uclibc.tgz  ------------------------------------------- rootfs compiled with himix100

3.osdrv directory structure：
osdrv
├─Makefile ------------------------------ osdrv compile script
├─tools --------------------------------- directory of all tools
│  ├─board ------------------------------ A variety of single-board tools
│  │  ├─ethtools ------------------------ ethtools tools
│  │  ├─reg-tools-1.0.0 ----------------- tools for accessing memory space and io space
│  │  ├─mtd-utils ----------------------- tool to read and write flash nude
│  │  ├─udev-167 ------------------------ udev toolset
│  │  ├─gdb ----------------------------- gdb tools
│  │  └─e2fsprogs ----------------------- e2fsprogs tools
│  └─pc ------------------------------------- tools used on PC
│      ├─jffs2_tool-------------------------- tools for making jffs2 file system
│      ├─cramfs_tool ------------------------ tools for making cramfs file system
│      ├─squashfs4.3 ------------------------ tools for making mksquashfs file system
│      ├─nand_production -------------------- nand Production tools
│      ├─lzma_tool -------------------------- lzma compress tool
│      ├─zlib ------------------------------- zlib tool
│      ├─mkyaffs2image -- ------------------- tools for making yaffs2 file system
│      └─uboot_tools ------------------------ tools for creating uboot image,xls files,ddr initialization script and reg_info.bin making tool
├─pub --------------------------------------- output directory
│  ├─image_uclibc ---------------------------- images compiled with himix100: uboot,uImage and images of filesystem
│  ├─bin ------------------------------------ tools not placed in the rootfs
│  │  ├─pc ---------------------------------- tools used on the PC
│  │  ├─board_uclibc ------------------------- board tools compiled with himix100
│  └─rootfs_uclibc.tgz ----------------------- rootfs compiled with himix100
├─opensource--------------------------------- A variety of opensource code
│  ├─busybox -------------------------------- busybox source code
│  ├─uboot ---------------------------------- uboot source code
│  └─kernel --------------------------------- kernel source code
└─rootfs_scripts ---------------------------- scripts to generate rootfs directory

4.Note
(1)Executable files in Linux may become non-executable after copying them to somewhere else under Windows, and result in souce code cannot be compiled. Many symbolic link files will be generated in the souce package after compiling the u-boot or the kernel. The volume of the source package will become very big, because all the symbolic link files are turning out to be real files in Windows. So, DO NOT copy source package in Windows.
(2)If a tool chain needs to be replaced by the other, remove the original compiled files compiled with the former tool chain, and then replace the compiler tool chain with the other. 
(3) compile single-board code
    a.The chip has floating-point unit and neon. The library provided by the file system is a library with hard soft floating-points and neon. Therefore, add the following options in Makefile when compiling the single-board code.
For example:
For A7
    CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fno-aggressive-loop-optimizations
    CXXFlAGS +=-mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fno-aggressive-loop-optimizations
Among these, CXXFlAGS may be different according to the specific name in user's Makefile. For example, it may be changed to CPPFLAGS.
