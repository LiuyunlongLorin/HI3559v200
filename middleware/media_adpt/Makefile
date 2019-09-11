LOCAL_DIR := $(shell pwd)
SHELL := /bin/bash
MIDDLEWARE_DIR = $(LOCAL_DIR)/..
include $(MIDDLEWARE_DIR)/base.mak


.PHONY: download compile prepare deps_compile

ALL: adpt_compile

prepare: download deps_compile

deps_compile:
	make -C $(CFG_CHIP_TYPE) deps_compile

download:
	make -C $(CFG_CHIP_TYPE) download

adpt_compile: prepare
	make -C $(CFG_CHIP_TYPE) adpt_compile

adpt_clean:
	make -C $(CFG_CHIP_TYPE) adpt_clean

install: adpt_compile
	if [ ! -d $(OUT_HEADER_DIR) ]; then mkdir -p $(OUT_HEADER_DIR);fi
	cp -vf include/*.h $(OUT_HEADER_DIR)
	make -C $(CFG_CHIP_TYPE) install
