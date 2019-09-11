# define MEDIA_LIBRARY_PATH and MEDIA_LIBS in different chip dir
makefile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
MEDIA_ADPT_ROOT := $(dir $(makefile_path))
include $(MEDIA_ADPT_ROOT)/$(CFG_CHIP_TYPE)/medialibs.mak
-include $(MEDIA_ADPT_ROOT)/$(CFG_CHIP_TYPE)/medialibs_liteos.mak
MEDIA_INCLUDE_PATH := $(addprefix -I$(MEDIA_ADPT_ROOT)/$(CFG_CHIP_TYPE)/, $(MW_MEDIA_INCLUDE))

MEDIA_INCLUDE_PATH += -I$(MEDIA_ADPT_ROOT)/include

MEDIA_DEPRECATED_INC_PATH := $(addprefix -I$(MEDIA_ADPT_ROOT)/$(CFG_CHIP_TYPE)/, $(MW_MEDIA_DEPRECATED_INC))

MEDIA_LIBS_PATH_LINK := $(addprefix -L$(MEDIA_ADPT_ROOT)/$(CFG_CHIP_TYPE)/, $(MW_MEDIA_LIBRARY_PATH))

MEDIA_LIBS_LINK := $(addprefix -l, $(MW_MEDIA_LIBS))

#some module(such as audio) of 3518EV300 linux version mpp only has so libs
MEDIA_SO_LIBS_LINK := $(addprefix -l, $(MW_MEDIA_SO_LIBS))

MEDIA_LITEOS_LIBS_LINK := $(addprefix -l, $(MW_MEDIA_LIBS_LITEOS))

MEDIA_LITEOS_LIBS_PATH_LINK := $(addprefix -L$(MEDIA_ADPT_ROOT)/$(CFG_CHIP_TYPE)/, $(MW_MEDIA_LIBRARY_PATH_LITEOS))

MEDIA_LITEOS_INCLUDE_PATH := $(addprefix -I$(MEDIA_ADPT_ROOT)/$(CFG_CHIP_TYPE)/, $(MW_MEDIA_INCLUDE_LITEOS))

MEDIA_LITEOS_INCLUDE_PATH += -I$(MEDIA_ADPT_ROOT)/include