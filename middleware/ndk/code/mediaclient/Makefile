LOCAL_DIR := $(shell pwd)
NDK_PATH := $(LOCAL_DIR)/../..
MAPI_PARAM_FILE := $(NDK_PATH)/Makefile.param
include $(NDK_PATH)/code/mediaclient/OSTYPE.param
include $(NDK_PATH)/Makefile.param

.PHONY:all comm vcap vproc venc disp acap aenc ao msg mapi log clean

all: comm vcap vproc venc disp acap aenc ao msg mapi log

comm:
	@cd comm; make

vcap:
	@cd vcap; make

vproc:
	@cd vproc; make

venc:
	@cd venc; make

disp:
	@cd disp; make

acap:
	@cd acap; make

aenc:
	@cd aenc; make

ao:
	@cd ao; make


msg:
	@cd msg; make

log:
	@cd log; make

MAPI_INCS := $(wildcard $(NDK_PATH)/code/include/*.h)
MAPI_INCS := $(notdir $(MAPI_INCS))
mapi:
	@mkdir -p $(REL_INC)
	@cd ../include; cp $(MAPI_INCS) $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/comm/arch/$(HIARCH)/*.h  $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/acap/arch/$(HIARCH)/*.h  $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/ao/arch/$(HIARCH)/*.h  $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/disp/arch/$(HIARCH)/*.h  $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/venc/arch/$(HIARCH)/*.h  $(REL_INC)
	@cp  $(NDK_PATH)/code/mediaserver/configs/sensor/$(HIARCH)/include/*.h  $(REL_INC)
clean:
	@cd comm; make clean
	@cd vcap; make clean
	@cd vproc; make clean
	@cd venc; make clean
	@cd aenc; make clean
	@cd ao; make clean
	@cd disp; make clean
	@cd acap; make clean
	@cd msg; make clean
	@cd log; make clean
