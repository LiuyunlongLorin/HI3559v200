# $(ROOT)/mpp/code/component/hifb/Makefile

ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

HIFB_HEADER     := hifb.h
BASE_MKP_HEADER := mod_ext.h mm_ext.h proc_ext.h
SYS_EXT_HEADER  := sys_ext.h
VO_EXT_HEADER   := vou_ext.h
TDE_EXT_HEADER  := hi_tde_ext.h
TDE_MKP_HEADER  := hi_tde_type.h hi_tde_errcode.h
INIT_HEADER     := hi_module_param.h
OSAL_HEADER     := hi_osal.h osal_list.h osal_ioctl.h osal_mmz.h
ifeq ($(HIARCH),hi3516ev200)
VGS_EXT_HEADER  := vgs_ext.h
VGS_HEADER      := hi_comm_vgs.h
VB_EXT_HEADER   := vb_ext.h
endif
INC_DIR          =$(MPP_PATH)/component/hifb/include

#*******************************************************************************
#********************* Set the source code and include path ********************
MPP_ROOT        := $(SDK_PATH)/mpp
CBB_ROOT        := $(SDK_PATH)/mpp/cbb
COMPONENT_ROOT  := $(SDK_PATH)/mpp/component

# Head file path variable's
CFG_INC += -I$(PWD)/include
CFG_INC += -I$(PWD)/src
CFG_INC += -I$(PWD)/drv/$(HIARCH)
CFG_INC += -I$(PWD)/drv/include
CFG_INC += -I$(REL_INC)
ifeq ($(CONFIG_SYNC_FILE),y)
CFG_INC += -I$(KERNEL_ROOT)/drivers/hi_fence
EXTRA_CFLAGS += -DCONFIG_HIFB_FENCE_SUPPORT
endif
#***************************************************************************#


#********************* List all the depended file ******************************
#** The DEPEND_FILE is not perfect, any modify will conduce to full compile
DEPEND_FILE := $(foreach file,$(subst -I, ,$(CFG_INC)), $(wildcard $(file)/*.h))

#********************* List all the release file *******************************

ifeq ($(HIGDB),y)
EXTRA_CFLAGS += -g
endif

EXTRA_CFLAGS += $(CFG_INC)
EXTRA_CFLAGS += $(MPP_CFLAGS)
EXTRA_CFLAGS += $(DRV_CFLAGS)


HIFB_MOD_NAME:=hifb
MOD_NAME:=hi_fb

#EXTRA_CFLAGS += -Wall -D$(HIARCH) -DHICHIP=$(HICHIP) -D$(HIDBG) -D$(HI_FPGA) -Wno-error=date-time
EXTRA_CFLAGS += -Wall -D$(HIARCH) -D$(HI_FPGA) -Wno-date-time
EXTRA_CFLAGS += -DCURSOR
-include $(PWD)/src/Makefile
-include $(PWD)/drv/$(ARCH_DIR)/Makefile

$(HIFB_MOD_NAME)-objs = $(HIFB_SRC:%.c=%.o)

ifneq ($(KERNELRELEASE),)
    obj-m += hifb.o
endif

# compile linux or HuaweiLite
include $(PWD)/Make.$(OSTYPE)
