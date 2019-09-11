LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../..
MC_PARAM_FILE:=../Makefile.param
include $(MC_PARAM_FILE)

#*************************************************************************
# all source file in this module
SRCS := mapi_aenc.c
SRCS += os/$(OSCFG)/mapi_aenc_os.c

-include $(MEDIASERVER_PATH)/adapt/aenc/aac/Makefile

CFLAGS += -I$(MEDIASERVER_PATH)/adapt/aenc/include
CFLAGS += -I$(MEDIASERVER_PATH)/adapt/aenc/aac/
#*************************************************************************
TARGET := $(HIARCH)_mapi_aenc

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
