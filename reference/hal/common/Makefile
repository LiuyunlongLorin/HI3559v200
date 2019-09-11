# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../..
OS_TYPE     := Linux
include $(REF_ROOT)/build/base.mak

MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := hal$(CUR_DIR_NAME)

SRC_ROOT    := $(MODULE_ROOT)/src
SRC_DIR     := $(SRC_ROOT)
OBJ_ROOT    := $(REF_OUT_HAL_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
LIB_ROOT    := $(REF_OUT_HAL_LIB)/$(OS_TYPE)

ifneq ($(CONFIG_ADC),y)
SRCS_OMIT   := $(SRC_ROOT)/hal_adc.c
endif

include $(REF_ROOT)/build/modbuild.mak