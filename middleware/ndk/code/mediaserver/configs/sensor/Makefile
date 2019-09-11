LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../../../..
MC_PARAM_FILE:=../../Makefile.param
include $(MC_PARAM_FILE)

.PHONY:all clean

all:
	@cd $(HIARCH); make
clean:
	@cd $(HIARCH); make clean

