COMPILE_ROOT := $(shell pwd)
MIDDLEWARE_DIR := $(COMPILE_ROOT)/../../..
include $(MIDDLEWARE_DIR)/cfg.mak
ifeq ($(CFG_OS_TYPE),liteos)
export OSTYPE :=HuaweiLite
else
export OSTYPE :=Linux
endif
include $(COMPILE_ROOT)/Make.$(OSTYPE)