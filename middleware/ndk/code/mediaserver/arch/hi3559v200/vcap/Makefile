LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../../../..
MC_PARAM_FILE:=../../../Makefile.param
include $(MC_PARAM_FILE)

SRCS := adpt_vcap_inner_config.c

CFLAGS += -I$(MPP_HEADER_LITEOS)
CFLAGS += -I ./../../include
CFLAGS += -I$(NDK_PATH)/code/include
CFLAGS += -I$(NDK_PATH)/code/include/inner
TARGET := $(HIARCH)_mapi_vcap_inner_cfg
#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../../../Make.$(OSTYPE)
