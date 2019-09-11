CUR_ROOT    := $(shell pwd)
PDT_ROOT    ?= $(CUR_ROOT)/../..
OS_TYPE     := Linux
include $(PDT_ROOT)/build/base.mak
SUBDIRS := $(CFG_DASHCAM_TYPE)

.PHONY: all $(SUBDIRS) clean
all:$(SUBDIRS)

$(SUBDIRS):
	@make -C $@||exit $?

clean:
	@for MOD in $(SUBDIRS);do\
		make clean -C $$MOD||exit $?;done
