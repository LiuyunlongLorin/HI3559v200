include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := c

LOCAL_SRCS = $(wildcard src/string/*.c) \
    $(wildcard src/stdlib/*.c) \
    $(wildcard src/stdio/*.c) \
    $(wildcard src/time/*.c) \
    $(wildcard src/libwc/*.c) \
    $(wildcard src/unistd/*.c) \
    $(wildcard src/*.c) \
    $(wildcard upstream-openbsd/lib/libc/stdio/*.c) \
    $(wildcard upstream-netbsd/lib/libc/stdlib/*.c) \
    $(wildcard upstream-openbsd/lib/libc/gen/*.c) \
    $(wildcard libc/dns/resolv/*.c)

LOCAL_SRCS += $(wildcard src/libwc/*.cpp) \
    $(wildcard src/*.cpp)

LOCAL_INCLUDE := \
    -I $(LITEOSTOPDIR)/lib/libc/upstream-openbsd/lib/libc/include \
    -I $(LITEOSTOPDIR)/lib/libc/upstream-openbsd/lib/libc/gdtoa \
    -I $(LITEOSTOPDIR)/lib/libc/upstream-openbsd/lib/libc/crypt \
    -I $(LITEOSTOPDIR)/lib/libc/upstream-openbsd/lib/libc/stdio \
    -I $(LITEOSTOPDIR)/lib/libc/upstream-openbsd/android/include \
    -I $(LITEOSTOPDIR)/lib/libc/upstream-netbsd/android/include \
    -I $(LITEOSTOPDIR)/lib/libc \
    -I $(LITEOSTOPDIR)/lib/libc/include \
    -I $(LITEOSTOPDIR)/lib/libc/private \
    -I $(LITEOSTOPDIR)/lib/libc/stdio \
    -I $(LITEOSTOPDIR)/lib/libc/tzcode \

ifeq ($(LOSCFG_LLTSER), y)
LOCAL_INCLUDE += -I $(LITEOSTOPDIR)/tools/gcov_ser
endif

LOCAL_FLAGS := $(LOCAL_INCLUDE)

include $(MODULE)
