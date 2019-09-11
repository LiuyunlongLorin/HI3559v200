CUR_DIR  := $(shell pwd)
MIDDLEWARE_DIR := $(CUR_DIR)/../../..
-include $(MIDDLEWARE_DIR)/base.mak

obj-m := hi_ssp_st7789.o

LINUX_ROOT := $(OSDRV_ROOT)/opensource/kernel/linux-4.9.y

export CFG_CHIP_TYPE
KBUILD_CFLAGS_MODULE += -DHI3559V200
export ARCH          ?= arm
export CROSS_COMPILE ?= $(CFG_SDK_TOOLCHAIN)

all: ko

ko:
	@make -C $(LINUX_ROOT) M=$(CUR_DIR) modules

clean:
	@make -C $(LINUX_ROOT) M=$(CUR_DIR) clean
