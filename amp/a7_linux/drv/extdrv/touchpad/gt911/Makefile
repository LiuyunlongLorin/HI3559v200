#modify for Action DV.
obj-m := gt_911.o
gt_911-objs :=i2cdev.o gt911.o

CUR_DIR  := $(shell pwd)
SDK_ROOT ?= $(CUR_DIR)/../../../..
-include  $(SDK_ROOT)/build/base.mak
dummy := $(call CreateDir, $(DRV_REL_PATH))

MODULE_PATH          ?= $(CUR_DIR)
export ARCH          ?= $(OSDRV_ARCH)
export CROSS_COMPILE ?= $(LINUX_CROSS)-
export CFG_CHIP_TYPE ?= $(CFG_CHIP_TYPE)

export CONFIG_REFB_ACTIONCAM
ifeq ($(CONFIG_REFB_ACTIONCAM), y)
KBUILD_CFLAGS_MODULE += -DBOARD_ACTIONCAM_REFB
endif

export CONFIG_REFB_DASHCAM
ifeq ($(CONFIG_REFB_DASHCAM), y)
KBUILD_CFLAGS_MODULE += -DBOARD_DASHCAM_REFB
endif

export CONFIG_DEMB
ifeq ($(CONFIG_DEMB), y)
KBUILD_CFLAGS_MODULE += -DBOARD_DEMB
endif

export CFG_CHIP_TYPE_UPPER ?= $(shell echo $(CFG_CHIP_TYPE) | tr '[a-z]' '[A-Z]')
KBUILD_CFLAGS_MODULE += -D$(CFG_CHIP_TYPE_UPPER)

EXTRA_CFLAGS += -I$(DRV_EXTDRV_PATH_LINUX)/gpio
KBUILD_EXTRA_SYMBOLS += $(DRV_EXTDRV_PATH_LINUX)/gpio/Module.symvers
all: ko install

install:
	@echo -e "\e[0;32;1m--Compiling '$(notdir $(CUR_DIR))'...\e[0;36;1m"
	@echo -e "\e[0m"
	@cp -f $(CUR_DIR)/*.ko $(DRV_REL_PATH)/
	@echo KCONFIG_CFLAGS = $(KCONFIG_CFLAGS)
	@echo KBUILD_CFLAGS_MODULE = $(KBUILD_CFLAGS_MODULE)

ko:
	@make -C $(LINUX_ROOT) M=$(MODULE_PATH) modules

clean:
	@-rm -rf $(MODULE_PATH)/*.o
	@-rm -rf $(MODULE_PATH)/*.cmd
	@-rm -rf $(MODULE_PATH)/*.out
	@-rm -rf $(MODULE_PATH)/.symvers
	@-rm -rf $(MODULE_PATH)/.order
	@rm -rf $(MODULE_PATH)/*.ko
	@make -C $(LINUX_ROOT) M=$(MODULE_PATH) clean

