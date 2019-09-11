LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/cfg.mak

WIDGET_SAMPLE_DIR    := widget_sample
SCENE_SAMPLE_DIR     := scene_sample
AUTO_SAMPLE_DIR      := auto_sample
HI3559A_SAMPLE_DIR   := 3559a_sample
#===============================================================================
#                         rules
#===============================================================================
.PHONY : all clean

ifeq ($(CFG_OS_TYPE), liteos)
all: auto_sample
clean: auto_sample_clean
else ifeq ($(CFG_CHIP_TYPE), hi3559av100)
all: 3559a_sample
clean: 3559a_sample_clean
else
all: widget_sample scene_sample
clean: widget_sample_clean scene_sample_clean
endif
#===============================================================================
#                         widget_sample
#===============================================================================
.PHONY : widget_sample widget_sample_clean

widget_sample:
	make -C $(WIDGET_SAMPLE_DIR)
widget_sample_clean:
	make -C $(WIDGET_SAMPLE_DIR) clean
#===============================================================================
#                         scene_sample
#===============================================================================
.PHONY : scene_sample scene_sample_clean

scene_sample:
	make -C $(SCENE_SAMPLE_DIR)
scene_sample_clean:
	make -C $(SCENE_SAMPLE_DIR) clean
#===============================================================================
#                         auto_sample
#===============================================================================
.PHONY : auto_sample auto_sample_clean

auto_sample:
	make -C $(AUTO_SAMPLE_DIR)
auto_sample_clean:
	make -C $(AUTO_SAMPLE_DIR) clean
#===============================================================================
#                         3559a_sample
#===============================================================================
.PHONY : 3559a_sample 3559a_sample_clean

3559a_sample:
	make -C $(HI3559A_SAMPLE_DIR)
3559a_sample_clean:
	make -C $(HI3559A_SAMPLE_DIR) clean
