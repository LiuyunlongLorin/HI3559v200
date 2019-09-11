#====================================================================================
#                          export variable
#====================================================================================
# MIDDLEWARE_DIR should defined in each Makefile, where include this file
include $(MIDDLEWARE_DIR)/cfg.mak

# Middleware use the same global varible names with Reference, e.g. CFG_SENSOR_TYPE0/1
# and NDK_ROOT, to keep configurable in both middleware package and Hi3559 package
PROJECT_ROOT := $(MIDDLEWARE_DIR)/..
#-include $(PROJECT_ROOT)/build/base.mak

COMPONENT_DIR := $(MIDDLEWARE_DIR)/component
COMMON_DIR := $(MIDDLEWARE_DIR)/common
NDK_DIR := $(MIDDLEWARE_DIR)/ndk
SAMPLE_DIR := $(MIDDLEWARE_DIR)/sample
MEDIA_ADPT_DIR := $(MIDDLEWARE_DIR)/media_adpt
THIRDPARTY_DIR := $(MIDDLEWARE_DIR)/thirdparty
OUT_DIR := $(MIDDLEWARE_DIR)/out
#if has ndk include ndk cfg
-include $(NDK_DIR)/build/config_$(CFG_CHIP_TYPE).mak
#AT := @
AT :=
#====================================================================================
#                          compile toolchain
#====================================================================================
-include $(MIDDLEWARE_DIR)/hichip_base_in.mak
-include $(MIDDLEWARE_DIR)/os_base_in.mak
#default disable miniaturized
CFG_ENABLE_MINI ?= n
ifeq ($(CFG_ENABLE_HUTAF_LLT_WRAPPER),y)
#HUTAF_LIB_DIR :=
HUTAF_LIBS := -lNCSCore_linux_HiSi-ARM
HUTAF_LLT_WRAPPER := lltwrapper
CC := $(HUTAF_LLT_WRAPPER) $(CFG_SDK_TOOLCHAIN)gcc
CXX := $(HUTAF_LLT_WRAPPER) $(CFG_SDK_TOOLCHAIN)g++
AR := $(CFG_SDK_TOOLCHAIN)ar
LD := $(HUTAF_LLT_WRAPPER) $(CFG_SDK_TOOLCHAIN)ld
else
CC := $(CFG_SDK_TOOLCHAIN)gcc
CXX := $(CFG_SDK_TOOLCHAIN)g++
AR := $(CFG_SDK_TOOLCHAIN)ar
LD := $(CFG_SDK_TOOLCHAIN)ld
endif

ifeq ($(FORTIFY_BUILD),1)
CC := sourceanalyzer -b hiberry_build $(CFG_SDK_TOOLCHAIN)gcc
CXX := sourceanalyzer -b hiberry_build $(CFG_SDK_TOOLCHAIN)g++
endif

STRIP := $(CFG_SDK_TOOLCHAIN)strip


CFLAGS +=-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -D_LARGEFILE64_SOURCE
CFLAGS += -DMW_VERSION=\"$(CFG_MW_VERSION)\"

SEC_CFLAGS := -O2
SEC_CFLAGS += -fstack-protector-all
SEC_CFLAGS += -fPIC

SEC_LFLAGS += -Wl,-z,relro,-z,now,-z,noexecstack
SEC_LFLAGS += -shared

CFLAGS += $(SEC_CFLAGS)
LFLAGS += $(SEC_LFLAGS)

CFLAGS += -ffunction-sections

ifeq ($(CFG_TURNON_PROC),y)
CFLAGS += -DENABLE_PROC
endif

ifeq ($(CFG_CFG_64BIT),y)
CFLAGS +=-Wextra -Wno-missing-field-initializers -Wformat=2 -Wpointer-arith
endif

ifeq ($(CFG_ENABLE_AUDIO),y)
CFLAGS += -DENABLE_AUDIO
endif

ifeq ($(CFG_ENABLE_FALLOCATE),y)
CFLAGS += -DFILE_FALLOCATE_ENABLE
endif

ifeq ($(CFG_ENABLE_DEBUG),y)
CFLAGS += -g
CFLAGS += -O0
endif

ifeq ($(CFG_ENABLE_SANITIZERS),y)
CFLAGS += -fsanitize=address -fsanitize=leak
endif

CXXFLAGS := $(CFLAGS)

#====================================================================================
#                          common funciton
#====================================================================================
CreateDir = $(shell [ -d $1 ] || mkdir -p $1 || echo ":mkdir '$1' fail")
RemoveDir = $(shell [ -d $1 ] && rm -rf $1 && echo -e "rmdir '$1'\t [ OK ]" || echo ":rm dir '$1' fail")
