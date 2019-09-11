include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := $(notdir $(shell pwd))

LOCAL_SRCS := $(wildcard src/base/*.c) $(wildcard src/cmds/*.c)

LOCAL_INCLUDE := \
	-I $(LITEOSTOPDIR)/fs/nfs \
	-I $(LITEOSTOPDIR)/shell/include \
	-I $(LITEOSTOPDIR)/fs/ramfs \
	-I $(LITEOSTOPDIR)/fs/fat \
	-I $(LITEOSTOPDIR)/kernel/base/include \

LOCAL_FLAGS := $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS)

include $(MODULE)
