
REF_ROOT := $(shell pwd)
include $(REF_ROOT)/build/base.mak

SUBDIRS := thirdparty common hal tools $(CFG_PDT_TYPE)
SUBDIRS_CLEAN := $(addsuffix _clean,$(SUBDIRS))

.PHONY: all clean distclean prepare install
.PHONY: $(SUBDIRS) $(SUBDIRS_CLEAN)
defalut: all

all: prepare $(SUBDIRS) install

clean: $(SUBDIRS_CLEAN)

distclean: clean
	@make distclean -C $(CFG_PDT_TYPE)

install:
	@echo -e "$(GREEN)--install '$(notdir $(CFG_PDT_TYPE))' finished$(NORMAL)"

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

