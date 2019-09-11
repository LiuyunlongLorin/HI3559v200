#===============================================================================
#                         export variable
#===============================================================================
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/..
include $(MIDDLEWARE_DIR)/base.mak

#===============================================================================
#                         local variable
#===============================================================================
CUNIT_DIR := c-unit


#===============================================================================
#                         rules
#===============================================================================
.PHONY : all clean install uninstall
#tiff compile rely on jpeg
exclude_softwares := 
exclude_softwares += c-unit
exclude_softwares += tinyxml

softwares := $(shell find . -maxdepth 1 -type d)
softwares := $(basename $(patsubst ./%,%,$(softwares)))
softwares := $(filter-out $(exclude_softwares), $(softwares))

all:
ifneq ($(CFG_OS_TYPE),liteos)
	if [ -d $(CUNIT_DIR) ]; then make -C $(CUNIT_DIR)||exit 1;fi
endif
	for soft in $(softwares);do make -j32 -C $$soft||exit 1;done

clean:
	for soft in $(softwares);do make -j32 -C $$soft clean;done

install:
ifneq ($(CFG_OS_TYPE),liteos)
	if [ -d $(CUNIT_DIR) ]; then make -C $(CUNIT_DIR) install||exit 1;fi
endif
	for soft in $(softwares);do make -j32 -C $$soft install||exit 1;done

uninstall:
	for soft in $(softwares);do make -j32 -C $$soft uninstall;done
