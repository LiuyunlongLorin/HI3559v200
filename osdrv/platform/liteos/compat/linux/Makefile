include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := $(notdir $(shell pwd))

LOCAL_SRCS :=  $(wildcard src/*.c)
ifeq ($(LITEOS_CPU_TYPE), cortex-a53_aarch64)
    LOCAL_SRCS +=  $(wildcard src/arm-arch/a53/*.c)
else ifeq ($(LITEOS_CPU_TYPE), cortex-m7)
    LOCAL_SRCS +=
else
    LOCAL_SRCS +=  $(wildcard src/arm-arch/common/*.S)
endif

LOCAL_INCLUDE := \
        -I $(LITEOSTOPDIR)/kernel/base/include \
        -I $(LITEOSTOPDIR)/lib/libc/tzcode \
        -I $(LITEOSTOPDIR)/compat/linux/include \

LOCAL_FLAGS := $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS)

include $(MODULE)
