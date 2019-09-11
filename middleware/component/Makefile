#===============================================================================
#                         export variable
#===============================================================================
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/..
include $(MIDDLEWARE_DIR)/base.mak
#===============================================================================
#                         rules
#===============================================================================
.PHONY : all clean install uninstall

exclude_components := fileformat netate

FILEFORMAT_DIR := fileformat
LITEPLAYER_DIR := liteplayer

ifeq ($(CFG_CHIP_TYPE),hi3559)
exclude_components += liteplayer
endif

ifeq ($(CFG_CHIP_TYPE),hi3556)
exclude_components += liteplayer
endif

ifeq ($(CFG_CHIP_TYPE),hi3559aes)
exclude_components += liteplayer
endif

components := $(shell find . -maxdepth 1 -type d)
components := $(basename $(patsubst ./%,%,$(components)))
components := $(filter-out $(exclude_components), $(components))

all:
	if [ -d $(FILEFORMAT_DIR) ]; then make -j32 -C $(FILEFORMAT_DIR)||exit 1;fi
	if [ -d $(LITEPLAYER_DIR) ]; then make -C $(LITEPLAYER_DIR)||exit 1;fi
	for comp in $(components);do make -j32 -C $$comp||exit 1;done

clean: $(components)
	if [ -d $(FILEFORMAT_DIR) ]; then make -j32 -C $(FILEFORMAT_DIR) clean;fi
	for comp in $(components);do make -j32 -C $$comp clean;done

install: $(components)
	if [ -d $(FILEFORMAT_DIR) ]; then make -j32 -C $(FILEFORMAT_DIR) install;fi
	if [ -d $(LITEPLAYER_DIR) ]; then make -C $(LITEPLAYER_DIR) install;fi
	for comp in $(components);do make -j32 -C $$comp install;done

uninstall: $(components)
	for comp in $(components);do make -j32 -C $$comp uninstall;done
