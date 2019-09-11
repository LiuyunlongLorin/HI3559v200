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
PIC_BASE_PATH = $(SRC_ROOT)/res/pic
XML_PATH = $(SRC_ROOT)/xml


CreateResult        :=
dummy :=  $(call CreateDir, $(HI_XML_EXTC_ROOT))
dummy +=  $(call CreateDir, $(HI_HIGV_BIN_ROOT))
ifneq ($(strip CreateResult),)
    err = $(error $(CreateResult))
endif

pic_file_cnt=$(shell ls $(PIC_PATH)/* 2> /dev/null | wc -l)

ifeq ($(CONFIG_LCD_UISRC_ADAPT_AUTO), y)
XML_PATH_DEST = $(SRC_ROOT)/xmldest
PIC_DEST_PATH = $(SRC_ROOT)/res/picdest
dummy +=  $(call CreateDir, $(XML_PATH_DEST))
dummy +=  $(call CreateDir, $(PIC_DEST_PATH))
UI_RESOURCE_RESOLUTION_CHANGE := \
$(shell awk 'BEGIN{printf("%04d%04d%04d%04d",'$(CONFIG_LCD_UISRC_BASE_WIDTH)','$(CONFIG_LCD_UISRC_BASE_HEIGHT)','$(CONFIG_LCD_UISRC_DEST_WIDTH)','$(CONFIG_LCD_UISRC_DEST_HEIGHT)')}')
picdest_file_cnt=$(shell ls $(PIC_DEST_PATH)/* 2> /dev/null | wc -l)
endif

ifeq ($(SVR_LONGBIT),64)
    XML2Bin = $(MWARE_COMPONENT_PATH)/higv/higv/tools/64bit_xml2bin/xml2bin
else
    XML2Bin = $(MWARE_COMPONENT_PATH)/higv/higv/tools/32bit_xml2bin/xml2bin
endif


.PHONY:  xml

xml: cpxml fontsizeadapt picstretch

	chmod +x $(XML2Bin)

ifeq ($(CONFIG_LCD_UISRC_ADAPT_AUTO), y)
	$(XML2Bin) -x $(XML_PATH_DEST) -C $(UI_RESOURCE_RESOLUTION_CHANGE) -b $(HI_HIGV_BIN_ROOT)/higv.bin -t c -e $(HI_XML_EXTC_ROOT)/ -l $(HI_XML_EXTC_ROOT)
else
	$(XML2Bin) -x $(XML_PATH) -b $(HI_HIGV_BIN_ROOT)/higv.bin -t c -e $(HI_XML_EXTC_ROOT)/ -l $(HI_XML_EXTC_ROOT)
endif
	chmod 777 $(HI_HIGV_BIN_ROOT)/higv.bin
	cp -u $(HI_XML_EXTC_ROOT)/higv_cextfile.c $(SRC_DIR)/
	cp -u $(HI_XML_EXTC_ROOT)/higv_cextfile.h $(SRC_ROOT)/include/
	cp -u $(HI_XML_EXTC_ROOT)/higv_language.h $(SRC_ROOT)/include/
	rm $(HI_XML_EXTC_ROOT)/*.c
	rm $(HI_XML_EXTC_ROOT)/*.h
	if [ -f piccompress ];then rm piccompress;fi
	if [ -f picstretch ];then rm picstretch;fi
	if [ -f fontsizeadapt ];then rm fontsizeadapt;fi

	make clean; make all;
################################################################
include $(REF_ROOT)/build/modbuild.mak

cpxml:
ifeq ($(CONFIG_LCD_UISRC_ADAPT_AUTO), y)
	cp -rf $(XML_PATH)/*.xml $(XML_PATH_DEST)
endif

fontsizeadapt:
ifeq ($(CONFIG_LCD_UISRC_ADAPT_AUTO), y)
	chmod +x $(CUR_ROOT)/fontsizeadapt.sh
	$(shell $(CUR_ROOT)/fontsizeadapt.sh $(CONFIG_LCD_UISRC_BASE_WIDTH) $(CONFIG_LCD_UISRC_BASE_HEIGHT) $(CONFIG_LCD_UISRC_DEST_WIDTH) $(CONFIG_LCD_UISRC_DEST_HEIGHT) 2> /dev/null)
endif

picstretch:
ifeq ($(CONFIG_LCD_UISRC_ADAPT_AUTO), y)
ifeq ($(picdest_file_cnt), 0)
	chmod +x $(CUR_ROOT)/picstretch.sh
	$(shell $(CUR_ROOT)/picstretch.sh $(CONFIG_LCD_UISRC_BASE_WIDTH) $(CONFIG_LCD_UISRC_BASE_HEIGHT) $(CONFIG_LCD_UISRC_DEST_WIDTH) $(CONFIG_LCD_UISRC_DEST_HEIGHT) 2> /dev/null)
endif
endif





