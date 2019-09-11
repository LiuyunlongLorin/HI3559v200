#*************************************************************************
ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif
#*************************************************************************
.PHONY: all clean
all:
	@cd $(PWD)/mpi;make
	@cd $(PWD)/mkp;make

clean:
	@cd $(PWD)/mpi;make clean
	@cd $(PWD)/mkp;make clean

