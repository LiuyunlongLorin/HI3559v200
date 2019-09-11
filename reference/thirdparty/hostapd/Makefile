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
LOCAL_PREFIX  = $(shell pwd)/../_install

################################################################################
BUILD_DIR                := $(shell pwd)/tmp
HOST                     := $(patsubst %-,%,$(CROSS_COMPILE))
SRC                      := hostapd-2.7
UTILS                    := hostapd
UTILS_DIR                := $(BUILD_DIR)/$(SRC)/hostapd
TARGETS		         := $(addprefix $(PREFIX)/$(TOOLS_DIR)/,$(UTILS))
################################################################################
all: download $(TARGETS) entropy

download:
	@if [ ! -f $(SRC).tar.gz ]; then curl --insecure -O https://w1.fi/releases/$(SRC).tar.gz; fi

entropy:
	@if [ ! -f entropy.bin ]; then chmod +x generate_entropy.sh;./generate_entropy.sh;chmod 755 entropy.bin;fi

$(PREFIX)/$(TOOLS_DIR)/$(UTILS): $(UTILS_DIR)/config_finish
	make -C $(UTILS_DIR) CC=$(HOST)-gcc && \
	$(CROSS_COMPILE)strip $(UTILS_DIR)/$(UTILS) && \
	test -d $(PREFIX)/$(TOOLS_DIR) || mkdir -p $(PREFIX)/$(TOOLS_DIR) && \
	cp -fv $(UTILS_DIR)/$(UTILS) $(PREFIX)/$(TOOLS_DIR)/

$(UTILS_DIR)/config_finish: $(BUILD_DIR)/$(SRC)
	@(echo -e "\n# The follow config add by Makefile"; \
		echo "CONFIG_DRIVER_NL80211=y"; \
		echo "CONFIG_LIBNL20=y"; \
		echo "CONFIG_IEEE80211N=y"; \
		echo "CONFIG_TLS=internal"; \
		echo "CONFIG_INTERNAL_LIBTOMMATH=y"; \
		echo "CFLAGS += -I$(LOCAL_PREFIX)/include"; \
		echo "LIBS += -L$(LOCAL_PREFIX)/lib") >> $(UTILS_DIR)/.config
	touch $@

$(BUILD_DIR)/$(SRC):
	test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)
	tar -xzf $(SRC).tar.gz -C $(BUILD_DIR)

clean:
	-rm -rf $(BUILD_DIR)
	-rm -rf $(PREFIX)/$(TOOLS_DIR)/$(UTILS)
distclean: clean
	-rm -rf $(BUILD_DIR) $(addprefix $(PREFIX)/tools/,$(UTILS))
	-rm -f entropy.bin

################################################################################
.PHONY: clean distclean
################################################################################
