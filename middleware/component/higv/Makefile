#===============================================================================
#                         export variable
#===============================================================================
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak

#===============================================================================
#                         local variable
#===============================================================================
HIGV_DIR := higv
HIGO_DIR := higo

#===============================================================================
#                         rules
#===============================================================================
.PHONY : all clean install uninstall

all: higv higo
clean: higv_clean higo_clean
install: higv_install higo_install
uninstall:

#===============================================================================
#                         higv
#===============================================================================
.PHONY : higv higv_clean higv_install

higv:
	make -C $(HIGV_DIR) all

higv_clean:
	make -C $(HIGV_DIR) clean

higv_install:
	make -C $(HIGV_DIR) install  

#===============================================================================
#                         higo
#===============================================================================
.PHONY : higo higo_clean higo_install

higo:
	make -C $(HIGO_DIR) all

higo_clean:
	make -C $(HIGO_DIR) clean

higo_install:
	make -C $(HIGO_DIR) install  
