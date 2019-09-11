#===============================================================================
#                         export variable
#================================================================================
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/..
include $(MIDDLEWARE_DIR)/base.mak

export HIARCH=$(CFG_CHIP_TYPE)


include build/config_$(HIARCH).mak

#===============================================================================
#                         rules
#===============================================================================


.PHONY: all install clean

linux:
	@cd code; make MAPI_CPU_TYPE=linux
	@cd sample/arch; make linux
	if [ -d "test/arch" ]; then \
        cd  test/arch;make dual || exit 1; cd -; \
    fi

linux_clean:
	@cd code; make clean MAPI_CPU_TYPE=linux ;cd -
	@cd sample/arch; make clean MAPI_CPU_TYPE=linux; cd -
	#@cd test/arch; make clean; cd -
	@rm -rf out

liteos:
	@cd code; make MAPI_CPU_TYPE=liteos
	@cd sample/arch; make liteos

liteos_clean:
	@cd code; make clean MAPI_CPU_TYPE=liteos ;cd -
	@cd sample/arch; make liteos_clean; cd -
	#@cd test/arch; make clean; cd -
	@rm -rf out

dual:
	@cd code; make MAPI_CPU_TYPE=dual
	@cd sample/arch; make dual

dual_clean:
	@cd code; make clean MAPI_CPU_TYPE=dual
	@cd sample/arch; make dual_clean


ifeq ($(CFG_OS_TYPE), linux_liteos)
all: dual

install: dual
	mkdir -p $(OUT_LIB_DIR)/
	mkdir -p $(OUT_HEADER_DIR)/
	mkdir -p $(OUT_LITEOS_LIB_DIR)/
	mkdir -p $(OUT_LITEOS_HEADER_DIR)/
	cp -rf out/$(HIARCH)/dual/host/include/*  $(OUT_HEADER_DIR)/
	cp -rf out/$(HIARCH)/dual/host/lib/*  $(OUT_LIB_DIR)/
	cp -rf out/$(HIARCH)/dual/slave/include/*  $(OUT_LITEOS_HEADER_DIR)/
	cp -rf out/$(HIARCH)/dual/slave/lib/*  $(OUT_LITEOS_LIB_DIR)

clean:dual_clean

else ifeq ($(CFG_OS_TYPE), liteos)
all: liteos
install: liteos
	mkdir -p $(OUT_LITEOS_LIB_DIR)/
	mkdir -p $(OUT_LITEOS_HEADER_DIR)/
	cp -rf out/$(HIARCH)/liteos/slave/include/*  $(OUT_LITEOS_HEADER_DIR)/
	cp -rf out/$(HIARCH)/liteos/slave/lib/*  $(OUT_LITEOS_LIB_DIR)

clean:liteos_clean	

else ifeq ($(CFG_OS_TYPE), linux)
all: linux
install: linux
	mkdir -p $(OUT_LIB_DIR)/
	mkdir -p $(OUT_HEADER_DIR)/
	cp -rf out/$(HIARCH)/linux/host/include/*  $(OUT_HEADER_DIR)/
	cp -rf out/$(HIARCH)/linux/host/lib/*  $(OUT_LIB_DIR)/

clean:linux_clean
	
endif

