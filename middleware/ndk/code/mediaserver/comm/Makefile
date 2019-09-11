LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../..
MC_PARAM_FILE:=../Makefile.param
include $(MC_PARAM_FILE)

#*************************************************************************
# all source file in this module
SRCS := mapi_sys.c
SRCS += arch/$(HIARCH)/mapi_sys_adapt.c
SRCS += os/$(OSCFG)/mapi_sys_os.c

CFLAGS += -I$(MEDIASERVER_PATH)/comm/arch/$(HIARCH)
CFLAGS += -I./os/include/

-include $(ARCH_CHIP)/sys/Makefile

#*************************************************************************
TARGET := $(HIARCH)_mapi_sys

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
