# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../..
OS_TYPE     := HuaweiLite
include $(REF_ROOT)/build/base.mak

MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := hal$(CUR_DIR_NAME)
HARDWARE_TYPE:= n3
ifeq ($(CONFIG_AHD),y)
	ifeq ($(CONFIG_AHD_N3),y)
		HARDWARE_TYPE:= n3
	endif
	SRC_ROOT    := $(MODULE_ROOT)
	SRC_DIR     := $(SRC_ROOT)/$(HARDWARE_TYPE)/
	OBJ_ROOT    := $(REF_OUT_HAL_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
	LIB_ROOT    := $(REF_OUT_HAL_LIB)/$(OS_TYPE)
	VSS_CFLAGS +=-D_GNU_SOURCE
	include $(REF_ROOT)/build/modbuild.mak
endif