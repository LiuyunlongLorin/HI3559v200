ifeq ($(CFG_REL_FOR_PDT),y)
SDK_ROOT              ?= $(MIDDLEWARE_DIR)/..
endif

###############################hi3559###############################################
ifeq ($(CFG_CHIP_TYPE),hi3559)
CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -mno-unaligned-access -fno-aggressive-loop-optimizations
CFLAGS += -Wall -Werror
CFLAGS += -D__HI3559__
OSDRV_ROOT            ?= $(MIDDLEWARE_DIR)/../osdrv
OSDRV_LITEOS_PATH     := $(OSDRV_ROOT)/platform/liteos
export LITEOS_ROOT    := $(OSDRV_LITEOS_PATH)/liteos
endif
###############################hi3556###############################################
ifeq ($(CFG_CHIP_TYPE),hi3556)
CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -mno-unaligned-access -fno-aggressive-loop-optimizations
CFLAGS += -Wall -Werror
CFLAGS += -D__HI3556__
OSDRV_ROOT            ?= $(MIDDLEWARE_DIR)/../osdrv
OSDRV_LITEOS_PATH     := $(OSDRV_ROOT)/platform/liteos
export LITEOS_ROOT    := $(OSDRV_LITEOS_PATH)/liteos
endif
###############################hi3559aes###############################################
ifeq ($(CFG_CHIP_TYPE),hi3559aes)
CFLAGS += -mcpu=cortex-a73.cortex-a53 -mabi=lp64 -Wno-error
CFLAGS += -D__HI3559AES__
endif
###############################hi3518ev200###############################################
ifeq ($(CFG_CHIP_TYPE),hi3518ev200)
CFLAGS += -mcpu=arm926ej-s
CFLAGS += -D__HI3518EV200__
endif
###############################hi3520dv200###############################################
ifeq ($(CFG_CHIP_TYPE),hi3520dv200)
CFLAGS += -march=armv7-a -mcpu=cortex-a9
CFLAGS += -D__HI3520DV200__
endif
###############################hi3536dv100###############################################
ifeq ($(CFG_CHIP_TYPE),hi3536dv100)
CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
CFLAGS += -D__HI3536DV100__
endif
##############################hi3516cv300###############################################
ifeq ($(CFG_CHIP_TYPE),hi3516cv300)
CFLAGS += -mcpu=arm926ej-s -mno-unaligned-access -fno-aggressive-loop-optimizations -Wno-error
CFLAGS += -D__HI3516CV300__
#OS DEFINE
DEP_DIR               := sdk
SDK_ROOT              ?= $(MIDDLEWARE_DIR)/../$(DEP_DIR)
OSDRV_ROOT            := $(SDK_ROOT)/osdrv
OSDRV_LITEOS_PATH     := $(OSDRV_ROOT)/opensource/liteos
export LITEOS_ROOT    := $(OSDRV_LITEOS_PATH)/liteos
#OSAL DEFINE
OSAL_ROOT             := $(MIDDLEWARE_DIR)/../osal
export OSAL_ROOT
endif
##############################hi3559av100###############################################
ifeq ($(CFG_CHIP_TYPE),hi3559av100)
CFLAGS += -mabi=lp64
CFLAGS += -Wall -Werror
CFLAGS += -D__HI3559AV100__
#OS DEFINE
DEP_DIR               := sdk
SDK_ROOT              ?= $(MIDDLEWARE_DIR)/../$(DEP_DIR)
OSDRV_ROOT            := $(SDK_ROOT)/osdrv
OSDRV_LITEOS_PATH     := $(OSDRV_ROOT)/platform/liteos_a53
export LITEOS_ROOT    := $(OSDRV_LITEOS_PATH)/liteos
endif
##############################hi3556av100###############################################
ifeq ($(CFG_CHIP_TYPE),hi3556av100)
CFLAGS += -mcpu=cortex-a53
CFLAGS += -Wall -Werror
CFLAGS += -D__HI3556AV100__
#OS DEFINE
DEP_DIR               := sdk
SDK_ROOT              ?= $(MIDDLEWARE_DIR)/../$(DEP_DIR)
OSDRV_ROOT            := $(SDK_ROOT)/osdrv
OSDRV_LITEOS_PATH     := $(OSDRV_ROOT)/platform/liteos
export LITEOS_ROOT    := $(OSDRV_LITEOS_PATH)
endif
##############################hi3519av100###############################################
ifeq ($(CFG_CHIP_TYPE),hi3519av100)
CFLAGS += -mcpu=cortex-a53
CFLAGS += -Wall -Werror
CFLAGS += -D__HI3519AV100__
#OS DEFINE
DEP_DIR               := sdk
SDK_ROOT              ?= $(MIDDLEWARE_DIR)/../$(DEP_DIR)
OSDRV_ROOT            := $(SDK_ROOT)/osdrv
OSDRV_LITEOS_PATH     := $(OSDRV_ROOT)/platform/liteos
export LITEOS_ROOT    := $(OSDRV_LITEOS_PATH)
endif
##############################hi3559v200###############################################
ifeq ($(CFG_CHIP_TYPE),hi3559v200)
CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
CFLAGS += -Wall -Werror
CFLAGS += -D__HI3559V200__
#OS DEFINE
DEP_DIR               := sdk
SDK_ROOT              ?= $(MIDDLEWARE_DIR)/../$(DEP_DIR)
OSDRV_ROOT            := $(SDK_ROOT)/osdrv
OSDRV_LITEOS_PATH     := $(OSDRV_ROOT)/platform/liteos
export LITEOS_ROOT    := $(OSDRV_LITEOS_PATH)
endif
##############################hi3516cv500###############################################
ifeq ($(CFG_CHIP_TYPE),hi3516cv500)
CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
CFLAGS += -Wall -Wno-error
CFLAGS += -D__HI3516CV500__
#OS DEFINE
DEP_DIR               := sdk
SDK_ROOT              ?= $(MIDDLEWARE_DIR)/../$(DEP_DIR)
OSDRV_ROOT            := $(SDK_ROOT)/osdrv
endif
##############################hi3518ev300###############################################
ifeq ($(CFG_CHIP_TYPE),hi3518ev300)
CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
CFLAGS += -Wall -Wno-error
CFLAGS += -D__HI3518EV300__
LITEOS_LIBS = --start-group $(LITEOS_LIBDEP) --end-group $(LITEOS_TABLES_LDFLAGS)
#OS DEFINE
DEP_DIR               := sdk
SDK_ROOT              ?= $(MIDDLEWARE_DIR)/../$(DEP_DIR)
OSDRV_ROOT            := $(SDK_ROOT)/osdrv
OSDRV_LITEOS_PATH     := $(OSDRV_ROOT)/platform/liteos
export LITEOS_ROOT    := $(OSDRV_LITEOS_PATH)/liteos
#OSAL DEFINE
OSAL_ROOT             := $(MIDDLEWARE_DIR)/../osal
export OSAL_ROOT
endif
##############################hix86###############################################
ifeq ($(CFG_CHIP_TYPE),hix86)
CFLAGS += -Wall -Werror
CFLAGS += -D__HIX86__
# invalid-offsetof is happen in cbb_list
CFLAGS += -Wno-error=invalid-offsetof
endif
