#*************************************************************************
ifeq ($(PARAM_FILE), ) 
    PARAM_FILE:=../../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(CBB_PARAM_FILE), ) 
    CBB_PARAM_FILE:=../../Makefile.param
    include $(CBB_PARAM_FILE)
endif
#*************************************************************************
# local header include

ISP_KERNEL_ROOT := $(ISP_ROOT)
MKP_CFLAGS+=-I $(PWD)/mkp/include
MKP_CFLAGS+=-I $(PWD)/arch/include
MKP_CFLAGS+=-I $(PWD)/arch/$(HIARCH)/include
MKP_CFLAGS+=-I $(ISP_KERNEL_ROOT)/include/arch/$(HIARCH)/include
MKP_CFLAGS+=-I $(ISP_KERNEL_ROOT)/user/firmware/include
MKP_CFLAGS+=-I $(ISP_KERNEL_ROOT)/user/firmware/arch/$(HIARCH)/include
#*************************************************************************
# all source file in this module
SRCS:=

# src relative path
CBB_ISP_ROOT=./
CBB_ISP_ARCH=arch/$(HIARCH)/

# Makefile absolute path

include $(ISP_KERNEL_ROOT)/kernel/mkp/src/Makefile
include $(ISP_KERNEL_ROOT)/kernel/arch/$(HIARCH)/hal/Makefile

INIT_FILE := ./init/$(OSTYPE)/isp_init.c

SRCS_O := $(SRCS)
SRCS   += $(INIT_FILE)

#*************************************************************************
# release header
#INC_FILE  := $(wildcard $(ISP_ROOT)/include/*.h) 
#INC_FILE  += $(wildcard include/*.h) 
#INC_FILE  += $(wildcard ../region/*.h) 
#INC_FILE  += ../region/region_ext.h
#INC_FILE  += $(wildcard )
#INC_FILE  += $(CBB_BASE_ARCH)/include/hi_defines.h

MOD_NAME := hi_isp

#*************************************************************************
TARGET := $(ARCH_LIBNAME)_isp

#*************************************************************************
# compile linux or huawei_liteos
include $(PWD)/../../Makefile.$(OSTYPE)

