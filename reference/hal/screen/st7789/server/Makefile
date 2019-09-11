# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../../../..
SDK_ROOT    ?= $(REF_ROOT)/..
include $(SDK_ROOT)/build/base.mak
ifeq ($(AMP_TYPE),linux)
OS_TYPE     := Linux
else
OS_TYPE     := HuaweiLite
endif

include $(REF_ROOT)/build/base.mak


MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := halscreen_st7789_$(CUR_DIR_NAME)

VSS_INC_PATH += -I$(CUR_ROOT)/../../comm/include

SRC_ROOT    := $(MODULE_ROOT)
SRC_DIR     := $(SRC_ROOT)/../

OBJ_ROOT    := $(REF_OUT_HAL_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
LIB_ROOT    := $(REF_OUT_HAL_LIB)/$(OS_TYPE)
LIB_TYPE    := static
include $(REF_ROOT)/build/modbuild.mak
