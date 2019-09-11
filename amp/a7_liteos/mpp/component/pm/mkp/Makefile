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
PM_ROOT := $(PWD)/..
ifeq ($(OSTYPE), liteos)
MPI_ROOT := $(PM_ROOT)/mpi
MKP_ROOT := $(PM_ROOT)/mkp
ARCH_ROOT := $(PM_ROOT)/arch/$(HIARCH)
else
MPI_ROOT :=../mpi
MKP_ROOT :=../mkp
ARCH_ROOT :=../arch/$(HIARCH)
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
# all source file in this module

SRCS:=
# the following include sequence can't be modfied
include $(PM_ROOT)/arch/Makefile
# Describe the include path, using the full path

# Enum the C files needed to be compiled, using the relative path
#common src
SRCS += $(MKP_ROOT)/src/pm.c
SRCS += $(MKP_ROOT)/src/pm_dev_api.c
SRCS += $(MKP_ROOT)/src/pm_dev.c
SRCS +=  $(MKP_ROOT)/src/regulator/pm_regulator.c
SRCS += $(MKP_ROOT)/src/dvfs/pm_dvfs.c
SRCS += $(MKP_ROOT)/src/avs/pm_avs.c
SRCS += $(MKP_ROOT)/src/proc/pm_proc.c
SRCS += $(MKP_ROOT)/src/adapt/$(OSTYPE)/pm_dev_adapt.c
SRCS += $(MKP_ROOT)/src/adapt/$(OSTYPE)/regulator/pm_regulator_adapt.c

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

#cpu domain
ifeq ($(CONFIG_PM_DOMAIN_CPU_ON), y)
    #add header & src
    MKP_CFLAGS += -I$(PM_ROOT)/mkp/src/domain_cpu
    SRCS += $(MKP_ROOT)/src/domain_cpu/pm_cpu.c
    SRCS += $(MKP_ROOT)/src/domain_cpu/pm_cpu_avs.c
    # os adapt 
    SRCS += $(MKP_ROOT)/src/adapt/$(OSTYPE)/domain_cpu/pm_cpu_adapt.c
    SRCS += $(MKP_ROOT)/src/adapt/$(OSTYPE)/domain_cpu/pm_cpu_freq.c
    SRCS += $(MKP_ROOT)/src/adapt/$(OSTYPE)/domain_cpu/pm_cpu_regulator.c
    MKP_CFLAGS += -I$(PM_ROOT)/mkp/src/adapt/$(OSTYPE)/domain_cpu
endif

#media domain
ifeq ($(CONFIG_PM_DOMAIN_MEDIA_ON), y)
    #add header & src
    MKP_CFLAGS += -I$(PM_ROOT)/mkp/src/domain_media
    SRCS += $(MKP_ROOT)/src/domain_media/pm_media.c
    SRCS += $(MKP_ROOT)/src/domain_media/pm_media_avs.c

    # os adapt src
    SRCS += $(MKP_ROOT)/src/adapt/$(OSTYPE)/domain_media/pm_media_regulator.c
    MKP_CFLAGS += -I$(PM_ROOT)/mkp/src/adapt/$(OSTYPE)/domain_media
endif

#core domain
ifeq ($(CONFIG_PM_DOMAIN_CORE_ON), y)
    #add header path
    MKP_CFLAGS += -I$(PM_ROOT)/mkp/src/domain_core
    SRCS += $(MKP_ROOT)/src/domain_core/pm_core.c
    SRCS += $(MKP_ROOT)/src/domain_core/pm_core_avs.c

    #add  hal src file
    SRCS += $(MKP_ROOT)/src/adapt/$(OSTYPE)/domain_core/pm_core_regulator.c
    MKP_CFLAGS += -I$(PM_ROOT)/mkp/src/adapt/$(OSTYPE)/domain_core

endif

ifeq ($(MPP_BUILD), y)
INIT_FILE := $(CBB_ROOT)/init/$(HIARCH)/$(OSTYPE)/pm_init.c
else
INIT_FILE := $(PM_ROOT)/init/$(OSTYPE)/pm_init.c
MKP_CFLAGS += -I$(PWD)/../../../mpp_amp_liteos/include
endif
ifeq ($(OSTYPE), liteos)
SRCS += $(INIT_FILE)
else
#SRCS += $(INIT_FILE)
SRCS += ./pm_init.c
endif

#*************************************************************************
# release header
INC_FILE := $(wildcard include/*.h)

MOD_NAME := hi_pm
TARGET   := $(ARCH_LIBNAME)_pm

#*************************************************************************
# compile linux or huawei_liteos
include $(PWD)/Makefile.$(OSTYPE)
