LOCAL_DIR := $(shell pwd)
SDK_ROOT ?= $(LOCAL_DIR)/../../..
include $(SDK_ROOT)/build/base.mak
include $(REF_ROOT)/build/compile_config.mak
################################################################################
ARCH		?=arm
#CROSS_COMPILE	?=arm-hisiv500-linux-
CROSS_COMPILE	:=$(CROSS)
PREFIX		?=$(shell pwd)/../out
LIBS_DIR	?= lib
TOOLS_DIR	?= tools
LOCAL_PREFIX         ?= $(shell pwd)/../_install
################################################################################
BUILD_DIR            := $(shell pwd)/tmp
HOST                 := arm-linux
CC                   := $(CROSS_COMPILE)gcc
SRC                  := libnl-3.0
UTILS                := libnl.so
TARGETS		     := $(addprefix $(PREFIX)/$(LIBS_DIR)/,$(UTILS))
################################################################################
all: download $(TARGETS)

download:
	@if [ ! -f $(SRC).tar.gz ]; then curl --insecure -O https://www.infradead.org/~tgr/libnl/files/$(SRC).tar.gz; fi

$(PREFIX)/$(LIBS_DIR)/$(UTILS): $(LOCAL_PREFIX)/$(UTILS)
	@test -d $(PREFIX)/$(LIBS_DIR) || mkdir -p $(PREFIX)/$(LIBS_DIR)
	@cp -rv $(LOCAL_PREFIX)/$(LIBS_DIR)/libnl.so.3.0.0  $(PREFIX)/$(LIBS_DIR)
	@cp -rv $(LOCAL_PREFIX)/$(LIBS_DIR)/libnl-genl.so.3.0.0  $(PREFIX)/$(LIBS_DIR)

$(LOCAL_PREFIX)/$(UTILS): $(BUILD_DIR)/$(SRC)
	@if [ -f $(LOCAL_PREFIX)/$(LIBS_DIR)/libnl.so.3.0.0 ] && \
		[ -f $(LOCAL_PREFIX)/$(LIBS_DIR)/libnl-genl.so.3.0.0 ]; then \
		echo "targets already exist"; \
	else \
		cd $(BUILD_DIR)/$(SRC) && \
			./configure --host=$(HOST) --prefix=$(LOCAL_PREFIX) CC=$(CC)&& \
			make && \
			make install && \
		cd -; \
	fi
	@cp -rv $(BUILD_DIR)/$(SRC)/include/linux $(LOCAL_PREFIX)/include/

$(BUILD_DIR)/$(SRC):
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)
	@tar -zxf $(SRC).tar.gz -C $(BUILD_DIR)

clean:
	-rm -rf $(BUILD_DIR) $(LOCAL_PREFIX)/$(LIBS_DIR)/$(UTILS)
	-rm -rf $(LOCAL_PREFIX)
	-rm -rf $(PREFIX)/$(LIBS_DIR)/libnl.so.3.0.0
	-rm -rf $(PREFIX)/$(LIBS_DIR)/libnl-genl.so.3.0.0
distclean: clean

################################################################################
.PHONY: clean distclean
################################################################################
