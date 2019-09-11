LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../..
MC_PARAM_FILE:=../Makefile.param
include $(MC_PARAM_FILE)

#*************************************************************************
# all source file in this module
SRCS := mapi_disp.c
ifeq ($(CFG_SUPPORT_HDMI), y)
SRCS += mapi_hdmi.c
SRCS += os/$(OSCFG)/mapi_hdmi_os.c
endif
SRCS += arch/$(HIARCH)/mapi_disp_adapt.c

#*************************************************************************
CFLAGS += -I./arch/include
CFLAGS += -I./os/include

TARGET := $(HIARCH)_mapi_disp

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
