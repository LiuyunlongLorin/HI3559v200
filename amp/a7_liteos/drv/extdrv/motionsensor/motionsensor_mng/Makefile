
ifeq ($(EXTDRV_PARAM_FILE), ) 
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif



#SRCS = $(wildcard *.c)
SRCS += ./src/motionsensor_exe.c
SRCS += ./src/motionsensor_proc.c
SRCS += ./src/motionsensor_buf.c
#SRCS += ./init/$(OSTYPE)/motionsensor_buf.c
#INC += -I./arch/$(ARCH_DIR)/
INC := -I$(REL_INC)
INC += -I$(OSAL_ROOT)/include
INC += -I$(CBB_ROOT)/include
#INC += -I$(CBB_ROOT)/base/arch/$(HIARCH)/include
#INC += -I$(CBB_ROOT)/base/ext_inc
#INC += -I$(CBB_ROOT)/sys/ext_inc
ifeq ($(MPP_BUILD),y)
	INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/include/
	INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_mng/include
	INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_chip/pub
else
	INC += -I$(DRV_ROOT)/extdrv/motionsensor/include/
	INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_mng/include
	INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_chip/pub
endif

ifeq ($(MPP_BUILD),y)
        INC += -I$(OSAL_ROOT)/$(OSTYPE)/kernel/himedia
else
        INC += -I$(OSAL_ROOT)/source/kernel/himedia
endif
EXTDRV_CFLAGS += $(INC)



		

#*************************************************************************
TARGET := motionsensor_mng

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../../Make.$(OSTYPE)

