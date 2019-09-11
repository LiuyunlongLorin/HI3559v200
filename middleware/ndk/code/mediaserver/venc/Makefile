LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../..
MC_PARAM_FILE:=../Makefile.param
include $(MC_PARAM_FILE)

#*************************************************************************
# all source file in this module
SRCS := mapi_venc.c
SRCS += arch/$(HIARCH)/mapi_venc_adapt.c
SRCS += os/$(OSCFG)/mapi_venc_os.c

-include $(ARCH_CHIP)/venc/Makefile


#*************************************************************************
CFLAGS += -I./arch/include
CFLAGS += -I./os/include

TARGET := $(HIARCH)_mapi_venc

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
