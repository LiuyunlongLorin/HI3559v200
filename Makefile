##########################################################################################
#	help config
##########################################################################################
.PHONY: help
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
GREEN="\e[32;1m"
NORMAL="\e[39m"
RED="\e[31m"
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
help:
	@echo -e ${GREEN}
	@#cat script/Readme|less
	@echo -e ${NORMAL}
##########################################################################################
#	menuconfig config
##########################################################################################
.PHONY: menuconfig
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++
KCONFIG_PATH= build/scripts/kconfig/
KCONFIG_EXE = build/scripts/kconfig/mconf
KCONFIG_CFG = Kconfig
#++++++++++++++++++++++++++++++++++++++++++++++++++++
prepare_kconfig:
	@if [ ! -f $(KCONFIG_EXE) ];then cd $(KCONFIG_PATH);make;cd -;fi

menuconfig: prepare_kconfig
	@-chmod -Rf 777 $(KCONFIG_EXE)
	$(if $(wildcard $(KCONFIG_EXE)), \
		$(KCONFIG_EXE) $(KCONFIG_CFG), \
	)

##########################################################################################
#	compile config
##########################################################################################
export SDK_ROOT  := $(shell pwd)
include $(SDK_ROOT)/build/base.mak

.PHONY: all clean distclean prepare
.PHONY: osdrv osdrv_clean osdrv_distclean
.PHONY: ndk ndk_clean ndk_distclean
.PHONY: middleware middleware_clean middleware_distclean
.PHONY: reference reference_clean reference_distclean

SDK_SUBDIR := $(OSDRV_ROOT) $(AMP_ROOT) $(OSAL_ROOT) $(MPP_ROOT) $(DRV_ROOT)


all: prepare osdrv drv ndk middleware reference success

clean: reference_clean middleware_clean ndk_clean drv_clean osdrv_clean

distclean: reference_distclean middleware_distclean ndk_distclean drv_distclean osdrv_distclean

success:
	@echo -e "\e[0;32;1m--install '$(notdir $(SDK_ROOT))' finished\e[0;36;1m"
	@echo -e "\e[0m"

sdk_prepare:
	for comp in $(SDK_SUBDIR);do \
		if [ -f $$comp/prepare.sh ];then cd $$comp;chmod +x ./prepare.sh;./prepare.sh $(CFG_CHIP_TYPE) $(LIBC_TYPE);cd -;fi;\
	done

sdk_distclean:
	for comp in $(SDK_SUBDIR);do \
		if [ -f $$comp/prepare.sh ];then cd $$comp;chmod +x ./prepare.sh;./prepare.sh clean;cd -;fi;\
	done

prepare: sdk_prepare middleware_prepare
	@chmod +x $(SDK_ROOT)/build/*
	@cd $(SDK_ROOT)/build && ./config_memory_$(CFG_CHIP_TYPE).sh .config;cd -

prepare_clean:
	@chmod +x $(SDK_ROOT)/build/*
	@cd $(SDK_ROOT)/build && ./config_memory_$(CFG_CHIP_TYPE).sh .config clean;cd -

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	OSDRV Compile
#++++++++++++++++++++++++++++++++++++++++++++++++++++
osdrv: prepare
	@chmod 777 $(OSDRV_ROOT)/* -R
	@make OSDRV_CROSS=$(OSDRV_CROSS) $(OSDRV_COMPILE_OPT) all -C $(OSDRV_ROOT)||exit $?

osdrv_clean:
	@make OSDRV_CROSS=$(OSDRV_CROSS) $(OSDRV_COMPILE_OPT) clean -C $(OSDRV_ROOT)||exit $?

osdrv_distclean:
	@make OSDRV_CROSS=$(OSDRV_CROSS) $(OSDRV_COMPILE_OPT) distclean -C $(OSDRV_ROOT)||exit $?

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	DRV Compile
#++++++++++++++++++++++++++++++++++++++++++++++++++++
drv: prepare
	@chmod 777 $(DRV_ROOT)/* -R
	@if [ -f $(DRV_EXTDRV_PATH_LINUX)/Makefile.board ];then make -C $(DRV_EXTDRV_PATH_LINUX)  -f Makefile.board OSTYPE=linux||exit $?;fi
	@if [ -f $(DRV_EXTDRV_PATH_LITEOS)/Makefile.board ];then make -C $(DRV_EXTDRV_PATH_LITEOS) -f Makefile.board OSTYPE=liteos||exit $?;fi
	@if [ -d $(OSAL_LINUX_PATH)/linux/kernel ];then cd $(OSAL_LINUX_PATH)/linux/kernel; make||exit $?; cd -;fi
	@if [ -d $(OSAL_LITEOS_PATH)/liteos/kernel ];then cd $(OSAL_LITEOS_PATH)/liteos/kernel; make||exit $?; cd -;fi

drv_clean:
	@if [ -f $(DRV_EXTDRV_PATH_LINUX)/Makefile.board ];then make clean -C $(DRV_EXTDRV_PATH_LINUX)  -f Makefile.board OSTYPE=linux||exit $?;fi
	@if [ -f $(DRV_EXTDRV_PATH_LITEOS)/Makefile.board ];then make clean -C $(DRV_EXTDRV_PATH_LITEOS) -f Makefile.board OSTYPE=liteos||exit $?;fi
	@if [ -d $(OSAL_LINUX_PATH)/linux/kernel ];then cd $(OSAL_LINUX_PATH)/linux/kernel; make clean||exit $?; cd -;fi
	@if [ -d $(OSAL_LITEOS_PATH)/liteos/kernel ];then cd $(OSAL_LITEOS_PATH)/liteos/kernel; make clean||exit $?; cd -;fi

drv_distclean: drv_clean

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	NDK Compile
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ndk: middleware_prepare
	@chmod 777 $(NDK_ROOT)/* -R
	@make install -C $(NDK_ROOT) $(NDK_COMPILE_OPT) ||exit $?

ndk_clean:
	@make clean -C $(NDK_ROOT) $(NDK_COMPILE_OPT)||exit $?

ndk_distclean:
	@make clean -C $(NDK_ROOT) $(NDK_COMPILE_OPT)||exit $?

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	MiddleWare Compile
#++++++++++++++++++++++++++++++++++++++++++++++++++++
middleware_prepare:
	@cd $(MWARE_ROOT); if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh $(CFG_CHIP_TYPE) $(LIBC_TYPE);fi;cd -

middleware: middleware_prepare
	@make install -C $(MWARE_ROOT) $(MW_COMPILE_OPT) ||exit $?

middleware_clean:
	@make uninstall -C $(MWARE_ROOT)||exit $?

middleware_distclean:
	@make uninstall -C $(MWARE_ROOT)||exit $?

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Reference Compile
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_PQT_SUPPORT_ON), y)
pq_prepare:
	@cd $(PQT_ROOT); if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh $(CFG_CHIP_TYPE) $(LIBC_TYPE);fi;tar -zxf Hi35*_PQ_V*.tgz;cd -
endif

ifeq ($(CONFIG_PQT_SUPPORT_ON), y)
reference: prepare pq_prepare
else
reference: prepare
endif
	@make -C $(REF_ROOT)||exit $?

reference_clean:
	@make clean -C $(REF_ROOT)||exit $?

reference_distclean:
	@make distclean -C $(REF_ROOT)||exit $?
