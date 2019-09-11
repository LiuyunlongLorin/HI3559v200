CUR_DIR  := $(shell pwd)
MIDDLEWARE_DIR := $(CUR_DIR)/../../..
-include $(MIDDLEWARE_DIR)/base.mak
#modify for Action DV.
obj-m := ft_6236.o
ft_6236-objs :=i2cdev.o hi_gpio.o ft6236.o

LINUX_ROOT := $(OSDRV_ROOT)/opensource/kernel/linux-4.9.y

export ARCH          ?= arm
export CROSS_COMPILE ?= $(CFG_SDK_TOOLCHAIN)
export CFG_CHIP_TYPE
KBUILD_CFLAGS_MODULE += -DHI3559V200

all: ko

ko:
	@make -C $(LINUX_ROOT) M=$(CUR_DIR) modules

clean:
	@make -C $(LINUX_ROOT) M=$(CUR_DIR) clean
