##########################################################################################
#	prepare param
##########################################################################################
export OSDRV_DIR=$(shell pwd)
export OSDRV_CROSS=arm-himix100-linux
export CHIP=hi3559v200
export ARCH=arm
export CROSS_COMPILE=$(OSDRV_CROSS)-
export OSDRV_CROSS_CFLAGS
MP_TYPE=amp
BOOT_MEDIA=spi
CFG_VERSION?=release

ifneq ($(BOOT_MEDIA),spi)
ifneq ($(BOOT_MEDIA),nand)
ifneq ($(BOOT_MEDIA),emmc)
$(error you must set valid BOOT_MEDIA:spi,nand,emmc!)
endif
endif
endif
ifneq ($(MP_TYPE),amp)
$(error you must set valid MP_TYPE:amp!)
endif
ifneq ($(CHIP),hi3559v200)
ifneq ($(CHIP),hi3556v200)
$(error you must set valid CHIP:hi3559v200,hi3556v200!)
endif
endif
ifeq ($(OSDRV_CROSS), )
$(error you must set OSDRV_CROSS first!)
endif
ifeq ($(OSDRV_CROSS),arm-himix100-linux)
LIB_TYPE:=uclibc
BUSYBOX_CFG:=config_v100_a7_softfp_neon
TOOLCHAIN_DIR:=arm-himix100-linux
TOOLCHAINI_VERSION:=
RUNTIME_LIB:=runtime_uclibc
CROSS_SPECIFIED:=y
endif

ifeq ($(OSDRV_CROSS),arm-himix200-linux)
LIB_TYPE:=glibc
BUSYBOX_CFG:=config_v200_a7_softfp_neon
TOOLCHAIN_DIR:=arm-himix200-linux
TOOLCHAINI_VERSION:=
RUNTIME_LIB:=runtime_glibc
CROSS_SPECIFIED:=y
endif

BUSYBOX_VER:=busybox-1.26.2
TOOLCHAIN_RUNTIME_LIB_C:=lib.tgz

TOOLCHAIN_RUNTIME_LIB:=a7_softfp_neon-vfpv4
OSDRV_CROSS_CFLAGS:=-mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -w

UBOOT_VER:=u-boot-2016.11
UBOOT_TAR:=$(UBOOT_VER).tgz
UBOOT:=u-boot-$(CHIP).bin
UBOOT_REG_BIN:=reg_info_$(CHIP).bin
ifeq ($(BOOT_MEDIA),emmc)
export UBOOT_CONFIG:=$(CHIP)_$(BOOT_MEDIA)_config
else
ifeq ($(BOOT_MEDIA),nand)
export UBOOT_CONFIG:=$(CHIP)_$(BOOT_MEDIA)_config
else
export UBOOT_CONFIG:=$(CHIP)_config
endif
endif

ifeq ($(MP_TYPE),amp)
KERNEL_VER:=linux-4.9.y
else
KERNEL_VER:=linux-4.9.y-$(MP_TYPE)
endif
KERNEL_FULL_VER:=linux-4.9.37
KERNEL_TAR:=$(KERNEL_FULL_VER).tar.gz
KERNEL_PATCH:= $(KERNEL_FULL_VER).patch

UIMAGE:=uImage_$(CHIP)_$(MP_TYPE)
ifeq ($(BOOT_MEDIA),emmc)
export KERNEL_CFG:=$(CHIP)_$(MP_TYPE)_emmc_defconfig
else
ifeq ($(BOOT_MEDIA),nand)
export KERNEL_CFG:=$(CHIP)_$(MP_TYPE)_spinand_defconfig
else
export KERNEL_CFG:=$(CHIP)_$(MP_TYPE)_defconfig
endif
endif
export PUB_IMAGE:=$(CHIP)_$(BOOT_MEDIA)_$(MP_TYPE)_image_$(LIB_TYPE)

EXT4_TOOL:=make_ext4fs
EXT4_IMAGE_BIN:=rootfs_$(CHIP)_96M.ext4
export PUB_BOARD:=board_$(LIB_TYPE)
export PUB_ROOTFS:=rootfs_$(LIB_TYPE)

JFFS2_IMAGE_BIN_64K:=rootfs_$(CHIP)_64k.jffs2
JFFS2_IMAGE_BIN_128K:=rootfs_$(CHIP)_128k.jffs2
JFFS2_IMAGE_BIN_256K:=rootfs_$(CHIP)_256k.jffs2

YAFFS_TOOL:=mkyaffs2image100
YAFFS2_IMAGE_BIN_2K_4BIT:=rootfs_$(CHIP)_2k_4bit.yaffs2
YAFFS2_IMAGE_BIN_2K_24BIT:=rootfs_$(CHIP)_2k_24bit.yaffs2
YAFFS2_IMAGE_BIN_4K_4BIT:=rootfs_$(CHIP)_4k_4bit.yaffs2
YAFFS2_IMAGE_BIN_4K_24BIT:=rootfs_$(CHIP)_4k_24bit.yaffs2

HIREGBING_PACKAGE_VER:=hiregbin-v5.0.1

REGBIN_XLSM ?=
TARGET_XLSM := $(REGBIN_XLSM)

ifeq ($(TARGET_XLSM),)
ifeq ($(CHIP), hi3559v200)
TARGET_XLSM:=Hi3559V200-DMEB_6L_T-DDR3_1800M_512MB_16bitx2-A7_900M-SYSBUS_300M.xlsm
endif
ifeq ($(CHIP), hi3556v200)
TARGET_XLSM:=Hi3556V200-DMEB-DDR3_1800M_16bit_128MB-A7_900M-SYSBUS_300M.xlsm
endif
endif

TOOLCHAIN_FILE:= $(shell which $(OSDRV_CROSS)-gcc )
TOOLCHAIN_DIR:=$(shell dirname $(shell dirname $(TOOLCHAIN_FILE)))
RUNTIMELIB_DIR=$(shell dirname $(TOOLCHAIN_DIR))/$(OSDRV_CROSS)/$(RUNTIME_LIB)
##########################################################################################
#	set task
##########################################################################################
ifeq ($(CROSS_SPECIFIED),y)
ifeq ($(MP_TYPE),amp)
all: prepare hiboot hikernel hiliteos hibusybox hiliteos_sample hiipcm hirootfs_build
clean: hiboot_dirclean hikernel_dirclean \
       hibusybox_dirclean hipctools_clean hiboardtools_clean hirootfs_clean hiipcm_clean hiliteos_dirclean
else
all: prepare hiboot hikernel hirootfs_prepare hibusybox \
	hipctools hiboardtools hirootfs_build
clean: hiboot_dirclean hikernel_dirclean \
       hibusybox_dirclean hipctools_clean hiboardtools_clean hirootfs_clean
endif
notools: hiboot hikernel hinotools_prepare hirootfs_notools_build
distclean:clean pub_clean
endif

a:=$(shell $(OSDRV_CROSS)-gcc --version)
b:=$(findstring $(TOOLCHAINI_VERSION),$(a))
c:= $(word 2, $(a))
##########################################################################################
#task [0]	prepare
##########################################################################################
prepare:
	mkdir $(OSDRV_DIR)/pub/$(PUB_IMAGE) -p
	chmod +x $(OSDRV_DIR)/osdrv_mem_cfg.sh
	mkdir $(OSDRV_DIR)/pub/bin/$(PUB_BOARD) -p
	mkdir $(OSDRV_DIR)/pub/bin/pc -p
	mkdir $(OSDRV_DIR)/pub/ko -p

##########################################################################################
#	prepare hiregbin tools
##########################################################################################
check_hiregbin:
HIREGBING_PACKAGE = $(shell find $(OSDRV_DIR)/tools/pc/uboot_tools/ \
					-name "$(HIREGBING_PACKAGE_VER).tgz")
#$(warning $(HIREGBING_PACKAGE))

ifeq ($(HIREGBING_PACKAGE), )
$(warning "---------------------------------------------------------------------")
$(warning "     Cannot found the $(HIREGBING_PACKAGE_VER).tgz package !!                 ")
$(warning "---------------------------------------------------------------------")
$(error )
endif

##########################################################################################
#	hiregbin_prepare
##########################################################################################
hiregbin_prepare:
	@echo "---------task [0]    hiregbin_prepare"
	tar xzf $(HIREGBING_PACKAGE) -C $(OSDRV_DIR)/tools/pc/uboot_tools
	chmod 777 $(OSDRV_DIR)/tools/pc/uboot_tools/$(HIREGBING_PACKAGE_VER)/hiregbin
	cp $(OSDRV_DIR)/tools/pc/uboot_tools/$(TARGET_XLSM) $(OSDRV_DIR)/tools/pc/uboot_tools/$(HIREGBING_PACKAGE_VER)
	pushd $(OSDRV_DIR)/tools/pc/uboot_tools/$(HIREGBING_PACKAGE_VER);./hiregbin $(TARGET_XLSM) $(UBOOT_REG_BIN);popd
	mv $(OSDRV_DIR)/tools/pc/uboot_tools/$(HIREGBING_PACKAGE_VER)/$(UBOOT_REG_BIN) $(OSDRV_DIR)/tools/pc/uboot_tools
	rm $(OSDRV_DIR)/tools/pc/uboot_tools/$(HIREGBING_PACKAGE_VER) -rf

##########################################################################################
hinotools_prepare:
	pushd $(OSDRV_DIR)/pub;tar xzf $(PUB_ROOTFS).tgz;popd
##########################################################################################
#task [1]	build uboot
##########################################################################################
hiboot: prepare hiregbin_prepare
	@echo "---------task [1]	build boot"
ifneq ($(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER), $(wildcard $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER)))
	pushd $(OSDRV_DIR)/opensource/uboot;tar xzf $(UBOOT_TAR) -C .;popd
	find $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER) | xargs touch
endif
	make -C $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER) ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- $(UBOOT_CONFIG)
	./osdrv_mem_cfg.sh uboot $(CHIP)
	pushd $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER);make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- -j 20 >/dev/null;popd
	cp $(OSDRV_DIR)/tools/pc/uboot_tools/$(UBOOT_REG_BIN) $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER)/.reg
	make -C $(OSDRV_DIR)/tools/pc/hi_gzip/
	cp $(OSDRV_DIR)/tools/pc/hi_gzip/bin/gzip $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER)/arch/arm/cpu/armv7/$(CHIP)/hw_compressed -rf
	chmod +x $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER)/arch/arm/cpu/armv7/$(CHIP)/hw_compressed/gzip
	pushd $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER);make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- u-boot-z.bin >/dev/null;popd
	cp $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER)/u-boot-$(CHIP).bin $(OSDRV_DIR)/pub/$(PUB_IMAGE)

hiboot_dirclean:
	rm $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER) -rf
	rm $(OSDRV_DIR)/tools/pc/uboot_tools/$(UBOOT_REG_BIN) -rf
	make -C $(OSDRV_DIR)/tools/pc/hi_gzip/ clean
	rm -rf $(OSDRV_DIR)/tools/pc/uboot_tools/*.bin

hiboot_clean:
	pushd $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER);make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- u-boot-z.clean
	make -C $(OSDRV_DIR)/opensource/uboot/$(UBOOT_VER) ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- distclean
	make -C $(OSDRV_DIR)/tools/pc/hi_gzip/ clean

##########################################################################################
#	check_kernel
##########################################################################################
check_kernel:
PACKAGE=$(notdir $(wildcard $(OSDRV_DIR)/opensource/kernel/*.tar.xz))
ifeq ($(PACKAGE), $(KERNEL_TAR))
result=$(shell xz -d $(OSDRV_DIR)/opensource/kernel/$(PACKAGE)\
	   && echo "pass" || "fail")
$(warning $(result))
endif
PACKAGE = $(shell find $(OSDRV_DIR)/opensource/kernel/ \
					-name "$(KERNEL_FULL_VER).tar*")

ifeq ($(PACKAGE), )
$(warning "---------------------------------------------------------------------")
$(warning "     Cannot found the $(KERNEL_FULL_VER) kernel source file                ")
$(warning "   Please download the compression package from www.kernel.org  	    ")
$(warning "---------------------------------------------------------------------")
$(error )
endif

##########################################################################################
#task [2]	build kernel
##########################################################################################
hikernel: prepare check_kernel
	@echo "---------task [2] build kernel"
ifneq ($(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER), $(wildcard $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER)))
	tar xf $(PACKAGE) -C $(OSDRV_DIR)/opensource/kernel/
	mv $(OSDRV_DIR)/opensource/kernel/$(KERNEL_FULL_VER) $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER)
	pushd $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER);patch -p1 < ../$(KERNEL_PATCH);popd
	find $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER) | xargs touch
endif
	./osdrv_mem_cfg.sh linux $(CHIP)
	make -C $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER) ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- $(KERNEL_CFG)

	cd $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER);\
		make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- uImage -j 20 >/dev/null;cd -
	cd $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER);\
		make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- modules -j 20 >/dev/null;cd -

	cd $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER); \
	find drivers/usb -name *.ko -exec cp -rf {} $(OSDRV_DIR)/pub/ko \; ;\
	find drivers/mmc -name *.ko -exec cp -rf {} $(OSDRV_DIR)/pub/ko \; ;\
	cd -;

	cp $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER)/arch/arm/boot/uImage $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(UIMAGE)

hikernel_dirclean:
	rm $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER) -rf

hikernel_clean:
	make -C $(OSDRV_DIR)/opensource/kernel/$(KERNEL_VER) ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- distclean

##########################################################################################
#task [3]	prepare rootfs
##########################################################################################
hirootfs_prepare: prepare
	@echo "---------task [3] prepare rootfs "
	@if [ ! -d $(OSDRV_DIR)/pub/$(PUB_ROOTFS) ]; then \
		tar xzf $(OSDRV_DIR)/rootfs_scripts/rootfs.tgz -C $(OSDRV_DIR)/pub; \
		mv -f $(OSDRV_DIR)/pub/rootfs $(OSDRV_DIR)/pub/$(PUB_ROOTFS); \
	fi
	tar xzf $(RUNTIMELIB_DIR)/$(TOOLCHAIN_RUNTIME_LIB)/$(TOOLCHAIN_RUNTIME_LIB_C) -C $(OSDRV_DIR)/pub/$(PUB_ROOTFS)
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);$(CROSS_COMPILE)strip -d ./lib/*;popd

##########################################################################################
#task [4]	build busybox
##########################################################################################
hibusybox: hirootfs_prepare
	@echo "---------task [4] build busybox "
ifneq ($(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER), $(wildcard $(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER)))
	tar xzf $(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER).tgz -C $(OSDRV_DIR)/opensource/busybox
	find $(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER) | xargs touch
endif
ifeq ($(CFG_VERSION), release)
	cp $(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER)/$(BUSYBOX_CFG)_full \
		$(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER)/.config
else
	cp $(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER)/$(BUSYBOX_CFG)_full \
		$(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER)/.config
endif
	pushd $(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER)/;make -j 20 >/dev/null;popd
	make -C $(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER) install
	cp -af $(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER)/_install/* $(OSDRV_DIR)/pub/$(PUB_ROOTFS)

hibusybox_clean:
	make -C $(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER) distclean

hibusybox_dirclean:
	rm $(OSDRV_DIR)/opensource/busybox/$(BUSYBOX_VER) -rf

##########################################################################################
#task [5]	build pc tools
##########################################################################################
hipctools: prepare
	@echo "---------task [5] build tools which run on pc"
	make -C $(OSDRV_DIR)/tools/pc/mkyaffs2image/
	cp $(OSDRV_DIR)/tools/pc/mkyaffs2image/bin/$(YAFFS_TOOL) $(OSDRV_DIR)/pub/bin/pc
	make -C $(OSDRV_DIR)/tools/pc/jffs2_tool/
	cp $(OSDRV_DIR)/tools/pc/jffs2_tool/mkfs.jffs2 $(OSDRV_DIR)/pub/bin/pc
	cp $(OSDRV_DIR)/tools/pc/jffs2_tool/mkfs.ubifs $(OSDRV_DIR)/pub/bin/pc
	cp $(OSDRV_DIR)/tools/pc/jffs2_tool/ubinize $(OSDRV_DIR)/pub/bin/pc
	make -C $(OSDRV_DIR)/tools/pc/cramfs_tool/
	cp $(OSDRV_DIR)/tools/pc/cramfs_tool/mkfs.cramfs $(OSDRV_DIR)/pub/bin/pc
	make -C $(OSDRV_DIR)/tools/pc/squashfs4.3/
	cp $(OSDRV_DIR)/tools/pc/squashfs4.3/mksquashfs $(OSDRV_DIR)/pub/bin/pc
	make -C $(OSDRV_DIR)/tools/pc/lzma_tool/
	cp $(OSDRV_DIR)/tools/pc/lzma_tool/lzma $(OSDRV_DIR)/pub/bin/pc
	make -C $(OSDRV_DIR)/tools/pc/ext4_utils/
	cp $(OSDRV_DIR)/tools/pc/ext4_utils/bin/$(EXT4_TOOL) $(OSDRV_DIR)/pub/bin/pc
	make -C $(OSDRV_DIR)/tools/pc/hi_gzip/
	cp $(OSDRV_DIR)/tools/pc/hi_gzip/bin/hi_gzip $(OSDRV_DIR)/pub/bin/pc
	cp $(OSDRV_DIR)/tools/pc/hi_gzip/bin/gzip $(OSDRV_DIR)/pub/bin/pc
	make -C $(OSDRV_DIR)/tools/pc/nand_production/fmc_nand_ecc_product_v100
	cp $(OSDRV_DIR)/tools/pc/nand_production/fmc_nand_ecc_product_v100/nand_product $(OSDRV_DIR)/pub/bin/pc

hipctools_clean:
	make -C $(OSDRV_DIR)/tools/pc/mkyaffs2image clean
	make -C $(OSDRV_DIR)/tools/pc/jffs2_tool distclean
	make -C $(OSDRV_DIR)/tools/pc/cramfs_tool distclean
	make -C $(OSDRV_DIR)/tools/pc/squashfs4.3 distclean
	make -C $(OSDRV_DIR)/tools/pc/lzma_tool distclean
	make -C $(OSDRV_DIR)/tools/pc/ext4_utils clean
	make -C $(OSDRV_DIR)/tools/pc/hi_gzip clean
	make -C $(OSDRV_DIR)/tools/pc/nand_production/fmc_nand_ecc_product_v100 clean


##########################################################################################
#task [6]	build board tools
##########################################################################################
hiboardtools: hirootfs_prepare
	@echo "---------task [6] build tools which run on board "
	make -C $(OSDRV_DIR)/tools/board/e2fsprogs
	cp -af $(OSDRV_DIR)/tools/board/e2fsprogs/bin/* $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	make -C $(OSDRV_DIR)/tools/board/reg-tools-1.0.0
	cp -af $(OSDRV_DIR)/tools/board/reg-tools-1.0.0/bin/* $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	make -C $(OSDRV_DIR)/tools/board/udev-167
	cp -af $(OSDRV_DIR)/tools/board/udev-167/rootfs/. $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/
	make -C $(OSDRV_DIR)/tools/board/gdb
	cp $(OSDRV_DIR)/tools/board/gdb/gdb-$(OSDRV_CROSS) $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)
	make -C $(OSDRV_DIR)/tools/board/mtd-utils/
	cp $(OSDRV_DIR)/tools/board/mtd-utils/bin/* $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)
	cp $(OSDRV_DIR)/tools/board/reg-tools-1.0.0/bin/himm $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)
	cp $(OSDRV_DIR)/tools/board/reg-tools-1.0.0/bin/himc $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)
	cp $(OSDRV_DIR)/tools/board/reg-tools-1.0.0/bin/himd $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)
	cp $(OSDRV_DIR)/tools/board/reg-tools-1.0.0/bin/himd.l $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)

hiboardtools_clean:
	make -C $(OSDRV_DIR)/tools/board/e2fsprogs distclean
	make -C $(OSDRV_DIR)/tools/board/reg-tools-1.0.0 clean
	make -C $(OSDRV_DIR)/tools/board/udev-167 clean
	make -C $(OSDRV_DIR)/tools/board/gdb distclean
	make -C $(OSDRV_DIR)/tools/board/mtd-utils distclean

##########################################################################################
#task [7]	build rootfs
##########################################################################################
hirootfs_build: hirootfs_prepare hibusybox hiboardtools hipctools hirootfs_notools_build
hirootfs_notools_build:
	@echo "---------task [7] build rootfs"
	chmod 777 $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)/*
	chmod 777 $(OSDRV_DIR)/pub/bin/pc/*
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/dev/* -rf
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin/himm -rf
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin/himc -rf
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin/himd -rf
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin/himd.l -rf
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);ln -s sbin/init init;popd
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS); chmod 750 * -R; chmod 750 usr/* -R; popd
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS); chmod -w usr/bin -R; chmod -w usr/sbin -R; chmod -w sbin -R; popd
	cp $(OSDRV_DIR)/pub/ko/*.ko $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/komod -rf

	find $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ -name '*svn' | xargs rm -rf
ifeq ($(BOOT_MEDIA),spi)
	pushd $(OSDRV_DIR)/pub/bin/pc;./mkfs.jffs2 -d $(OSDRV_DIR)/pub/$(PUB_ROOTFS) -l -e 0x40000 -o $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(JFFS2_IMAGE_BIN_256K);popd
	pushd $(OSDRV_DIR)/pub/bin/pc;./mkfs.jffs2 -d $(OSDRV_DIR)/pub/$(PUB_ROOTFS) -l -e 0x20000 -o $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(JFFS2_IMAGE_BIN_128K);popd
	pushd $(OSDRV_DIR)/pub/bin/pc;./mkfs.jffs2 -d $(OSDRV_DIR)/pub/$(PUB_ROOTFS) -l -e 0x10000 -o $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(JFFS2_IMAGE_BIN_64K);popd
endif
ifeq ($(BOOT_MEDIA),emmc)
	pushd $(OSDRV_DIR)/pub/bin/pc;./$(EXT4_TOOL) -l 96M -s $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(EXT4_IMAGE_BIN) $(OSDRV_DIR)/pub/$(PUB_ROOTFS);popd
endif
ifeq ($(BOOT_MEDIA),nand)
	pushd $(OSDRV_DIR)/pub/bin/pc;./$(YAFFS_TOOL) $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(YAFFS2_IMAGE_BIN_2K_4BIT) 1 2;popd
	chmod 644 $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(YAFFS2_IMAGE_BIN_2K_4BIT)
	pushd $(OSDRV_DIR)/pub/bin/pc;./$(YAFFS_TOOL) $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(YAFFS2_IMAGE_BIN_2K_24BIT) 1 4;popd
	chmod 644 $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(YAFFS2_IMAGE_BIN_2K_24BIT)
	pushd $(OSDRV_DIR)/pub/bin/pc;./$(YAFFS_TOOL) $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(YAFFS2_IMAGE_BIN_4K_4BIT) 2 2;popd
	chmod 644 $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(YAFFS2_IMAGE_BIN_4K_4BIT)
	pushd $(OSDRV_DIR)/pub/bin/pc;./$(YAFFS_TOOL) $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(YAFFS2_IMAGE_BIN_4K_24BIT) 2 4;popd
	chmod 644 $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(YAFFS2_IMAGE_BIN_4K_24BIT)

	cp $(OSDRV_DIR)/tools/pc/ubi_sh/mkubiimg.sh $(OSDRV_DIR)/pub/$(PUB_IMAGE)
	chmod +x $(OSDRV_DIR)/pub/$(PUB_IMAGE)/mkubiimg.sh
	# build the pagesize = 2k, blocksize = 128k, part_size = 32M #
	pushd $(OSDRV_DIR)/pub/$(PUB_IMAGE);./mkubiimg.sh $(CHIP) 2k 128k $(OSDRV_DIR)/pub/$(PUB_ROOTFS) 32M $(OSDRV_DIR)/pub/bin/pc;popd
	# build the pagesize = 4k, blocksize = 256k, part_size = 50M #
	pushd $(OSDRV_DIR)/pub/$(PUB_IMAGE);./mkubiimg.sh $(CHIP) 4k 256k $(OSDRV_DIR)/pub/$(PUB_ROOTFS) 50M $(OSDRV_DIR)/pub/bin/pc;popd
	rm $(OSDRV_DIR)/pub/$(PUB_IMAGE)/mkubiimg.sh
endif
	pushd $(OSDRV_DIR)/pub;tar czf $(PUB_ROOTFS).tgz $(PUB_ROOTFS);popd
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS); chmod +w usr/bin -R; chmod +w usr/sbin -R; chmod +w sbin -R; popd
	rm -rf $(OSDRV_DIR)/pub/$(PUB_ROOTFS);
	@echo "---------finish osdrv work"

hirootfs_clean:
ifeq ($(OSDRV_DIR)/pub/$(PUB_ROOTFS), $(wildcard $(OSDRV_DIR)/pub/$(PUB_ROOTFS)))
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS); chmod +w usr/bin -R; chmod +w usr/sbin -R; chmod +w sbin -R; popd
endif
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ -rf

##########################################################################################
#task [9] ipcm build
##########################################################################################
hiipcm: hirootfs_prepare hiliteos
	@echo -e $(INFO)"------- task [9] build ipcm"$(DONE)
	rm -rf  $(OSDRV_DIR)/components/ipcm/ipcm
	pushd $(OSDRV_DIR)/components/ipcm;tar xzf ipcm.tgz -C .;popd
	./osdrv_mem_cfg.sh ipcm $(CHIP)
	pushd  $(OSDRV_DIR)/components/ipcm/ipcm; make PLATFORM=$(CHIP) CFG=$(CHIP)_a7_liteos_config clean; popd
	pushd  $(OSDRV_DIR)/components/ipcm/ipcm; make PLATFORM=$(CHIP) CFG=$(CHIP)_a7_liteos_config all; popd
	cp $(OSDRV_DIR)/components/ipcm/ipcm/out/node_1/*.a $(OSDRV_DIR)/platform/liteos/out/$(CHIP)/lib
	pushd  $(OSDRV_DIR)/components/ipcm/ipcm; make PLATFORM=$(CHIP) CFG=$(CHIP)_a7_linux_config clean; popd
	pushd  $(OSDRV_DIR)/components/ipcm/ipcm; make PLATFORM=$(CHIP) CFG=$(CHIP)_a7_linux_config all; popd
	cp $(OSDRV_DIR)/components/ipcm/ipcm/out/node_0/sharefs $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	cp $(OSDRV_DIR)/components/ipcm/ipcm/out/node_0/virt-tty $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	cp $(OSDRV_DIR)/components/ipcm/ipcm/out/node_0/*.ko $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/komod
hiipcm_clean:
	rm -rf $(OSDRV_DIR)/components/ipcm/ipcm
##########################################################################################
#task [10]	clean pub
##########################################################################################
pub_clean:
	rm $(OSDRV_DIR)/pub/* -rf

##########################################################################################
#task [11]  a7 liteos build
##########################################################################################
hiliteos: prepare
ifneq ($(OSDRV_DIR)/platform/liteos, $(wildcard $(OSDRV_DIR)/platform/liteos))
	pushd $(OSDRV_DIR)/platform;tar xzf liteos.tgz;popd
else
	@echo -e $(NOTE)"Attention:"$(DONE)
	@echo -e $(NOTE)"you should compile ipcm for new libs!"$(DONE)
endif
	./osdrv_mem_cfg.sh liteos $(CHIP)
ifeq ($(IRQ_BIND), liteos)
	pushd $(OSDRV_DIR)/platform/liteos;cp ./tools/build/config/$(CHIP)_defconfig .config;make IRQ_BIND=liteos -j 20 >/dev/null;popd
else
	pushd $(OSDRV_DIR)/platform/liteos;cp ./tools/build/config/$(CHIP)_defconfig .config;make -j 20 >/dev/null;popd
endif

hiliteos_clean:
	make -C $(OSDRV_DIR)/platform/liteos clean
hiliteos_distclean:
	make -C $(OSDRV_DIR)/platform/liteos clean
hiliteos_dirclean:
	#rm -rf $(OSDRV_DIR)/platform/liteos

##########################################################################################
#task [12]  a7 liteos sample.bin build
##########################################################################################
hiliteos_sample: prepare hiliteos
	pushd $(OSDRV_DIR)/platform/liteos/sample/sample_osdrv; \
	make clean >/dev/null; \
	make >/dev/null; \
	cp sample.bin $(OSDRV_DIR)/pub/$(PUB_IMAGE); \
	popd
	pushd $(OSDRV_DIR)/tools/pc/hi_gzip/bin/; \
	./hi_gzip $(OSDRV_DIR)/pub/$(PUB_IMAGE)/sample.bin $(OSDRV_DIR)/pub/$(PUB_IMAGE)/sample_gzip.bin; \
	popd

