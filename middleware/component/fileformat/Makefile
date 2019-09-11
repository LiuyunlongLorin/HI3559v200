#===============================================================================
#                         export variable
#================================================================================
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak
#===============================================================================
#                         rules
#===============================================================================
.PHONY : all clean install uninstall

exclude_compoents :=

components := $(shell find . -maxdepth 1 -type d)
components := $(basename $(patsubst ./%,%,$(components)))
components := $(filter-out $(exclude_compoents), $(components))

all:
	for comp in $(components);do make -C $$comp || exit 1 ;done

clean: $(components)
	for comp in $(components);do make -C $$comp clean || exit 1;done

install: $(components)
	for comp in $(components);do make -C $$comp install || exit 1;done

uninstall: $(components)
	for comp in $(components);do make -C $$comp uninstall || exit 1;done
