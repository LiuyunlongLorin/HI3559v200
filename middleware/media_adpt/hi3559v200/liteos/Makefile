SHELL := /bin/bash
COMPILE_ROOT := $(shell pwd)
MIDDLEWARE_DIR := $(COMPILE_ROOT)/../../../
include $(MIDDLEWARE_DIR)/base.mak
NDK_ROOT := $(MIDDLEWARE_DIR)/ndk
MPP_ROOT := $(SDK_ROOT)/amp/a7_liteos/mpp
MW_STATIC_MEDIA_LIB := $(COMPILE_ROOT)/../libmw_media_adpt_liteos.a
MW_SHARED_MEDIA_LIB := $(COMPILE_ROOT)/../libmw_media_adpt_liteos.so
HIARCH := hi3559v200
SRCS := $(wildcard *.c)
#SRCS  += $(MPP_ROOT)/init/sdk_init.c
#SRCS  += $(MPP_ROOT)/init/sdk_exit.c
OBJS := $(patsubst %.c,%.o,$(SRCS))
LiteCFLAGS +=  -D__HI3559V200__ -Wno-error -DHI_ACODEC_TYPE_INNER -DHI_ACODEC_TYPE_HDMI
INCLUDE_PATH := -I../include \
	-I$(MPP_ROOT)/include \
	-I$(SDK_ROOT)/amp/a7_liteos/osal/include \
	-I$(SDK_ROOT)/amp/a7_liteos/hisyslink/ipcmsg/include \
	-I$(SDK_ROOT)/amp/a7_liteos/hisyslink/datafifo/include \
	-I$(NDK_ROOT)/out/$(HIARCH)/dual/slave/include

.PHONY: all clean install uninstall

all: $(MW_SHARED_MEDIA_LIB) $(MW_STATIC_MEDIA_LIB)

clean:
	rm -fv *.o
	rm -fv *.a
	rm -fv *.so
	rm -rf $(OBJS)
	rm -rf $(MW_STATIC_MEDIA_LIB)
	rm -rf $(MW_SHARED_MEDIA_LIB)

install:
	if [ ! -d $(OUT_LITEOS_LIB_DIR) ]; then mkdir -p $(OUT_LITEOS_LIB_DIR);fi
	cp -v *.a $(OUT_LITEOS_LIB_DIR)
	cp -v *.so $(OUT_LITEOS_LIB_DIR)

$(MW_STATIC_MEDIA_LIB): $(OBJS)
	$(AT)$(AR) -rcv $@ $^

$(MW_SHARED_MEDIA_LIB): $(OBJS)
#	$(AT)$(CC) $(LiteCFLAGS) -fPIC -shared -o $@ $^

$(OBJS):%.o:%.c
	$(AT)$(CC) $(LiteCFLAGS) -c $< -o $@ $(INCLUDE_PATH)