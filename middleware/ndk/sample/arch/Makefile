NDK_PATH=$(shell pwd)/../..
include ../../build/chip.mak
include ../../build/config_$(HIARCH).mak

.PHONY: linux dual dual_clean liteos liteos_clean all venc vio audio stitch photo dump switchmode clean

linux:
	@cd $(HIARCH); make linux

linux_clean:
	@cd $(HIARCH); make linux_clean

dual:
	@cd $(HIARCH); make dual

dual_clean:
	@cd $(HIARCH); make dual_clean

liteos:
	@cd $(HIARCH); make liteos

liteos_clean:
	@cd $(HIARCH); make liteos_clean

all:
	@cd $(HIARCH); make all

venc:
	@cd $(HIARCH); make venc

vio:
	@cd $(HIARCH); make vio

audio:
	@cd $(HIARCH); make audio

stitch:
	@cd $(HIARCH); make stitch

photo:
	@cd $(HIARCH); make photo

dump:
	@cd $(HIARCH); make dump

switchmode:
	@cd $(HIARCH); make switchmode

clean:
	@cd $(HIARCH); make clean
