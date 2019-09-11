LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../../..
include $(MIDDLEWARE_DIR)/base.mak

COMPILE_ROOT := $(LOCAL_DIR)

MODNAME :=
MODNAME +=  $(shell test -d demux && echo demux)
MODNAME +=  $(shell test -d avplay && echo avplay)
MODNAME +=  $(shell test -d player && echo player)
SUBDIRS =$(MODNAME)


all: subdirs

.PHONY: subdirs $(SUBDIRS) clean
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) $(MFLAGS) -C $(COMPILE_ROOT)/$@

clean:
	@for MOD in $(MODNAME);do\
		cd $(COMPILE_ROOT)/$$MOD ;$(MAKE) $(MFLAGS) clean;done


install:
	@for MOD in $(MODNAME);do\
                cd $(COMPILE_ROOT)/$$MOD ;$(MAKE) $(MFLAGS) install || exit 1;done
