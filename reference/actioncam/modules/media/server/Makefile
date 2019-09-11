
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
ifeq ($(AMP_TYPE),linux_liteos)
SRCS_OMIT   :=
SRCS_OMIT   += $(SRC_ROOT)/common/hi_product_media_hdmi.c
endif

OBJ_ROOT    := $(PDT_OUT_MODULES_OBJ)/$(MODULE_NAME)/$(OS_TYPE)
LIB_ROOT    := $(PDT_OUT_MODULES_LIB)/$(OS_TYPE)
LIB_TYPE    := static

include $(REF_ROOT)/build/modbuild.mak
