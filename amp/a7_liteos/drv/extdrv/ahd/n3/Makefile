
CUR_DIR  := $(shell pwd)
SDK_ROOT ?= $(CUR_DIR)/../../../../../../
include $(SDK_ROOT)/build/base.mak
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

CFLAGS += $(VSS_CFLAGS)

# Compile
SRCS := n3_drv.c n3_func.c litos_i2c.c
OBJS   := $(SRCS:%.c=%.o)
TARGET := hi_n3

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
