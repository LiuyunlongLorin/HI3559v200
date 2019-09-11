LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../../..
MC_PARAM_FILE:=../Makefile.param
include $(MC_PARAM_FILE)

CFLAGS += -I$(NDK_PATH)/code/include/

SRCS := $(wildcard *.c)
TARGET := sample_dump

# compile rule
include $(PWD)/../$(OSTYPE).mak
