
CUR_ROOT    := $(shell pwd)
PDT_ROOT    ?= $(CUR_ROOT)/../..
SDK_ROOT    ?= $(PDT_ROOT)/../..
include $(SDK_ROOT)/build/base.mak

ifeq ($(AMP_TYPE),linux_liteos)

.PHONY: all clean client server
ifeq ($(AMP_TYPE), linux_liteos)
all: client server
else
all: server
endif

client:
	@make -C client||exit $?

clean:
	@make clean -C server||exit $?
	@make clean -C client||exit $?
	@-rm server/logo*.dat

else
.PHONY: all clean server
all: server

clean:
	@make clean -C server||exit $?
	@-rm server/logo*.dat
endif

server:
	@make -C server||exit $?


