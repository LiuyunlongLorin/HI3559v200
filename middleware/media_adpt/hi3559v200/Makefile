SHELL := /bin/bash

COMPILE_ROOT := $(shell pwd)
MIDDLEWARE_DIR := $(COMPILE_ROOT)/../..
include $(MIDDLEWARE_DIR)/base.mak
NDK_ROOT := $(MIDDLEWARE_DIR)/ndk
SDK_LINK_ROOT := $(DEP_DIR)
NDK_LINK_ROOT := ndk
MPP_ROOT := $(SDK_ROOT)/amp/a7_linux/mpp
MW_STATIC_MEDIA_LIB := libmw_media_adpt.a
MW_SHARED_MEDIA_LIB := libmw_media_adpt.so
MW_STATIC_PROC_LIB := libuproc.a
MW_SHARED_PROC_LIB := libuproc.so
MW_LITEOS := liteos
FT_6236 := ft6236
ST_7789 := st7789

PROC_SRCS := hi_userproc.c
PROC_OBJ := $(patsubst %.c,%.o, $(PROC_SRCS))
SRCS := $(filter-out $(PROC_SRCS),$(wildcard *.c))
OBJS := $(filter-out $(PROC_OBJ),$(patsubst %.c,%.o, $(notdir $(SRCS))))
CFLAGS += -D__HI3559V200__  -Wno-error
.PHONY: compile deps_compile adpt_compile header_file_link ft6236_compile ft6236_clean st7789_compile st7789_clean


INCLUDE_PATH := -I../include \
	-I$(MPP_ROOT)/include \
	-I$(SDK_ROOT)/amp/a7_liteos/mpp/include \
	-I$(SDK_ROOT)/amp/a7_linux/hisyslink/datafifo/include \
	-I$(SDK_ROOT)/amp/a7_linux/hisyslink/ipcmsg/include \
	-I$(SDK_ROOT)/amp/a7_linux/drv/extdrv/userproc/include \
	-I$(NDK_ROOT)/code/mediaserver/adapt/aenc/include \
	-I$(NDK_ROOT)/out/hi3559v200/dual/host/include \
	-I$(NDK_ROOT)/code/include

download:
	@if [ -L $(SDK_LINK_ROOT) ]; then \
		echo "use internal sdk"; \
	else \
		echo "link sdk"; \
		ln -s  $(SDK_ROOT) $(SDK_LINK_ROOT); \
	fi
	@if [ -L $(NDK_LINK_ROOT) ]; then \
		echo "use internal sdk"; \
	else \
		echo "link ndk"; \
		ln -s  $(NDK_ROOT) $(NDK_LINK_ROOT); \
	fi
compile: download header_file_link deps_compile adpt_compile

deps_compile:
#	make -C $(REF_PATH) osdrv_clean
#	make -C $(REF_PATH) osdrv
#	make -C $(REF_PATH)/ndk all

adpt_compile: header_file_link $(MW_SHARED_MEDIA_LIB) $(MW_STATIC_MEDIA_LIB) $(MW_STATIC_PROC_LIB) $(MW_SHARED_PROC_LIB) liteos_adapt ft6236_compile st7789_compile

adpt_clean: ft6236_clean st7789_clean
	rm -fv *.o
	rm -fv *.a
	rm -fv *.so
	rm -rf $(OBJS)
	rm -rf $(SDK_LINK_ROOT)
	rm -rf $(NDK_LINK_ROOT)
	if [ -d $(MW_LITEOS) ]; then make -C $(MW_LITEOS) clean; fi
install:
	if [ ! -d $(OUT_LIB_DIR) ]; then mkdir -p $(OUT_LIB_DIR); fi
	if [ ! -d $(OUT_LITEOS_LIB_DIR) ]; then mkdir -p $(OUT_LITEOS_LIB_DIR); fi
	cp -v libmw_media_adpt.a $(OUT_LIB_DIR)
	cp -v libmw_media_adpt.so $(OUT_LIB_DIR)
	cp -v libuproc.a $(OUT_LIB_DIR)
	cp -v libuproc.so $(OUT_LIB_DIR)
	cp -v libmw_media_adpt_liteos.a $(OUT_LITEOS_LIB_DIR)
#	cp -v libmw_media_adpt_liteos.so $(OUT_LITEOS_LIB_DIR)

header_file_link:
	ln -sf $(MPP_ROOT)/include/hi_type.h ../include/hi_type.h

$(MW_STATIC_MEDIA_LIB): $(OBJS)
	$(AT)$(AR) -rcv $@ $^

$(MW_SHARED_MEDIA_LIB): $(OBJS)
	$(AT)$(CC) $(CFLAGS) -fPIC -shared -o $@ $^

$(OBJS):%.o:%.c
	$(AT)$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE_PATH)

$(MW_STATIC_PROC_LIB): $(PROC_OBJ)
	$(AT)$(AR) -rcv $@ $^

$(MW_SHARED_PROC_LIB): $(PROC_OBJ)
	$(AT)$(CC) $(CFLAGS) -fPIC -shared -o $@ $^

$(PROC_OBJ):%.o:%.c
	$(AT)$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE_PATH)

liteos_adapt: res_prepare
	if [ -d $(MW_LITEOS) ]; then make -C $(MW_LITEOS); fi

ft6236_compile:
	$(AT)make -C $(FT_6236) all

ft6236_clean:
	$(AT)make -C $(FT_6236) clean

st7789_compile:
	$(AT)make -C $(ST_7789) all

st7789_clean:
	$(AT)make -C $(ST_7789) clean
#cp resample so to sample.
res_prepare:
	mkdir -p $(MIDDLEWARE_DIR)/sample/liteos/so;
	if [ -f $(SDK_ROOT)/amp/a7_liteos/mpp/lib/libhive_record.so ]; then \
		cp  $(SDK_ROOT)/amp/a7_liteos/mpp/lib/libhive_record.so $(MIDDLEWARE_DIR)/sample/liteos/so/; \
		cp $(SDK_ROOT)/amp/a7_liteos/mpp/lib/libhive_RES* $(MIDDLEWARE_DIR)/sample/liteos/so/; \
	fi