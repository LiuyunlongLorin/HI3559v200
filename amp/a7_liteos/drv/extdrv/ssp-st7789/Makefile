
ifeq ($(EXTDRV_PARAM_FILE), ) 
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

#SRCS = $(wildcard *.c)
SRCS := ssp_st7789.c 

INC := -I$(REL_INC)
#INC += -I./arch/$(ARCH_DIR)/
INC += -I$(OSAL_ROOT)/include

EXTDRV_CFLAGS += $(INC)
	
#*************************************************************************
TARGET := hi_ssp_st7789

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)

