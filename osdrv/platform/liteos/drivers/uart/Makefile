include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := $(notdir $(shell pwd))

LOCAL_SRCS := $(wildcard src/*.c)

UART_INCLUDE += -I $(LITEOSTOPDIR)/drivers/uart/src
LOCAL_FLAGS := $(UART_INCLUDE)

include $(MODULE)
