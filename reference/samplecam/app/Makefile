CUR_ROOT    := $(shell pwd)
PDT_ROOT    := $(CUR_ROOT)/..
include $(PDT_ROOT)/build/base.mak

SUBDIRS := vio
ifeq ($(CONFIG_SCREEN),y)
SUBDIRS += lcd
endif
ifeq ($(CONFIG_TOUCHPAD),y)
SUBDIRS += touchpad
endif

.PHONY: all $(SUBDIRS) clean

all:$(SUBDIRS)

$(SUBDIRS):
	@make -C $@||exit $?

clean:
	@for MOD in $(SUBDIRS);do\
		make clean -C $$MOD||exit $?;done
