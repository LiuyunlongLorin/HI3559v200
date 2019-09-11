LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../../..
include ../Makefile.param

CFLAGS += -I$(NDK_PATH)/code/include/


SRCS := $(wildcard *.c)
TARGET := $(SRCS:%.c=%)

# compile rule
include $(PWD)/../$(OSTYPE).mak
