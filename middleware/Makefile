#===============================================================================
#                         export variable
#===============================================================================
MIDDLEWARE_DIR:= $(shell pwd)

include $(MIDDLEWARE_DIR)/base.mak

export &(SDK_ROOT)
#====================================================================================
#                          global targets params
#====================================================================================
ALL_OBJS :=
ifeq ($(CFG_ENABLE_NDK),y)
ALL_OBJS += ndk
endif
ALL_OBJS += media_adpt
ALL_OBJS += common
ALL_OBJS += thirdparty
ALL_OBJS += component
ALL_OBJS += sample

CLEAN_OBJS :=
CLEAN_OBJS += sample_clean
CLEAN_OBJS += component_clean
CLEAN_OBJS += common_clean
CLEAN_OBJS += thirdparty_clean
CLEAN_OBJS += media_adpt_clean
ifeq ($(CFG_ENABLE_NDK),y)
CLEAN_OBJS += ndk_clean
endif

INSTALL_OBJS :=
ifeq ($(CFG_ENABLE_NDK),y)
INSTALL_OBJS += ndk_install
endif
INSTALL_OBJS += media_adpt_install
INSTALL_OBJS += common_install
INSTALL_OBJS += thirdparty_install
INSTALL_OBJS += component_install
INSTALL_OBJS += sample_install

UNINSTALL_OBJS :=
UNINSTALL_OBJS += sample_uninstall
UNINSTALL_OBJS += component_uninstall
UNINSTALL_OBJS += common_uninstall
UNINSTALL_OBJS += thirdparty_uninstall
UNINSTALL_OBJS += media_adpt_uninstall
ifeq ($(CFG_ENABLE_NDK),y)
UNINSTALL_OBJS += ndk_uninstall
endif
#====================================================================================
#                          global targets, $(XXX_OBJS) config in base.mak
#====================================================================================
.PHONY: all clean install uninstall prepare

all: $(ALL_OBJS)
clean: $(CLEAN_OBJS)
	$(AT)rm -rf $(OUT_DIR)
install: prepare $(INSTALL_OBJS)
prepare:
	mkdir -p $(OUT_DIR)
	mkdir -p $(OUT_HEADER_DIR)
	mkdir -p $(OUT_LIB_DIR)
uninstall: clean
#====================================================================================
#                          media_adapt
#====================================================================================
.PHONY:  media_adpt  media_adpt_clean  media_adpt_install  media_adpt_uninstall

media_adpt:
	make -C $(MEDIA_ADPT_DIR) prepare || exit 1
	make -C $(MEDIA_ADPT_DIR) adpt_compile || exit 1
media_adpt_clean:
	make -C $(MEDIA_ADPT_DIR) adpt_clean || exit 1
media_adpt_install:
	make -C $(MEDIA_ADPT_DIR) install || exit 1
media_adpt_uninstall:
	make -C $(MEDIA_ADPT_DIR) uninstall || exit 1
#====================================================================================
#                          component
#====================================================================================
.PHONY:  component  component_clean  component_install  component_uninstall

component:
	make -C $(COMPONENT_DIR) || exit 1
component_clean:
	make -C $(COMPONENT_DIR) clean || exit 1
component_install:
	make -C $(COMPONENT_DIR) install || exit 1
component_uninstall:
	make -C $(COMPONENT_DIR) uninstall || exit 1
#====================================================================================
#                          common
#====================================================================================
.PHONY:  common  common_clean  common_install  common_uninstall

common:
	make -C $(COMMON_DIR) || exit 1
common_clean:
	make -C $(COMMON_DIR) clean || exit 1
common_install:
	make -C $(COMMON_DIR) install || exit 1
common_uninstall:
	make -C $(COMMON_DIR) uninstall || exit 1
#====================================================================================
#                          sample
#====================================================================================
.PHONY:  sample  sample_clean  sample_install  sample_uninstall

sample:
	make -C $(SAMPLE_DIR) || exit 1
sample_clean:
	make -C $(SAMPLE_DIR) clean || exit 1
sample_install:
	make -C $(SAMPLE_DIR) install || exit 1
sample_uninstall:
	make -C $(SAMPLE_DIR) uninstall || exit 1
#====================================================================================
#                          thirdparty
#====================================================================================
.PHONY:  thirdparty  thirdparty_clean  thirdparty_install  thirdparty_uninstall

thirdparty:
	make -C $(THIRDPARTY_DIR) || exit 1
thirdparty_clean:
	make -C $(THIRDPARTY_DIR) clean || exit 1
thirdparty_install:
	make -C $(THIRDPARTY_DIR) install || exit 1
thirdparty_uninstall:
	make -C $(THIRDPARTY_DIR) uninstall || exit 1
#====================================================================================
#                          ndk
#====================================================================================
.PHONY:  ndk  ndk_clean  ndk_install  ndk_uninstall

ndk:
	if [ -d $(NDK_DIR) ]; then \
	cd  $(NDK_DIR);make all || exit 1; cd -; \
	fi
ndk_clean:
	if [ -d $(NDK_DIR) ]; then \
	cd  $(NDK_DIR); make clean; cd -; \
	fi
ndk_install:
	if [ -d $(NDK_DIR) ]; then \
	cd  $(NDK_DIR);make install || exit 1; cd -; \
	fi
ndk_uninstall:
	if [ -d $(NDK_DIR) ]; then \
	cd  $(NDK_DIR); make clean; cd -; \
	fi
