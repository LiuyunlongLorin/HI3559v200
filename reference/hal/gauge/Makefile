# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../..
OS_TYPE     := Linux
include $(REF_ROOT)/build/base.mak

MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := hal$(CUR_DIR_NAME)
HARDWARE_TYPE:= bq27421
ifeq ($(CONFIG_GAUGE),y)
	ifeq ($(CONFIG_GAUGE_BQ27421),y)
		HARDWARE_TYPE:= bq27421
		VSS_INC_PATH += -I$(SDK_ROOT)/drv/extdrv/gauge/bq27421/
	endif
	ifeq ($(CONFIG_GAUGE_ADC),y)
		HARDWARE_TYPE:= adc
	endif
	SRC_ROOT    := $(MODULE_ROOT)/src
	SRC_DIR     := $(SRC_ROOT)/$(HARDWARE_TYPE)/
	OBJ_ROOT    := $(REF_OUT_HAL_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
	LIB_ROOT    := $(REF_OUT_HAL_LIB)/$(OS_TYPE)
	VSS_CFLAGS +=-D_GNU_SOURCE

	include $(REF_ROOT)/build/modbuild.mak
endif
