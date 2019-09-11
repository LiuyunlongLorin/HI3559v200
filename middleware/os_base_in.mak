ifeq ($(CFG_OS_TYPE),linux_liteos)
export LITEOSTOPDIR=$(LITEOS_ROOT)
include $(LITEOSTOPDIR)/config.mk
LiteCFLAGS := $(CFLAGS)
LiteCFLAGS += -fno-builtin -nostdinc -nostdlib
LiteCFLAGS += $(LITEOS_CXXFLAGS) $(LITEOS_MACRO) $(LITEOS_OSDRV_INCLUDE) $(LITEOS_USR_INCLUDE) $(LITEOS_CMACRO)
LiteCFLAGS += -D__HuaweiLite__ -D__KERNEL__
endif

ifeq ($(CFG_OS_TYPE),liteos)
export LITEOSTOPDIR=$(LITEOS_ROOT)
include $(LITEOSTOPDIR)/config.mk
CFLAGS += -fno-builtin -nostdinc -nostdlib
CFLAGS += $(LITEOS_CXXFLAGS) $(LITEOS_OSDRV_INCLUDE) $(LITEOS_USR_INCLUDE)
CFLAGS += -D__HuaweiLite__ -D__KERNEL__
endif

ifeq ($(CFG_OS_TYPE),linux_liteos)
OUT_LIB_DIR := $(OUT_DIR)/linux/lib
OUT_HEADER_DIR := $(OUT_DIR)/linux/include
OUT_LITEOS_LIB_DIR := $(OUT_DIR)/liteos/lib
OUT_LITEOS_HEADER_DIR := $(OUT_DIR)/liteos/include
else
OUT_LIB_DIR := $(OUT_DIR)/lib
OUT_HEADER_DIR := $(OUT_DIR)/include
OUT_LITEOS_LIB_DIR := $(OUT_DIR)/lib
OUT_LITEOS_HEADER_DIR := $(OUT_DIR)/include
endif
