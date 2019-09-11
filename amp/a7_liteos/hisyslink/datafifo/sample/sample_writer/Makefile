#OSTYPE should be liteos, CPU_TYPE should be single
OSTYPE=liteos

ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(HIARCH),$(filter $(HIARCH), hi3559av100))
    CPU_TYPE=single
endif

CFLAGS += -I../../include \
          -I$(REL_INC)

SRCS := $(wildcard *.c)
TARGET := $(SRCS:%.c=%)


DATAFIFO_LIB := ../../out/$(CPU_TYPE)_$(OSTYPE)/libdatafifo_$(CPU_TYPE)_$(OSTYPE).a

EXT_LIBA := -L$(REL_LIB) -lmpi -lsecurec

# compile rule
include $(PWD)/$(OSTYPE).mak
