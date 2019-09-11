
ifeq ($(EXTDRV_PARAM_FILE), ) 
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

SRCS := hisi_spi.c

INC := -I$(REL_INC)
INC += -I$(OSAL_ROOT)/include
EXTDRV_CFLAGS += $(INC)
EXTDRV_CFLAGS += $(COMM_INC)


		
#*************************************************************************
TARGET := hi_spi

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../../Make.$(OSTYPE)

