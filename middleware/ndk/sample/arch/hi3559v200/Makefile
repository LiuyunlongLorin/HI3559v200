NDK_PATH=$(shell pwd)/../../..
include ../../../build/chip.mak
include ../../../build/config_$(HIARCH).mak

.PHONY:all venc vio audio stitch photo dump switchmode clean cleanstream


linux:
	@cd venc; make MAPI_CPU_TYPE=linux
	@cd vio; make MAPI_CPU_TYPE=linux
	@cd audio; make MAPI_CPU_TYPE=linux
	@cd dump; make MAPI_CPU_TYPE=linux

dual:export MAPI_CPU_TYPE=dual
dual:
	@cd HuaweiLite;make prepare; make all OSTYPE=liteos
	@cd venc; make OSTYPE=linux
	@cd vio; make OSTYPE=linux
	@cd audio; make OSTYPE=linux
	@cd dump; make OSTYPE=linux


dual_clean:MAPI_CPU_TYPE=dual
dual_clean:
	@cd HuaweiLite;make clean OSTYPE=liteos
	@cd venc; make clean OSTYPE=linux
	@cd vio; make clean OSTYPE=linux
	@cd audio; make clean OSTYPE=linux
	@cd dump; make clean OSTYPE=linux
	@cd switchmode; make clean OSTYPE=linux


liteos:export OSTYPE=liteos
liteos:export MAPI_CPU_TYPE=liteos
liteos:
	@cd venc;  make all
	@cd audio;make prepare; make all
	@cd dump; make all

liteos_clean:export OSTYPE=liteos
liteos_clean:export MAPI_CPU_TYPE=liteos
liteos_clean:
	@cd venc;  make clean
	@cd audio; make clean
	@cd dump; make clean

all: venc vio audio stitch photo dump switchmode


venc:
	@cd venc; make

vio:
	@cd vio; make

audio:
	@cd audio; make

dump:
	@cd photo;make

clean:
	@cd venc; make clean
	@cd vio; make clean
	@cd audio; make clean
	@cd dump;make clean
