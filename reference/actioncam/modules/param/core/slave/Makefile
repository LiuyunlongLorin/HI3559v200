
CUR_ROOT    := $(shell pwd)
PDT_ROOT    ?= $(CUR_ROOT)/../../../..
OS_TYPE     := HuaweiLite
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
MEDIAMODE_CNT = $(shell grep -nr "config_product_mediamode_*" $(PDT_INICFG_PATH)/config_cfgaccess_entry.ini |wc -l)
MPHOTO_MEDIAMODE_CNT = $(shell grep -nr "HI_PDT_MEDIAMODE_.*_MPHOTO*" $(PDT_INICFG_PATH)/config_product_valueset.ini |wc -l)
LDCV2_CFG_CNT   = $(shell grep -nr "config_product_ldcv2cfg_*" $(PDT_INICFG_PATH)/config_cfgaccess_entry.ini |wc -l)
VSS_MOD_CFLAGS := -DPDT_PARAM_MEDIAMODE_CNT=$(MEDIAMODE_CNT)
VSS_MOD_CFLAGS += -DPDT_PARAM_MPHOTO_MEDIAMODE_CNT=$(MPHOTO_MEDIAMODE_CNT)
VSS_MOD_CFLAGS += -DPDT_PARAM_LDCV2_CFG_CNT=$(LDCV2_CFG_CNT)

ifeq ($(CONFIG_MOTIONSENSOR),y)
VSS_MOD_CFLAGS += -DSUPPORT_GYRO
endif

include $(REF_ROOT)/build/modbuild.mak
