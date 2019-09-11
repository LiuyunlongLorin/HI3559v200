LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR = $(LOCAL_DIR)/..
include $(MIDDLEWARE_DIR)/base.mak

LIVESTREAM_DIR := livestream
HIOSCSERVER_DIR := hioscserver
HIOSCCLIENT_DIR := hioscclient
LITEPLAYER_DIR := liteplayer
SAMPLE_COMMON_DIR := common
LITEOS_SAMPLE_DIR := liteos
#===============================================================================
#                         rules
#===============================================================================
.PHONY : all clean install
#===============================================================================
#                         rtspserver
#===============================================================================
.PHONY : rtspserver rtspserver_clean

exclude_samples := HuaweiLite livestream common phone mp4_editor

ifneq ($(CFG_OS_TYPE),linux_liteos)
exclude_samples += $(LITEOS_SAMPLE_DIR)
endif

ifeq ($(CFG_CHIP_TYPE),hi3559)
exclude_samples += wav_muxer
else ifeq ($(CFG_CHIP_TYPE),hi3556)
exclude_samples += wav_muxer
else ifeq ($(CFG_CHIP_TYPE),hi3559aes)
exclude_samples += wav_muxer
else ifeq ($(CFG_CHIP_TYPE),hi3516cv300)
exclude_samples += dng_muxer dng_demuxer wav_muxer
else ifeq ($(CFG_CHIP_TYPE),hi3518ev300)
exclude_samples += dng_muxer dng_demuxer wav_muxer
endif

ifeq ($(CFG_CHIP_TYPE),hi3559)
exclude_samples += liteplayer
endif

ifeq ($(CFG_CHIP_TYPE),hi3556)
exclude_samples += liteplayer
endif

exclude_samples += mp4_editor
samples := $(shell find . -maxdepth 1 -type d)
samples := $(basename $(patsubst ./%,%,$(samples)))
samples := $(filter-out $(exclude_samples), $(samples))

all:
	if [ -d $(LIVESTREAM_DIR) ]; then make -C $(LIVESTREAM_DIR)||exit 1;fi
	if [ -d $(SAMPLE_COMMON_DIR) ]; then make -C $(SAMPLE_COMMON_DIR)||exit 1;fi
	if [ -d $(HIOSCSERVER_DIR) ]; then make -C $(HIOSCSERVER_DIR)||exit 1;fi
	if [ -d $(HIOSCCLIENT_DIR) ]; then make -C $(HIOSCCLIENT_DIR)||exit 1;fi
	for sample in $(samples);do make -C $$sample||exit 1;done

clean:
	if [ -d $(LIVESTREAM_DIR) ]; then make -C $(LIVESTREAM_DIR) clean;fi
	if [ -d $(SAMPLE_COMMON_DIR) ]; then make -C $(SAMPLE_COMMON_DIR) clean;fi
	if [ -d $(HIOSCSERVER_DIR) ]; then make -C $(HIOSCSERVER_DIR) clean;fi
	if [ -d $(HIOSCCLIENT_DIR) ]; then make -C $(HIOSCCLIENT_DIR) clean;fi
	for sample in $(samples);do make -C $$sample clean;done

install:all
	if [ -d $(LIVESTREAM_DIR) ]; then make -C $(LIVESTREAM_DIR) install||exit 1;fi
	if [ -d $(HIOSCSERVER_DIR) ]; then make -C $(HIOSCSERVER_DIR) install||exit 1;fi
	if [ -d $(HIOSCCLIENT_DIR) ]; then make -C $(HIOSCCLIENT_DIR) install||exit 1;fi
	if [ -d $(LITEPLAYER_DIR) ]; then make -C $(LITEPLAYER_DIR) install||exit 1;fi
