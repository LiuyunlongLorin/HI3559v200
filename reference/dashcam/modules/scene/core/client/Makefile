# Module build makefile
CUR_ROOT    := $(shell pwd)
PDT_ROOT    ?= $(CUR_ROOT)/../../../..
OS_TYPE     := Linux
include $(PDT_ROOT)/build/base.mak

MODULE_ROOT := $(CUR_ROOT)/..
MODULE_NAME := scene_client

SRC_ROOT    := $(REF_ROOT)
SRC_DIR     := $(CUR_ROOT)

VSS_INC_PATH += -I$(CUR_ROOT)/../core/server

OBJ_ROOT    := $(PDT_OUT_MODULES_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
LIB_ROOT    := $(PDT_OUT_MODULES_LIB)/$(OS_TYPE)
LIB_TYPE    := static

PIPETYPE_CNT    = $(shell grep -nr "config_product_scene_*" $(PDT_SCENE_INICFG_PATH)/config_cfgaccess_entry.ini |wc -l)
VSS_MOD_CFLAGS := -DPDT_SCENE_PIPETYPE_CNT=$(PIPETYPE_CNT) 

include $(REF_ROOT)/build/modbuild.mak
