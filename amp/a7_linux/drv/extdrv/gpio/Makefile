obj-m         :=  hi_gpio.o

CUR_DIR  := $(shell pwd)
SDK_ROOT ?= $(CUR_DIR)/../../..
-include  $(SDK_ROOT)/build/base.mak
dummy := $(call CreateDir, $(DRV_REL_PATH))

MODULE_PATH          ?= $(CUR_DIR)
export ARCH          ?= $(OSDRV_ARCH)
export CROSS_COMPILE ?= $(LINUX_CROSS)-
export CFG_CHIP_TYPE ?= $(CFG_CHIP_TYPE)

export CFG_CHIP_TYPE_UPPER ?= $(shell echo $(CFG_CHIP_TYPE) | tr '[a-z]' '[A-Z]')
KBUILD_CFLAGS_MODULE += -D$(CFG_CHIP_TYPE_UPPER)

all: ko install

install:
	@echo -e "\e[0;32;1m--Compiling '$(notdir $(CUR_DIR))'...\e[0;36;1m"
	@echo -e "KBUILD_CFLAGS_MODULE: $(KBUILD_CFLAGS_MODULE)"
	@echo -e "\e[0m"
	@cp -f $(CUR_DIR)/*.ko $(DRV_REL_PATH)/

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
