include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := common

LOCAL_SRCS := $(wildcard *.c)

LOCAL_INCLUDE := \
    -I $(LITEOSTOPDIR)/compat/posix/src \
    -I $(LITEOSTOPDIR)/lib/libc/stdio \
    -I $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/include

LOCAL_FLAGS := $(LOCAL_INCLUDE)

include $(MODULE)
