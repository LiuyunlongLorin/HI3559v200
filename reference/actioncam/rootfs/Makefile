# Base Configure
CUR_DIR  := $(shell pwd)
PDT_ROOT ?= $(CUR_DIR)/..
include $(PDT_ROOT)/build/base.mak

# PDT
PDT_UI_PATH      := $(PDT_MODULE_PATH)/ui
BUILD_TIME       := $(shell date +%Y%m%d)
BUILD_SOFTVERSION:= $(shell echo $(CONFIG_SOFT_VERSION)).$(BUILD_TIME)
DEVICE_MODEL     := $(shell echo $(CONFIG_DEVICE_MODEL))

# OSDRV
OSDRV_PUB_PC     := $(OSDRV_PUB_PC_BIN_PATH)
OSDRV_PUB_IMG    := $(OSDRV_PUB_IMG_PATH)
JFFS2_BIN_NAME   := mkfs.jffs2
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
PARAM_FS_PATH           := $(APPFS_PATH)/param
PARAM_INICFG_PATH       := $(PDT_INICFG_PATH)
PNGQUANT_PATH           := $(shell which pngquant)

# Upgrade
UPGRADE_TOOL_PATH       := $(REF_OUT_TOOL_BIN)/upgrade_generate
UPGRADE_TOOL_NAME       := upgrade_generate
UPGRADE_PKT_NAME        := upgrade_$(DEVICE_MODEL)_$(BUILD_SOFTVERSION).sw

# Burn Config
BURN_CFG_PATH           := $(CUR_DIR)/configs/$(CFG_CHIP_TYPE)
BURN_CONFIG             := $(BURN_CFG_PATH)/config_$(CFG_ROOTFS_TYPE)_$(CFG_APPFS_TYPE)_$(FLASH_TYPE)
BURN_XML                := $(BURN_CFG_PATH)/config_$(CFG_ROOTFS_TYPE)_$(CFG_APPFS_TYPE)_$(FLASH_TYPE).xml
BURN_AI                 := $(BURN_CFG_PATH)/config_$(CFG_ROOTFS_TYPE)_$(CFG_APPFS_TYPE)_$(FLASH_TYPE).ai
CFG_LITEOS_TEXT_BASE    := $(shell awk 'BEGIN{printf("0x%x",'$(CONFIG_MEM_HUAWEILITE_SYS_BASE)'+'$(OSDRV_LITEOS_TEXT_OFFSET)')}')
CFG_LITEOS_READ_ADDR    := $(shell awk 'BEGIN{printf("0x%x",'$(CONFIG_MEM_HUAWEILITE_MMZ_BASE)'+'0x1000000')}')
CFG_LINUX_BOOT_ADDR     := $(shell awk 'BEGIN{printf("0x%x",'$(CONFIG_MEM_LINUX_SYS_BASE)'+'0x2000000')}')
CFG_PARAM_READ_ADDR     := $(shell awk 'BEGIN{printf("0x%x",'$(CFG_LINUX_BOOT_ADDR)')}')
CFG_PARAMBAK_READ_ADDR  := $(shell awk 'BEGIN{printf("0x%x",'$(CFG_PARAM_READ_ADDR)'+'0x100000')}')
CFG_PARAM_BASE_ADDR     := $(shell awk 'BEGIN{printf("0x%x",'$(CONFIG_MEM_PARAM_BASE)')}')

# Memory Configure
CFG_LINUX_SYS_LEN = $(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_SYS_SIZE)'/0x100000)}')
CFG_LINUX_MMZ_ANONYMOUS_LEN = $(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_MMZ_ANONYMOUS_SIZE)'/0x400)}')
CFG_MEM_LINUX_MMZ_BASE = $(shell awk 'BEGIN{printf("0x%x",'$(CONFIG_MEM_LINUX_MMZ_BASE)'+'$(CONFIG_MEM_LINUX_MMZ_ANONYMOUS_SIZE)')}')
CFG_LINUX_MMZ_HIGO_LEN = $(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_MMZ_HIGO_SIZE)'/0x400)}')
CFG_HUAWEILITE_MMZ_LEN = $(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_HUAWEILITE_MMZ_SIZE)'/0x400)}')


# Compile PHONY
.PHONY: all install clean distclean
default: all

all: rootfs appfs install upgrade_pkt font_check

install: common_install
ifeq ($(CONFIG_REFB_ACTIONCAM)_$(CFG_CHIP_TYPE),y_hi3559av100)
	@-cp -f $(OSDRV_PUB_IMG)/mini-boot-$(CFG_CHIP_TYPE)_refb.bin $(PDT_OUT_BURN)/$(UBOOT_MINI_IMG);
	@cp -f $(OSDRV_PUB_IMG)/u-boot-$(CFG_CHIP_TYPE)_refb.bin $(PDT_OUT_BURN)/$(UBOOT_IMG_PKT);
else
	@cp -f $(OSDRV_PUB_IMG)/u-boot-$(CFG_CHIP_TYPE).bin $(PDT_OUT_BURN)/$(UBOOT_IMG_PKT);
endif
	@cp -f $(OSDRV_PUB_IMG_BIN) $(PDT_OUT_BURN)/$(KERNEL_IMG_PKT)
	@if [ -f $(OSDRV_PUB_IMG_BIN)_zip ];then cp -f $(OSDRV_PUB_IMG_BIN)_zip $(PDT_OUT_BURN)/$(KERNEL_IMG_PKT)_zip;fi

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
	@sed -i 's/param_readaddr/$(CFG_PARAM_READ_ADDR)/g' $(PDT_OUT_BURN)/config
	@sed -i 's/parambak_readaddr/$(CFG_PARAMBAK_READ_ADDR)/g' $(PDT_OUT_BURN)/config
	@sed -i 's/param_baseaddr/$(CFG_PARAM_BASE_ADDR)/g' $(PDT_OUT_BURN)/config
	@sed -i 's/linux_bootaddr/$(CFG_LINUX_BOOT_ADDR)/g' $(PDT_OUT_BURN)/config
	@sed -i 's/liteos_textbase/$(CFG_LITEOS_TEXT_BASE)/g' $(PDT_OUT_BURN)/config
	@sed -i 's/liteos_readaddr/$(CFG_LITEOS_READ_ADDR)/g' $(PDT_OUT_BURN)/config
	@if [ -f $(BURN_XML) ];then cp -f $(BURN_XML) $(PDT_OUT_BURN)/usb-burn.xml;fi
	@if [ -f $(BURN_AI) ];then cp -f $(BURN_AI) $(PDT_OUT_BURN)/eth-burn.ai;fi
	@if [ -f $(CUR_DIR)/rawparam ];then cp -f $(CUR_DIR)/rawparam $(PDT_OUT_BURN)/rawparam;fi
	@if [ -f $(CUR_DIR)/rawparam ];then cp -f $(CUR_DIR)/rawparam $(PDT_OUT_BURN)/rawparambak;fi
ifeq ($(CFG_DEBUG_IMAGE_MAP_ON),yes)
	@cp $(PDT_OUT_BIN)/master/main_app $(PDT_OUT_BURN) -f
	@cp $(PDT_OUT_BIN)/master/main_app.map $(PDT_OUT_BURN) -f
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

upgrade_pkt:
	@cp $(UPGRADE_TOOL_PATH)/$(UPGRADE_TOOL_NAME) $(PDT_OUT_BURN)/
	@cd $(PDT_OUT_BURN)/;chmod +x ./$(UPGRADE_TOOL_NAME);./$(UPGRADE_TOOL_NAME) $(DEVICE_MODEL) $(BUILD_SOFTVERSION) config
	@rm -f $(PDT_OUT_BURN)/$(UPGRADE_TOOL_NAME)

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
ifeq ($(CFG_CHIP_TYPE),hi3559av100)
	@sed -i 's;#sleep ;sleep ;g' $(ROOTFS_PATH)/etc/init.d/S10mpp
endif
	@-find $(ROOTFS_PATH)/bin/ -name "mkfs.ext*" |xargs rm -f
ifeq ($(CONFIG_RELEASE), y)
	@-find $(ROOTFS_PATH)/lib/ -name "libstdc++.so*" |xargs rm -f
endif
	@cd $(ROOTFS_PATH)/bin/;rm -f udevadm udevd mke2fs;cd -
	@rm -f $(ROOTFS_PATH)/etc/init.d/S0*udev
	@-find $(ROOTFS_PATH)/bin/ -type f -name "*" |xargs $(STRIP)
	@-find $(ROOTFS_PATH)/ -type f -name "*.so*" |grep -v libgcc_s.so |xargs $(STRIP)
ifneq ($(CONFIG_DEBUG),y)
	@#upx --best $(ROOTFS_PATH)/bin/busybox
	@#upx --best $(ROOTFS_PATH)/bin/btools
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
	@$(OSDRV_PUB_PC)/$(JFFS2_BIN_NAME) -e 0x10000 -d $(APPFS_PATH)/ -o $(APPFS_JFFS2_IMG) -X lzma -x zlib -x rtime -m size
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

appfs_prepare: appfs_clean
	@mkdir $(APPFS_PATH)
	@# Private
	@cp $(APPFS_PRIV_PATH)/* $(APPFS_PATH) -ar
	@mkdir $(APPFS_PATH)/private $(APPFS_PATH)/sd
	@# Param
	@mkdir -p $(PARAM_FS_PATH)
	@cp $(PARAM_INICFG_PATH)/* $(PARAM_FS_PATH)
	@sed -i 's/^software_version *=.*/software_version   = \"$(BUILD_SOFTVERSION)\"/' $(PARAM_FS_PATH)/config_product_devmng.ini
	@sed -i 's/^model *=.*/model              = \"$(DEVICE_MODEL)\"/' $(PARAM_FS_PATH)/config_product_devmng.ini
ifeq ($(CONFIG_MOTIONSENSOR),y)
	@sed -i 's/^dis_type *=.*/dis_type   = \"DIS_MODE_GYRO\"/' $(PARAM_FS_PATH)/config_product_media_common.ini
endif
	@cp $(PDT_OUT_MODULES_BIN)/ini2bin_pc $(PARAM_FS_PATH)
	@cp $(PDT_OUT_MODULES_BIN)/ini2bin_board $(PARAM_FS_PATH)
	@cp $(PDT_OUT_MODULES_BIN)/bin2flash $(PARAM_FS_PATH)
	@cd $(PARAM_FS_PATH);chmod +x ./ini2bin_pc;./ini2bin_pc >/dev/null;rm ./ini2bin_pc;cd -
	@cp $(PDT_OUT_MODULES_BIN)/bin2image_pc $(CUR_DIR)
	@cd $(CUR_DIR);chmod +x bin2image_pc;cp $(PARAM_FS_PATH)/param.bin .;./bin2image_pc;rm bin2image_pc param.bin;cd -
ifneq ($(CONFIG_DEBUG),y)
ifneq ($(CONFIG_MOTIONSENSOR),y)
	@cd $(PARAM_FS_PATH);rm -f *.ini ini2bin_board bin2flash param.bin
endif
endif
	@# Komod
	@if [ ! -d $(APPFS_PATH)/komod ];then mkdir $(APPFS_PATH)/komod/;fi
	@cd $(APPFS_PATH)/komod;cp load_module_$(CFG_CHIP_TYPE) load_module;rm load_module_*;cd -
	@sed -i "s/^mmz_anonymous_start=.*/mmz_anonymous_start=$(CONFIG_MEM_LINUX_MMZ_BASE)/"   $(APPFS_PATH)/komod/load_module
	@sed -i "s/^mmz_anonymous_size=.*/mmz_anonymous_size=$(CFG_LINUX_MMZ_ANONYMOUS_LEN)K/"   $(APPFS_PATH)/komod/load_module
	@sed -i "s/^mmz_higo_start=.*/mmz_higo_start=$(CFG_MEM_LINUX_MMZ_BASE)/"   $(APPFS_PATH)/komod/load_module
	@sed -i "s/^mmz_higo_size=.*/mmz_higo_size=$(CFG_LINUX_MMZ_HIGO_LEN)K/"   $(APPFS_PATH)/komod/load_module
	@sed -i "s/^mmz_map=.*/mmz_map=$(CONFIG_MEM_HUAWEILITE_MMZ_BASE)/"   $(APPFS_PATH)/komod/load_module
	@sed -i "s/^mmz_map_len=.*/mmz_map_len=$(CFG_HUAWEILITE_MMZ_LEN)K/"   $(APPFS_PATH)/komod/load_module
ifeq ($(CONFIG_MEM_LINUX_MMZ_HDMI_RELOAD),y)
	@sed -i 's;#LCD insmod hifb.ko ;insmod hifb.ko ;g' $(APPFS_PATH)/komod/load_module
else
	@sed -i 's;#ALL insmod hifb.ko ;insmod hifb.ko ;g' $(APPFS_PATH)/komod/load_module
endif
	@tar -xf $(ROOTFS_INIT_PKT) -C $(CUR_DIR)
	@chmod 750 $(CUR_DIR)/$(basename $(notdir $(ROOTFS_INIT_PKT))) -R
	@mv $(CUR_DIR)/$(basename $(notdir $(ROOTFS_INIT_PKT))) $(ROOTFS_PATH)_appfs
	@cp -f $(ROOTFS_PATH)_appfs/komod/* $(APPFS_PATH)/komod/
	@rm -rf $(ROOTFS_PATH)_appfs
	@-sed -i "s;^cd /komod/;cd /app/komod/;" `find $(APPFS_PATH)/komod/ -name "*.sh"`
	@cd $(MPP_REL_LINUX_KO_PATH);cp *.ko $(APPFS_PATH)/komod/;cd -
ifneq ($(findstring $(CFG_CHIP_TYPE), hi3556av100 hi3559v200),)
	@cd $(APPFS_PATH)/komod/;rm -f $(CFG_CHIP_TYPE)_acodec.ko $(CFG_CHIP_TYPE)_adec.ko $(CFG_CHIP_TYPE)_aenc.ko \
	$(CFG_CHIP_TYPE)_ai.ko $(CFG_CHIP_TYPE)_aio.ko $(CFG_CHIP_TYPE)_ao.ko $(CFG_CHIP_TYPE)_ir.ko \
	$(CFG_CHIP_TYPE)_motionfusion.ko $(CFG_CHIP_TYPE)_wdt.ko hi_cipher.ko;cd -
endif
	@cd $(LINUX_ROOT);cp -f `find drivers/usb/ -name *.ko` $(APPFS_PATH)/komod/;cd -
	@-cd $(LINUX_ROOT);cp -f `find drivers/mmc/ -name *.ko` $(APPFS_PATH)/komod/;cd -
	@-cd $(LINUX_ROOT);cp -f `find drivers/media/ -name *.ko` $(APPFS_PATH)/komod/;cd -
	@-cd $(DRV_REL_PATH);cp *.ko $(APPFS_PATH)/komod/;cd -
ifeq ($(CONFIG_REFB_ACTIONCAM),y)
	@-cd $(APPFS_PATH)/komod/;mv ft_6356_refb.ko ft_6356.ko;cd -
endif
ifeq ($(CONFIG_WIFI),y)
ifeq ($(CONFIG_WIFI_BCM6255),y)
	@cd $(DRV_REL_PATH)/wifi/sdio_ap6255;cp *.ko $(APPFS_PATH)/komod/;cd -
	@if [ -d $(DRV_REL_PATH)/wifi/sdio_ap6255/firmware ];then cd $(DRV_REL_PATH)/wifi/sdio_ap6255;cp -rf firmware $(APPFS_PATH)/;cd -;fi
else ifeq ($(CONFIG_WIFI_BCM6356S),y)
	@-cd $(DRV_REL_PATH)/wifi/sdio_ap6356s;cp *.ko $(APPFS_PATH)/komod/;cd -
	@if [ -d $(DRV_REL_PATH)/wifi/sdio_ap6356s/firmware ];then cd $(DRV_REL_PATH)/wifi/sdio_ap6356s;cp -rf firmware $(APPFS_PATH)/;cd -;fi
else ifeq ($(CONFIG_WIFI_BCM6214A),y)
	@-cd $(DRV_REL_PATH)/wifi/sdio_ap6214a;cp *.ko $(APPFS_PATH)/komod/;cd -
	@if [ -d $(DRV_REL_PATH)/wifi/sdio_ap6214a/firmware ];then cd $(DRV_REL_PATH)/wifi/sdio_ap6214a;cp -rf firmware $(APPFS_PATH)/;cd -;fi
else ifeq ($(CONFIG_WIFI_RTL8189),y)
	@-cd $(DRV_REL_PATH)/wifi/sdio_rtl8189ftv;cp *.ko $(APPFS_PATH)/komod/;cd -
endif
endif
	@# Lib
	@if [ ! -d $(APPFS_PATH)/lib ];then mkdir $(APPFS_PATH)/lib/;fi
	@-cp $(PDT_OUT_LIB_LINUX)/*.so $(APPFS_PATH)/lib/ -f
ifeq ($(CONFIG_WIFI),y)
	@cp $(REF_THIRDPARTY_PATH)/out/lib/libnl* $(APPFS_PATH)/lib/;
	@cd $(APPFS_PATH)/lib/;ln -s libnl-genl.so.3.0.0 libnl-genl.so.3; ln -s libnl.so.3.0.0 libnl.so.3;cd -
endif
ifneq ($(CONFIG_DEBUG)_$(CONFIG_COREDUMP_ON),y_y)
	@-find $(APPFS_PATH)/lib/ -name "*.so*" |xargs $(STRIP)
endif
	@# Bin
	@if [ ! -d $(APPFS_PATH)/bin ];then mkdir $(APPFS_PATH)/bin/;fi
ifeq ($(CONFIG_WIFI),y)
	@cp $(REF_THIRDPARTY_PATH)/out/tools/hostapd $(APPFS_PATH)/bin/;
	@cp $(REF_THIRDPARTY_PATH)/hostapd/entropy.bin  $(APPFS_PATH)/bin/
	@#upx --best $(APPFS_PATH)/bin/hostapd
endif
	@cp $(PDT_UI_PATH)/res $(APPFS_PATH)/bin/ -ar
	@cp $(PDT_UI_PATH)/higv/higv.bin $(APPFS_PATH)/bin/ -f
	@cp $(PDT_UI_PATH)/bin/en.lang $(APPFS_PATH)/bin/ -f
	@cp $(PDT_UI_PATH)/bin/zh.lang $(APPFS_PATH)/bin/ -f
	@cp $(PDT_OUT_BIN)/linux/main_app $(APPFS_PATH)/bin/ -f
ifneq ($(CONFIG_DEBUG)_$(CONFIG_COREDUMP_ON),y_y)
	@#if [ $(CFG_LLT) = "false" ];then $(STRIP) $(APPFS_PATH)/bin/main_app;fi
	$(STRIP) $(APPFS_PATH)/bin/main_app
ifneq ($(LIBC_TYPE),glibc)
	@#upx --best $(APPFS_PATH)/bin/main_app
endif
	@sed -i "s;^ulimit -c unlimited;#ulimit -c unlimited;" $(APPFS_PATH)/bootapp
	@sed -i "s;^echo \"/app/sd/core.%e-%p-%t\"> /proc/sys/kernel/core_pattern;#echo \"/app/sd/core.%e-%p-%t\"> /proc/sys/kernel/core_pattern;" $(APPFS_PATH)/bootapp
endif
ifeq ($(CONFIG_PQT_STREAM_SUPPORT_ON), y)
	@sed -i "s;^./main_app;#./main_app;" $(APPFS_PATH)/bootapp
endif
	@# Sharefs
	@if [ ! -d $(APPFS_PATH)/sharefs ];then mkdir $(APPFS_PATH)/sharefs/;fi
ifeq ($(CONFIG_POST_PROCESS),y)
ifeq ($(CFG_CHIP_TYPE),hi3559av100)
	@mkdir $(APPFS_PATH)/dsp_bin;cp $(MPP_SAMPLE_LINUX_DSP_BIN)/dsp0 $(APPFS_PATH)/dsp_bin/ -rf
endif
ifeq ($(CFG_CHIP_TYPE),hi3556av100)
	@cp -rf $(MPP_SAMPLE_LINUX_DSP_BIN)/* $(APPFS_PATH)/sharefs/
endif
endif
ifeq ($(CONFIG_DEBUG),y)
	@cp $(PDT_SCENE_INICFG_PATH)/*.ini $(APPFS_PATH)/sharefs/
	@cp $(PDT_OUT_MODULES_BIN)/scene_ini2bin_pc $(APPFS_PATH)/sharefs/
	@cp $(PDT_OUT_MODULES_BIN)/scene_ini2bin_board $(APPFS_PATH)/sharefs/
	@cd $(APPFS_PATH)/sharefs/;chmod +x ./scene_ini2bin_pc;./scene_ini2bin_pc;rm ./scene_ini2bin_pc;cd -
endif
	@cp -f $(PDT_OUT_LIB_HUAWEILITE)/*.so $(APPFS_PATH)/sharefs/
ifeq ($(CONFIG_PQT_SUPPORT_ON), y)
	@cp -f $(PQT_PATH)/config.cfg $(APPFS_PATH)/sharefs/
	@cd $(APPFS_PATH);ln -s ../sharefs/config.cfg $(APPFS_PATH)/bin/config.cfg;cd -
	@cp -f $(PQT_PATH)/StartControl.sh $(APPFS_PATH)/bin/
	@cp -f $(PQT_PATH)/ittb_control $(APPFS_PATH)/bin/
	@cp -rf $(PQT_PATH)/libs $(APPFS_PATH)/bin/
ifeq ($(CONFIG_PQT_STREAM_SUPPORT_ON), y)
	@cp -f $(PQT_PATH)/HiIspTool.sh $(APPFS_PATH)/bin/
	@cp -f $(PQT_PATH)/ittb_stream $(APPFS_PATH)/bin/
	@cp -rf $(PQT_PATH)/configs $(APPFS_PATH)/bin/
	@cp -rf $(PQT_PATH)/configs $(APPFS_PATH)/sharefs/
	@mkdir $(APPFS_PATH)/bin/configs/common -p
	@cp -f $(PQT_PATH)/configs/common/config_mt.ini $(APPFS_PATH)/bin/configs/common/
endif
endif
	@chmod 750 $(APPFS_PATH)/ -R
	@cd $(APPFS_PATH)/bin;ln -s ../sd $(APPFS_PATH)/bin/sd;cd -
	@chmod 755 $(APPFS_PATH)/sd -R
	@-find $(APPFS_PATH) -name "*.svn" |xargs rm -rf
	@-find $(ROOTFS_PATH) -name "*.map" |xargs rm -rf
	make piccom
piccom:
	@pic_file_cnt=$(shell ls $(APPFS_PATH)/bin/res/pic/* 2> /dev/null | wc -l)
ifneq ($(pic_file_cnt), 0)
ifeq ($(findstring pngquant, $(PNGQUANT_PATH)), pngquant)
	@chmod +x $(CUR_DIR)/piccompress.sh
	@$(shell $(CUR_DIR)/piccompress.sh  $(APPFS_PATH)/bin/res/pic 2> /dev/null)
	@echo -e "\033[44;37m   pic compress! \033[0m"
else
	@echo -e "\033[44;37m   pic no compress! \033[0m"
endif
endif

appfs_clean:
	@if [ -d $(APPFS_PATH) ];then chmod 750 $(APPFS_PATH) -R; rm -rf $(APPFS_PATH);fi

BASE_FONT_FILE:= $(APPFS_PATH)/bin/res/font/vector.ttf
font_check:
	@if [ ! -f $(BASE_FONT_FILE) ];then make no_fontlib_warning;fi

no_fontlib_warning:
	@echo -e ""
	@echo -e ""
	@echo -e "\033[41;37m/*                  Error!!!                         */\033[0m"
	@echo -e "\033[44;37m/*                  Error!!!                         */\033[0m"
	@echo -e ""
	@echo -e "\033[44;37m  There aren't ttf fonts in the project!!!\033[0m"
	@echo -e "\033[44;37m  Please put the fonts to ./references/actioncam/modules/ui/res/font/ directory!\033[0m"
	@echo -e "\033[44;37m  You can see HiMobileCam SDK install document to get more information\033[0m"
	@echo -e ""
	@echo -e ""
	@echo -e ""
	@echo -e ""
