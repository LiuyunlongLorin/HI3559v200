
CUR_DIR  := $(shell pwd)
SDK_ROOT ?= $(CUR_DIR)/../../../../../../
include $(SDK_ROOT)/build/base.mak
include $(SDK_ROOT)/build/kconfig.mak
dummy    := $(call CreateDir, $(DRV_REL_PATH_LITEOS))

# HuaweiLite Compile Platform
export LITEOS_PLATFORM ?= $(CFG_CHIP_TYPE)
export LITEOSTOPDIR    ?= $(LITEOS_ROOT)
include $(LITEOSTOPDIR)/config.mk
VSS_CFLAGS += -fno-builtin -nostdinc -nostdlib
VSS_CFLAGS += $(LITEOS_MACRO) $(LITEOS_OSDRV_INCLUDE) $(LITEOS_USR_INCLUDE)
VSS_CFLAGS += -D__HuaweiLite__
CC      = $(HUAWEILITE_CROSS)-gcc
AR      = $(HUAWEILITE_CROSS)-ar

ifeq ($(CFG_CHIP_TYPE), hi3556av100)
VSS_CFLAGS += -DHI3556AV100
else ifeq ($(CFG_CHIP_TYPE), hi3519av100)
VSS_CFLAGS += -DHI3556AV100 -DHI3519AV100
else ifeq ($(CFG_CHIP_TYPE), hi3559v200)
VSS_CFLAGS += -DHI3559V200
endif

ifeq ($(CONFIG_DEMB), y)
VSS_CFLAGS += -DBOARD_DEMB
else ifeq ($(CONFIG_REFB_ACTIONCAM), y)
VSS_CFLAGS += -DBOARD_ACTIONCAM_REFB
endif

CFLAGS += $(VSS_CFLAGS)

# Compile
SRCS   := hi_ssp_st7796.c
OBJS   := $(SRCS:%.c=%.o)
TARGET := hi_ssp_st7796s_rgb16bit

LIB    := lib$(TARGET).a

.PHONY: all clean

all : prepare $(OBJS) $(LIB) success

prepare:
	@echo "";echo ""
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo -e "\033[31;32m COMPILING `basename $(LIB)` ... ... \033[0m"
	@echo ""

success: $(TARGET_LIB) $(TARGET_SHAREDLIB)
	@echo ""
	@echo -e "\033[31;32m `basename $(LIB)` Was SUCCESSFUL COMPILE \033[0m"
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo "";echo ""
	@cp -f $(CUR_DIR)/*.a $(DRV_REL_PATH_LITEOS)/

$(OBJS): %.o : %.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	@rm -f $(OBJS) $(LIB)
