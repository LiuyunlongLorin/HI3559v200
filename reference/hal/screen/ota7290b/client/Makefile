# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../../../..
OS_TYPE     := Linux
include $(REF_ROOT)/build/base.mak


MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := halscreen_ota7290b_$(CUR_DIR_NAME)

VSS_INC_PATH += -I$(CUR_ROOT)/../../comm/include

SRC_ROOT    := $(MODULE_ROOT)
SRC_DIR     := $(SRC_ROOT)/../


OBJ_ROOT    := $(REF_OUT_HAL_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
LIB_ROOT    := $(REF_OUT_HAL_LIB)/$(OS_TYPE)
LIB_TYPE    := static
include $(REF_ROOT)/build/modbuild.mak
