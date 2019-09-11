# Hisilicon Hi35xx sample Makefile
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak
include $(MEDIA_ADPT_DIR)/medialibs.mak

PLAYER_DIR := $(LOCAL_DIR)/../liteplayer/player_sample/

SRCS  := $(LOCAL_DIR)/mw_sample.c
SRCS  += $(LITEOS_ROOT)/kernel/extended/dynload/src/los_dynload_gsymbol.c
SRCS  += $(LOCAL_DIR)/app_init.c

OBJS  := $(SRCS:%.c=%.o)

TARGET := media_app

TARGET_ZIP := media_app_zip.bin

ACODEC_TYPE ?= ACODEC_TYPE_INNER

LiteCFLAGS += -Wno-error -DHI_ACODEC_TYPE_INNER -DHI_ACODEC_TYPE_HDMI

APP_LIB_PATH := -L$(COMPONENT_DIR)/liteplayer/lib/liteos/
APP_LIB_PATH += $(MEDIA_LITEOS_LIBS_PATH_LINK)

APP_LIB :=$(APP_LIB_PATH) -lavplay_liteos -lfsm_liteos --start-group $(MEDIA_LITEOS_LIBS_LINK)  --end-group


LITEOS_LIBDEPS = --start-group $(LITEOS_LIBDEP) --end-group $(LITEOS_TABLES_LDFLAGS)

LDFLAGS := $(LITEOS_LDFLAGS) --gc-sections

APP_INC := $(MEDIA_LITEOS_INCLUDE_PATH) -I$(COMPONENT_DIR)/liteplayer/include/liteos

BIN := $(TARGET).bin
MAP := $(TARGET).map

ifeq ($(CFG_CHIP_TYPE),hi3559av100)
LiteCFLAGS += -D__HI3559AV100__
else ifeq ($(CFG_CHIP_TYPE),hi3559v200)
LiteCFLAGS += -D__HI3559V200__
else ifeq ($(CFG_CHIP_TYPE),hi3556av100)
LiteCFLAGS += -D__HI3556AV100__
endif

.PHONY : clean all prepare

all: prepare $(BIN)

prepare:
	pushd $(PLAYER_DIR); make; popd;
	mkdir -p so;cp $(PLAYER_DIR)/AAC/lib*.so so/
	@echo "./sym.sh so";cd $(LITEOSTOPDIR)/tools/scripts/dynload_tools;chmod 777 * -R;./sym.sh $(LOCAL_DIR)/so
	@if [ -f $(OSDRV_ROOT)/pub/bin/pc/hi_gzip ]; then \
	cp -rf $(OSDRV_ROOT)/pub/bin/pc/hi_gzip .; \
	fi
	@if [ -f $(OSDRV_ROOT)/pub/bin/pc/gzip ]; then \
	cp -rf $(OSDRV_ROOT)/pub/bin/pc/gzip .; \
	fi

$(BIN):$(TARGET)
	@$(OBJCOPY) -O binary $(TARGET) $(BIN)
	@if [ -f hi_gzip ]; then \
	./hi_gzip $(TARGET).bin $(TARGET_ZIP); \
	fi


$(TARGET):$(OBJS)
	$(LD) $(LDFLAGS) -Map=$(MAP) -o $(TARGET) $(OBJS) $(APP_LIB) $(LITEOS_LIBDEPS)
	@$(OBJDUMP) -d $(TARGET) > $(TARGET).asm

$(OBJS):%.o:%.c
	$(CC)  $(LiteCFLAGS) $(APP_INC) -c $< -o $@

clean:
	@rm -f $(TARGET) $(BIN) $(MAP) *.asm
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)
	@rm -rf ./so
	@rm -rf hi_gzip
	@rm -rf gzip
	@rm -rf $(TARGET_ZIP)
