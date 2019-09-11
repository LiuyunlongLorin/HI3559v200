
PDT_ROOT := $(shell pwd)
include $(PDT_ROOT)/build/base.mak

SUBDIRS := modules app rootfs
SUBDIRS_CLEAN := $(addsuffix _clean,$(SUBDIRS))

.PHONY: all $(SUBDIRS) clean distclean
all:$(SUBDIRS)

clean: $(SUBDIRS_CLEAN)

distclean: clean
	@make distclean -C rootfs
	@rm -rf $(PDT_OUT_ROOT)

$(SUBDIRS):
	@echo -e "\e[0;32;1m--Compiling '$@'...\e[0;36;1m"
	@echo -e "\e[0m"
	@make -C $@||exit $?
	@echo -e "\e[0;32;1m--Compile '$@' finished\e[0;36;1m"
	@echo -e "\e[0m"

$(SUBDIRS_CLEAN):
	@echo -e "\e[0;32;1m--execute '$@'...\e[0;36;1m"
	@echo -e "\e[0m"
	@cd $(patsubst %_clean,%,$@);make clean||exit $?;cd -
