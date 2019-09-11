include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := csysdeps

LOCAL_INCLUDE := \
    -I $(LITEOSTOPDIR)/lib/libc \
    -I $(LITEOSTOPDIR)/lib/libc/private

LOCAL_SRCS = $(wildcard *.c) $(wildcard *.S)

LOCAL_FLAGS := $(LOCAL_INCLUDE)

include $(MODULE)
