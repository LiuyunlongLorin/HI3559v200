LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak

COMPILE_ROOT := $(LOCAL_DIR)

ifeq ($(CFG_CHIP_TYPE),hi3559)
MODNAME :=
else ifeq ($(CFG_CHIP_TYPE),hi3556)
MODNAME :=
else
MODNAME := player_sample demux_sample
endif

SUBDIRS =$(MODNAME)

all: subdirs

.PHONY: subdirs $(SUBDIRS) clean
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) $(MFLAGS) -C $(COMPILE_ROOT)/$@

clean:
	@for MOD in $(MODNAME);do\
		cd $(COMPILE_ROOT)/$$MOD ;$(MAKE) $(MFLAGS) clean;done

install: subdirs
	if [ ! -d $(OUT_LITEOS_LIB_DIR) ]; then mkdir -p $(OUT_LITEOS_LIB_DIR);fi
	@cp player_sample/AAC/*.so $(OUT_LITEOS_LIB_DIR)/
