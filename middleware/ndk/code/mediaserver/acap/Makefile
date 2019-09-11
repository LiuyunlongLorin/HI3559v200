LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../..
MC_PARAM_FILE:=../Makefile.param
include $(MC_PARAM_FILE)

#*************************************************************************
# all source file in this module
SRCS := mapi_acap.c
SRCS += arch/$(HIARCH)/mapi_acap_adapt.c

-include $(ARCH_CHIP)/audio/Makefile


#*************************************************************************
CFLAGS += -I./arch/include

TARGET := $(HIARCH)_mapi_acap

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
