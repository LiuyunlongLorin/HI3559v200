###############################################################################
# export variables
###############################################################################
CUR_ROOT    := $(shell pwd)
PDT_ROOT    ?= $(CUR_ROOT)/../../..
OS_TYPE     := Linux
include $(PDT_ROOT)/build/base.mak

MODULE_ROOT := $(CUR_ROOT)
MODULE_NAME := $(CUR_DIR_NAME)

SRC_ROOT    := $(MODULE_ROOT)
SRC_DIR     := $(SRC_ROOT)/src

OBJ_ROOT    := $(PDT_OUT_MODULES_OBJ)/$(MODULE_NAME)/$(OS_TYPE)
LIB_ROOT    := $(PDT_OUT_MODULES_LIB)/$(OS_TYPE)
LIB_TYPE    := static
########################################################################

# Specify Source/Object/Target Path
HI_XML_EXTC_ROOT := $(SRC_ROOT)/bin
HI_HIGV_BIN_ROOT := $(SRC_ROOT)/higv
XML_PATH = $(SRC_ROOT)/xml


CreateResult        :=
dummy :=  $(call CreateDir, $(HI_XML_EXTC_ROOT))
dummy +=  $(call CreateDir, $(HI_HIGV_BIN_ROOT))
ifneq ($(strip CreateResult),)
    err = $(error $(CreateResult))
endif

ifeq ($(SVR_LONGBIT),64)
    XML2Bin = $(MWARE_COMPONENT_PATH)/higv/higv/tools/64bit_xml2bin/xml2bin
else
    XML2Bin = $(MWARE_COMPONENT_PATH)/higv/higv/tools/32bit_xml2bin/xml2bin
endif


.PHONY:  xml

xml:
	chmod +x $(XML2Bin)
	$(XML2Bin) -x $(XML_PATH) -b $(HI_HIGV_BIN_ROOT)/higv.bin -t c -e $(HI_XML_EXTC_ROOT)/ -l $(HI_XML_EXTC_ROOT)
	chmod 777 $(HI_HIGV_BIN_ROOT)/higv.bin
	cp -u $(HI_XML_EXTC_ROOT)/higv_cextfile.c $(SRC_DIR)/
	cp -u $(HI_XML_EXTC_ROOT)/higv_cextfile.h $(SRC_ROOT)/include/
	cp -u $(HI_XML_EXTC_ROOT)/higv_language.h $(SRC_ROOT)/include/
	rm $(HI_XML_EXTC_ROOT)/*.c
	rm $(HI_XML_EXTC_ROOT)/*.h

	make clean; make all;
################################################################
include $(REF_ROOT)/build/modbuild.mak

