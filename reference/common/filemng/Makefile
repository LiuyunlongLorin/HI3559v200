# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../..
OS_TYPE     := Linux
include $(REF_ROOT)/build/base.mak


MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := $(CUR_DIR_NAME)

SRC_ROOT    := $(MODULE_ROOT)/src
SRC_DIR     := $(SRC_ROOT)
SRCS_OMIT   :=
ifneq ($(CONFIG_DCF),y)
SRCS_OMIT   += $(SRC_ROOT)/hi_filemng_dcf.c
else
MODULE_NAME := $(CUR_DIR_NAME)_dcf
endif
ifneq ($(CONFIG_DTCF),y)
SRCS_OMIT   += $(SRC_ROOT)/hi_filemng_dtcf.c
else
MODULE_NAME := $(CUR_DIR_NAME)_dtcf
endif

OBJ_ROOT    := $(REF_OUT_COMMON_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
LIB_ROOT    := $(REF_OUT_COMMON_LIB)/$(OS_TYPE)

include $(REF_ROOT)/build/modbuild.mak
