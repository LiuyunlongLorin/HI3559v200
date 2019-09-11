LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../..
MC_PARAM_FILE:=../Makefile.param
include $(MC_PARAM_FILE)

#*************************************************************************
# all source file in this module
SRCS := mapi_vcap.c
SRCS += mapi_vcap_motionfusion.c
SRCS += mapi_vcap_hal_ahd.c
SRCS += mapi_vcap_peripheral.c
SRCS += arch/$(HIARCH)/mapi_vcap_adapt.c

CFLAGS += -I$(MEDIASERVER_PATH)/configs/sensor/$(HIARCH)/include
CFLAGS += -I$(MEDIASERVER_PATH)/vcap/arch/include

#*************************************************************************
TARGET := $(HIARCH)_mapi_vcap

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
