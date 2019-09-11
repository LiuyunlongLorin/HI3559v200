CUR_ROOT    := $(shell pwd)
SDK_ROOT    ?= $(CUR_ROOT)/../../../../../
include $(SDK_ROOT)/build/base.mak

.PHONY : all clean

all:
ifeq ($(CONFIG_TOUCHPAD_FT6236),y)
	@if [ -d ft6236 ];then cd ft6236; make;fi
endif
ifeq ($(CONFIG_TOUCHPAD_FT6356),y)
	@if [ -d ft6356 ];then cd ft6356; make;fi
endif
ifeq ($(CONFIG_TOUCHPAD_GT911),y)
	@if [ -d gt911 ];then cd gt911; make;fi
endif
clean:
	@if [ -d ft6236 ];then cd ft6236; make clean;fi
	@if [ -d ft6356 ];then cd ft6356; make clean;fi
	@if [ -d gt911 ];then cd gt911; make clean;fi
