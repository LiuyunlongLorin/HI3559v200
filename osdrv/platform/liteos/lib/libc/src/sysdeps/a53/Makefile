include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := csysdeps

LOCAL_INCLUDE := \
    -I $(LITEOSTOPDIR)/lib/libc \
    -I $(LITEOSTOPDIR)/lib/libc/private

LOCAL_SRCS := $(wildcard ./*.c) memcpy.S memset.S memmove.S memchr.S strlen.S strcmp.S strncmp.S strnlen.S strcpy.S memcmp.S

LOCAL_FLAGS := $(LOCAL_INCLUDE)

include $(MODULE)
