export  OSTYPE=liteos

LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../../..
MC_PARAM_FILE:=../Makefile.param
include $(MC_PARAM_FILE)

CFLAGS += -I$(NDK_PATH)/code/include/
CFLAGS += -Wno-error

TARGET := LiteOS

# compile rule
include $(PWD)/../liteos.mak

