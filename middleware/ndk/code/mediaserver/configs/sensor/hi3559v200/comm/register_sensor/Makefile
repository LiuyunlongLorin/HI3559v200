LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../../../../../..
MC_PARAM_FILE:=../../../../../Makefile.param
include $(MC_PARAM_FILE)

SRCS := register_sensor.c

CFLAGS  += -Wall -fPIC
CFLAGS  += -O2
CFLAGS  += $(LIBS_CFLAGS)
CFLAGS  += -I $(SENSOR_CFG_PATH)/include
CFLAGS  += -I $(REL_INC)
CFLAGS  += -I $(MPP_HEADER_LITEOS)
CFLAGS  += -I$(NDK_PATH)/code/mediaserver/utils
CFLAGS += -I$(NDK_PATH)/code/include
CFLAGS += -I$(NDK_PATH)/code/include/inner

TARGET := $(HIARCH)_mapi_register_sensor
#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../../../../../Make.$(OSTYPE)
