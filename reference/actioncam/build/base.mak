ifeq ($(PDT_BASECFG),)
PDT_BASECFG = 1

ifeq ($(PDT_ROOT),)
$(error please specify reference path)
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Reference Product Define
#++++++++++++++++++++++++++++++++++++++++++++++++++++
PDT_ROOT               := $(shell cd $(PDT_ROOT); pwd)
REF_ROOT               := $(shell cd $(PDT_ROOT)/..; pwd)
include $(REF_ROOT)/build/base.mak
PDT_MODULE_PATH        := $(PDT_ROOT)/modules
PDT_PRODUCT_PATH       := $(PDT_ROOT)/product/$(CFG_CHIP_TYPE)_$(CFG_BOARD_TYPE)
ifeq ($(CONFIG_PARAM_DIR_SUFFIX),)
PDT_INICFG_PATH        := $(PDT_MODULE_PATH)/param/inicfg/$(CFG_CHIP_TYPE)/$(CFG_SENSOR_TYPE0)_$(CONFIG_MEM_TOTAL_SIZE)M
else ifeq ($(CONFIG_PARAM_DIR_SUFFIX),"")
PDT_INICFG_PATH        := $(PDT_MODULE_PATH)/param/inicfg/$(CFG_CHIP_TYPE)/$(CFG_SENSOR_TYPE0)_$(CONFIG_MEM_TOTAL_SIZE)M
else
PDT_INICFG_PATH        := $(PDT_MODULE_PATH)/param/inicfg/$(CFG_CHIP_TYPE)/$(CFG_SENSOR_TYPE0)_$(CONFIG_MEM_TOTAL_SIZE)M_$(CONFIG_PARAM_DIR_SUFFIX)
endif
PDT_SCENE_INICFG_PATH  := $(PDT_MODULE_PATH)/scene/inicfg/$(CFG_CHIP_TYPE)/$(CFG_SENSOR_TYPE0)

# Product Out Definition
PDT_OUT_ROOT           := $(REF_OUT_ROOT)/$(CFG_CHIP_TYPE)_$(CFG_PDT_TYPE)_$(CFG_BOARD_TYPE)_$(CFG_SENSOR_TYPE0)
PDT_OUT_MODULES        := $(PDT_OUT_ROOT)/modules
PDT_OUT_MODULES_OBJ    := $(PDT_OUT_MODULES)/obj
PDT_OUT_MODULES_LIB    := $(PDT_OUT_MODULES)/lib
PDT_OUT_MODULES_BIN    := $(PDT_OUT_MODULES)/bin

PDT_OUT_LIB            := $(PDT_OUT_ROOT)/lib
PDT_OUT_LIB_LINUX      := $(PDT_OUT_LIB)/Linux
PDT_OUT_LIB_HUAWEILITE := $(PDT_OUT_LIB)/HuaweiLite

PDT_OUT_BIN            := $(PDT_OUT_ROOT)/bin/$(FLASH_TYPE)
PDT_OUT_BURN           := $(PDT_OUT_ROOT)/burn/$(FLASH_TYPE)

include $(PDT_MODULE_PATH)/inc.mak

dummy := $(call CreateDir, $(PDT_OUT_BIN))
dummy += $(call CreateDir, $(PDT_OUT_BURN))
dummy += $(call CreateDir, $(PDT_OUT_LIB_LINUX))
dummy += $(call CreateDir, $(PDT_OUT_LIB_HUAWEILITE))

endif # ifndef $(PDT_BASECFG)
