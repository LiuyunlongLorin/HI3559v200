ifeq ($(MS_PARAM_FILE), )
    MS_PARAM_FILE:=../Makefile.param
    include $(MS_PARAM_FILE)
endif

CFLAGS += -I$(NDK_PATH)/code/mediaserver/configs/pm/include
#*************************************************************************
# all source file in this module
SRCS := mapi_pm.c

#*************************************************************************
TARGET := $(HIARCH)_mapi_pm

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
