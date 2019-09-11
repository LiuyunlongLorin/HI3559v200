
CUR_ROOT    := $(shell pwd)
PDT_ROOT    ?= $(CUR_ROOT)/../../../..
OS_TYPE     := Linux
include $(PDT_ROOT)/build/base.mak

MODULE_ROOT := $(shell cd $(CUR_ROOT)/..; pwd)
MODULE_NAME := param_$(CUR_DIR_NAME)

SRC_ROOT    := $(MODULE_ROOT)
SRC_DIR     := $(SRC_ROOT)/common
SRC_DIR     += $(CUR_ROOT)

OBJ_ROOT    := $(PDT_OUT_MODULES_OBJ)/$(CUR_PARENT_DIR_NAME)/$(OS_TYPE)
LIB_ROOT    := $(PDT_OUT_MODULES_LIB)/$(OS_TYPE)

LIB_TYPE    := static

# MeidaMode Count
MEDIAMODE_CNT = $(shell grep -nr "config_product_mediamode_*" $(PDT_INICFG_PATH)/config_cfgaccess_entry.ini |grep -v comm |wc -l)
VSS_MOD_CFLAGS := -DPDT_PARAM_MEDIAMODE_CNT=$(MEDIAMODE_CNT)

include $(REF_ROOT)/build/modbuild.mak
