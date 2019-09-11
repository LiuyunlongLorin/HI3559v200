# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../..
OS_TYPE     := Linux
include $(REF_ROOT)/build/base.mak

MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := hal$(CUR_DIR_NAME)
ifeq ($(CONFIG_DEMB), y)
    BOARD_SRC_DIR := $(CFG_CHIP_TYPE)_demb
else
    BOARD_SRC_DIR := $(CFG_CHIP_TYPE)_$(CFG_PDT_TYPE)_$(CFG_BOARD_TYPE)
endif
SRC_ROOT    := $(MODULE_ROOT)/src/$(BOARD_SRC_DIR)
SRC_DIR     := $(SRC_ROOT)
OBJ_ROOT    := $(REF_OUT_HAL_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
LIB_ROOT    := $(REF_OUT_HAL_LIB)/$(OS_TYPE)
include $(REF_ROOT)/build/modbuild.mak