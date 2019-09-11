# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../..
OS_TYPE     := Linux
include $(REF_ROOT)/build/base.mak

MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := hal$(CUR_DIR_NAME)
TOUCHPAD_TYPE := ft6356
ifeq ($(CONFIG_TOUCHPAD),y)
	ifeq ($(CONFIG_TOUCHPAD_FT6356),y)
		TOUCHPAD_TYPE := ft6356
	else ifeq  ($(CONFIG_TOUCHPAD_FT6236),y)
		TOUCHPAD_TYPE := ft6236
	else ifeq  ($(CONFIG_TOUCHPAD_GT911),y)
		TOUCHPAD_TYPE := gt911
	endif
endif
SRC_ROOT    := $(MODULE_ROOT)/src
SRC_DIR     := $(SRC_ROOT)/$(TOUCHPAD_TYPE)/
OBJ_ROOT    := $(REF_OUT_HAL_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
LIB_ROOT    := $(REF_OUT_HAL_LIB)/$(OS_TYPE)

include $(REF_ROOT)/build/modbuild.mak
