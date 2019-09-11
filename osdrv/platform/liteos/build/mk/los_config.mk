-include $(LITEOSTOPDIR)/.config
ifeq ($(LOSCFG_COMPILER_HIMIX100_32), y)
CROSS_COMPILE  := arm-himix100-linux-
COMPILE_NAME  := arm-himix100-linux
VERSION_NUM  := 6.3.0
else ifeq ($(LOSCFG_COMPILER_HIMIX200_32), y)
CROSS_COMPILE  := arm-himix200-linux-
COMPILE_NAME  := arm-himix200-linux
VERSION_NUM  := 6.3.0
else ifeq ($(LOSCFG_COMPILER_LITEOS_32), y)
CROSS_COMPILE  := arm-liteos-eabi-
COMPILE_NAME  := arm-liteos-eabi
VERSION_NUM  := 6.3.0
else ifeq ($(LOSCFG_COMPILER_CORTEX_M7), y)
CROSS_COMPILE := arm-none-eabi-
COMPILE_NAME := arm-none-eabi
VERSION_NUM := 4.9.3
else ifeq ($(LOSCFG_COMPILER_HIMIX100_64), y)
CROSS_COMPILE = aarch64-himix100-linux-
COMPILE_NAME = aarch64-himix100-linux
VERSION_NUM = 6.3.0
endif

HIDE := @
CC = $(CROSS_COMPILE)gcc
AS  = $(CROSS_COMPILE)as
AR  = $(CROSS_COMPILE)ar
LD  = $(CROSS_COMPILE)ld
GPP = $(CROSS_COMPILE)g++
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size
NM = $(CROSS_COMPILE)nm
MKDIR = mkdir
OBJ_MKDIR = if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
RM = -rm -rf
ECHO = echo
OS ?=$(shell uname -s)



ARFLAGS  := cr

## platform relative ##
LITEOS_ARM_ARCH :=
LITEOS_CPU_TYPE :=
## c as cxx ld options ##
LITEOS_ASOPTS :=
LITEOS_COPTS_BASE :=
LITEOS_COPTS_EXTRA :=
LITEOS_COPTS_EXTRA_INTERWORK :=
LITEOS_COPTS_DEBUG :=
LITEOS_COPTS_NODEBUG :=
LITEOS_INTERWORK :=
LITEOS_CXXOPTS :=
LITEOS_CXXOPTS_BASE :=
LITEOS_LD_OPTS :=
LITEOS_GCOV_OPTS :=
## dynload ld options ##
LITEOS_DYNLOADOPTS :=
## macro define ##
LITEOS_CMACRO :=
LITEOS_CMACRO_TEST :=
LITEOS_CXXMACRO :=
## head file path and ld path ##
LITEOS_PLATFORM_INCLUDE :=
LITEOS_CXXINCLUDE :=
LITEOS_LD_PATH :=
LITEOS_LD_SCRIPT :=
LITEOS_MK_PATH :=
## c as cxx ld flags ##
LITEOS_ASFLAGS :=
LITEOS_CFLAGS :=
LITEOS_CFLAGS_INTERWORK :=
LITEOS_LDFLAGS :=
LITEOS_CXXFLAGS :=
## depended lib ##
LITEOS_BASELIB :=
LITEOS_LIBDEP :=
## directory ##
LIB_BIGODIR :=
LIB_SUBDIRS :=
##compiler##
LITEOS_COMPILER_PATH :=
LITEOS_COMPILER_CXX_PATH :=
LITEOS_COMPILER_CXXLIB_PATH :=
LITEOS_COMPILER_GCCLIB_PATH  :=

## variable define ##
OUT  = $(LITEOSTOPDIR)/out/$(LITEOS_PLATFORM)
BUILD  = $(OUT)/obj
MK_PATH  = $(LITEOSTOPDIR)/build/mk
CXX_PATH  = $(LITEOSTOPDIR)/lib/cxxstl
JFFS_PATH  = $(LITEOSTOPDIR)/fs/jffs2
LITEOS_SCRIPTPATH ?= $(LITEOSTOPDIR)/tools/scripts
LITEOS_LIB_BIGODIR  = $(OUT)/lib/obj
LOSCFG_ENTRY_SRC    = $(LITEOSTOPDIR)/platform/bsp/common/los_config.c

### include variable
MODULE = $(MK_PATH)/module.mk
ifeq ($(LOSCFG_COMPILER_HIMIX100_32), y)
LITEOS_CMACRO      += -D__COMPILER_HUAWEILITEOS__
else ifeq ($(LOSCFG_COMPILER_HIMIX200_32), y)
LITEOS_CMACRO      += -D__COMPILER_HUAWEILITEOS__
else ifeq ($(LOSCFG_COMPILER_LITEOS_32), y)
LITEOS_CMACRO      += -D__COMPILER_HUAWEILITEOS__
else ifeq ($(LOSCFG_COMPILER_HIMIX100_64), y)
LITEOS_CMACRO      += -D__COMPILER_HUAWEILITEOS__
endif
LITEOS_CMACRO      += -D__LITEOS__
LITEOS_BASELIB  += -lgcc
ifneq ($(LOSCFG_ARCH_CORTEX_M7), y)
LITEOS_BASELIB  += -lgcc_eh
endif
AS_OBJS_LIBC_FLAGS  = -D__ASSEMBLY__

-include $(LITEOSTOPDIR)/build/mk/dynload_ld.mk
####################################### Chip and CPU Option Begin #########################################
ifeq ($(LOSCFG_PLATFORM_HI3516A), y)
    LITEOS_CMACRO_TEST += -DTEST3516A
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
    LITEOS_PLATFORM    := hi3516a
    LITEOS_CPU_TYPE    := cortex-a7
    LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A7

else ifeq ($(LOSCFG_PLATFORM_HI3518EV200), y)
    LITEOS_CMACRO_TEST += -DTEST3518E
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
    LITEOS_PLATFORM    := hi3518ev200
    LITEOS_CPU_TYPE    := arm926
    LITEOS_CMACRO      += -DLOSCFG_ARCH_ARM926

else ifeq ($(LOSCFG_PLATFORM_HI3516CV300), y)
    LITEOS_CMACRO_TEST += -DTEST3516CV300
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
    LITEOS_PLATFORM    := hi3516cv300
    LITEOS_CPU_TYPE    := arm926
    LITEOS_CMACRO      += -DLOSCFG_ARCH_ARM926

else ifeq ($(LOSCFG_PLATFORM_HIM5V100), y)
    LITEOS_CMACRO_TEST += -DTESTM5V100
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
    LITEOS_PLATFORM    := him5v100
    LITEOS_CPU_TYPE    := arm926
    WARNING_AS_ERROR   :=
    LITEOS_CMACRO      += -DLOSCFG_ARCH_ARM926

else ifeq ($(LOSCFG_PLATFORM_HI3911), y)
    LITEOS_CMACRO_TEST += -DTEST3911
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
    LITEOS_PLATFORM    := hi3911
    LITEOS_CPU_TYPE    := arm926
    WARNING_AS_ERROR   :=
    LITEOS_CMACRO      += -DLOSCFG_ARCH_ARM926

else ifeq ($(LOSCFG_PLATFORM_HI3519), y)
    ifeq ($(LOSCFG_ARCH_CORTEX_A7), y)
        LITEOS_CMACRO_TEST += -DTEST3519
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3519/cortex-a7
        LITEOS_CPU_TYPE    := cortex-a7
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A7
    else ifeq ($(LOSCFG_ARCH_CORTEX_A17), y)
        LITEOS_CMACRO_TEST += -DTEST3519
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3519/cortex-a17
        LITEOS_CPU_TYPE    := cortex-a17
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A17
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3519V101), y)
    ifeq ($(LOSCFG_ARCH_CORTEX_A7), y)
        LITEOS_CMACRO_TEST += -DTEST3519V101
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3519v101/cortex-a7
        LITEOS_CPU_TYPE    := cortex-a7
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A7
    else ifeq ($(LOSCFG_ARCH_CORTEX_A17), y)
        LITEOS_CMACRO_TEST += -DTEST3519V101
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3519v101/cortex-a17
        LITEOS_CPU_TYPE    := cortex-a17
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A17
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3559), y)
    ifeq ($(LOSCFG_ARCH_CORTEX_A7), y)
        LITEOS_CMACRO_TEST += -DTEST3559
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3559/cortex-a7
        LITEOS_CPU_TYPE    := cortex-a7
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A7
    else ifeq ($(LOSCFG_ARCH_CORTEX_A17), y)
        LITEOS_CMACRO_TEST += -DTEST3559
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3559/cortex-a17
        LITEOS_CPU_TYPE    := cortex-a17
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A17
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3731), y)
        LITEOS_CMACRO_TEST += -DTEST3731 -DLOSCFG_USB_ONLY_HOST_MODE -DMULTI_DEVICE
        LITEOS_CXXMACRO   += -DTEST3731
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
        LITEOS_PLATFORM    := hi3731
        LITEOS_CPU_TYPE    := arm926
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_ARM926

else ifeq ($(LOSCFG_PLATFORM_HI3559AV100ES), y)
    ifeq ($(LOSCFG_ARCH_CORTEX_M7), y)
        LITEOS_CMACRO_TEST += -DTEST3559AES_M7
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
        LITEOS_PLATFORM    := hi3559av100es/cortex-m7
        LITEOS_CPU_TYPE    := cortex-m7
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_M7
        LITEOS_LD_PATH     += -L$(shell which arm-none-eabi-gcc | sed -u 's/arm-none-eabi-gcc//g')../lib/gcc/arm-none-eabi/4.9.3/armv7e-m/fpu/
    else ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH64), y)
        LITEOS_CMACRO_TEST +=
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=8
        LITEOS_PLATFORM    := hi3559av100es/cortex-a53_aarch64
        LITEOS_CPU_TYPE    := cortex-a53_aarch64
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A53_AARCH64 -DTEST3559A
        LOSCFG_ARCH_CORTEX_A53 := y
    else ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH32), y)
        LITEOS_CMACRO_TEST +=
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3559av100es/cortex-a53_aarch32
        LITEOS_CPU_TYPE    := cortex-a53_aarch32
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A53_AARCH32 -DTEST3559A
        LOSCFG_ARCH_CORTEX_A53 := y
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3559AV100), y)
    ifeq ($(LOSCFG_ARCH_CORTEX_M7), y)
        LITEOS_CMACRO_TEST += -DTEST3559A_M7
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
        LITEOS_PLATFORM    := hi3559av100/cortex-m7
        LITEOS_CPU_TYPE    := cortex-m7
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_M7
        LITEOS_LD_PATH     += -L$(shell which arm-none-eabi-gcc | sed -u 's/arm-none-eabi-gcc//g')../lib/gcc/arm-none-eabi/4.9.3/armv7e-m/fpu/
    else ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH64), y)
        LITEOS_CMACRO_TEST +=
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=8
        LITEOS_PLATFORM    := hi3559av100/cortex-a53_aarch64
        LITEOS_CPU_TYPE    := cortex-a53_aarch64
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A53_AARCH64 -DTEST3559A
        LOSCFG_ARCH_CORTEX_A53 := y
        ifeq ($(LOSCFG_PLATFORM_HISI_AMP), y)
            LITEOS_BASELIB     += -lsharefs
        endif
     else ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH32), y)
        LITEOS_CMACRO_TEST +=
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3559av100/cortex-a53_aarch32
        LITEOS_CPU_TYPE    := cortex-a53_aarch32
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A53_AARCH32 -DTEST3559A
        LOSCFG_ARCH_CORTEX_A53 := y
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3556AV100), y)
    ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH32), y)
        LITEOS_CMACRO_TEST +=
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3556av100/cortex-a53_aarch32
        LITEOS_CPU_TYPE    := cortex-a53_aarch32
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A53_AARCH32
        LOSCFG_ARCH_CORTEX_A53 := y
    endif
    ifeq ($(LOSCFG_PLATFORM_HISI_AMP), y)
        LITEOS_BASELIB     += -lsharefs
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3519AV100), y)
    ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH32), y)
        LITEOS_CMACRO_TEST +=
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3519av100/cortex-a53_aarch32
        LITEOS_CPU_TYPE    := cortex-a53_aarch32
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A53_AARCH32
        LOSCFG_ARCH_CORTEX_A53 := y
    endif
    ifeq ($(LOSCFG_PLATFORM_HISI_AMP), y)
        LITEOS_BASELIB     += -lsharefs
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3516CV500), y)
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
    LITEOS_PLATFORM    := hi3516cv500
    LITEOS_CPU_TYPE    := cortex-a7
    LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A7
    ifeq ($(LOSCFG_PLATFORM_HISI_AMP), y)
        LITEOS_BASELIB     += -lsharefs
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3516EV200), y)
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
    LITEOS_PLATFORM    := hi3516ev200
    LITEOS_CPU_TYPE    := cortex-a7
    LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A7
    ifeq ($(LOSCFG_PLATFORM_HISI_AMP), y)
        LITEOS_BASELIB     += -lsharefs
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3516DV300), y)
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
    LITEOS_PLATFORM    := hi3516dv300
    LITEOS_CPU_TYPE    := cortex-a7
    LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A7
    ifeq ($(LOSCFG_PLATFORM_HISI_AMP), y)
        LITEOS_BASELIB     += -lsharefs
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3556V200), y)
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
    LITEOS_PLATFORM    := hi3556v200
    LITEOS_CPU_TYPE    := cortex-a7
    LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A7
    ifeq ($(LOSCFG_PLATFORM_HISI_AMP), y)
        LITEOS_BASELIB     += -lsharefs
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3559V200), y)
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
    LITEOS_PLATFORM    := hi3559v200
    LITEOS_CPU_TYPE    := cortex-a7
    LITEOS_CMACRO      += -DLOSCFG_ARCH_CORTEX_A7
    ifeq ($(LOSCFG_PLATFORM_HISI_AMP), y)
        LITEOS_BASELIB     += -lsharefs
    endif
    ifeq ($(IRQ_BIND), liteos)
    LITEOS_CMACRO      += -DLOSCFG_DRIVERS_SVP
    endif
endif
ifeq ($(LOSCFG_PLATFORM_HISI_AMP), y)
    LITEOS_BASELIB     += -lipcm -lvirt-tty
endif
####################################### Chip and CPU Option End #########################################

####################################### Kernel Option Begin ###########################################
ifeq ($(LOSCFG_KERNEL_LITEKERNEL), y)
    LITEOS_BASELIB += -lbase
    LIB_SUBDIRS       += kernel/base
    LITEOS_KERNEL_INCLUDE   := -I $(LITEOSTOPDIR)/kernel/include
ifeq ($(LOSCFG_KERNEL_MEM_TLSF), y)
    MEM_TYPE    := mem_tlsf
else
    MEM_TYPE    := mem_bestfit
endif
endif

ifeq ($(LOSCFG_KERNEL_CPUP), y)
    LITEOS_BASELIB   += -lcpup
    LIB_SUBDIRS         += kernel/extended/cpup
    LITEOS_CPUP_INCLUDE := -I $(LITEOSTOPDIR)/kernel/extended/cpup
endif

ifeq ($(LOSCFG_KERNEL_MPU), y)
    LITEOS_BASELIB += -lmpu
    LIB_SUBDIRS += kernel/extended/mpu
    LITEOS_CMACRO += -DLOSCFG_KERNEL_MPU
endif
ifeq ($(LOSCFG_KERNEL_CPPSUPPORT), y)
    LITEOS_BASELIB += -lsupc++
    LITEOS_BASELIB    += -lcppsupport -lstdc++
    LIB_SUBDIRS       += kernel/extended/cppsupport
    LITEOS_CPPSUPPORT_INCLUDE   += -I $(LITEOSTOPDIR)/kernel/extended/cppsupport

endif

ifeq ($(LOSCFG_KERNEL_DYNLOAD), y)
    LITEOS_BASELIB   += -ldynload
    LIB_SUBDIRS      += kernel/extended/dynload
    LITEOS_DYNLOAD_INCLUDE   += -I $(LITEOSTOPDIR)/kernel/extended/dynload/include
endif

ifeq ($(LOSCFG_KERNEL_RUNSTOP), y)
    LITEOS_BASELIB += -lrunstop
    LIB_SUBDIRS       += kernel/extended/runstop
    LITEOS_RUNSTOP_INCLUDE   += -I $(LITEOSTOPDIR)/kernel/extended/runstop
endif

ifeq ($(LOSCFG_KERNEL_SCATTER), y)
    LITEOS_BASELIB += -lscatter
    LIB_SUBDIRS       += kernel/extended/scatter
    LITEOS_SCATTER_INCLUDE   += -I $(LITEOSTOPDIR)/kernel/extended/scatter
endif

ifeq ($(LOSCFG_KERNEL_TICKLESS), y)
    LITEOS_BASELIB += -ltickless
    LIB_SUBDIRS       += kernel/extended/tickless
    LITEOS_TICKLESS_INCLUDE   += -I $(LITEOSTOPDIR)/kernel/extended/tickless
endif
################################### Kernel Option End ################################

#################################### Lib Option Begin ###############################
ifeq ($(LOSCFG_LIB_LIBC), y)
    LITEOS_BASELIB  += -lc -lcsysdeps -lsec
    LIB_SUBDIRS        += lib/libc
    LIB_SUBDIRS        += lib/libsec
    ifeq ($(LITEOS_CPU_TYPE), cortex-a7)
        LIB_SUBDIRS        += lib/libc/src/sysdeps/a7
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi/asm-arm
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/arch-arm/include
    endif
    ifeq ($(LITEOS_CPU_TYPE), cortex-a17)
        LIB_SUBDIRS        += lib/libc/src/sysdeps/a7
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi/asm-arm
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/arch-arm/include
    endif
    ifeq ($(LITEOS_CPU_TYPE), arm926)
        LIB_SUBDIRS        += lib/libc/src/sysdeps/common
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi/asm-arm
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/arch-arm/include
    endif
    ifeq ($(LITEOS_CPU_TYPE), cortex-m7)
        LIB_SUBDIRS         += lib/libc/src/sysdeps/m7
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi/asm-arm
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/arch-arm/include
    endif
    ifeq ($(LITEOS_CPU_TYPE), cortex-a53_aarch64)
        LIB_SUBDIRS        += lib/libc/src/sysdeps/a53
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi/asm-arm64
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/arch-arm64/include
    endif
    ifeq ($(LITEOS_CPU_TYPE), cortex-a53_aarch32)
        LIB_SUBDIRS        += lib/libc/src/sysdeps/a7
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi/asm-arm
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/arch-arm/include
    endif

    LITEOS_LIBC_INCLUDE   += \
        -I $(LITEOSTOPDIR)/lib/include \
        -I $(LITEOSTOPDIR)/lib/libc/include \
        -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi \
        -I $(LITEOSTOPDIR)/lib/libsec/include
endif


ifeq ($(LOSCFG_LIB_LIBCMINI), y)
    LITEOS_BASELIB  += -lcmini
    LIB_SUBDIRS        += lib/libcmini
    LITEOS_LIBC_INCLUDE   += -I $(LITEOSTOPDIR)/lib/libcmini
endif

ifeq ($(LOSCFG_LIB_LIBM), y)
    LITEOS_BASELIB   += -lm
    LIB_SUBDIRS         += lib/libm
    LITEOS_LIBM_INCLUDE += -I $(LITEOSTOPDIR)/lib/libm/include
endif

ifeq ($(LOSCFG_LIB_ZLIB), y)
    LITEOS_BASELIB += -lz
    LIB_SUBDIRS    += lib/zlib
    LITEOS_ZLIB_INCLUDE += -I $(LITEOSTOPDIR)/lib/zlib/include
endif
################################### Lib Option End ######################################

####################################### Compat Option Begin #########################################
ifeq ($(LOSCFG_COMPAT_CMSIS), y)
    LITEOS_BASELIB  += -lcmsis
    LIB_SUBDIRS     += compat/cmsis
    LITEOS_CMSIS_INCLUDE += -I $(LITEOSTOPDIR)/compat/cmsis/include
endif

ifeq ($(LOSCFG_COMPAT_POSIX), y)
    LITEOS_BASELIB += -lposix
    LIB_SUBDIRS       += compat/posix
    LITEOS_POSIX_INCLUDE   += \
        -I $(LITEOSTOPDIR)/compat/posix/include
endif

ifeq ($(LOSCFG_COMPAT_LINUX), y)
    LITEOS_BASELIB += -llinux
    LIB_SUBDIRS       += compat/linux
    LITEOS_LINUX_INCLUDE   += -I $(LITEOSTOPDIR)/compat/linux/include
endif
######################################## Compat Option End ############################################

################################## Frameworks Option Begin ##########################
ifeq ($(LOSCFG_FRAMEWORKS_M2MCOMMON), y)
    LITEOS_BASELIB += -lm2mcomm
    LIB_SUBDIRS    += frameworks/m2mcomm
endif
################################## Frameworks Option End ##########################


#################################### FS Option Begin ##################################
ifeq ($(LOSCFG_FS_VFS), y)
    LITEOS_BASELIB += -lvfs -lmulti_partition
    LIB_SUBDIRS       += fs/vfs fs/vfs/multi_partition
    LITEOS_VFS_INCLUDE   += -I $(LITEOSTOPDIR)/fs/include
    LITEOS_VFS_INCLUDE   += -I $(LITEOSTOPDIR)/fs/vfs/include/driver
    LITEOS_VFS_MTD_INCLUDE := -I $(LITEOSTOPDIR)/fs/vfs/include/multi_partition
endif

ifeq ($(LOSCFG_FS_FAT), y)
    LITEOS_BASELIB  += -lfat
    LIB_SUBDIRS     += fs/fat
    LOSCFG_DRIVER_DISK := y
    LITEOS_FAT_INCLUDE += -I $(LITEOSTOPDIR)/fs/fat/include
endif

ifeq ($(LOSCFG_FS_FAT_FSCK), y)
    LITEOS_BASELIB += -lfsck
    LIB_SUBDIRS    += fs/fat/fsck
endif

ifeq ($(LOSCFG_FS_FAT_VIRTUAL_PARTITION), y)
    LITEOS_BASELIB += -lvirpart
    LIB_SUBDIRS += fs/fat/virpart
    LITEOS_FAT_VIRPART_INCLUDE += -I $(LITEOSTOPDIR)/fs/virpart/include
endif

ifeq ($(LOSCFG_FS_FAT_CACHE), y)
    LITEOS_BASELIB  += -lbcache
    LIB_SUBDIRS     += fs/vfs/bcache
    LITEOS_FAT_CACHE_INCLUDE += -I $(LITEOSTOPDIR)/fs/vfs/include/bcache
endif


ifeq ($(LOSCFG_FS_RAMFS), y)
    LITEOS_BASELIB  += -lramfs
    LIB_SUBDIRS     += fs/ramfs
endif

ifeq ($(LOSCFG_FS_YAFFS), y)
    LITEOS_BASELIB  += -lyaffs2
    LIB_SUBDIRS     += fs/yaffs2
endif


ifeq ($(LOSCFG_FS_NFS), y)
    LITEOS_BASELIB  += -lnfs
    LIB_SUBDIRS     += fs/nfs
endif

ifeq ($(LOSCFG_FS_PROC), y)
    LITEOS_BASELIB  += -lproc
    LIB_SUBDIRS     += fs/proc
    LITEOS_PROC_INCLUDE += -I $(LITEOSTOPDIR)/fs/proc/include
endif


ifeq ($(LOSCFG_FS_JFFS), y)
    LITEOS_BASELIB  += -ljffs2
    LIB_SUBDIRS     += fs/jffs2
endif

ifeq ($(LOSCFG_FS_ROMFS), y)
    LITEOS_BASELIB  += -lromfs
    LIB_SUBDIRS     += fs/romfs
endif
#################################### FS Option End ##################################


################################### Net Option Begin ###################################
ifeq ($(LOSCFG_NET_LWIP_SACK), y)
    LITEOS_BASELIB += -llwip
    LIB_SUBDIRS       += net/lwip_sack
    LITEOS_LWIP_SACK_INCLUDE   += \
        -I $(LITEOSTOPDIR)/net/lwip_sack/include \
        -I $(LITEOSTOPDIR)/net/lwip_sack/include/ipv4 \
        -I $(LITEOSTOPDIR)/net/mac

    LITEOS_CMACRO     +=  $(LWIP_MACROS)
endif


ifeq ($(LOSCFG_NET_WPA), y)
    LITEOS_BASELIB    += -lwpa_supplicant
    LIB_SUBDIRS       +=  net/wpa_supplicant
endif

ifeq ($(LOSCFG_NET_PCAP), y)
    LITEOS_BASELIB += -lpcap
    LIB_SUBDIRS += tools/pcap
endif
#################################### Net Option End####################################

################################## Driver Option Begin #################################
ifeq ($(LOSCFG_DRIVERS_BASE), y)
    LITEOS_BASELIB += -ldriverbase
    LIB_SUBDIRS       += drivers/base
    LITEOS_DBASE_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/base/include
endif

ifeq ($(LOSCFG_DRIVERS_GPIO), y)
    LITEOS_BASELIB     += -lgpio
    LIB_SUBDIRS           += drivers/gpio
    LITEOS_GPIO_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/gpio/include
endif

ifeq ($(LOSCFG_DRIVERS_HIDMAC), y)
    LITEOS_BASELIB    += -lhidmac
    LIB_SUBDIRS       += drivers/hidmac
    LITEOS_HIDMAC_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/hidmac/include
endif

ifeq ($(LOSCFG_DRIVERS_HIEDMAC), y)
    LITEOS_BASELIB    += -lhiedmac
    LIB_SUBDIRS       += drivers/hiedmac
    LITEOS_HIDMAC_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/hiedmac/include
endif

ifeq ($(LOSCFG_DRIVERS_HIETH_SF), y)
       LITEOS_HIETH_SF_INCLUDE += -I $(LITEOSTOPDIR)/drivers/hieth-sf/include
       LITEOS_BASELIB    += -lhieth-sf
       LIB_SUBDIRS       +=  drivers/hieth-sf
endif

ifeq ($(LOSCFG_DRIVERS_HIGMAC), y)
    LITEOS_HIGMAC_INCLUDE += -I $(LITEOSTOPDIR)/drivers/higmac/include
    LITEOS_BASELIB    += -lhigmac
    LIB_SUBDIRS       += drivers/higmac
endif

ifeq ($(LOSCFG_DRIVERS_I2C), y)
    LITEOS_BASELIB += -li2c
    LIB_SUBDIRS       += drivers/i2c
    LITEOS_I2C_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/i2c/include
endif

ifeq ($(LOSCFG_DRIVERS_REGULATOR), y)
    LITEOS_BASELIB += -lregulator
    LIB_SUBDIRS       += drivers/regulator
    LITEOS_REGULATOR_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/regulator/include
endif

ifeq ($(LOSCFG_DRIVERS_CPUFREQ), y)
    LITEOS_BASELIB += -lcpufreq
    LIB_SUBDIRS       += drivers/cpufreq
    LITEOS_CPUFREQ_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/cpufreq/include
endif

ifeq ($(LOSCFG_DRIVERS_DEVFREQ), y)
    LITEOS_BASELIB += -ldevfreq
    LIB_SUBDIRS       += drivers/devfreq
    LITEOS_DEVFREQ_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/devfreq/include
endif

ifeq ($(LOSCFG_DRIVERS_MEM), y)
    LITEOS_BASELIB += -lmem
    LIB_SUBDIRS       += drivers/mem
endif

ifeq ($(LOSCFG_DRIVERS_MMC), y)
ifeq ($(LOSCFG_ARCH_CORTEX_A53), y)
    MMC_HOST_DIR := sdhci
else
    MMC_HOST_DIR := himci
endif

    LITEOS_MMC_INCLUDE += -I $(LITEOSTOPDIR)/drivers/mmc/include
    LITEOS_BASELIB  += -lmmc
    LIB_SUBDIRS        += drivers/mmc
    LOSCFG_DRIVER_DISK := y
endif

ifeq ($(LOSCFG_DRIVERS_MTD), y)
    LITEOS_BASELIB    += -lmtd_common
    LIB_SUBDIRS       += drivers/mtd/common

    ifeq ($(LOSCFG_DRIVERS_MTD_NAND), y)
        ifeq ($(LOSCFG_DRIVERS_MTD_NAND_HINFC620), y)
            NAND_DRIVER_DIR := hinfc620
    else ifeq ($(LOSCFG_DRIVERS_MTD_NAND_HIFMC100), y)
            NAND_DRIVER_DIR := hifmc100
    else ifeq ($(LOSCFG_DRIVERS_MTD_NAND_HISNFC100), y)
            NAND_DRIVER_DIR := hisnfc100
    else ifeq ($(LOSCFG_DRIVERS_MTD_NAND_HIFMC100_PARALLEL), y)
            NAND_DRIVER_DIR := hifmc100_nand
    endif

        LITEOS_BASELIB += -lnand_flash
        LIB_SUBDIRS       += drivers/mtd/nand
        LITEOS_MTD_NAND_INCLUDE  += -I $(LITEOSTOPDIR)/drivers/mtd/nand/include

    endif

    ifeq ($(LOSCFG_DRIVERS_MTD_SPI_NOR), y)
        ifeq ($(LOSCFG_DRIVERS_MTD_SPI_NOR_HISFC350), y)
            NOR_DRIVER_DIR := hisfc350
    else ifeq ($(LOSCFG_DRIVERS_MTD_SPI_NOR_HIFMC100), y)
            NOR_DRIVER_DIR := hifmc100
    endif

        LITEOS_BASELIB   += -lspinor_flash
        LIB_SUBDIRS      += drivers/mtd/spi_nor
        LITEOS_MTD_SPI_NOR_INCLUDE  +=  -I $(LITEOSTOPDIR)/drivers/mtd/spi_nor/include

    endif
endif

ifeq ($(LOSCFG_DRIVERS_RANDOM), y)
    LITEOS_BASELIB += -lrandom
    LIB_SUBDIRS    += drivers/random
    LITEOS_RANDOM_INCLUDE += -I $(LITEOSTOPDIR)/drivers/random/include
endif

ifeq ($(LOSCFG_DRIVERS_RTC), y)
    LITEOS_BASELIB  += -lrtc
    LIB_SUBDIRS        += drivers/rtc
    LITEOS_RTC_INCLUDE += -I $(LITEOSTOPDIR)/drivers/rtc/include
endif

ifeq ($(LOSCFG_DRIVERS_SPI), y)
    LITEOS_BASELIB  +=  -lspi
    LIB_SUBDIRS     += drivers/spi
    LITEOS_SPI_INCLUDE += -I $(LITEOSTOPDIR)/drivers/spi/include
endif

ifeq ($(LOSCFG_DRIVERS_UART), y)
    LITEOS_BASELIB   += -luart
    LITEOS_BASELIB      += -luart
    LIB_SUBDIRS         +=  drivers/uart
    LITEOS_UART_INCLUDE += -I $(LITEOSTOPDIR)/drivers/uart/include
endif

ifeq ($(LOSCFG_DRIVERS_USB), y)
    LITEOS_BASELIB  += -lusb
    LIB_SUBDIRS     += drivers/usb
    LITEOS_USB_INCLUDE += -I $(LITEOSTOPDIR)/drivers/usb
    LOSCFG_DRIVER_DISK := y
endif

ifeq ($(LOSCFG_DRIVERS_VIDEO), y)
    LITEOS_BASELIB += -lvideo
    LIB_SUBDIRS       += drivers/video
endif

ifeq ($(LOSCFG_DRIVERS_WIFI_BCM), y)
    LITEOS_BASELIB += -lwwd -lwifi_adapt
    LIB_SUBDIRS       += drivers/wifi/bcm_wifi  drivers/wifi/wifi_adapt
else ifeq ($(LOSCFG_DRIVERS_WIFI_QRD), y)
    LITEOS_BASELIB += -lar6003
    LIB_SUBDIRS       += drivers/wifi/ar6k3_wifi
else ifeq ($(LOSCFG_DRIVERS_WIFI_RTL), y)
    LITEOS_BASELIB += -lusb_rtl8188eus
    LIB_SUBDIRS       += drivers/wifi_usb/usb_rtl8188eus
endif

ifeq ($(LOSCFG_DRIVERS_WTDG), y)
    LITEOS_BASELIB +=  -lwtdg
    LIB_SUBDIRS    += drivers/wtdg
    LITEOS_WTDG_INCLUDE := -I $(LITEOSTOPDIR)/drivers/wtdg/include
endif

ifeq ($(LOSCFG_DRIVER_DISK), y)
    LITEOS_BASELIB += -ldisk
    LIB_SUBDIRS += fs/vfs/disk
endif
############################## Driver Option End #######################################

############################# Tools && Debug Option Begin ##############################
ifeq ($(LOSCFG_GDB), y)
    LITEOS_BASELIB    += -lgdb
    LIB_SUBDIRS       += tools/gdb
    LITEOS_CMACRO += -DLOSCFG_GDB -DLOS_GDB_ICACHE
endif
ifeq ($(LOSCFG_COREDUMP), y)
    LITEOS_BASELIB    += -lcoredump
    LIB_SUBDIRS       += tools/coredump
    LITEOS_CMACRO += -DLOSCFG_COREDUMP
    LOSCFG_COMPRESS := y
endif

ifeq ($(LOSCFG_LLTSER) ,y)
    LITEOS_BASELIB    += -lgcov_ser
    LIB_SUBDIRS       += tools/gcov_ser
    LITEOS_CMACRO += -DLOSCFG_LLTSER
    LOSCFG_COMPRESS := y
endif

ifeq ($(LOSCFG_COMPRESS), y)
    LITEOS_BASELIB    += -lcompress
    LIB_SUBDIRS       += tools/compress
endif

ifeq ($(LOSCFG_COMPILE_DEBUG), y)
    LITEOS_COPTS_OPTMIZE = -O0
    LITEOS_COPTS_OPTION  = -g -gdwarf-2
else
    LITEOS_COPTS_OPTMIZE = -O2
    LITEOS_COPTS_OPTMIZE_NODEBUG = -O0
endif
    LITEOS_COPTS_DEBUG  += $(LITEOS_COPTS_OPTION) $(LITEOS_COPTS_OPTMIZE)
    LITEOS_INTERWORK += $(LITEOS_COPTS_OPTION) $(LITEOS_COPTS_OPTMIZE)
    LITEOS_CXXOPTS_BASE += $(LITEOS_COPTS_OPTION) $(LITEOS_COPTS_OPTMIZE)
    LITEOS_ASOPTS   += $(LITEOS_COPTS_OPTION)
    LITEOS_NODEBUG  += $(LITEOS_COPTS_OPTMIZE_NODEBUG)

ifeq ($(LOSCFG_SHELL), y)
    LITEOS_BASELIB += -lshell
    LIB_SUBDIRS       += shell
    LITEOS_SHELL_INCLUDE  += -I $(LITEOSTOPDIR)/shell/include
endif


ifeq ($(LOSCFG_NET_TELNET), y)
    LITEOS_BASELIB += -ltelnet
    LIB_SUBDIRS       += net/telnet
    LITEOS_TELNET_INCLUDE   += \
        -I $(LITEOSTOPDIR)/net/telnet/include
endif

ifeq ($(LOSCFG_3RDPARTY), y)
    -include $(LITEOSTOPDIR)/3rdParty/3rdParty.mk
endif

ifeq ($(LOSCFG_PLATFORM_OSAPPINIT), y)

else ifeq ($(LOSCFG_TEST), y)
    -include $(LITEOSTOPDIR)/test/test.mk
endif


ifeq ($(LOSCFG_TOOLS_IPERF), y)
    LITEOS_BASELIB    += -liperf
    LIB_SUBDIRS       += tools/iperf-2.0.5
endif



############################# Tools && Debug Option End #################################

############################# Security Option Begin ##############################
LITEOS_SSP = -fno-stack-protector
ifeq ($(LOSCFG_CC_STACKPROTECTOR), y)
    LITEOS_SSP = -fstack-protector --param ssp-buffer-size=4
endif

ifeq ($(LOSCFG_CC_STACKPROTECTOR_ALL), y)
    LITEOS_SSP = -fstack-protector-all
endif

############################# Security Option End ##############################
############################# Platform Option Begin#################################

-include $(LITEOSTOPDIR)/platform/bsp/bsp.mk

LITEOS_BASELIB +=  -lcommon
LIB_SUBDIRS    += platform/bsp/common
############################# Platform Option End #################################
LITEOS_COMPILER_PATH = $(shell $(LITEOSTOPDIR)/build/mk/get_compiler_path.sh  $(CROSS_COMPILE))
LITEOS_COMPILER_CXX_PATH = $(LITEOS_COMPILER_PATH)/$(COMPILE_NAME)/include
ifeq ($(LOSCFG_COMPILER_HIMIX100_32), y)
LITEOS_COMPILER_CXXLIB_PATH = $(LITEOS_COMPILER_PATH)/$(COMPILE_NAME)/lib/$(LITEOS_GCCLIB)
LITEOS_COMPILER_GCCLIB_PATH = $(LITEOS_COMPILER_PATH)/lib/gcc/$(COMPILE_NAME)/$(VERSION_NUM)/$(LITEOS_GCCLIB)
else ifeq ($(LOSCFG_COMPILER_HIMIX200_32), y)
LITEOS_COMPILER_CXXLIB_PATH = $(LITEOS_COMPILER_PATH)/$(COMPILE_NAME)/lib/$(LITEOS_GCCLIB)
LITEOS_COMPILER_GCCLIB_PATH = $(LITEOS_COMPILER_PATH)/lib/gcc/$(COMPILE_NAME)/$(VERSION_NUM)/$(LITEOS_GCCLIB)
else ifeq ($(LOSCFG_COMPILER_LITEOS_32), y)
LITEOS_COMPILER_CXXLIB_PATH = $(LITEOS_COMPILER_PATH)/$(COMPILE_NAME)/lib/$(LITEOS_GCCLIB)
LITEOS_COMPILER_GCCLIB_PATH = $(LITEOS_COMPILER_PATH)/lib/gcc/$(COMPILE_NAME)/$(VERSION_NUM)/$(LITEOS_GCCLIB)
else ifeq ($(LOSCFG_COMPILER_CORTEX_M7), y)
LITEOS_COMPILER_CXXLIB_PATH = $(LITEOS_COMPILER_PATH)/$(COMPILE_NAME)/lib/$(LITEOS_GCCLIB)
LITEOS_COMPILER_GCCLIB_PATH = $(LITEOS_COMPILER_PATH)/lib/gcc/$(COMPILE_NAME)/$(VERSION_NUM)/$(LITEOS_GCCLIB)
else ifeq ($(LOSCFG_COMPILER_HIMIX100_64), y)
LITEOS_COMPILER_CXXLIB_PATH = $(LITEOS_COMPILER_PATH)/$(COMPILE_NAME)/lib64
LITEOS_COMPILER_GCCLIB_PATH = $(LITEOS_COMPILER_PATH)/lib/gcc/$(COMPILE_NAME)/$(VERSION_NUM)
endif

LITEOS_CXXINCLUDE += \
    -I $(LITEOS_COMPILER_CXX_PATH)/c++/$(VERSION_NUM) \
    -I $(LITEOS_COMPILER_CXX_PATH)/c++/$(VERSION_NUM)/ext \
    -I $(LITEOS_COMPILER_CXX_PATH)/c++/$(VERSION_NUM)/backward \
    -I $(LITEOSTOPDIR)/compat/posix/include \
    -I $(LITEOSTOPDIR)/lib/libm/include \
    -I $(LITEOSTOPDIR)/lib/libc/include \
    -I $(LITEOSTOPDIR)/fs/include \
    -I $(LITEOSTOPDIR)/kernel/include \
    $(LITEOS_LIBC_INCLUDE)

ifeq ($(LOSCFG_COMPILER_HIMIX100_32), y)
    LITEOS_CXXINCLUDE += \
        -I $(LITEOS_COMPILER_CXX_PATH)/c++/$(VERSION_NUM)/arm-linux-androideabi \
    	-I $(LITEOS_COMPILER_PATH)/lib/gcc/arm-linux-androideabi/$(VERSION_NUM)/include
    LITEOS_CXXMACRO     += -DLOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT
    LITEOS_CMACRO     += -DLOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT
else ifeq ($(LOSCFG_COMPILER_HIMIX200_32), y)
    LITEOS_CXXINCLUDE += \
        -I $(LITEOS_COMPILER_CXX_PATH)/c++/$(VERSION_NUM)/arm-linux-androideabi \
    	-I $(LITEOS_COMPILER_PATH)/lib/gcc/arm-linux-androideabi/$(VERSION_NUM)/include
    LITEOS_CXXMACRO     += -DLOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT
    LITEOS_CMACRO     += -DLOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT
else ifeq ($(LOSCFG_COMPILER_LITEOS_32), y)
    LITEOS_CXXINCLUDE += \
        -I $(LITEOS_COMPILER_CXX_PATH)/c++/$(VERSION_NUM)/arm-linux-androideabi \
    	-I $(LITEOS_COMPILER_PATH)/lib/gcc/arm-linux-androideabi/$(VERSION_NUM)/include
    LITEOS_CXXMACRO     += -DLOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT
    LITEOS_CMACRO     += -DLOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT
else ifeq ($(LOSCFG_COMPILER_HIMIX100_64), y)
    LITEOS_CXXINCLUDE += \
        -I $(LITEOS_COMPILER_CXX_PATH)/c++/$(VERSION_NUM)/aarch64-linux-android \
    	-I $(LITEOS_COMPILER_PATH)/lib/gcc/aarch64-linux-android/$(VERSION_NUM)/include
    LITEOS_CXXMACRO     += -DLOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT
    LITEOS_CMACRO     += -DLOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT
else ifeq ($(LOSCFG_COMPILER_CORTEX_M7), y)
	    LITEOS_CXXINCLUDE += -I $(LITEOS_COMPILER_PATH)/lib/gcc/arm-none-eabi/$(VERSION_NUM)/include
endif

LITEOS_CXXOPTS_BASE  += -std=c++11 -nostdlib -nostdinc -nostdinc++ -fexceptions -fpermissive -fno-use-cxa-atexit -fno-builtin -frtti -fno-pic -Winvalid-pch



LITEOS_EXTKERNEL_INCLUDE   := $(LITEOS_CPPSUPPORT_INCLUDE) $(LITEOS_DYNLOAD_INCLUDE) \
                              $(LITEOS_RUNSTOP_INCLUDE)    $(LITEOS_SCATTER_INCLUDE) \
                              $(LITEOS_TICKLESS_INCLUDE)
LITEOS_COMPAT_INCLUDE      := $(LITEOS_CMSIS_INCLUDE)      $(LITEOS_POSIX_INCLUDE) \
                              $(LITEOS_LINUX_INCLUDE)
LITEOS_FS_INCLUDE          := $(LITEOS_VFS_INCLUDE)        $(LITEOS_FAT_CACHE_INCLUDE) \
                              $(LITEOS_VFS_MTD_INCLUDE)    $(LITEOS_PROC_INCLUDE) \
                              $(LITEOS_FAT_VIRPART_INCLUDE) $(LITEOS_FAT_INCLUDE)
LITEOS_NET_INCLUDE         := $(LITEOS_LWIP_SACK_INCLUDE)
LITEOS_LIB_INCLUDE         := $(LITEOS_LIBC_INCLUDE)       $(LITEOS_LIBM_INCLUDE) \
                              $(LITEOS_ZLIB_INCLUDE)
LITEOS_DRIVERS_INCLUDE     := $(LITEOS_CELLWISE_INCLUDE)   $(LITEOS_GPIO_INCLUDE) \
                              $(LITEOS_HIDMAC_INCLUDE)     $(LITEOS_HIETH_SF_INCLUDE) \
                              $(LITEOS_HIGMAC_INCLUDE)     $(LITEOS_I2C_INCLUDE) \
                              $(LITEOS_LCD_INCLUDE)        $(LITEOS_MMC_INCLUDE) \
                              $(LITEOS_MTD_NAND_INCLUDE)   $(LITEOS_MTD_SPI_NOR_INCLUDE) \
                              $(LITEOS_RANDOM_INCLUDE)     $(LITEOS_RTC_INCLUDE) \
                              $(LITEOS_SPI_INCLUDE)        $(LITEOS_USB_INCLUDE) \
                              $(LITEOS_WTDG_INCLUDE)       $(LITEOS_DBASE_INCLUDE) \
                              $(LITEOS_CPUFREQ_INCLUDE)    $(LITEOS_DEVFREQ_INCLUDE) \
                              $(LITEOS_REGULATOR_INCLUDE)
LOSCFG_TOOLS_DEBUG_INCLUDE := $(LITEOS_SHELL_INCLUDE)      $(LITEOS_UART_INCLUDE) \
                              $(LITEOS_TELNET_INCLUDE)


FP = -fno-omit-frame-pointer
LITEOS_COPTS_BASE  := -fno-pic -fno-aggressive-loop-optimizations -fno-builtin -nostdinc -nostdlib $(WARNING_AS_ERROR) $(LITEOS_SSP)
LITEOS_COPTS_EXTRA := -Wnonnull -std=c99 -Wpointer-arith -Wstrict-prototypes -Winvalid-pch \
                      -Wno-write-strings -ffunction-sections \
                      -fdata-sections -fno-exceptions $(FP)
ifneq ($(LOSCFG_ARCH_CORTEX_A53_AARCH64), y)
LITEOS_COPTS_BASE += -mno-unaligned-access
LITEOS_COPTS_EXTRA += -mthumb-interwork
endif
ifneq ($(LOSCFG_COMPILER_CORTEX_M7), y)
LITEOS_COPTS_EXTRA += -fno-short-enums
endif
ifeq ($(LOSCFG_THUMB), y)
LITEOS_COPTS_EXTRA_INTERWORK := $(LITEOS_COPTS_EXTRA) -mthumb
LITEOS_CMACRO     += -DLOSCFG_INTERWORK_THUMB
else
LITEOS_COPTS_EXTRA_INTERWORK := $(LITEOS_COPTS_EXTRA)
#-fno-inline
endif

ifeq ($(LOSCFG_LLTREPORT) ,y)
LITEOS_GCOV_OPTS := -fprofile-arcs -ftest-coverage
LITEOS_BASELIB += -lgcov
endif
LITEOS_LD_OPTS += -nostartfiles -static --gc-sections
LITEOS_LD_OPTS += $(LITEOS_DYNLOADOPTS)
LITEOS_LD_PATH += -L$(LITEOS_SCRIPTPATH)/ld -L$(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM) -L$(OUT)/lib -L$(LITEOS_LIB_BIGODIR) -L$(LITEOSTOPDIR)/tools/build
LITEOS_LD_PATH += -L$(LITEOS_COMPILER_GCCLIB_PATH) -L$(LITEOS_COMPILER_CXXLIB_PATH)
ifeq ($(LOSCFG_VENDOR) ,y)
LITEOS_LD_PATH +=  -L$(OUT)/lib/rdk -L$(OUT)/lib/sdk \
                   -L$(OUT)/lib/main_server
endif

ifeq ($(LOSCFG_ARCH_CORTEX_M7), y)
    LITEOS_LD_SCRIPT := -T$(LITEOSTOPDIR)/platform/bsp/board/hi3559av100/cortex-m7/liteos.ld
else
    LITEOS_LD_SCRIPT := -T$(LITEOSTOPDIR)/liteos.ld
endif

# temporary
LITEOS_PLATFORM_INCLUDE += \
        -I $(LITEOSTOPDIR)/kernel/base/include
LITEOS_CXXINCLUDE += \
        $(LITEOS_NET_INCLUDE) \
        -I $(LITEOSTOPDIR)/kernel/base/include

LITEOS_COPTS_NODEBUG    := $(LITEOS_NODEBUG) $(LITEOS_COPTS_BASE) $(LITEOS_COPTS_EXTRA)
LITEOS_COPTS_INTERWORK  := $(LITEOS_INTERWORK) $(LITEOS_COPTS_BASE) $(LITEOS_COPTS_EXTRA_INTERWORK)
LITEOS_BASE_INCLUDE := $(LITEOS_KERNEL_INCLUDE) $(LITEOS_PLATFORM_INCLUDE) \
                       $(LITEOS_LIB_INCLUDE) $(LITEOS_FS_INCLUDE) \
                       $(LITEOS_EXTKERNEL_INCLUDE) \
                       $(LITEOS_COMPAT_INCLUDE) $(LITEOS_DRIVERS_INCLUDE) \
                       $(LOSCFG_TOOLS_DEBUG_INCLUDE) $(LITEOS_NET_INCLUDE)
LITEOS_CFLAGS_INTERWORK := $(LITEOS_COPTS_INTERWORK) $(LITEOS_CMACRO) \
                           $(LITEOS_CMACRO_TEST)     $(LITEOS_IMAGE_MACRO) \
                           $(LITEOS_BASE_INCLUDE)
LITEOS_CFLAGS_NODEBUG := $(LITEOS_COPTS_NODEBUG) $(LITEOS_CMACRO) \
                         $(LITEOS_CMACRO_TEST)   $(LITEOS_IMAGE_MACRO) \
                         $(LITEOS_BASE_INCLUDE)
