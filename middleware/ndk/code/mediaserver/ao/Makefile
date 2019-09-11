LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../..
MC_PARAM_FILE:=../Makefile.param
include $(MC_PARAM_FILE)

#*************************************************************************
# all source file in this module
SRCS := mapi_ao.c
SRCS += arch/$(HIARCH)/mapi_ao_adapt.c
#*************************************************************************

TARGET := $(HIARCH)_mapi_ao

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
