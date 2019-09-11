# $(ROOT)/mpp/Mafile

ifeq ($(PARAM_FILE), ) 
    PARAM_FILE:=../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

BASE_MKP_HEADER := mod_ext.h vb_ext.h
SYS_EXT_HEADER := sys_ext.h

ifeq ($(MPP_BUILD), y)
	INC_DIR=$(MPP_PATH)/../drv/extdrv/$(HIARCH)/motionsensor/motionsensor_mng/include
else
	INC_DIR=$(MPP_PATH)/../drv/extdrv/motionsensor/motionsensor_mng/include
endif

.PHONY: default clean


default:
ifeq ($(CONFIG_HI_MOTIONFUSION_SUPPORT), y)
ifeq ($(MPP_BUILD), y)
	@echo "MPPP_PATH=$(MPP_PATH)"
	@echo "INC_DIR==$(INC_DIR)"
	@-pushd $(INC_DIR) > /dev/null; rm -f $(BASE_MKP_HEADER); rm -f $(SYS_EXT_HEADER);  popd > /dev/null
	@-pushd $(MPP_PATH)/cbb/base/ext_inc > /dev/null;cp $(BASE_MKP_HEADER) $(INC_DIR); popd > /dev/null
	@-pushd $(MPP_PATH)/cbb/sys/ext_inc > /dev/null;cp $(SYS_EXT_HEADER) $(INC_DIR); popd > /dev/null
endif
	@echo -e "\e[0;32;1m--Compiling 'hisi_spi'...\e[0;36;1m"
	@@echo -e "\e[0m"
	@cd ./hisi_spi;make;cd ../;
	@echo -e "\e[0;32;1m--Compiling 'motionsensor_mng'...\e[0;36;1m"
	@@echo -e "\e[0m"
	@cd ./motionsensor_mng;make;cd ../;
	@echo -e "\e[0;32;1m--Compiling 'motionsensor_chip'...\e[0;36;1m"
	@@echo -e "\e[0m"
	@cd ./motionsensor_chip;make;cd ../;
endif

clean:
ifeq ($(CONFIG_HI_MOTIONFUSION_SUPPORT), y)
	@cd ./hisi_spi;make clean;cd ../;
	@cd ./motionsensor_mng;make clean;cd ../;
	@cd ./motionsensor_chip;make clean;cd ../;
endif
