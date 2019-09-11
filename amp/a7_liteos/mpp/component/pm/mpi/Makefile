#*************************************************************************
ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../Makefile.param
    include $(PARAM_FILE)
endif
ifeq ($(MPP_BUILD), n)
ifeq ($(CBB_PARAM_FILE), )
    CBB_PARAM_FILE:=../../Makefile.param
    include $(CBB_PARAM_FILE)
endif
endif
#*************************************************************************
# cbb header
CBB_ROOT := $(SDK_PATH)/mpp/cbb
COMPONENT_ROOT := $(SDK_PATH)/mpp/component

# local path
PM_ROOT := $(PWD)/../
ifeq ($(OSTYPE), liteos)
MPI_ROOT := $(PM_ROOT)/mpi
else
MPI_ROOT :=../mpi
endif

# Head file path variable's
REL_KINC  := $(REL_INC)/include
COMMON_INC += -I$(CBB_ROOT)/include \
           -I$(CBB_ROOT)/init/$(HIARCH)/$(OSTYPE) \
	   -I$(CBB_ROOT)/init/include/$(OSTYPE) \
           -I$(CBB_ROOT)/sys/include \
		   -I$(CBB_ROOT)/sys/include/inner \
           -I$(CBB_ROOT)/sys/ext_inc \
           -I$(CBB_ROOT)/sys/arch/include \
           -I$(CBB_ROOT)/sys/arch/$(HIARCH)/include \
           -I$(CBB_ROOT)/sys/mkp/include \
           -I$(CBB_ROOT)/base/include/inner \
           -I$(CBB_ROOT)/base/include \
           -I$(CBB_ROOT)/base/ext_inc \
           -I$(CBB_ROOT)/base/arch/include \
           -I$(CBB_ROOT)/base/arch/$(HIARCH)/include \
           -I$(CBB_ROOT)/base/mkp/include \
	   -I$(CBB_ROOT)/../component/tde/driver/include \
	   -I$(CBB_ROOT)/../component/tde/driver/src/include \
	   -I$(CBB_ROOT)/../component/tde/include \
	   -I$(CBB_ROOT)/../component/hdmi/src/include \
	   -I$(CBB_ROOT)/../../osal/$(OSTYPE)/kernel/mmz \
	   -I$(CBB_ROOT)/../../osal/include \
	   -I$(CBB_ROOT)/../../osal/$(OSTYPE)/kernel/himedia \
	   -I$(PM_ROOT)/../securec/include
#***************************************************************************#

#********************* List all the depended file ******************************
#** The DEPEND_FILE is not perfect, any modify will conduce to full compile
DEPEND_FILE := $(foreach file,$(subst -I, ,$(COMMON_INC)), $(wildcard $(file)/*.h))

#********************* List all the release file *******************************
# REL_FILE := $(shell find $(CODE_ROOT)/include -name "*.h")
#REL_FILE += $(CODE_ROOT)/mkp/include/mod_ext.h
REL_FILE += $(CBB_ROOT)/base/ext_inc/mod_ext.h

#*************************************************************************
# local header MKP_CFLAGS hi3519av100 $(HIARCH
MKP_CFLAGS +=-I $(PM_ROOT)/arch/include
MKP_CFLAGS +=-I $(PM_ROOT)/include
MKP_CFLAGS +=-I $(PM_ROOT)/ext_inc
MKP_CFLAGS += $(COMMON_INC)

#*************************************************************************
#common src header path
MKP_CFLAGS += -I $(PM_ROOT)/mkp/include
MKP_CFLAGS += -I $(PM_ROOT)/mkp/src
MKP_CFLAGS += -I $(PM_ROOT)/mkp/src/regulator
MKP_CFLAGS += -I $(PM_ROOT)/mkp/src/dvfs
MKP_CFLAGS += -I $(PM_ROOT)/mkp/src/avs
MKP_CFLAGS += -I $(PM_ROOT)/mkp/src
MKP_CFLAGS += -I $(PM_ROOT)/mkp/src/proc
MKP_CFLAGS += -I $(PM_ROOT)/mkp/src/adapt/$(OSTYPE)
MKP_CFLAGS += -I $(PM_ROOT)/mkp/src/adapt/$(OSTYPE)/regulator

# all source file in this module

SRCS:=
SRCS += $(MPI_ROOT)/mpi_pm.c

MOD_NAME := hi_pm
TARGET   := pm
#*************************************************************************
# compile linux or huawei_liteos
include $(PWD)/Makefile.$(OSTYPE)
