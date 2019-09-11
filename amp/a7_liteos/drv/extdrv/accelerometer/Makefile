CUR_ROOT    := $(shell pwd)
SDK_ROOT    ?= $(CUR_ROOT)/../../../
include $(SDK_ROOT)/build/base.mak

.PHONY : all clean

all:
ifeq ($(CONFIG_GSENSOR_MC3416),y)
	@if [ -d mc3416 ];then cd mc3416; make;fi
endif
clean:
	@if [ -d mc3416 ];then cd mc3416; make clean;fi
