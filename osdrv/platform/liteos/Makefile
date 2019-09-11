#################################################################
#
# bugs:
#      1. 需要显式指定库位置、名称；
#      2. make 及 make clean处理得不好(对于库，要么删除再编译，要么无操作)；
##################################################################
LITEOSTOPDIR := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
export OS=$(shell uname -s)
ifneq ($(OS), Linux)
LITEOSTOPDIR := $(shell dirname $(subst \,/,$(LITEOSTOPDIR))/./)
endif

export LITEOSTOPDIR

-include $(LITEOSTOPDIR)/config.mk

RM = rm -rf
MAKE = make
__LIBS = libs

LITEOS_TARGET = vs_server
LITEOS_LIBS_TARGET = libs_target
LITEOS_MENUCONFIG_H = platform/bsp/common/menuconfig.h

#all: $(OUT) $(BUILD) $(LITEOS_TARGET)
all: $(OUT) $(BUILD) $(LITEOS_LIBS_TARGET)

##### make wow && make scatter && make wow_scatter #####
-include $(LITEOS_MK_PATH)/wow_scatter.mk

##### make dynload #####
-include $(LITEOS_MK_PATH)/dynload.mk

#-----need move when make version-----#
##### make lib #####
$(__LIBS): $(OUT) $(CXX_INCLUDE)

$(OUT): $(LITEOS_MENUCONFIG_H)
	$(HIDE)mkdir -p $(OUT)/lib
	$(HIDE)$(CC) -E $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/board.ld.S -o $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/board.ld -P
	$(HIDE)$(CC) -E -I $(LITEOSTOPDIR)/platform/bsp/common -I $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/include $(LITEOSTOPDIR)/tools/build/system_config.ld.S -o $(LITEOSTOPDIR)/tools/build/system_config.ld -P

$(BUILD):
	$(HIDE)mkdir -p $(BUILD)

$(LITEOS_LIBS_TARGET): $(__LIBS)
	$(HIDE)for dir in $(LITEOS_SUBDIRS); \
		do $(MAKE) -C $$dir all || exit 1; \
	done
	$(HIDE)echo "=============== make lib done  ==============="

##### make menuconfig #####
menuconfig:
	make -C tools/menuconfig menuconfig
savemenuconfig:
	make -C tools/menuconfig savemenuconfig
##### make #####


$(LITEOS_MENUCONFIG_H):
ifneq ($(LITEOS_MENUCONFIG_H), $(wildcard $(LITEOS_MENUCONFIG_H)))
	$(HIDE)+make savemenuconfig
endif
$(LITEOS_TARGET): $(__LIBS)
	$(HIDE)touch $(LOSCFG_ENTRY_SRC)

ifneq ($(LOSCFG_TEST), y)
ifeq ($(LOSCFG_KERNEL_RUNSTOP)$(LOSCFG_KERNEL_SCATTER), yy)
	$(HIDE)+make wow_scatter
else ifeq ($(LOSCFG_KERNEL_RUNSTOP), y)
	$(HIDE)+make wow
else ifeq ($(LOSCFG_KERNEL_SCATTER), y)
	$(HIDE)+make scatter
endif
endif

	$(HIDE)for dir in $(LITEOS_SUBDIRS); \
	do $(MAKE) -C $$dir all || exit 1; \
	done

ifneq ($(LOSCFG_TEST), y)
ifeq ($(LOSCFG_KERNEL_RUNSTOP)$(LOSCFG_KERNEL_SCATTER), yy)
	$(HIDE)$(SCRIPTS_PATH)/scatter_sr/scatter.sh $(CROSS_COMPILE) y y $(SCRIPTS_PATH)/scatter_sr $(SCRIPTS_PATH)/ld $(LITEOSTOPDIR)/liteos_tmp $(LIB_BIGODIR)
else ifeq ($(LOSCFG_KERNEL_RUNSTOP), y)
	$(HIDE)$(SCRIPTS_PATH)/scatter_sr/scatter.sh $(CROSS_COMPILE) y n $(SCRIPTS_PATH)/scatter_sr $(SCRIPTS_PATH)/ld $(LITEOSTOPDIR)/liteos_tmp $(LIB_BIGODIR)
else ifeq ($(LOSCFG_KERNEL_SCATTER), y)
	$(HIDE)$(SCRIPTS_PATH)/scatter_sr/scatter.sh $(CROSS_COMPILE) n y $(SCRIPTS_PATH)/scatter_sr $(SCRIPTS_PATH)/ld $(LITEOSTOPDIR)/liteos_tmp $(LIB_BIGODIR)
else
	$(HIDE)$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld
endif
else
	$(HIDE)$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld
endif

	$(LD) $(LITEOS_LDFLAGS) $(LITEOS_TABLES_LDFLAGS) $(LITEOS_DYNLDFLAGS) -Map=$(OUT)/$@.map -o $(OUT)/$@ --start-group $(LITEOS_LIBDEP) --end-group

	$(OBJCOPY) -O binary $(OUT)/$@ $(OUT)/$@.bin
	$(OBJDUMP) -d $(OUT)/$@ >$(OUT)/$@.asm
ifeq ($(LOSCFG_KERNEL_SCATTER), y)
	$(HIDE)$(SCRIPTS_PATH)/scatter_sr/scatter_size.sh $(OUT)/$(LITEOS_TARGET)
endif

sample_clean:
	$(HIDE)cd sample/sample_osdrv;make clean;cd ../..;
sample:all
	$(HIDE)cd sample/sample_osdrv;make;cd ../..;

clean:
	$(HIDE)for dir in $(LITEOS_SUBDIRS); \
		do make -C $$dir clean|| exit 1; \
	done
	$(HIDE)make -C tools/menuconfig distclean
	$(HIDE)make wow_scatter_clean
	$(HIDE)$(RM) $(__OBJS) $(LITEOS_TARGET) $(OUT) $(BUILD) $(LITEOS_MENUCONFIG_H) *.bak *~
	$(HIDE)-$(RM) $(LITEOSTOPDIR)/tools/build/system_config.ld
	$(HIDE)$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld
	$(HIDE)cd sample/sample_osdrv;make clean;cd ../..;
	$(HIDE)echo "clean $(LITEOS_PLATFORM) finish"

cleanall:clean
	$(HIDE)rm -rf $(LITEOSTOPDIR)/out
	$(HIDE)find platform/bsp/board/ -name board.ld -exec rm -rf {} \;
	$(HIDE)cd sample/sample_osdrv;make clean;cd ../..;
	$(HIDE)echo "clean all"
	$(HIDE)make wow_scatter_clean

.PHONY: all lib clean cleanall $(LITEOS_TARGET) sample sample_clean
