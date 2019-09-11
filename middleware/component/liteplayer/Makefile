MIDDLEWARE_DIR := $(shell pwd)/../..
include $(MIDDLEWARE_DIR)/base.mak

.PHONY: all clean install

LIB_PATH = $(shell pwd)/lib
HEADER_PATH = $(shell pwd)/include

LIB_FILE=$(LIB_PATH)/*.a
DLL_FILE=$(LIB_PATH)/*.so
HEADER_FILE=$(HEADER_PATH)/*.h

ifeq ($(CFG_OS_TYPE),linux_liteos)
LITEOS_LIB_PATH = $(LIB_PATH)/liteos
LITEOS_HEADER_PATH = $(HEADER_PATH)/liteos
LITEOS_LIB_FILE=$(LITEOS_LIB_PATH)/*.a
LITEOS_HEADER_FILE=$(LITEOS_HEADER_PATH)/*.h
endif

all: 
	make  -C src

clean:
	make -C src clean

install: all
	mkdir -p $(OUT_LIB_DIR)/
	mkdir -p $(OUT_HEADER_DIR)/
	cp -rf $(LIB_FILE) $(OUT_LIB_DIR)/
	cp -rf $(DLL_FILE) $(OUT_LIB_DIR)/
	cp -rf $(HEADER_FILE) $(OUT_HEADER_DIR)/
	mkdir -p $(OUT_LITEOS_LIB_DIR)/
	mkdir -p $(OUT_LITEOS_HEADER_DIR)/	
ifeq ($(CFG_OS_TYPE),linux_liteos)	
	cp -rf $(LITEOS_LIB_FILE) $(OUT_LITEOS_LIB_DIR)/
	cp -rf $(LITEOS_HEADER_FILE) $(OUT_LITEOS_HEADER_DIR)/
endif
# Clean this project
clean:
