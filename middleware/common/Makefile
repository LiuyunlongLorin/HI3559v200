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
HEADER_FILE = $(shell pwd)/include/hi_track_source.h
HEADER_FILE += $(shell pwd)/include/hi_error_def.h
HEADER_FILE += $(shell pwd)/include/hi_defs.h
HEADER_FILE += $(shell pwd)/include/hi_mw_type.h
exclude_compoents := include

components := $(shell find . -maxdepth 1 -type d)
components := $(basename $(patsubst ./%,%,$(components)))
components := $(filter-out $(exclude_compoents), $(components))

all:
	for comp in $(components);do make -j32 -C $$comp||exit 1;done

clean: $(components)
	for comp in $(components);do make -j32 -C $$comp clean;done

install: $(components)
	for comp in $(components);do make -j32 -C $$comp install;done
	cp -rf $(HEADER_FILE) $(OUT_HEADER_DIR)/

uninstall: $(components)
	for comp in $(components);do make -j32 -C $$comp uninstall;done
