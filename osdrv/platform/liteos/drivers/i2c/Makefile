I2C_SAMPLE := n

include $(LITEOSTOPDIR)/config.mk
MODULE_NAME := $(notdir $(shell pwd))

LOCAL_SRCS := src/i2c.c src/i2c_dev.c src/i2c_shell.c
ifdef LOSCFG_HOST_TYPE_HIBVT
LOCAL_SRCS += src/i2c-hibvt.c
else
LOCAL_SRCS += src/i2c-hisilicon.c
endif

ifeq ($(LOSCFG_DRIVERS_HIDMAC), y)
LOCAL_INCLUDE := -I $(LITEOSTOPDIR)/drivers/hidmac/src
else
LOCAL_INCLUDE :=
endif

ifeq ($(I2C_SAMPLE), y)
I2C_BUS_DRVIER_DEMO := y
LOCAL_SRCS += sample/cw201x_bat.c
endif

LOCAL_FLAGS := $(LOCAL_INCLUDE)

include $(MODULE)
