ifeq ($(AMP_TYPE), liteos)
OS_TYPE := HuaweiLite
endif

# Common Compile CFlags
ifeq ($(OS_TYPE),HuaweiLite)
    export LITEOS_PLATFORM ?= $(CFG_CHIP_TYPE)
    export LITEOSTOPDIR    ?= $(LITEOS_ROOT)
    include $(LITEOSTOPDIR)/config.mk
    VSS_CFLAGS += -fno-builtin -nostdinc -nostdlib
    ifeq ($(CONFIG_HI3518EV300), y)
    VSS_CFLAGS += $(LITEOS_CFLAGS)
    else
    VSS_CFLAGS += $(LITEOS_MACRO) $(LITEOS_OSDRV_INCLUDE) $(LITEOS_USR_INCLUDE)
    endif
    VSS_CFLAGS += -D__HuaweiLite__
    ifeq ($(CONFIG_HI3559AV100), y)
    VSS_CFLAGS += -mcpu=cortex-a53
    else ifeq ($(CONFIG_HI3556AV100), y)
    VSS_CFLAGS += -mcpu=cortex-a53 -mfloat-abi=softfp -mfpu=neon-vfpv4
    else ifeq ($(CONFIG_HI3559V200), y)
    VSS_CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
    else ifeq ($(CONFIG_HI3556V200), y)
    VSS_CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
    endif
else
    ifeq ($(CONFIG_HI3559AV100), y)
    #VSS_CFLAGS += -mcpu=cortex-a73.cortex-a53
    else ifeq ($(CONFIG_HI3556AV100), y)
    VSS_CFLAGS += -mcpu=cortex-a53 -mfloat-abi=softfp -mfpu=neon-vfpv4
    else ifeq ($(CONFIG_HI3559V200), y)
    VSS_CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
    else ifeq ($(CONFIG_HI3556V200), y)
    VSS_CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
    endif
    VSS_CFLAGS += -D__LINUX__
endif
VSS_CFLAGS += -fno-aggressive-loop-optimizations
VSS_CFLAGS += -ffunction-sections -fdata-sections -funwind-tables
VSS_CFLAGS += -Wall -Warray-bounds -Werror
VSS_CFLAGS += -Wno-error=strict-prototypes
VSS_CFLAGS += -fPIC -fstack-protector
VSS_CFLAGS += --param ssp-buffer-size=4
#VSS_CFLAGS += -Wstack-protector
VSS_CFLAGS += -Wl,-z,relro -Wl,-z,noexecstack
VSS_CFLAGS += -D_GNU_SOURCE

ifeq ($(CONFIG_DEBUG), y)
VSS_CFLAGS += -g3
VSS_CFLAGS += -DCFG_AUTOTEST_ENABLE
else ifeq ($(CONFIG_RELEASE), y)
VSS_CFLAGS += -Os
VSS_CFLAGS += -DNDEBUG
endif

ifneq ($(CONFIG_LOG_LEVEL_DEBUG), y)
VSS_CFLAGS += -Wno-error=unused-variable -Wno-error=unused-but-set-variable
endif

# .Config Compile CFLAGS
VSS_CFLAGS += $(KCONFIG_CFLAGS)

VSS_CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

# CROSS COMPILE TOOLCHAIN
ifeq ($(OS_TYPE),HuaweiLite)
CROSS   = $(HUAWEILITE_CROSS)-
  ifeq ($(HUAWEILITE_ARCH64),y)
    ARCH64 = y
  endif
else
CROSS   = $(LINUX_CROSS)-
  ifeq ($(LINUX_ARCH64),y)
    ARCH64 = y
  endif
endif
ifeq ($(CONFIG_HLLT),y)
CC      = lltwrapper $(CROSS)gcc
CXX     = lltwrapper $(CROSS)gcc
CCDEP   = lltwrapper $(CROSS)gcc
AR      = lltwrapper $(CROSS)ar
LINK    = lltwrapper $(CROSS)gcc
else
CC      = $(CROSS)gcc
CXX     = $(CROSS)gcc
CCDEP   = $(CROSS)gcc
AR      = $(CROSS)ar
LINK    = $(CROSS)gcc
endif

STRIP   = $(CROSS)strip
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
RANLIB  = true
ARFLAGS = rcv

ifeq ($(ARCH64),y)
VSS_CFLAGS += -Wformat -Wpointer-arith
else
VSS_CFLAGS += -mno-unaligned-access
endif
