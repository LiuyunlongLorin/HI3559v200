# Module build makefile
CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../..
OS_TYPE     := Linux
include $(REF_ROOT)/build/base.mak


MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := devmng

SRC_ROOT    := $(MODULE_ROOT)
SRC_DIR     := $(SRC_ROOT)/src

SRCS_OMIT :=
ifneq ($(CONFIG_WATCHDOG),y)
SRCS_OMIT += $(SRC_DIR)/hi_watchdogmng.c
endif
ifneq ($(CONFIG_LED),y)
SRCS_OMIT += $(SRC_DIR)/hi_ledmng.c
endif
ifneq ($(CONFIG_GAUGE),y)
SRCS_OMIT += $(SRC_DIR)/hi_gaugemng.c
endif
ifneq ($(CONFIG_AHD), y)
SRCS_OMIT += $(SRC_DIR)/hi_ahdmng.c
endif
ifneq ($(CONFIG_ACC), y)
SRCS_OMIT += $(SRC_DIR)/hi_accmng.c
endif

OBJ_ROOT    := $(REF_OUT_COMMON_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
LIB_ROOT    := $(REF_OUT_COMMON_LIB)/$(OS_TYPE)

include $(REF_ROOT)/build/modbuild.mak
