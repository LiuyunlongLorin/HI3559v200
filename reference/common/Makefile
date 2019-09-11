CUR_ROOT    := $(shell pwd)
REF_ROOT    ?= $(CUR_ROOT)/..
include $(REF_ROOT)/build/base.mak

.PHONY : all clean

exclude_components := sample include hiconfaccess scene media
ifneq ($(CONFIG_VIDEO_DETECT),y)
exclude_components += videodetect
endif
ifneq ($(CONFIG_RAWDEBUG), y)
exclude_components += rawcap
endif

components := $(shell find . -maxdepth 1 -type d)
components := $(basename $(patsubst ./%,%,$(components)))
components := $(filter-out $(exclude_components), $(components))

all:
	for comp in $(components);do make -C $$comp||exit;done

clean: $(components)
	for comp in $(components);do make -C $$comp clean||exit;done
