include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := $(notdir $(shell pwd))

LOCAL_SRCS :=  $(wildcard src/*.c)

LOCAL_INCLUDE := \
    -I $(LITEOSTOPDIR)/compat/posix/src \
    -I $(LITEOSTOPDIR)/kernel/base/include \

LOCAL_FLAGS := $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS)

include $(MODULE)
