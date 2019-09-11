CUR_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(CUR_DIR)/../../..
include $(MIDDLEWARE_DIR)/base.mak

LIB_FILE := $(shell find $(CUR_DIR) -name '*.a')
DLL_FILE := $(shell find $(CUR_DIR) -name '*.so')
HEADER_FILE := $(shell find $(CUR_DIR) -name '*.h')

.PHONY: all clean install 

all:   $(LIB_FILE) $(DLL_FILE) $(HEADER_FILE)
install: all
	$(AT)mkdir -p $(OUT_LIB_DIR)
	$(AT)mkdir -p $(OUT_HEADER_DIR)
	$(AT)cp -rf $(LIB_FILE) $(OUT_LIB_DIR)/
	$(AT)cp -rf $(DLL_FILE) $(OUT_LIB_DIR)/
	$(AT)cp -rf $(HEADER_FILE) $(OUT_HEADER_DIR)/
