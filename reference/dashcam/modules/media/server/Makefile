
CUR_ROOT    := $(shell pwd)
PDT_ROOT    ?= $(CUR_ROOT)/../../..
SDK_ROOT    ?= $(PDT_ROOT)/../..
include $(SDK_ROOT)/build/base.mak
ifeq ($(AMP_TYPE),linux)
OS_TYPE     := Linux
else
OS_TYPE     := HuaweiLite
endif
include $(PDT_ROOT)/build/base.mak

MODULE_ROOT := $(CUR_ROOT)/..
MODULE_NAME := $(CUR_PARENT_DIR_NAME)_$(CUR_DIR_NAME)

SRC_ROOT    := $(MODULE_ROOT)
SRC_DIR     := $(SRC_ROOT)/common/
SRC_DIR     += $(SRC_ROOT)/$(CUR_DIR_NAME)/

OBJ_ROOT    := $(PDT_OUT_MODULES_OBJ)/$(MODULE_NAME)/$(OS_TYPE)
LIB_ROOT    := $(PDT_OUT_MODULES_LIB)/$(OS_TYPE)
LIB_TYPE    := static

dat_file_cnt=$(shell ls $(CUR_ROOT)/logo*.dat 2> /dev/null | wc -l)
ifeq ($(dat_file_cnt), 0)
$(shell $(CUR_ROOT)/bmp2osd.sh > /dev/null)
endif

ifneq ($(AMP_TYPE), linux_liteos)
SRCS_OMIT    := $(SRC_ROOT)/$(CUR_DIR_NAME)/product_media_msg.c
endif

include $(REF_ROOT)/build/modbuild.mak
