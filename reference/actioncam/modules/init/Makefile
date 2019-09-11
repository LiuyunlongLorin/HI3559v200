
CUR_ROOT    := $(shell pwd)
SDK_ROOT    := $(CUR_ROOT)/../../../..
include $(SDK_ROOT)/build/base.mak

ifeq ($(AMP_TYPE), linux_liteos)

.PHONY: all clean amp
all: amp

amp:
	@make -C amp||exit $?

clean:
	@make clean -C amp||exit $?

else

.PHONY: all clean
all:

clean:

endif


