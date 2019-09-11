
ifeq ($(EXTDRV_PARAM_FILE), ) 
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

SRCS := sample_ist.c

INC := -I$(REL_INC)
INC += -I$(OSAL_ROOT)/include
INC += -I$(ISP_ROOT)/ext_inc
INC += -I$(ISP_ROOT)/kernel/mkp/include
INC += -I$(ISP_ROOT)/kernel/arch/$(HIARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/arch/$(HIARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/include
EXTDRV_CFLAGS += $(INC)
EXTDRV_CFLAGS += $(COMM_INC)


		
#*************************************************************************
TARGET := hi_sample_ist
PROG := ist_test

#*************************************************************************
# compile linux or HuaweiLite
SUB_BUILD=$(PROG)
SUB_CLEAN=app_clean
include $(PWD)/../Make.$(OSTYPE)

$(PROG):%:%.o 
	$(CC) -g -Wall -o ist_test ist_test.c	strfunc.c
	@rm *.o

app_clean:
	@rm -f ist_test *.bak

