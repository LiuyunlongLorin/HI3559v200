


ifeq ($(EXTDRV_PARAM_FILE), ) 
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif


SRCS = piris.c
#INC := -I$(REL_INC)
#INC += -I./arch/$(ARCH_DIR)/
#CFLAGS += -I$(OSAL_ROOT)/include

INC := -I$(REL_INC)
ifeq ($(MPP_BUILD),y)
	INC += -I$(OSAL_ROOT)/$(OSTYPE)/kernel/himedia
else
	INC += -I$(OSAL_ROOT)/source/kernel/himedia
endif
INC += -I$(ISP_ROOT)/ext_inc
INC += -I$(ISP_ROOT)/kernel/mkp/include
INC += -I$(ISP_ROOT)/kernel/arch/$(HIARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/arch/$(HIARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/include
INC += -I$(DRV_ROOT)/extdrv/$(EXTDRVVER)/piris/arch/$(ARCH_DIR)/
INC += -I$(OSAL_ROOT)/include

EXTDRV_CFLAGS += $(INC)
EXTDRV_CFLAGS += $(COMM_INC)


		
		
#*************************************************************************
TARGET := hi_piris

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)	