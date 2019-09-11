CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/../..
include $(REF_ROOT)/build/base.mak

.PHONY : all clean

exclude_components :=include
ifeq ($(AMP_TYPE), liteos)
exclude_components +=msg
endif

components := $(shell find . -maxdepth 1 -type d)
components := $(basename $(patsubst ./%,%,$(components)))
components := $(filter-out $(exclude_components), $(components))

all:
	for comp in $(components);do make -C $$comp||exit;done

clean: $(components)
	for comp in $(components);do make -C $$comp clean;done

