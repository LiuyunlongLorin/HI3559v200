
ifeq ($(EXTDRV_PARAM_FILE), )
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

ifeq ($(MPP_BUILD),n)
include $(MPP_PATH)/component/Makefile.param
endif

# SRCS = $(wildcard *.c)
SRCS := serdes.c
#INC := -I$(REL_INC)
#INC += -I./arch/$(ARCH_DIR)/
INC += -I$(OSAL_ROOT)/include
INC += -I$(DRV_ROOT)/extdrv/$(EXTDRVVER)/serdes/arch/$(ARCH_DIR)/
INC += -I$(CBB_ROOT)/base/ext_inc
INC += -I$(CBB_ROOT)/vi/include
INC += -I$(ISP_ROOT)/ext_inc
INC += -I$(ISP_ROOT)/include
INC += -I$(ISP_ROOT)/include/arch/$(HIARCH)/include
INC += -I$(ISP_ROOT)/kernel/arch/$(HIARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/arch/$(HIARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/include
INC += -I$(ISP_ROOT)/kernel/mkp/include
ifeq ($(MPP_BUILD),y)
        INC += -I$(OSAL_ROOT)/$(OSTYPE)/kernel/himedia
else
        INC += -I$(OSAL_ROOT)/source/kernel/himedia
        INC += -I$(MPP_PATH)/include
endif
EXTDRV_CFLAGS += $(INC)





#*************************************************************************
TARGET := hi_serdes

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)

