include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := $(notdir $(shell pwd))

LOCAL_SRCS := $(wildcard src/*.c)

LOCAL_INCLUDE := \
	-I $(LITEOSTOPDIR)/fs/romfs/include \
	-I $(LITEOSTOPDIR)/fs/include \

LOCAL_FLAGS := $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS)

include $(MODULE)
