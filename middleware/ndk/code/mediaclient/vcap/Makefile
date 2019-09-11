LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../..
MC_PARAM_FILE:=../Makefile.param
include $(MC_PARAM_FILE)

#*************************************************************************
# all source file in this module
SRCS := mapi_vcap_client.c
SRCS += mapi_vcap_client_motionsfusion.c
SRCS += mapi_vcap_client_hal_ahd.c

#*************************************************************************
TARGET := $(HIARCH)_vcap_client

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
