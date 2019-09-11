# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../..
OS_TYPE     := Linux
include $(REF_ROOT)/build/base.mak

MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := $(CUR_DIR_NAME)

SRC_ROOT    := $(MODULE_ROOT)/src
SRC_DIR     := $(SRC_ROOT)
ifeq ($(AMP_TYPE), liteos)
SRCS_OMIT   := $(SRC_DIR)/Linux/hi_uvc_gadget.c
else
SRCS_OMIT   := $(SRC_DIR)/HuaweiLite/hi_uvc_gadget.c
endif

OBJ_ROOT    := $(REF_OUT_COMMON_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
LIB_ROOT    := $(REF_OUT_COMMON_LIB)/$(OS_TYPE)

include $(REF_ROOT)/build/modbuild.mak
