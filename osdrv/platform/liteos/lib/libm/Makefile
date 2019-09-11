include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := m

LOCAL_SRCS := $(wildcard src/*.c) \
    $(wildcard upstream-freebsd/lib/msun/src/*.c)

LOCAL_INCLUDE := \
    -I $(LITEOSTOPDIR)/lib/libm/upstream-freebsd/lib/msun/src

LOCAL_FLAGS := $(LOCAL_INCLUDE)

include $(MODULE)
