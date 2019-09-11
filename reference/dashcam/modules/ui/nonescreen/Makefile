###############################################################################
# export variables
###############################################################################
CUR_ROOT    := $(shell pwd)
PDT_ROOT    ?= $(CUR_ROOT)/../../..
OS_TYPE     := Linux
include $(PDT_ROOT)/build/base.mak

MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := $(CUR_DIR_NAME)

SRC_ROOT    := $(MODULE_ROOT)
SRC_DIR     := $(SRC_ROOT)/src

OBJ_ROOT    := $(PDT_OUT_MODULES_OBJ)/$(MODULE_NAME)/$(OS_TYPE)
LIB_ROOT    := $(PDT_OUT_MODULES_LIB)/$(OS_TYPE)
LIB_TYPE    := static


include $(REF_ROOT)/build/modbuild.mak

