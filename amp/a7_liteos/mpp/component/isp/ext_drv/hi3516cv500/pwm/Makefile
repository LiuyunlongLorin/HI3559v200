
ifeq ($(EXTDRV_PARAM_FILE), ) 
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif



#SRCS = $(wildcard *.c)
SRCS = pwm.c
INC := -I$(REL_INC)
ifeq ($(MPP_BUILD),y)
	INC += -I$(OSAL_ROOT)/$(OSTYPE)/kernel/himedia
else
	INC += -I$(OSAL_ROOT)/source/kernel/himedia
endif
INC += -I$(OSAL_ROOT)/include

INC += -I$(DRV_ROOT)/extdrv/$(EXTDRVVER)/pwm/arch/$(ARCH_DIR)/
EXTDRV_CFLAGS += $(INC)
EXTDRV_CFLAGS += $(COMM_INC)




		
		
#*************************************************************************
TARGET := hi_pwm

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)

	