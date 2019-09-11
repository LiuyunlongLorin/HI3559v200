LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../../../../../..
MC_PARAM_FILE:=../../../../../Makefile.param
include $(MC_PARAM_FILE)

SRCS := comm_sensor_intf_cfg_params.c

ARFLAGS = rcv
ARFLAGS_SO = $(LIBS_LD_CFLAGS)
ARFLAGS_SO += -shared -fPIC -o
CFLAGS  += -Wall -fPIC
CFLAGS  += -O2
CFLAGS  += $(LIBS_CFLAGS)
CFLAGS  += -I $(SENSOR_CFG_PATH)/include
CFLAGS  += -I $(REL_INC)
CFLAGS  += -I $(MPP_HEADER_LITEOS)
CFLAGS  += -I$(NDK_PATH)/code/mediaserver/utils
CFLAGS += -I$(NDK_PATH)/code/include
CFLAGS += -I$(NDK_PATH)/code/include/inner

TARGET := $(HIARCH)_mapi_comm_sensor_cfg
#*************************************************************************
# compile linux or HuaweiLite
include $(NDK_PATH)/code/mediaserver/Make.$(OSTYPE)
