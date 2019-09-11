include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := bsp

LOCAL_SRCS = $(wildcard board/$(LITEOS_PLATFORM)/*.c) \
            $(wildcard $(HWI_SRC)/*.c) \
            $(wildcard $(MMU_SRC)/*.c) \
            $(wildcard $(TIMER_SRC)/*.c) \
            $(wildcard $(UART_SRC)/*.c)

LOCAL_INCLUDE := \

ifeq ($(LOSCFG_PLATFORM_DVFS), y)
LOCAL_SRCS += $(wildcard board/$(LITEOS_PLATFORM)/dvfs/*.c)
endif

ifeq ($(LOSCFG_SHELL), y)
LOCAL_SRCS += $(wildcard board/$(LITEOS_PLATFORM)/extcmd/*.c)
endif

ifeq ($(LOSCFG_DRIVERS_MMC), y)
LOCAL_SRCS += $(wildcard board/$(LITEOS_PLATFORM)/mmc/*.c)
MMC_INCLUDE += -I $(LITEOSTOPDIR)/drivers/mmc/host/$(MMC_HOST_DIR)
LOCAL_INCLUDE += $(MMC_INCLUDE)

endif

ifeq ($(LOSCFG_DRIVERS_NETDEV), y)
LOCAL_SRCS += $(wildcard $(NET_SRC)/*.c)
endif

ifeq ($(LOSCFG_DRIVERS_RANDOM), y)
LOCAL_SRCS += $(wildcard board/$(LITEOS_PLATFORM)/random/*.c)
endif

ifeq ($(LOSCFG_DRIVERS_USB), y)
LOCAL_SRCS += $(wildcard $(USB_SRC)/*.c)
endif

ifeq ($(LOSCFG_PLATFORM_ADAPT), y)
LOCAL_SRCS += $(wildcard board/$(LITEOS_PLATFORM)/os_adapt/*.c)
endif

LOCAL_INCLUDE += -I $(wildcard board)/$(LITEOS_PLATFORM)/include/pm
LOCAL_SRCS += $(wildcard board/$(LITEOS_PLATFORM)/pm/*.c)

ifeq ($(LOSCFG_KERNEL_TICKLESS), y)
LOCAL_INCLUDE += -I $(LITEOSTOPDIR)/kernel/extended/include
endif

ifeq ($(LOSCFG_CPUP_INCLUDE_IRQ), y)
LOCAL_INCLUDE += -I $(LITEOSTOPDIR)/kernel/extended/include
endif

ALL_ASSRCS := $(wildcard board/$(LITEOS_PLATFORM)/*.S)
ASSRCS := $(subst board/$(LITEOS_PLATFORM)/board.ld.S,,$(ALL_ASSRCS))

LOCAL_SRCS += $(ASSRCS)

LOCAL_FLAGS := $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS)

include $(MODULE)
