# Base Configure
CUR_DIR  := $(shell pwd)
PDT_ROOT ?= $(CUR_DIR)/..
include $(PDT_ROOT)/build/base.mak

# OSDRV
OSDRV_PUB_PC     := $(OSDRV_PUB_PC_BIN_PATH)
OSDRV_PUB_IMG    := $(OSDRV_PUB_IMG_PATH)
JFFS2_BIN_NAME   ?= mkfs.jffs2
SQUASH_BIN_NAME  := mksquashfs
UBIFS_BIN_NAME   := mkfs.ubifs
UBINIZE_BIN_NAME := ubinize
YAFFS_BIN_NAME   := mkyaffs2image100

# uboot
UBOOT_MINI_IMG   := mini-boot.bin
UBOOT_IMG_PKT    := u-boot.bin

# kernel
KERNEL_MINI_IMG  := miniuImage
KERNEL_IMG_PKT   := uImage

# Rootfs
ROOTFS_INIT_PKT         := $(OSDRV_ROOTFS_PKT)
ROOTFS_PATH             := $(CUR_DIR)/rootfs
ROOTFS_FAST_PRIV        := $(CUR_DIR)/mini_rootfs_priv
ROOTFS_PRIV_PATH        := $(CUR_DIR)/rootfs_priv
ROOTFS_JFFS2_IMG        := $(CUR_DIR)/rootfs.jffs2
ROOTFS_SQUASH_IMG       := $(CUR_DIR)/rootfs.squashfs
ROOTFS_YAFFS_IMG        := $(CUR_DIR)/rootfs.yaffs
ROOTFS_UBIFS_IMG        := $(CUR_DIR)/rootfs.ubifs
ROOTFS_UBIFS_IMG_TEMP   := $(CUR_DIR)/rootfs_temp.ubifs

# Appfs(Including Param)
APPFS_PATH              := $(CUR_DIR)/appfs
APPFS_PRIV_PATH         := $(CUR_DIR)/appfs_priv
APPFS_JFFS2_IMG         := $(CUR_DIR)/appfs.jffs2
APPFS_YAFFS_IMG         := $(CUR_DIR)/appfs.yaffs
APPFS_UBIFS_IMG         := $(CUR_DIR)/appfs.ubifs
APPFS_UBIFS_IMG_TEMP    := $(CUR_DIR)/appfs_temp.ubifs

# Burn Config
BURN_CFG_PATH           := $(CUR_DIR)/configs/$(CFG_CHIP_TYPE)
BURN_CONFIG             := $(BURN_CFG_PATH)/config_$(CFG_ROOTFS_TYPE)_$(CFG_APPFS_TYPE)_$(FLASH_TYPE)
BURN_XML                := $(BURN_CFG_PATH)/config_$(CFG_ROOTFS_TYPE)_$(CFG_APPFS_TYPE)_$(FLASH_TYPE).xml
BURN_AI                 := $(BURN_CFG_PATH)/config_$(CFG_ROOTFS_TYPE)_$(CFG_APPFS_TYPE)_$(FLASH_TYPE).ai
CFG_LITEOS_TEXT_BASE    := $(shell awk 'BEGIN{printf("0x%x",'$(CONFIG_MEM_HUAWEILITE_SYS_BASE)'+'$(OSDRV_LITEOS_TEXT_OFFSET)')}')
CFG_LITEOS_READ_ADDR    := $(shell awk 'BEGIN{printf("0x%x",'$(CONFIG_MEM_HUAWEILITE_MMZ_BASE)'+'0x1000000')}')
CFG_LINUX_BOOT_ADDR     := $(shell awk 'BEGIN{printf("0x%x",'$(CONFIG_MEM_LINUX_SYS_BASE)'+'0x2000000')}')

# Memory Configure
CFG_LINUX_SYS_LEN = $(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_SYS_SIZE)'/0x100000)}')
CFG_LINUX_MMZ_ANONYMOUS_LEN = $(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_MMZ_ANONYMOUS_SIZE)'/0x400)}')
CFG_LINUX_MMZ_HIGO_BASE = $(shell awk 'BEGIN{printf("0x%x",'$(CONFIG_MEM_LINUX_MMZ_BASE)'+'$(CONFIG_MEM_LINUX_MMZ_ANONYMOUS_SIZE)')}')
CFG_LINUX_MMZ_HIGO_LEN = $(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_MMZ_HIGO_SIZE)'/0x400)}')
CFG_HUAWEILITE_MMZ_LEN = $(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_HUAWEILITE_MMZ_SIZE)'/0x400)}')

# Compile PHONY

.PHONY: all install clean distclean
default: all

ifneq ($(AMP_TYPE), liteos)
all: rootfs appfs install
else
all: appfs install
endif

install: common_install
	@cp -f $(OSDRV_PUB_IMG)/u-boot-$(CFG_SUBCHIP_TYPE).bin $(PDT_OUT_BURN)/$(UBOOT_IMG_PKT);
ifneq ($(AMP_TYPE), liteos)
	@cp -f $(OSDRV_PUB_IMG_BIN) $(PDT_OUT_BURN)/$(KERNEL_IMG_PKT)
	@if [ -f $(OSDRV_PUB_IMG_BIN)_zip ];then cp -f $(OSDRV_PUB_IMG_BIN)_zip $(PDT_OUT_BURN)/$(KERNEL_IMG_PKT)_zip;fi
endif

common_install:
ifeq ($(CONFIG_ROOTFS_JFFS2),y)
	@if [ -f $(ROOTFS_JFFS2_IMG) ];then cp -f $(ROOTFS_JFFS2_IMG) $(PDT_OUT_BURN);fi
	@if [ -f $(APPFS_JFFS2_IMG) ];then cp -f $(APPFS_JFFS2_IMG) $(PDT_OUT_BURN);fi
else ifeq ($(CONFIG_ROOTFS_SQUASHFS),y)
	@if [ -f $(ROOTFS_SQUASH_IMG) ];then cp -f $(ROOTFS_SQUASH_IMG) $(PDT_OUT_BURN);fi
	@if [ -f $(APPFS_JFFS2_IMG) ];then cp -f $(APPFS_JFFS2_IMG) $(PDT_OUT_BURN);fi
else ifeq ($(CONFIG_ROOTFS_UBIFS),y)
	@if [ -f $(ROOTFS_UBIFS_IMG) ];then cp -f $(ROOTFS_UBIFS_IMG) $(PDT_OUT_BURN);fi
	@if [ -f $(APPFS_UBIFS_IMG) ];then cp -f $(APPFS_UBIFS_IMG) $(PDT_OUT_BURN);fi
else ifeq ($(CONFIG_ROOTFS_YAFFS),y)
	@if [ -f $(ROOTFS_YAFFS_IMG) ];then cp -f $(ROOTFS_YAFFS_IMG) $(PDT_OUT_BURN);fi
	@if [ -f $(APPFS_YAFFS_IMG) ];then cp -f $(APPFS_YAFFS_IMG) $(PDT_OUT_BURN);fi
endif
	@if [ -f $(BURN_CONFIG) ];then cp -f $(BURN_CONFIG) $(PDT_OUT_BURN)/config;fi
	@sed -i 's/mem=.*M /mem=$(CFG_LINUX_SYS_LEN)M /g' $(PDT_OUT_BURN)/config
	@sed -i 's/linux_bootaddr/$(CFG_LINUX_BOOT_ADDR)/g' $(PDT_OUT_BURN)/config
	@sed -i 's/liteos_textbase/$(CFG_LITEOS_TEXT_BASE)/g' $(PDT_OUT_BURN)/config
	@sed -i 's/liteos_readaddr/$(CFG_LITEOS_READ_ADDR)/g' $(PDT_OUT_BURN)/config
	@if [ -f $(BURN_XML) ];then cp -f $(BURN_XML) $(PDT_OUT_BURN)/usb-burn.xml;fi
	@if [ -f $(BURN_AI) ];then cp -f $(BURN_AI) $(PDT_OUT_BURN)/eth-burn.ai;fi
	@if [ -f $(CUR_DIR)/rawparam ];then cp -f $(CUR_DIR)/rawparam $(PDT_OUT_BURN)/rawparam;fi
	@if [ -f $(CUR_DIR)/rawparam ];then cp -f $(CUR_DIR)/rawparam $(PDT_OUT_BURN)/rawparambak;fi
ifeq ($(CFG_DEBUG_IMAGE_MAP_ON),yes)
	@cp $(PDT_OUT_BIN)/slave/media_app.bin $(PDT_OUT_BURN) -f
	@cp $(PDT_OUT_BIN)/slave/media_app.map $(PDT_OUT_BURN) -f
endif

clean: rootfs_clean appfs_clean

distclean: clean
	@if [ -f $(ROOTFS_JFFS2_IMG) ];then rm -f $(ROOTFS_JFFS2_IMG);fi
	@if [ -f $(ROOTFS_SQUASH_IMG) ];then rm -f $(ROOTFS_SQUASH_IMG);fi
	@if [ -f $(ROOTFS_UBIFS_IMG) ];then rm -f $(ROOTFS_UBIFS_IMG);fi
	@if [ -f $(ROOTFS_UBIFS_IMG_TEMP) ];then rm -f $(ROOTFS_UBIFS_IMG_TEMP);fi
	@if [ -f $(ROOTFS_YAFFS_IMG) ];then rm -f $(ROOTFS_YAFFS_IMG);fi
	@if [ -f $(APPFS_JFFS2_IMG) ];then rm -f $(APPFS_JFFS2_IMG);fi
	@if [ -f $(APPFS_YAFFS_IMG) ];then rm -f $(APPFS_YAFFS_IMG);fi
	@if [ -f $(APPFS_UBIFS_IMG) ];then rm -f $(APPFS_UBIFS_IMG);fi
	@if [ -f $(APPFS_UBIFS_IMG_TEMP) ];then rm -f $(APPFS_UBIFS_IMG_TEMP);fi
	@if [ -f $(CUR_DIR)/rawparam ];then rm -f $(CUR_DIR)/rawparam;fi


rootfs: rootfs_prepare rootfs_install

rootfs_install:
	@chmod +x $(OSDRV_PUB_PC)/*
	@chmod +x $(CUR_DIR)/*
ifeq ($(CONFIG_ROOTFS_JFFS2),y)
	@$(OSDRV_PUB_PC)/$(JFFS2_BIN_NAME) -e 0x10000 -d $(ROOTFS_PATH)/ -o $(ROOTFS_JFFS2_IMG) -X lzma -x zlib -x rtime -m size
else ifeq ($(CONFIG_ROOTFS_SQUASHFS),y)
	@if [ -f $(ROOTFS_SQUASH_IMG) ];then rm -f $(ROOTFS_SQUASH_IMG);fi
	@$(OSDRV_PUB_PC)/$(SQUASH_BIN_NAME) $(ROOTFS_PATH)/ $(ROOTFS_SQUASH_IMG) -comp lzma
else ifeq ($(CONFIG_ROOTFS_UBIFS),y)
    ifeq ($(CONFIG_SPINOR),y)
	@$(OSDRV_PUB_PC)/$(UBIFS_BIN_NAME) -d $(ROOTFS_PATH)/ -m $(RAMPARAM_PAGESIZE) -o $(ROOTFS_UBIFS_IMG_TEMP) -e 65408  -c 128 -v
	@$(OSDRV_PUB_PC)/$(UBINIZE_BIN_NAME) -o $(ROOTFS_UBIFS_IMG) -m $(RAMPARAM_PAGESIZE) -p 64KiB $(CUR_DIR)/configs/ubi_rootfs.cfg -v
    else
	@$(OSDRV_PUB_PC)/$(UBIFS_BIN_NAME) -d $(ROOTFS_PATH)/ -m $(RAMPARAM_PAGESIZE) -o $(ROOTFS_UBIFS_IMG_TEMP) -e 126976 -c 60 -F -v
	@$(OSDRV_PUB_PC)/$(UBINIZE_BIN_NAME) -o $(ROOTFS_UBIFS_IMG) -m $(RAMPARAM_PAGESIZE) -p 128KiB $(CUR_DIR)/configs/ubi_rootfs.cfg -v
    endif
else ifeq ($(CONFIG_ROOTFS_YAFFS),y)
	@$(OSDRV_PUB_PC)/$(YAFFS_BIN_NAME) $(ROOTFS_PATH)/ $(ROOTFS_YAFFS_IMG) $(YAFFS_PAGESIZE_PARAM) $(YAFFS_ECCTYPE_PARAM)
endif
	@if [ -d $(ROOTFS_PATH) ];then chmod 750 $(ROOTFS_PATH)/ -R;fi

rootfs_prepare: rootfs_clean
	@tar -xf $(ROOTFS_INIT_PKT) -C $(CUR_DIR)
	@chmod 750 $(CUR_DIR)/$(basename $(notdir $(ROOTFS_INIT_PKT))) -R
	@mv $(CUR_DIR)/$(basename $(notdir $(ROOTFS_INIT_PKT))) $(ROOTFS_PATH)
	@chmod 750 $(ROOTFS_PATH)/ -R
	@if [ -f $(OSDRV_PUB_LINUX_BIN)/himm ];then cp -f $(OSDRV_PUB_LINUX_BIN)/himm $(ROOTFS_PATH)/bin/;fi
	@if [ -f $(OSDRV_PUB_LINUX_BIN)/himc ];then cp -f $(OSDRV_PUB_LINUX_BIN)/himc $(ROOTFS_PATH)/bin/;fi
	@if [ -f $(OSDRV_PUB_LINUX_BIN)/himd ];then cp -f $(OSDRV_PUB_LINUX_BIN)/himd $(ROOTFS_PATH)/bin/;fi
	@if [ -f $(OSDRV_PUB_LINUX_BIN)/himd.l ];then cp -f $(OSDRV_PUB_LINUX_BIN)/himd.l $(ROOTFS_PATH)/bin/;fi
	@cp $(ROOTFS_PRIV_PATH)/* $(ROOTFS_PATH) -ar
	@#cd $(LINUX_ROOT);cp -f `find drivers/usb/ -name *.ko` $(ROOTFS_PATH)/komod/;cd -
	@rm -f $(ROOTFS_PATH)/komod/*
	@if [ ! -d $(ROOTFS_PATH)/app ];then mkdir $(ROOTFS_PATH)/app/;fi
ifeq ($(CONFIG_ROOTFS_JFFS2),y)
	@sed -i 's;#mount -t jffs2 ;mount -t jffs2 ;g' $(ROOTFS_PATH)/etc/init.d/S10mpp
else ifeq ($(CONFIG_ROOTFS_UBIFS),y)
	@sed -i 's;#ubiattach ;ubiattach ;g' $(ROOTFS_PATH)/etc/init.d/S10mpp
	@sed -i 's;#mount -t ubifs ;mount -t ubifs ;g' $(ROOTFS_PATH)/etc/init.d/S10mpp
else ifeq ($(CONFIG_ROOTFS_YAFFS),y)
	@sed -i 's;#mount -t yaffs ;mount -t yaffs ;g' $(ROOTFS_PATH)/etc/init.d/S10mpp
endif
	@-find $(ROOTFS_PATH)/bin/ -name "mkfs.ext*" |xargs rm -f
	@cd $(ROOTFS_PATH)/bin/;rm -f udevadm udevd mke2fs;cd -
	@rm -f $(ROOTFS_PATH)/etc/init.d/S0*udev
ifneq ($(CONFIG_DEBUG),y)
	@rm $(ROOTFS_PATH)/bin/sharefs
endif
	@chmod 750 $(ROOTFS_PATH)/ -R
	@chmod -w $(ROOTFS_PATH)/usr/bin; chmod -w $(ROOTFS_PATH)/usr/sbin; chmod -w $(ROOTFS_PATH)/sbin
	@-find $(ROOTFS_PATH) -name "*.svn" |xargs rm -rf
	@-find $(ROOTFS_PATH) -name "*.map" |xargs rm -rf
	@mkdir -p $(PDT_OUT_BIN)/; cp -f $(OSDRV_PUB_GDB_FILE) $(PDT_OUT_BIN)/;

rootfs_clean:
	@if [ -d $(ROOTFS_PATH) ];then chmod 750 $(ROOTFS_PATH)/ -R; rm -rf $(ROOTFS_PATH);fi

appfs: appfs_prepare appfs_install

appfs_install:
ifeq ($(CONFIG_APPFS_JFFS2),y)
ifneq ($(AMP_TYPE), liteos)
	@$(OSDRV_PUB_PC)/$(JFFS2_BIN_NAME) -e 0x10000 -d $(APPFS_PATH)/ -o $(APPFS_JFFS2_IMG) -X lzma -x zlib -x rtime -m size
else
	@$(OSDRV_PUB_PC)/$(JFFS2_BIN_NAME) -e 0x10000 -d $(APPFS_PATH)/ -o $(APPFS_JFFS2_IMG)
endif
else ifeq ($(CONFIG_APPFS_UBIFS),y)
    ifeq ($(FLASH_TYPE),spinor)
	@$(OSDRV_PUB_PC)/$(UBIFS_BIN_NAME) -d $(APPFS_PATH)/ -m $(RAMPARAM_PAGESIZE) -o $(APPFS_UBIFS_IMG_TEMP) -e 65408 -c 480 -v
	@$(OSDRV_PUB_PC)/ubinize -o $(APPFS_UBIFS_IMG) -m $(RAMPARAM_PAGESIZE) -p 64KiB $(CUR_DIR)/configs/ubi_appfs.cfg -v
    else
	@$(OSDRV_PUB_PC)/$(UBIFS_BIN_NAME) -d $(APPFS_PATH)/ -m $(RAMPARAM_PAGESIZE) -o $(APPFS_UBIFS_IMG_TEMP) -e 126976 -c 240 -F -v
	@$(OSDRV_PUB_PC)/ubinize -o $(APPFS_UBIFS_IMG) -m $(RAMPARAM_PAGESIZE) -p 128KiB $(CUR_DIR)/configs/ubi_appfs.cfg -v
    endif
else ifeq ($(CONFIG_APPFS_YAFFS),y)
	@$(OSDRV_PUB_PC)/$(YAFFS_BIN_NAME) $(APPFS_PATH)/ $(APPFS_YAFFS_IMG) $(YAFFS_PAGESIZE_PARAM) $(YAFFS_ECCTYPE_PARAM)
endif

ifneq ($(AMP_TYPE), liteos)
appfs_prepare: appfs_clean private komod lib bin sharefs
else
appfs_prepare: appfs_clean private lib bin sharefs
endif
	@chmod 750 $(APPFS_PATH)/ -R
	@cd $(APPFS_PATH)/bin;ln -s ../sd $(APPFS_PATH)/bin/sd;cd -
	@chmod 755 $(APPFS_PATH)/sd -R
	@-find $(APPFS_PATH) -name "*.svn" |xargs rm -rf
	@-find $(APPFS_PATH) -name "*.map" |xargs rm -rf

private:
	@mkdir $(APPFS_PATH)
	@cp $(APPFS_PRIV_PATH)/* $(APPFS_PATH) -ar
	@mkdir $(APPFS_PATH)/private $(APPFS_PATH)/sd

komod:
	@if [ ! -d $(APPFS_PATH)/komod ];then mkdir $(APPFS_PATH)/komod/;fi
	@cd $(APPFS_PATH)/komod;cp load_module_$(CFG_CHIP_TYPE) load_module;rm load_module_*;cd -
	@sed -i "s/^mmz_anonymous_start=.*/mmz_anonymous_start=$(CONFIG_MEM_LINUX_MMZ_BASE)/"   $(APPFS_PATH)/komod/load_module
	@sed -i "s/^mmz_anonymous_size=.*/mmz_anonymous_size=$(CFG_LINUX_MMZ_ANONYMOUS_LEN)K/"   $(APPFS_PATH)/komod/load_module
	@sed -i "s/^mmz_higo_start=.*/mmz_higo_start=$(CFG_LINUX_MMZ_HIGO_BASE)/"   $(APPFS_PATH)/komod/load_module
	@sed -i "s/^mmz_higo_size=.*/mmz_higo_size=$(CFG_LINUX_MMZ_HIGO_LEN)K/"   $(APPFS_PATH)/komod/load_module
	@sed -i "s/^mmz_map=.*/mmz_map=$(CONFIG_MEM_HUAWEILITE_MMZ_BASE)/"   $(APPFS_PATH)/komod/load_module
	@sed -i "s/^mmz_map_len=.*/mmz_map_len=$(CFG_HUAWEILITE_MMZ_LEN)K/"   $(APPFS_PATH)/komod/load_module
	@tar -xf $(ROOTFS_INIT_PKT) -C $(CUR_DIR)
	@chmod 750 $(CUR_DIR)/$(basename $(notdir $(ROOTFS_INIT_PKT))) -R
	@mv $(CUR_DIR)/$(basename $(notdir $(ROOTFS_INIT_PKT))) $(ROOTFS_PATH)_appfs
	@cp -f $(ROOTFS_PATH)_appfs/komod/* $(APPFS_PATH)/komod/
	@rm -rf $(ROOTFS_PATH)_appfs
	@-sed -i "s;^cd /komod/;cd /app/komod/;" `find $(APPFS_PATH)/komod/ -name *.sh`
	@cd $(MPP_REL_LINUX_KO_PATH);cp *.ko $(APPFS_PATH)/komod/;cd -
ifneq ($(findstring $(CFG_CHIP_TYPE), hi3556av100 hi3519av100 hi3559v200),)
	@cd $(APPFS_PATH)/komod/;rm -f $(CFG_CHIP_TYPE)_acodec.ko $(CFG_CHIP_TYPE)_adec.ko $(CFG_CHIP_TYPE)_aenc.ko \
	$(CFG_CHIP_TYPE)_ai.ko $(CFG_CHIP_TYPE)_aio.ko $(CFG_CHIP_TYPE)_ao.ko $(CFG_CHIP_TYPE)_ir.ko \
	$(CFG_CHIP_TYPE)_motionfusion.ko hi_cipher.ko;cd -
endif
	@cd $(LINUX_ROOT);cp -f `find drivers/usb/ -name *.ko` $(APPFS_PATH)/komod/;cd -
	@-cd $(LINUX_ROOT);cp -f `find drivers/mmc/ -name *.ko` $(APPFS_PATH)/komod/;cd -
	@-cd $(LINUX_ROOT);cp -f `find drivers/media/ -name *.ko` $(APPFS_PATH)/komod/;cd -
	@-cd $(DRV_REL_PATH);cp *.ko $(APPFS_PATH)/komod/;cd -

lib:
	@if [ ! -d $(APPFS_PATH)/lib ];then mkdir $(APPFS_PATH)/lib/;fi

bin:
	@if [ ! -d $(APPFS_PATH)/bin ];then mkdir $(APPFS_PATH)/bin/;fi
ifneq ($(AMP_TYPE), liteos)
	@if [ -f $(PDT_OUT_BIN)/vio/vio_app ];then cp -f $(PDT_OUT_BIN)/vio/vio_app $(APPFS_PATH)/bin/;fi
	@if [ -f $(PDT_OUT_BIN)/lcd/lcd_app ];then cp -f $(PDT_OUT_BIN)/lcd/lcd_app $(APPFS_PATH)/bin/;fi
	@if [ -f $(PDT_OUT_BIN)/touchpad/tp_app ];then cp -f $(PDT_OUT_BIN)/touchpad/tp_app $(APPFS_PATH)/bin/;fi
endif

sharefs:
	@if [ ! -d $(APPFS_PATH)/sharefs ];then mkdir $(APPFS_PATH)/sharefs/;fi

appfs_clean:
	@if [ -d $(APPFS_PATH) ];then chmod 750 $(APPFS_PATH) -R; rm -rf $(APPFS_PATH);fi
