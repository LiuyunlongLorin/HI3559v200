CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/..
include $(REF_ROOT)/build/base.mak

.PHONY : all clean

components := thttpd iniparser

ifneq ($(AMP_TYPE), liteos)
ifeq ($(CONFIG_WIFI), y)
exist = $(shell if [ -d "hostapd" ]; then echo "exist"; else echo "notexist"; fi;)
ifeq ($(exist), exist)
components += libnl hostapd
endif
endif
endif

exclude_components :=

components := $(filter-out $(exclude_components), $(components))

all:
	for comp in $(components);do make -C $$comp||exit;done

clean: $(components)
	for comp in $(components);do make -C $$comp clean;done

