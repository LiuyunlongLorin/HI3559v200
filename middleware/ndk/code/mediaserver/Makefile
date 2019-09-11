LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../..
include $(NDK_PATH)/code/mediaserver/OSTYPE.param
include $(NDK_PATH)/Makefile.param

.PHONY:all  comm mapi vcap vcap_adpt vproc venc disp acap aenc ao msg base adapt config log clean

all:  comm mapi vcap vcap_adpt vproc venc disp acap aenc ao msg base adapt config log


base:
ifneq ($(MAPI_CPU_TYPE), linux)
	  @cd base/$(HIARCH); make
endif

comm:
	@cd comm; make

vcap:
	@cd vcap; make

vcap_adpt:
	@cd arch/$(HIARCH)/vcap;make
vproc:
	@cd vproc; make

venc:
	@cd venc; make

aenc:
	@cd aenc; make

ao:
	@cd ao; make

disp:
	@cd disp; make

acap:
	@cd acap; make

config:
	@cd configs; make

msg:
ifeq ($(MAPI_CPU_TYPE), dual)
	@cd msg; make
endif

log:
	@cd log; make

MAPI_INCS := $(wildcard $(NDK_PATH)/code/include/*.h)
MAPI_INCS := $(notdir $(MAPI_INCS))


mapi:
	@mkdir -p $(REL_INC)
	@cd ../include; cp $(MAPI_INCS) $(REL_INC);cd -
	@cp  $(NDK_PATH)/code/mediaserver/comm/arch/$(HIARCH)/*.h  $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/acap/arch/$(HIARCH)/*.h  $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/ao/arch/$(HIARCH)/*.h  $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/disp/arch/$(HIARCH)/*.h  $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/venc/arch/$(HIARCH)/*.h  $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/configs/sensor/$(HIARCH)/include/*.h  $(REL_INC)
clean:
ifneq ($(MAPI_CPU_TYPE), linux)
	@cd base/$(HIARCH); make clean
endif
	@cd comm; make clean
	@cd vcap; make clean
	@cd vproc; make clean
	@cd venc; make clean
	@cd aenc; make clean
	@cd ao; make clean
	@cd acap; make clean
	@cd disp; make clean
	@cd configs; make clean
ifeq ($(MAPI_CPU_TYPE), dual)
	@cd msg; make clean
endif
	@cd log; make clean
#	@cd pm; make clean
	@cd arch/$(HIARCH)/vcap;make clean
