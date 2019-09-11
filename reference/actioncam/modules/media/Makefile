
CUR_ROOT    := $(shell pwd)
PDT_ROOT    ?= $(CUR_ROOT)/../..
SDK_ROOT    ?= $(PDT_ROOT)/../..
include $(SDK_ROOT)/build/base.mak

ifeq ($(AMP_TYPE),linux_liteos)

.PHONY: all clean client server
all: client server

client:
	@make -C client||exit $?

clean:
	@make clean -C server||exit $?
	@make clean -C client||exit $?

else
.PHONY: all clean server
all: server

clean:
	@make clean -C server||exit $?
endif

server:
	@make -C server||exit $?


