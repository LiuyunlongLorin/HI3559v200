# Create by y00260229,2016/12/29
#*************************************************************************
ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif


ISP_INC := ./include
ISP_ARCH_INC := ./include/arch/$(HIARCH)/include
3A_INC := ./user/3a/include

.PHONY: clean all show kernel user extdrv

all: show kernel user extdrv

show:
	@echo -e "\e[0;32;1m--Compiling 'isp'... Configs as follow:\e[0;36;1m"
	@echo ---- CROSS=$(CROSS)
	@echo ---- HIARCH=$(HIARCH), HICHIP=$(HICHIP), CVER=$(CVER), HIDBG=$(HIDBG)
	@echo ---- SDK_PATH=$(SDK_PATH) , PARAM_FILE=$(PARAM_FILE)
	@echo ---- KERNEL_ROOT=$(KERNEL_ROOT)
	@echo -e "\e[0m"
	@mkdir -p $(REL_INC); cp $(ISP_INC)/*.h $(ISP_ARCH_INC)/*.h $(3A_INC)/*.h $(REL_INC) -rf ;
kernel:
	@cd kernel; make

user:
	@cd user; make

extdrv:
	@cd ext_drv/$(HIARCH); make

clean:
	@cd kernel; make clean
	@cd user; make clean
	@cd ext_drv/$(HIARCH); make clean
