MIDDLEWARE_DIR := $(shell pwd)/../..
include $(MIDDLEWARE_DIR)/base.mak

.PHONY: all clean install

LIB_PATH = $(shell pwd)/lib
HEADER_PATH = $(shell pwd)/include
LIB_FILE=$(LIB_PATH)/*.a
DLL_FILE=$(LIB_PATH)/*.so
HEADER_FILE=$(HEADER_PATH)/*.h

all:    $(LIB_FILE) $(DLL_FILE) $(HEADER_FILE)
install: all
	cp -rf $(LIB_FILE) $(OUT_LIB_DIR)/
	cp -rf $(DLL_FILE) $(OUT_LIB_DIR)/
	cp -rf $(HEADER_FILE) $(OUT_HEADER_DIR)/

# Clean this project
clean:
