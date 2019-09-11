COMPILE_ROOT := $(shell pwd)
MIDDLEWARE_DIR := $(COMPILE_ROOT)/../..
include $(MIDDLEWARE_DIR)/cfg.mak
ifeq ($(CFG_OS_TYPE),liteos)
export OSTYPE :=HuaweiLite
else
export OSTYPE :=Linux
endif

ifeq ($(CFG_CHIP_TYPE),hi3559)
CFLAGS += -DSENSOR_IMX117
else ifeq ($(CFG_CHIP_TYPE),hi3556)
CFLAGS += -DSENSOR_OV4689
else ifeq ($(CFG_CHIP_TYPE),hi3559av100)
CFLAGS += -DSENSOR_IMX477
else ifeq ($(CFG_CHIP_TYPE),hi3516cv300)
CFLAGS += -DSENSOR_OV2718
else ifeq ($(CFG_CHIP_TYPE),hi3518ev300)
CFLAGS += -DSENSOR_IMX307
else
CFLAGS += -DSENSOR_IMX477
endif
include $(COMPILE_ROOT)/Make.$(OSTYPE)
