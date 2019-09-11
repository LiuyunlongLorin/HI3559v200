CUR_ROOT    := $(shell pwd)
HAL_ROOT    ?= $(CUR_ROOT)/..
SDK_ROOT    ?= $(HAL_ROOT)/../..
include $(SDK_ROOT)/build/base.mak

.PHONY : all clean

all:
	@cd comm;    make
ifeq ($(CONFIG_SCREEN_OTA5182),y)
	@cd ota5182; make
endif
ifeq ($(CONFIG_SCREEN_ST7796S),y)
	@cd st7796s; make
endif
ifeq ($(CONFIG_SCREEN_ST7789),y)
	@cd st7789; make
endif
ifeq ($(CONFIG_SCREEN_ST7796S_RGB16BIT),y)
	@cd st7796s_rgb16bit; make
endif
ifeq ($(CONFIG_SCREEN_OTA7290B),y)
	@cd ota7290b; make
endif
ifeq ($(CONFIG_SCREEN_ILI8961),y)
	@cd ili8961; make
endif
clean:
	@if [ -d comm ]; then cd comm; make clean; fi
	@if [ -d ota5182 ]; then cd ota5182; make clean; fi
	@if [ -d st7796s ]; then cd st7796s; make clean; fi
	@if [ -d st7789 ]; then cd st7789; make clean; fi
	@if [ -d st7796s_rgb16bit ]; then cd st7789; make clean; fi
	@if [ -d ota7290b ]; then cd ota7290b; make clean; fi
	@if [ -d ili8961 ]; then cd ili8961; make clean; fi