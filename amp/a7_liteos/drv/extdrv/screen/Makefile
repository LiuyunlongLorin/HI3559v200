CUR_ROOT    := $(shell pwd)
SDK_ROOT    ?= $(CUR_ROOT)/../../../../../
include $(SDK_ROOT)/build/base.mak

.PHONY : all clean

all:
ifeq ($(CONFIG_SCREEN_ST7789),y)
	@if [ -d st7789 ];then cd st7789; make;fi
endif
ifeq ($(CONFIG_SCREEN_ST7796S_RGB16BIT),y)
	@if [ -d st7796s_rgb16bit ];then cd st7796s_rgb16bit; make;fi
endif
ifeq ($(CONFIG_SCREEN_ILI8961),y)
	@if [ -d ili8961 ];then cd ili8961; make;fi
endif
clean:
	@if [ -d st7796s_rgb16bit ];then cd st7796s_rgb16bit; make clean;fi
	@if [ -d ili8961 ];then cd ili8961; make clean;fi

