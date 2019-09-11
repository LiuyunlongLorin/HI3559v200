CUR_ROOT    := $(shell pwd)
SDK_ROOT    ?= $(CUR_ROOT)/../../../../../
include $(SDK_ROOT)/build/base.mak

.PHONY : all clean

all:
ifeq ($(CONFIG_GAUGE_BQ27421),y)
	@if [ -d bq27421 ];then cd bq27421; make;fi
endif
clean:
	@if [ -d bq27421 ];then cd bq27421; make clean;fi

