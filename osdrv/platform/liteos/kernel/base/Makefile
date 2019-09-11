include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := $(notdir $(shell pwd))

LOCAL_SRCS := 	$(wildcard ipc/*.c) $(wildcard core/*.c)  $(wildcard mem/*.c)	\
		$(wildcard om/*.c)\
		$(wildcard misc/*.c)\
		$(wildcard mem/$(MEM_TYPE)/*.c)

ifeq ($(LOSCFG_MEM_RECORDINFO), y)
LOCAL_SRCS += $(wildcard mem/memrecord/*.c)
endif

LOCAL_INCLUDE := \
	-I $(LITEOSTOPDIR)/kernel/base/include -I $(LITEOSTOPDIR)/kernel/extended/include \
	-I $(LITEOSTOPDIR)/platform/cpu/arm/$(LITEOS_CPU_CAT)/$(LITEOS_CPU_TYPE)/include \
	-I $(LITEOSTOPDIR)/kernel/base/mem

LOCAL_FLAGS := $(LITEOS_CFLAGS_INTERWORK) $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS)

include $(MODULE)
