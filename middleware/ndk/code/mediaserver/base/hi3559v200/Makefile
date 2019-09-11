LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../../..
MC_PARAM_FILE:=../../Makefile.param
include $(MC_PARAM_FILE)

#*************************************************************************
# all source file in this module
SRCS := load_sdk.c

#*************************************************************************
TARGET := $(HIARCH)load_sdk

CFLAGS += $(MS_CFLAGS)
CFLAGS += -Wno-error

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../../Make.$(OSTYPE)
