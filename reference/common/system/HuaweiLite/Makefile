# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../../..
OS_TYPE     := HuaweiLite
include $(REF_ROOT)/build/base.mak


MODULE_ROOT := $(CUR_ROOT)/..
MODULE_NAME := system

SRC_ROOT    := $(MODULE_ROOT)/src
SRC_DIR     := $(SRC_ROOT)/

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Source FileFilter
#++++++++++++++++++++++++++++++++++++++++++++++++++++
SRCS_OMIT := $(shell find $(SRC_DIR) -name 'hi_system_hi35*.c')
SRCS_OMIT_OMIT := $(shell find $(SRC_DIR) -name 'hi_system_$(CFG_CHIP_TYPE).c')
ifneq ($(strip $(SRCS_OMIT)),)
SRCS_OMIT := $(filter-out $(SRCS_OMIT_OMIT),$(SRCS_OMIT))
endif

OBJ_ROOT    := $(REF_OUT_COMMON_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
LIB_ROOT    := $(REF_OUT_COMMON_LIB)/$(OS_TYPE)

include $(REF_ROOT)/build/modbuild.mak
