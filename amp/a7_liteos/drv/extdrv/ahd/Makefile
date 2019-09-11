CUR_ROOT    := $(shell pwd)
SDK_ROOT    ?= $(CUR_ROOT)/../../../../../
include $(SDK_ROOT)/build/base.mak

.PHONY : all clean

all:
ifeq ($(CONFIG_AHD_N3),y)
	@if [ -d n3 ];then cd n3; make;fi
endif
clean:
	@if [ -d CONFIG_AHD_N3 ];then cd n3; make clean;fi

